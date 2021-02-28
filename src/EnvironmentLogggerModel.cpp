#include "EnvironmentLoggerModel.hpp"

class MH_Z14_m5stack : MH_Z14 {
public:
    MH_Z14_m5stack(HardwareSerial *ser) : hw_serial(ser){};

private:
    HardwareSerial *hw_serial;

protected:
    void doInit() { hw_serial->begin(9600); };
    int checkReadableDataNum() { return hw_serial->available(); };

    void doFlush() { hw_serial->flush(); }

    int doRead(uint8_t *rbuf, uint32_t size) {
        int i = 0;
        for(i = 0; i < size; i++) {
            rbuf[i] = hw_serial->read();
        }
        return i;
    }

    void doWrite(const uint8_t *tbuf, uint32_t size) {
        hw_serial->write(tbuf, size);
    }

    int nowMs() { return millis(); }
};

static HardwareSerial HW_Serial(2);
static MH_Z14_m5stack mh_z14(&HW_Serial);

MH_Z14 *get_ptr_mhz14() { return (MH_Z14 *)&mh_z14; };

class BME280_m5stack : BME280 {
public:
    BME280_m5stack(TwoWire *wire) : p_wire(wire){};

private:
    TwoWire *p_wire;

protected:
    void doInit() {
        p_wire->begin();
        pinMode(21, INPUT_PULLUP);
        pinMode(22, INPUT_PULLUP);
    };

    void doRead(uint8_t r_addr, uint8_t *r_buf, uint32_t size) {
        p_wire->beginTransmission(device_i2c_address);
        p_wire->write(r_addr);
        p_wire->endTransmission();
        p_wire->requestFrom(device_i2c_address, size);
        int i = 0;
        while(p_wire->available()) {
            r_buf[i] = p_wire->read();
            i++;
        }
    }

    void doWrite(uint8_t w_addr, uint8_t w_data) {
        p_wire->beginTransmission(device_i2c_address);
        p_wire->write(w_addr);
        p_wire->write(w_data);
        p_wire->endTransmission();
    }

    int nowMs() { return millis(); }
};

static BME280_m5stack bme280(&Wire);

BME280 *get_ptr_bme280() { return (BME280 *)&bme280; };