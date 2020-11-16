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

    switch (dataType) {
        case T_RH_V3:
            _sampleSize = 4;
            _sampleCntPerPacket = 4;
            _sampleTypeDL = 0;
            _sampleTypeAdv = 4;
            break;
        case T_RH_V4:
            _sampleSize = 4;
            _sampleCntPerPacket = 4;
            _sampleTypeDL = 5;
            _sampleTypeAdv = 6;
            break;
        case T_RH_CO2:
            _sampleSize = 6;
            _sampleCntPerPacket = 3;
            _sampleTypeDL = 9;
            _sampleTypeAdv = 10;
            break;
        case T_RH_CO2_ALT:
            _sampleSize = 8;
            _sampleCntPerPacket = 2;
            _sampleTypeDL = 7;
            _sampleTypeAdv = 8;
            break;
        case T_RH_CO2_PM25:
            _sampleSize = 8;
            _sampleCntPerPacket = 2;
            _sampleTypeDL = 11;
            _sampleTypeAdv = 12;
            break;
        default:
            break;
    }

    _advSampleType = 0;
    _lastCacheTime = 0;
    _deviceIdString = "n/a";

    _sampleBufferSize = 0;
    _sampleBufferCapcity = _computeRealSampleBufferSize();

    _advertisedData[2] = _advSampleType;
    _advertisedData[3] = _sampleTypeAdv;
}

void GadgetBle::begin() {
    _bleInit();
}

void GadgetBle::writeTemperature(float value) {
    if (isnan(value)) {
        return;
    }

    int converted = (int)std::round(((value + 45) / 175) * 65535);

    _writeValue(converted, Unit::T);
}

void GadgetBle::writeHumidity(float value) {
    if (isnan(value)) {
        return;
    }

    int converted = (int)std::round((value / 100) * 65535);
    // special conversion for SHT4x RH samples
    if (_dataType == DataType::T_RH_V4) {
        converted = (int)std::round(((value + 6.0) * 65535) / 125.0);
    }

    _writeValue(converted, Unit::RH);
}

void GadgetBle::writeCO2(float value) {
    if (isnan(value)) {
        return;
    }

    int converted = (uint16_t)std::round(value);

    _writeValue(converted, Unit::CO2);
}

void GadgetBle::writePM2p5(float value) {
    if (isnan(value)) {
        return;
    }

    int converted = (int)std::round((value / 1000) * 65535);

    _writeValue(converted, Unit::PM2P5);
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
    std::string value = characteristic->getValue();
    _sampleIntervalMs =
        value[0] + (value[1] << 8) + (value[2] << 16) + (value[3] << 24);
    _sampleBufferWraped = false;
    _sampleBufferIdx = 0;
    _sampleBufferSize = 0;
    _sampleCntChar->setValue(_sampleBufferSize);
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
    std::string manufData((char*)(_advertisedData.data()),
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
    for (int i = 0; i < _sampleSize; i++) {
        _sampleBuffer[_sampleBufferIdx++] = _currentSample[i];
    }

    if (_sampleBufferIdx + _sampleSize - 1 >= _sampleBufferCapcity) {
        _sampleBufferIdx = 0;
        _sampleBufferWraped = true;
    }

    _sampleBufferSize = _computeBufferSize();
    _sampleCntChar->setValue(_sampleBufferSize);
}

// This requires proper adjustment as soon as we have more data types!
int GadgetBle::_getPositionInSample(Unit unit) {
    switch (unit) {
        case T:
            return 0;
        case RH:
            return 2;
        case CO2:
            return 4;
        case PM2P5:
            return 6;
    }
    return INVALID_POSITION;
}

void GadgetBle::_writeValue(int convertedValue, Unit unit) {
    int position = _getPositionInSample(unit);
    if (position == INVALID_POSITION) {
        return;
    }

    _advertisedData[position + ADV_SAMPLE_OFFSET] = (uint8_t)convertedValue;
    _advertisedData[position + ADV_SAMPLE_OFFSET + 1] =
        (uint8_t)(convertedValue >> 8);

    // update current sample cache
    _currentSample[position] = (uint8_t)convertedValue;
    _currentSample[position + 1] = (uint8_t)(convertedValue >> 8);
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
    return (uint16_t)(((_sampleBufferWraped) ? (double)_sampleBufferCapcity
                                             : (double)_sampleBufferIdx) /
                      _sampleSize);
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

            uint32_t ageLastSampleMs = (uint32_t)std::round(
                (esp_timer_get_time() - _lastCacheTime) / 1000);

            _downloadHeader[4] = _sampleTypeDL;
            _downloadHeader[5] = _sampleTypeDL >> 8;
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
            for (int j = 0; j < _sampleCntPerPacket; j++) {
                for (int i = 0; i < _sampleSize; i++) {
                    uint32_t idx = ((_downloadSeqNumber - 1) *
                                    (_sampleSize * _sampleCntPerPacket)) +
                                   i + (j * _sampleSize);
                    if (_sampleBufferWraped) {
                        idx = (_sampleBufferIdx + idx) % _sampleBufferCapcity;
                    }
                    valueBuffer[i + 2 + (j * _sampleSize)] = _sampleBuffer[idx];
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

int GadgetBle::_computeRealSampleBufferSize() {
    return (int)std::floor(SAMPLE_BUFFER_SIZE_BYTES / _sampleSize) *
           _sampleSize;
}
