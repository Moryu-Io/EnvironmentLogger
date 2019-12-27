#include "MH_Z14.hpp"
#include "BME280.hpp"

MH_Z14 mh_z14;
BME280 bme280;

void setup(){
    Serial.begin(9600);

    mh_z14.initialize();
    bme280.initialize();
    delay(1000);
}

void loop(){
  uint16_t _co2ppm = 0;

  if(mh_z14.get_CO2_ppm(_co2ppm)){
    String _co2_str = "CO2ppm = ";
    _co2_str += String(_co2ppm);
    Serial.println(_co2_str);
  }else{
    Serial.println("CO2 read failure...");
  }
    String _temp_str = "temp = ";
    _temp_str += String(bme280.read_Temp_s32()/1000);
    Serial.println(_temp_str);
      String _pres_str = "press = ";
    _pres_str += String(bme280.read_press_u32()>>8);
    Serial.println(_pres_str);
        String _hum_str = "humd = ";
    _hum_str += String(bme280.read_humidity_u32()>>10);
    Serial.println(_hum_str);
  delay(2000); 
}