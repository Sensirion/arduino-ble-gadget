/*
 GadgetBle.h - Library for providing sensor data via Bluetooth.
 Created by Bjoern Muntwyler, November 7, 2020.
 Released under BSD-3 licence
 */

// TODO: Still need to properly handle packet size and sample type/adv. sample
// type/logger sample type in respective packets

#include "Sensirion_GadgetBle_Lib.h"
#include "Arduino.h"

static const int INVALID_POSITION = -1;
static const int ADV_SAMPLE_OFFSET = 6;

GadgetBle::GadgetBle(DataType dataType) {
    // Company identifier
    _advertisedData[0] = 0xD5;
    _advertisedData[1] = 0x06;

    _deviceIdString = "n/a";

    setDataType(dataType);
}

void GadgetBle::enableWifiSetupSettings(
    std::function<void(std::string, std::string)> onWifiSettingsChanged) {
    _onWifiSettingsChanged = onWifiSettingsChanged;
}

void GadgetBle::setCurrentWifiSsid(std::string ssid) {
    _wifiSsidSetting = ssid;
    if (_wifiSsidChar != NULL) {
        _wifiSsidChar->setValue(_wifiSsidSetting);
    }
}

void GadgetBle::begin() {
    _bleInit();
}

void GadgetBle::setDataType(DataType dataType) {
    switch (dataType) {
        case T_RH_V3:
            _sampleType = {
                DataType::T_RH_V3, // datatype
                0,                 // advertisementType
                4,                 // advSampleType
                0,                 // dlSampleType
                4,                 // sampleSize
                4,                 // sampleCntPerPacket
                {{Unit::T, {0, &GadgetBle::_convertTemperatureV1}}, // unitEnc
                 {Unit::RH, {2, &GadgetBle::_convertHumidityV1}}},
            };
            break;
        case T_RH_V4:
            _sampleType = {
                DataType::T_RH_V4, // datatype
                0,                 // advertisementType
                6,                 // advSampleType
                5,                 // dlSampleType
                4,                 // sampleSize
                4,                 // sampleCntPerPacket
                {{Unit::T, {0, &GadgetBle::_convertTemperatureV1}}, // unitEnc
                 {Unit::RH, {2, &GadgetBle::_convertHumidityV2}}},
            };
            break;
        case T_RH_VOC:
            _sampleType = {
                DataType::T_RH_VOC, // datatype
                0,                  // advertisementType
                3,                  // advSampleType
                1,                  // dlSampleType
                6,                  // sampleSize
                3,                  // sampleCntPerPacket
                {{Unit::T, {0, &GadgetBle::_convertTemperatureV1}}, // unitEnc
                 {Unit::RH, {2, &GadgetBle::_convertHumidityV1}},
                 {Unit::VOC, {4, &GadgetBle::_convertSimple}}},
            };
            break;
        case T_RH_CO2:
            _sampleType = {
                DataType::T_RH_CO2, // datatype
                0,                  // advertisementType
                10,                 // advSampleType
                9,                  // dlSampleType
                6,                  // sampleSize
                3,                  // sampleCntPerPacket
                {{Unit::T, {0, &GadgetBle::_convertTemperatureV1}}, // unitEnc
                 {Unit::RH, {2, &GadgetBle::_convertHumidityV1}},
                 {Unit::CO2, {4, &GadgetBle::_convertSimple}}},
            };
            break;
        case T_RH_CO2_ALT:
            _sampleType = {
                DataType::T_RH_CO2_ALT, // datatype
                0,                      // advertisementType
                8,                      // advSampleType
                7,                      // dlSampleType
                8,                      // sampleSize
                2,                      // sampleCntPerPacket
                {{Unit::T, {0, &GadgetBle::_convertTemperatureV1}}, // unitEnc
                 {Unit::RH, {2, &GadgetBle::_convertHumidityV1}},
                 {Unit::CO2, {4, &GadgetBle::_convertSimple}}},
            };
            break;
        case T_RH_CO2_PM25:
            _sampleType = {
                DataType::T_RH_CO2_PM25, // datatype
                0,                       // advertisementType
                12,                      // advSampleType
                11,                      // dlSampleType
                8,                       // sampleSize
                2,                       // sampleCntPerPacket
                {{Unit::T, {0, &GadgetBle::_convertTemperatureV1}}, // unitEnc
                 {Unit::RH, {2, &GadgetBle::_convertHumidityV1}},
                 {Unit::CO2, {4, &GadgetBle::_convertSimple}},
                 {Unit::PM2P5, {6, &GadgetBle::_convertPM2p5V1}}},
            };
            break;

        case T_RH_HCHO:
            _sampleType = {
                DataType::T_RH_HCHO, // datatype
                0,                   // advertisementType
                14,                  // advSampleType
                13,                  // dlSampleType
                6,                   // sampleSize
                3,                   // sampleCntPerPacket
                {{Unit::T, {0, &GadgetBle::_convertTemperatureV1}}, // unitEnc
                 {Unit::RH, {2, &GadgetBle::_convertHumidityV1}},
                 {Unit::HCHO, {4, &GadgetBle::_convertHCHOV1}}},
            };
            break;
        case T_RH_VOC_PM25:
            _sampleType = {
                DataType::T_RH_VOC_PM25, // datatype
                0,                       // advertisementType
                16,                      // advSampleType
                15,                      // dlSampleType
                8,                       // sampleSize
                2,                       // sampleCntPerPacket
                {{Unit::T, {0, &GadgetBle::_convertTemperatureV1}}, // unitEnc
                 {Unit::RH, {2, &GadgetBle::_convertHumidityV1}},
                 {Unit::VOC, {4, &GadgetBle::_convertSimple}},
                 {Unit::PM2P5, {6, &GadgetBle::_convertPM2p5V1}}},
            };
            break;
        case T_RH_CO2_VOC_PM25_HCHO:
            _sampleType = {
                DataType::T_RH_CO2_VOC_PM25_HCHO, // datatype
                0,                                // advertisementType
                20,                               // advSampleType
                19,                               // dlSampleType
                12,                               // sampleSize
                1,                                // sampleCntPerPacket
                {{Unit::T, {0, &GadgetBle::_convertTemperatureV1}}, // unitEnc
                 {Unit::RH, {2, &GadgetBle::_convertHumidityV1}},
                 {Unit::CO2, {4, &GadgetBle::_convertSimple}},
                 {Unit::VOC, {6, &GadgetBle::_convertSimple}},
                 {Unit::PM2P5, {8, &GadgetBle::_convertPM2p5V1}},
                 {Unit::HCHO, {10, &GadgetBle::_convertHCHOV1}}},
            };
            break;
        case T_RH_VOC_NOX:
            _sampleType = {
                DataType::T_RH_VOC_NOX, // datatype
                0,                      // advertisementType
                22,                     // advSampleType
                21,                     // dlSampleType
                8,                      // sampleSize
                2,                      // sampleCntPerPacket
                {{Unit::T, {0, &GadgetBle::_convertTemperatureV1}}, // unitEnc
                 {Unit::RH, {2, &GadgetBle::_convertHumidityV1}},
                 {Unit::VOC, {4, &GadgetBle::_convertSimple}},
                 {Unit::NOX, {6, &GadgetBle::_convertSimple}}},
            };
            break;
        case T_RH_VOC_NOX_PM25:
            _sampleType = {
                DataType::T_RH_VOC_NOX_PM25, // datatype
                0,                           // advertisementType
                24,                          // advSampleType
                23,                          // dlSampleType
                10,                          // sampleSize
                2,                           // sampleCntPerPacket
                {{Unit::T, {0, &GadgetBle::_convertTemperatureV1}}, // unitEnc
                 {Unit::RH, {2, &GadgetBle::_convertHumidityV1}},
                 {Unit::VOC, {4, &GadgetBle::_convertSimple}},
                 {Unit::NOX, {6, &GadgetBle::_convertSimple}},
                 {Unit::PM2P5, {8, &GadgetBle::_convertPM2p5V2}}},
            };
            break;
        default:
            break;
    }

    _lastCacheTime = 0;

    _sampleBufferIdx = 0;
    _sampleBufferWraped = false;
    _sampleBufferSize = 0;
    _sampleBufferCapacity = _computeRealSampleBufferSize();

    if (_sampleCntChar != NULL) {
        _sampleCntChar->setValue(_sampleBufferSize);
    }

    _advertisedData[2] = _sampleType.advertisementType;
    _advertisedData[3] = _sampleType.advSampleType;
}

void GadgetBle::writeTemperature(float value) {
    _writeValue(value, Unit::T);
}

void GadgetBle::writeHumidity(float value) {
    _writeValue(value, Unit::RH);
}

void GadgetBle::writeCO2(float value) {
    _writeValue(value, Unit::CO2);
}

void GadgetBle::writeVOC(float value) {
    _writeValue(value, Unit::VOC);
}

void GadgetBle::writeNOx(float value) {
    _writeValue(value, Unit::NOX);
}

void GadgetBle::writePM2p5(float value) {
    _writeValue(value, Unit::PM2P5);
}

void GadgetBle::writeHCHO(float value) {
    _writeValue(value, Unit::HCHO);
}

void GadgetBle::commit() {
    if (esp_timer_get_time() - _lastCacheTime >= (_sampleIntervalMs * 1000)) {
        _lastCacheTime = esp_timer_get_time();
        _addCurrentSampleToHistory();
    }

    _bleAdvertising->stop();
    _updateAdvertising();
    _bleAdvertising->start();
}

void GadgetBle::handleEvents() {
    _updateConnectionState();
    _handleDownload();
}

// BLEServerCallbacks

void GadgetBle::onConnect(BLEServer* serverInst) {
    _deviceConnected = true;
};

void GadgetBle::onDisconnect(BLEServer* serverInst) {
    _deviceConnected = false;
}

// BLECharacteristicCallbacks

void GadgetBle::onWrite(BLECharacteristic* characteristic) {
    if (characteristic->getUUID().toString().compare(LOGGER_INTERVAL_UUID) ==
        0) {
        std::string value = characteristic->getValue();
        _sampleIntervalMs =
            value[0] + (value[1] << 8) + (value[2] << 16) + (value[3] << 24);
        _sampleBufferWraped = false;
        _sampleBufferIdx = 0;
        _sampleBufferSize = 0;
        _sampleCntChar->setValue(_sampleBufferSize);
    } else if (characteristic->getUUID().toString().compare(
                   WIFI_SSID_CHAR_UUID) == 0) {
        _wifiSsidSetting = characteristic->getValue();
    } else if (characteristic->getUUID().toString().compare(
                   WIFI_PWD_CHAR_UUID) == 0) {
        std::string wifiPwd = characteristic->getValue();
        if (_onWifiSettingsChanged != NULL) {
            _onWifiSettingsChanged(_wifiSsidSetting, wifiPwd);
        }
    }
}

// Internal Stuff

void GadgetBle::_bleInit() {
    BLEDevice::init(GADGET_NAME);

    // Get Mac address and compute deviceId
    std::string macAddress = BLEDevice::getAddress().toString();

    // - Mac address: Add device Id to BLE advertising frame
    _advertisedData[5] = strtol(macAddress.substr(15, 17).c_str(), NULL, 16);
    _advertisedData[4] = strtol(macAddress.substr(12, 14).c_str(), NULL, 16);

    // - Mac address: Keep deviceIdString for application
    char cDevId[6];
    snprintf(cDevId, sizeof(cDevId), "%s:%s", macAddress.substr(12, 14).c_str(),
             macAddress.substr(15, 17).c_str());
    _deviceIdString = cDevId;

    // Initialize BLEServer
    BLEServer* bleServer = BLEDevice::createServer();
    bleServer->setCallbacks(this);

    // - Create Download Service
    BLEService* bleDownloadService =
        bleServer->createService(DOWNLOAD_SERVICE_UUID);

    // - Download Service: Sample Count Characeristic
    _sampleCntChar = bleDownloadService->createCharacteristic(
        SAMPLE_CNT_CHAR_UUID, BLECharacteristic::PROPERTY_READ);
    _sampleCntChar->setValue(_sampleBufferSize);

    // - Download Service: Logging Interval Characteristic
    BLECharacteristic* loggerIntervalChar =
        bleDownloadService->createCharacteristic(
            LOGGER_INTERVAL_UUID, BLECharacteristic::PROPERTY_READ |
                                      BLECharacteristic::PROPERTY_WRITE);
    loggerIntervalChar->setValue(_sampleIntervalMs);
    loggerIntervalChar->setCallbacks(this);

    // - Download Service: Data Transfer Characteristic
    _transferChar = bleDownloadService->createCharacteristic(
        TRANSFER_NOTIFY_UUID, BLECharacteristic::PROPERTY_NOTIFY);
    _transferDescr = new BLE2902();
    _transferChar->addDescriptor(_transferDescr);

    // - Download Service: Start
    bleDownloadService->start();

    // - Create Gadget Settings Service
    BLEService* bleGadgetSettingsService =
        bleServer->createService(GADGET_SETTINGS_SERVICE_UUID);

    // - Gadget Settings Service: WiFi SSID Characteristic
    if (_onWifiSettingsChanged != NULL) {
        _wifiSsidChar = bleGadgetSettingsService->createCharacteristic(
            WIFI_SSID_CHAR_UUID, BLECharacteristic::PROPERTY_READ |
                                     BLECharacteristic::PROPERTY_WRITE);
        _wifiSsidChar->setValue(_wifiSsidSetting);
        _wifiSsidChar->setCallbacks(this);

        // - Gadget Settings Service: WiFi Password Characteristic
        BLECharacteristic* wifiPwdChar =
            bleGadgetSettingsService->createCharacteristic(
                WIFI_PWD_CHAR_UUID, BLECharacteristic::PROPERTY_WRITE);
        wifiPwdChar->setValue("n/a");
        wifiPwdChar->setCallbacks(this);
    }

    // - Gadget Settings Service: Start
    bleGadgetSettingsService->start();

    // Initialize BLE Advertising
    _bleAdvertising = BLEDevice::getAdvertising();
    _bleAdvertising->addServiceUUID(DOWNLOAD_SERVICE_UUID);
    _bleAdvertising->setScanResponse(true);
    // helps with iPhone connection issues
    _bleAdvertising->setMinPreferred(0x06);
    _bleAdvertising->setMaxPreferred(0x12);

    _updateAdvertising();

    // - BLE Advertising: Start
    _bleAdvertising->start();
}

void GadgetBle::_updateAdvertising() {
    // The API of scanResponse expects a string, so we need to convert our
    // binary advertising data to a string.
    std::string manufData(reinterpret_cast<char*>(_advertisedData.data()),
                          _advertisedData.size());

    BLEAdvertisementData scanResponse;
    scanResponse.setManufacturerData(manufData);
    _bleAdvertising->setScanResponseData(scanResponse);

    BLEAdvertisementData advert;
    advert.setName(GADGET_NAME);
    advert.setManufacturerData(manufData);
    _bleAdvertising->setAdvertisementData(advert);
}

void GadgetBle::_addCurrentSampleToHistory() {
    for (int i = 0; i < _sampleType.sampleSize; i++) {
        _sampleBuffer[_sampleBufferIdx++] = _currentSample[i];
    }

    if (_sampleBufferIdx + _sampleType.sampleSize - 1 >=
        _sampleBufferCapacity) {
        _sampleBufferIdx = 0;
        _sampleBufferWraped = true;
    }

    _sampleBufferSize = _computeBufferSize();
    _sampleCntChar->setValue(_sampleBufferSize);
}

// This requires proper adjustment as soon as we have more data types!
int GadgetBle::_getPositionInSample(Unit unit) {
    if (_sampleType.unitEnc.count(unit) == 0) {
        return INVALID_POSITION;
    }
    return _sampleType.unitEnc.at(unit).offset;
}

void GadgetBle::_writeValue(float value, Unit unit) {
    if (isnan(value)) {
        return;
    }

    if (_sampleType.unitEnc.count(unit) == 0) {
        return;
    }
    t_converter convert = _sampleType.unitEnc.at(unit).converterFct;
    uint16_t convertedValue = ((this)->*convert)(value);

    int position = _getPositionInSample(unit);
    if (position == INVALID_POSITION) {
        return;
    }

    _advertisedData[position + ADV_SAMPLE_OFFSET] =
        static_cast<uint8_t>(convertedValue);
    _advertisedData[position + ADV_SAMPLE_OFFSET + 1] =
        static_cast<uint8_t>(convertedValue >> 8);

    // update current sample cache
    _currentSample[position] = static_cast<uint8_t>(convertedValue);
    _currentSample[position + 1] = static_cast<uint8_t>(convertedValue >> 8);
}

uint16_t GadgetBle::_convertSimple(float value) {
    return static_cast<uint16_t>(value + 0.5f);
}

uint16_t GadgetBle::_convertTemperatureV1(float value) {
    return static_cast<uint16_t>((((value + 45) / 175) * 65535) + 0.5f);
}

uint16_t GadgetBle::_convertHumidityV1(float value) {
    return static_cast<uint16_t>(((value / 100) * 65535) + 0.5f);
}

uint16_t GadgetBle::_convertHumidityV2(float value) {
    return static_cast<uint16_t>((((value + 6.0) * 65535) / 125.0) + 0.5f);
}

uint16_t GadgetBle::_convertPM2p5V1(float value) {
    return static_cast<uint16_t>(((value / 1000) * 65535) + 0.5f);
}

uint16_t GadgetBle::_convertPM2p5V2(float value) {
    return static_cast<uint16_t>((value * 10) + 0.5f);
}

uint16_t GadgetBle::_convertHCHOV1(float value) {
    return static_cast<uint16_t>((value * 5) + 0.5f);
}

// Download Logger Related

void GadgetBle::_updateConnectionState() {
    // connecting
    if (_deviceConnected && !_oldDeviceConnected) {
        _downloadSeqNumber = 0;
        _oldDeviceConnected = _deviceConnected;
    }

    // disconnecting
    if (!_deviceConnected && _oldDeviceConnected) {
        _transferDescr->setNotifications(false);
        _downloadSeqNumber = 0;
        _oldDeviceConnected = _deviceConnected;
    }
}

uint16_t GadgetBle::_computeBufferSize() {
    return static_cast<uint16_t>(
        ((_sampleBufferWraped) ? static_cast<double>(_sampleBufferCapacity)
                               : static_cast<double>(_sampleBufferIdx)) /
        _sampleType.sampleSize);
}

bool GadgetBle::_handleDownload() {
    uint16_t sampleCnt = _computeBufferSize();

    // Download Finished
    if (_downloading && _downloadSeqNumber >= sampleCnt) {
        _downloading = false;
        _transferDescr->setNotifications(false);
        _downloadSeqNumber = 0;
        return false;
    }

    if (_transferDescr->getNotifications()) {
        _downloading = true;
        if (_downloadSeqNumber == 0) {
            // send header
            sampleCnt -= 1;

            uint32_t ageLastSampleMs = static_cast<uint32_t>(
                ((esp_timer_get_time() - _lastCacheTime) / 1000) + 0.5f);

            _downloadHeader[4] = _sampleType.dlSampleType;
            _downloadHeader[5] = _sampleType.dlSampleType >> 8;
            _downloadHeader[6] = _sampleIntervalMs;
            _downloadHeader[7] = _sampleIntervalMs >> 8;
            _downloadHeader[8] = _sampleIntervalMs >> 16;
            _downloadHeader[9] = _sampleIntervalMs >> 24;
            _downloadHeader[10] = ageLastSampleMs;
            _downloadHeader[11] = ageLastSampleMs >> 8;
            _downloadHeader[12] = ageLastSampleMs >> 16;
            _downloadHeader[13] = ageLastSampleMs >> 24;
            _downloadHeader[14] = sampleCnt;
            _downloadHeader[15] = (sampleCnt >> 8);
            _transferChar->setValue(_downloadHeader.data(),
                                    _downloadHeader.size());
            _downloadSeqNumber++;
            _transferChar->notify();
        } else {
            std::array<uint8_t, DOWNLOAD_PKT_SIZE> valueBuffer = {};
            valueBuffer[0] = _downloadSeqNumber;
            valueBuffer[1] = (_downloadSeqNumber >> 8);
            for (int j = 0; j < _sampleType.sampleCntPerPacket; j++) {
                for (int i = 0; i < _sampleType.sampleSize; i++) {
                    uint32_t idx = ((_downloadSeqNumber - 1) *
                                    (_sampleType.sampleSize *
                                     _sampleType.sampleCntPerPacket)) +
                                   i + (j * _sampleType.sampleSize);
                    if (_sampleBufferWraped) {
                        idx = (_sampleBufferIdx + idx) % _sampleBufferCapacity;
                    }
                    valueBuffer[i + 2 + (j * _sampleType.sampleSize)] =
                        _sampleBuffer[idx];
                }
            }

            _transferChar->setValue(valueBuffer.data(), valueBuffer.size());
            _downloadSeqNumber++;
            _transferChar->notify();
        }
        return true;
    }

    return false;
}

uint16_t GadgetBle::_computeRealSampleBufferSize() {
    return static_cast<uint16_t>(
               std::floor(SAMPLE_BUFFER_SIZE_BYTES / _sampleType.sampleSize)) *
           _sampleType.sampleSize;
}
