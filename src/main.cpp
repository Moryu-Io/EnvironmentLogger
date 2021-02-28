#include "Ambient_private_def.hpp"
#include "EnvironmentLoggerModel.hpp"
#include <Arduino.h>
#include <Wire.h>
//#include <WiFi.h>
#include "Ambient.h"
#include "esp_deep_sleep.h"
#include "time.h"
#include <M5Stack.h>

const char *ntpServer        = "ntp.jst.mfeed.ad.jp";
const long gmtOffset_sec     = 9 * 3600;
const int daylightOffset_sec = 0;
static uint8_t
conv2d(const char *p); // Forward declaration needed for IDE 1.6.x
uint8_t hh = conv2d(__TIME__), mm = conv2d(__TIME__ + 3),
        ss  = conv2d(__TIME__ + 6); // Get H, M, S from compile time
byte xcolon = 0, xsecs = 0;

WiFiClient client;
Ambient ambient;

MH_Z14 *p_mh_z14;
BME280 *p_bme280;
// BME280 bme280(HSPI, 2, 5, 36, 35);
// C LK 青 2
// CS 緑 35
// MISO/SDO 紫 5
// MOSI/SDI 灰 36

String _time_str = "";
String _co2_str  = "";
String _temp_str = "";
String _pres_str = "";
String _hum_str  = "";

void LCDprint_erase() {
    M5.Lcd.sleep();
    M5.Lcd.setBrightness(0);
}

void LCDprint_environmentaldata() {
    M5.Lcd.clear();
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(2);

    M5.Lcd.println(_time_str);
    M5.Lcd.println(_co2_str);
    M5.Lcd.println(_temp_str);
    M5.Lcd.println(_pres_str);
    M5.Lcd.println(_hum_str);
}

// Function to extract numbers from compile time string
static uint8_t conv2d(const char *p) {
    uint8_t v = 0;
    if('0' <= *p && *p <= '9') v = *p - '0';
    return 10 * v + *++p - '0';
}

hw_timer_t *samplingTimer = NULL;
void IRAM_ATTR onTimer0() {
    static uint32_t pushed_flag_and_timecount = 0;
    if(M5.BtnC.isPressed() && (pushed_flag_and_timecount == 0)) {
        M5.Lcd.begin();
        LCDprint_environmentaldata();
        pushed_flag_and_timecount = 1;
    }

    if(pushed_flag_and_timecount >= 100) {
        LCDprint_erase();
        pushed_flag_and_timecount = 0;
    } else if(pushed_flag_and_timecount >= 1) {
        pushed_flag_and_timecount++;
    }
}

void setup() {
    p_mh_z14 = get_ptr_mhz14();
    p_bme280 = get_ptr_bme280();
    M5.begin();
    Serial.begin(9600);
    M5.Lcd.println("Hello!");

    p_mh_z14->initialize();
    p_bme280->initialize();

    // WiFi.begin(WIFI_SSID, WIFI_KEY);
    while(WiFi.status() != WL_CONNECTED) { //  Wi-Fi AP接続待ち
        WiFi.begin(WIFI_SSID, WIFI_KEY);
        delay(1000);
    }
    M5.Lcd.println("World!1111");

    ambient.begin(AMB_CHANNEL_ID, (const char *)AMB_WRITE_KEY, &client);

    samplingTimer = timerBegin(0, 8000, true); // 100usのタイマーを初期設定する
    timerAttachInterrupt(samplingTimer, &onTimer0,
                         true); // 割り込み処理関数を設定する
    timerAlarmWrite(samplingTimer, 100 * 10,
                    true); // 100msのタイマー値を設定する

    timerAlarmEnable(samplingTimer); // タイマーを起動する

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
    LCDprint_erase();

    int32_t wifi_request_count = 0;
    WiFi.begin(WIFI_SSID, WIFI_KEY);
    while(WiFi.status() != WL_CONNECTED) { //  Wi-Fi AP接続待ち
        wifi_request_count++;
        if(wifi_request_count > 100) {
            M5.Lcd.println("Wifi connection error!");
            WiFi.begin(WIFI_SSID, WIFI_KEY);
            wifi_request_count = 0;
        }
        delay(100);
    }
    // M5.Lcd.clear();
    // M5.Lcd.setCursor(0,0);
    // M5.Lcd.setTextSize(2);

    ambient.begin(AMB_CHANNEL_ID, (const char *)AMB_WRITE_KEY, &client);

    /* CO2取得 */
    uint16_t _co2ppm = 0;
    p_mh_z14->get_CO2_ppm(_co2ppm);
    _co2_str = "CO2   : ";
    _co2_str += String(_co2ppm);
    _co2_str += String(" [ppm]");
    // M5.Lcd.println(_co2_str);

    /* 気温取得 */
    float _temp_f = (float)p_bme280->readTemperature();
    _temp_str     = "temp  : ";
    _temp_str += String(_temp_f);
    _temp_str += String(" [deg]");
    // M5.Lcd.println(_temp_str);

    /* 気圧取得 */
    float _press_f = (float)p_bme280->readPressure();
    _pres_str      = "press : ";
    _pres_str += String(_press_f);
    _pres_str += String(" [hPa]");
    // M5.Lcd.println(_pres_str);

    /* 湿度取得 */
    float _hum_f = (float)p_bme280->readHumidity();
    _hum_str     = "humd  : ";
    _hum_str += String(_hum_f);
    _hum_str += String(" [%]");
    // M5.Lcd.println(_hum_str);

    if(_press_f > 700.0f) {
        ambient.set(1, String(_temp_f).c_str());
        ambient.set(2, String(_press_f).c_str());
        ambient.set(3, String(_hum_f).c_str());
        ambient.set(4, String(_co2ppm).c_str());
        ambient.send();
        delay(2);
    }

    struct tm timeinfo;
    getLocalTime(&timeinfo);
    _time_str = String(timeinfo.tm_year + 1900);
    _time_str += String(" / ");
    _time_str += String(timeinfo.tm_mon + 1);
    _time_str += String(" / ");
    _time_str += String(timeinfo.tm_mday);
    _time_str += String("   ");
    _time_str += String(timeinfo.tm_hour);
    _time_str += String(" : ");
    _time_str += String(timeinfo.tm_min);

    WiFi.disconnect(true);

    static uint32_t last_process_time_ms = 0;
    while(millis() - last_process_time_ms < 60 * 1000) {
        delay(100);
        M5.update();
    }
    last_process_time_ms = millis();
    // delay(60*1000);
}