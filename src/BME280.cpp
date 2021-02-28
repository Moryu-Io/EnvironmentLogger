#include "BME280.hpp"

/* 定数類 */
static constexpr uint8_t REG_ADDR_CALIB_T_P = 0x88;
static constexpr uint8_t REG_ADDR_CALIB_H1  = 0xA1;
static constexpr uint8_t REG_ADDR_CALIB_H2  = 0xE1;
static constexpr uint8_t REG_ADDR_CTRL_HUM  = 0xF2;
static constexpr uint8_t REG_ADDR_CTRL_MEAS = 0xF4;
static constexpr uint8_t REG_ADDR_CONFIG    = 0xF5;
static constexpr uint8_t REG_ADDR_PRES  = 0xF7;
static constexpr uint8_t REG_ADDR_TEMP  = 0xFA;
static constexpr uint8_t REG_ADDR_HUMID = 0xFD;

static constexpr uint8_t REG_SIZE_CALIB_T_P = 24;
static constexpr uint8_t REG_SIZE_CALIB_H1  = 1;
static constexpr uint8_t REG_SIZE_CALIB_H2  = 7;
static constexpr uint8_t REG_SIZE_PRES  = 3;
static constexpr uint8_t REG_SIZE_TEMP  = 3;
static constexpr uint8_t REG_SIZE_HUMID = 2;

/**
 * @brief 初期化
 * 
 */
void BME280::initialize(void) {
    doInit();

    uint8_t osrs_t   = 1; // Temperature oversampling x 1
    uint8_t osrs_p   = 1; // Pressure oversampling x 1
    uint8_t osrs_h   = 1; // Humidity oversampling x 1
    uint8_t mode     = 3; // Normal mode
    uint8_t t_sb     = 5; // Tstandby 1000ms
    uint8_t filter   = 0; // Filter off
    uint8_t spi3w_en = 0; // 3-wire SPI Disable

    uint8_t ctrl_meas_reg = (osrs_t << 5) | (osrs_p << 2) | mode;
    uint8_t config_reg    = (t_sb << 5) | (filter << 2) | spi3w_en;
    uint8_t ctrl_hum_reg  = osrs_h;

    doWrite(REG_ADDR_CTRL_HUM, ctrl_hum_reg);
    doWrite(REG_ADDR_CTRL_MEAS, ctrl_meas_reg);
    doWrite(REG_ADDR_CONFIG, config_reg);

    readTrim();
}

/**
 * @brief 現在の気温を取得
 * 
 * @return float : 気温[deg]
 */
float BME280::readTempDeg(void) {
    uint8_t data[REG_SIZE_TEMP];

    doRead(REG_ADDR_TEMP, data, REG_SIZE_TEMP);

    int64_t temp_raw = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);

    /* 式の意味はデータシート参照 */
    int64_t var1, var2;
    var1 = (((temp_raw >> 3) - ((int64_t)dig_T1 << 1)) * (int64_t)dig_T2) >> 11;
    var2 = (((((temp_raw >> 4) - (int64_t)dig_T1) * ((temp_raw >> 4) - (int64_t)dig_T1)) >> 12)
            * (int64_t)dig_T3) >> 14;

    pre_temp = var1 + var2;

    return ((pre_temp * 5 + 128) >> 8) / 100.0;
}

/**
 * @brief 現在の気圧を取得する
 * 
 * @return float : 気圧[hPa]
 */
float BME280::readPreshPa(void) {
    uint8_t data[REG_SIZE_PRES];

    doRead(REG_ADDR_PRES, data, REG_SIZE_PRES);

    uint64_t pres_raw = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);

    /* 式の意味はデータシート参照 */
    int64_t var1, var2;
    var1 = (pre_temp >> 1) - (int64_t)64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 1);
    var2 = (var2 >> 2) + ((int64_t)dig_P4 << 16);
    var1 = (((dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) +
            (((int64_t)dig_P2 * var1) >> 1)) >> 18;
    var1 = ((0x8000 + var1) * (int64_t)dig_P1) >> 15;
    if(var1 == 0) { return 0; }

    uint64_t P = ((uint64_t)(0x100000 - pres_raw) - (var2 >> 12)) * 3125;
    if(P < 0x80000000) {
        P = (P << 1) / (uint64_t)var1;
    } else {
        P = 2 * P / (uint64_t)var1;
    }
    var1 = (((int64_t)dig_P9) * ((int64_t)(((P >> 3) * (P >> 3)) >> 13))) >> 12;
    var2 = ((int64_t)(P >> 2) * (int64_t)dig_P8) >> 13;
    P    = (uint64_t)((int64_t)P + ((var1 + var2 + dig_P7) >> 4));

    return P / 100.0;
}

/**
 * @brief 現在の湿度を取得する
 * 
 * @return float : 湿度[%]
 */
float BME280::readHumidPcnt(void) {
    uint8_t data[REG_SIZE_HUMID];

    doRead(REG_ADDR_HUMID, data, REG_SIZE_HUMID);

    uint64_t hum_raw = (data[0] << 8) | data[1];

    /* 式の意味はデータシート参照 */
    int64_t v_x1 = (pre_temp - ((int64_t)76800));
    v_x1 = ((((hum_raw << 14) - (((int64_t)dig_H4) << 20) - (int64_t)dig_H5 * v_x1) + (int64_t)16384) >> 15)
            * (((((((v_x1 * (int64_t)dig_H6) >> 10) * (((v_x1 * (int64_t)dig_H3) >> 11) + (int64_t)32768)) >> 10) + (int64_t)2097152)
                * (int64_t)dig_H2
               + 8192) >> 14);
    v_x1 = v_x1 - (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) * (int64_t)dig_H1) >> 4);

    v_x1 = (v_x1 < 0) ? 0 : v_x1;
    v_x1 = (v_x1 > 419430400) ? 419430400 : v_x1;
    
    return (uint64_t)(v_x1 >> 12) / 1024.0;
}

/**
 * @brief キャリブレーション情報を取得
 * 
 */
void BME280::readTrim() {
    uint8_t data[32];

    doRead(REG_ADDR_CALIB_T_P, data, REG_SIZE_CALIB_T_P);
    doRead(REG_ADDR_CALIB_H1, &data[24], REG_SIZE_CALIB_H1);
    doRead(REG_ADDR_CALIB_H2, &data[25], REG_SIZE_CALIB_H2);

    dig_T1 = (data[1] << 8) | data[0];
    dig_T2 = (data[3] << 8) | data[2];
    dig_T3 = (data[5] << 8) | data[4];

    dig_P1 = (data[7] << 8) | data[6];
    dig_P2 = (data[9] << 8) | data[8];
    dig_P3 = (data[11] << 8) | data[10];
    dig_P4 = (data[13] << 8) | data[12];
    dig_P5 = (data[15] << 8) | data[14];
    dig_P6 = (data[17] << 8) | data[16];
    dig_P7 = (data[19] << 8) | data[18];
    dig_P8 = (data[21] << 8) | data[20];
    dig_P9 = (data[23] << 8) | data[22];

    dig_H1 = data[24];
    dig_H2 = (data[26] << 8) | data[25];
    dig_H3 = data[27];
    dig_H4 = (data[28] << 4) | (0x0F & data[29]);
    dig_H5 = (data[30] << 4) | ((data[29] >> 4) & 0x0F);
    dig_H6 = data[31];
}
