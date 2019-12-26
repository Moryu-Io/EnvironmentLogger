#include "MH_Z14.hpp"


MH_Z14::TxCMD Tx_ReqReadGasConc = {
    0xFF, 0x01, 0x86, {}, 0x79
};

void MH_Z14::initialize(){
    HardwareSerial::begin(9600);
}

bool MH_Z14::get_CO2_ppm(uint16_t& co2ppm){
    RxCMD Rx_GasConc = {};

    HardwareSerial::flush();
    HardwareSerial::write((uint8_t*)&Tx_ReqReadGasConc, 9);

    while(HardwareSerial::available()<9);

    uint8_t _checksum = 0;
    uint8_t _rbuf = 0; 
    for(int i=0;i<9;i++){
        _rbuf = HardwareSerial::read();
        
        ((uint8_t*)&Rx_GasConc)[i] = _rbuf;
        if((i != 0) && (i != 8)) _checksum += _rbuf;
    }
    _checksum = 0xFF - _checksum;
    _checksum += 1;
    if(_checksum != Rx_GasConc.check){
        return false;
    }

    uint16_t _gasc_h = ((uint16_t)Rx_GasConc.gas_high << 8) & 0xFF00;
    uint16_t _gasc_l = (uint16_t)Rx_GasConc.gas_low & 0x00FF;
    co2ppm = _gasc_h | _gasc_l;

    return true;
}

