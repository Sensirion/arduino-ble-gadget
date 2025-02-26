#include "SCD4xDataProvider.h"

void SCD4xDataProvider::_setupSCDBLECharacteristics() {
    SCDDataProvider::_setupSCDBLECharacteristics();
    if (_enableASCIntervalCharacteristic) {
        _BLELibrary.createCharacteristic(SCD_SERVICE_UUID,
                                         SCD_REQUEST_UUID_ASC_INTERVAL,
                                         Permission::READWRITE_PERMISSION);
        _BLELibrary.characteristicSetValue(SCD_REQUEST_UUID_ASC_INTERVAL, 0);
    }
    if (_enableASCInitIntervalCharacteristic) {
        _BLELibrary.createCharacteristic(SCD_SERVICE_UUID,
                                         SCD_REQUEST_UUID_ASC_INITIAL_INTERVAL,
                                         Permission::READWRITE_PERMISSION);
        _BLELibrary.characteristicSetValue(
            SCD_REQUEST_UUID_ASC_INITIAL_INTERVAL, 0);
    }
    if (_enableASCTargetCharacteristic) {
        _BLELibrary.createCharacteristic(SCD_SERVICE_UUID,
                                         SCD_REQUEST_UUID_ASC_TARGET,
                                         Permission::READWRITE_PERMISSION);
        _BLELibrary.characteristicSetValue(SCD_REQUEST_UUID_ASC_TARGET, 0);
    }
}

void SCD4xDataProvider::enableASCIntervalCharacteristic(
    std::function<void(std::string)> callbackFunction) {
    _BLELibrary.registerCallback(SCD_REQUEST_UUID_ASC_INTERVAL,
                                 callbackFunction);
    _enableSCDService = true;
    _enableASCIntervalCharacteristic = true;
}

void SCD4xDataProvider::enableASCInitIntervalCharacteristic(
    std::function<void(std::string)> callbackFunction) {
    _BLELibrary.registerCallback(SCD_REQUEST_UUID_ASC_INITIAL_INTERVAL,
                                 callbackFunction);
    _enableSCDService = true;
    _enableASCInitIntervalCharacteristic = true;
}

void SCD4xDataProvider::enableASCTargetCharacteristic(
    std::function<void(std::string)> callbackFunction) {
    _BLELibrary.registerCallback(SCD_REQUEST_UUID_ASC_TARGET, callbackFunction);
    _enableSCDService = true;
    _enableASCTargetCharacteristic = true;
}

void SCD4xDataProvider::setASCInitInterval(uint16_t value) {
    _BLELibrary.characteristicSetValue(SCD_REQUEST_UUID_ASC_INITIAL_INTERVAL,
                                       (int)value);
}

void SCD4xDataProvider::setASCInterval(uint16_t value) {
    _BLELibrary.characteristicSetValue(SCD_REQUEST_UUID_ASC_INTERVAL,
                                       (int)value);
}

void SCD4xDataProvider::setASCTarget(uint16_t value) {
    _BLELibrary.characteristicSetValue(SCD_REQUEST_UUID_ASC_TARGET, (int)value);
}
