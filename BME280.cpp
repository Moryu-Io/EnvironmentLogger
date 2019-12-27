#include "BME280.hpp"

void BME280::initialize(){
    pinMode(PIN_CS, OUTPUT);
    SPIClass::begin(PIN_SCK,
                    PIN_MISO,
                    PIN_MOSI,
                    PIN_CS);

    SPIClass::beginTransaction(
        SPISettings(SPI_CLOCK_HZ,
                    MSBFIRST,
                    SPI_MODE3));

    digitalWrite(PIN_CS, HIGH);

    write_data(BME280_REG_CONFIG, 0b10100100);
    write_data(BME280_REG_CTRL_MEAS, 0b10110101);
    write_data(BME280_REG_CTRL_HUM,  0b00000101);
    now_mode = mode::FORCED;

    read_calib();
}

void BME280::write_data(uint8_t _reg, uint8_t _d){
    uint8_t _w_reg = (_reg & 0b01111111);

    digitalWrite(PIN_CS, LOW);

    SPIClass::transfer(_w_reg);
    SPIClass::transfer(_d);

    digitalWrite(PIN_CS, HIGH);
}

uint8_t BME280::read_data(uint8_t _reg){
    uint8_t _r_reg = _reg | 0b10000000;

    digitalWrite(PIN_CS, LOW);
    
    SPIClass::transfer(_r_reg);
    uint8_t _r_data = SPIClass::transfer(0x00);

    digitalWrite(PIN_CS, HIGH);

    return _r_data;
}

void BME280::read_dataarray(uint8_t _reg, uint32_t _len, uint8_t* _d){
    uint8_t _r_reg = _reg | 0b10000000;
    uint8_t _dummy[256] = {};

    digitalWrite(PIN_CS, LOW);   

    SPIClass::transfer(_r_reg);
    for(int i=0;i<_len;i++){
        _d[i] = SPIClass::transfer(0x00);
    }

    digitalWrite(PIN_CS, HIGH);
}

void BME280::set_mode(mode _m){
    now_mode = _m;
    uint8_t _d = read_data(BME280_REG_CTRL_MEAS);
    uint8_t _w_d = (_d & 0b11111100) | (uint8_t)_m;

    write_data(BME280_REG_CTRL_MEAS, _w_d);
}


void BME280::read_calib(){
    uint8_t _r_data1[BME280_REGLEN00] = {};
    uint8_t _r_data2[BME280_REGLEN26] = {};

    read_dataarray(BME280_REG_CALIB00, BME280_REGLEN00, _r_data1);
    read_dataarray(BME280_REG_CALIB26, BME280_REGLEN26, _r_data2);

    dig_T1 = *(uint16_t*)(&(_r_data1[0]));
    dig_T2 = *(int16_t*)(&(_r_data1[2]));
    dig_T3 = *(int16_t*)(&(_r_data1[4]));
    
    dig_P1 = *(uint16_t*)(&(_r_data1[6]));
    dig_P2 = *(int16_t*)(&(_r_data1[8]));
    dig_P3 = *(int16_t*)(&(_r_data1[10]));
    dig_P4 = *(int16_t*)(&(_r_data1[12]));
    dig_P5 = *(int16_t*)(&(_r_data1[14]));
    dig_P6 = *(int16_t*)(&(_r_data1[16]));
    dig_P7 = *(int16_t*)(&(_r_data1[18]));
    dig_P8 = *(int16_t*)(&(_r_data1[20]));
    dig_P9 = *(int16_t*)(&(_r_data1[22]));

    dig_H1 = _r_data1[25];
    dig_H2 = *(int16_t*)(&(_r_data2[0]));
    dig_H3 = _r_data2[2];

    dig_H4 = (int16_t)((((uint16_t)_r_data2[3]<<4) & 0xFFF0) | ((uint16_t)_r_data2[4] & 0x000F));
    dig_H5 = (int16_t)((((uint16_t)_r_data2[4]<<4) & 0x0F00) | ((uint16_t)_r_data2[5] & 0x00FF));
    dig_H6 = _r_data2[6];

}

int32_t BME280::read_Temp_s32(){
    uint8_t _temp_arr[3] = {};
    read_dataarray(BME280_REG_TEMP_MSB, 3, _temp_arr);

    int32_t _temp_high = ((int32_t)_temp_arr[0] << 12)  & 0xFF000;
    int32_t _temp_mid = ((int32_t)_temp_arr[1] << 4)    & 0x00FF0;
    int32_t _temp_low = ((int32_t)_temp_arr[2] >> 4)    & 0x0000F;

    int32_t _temp_adc = _temp_high | _temp_mid | _temp_low;

    // 補整計算
    int32_t _var1, _var2, _temp_cor;

    _var1 = (((_temp_adc>>3) - (((int32_t)dig_T1<<1))) * ((int32_t)dig_T2))>>11;
    _var2 = (((((_temp_adc>>4) - ((int32_t)dig_T1)) * ((_temp_adc>>4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;

    t_fine = _var1 + _var2;

    _temp_cor = (t_fine*5 + 128) >> 8;

    return _temp_cor;
}

uint32_t BME280::read_press_u32(){
    uint8_t _press_arr[3] = {};
    read_dataarray(BME280_REG_PRESS_MSB, 3, _press_arr);

    int32_t _p_high = ((int32_t)_press_arr[0] << 12)  & 0xFF000;
    int32_t _p_mid = ((int32_t)_press_arr[1] << 4)    & 0x00FF0;
    int32_t _p_low = ((int32_t)_press_arr[2] >> 4)    & 0x0000F;

    int32_t _p_adc = _p_high | _p_mid | _p_low;

    // 補整計算
    int64_t _var1, _var2, _p_cor;

    _var1 = (int64_t)t_fine - 128000;
    _var2 = _var1*_var1*(int64_t)dig_P6;
    _var2 = _var2 + ((_var1*(int64_t)dig_P5)<<17);
    _var2 = _var2 + (((int64_t)dig_P4) << 35);
    _var1 = ((_var1*_var1*(int64_t)dig_P3)>>8) + ((_var1*(int64_t)dig_P2)<<12);
    _var1 = ((((int64_t)1)<<47)+_var1) * ((int64_t)dig_P1)>>33;

    if(_var1==0) return 0;
    _p_cor = 1048576 - _p_adc;
    _p_cor = (((_p_cor<<31) - _var2)*3125)/_var1;

    _var1 = (((int64_t)dig_P9)*(_p_cor>>13) * (_p_cor>>13)) >> 25;
    _var2 = (((int64_t)dig_P8) * _p_cor) >> 19;

    _p_cor = ((_p_cor + _var1 + _var2) >> 8) + (((int64_t)dig_P7)<<4);

    return (uint32_t)_p_cor;
}

uint32_t BME280::read_humidity_u32(){
    uint8_t _hum_arr[2] = {};
    read_dataarray(BME280_REG_HUM_MSB, 2, _hum_arr);

    int32_t _h_high = ((int32_t)_hum_arr[0] << 8)  & 0xFF00;
    int32_t _h_low = ((int32_t)_hum_arr[1])    & 0x00FF;

    int32_t _h_adc = _h_high |  _h_low;

    // 補整計算
    int32_t _var1, _h_cor;

    _var1 = (t_fine - ((int32_t)76800));
    _var1 = (((((_h_adc << 14) -(((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * _var1)) + 
            ((int32_t)16384)) >> 15) * (((((((_var1 * ((int32_t)dig_H6)) >> 10) * 
            (((_var1 * ((int32_t)dig_H3)) >> 11) + ((int32_t) 32768))) >> 10) + ((int32_t)2097152)) * 
            ((int32_t) dig_H2) + 8192) >> 14));
    _var1 = (_var1 - (((((_var1 >> 15) * (_var1 >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
    _var1 = (_var1 < 0 ? 0 : _var1);
    _h_cor = ((_var1 > 419430400 ? 419430400 : _var1) >> 12);

    return (uint32_t)_h_cor;
}