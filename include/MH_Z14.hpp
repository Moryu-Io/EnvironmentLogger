#ifndef MH_Z14_HPP_
#define MH_Z14_HPP_

#include <stdint.h>

class MH_Z14 {
public:
    MH_Z14(){};

    struct TxCMD {
        uint8_t start;
        uint8_t sensNo;
        uint8_t cmd;
        uint8_t dummy[5];
        uint8_t check;
    };

    struct RxCMD {
        uint8_t start;
        uint8_t cmd;
        uint8_t gas_high;
        uint8_t gas_low;
        uint8_t dummy[4];
        uint8_t check;
    };

protected:
    /* Driverアクセス */
    virtual void doInit()                                    = 0;
    virtual int checkReadableDataNum()                       = 0;
    virtual void doFlush()                                   = 0;
    virtual int doRead(uint8_t *rbuf, uint32_t size)         = 0;
    virtual void doWrite(const uint8_t *tbuf, uint32_t size) = 0;

    /* タイムアウト用カウント */
    virtual int nowMs() = 0;

public:
    void initialize();
    bool get_CO2_ppm(uint16_t &co2ppm);
};

#endif