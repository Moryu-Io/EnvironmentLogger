#include "MH_Z14.hpp"

/* 定数類 */
static constexpr uint32_t TIMEOUT_MS             = 3000;
static constexpr MH_Z14::TxCMD Tx_ReqReadGasConc = {0xFF, 0x01, 0x86, {}, 0x79};

/**
 * @brief 初期化
 *
 */
void MH_Z14::initialize() { doInit(); }

/**
 * @brief CO2濃度を取得する
 *
 * @param co2ppm [out] CO2濃度(単位ppm)
 * @return true
 * @return false
 */
bool MH_Z14::get_CO2_ppm(uint16_t &co2ppm) {
    doFlush();

    /* Co2濃度取得リクエストを投げる  */
    doWrite((uint8_t *)&Tx_ReqReadGasConc, sizeof(Tx_ReqReadGasConc));

    uint32_t preCalltime_ms = nowMs();
    while(checkReadableDataNum() < sizeof(RxCMD)) {
        if((nowMs() - preCalltime_ms) > TIMEOUT_MS) {
            return false;
        }
    }
    RxCMD rxcmd = {};
    doRead((uint8_t *)&rxcmd, sizeof(rxcmd));

    /* calc checksum  */
    uint8_t _checksum = 0;
    for(int i = 1; i < (sizeof(rxcmd) - 1); i++) {
        _checksum = _checksum + (uint8_t) * ((uint8_t *)&rxcmd + i);
    }
    _checksum = 0xFF - _checksum;
    _checksum += 1;
    if(_checksum != rxcmd.check) {
        return false;
    }

    /* CO2濃度の計算 */
    uint16_t _gasc_h = ((uint16_t)rxcmd.gas_high << 8) & 0xFF00;
    uint16_t _gasc_l = (uint16_t)rxcmd.gas_low & 0x00FF;
    co2ppm           = _gasc_h | _gasc_l;

    return true;
}
