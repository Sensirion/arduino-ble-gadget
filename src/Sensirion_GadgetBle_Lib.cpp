/*
 GadgetBle.h - Library for providing sensor data via Bluetooth.
 Created by Bjoern Muntwyler, November 7, 2020.
 Released under BSD-3 licence
 */

// TODO: Still need to properly handle packet size and sample type/adv. sample
// type/logger sample type in respective packets

#include "Sensirion_GadgetBle_Lib.h"
#include "Arduino.h"

#define INVALID_POSITION -1

static const int ADV_SAMPLE_OFFSET = 6;

static int64_t lastCacheTime = 0;

static std::array<uint8_t, MAX_SAMPLE_SIZE> currentSample = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
static std::array<uint8_t, SAMPLE_BUFFER_SIZE_BYTES> sampleBuffer;
static uint32_t sampleBufferIdx = 0;
static uint16_t sampleBufferSize = 0;
static bool sampleBufferWraped = false;
static uint16_t downloadSeqNumber = 0;
static uint32_t sampleIntervalMs = 600000; // 10 minutes
static bool downloading = false;

// Download Header template
// Byte 0: 2 bytes sequcnce number
// Byte 2: 1 byte version number
// Byte 3: 1 byte protocol identifier
// Byte 4: 2 bytes sample type
// Byte 6: 4 bytes sampling interval in ms
// Byte 10: 4 bytes age lastest sample in ms
// Byte 14: 2 bytes sample count
// Byte 16: 4 bytes unused
static std::array<uint8_t, DOWNLOAD_PKT_SIZE> downloadHeader = {
    0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

// Advertisement Data
// Note, that the GADGET_NAME will be also attached by the BLE library, so it
// can not be too long!
// Byte 0: 2 bytes for BLE company identifier
// Byte 2: advertising type
// Byte 3: sample type
// Byte 4: device identifier
// Byte 6: 2 bytes for sample value
// Byte 8: 2 bytes for sample value
// Byte 10: 2 bytes for sample value
// Byte 12: 2 bytes for sample value
static std::array<uint8_t, 14> advertisedData = {
    0xD5, 0x06, 0x00, 0x00, 0xFF, 0xFF, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static bool deviceConnected = false;
static bool oldDeviceConnected = false;

class ConnectionStateTracker: public BLEServerCallbacks {
    void onConnect(BLEServer* serverInst) {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer* serverInst) {
        deviceConnected = false;
    }
};

class LogIntervalCallback: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* characteristic) {
        std::string value = characteristic->getValue();
        sampleIntervalMs =
            value[0] + (value[1] << 8) + (value[2] << 16) + (value[3] << 24);
        sampleBufferWraped = false;
        sampleBufferIdx = 0;
    }
};

GadgetBle::GadgetBle(DataType dataType) {
    lastCacheTime = 0;

    _deviceIdString = "n/a";
    _advSampleType = 0;

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
    _sampleBufferSize = _computeRealSampleBufferSize();

    advertisedData[2] = _advSampleType;
    advertisedData[3] = _sampleTypeAdv;
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

void GadgetBle::writeCO2(uint16_t value) {
    if (isnan(value)) {
        return;
    }

    _writeValue(value, Unit::CO2);
}

void GadgetBle::writePM2p5(float value) {
    if (isnan(value)) {
        return;
    }

    int converted = (int)std::round((value / 1000) * 65535);

    _writeValue(converted, Unit::PM2P5);
}

void GadgetBle::commit() {
    if (esp_timer_get_time() - lastCacheTime >= (sampleIntervalMs * 1000)) {
        lastCacheTime = esp_timer_get_time();
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

// Internal Stuff

void GadgetBle::_bleInit() {
    BLEDevice::init(GADGET_NAME);

    // Get Mac address and compute deviceId
    std::string macAddress = BLEDevice::getAddress().toString();

    // - Mac address: Add device Id to BLE advertising frame
    advertisedData[5] = strtol(macAddress.substr(15, 17).c_str(), NULL, 16);
    advertisedData[4] = strtol(macAddress.substr(12, 14).c_str(), NULL, 16);

    // - Mac address: Keep deviceIdString for application
    char cDevId[6];
    snprintf(cDevId, sizeof(cDevId), "%s:%s", macAddress.substr(12, 14).c_str(),
             macAddress.substr(15, 17).c_str());
    _deviceIdString = cDevId;

    // Initialize BLEServer
    BLEServer* bleServer = BLEDevice::createServer();
    bleServer->setCallbacks(new ConnectionStateTracker());

    // - Create Download Service
    BLEService* bleDownloadService =
        bleServer->createService(DOWNLOAD_SERVICE_UUID);

    // - Download Service: Sample Count Characeristic
    _sampleCntChar = bleDownloadService->createCharacteristic(
        SAMPLE_CNT_CHAR_UUID, BLECharacteristic::PROPERTY_READ);
    _sampleCntChar->setValue(sampleBufferSize);

    // - Download Service: Logging Interval Characteristic
    BLECharacteristic* loggerIntervalChar =
        bleDownloadService->createCharacteristic(
            LOGGER_INTERVAL_UUID, BLECharacteristic::PROPERTY_READ |
                                      BLECharacteristic::PROPERTY_WRITE);
    loggerIntervalChar->setValue(sampleIntervalMs);
    loggerIntervalChar->setCallbacks(new LogIntervalCallback());

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
    _bleAdvertising->setMinPreferred(
        0x06); // helps with iPhone connection issues
    _bleAdvertising->setMinPreferred(0x12);

    _updateAdvertising();

    // - BLE Advertising: Start
    _bleAdvertising->start();
}

void GadgetBle::_updateAdvertising() {
    std::string manufData((char*)(advertisedData.data()),
                          advertisedData.size());

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
        sampleBuffer[sampleBufferIdx++] = currentSample[i];
    }

    if (sampleBufferIdx + _sampleSize - 1 >= _sampleBufferSize) {
        sampleBufferIdx = 0;
        sampleBufferWraped = true;
    }

    sampleBufferSize = _computeBufferSize();
    _sampleCntChar->setValue(sampleBufferSize);
}

// This requires proper adjustment as soon as we have more data types!
int GadgetBle::_getPositionInSample(Unit unit) {
    switch (unit) {
        case T:
            return 0;
            break;
        case RH:
            return 2;
            break;
        case CO2:
            return 4;
            break;
        case PM2P5:
            return 6;
            break;
        default:
            return INVALID_POSITION;
            break;
    }
}

void GadgetBle::_writeValue(int convertedValue, Unit unit) {
    int position = _getPositionInSample(unit);
    if (position == INVALID_POSITION) {
        return;
    }

    advertisedData[position + ADV_SAMPLE_OFFSET] = (uint8_t)convertedValue;
    advertisedData[position + ADV_SAMPLE_OFFSET + 1] =
        (uint8_t)(convertedValue >> 8);

    // update current sample cache
    currentSample[position] = (uint8_t)convertedValue;
    currentSample[position + 1] = (uint8_t)(convertedValue >> 8);
}

// Download Logger Related

void GadgetBle::_updateConnectionState() {
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        downloadSeqNumber = 0;
        oldDeviceConnected = deviceConnected;
    }

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        _transferDescr->setNotifications(false);
        downloadSeqNumber = 0;
        oldDeviceConnected = deviceConnected;
    }
}

uint16_t GadgetBle::_computeBufferSize() {
    return (uint16_t)(((sampleBufferWraped) ? (double)_sampleBufferSize
                                            : (double)sampleBufferIdx) /
                      _sampleSize);
}

bool GadgetBle::_handleDownload() {
    uint16_t sampleCnt = _computeBufferSize();

    // Download Finished
    if (downloading && downloadSeqNumber >= sampleCnt) {
        downloading = false;
        _transferDescr->setNotifications(false);
        downloadSeqNumber = 0;
        return false;
    }

    if (_transferDescr->getNotifications()) {
        downloading = true;
        if (downloadSeqNumber == 0) {
            // send header
            sampleCnt -= 1;

            uint32_t ageLastSampleMs = (uint32_t)std::round(
                (esp_timer_get_time() - lastCacheTime) / 1000);

            downloadHeader[4] = _sampleTypeDL;
            downloadHeader[5] = _sampleTypeDL >> 8;
            downloadHeader[6] = sampleIntervalMs;
            downloadHeader[7] = sampleIntervalMs >> 8;
            downloadHeader[8] = sampleIntervalMs >> 16;
            downloadHeader[9] = sampleIntervalMs >> 24;
            downloadHeader[10] = ageLastSampleMs;
            downloadHeader[11] = ageLastSampleMs >> 8;
            downloadHeader[12] = ageLastSampleMs >> 16;
            downloadHeader[13] = ageLastSampleMs >> 24;
            downloadHeader[14] = sampleCnt;
            downloadHeader[15] = (sampleCnt >> 8);
            _transferChar->setValue(downloadHeader.data(),
                                    downloadHeader.size());
            downloadSeqNumber++;
            _transferChar->notify();
        } else {
            uint8_t valueBuffer[DOWNLOAD_PKT_SIZE];
            valueBuffer[0] = downloadSeqNumber;
            valueBuffer[1] = (downloadSeqNumber >> 8);
            for (int j = 0; j < _sampleCntPerPacket; j++) {
                for (int i = 0; i < _sampleSize; i++) {
                    uint32_t idx = ((downloadSeqNumber - 1) *
                                    (_sampleSize * _sampleCntPerPacket)) +
                                   i + (j * _sampleSize);
                    if (sampleBufferWraped) {
                        idx = (sampleBufferIdx + idx) % _sampleBufferSize;
                    }
                    valueBuffer[i + 2 + (j * _sampleSize)] = sampleBuffer[idx];
                }
            }

            _transferChar->setValue((uint8_t*)&valueBuffer, DOWNLOAD_PKT_SIZE);
            downloadSeqNumber++;
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
