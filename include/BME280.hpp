#ifndef BME280_HPP_
#define BME280_HPP_

#include <Wire.h>

#define BME280_ADDRESS 0x76

class BME280 {
public:
    BME280(void);

    virtual ~BME280();

    void initialize(void);

    double readTemperature(void);

    double readPressure(void);

    double readHumidity(void);

private:
    signed long int t_fine;

    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;

    void writeReg(uint8_t reg_address, uint8_t data);
    void readTrim(void);

protected:
    uint8_t device_i2c_address = 0x76;

    /* Driverアクセス */
    virtual void doInit()                                              = 0;
    virtual void doRead(uint8_t r_addr, uint8_t *r_buf, uint32_t size) = 0;
    virtual void doWrite(uint8_t w_addr, uint8_t w_data)               = 0;

    /* タイムアウト用カウント */
    virtual int nowMs() = 0;
};

#endif // MBED_BME280_H
