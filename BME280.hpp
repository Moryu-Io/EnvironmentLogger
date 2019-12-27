#ifndef BME280_HPP_
#define BME280_HPP_

#include <Arduino.h>
#include <SPI.h>

class BME280 : SPIClass{
public:
    BME280() : SPIClass(HSPI){};
    BME280(uint8_t spi_bus, int8_t sck, int8_t miso, int8_t mosi, int8_t ss) 
        : SPIClass(spi_bus), PIN_SCK(sck), PIN_MISO(miso), PIN_MOSI(mosi), PIN_CS(ss){};

    void initialize();

    int32_t read_Temp_s32();
    uint32_t read_press_u32();
    uint32_t read_humidity_u32();

    enum mode{
        SLEEP = 0x00,
        FORCED = 0x01,
        NORMAL = 0x11
    };
    void set_mode(mode _m);

private:
    int8_t PIN_SCK = 14;
    int8_t PIN_MISO = 12;
    int8_t PIN_MOSI = 13;
    int8_t PIN_CS = 26;
    uint32_t SPI_CLOCK_HZ = 1000000;

    void write_data(uint8_t _reg, uint8_t _d);
    uint8_t read_data(uint8_t _reg);
    void read_dataarray(uint8_t _reg, uint32_t _len, uint8_t* _d);

    // 補整係数
    void read_calib();
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;

    int32_t t_fine = 0;
    mode now_mode = mode::SLEEP;
};

constexpr static uint8_t BME280_REG_HUM_LSB     = 0xFE;
constexpr static uint8_t BME280_REG_HUM_MSB     = 0xFD;
constexpr static uint8_t BME280_REG_TEMP_xLSB   = 0xFC;
constexpr static uint8_t BME280_REG_TEMP_LSB    = 0xFB;
constexpr static uint8_t BME280_REG_TEMP_MSB    = 0xFA; 
constexpr static uint8_t BME280_REG_PRESS_xLSB  = 0xF9;
constexpr static uint8_t BME280_REG_PRESS_LSB   = 0xF8;
constexpr static uint8_t BME280_REG_PRESS_MSB   = 0xF7;

constexpr static uint8_t BME280_REG_CONFIG      = 0xF5;
constexpr static uint8_t BME280_REG_CTRL_MEAS   = 0xF4;
constexpr static uint8_t BME280_REG_STATUS      = 0xF3;
constexpr static uint8_t BME280_REG_CTRL_HUM    = 0xF2;
constexpr static uint8_t BME280_REG_RESET       = 0xE0;
constexpr static uint8_t BME280_REG_ID          = 0xD0;

constexpr static uint8_t BME280_REG_CALIB00 = 0x88;
constexpr static uint8_t BME280_REGLEN00      = 26;

constexpr static uint8_t BME280_REG_CALIB26 = 0xE1;
constexpr static uint8_t BME280_REGLEN26      = 16;


#endif