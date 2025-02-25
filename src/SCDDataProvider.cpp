#include "SCDDataProvider.h"

void SCDDataProvider::_setupSCDBLECharacteristics() {
    if (_enableForcedRecalibrationCharacteristic) {
        _BLELibrary.createCharacteristic(SCD_SERVICE_UUID, SCD_FRC_REQUEST_UUID,
                                         Permission::READWRITE_PERMISSION);
        _BLELibrary.characteristicSetValue(SCD_FRC_REQUEST_UUID, 0);
    }
    if (_enableMeasurementIntervalCharacteristic) {
        _BLELibrary.createCharacteristic(SCD_SERVICE_UUID,
                                         SCD_REQUEST_UUID_MEASUREMENT_INTERVAL,
                                         Permission::READWRITE_PERMISSION);
        _BLELibrary.characteristicSetValue(
            SCD_REQUEST_UUID_MEASUREMENT_INTERVAL, 0);
    }
    if (_enableTempOffsetCharacteristic) {
        _BLELibrary.createCharacteristic(SCD_SERVICE_UUID,
                                         SCD_REQUEST_UUID_TEMPERATURE_OFFSET,
                                         Permission::READWRITE_PERMISSION);
        _BLELibrary.characteristicSetValue(SCD_REQUEST_UUID_TEMPERATURE_OFFSET,
                                           0);
    }
    if (_enableAltitudeCharacteristic) {
        _BLELibrary.createCharacteristic(SCD_SERVICE_UUID,
                                         SCD_REQUEST_UUID_ALTITUDE_SETTING,
                                         Permission::READWRITE_PERMISSION);
        _BLELibrary.characteristicSetValue(SCD_REQUEST_UUID_ALTITUDE_SETTING,
                                           0);
    }
    if (_enableASCCharacteristic) {
        _BLELibrary.createCharacteristic(SCD_SERVICE_UUID,
                                         SCD_REQUEST_UUID_ASC_STATUS,
                                         Permission::READWRITE_PERMISSION);
        _BLELibrary.characteristicSetValue(SCD_REQUEST_UUID_ASC_STATUS, 0);
    }
}

void SCDDataProvider::_setupBLEServices() {
    DataProvider::_setupBLEServices();
    // SCD Service
    if (_enableSCDService) {
        _BLELibrary.createService(SCD_SERVICE_UUID);
        _setupSCDBLECharacteristics();
        _BLELibrary.startService(SCD_SERVICE_UUID);
    }
}

void SCDDataProvider::enableMeasurementIntervalCharacteristic(
    std::function<void(std::string)> callbackFunction) {
    _BLELibrary.registerCallback(SCD_REQUEST_UUID_MEASUREMENT_INTERVAL,
                                 callbackFunction);
    _enableSCDService = true;
    _enableMeasurementIntervalCharacteristic = true;
}

void SCDDataProvider::enableTempOffsetCharacteristic(
    std::function<void(std::string)> callbackFunction) {
    _BLELibrary.registerCallback(SCD_REQUEST_UUID_TEMPERATURE_OFFSET,
                                 callbackFunction);
    _enableSCDService = true;
    _enableTempOffsetCharacteristic = true;
}

void SCDDataProvider::enableAltitudeCharacteristic(
    std::function<void(std::string)> callbackFunction) {
    _BLELibrary.registerCallback(SCD_REQUEST_UUID_ALTITUDE_SETTING,
                                 callbackFunction);
    _enableSCDService = true;
    _enableAltitudeCharacteristic = true;
}

void SCDDataProvider::enableForcedRecalibrationCharacteristic(
    std::function<void(std::string)> callbackFunction) {
    _BLELibrary.registerCallback(SCD_FRC_REQUEST_UUID, callbackFunction);
    _enableSCDService = true;
    _enableForcedRecalibrationCharacteristic = true;
}

void SCDDataProvider::enableASCCharacteristic(
    std::function<void(std::string)> callbackFunction) {
    _BLELibrary.registerCallback(SCD_REQUEST_UUID_ASC_STATUS, callbackFunction);
    _enableSCDService = true;
    _enableASCCharacteristic = true;
}

void SCDDataProvider::setASCStatus(uint16_t value) {
    _BLELibrary.characteristicSetValue(SCD_REQUEST_UUID_ASC_STATUS, (int)value);
}
void SCDDataProvider::setMeasurementInterval(uint16_t value) {
    _BLELibrary.characteristicSetValue(SCD_REQUEST_UUID_MEASUREMENT_INTERVAL,
                                       (int)value);
}
void SCDDataProvider::setTempOffset(uint16_t value) {
    _BLELibrary.characteristicSetValue(SCD_REQUEST_UUID_TEMPERATURE_OFFSET,
                                       (int)value);
}
void SCDDataProvider::setAltitude(uint16_t value) {
    _BLELibrary.characteristicSetValue(SCD_REQUEST_UUID_ALTITUDE_SETTING,
                                       (int)value);
}
