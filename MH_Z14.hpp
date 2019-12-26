#ifndef MH_Z14_HPP_
#define MH_Z14_HPP_

#include <Arduino.h> 

class MH_Z14 : HardwareSerial{
    public:
        MH_Z14() : HardwareSerial(2){};
        MH_Z14(int i) : HardwareSerial(i){};

        struct TxCMD{
            uint8_t start;
            uint8_t sensNo;
            uint8_t cmd;
            uint8_t dummy[5];
            uint8_t check;
        };

        struct RxCMD{
            uint8_t start;
            uint8_t cmd;
            uint8_t gas_high;
            uint8_t gas_low;
            uint8_t dummy[4];
            uint8_t check;            
        };

    void initialize();
    bool get_CO2_ppm(uint16_t& co2ppm);

    private:
        
    
        

};

#endif