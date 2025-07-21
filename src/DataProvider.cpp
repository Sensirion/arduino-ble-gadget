#include "DataProvider.h"
#include <math.h>

void DataProvider::begin() {
    _setupBLEInfrastructure();

    _sampleHistory.setSampleSize(_sampleConfig.sampleSizeBytes);

    // Fill advertisement header
    _advertisementHeader.writeCompanyId(0x06D5);
    _advertisementHeader.writeSensirionAdvertisementType(0x00);

    // Use part of MAC address as device id
    std::string macAddress = _BLELibrary.getDeviceAddress();
    _advertisementHeader.writeDeviceId(
        static_cast<uint8_t>(
            strtol(macAddress.substr(12, 14).c_str(), NULL, 16)),
        static_cast<uint8_t>(
            strtol(macAddress.substr(15, 17).c_str(), NULL, 16)));

    _BLELibrary.setAdvertisingData(_buildAdvertisementData());
    _BLELibrary.startAdvertising();
}

void DataProvider::writeValueToCurrentSample(float value,
                                             SignalType signalType) {
    // Check for valid value
    if (isnan(value)) {
        return;
    }

    // Check for correct signal type
    if (_sampleConfig.sampleSlots.count(signalType) == 0) {
        // implies signal type is not part of sample
        return;
    }

    // Get relevant metaData
    uint16_t (*encodingFunction)(float value) =
        _sampleConfig.sampleSlots.at(signalType).encodingFunction;
    size_t offset = _sampleConfig.sampleSlots.at(signalType).offset;

    // Convert float to 16 bit int
    uint16_t convertedValue = encodingFunction(value);
    _currentSample.writeValue(convertedValue, offset);
}

void DataProvider::commitSample() {
    uint64_t currentTimeStamp = millis();
    if ((currentTimeStamp - _latestHistoryTimeStamp) >=
        _historyIntervalMilliSeconds) {
        _sampleHistory.putSample(_currentSample);
        _latestHistoryTimeStamp = currentTimeStamp;

        if (_downloadState == INACTIVE) {
            _latestHistoryTimeStampAtDownloadStart = currentTimeStamp;
            _BLELibrary.characteristicSetValue(
                NUMBER_OF_SAMPLES_UUID,
                _sampleHistory.numberOfSamplesInHistory());
        }
    }

    // Update Advertising
    _BLELibrary.stopAdvertising();
    _BLELibrary.setAdvertisingData(_buildAdvertisementData());
    _BLELibrary.startAdvertising();
}

void DataProvider::setBatteryLevel(int value) {
    _BLELibrary.characteristicSetValue(BATTERY_LEVEL_UUID, value);
}

void DataProvider::handleDownload() {
    if (_downloadState == INACTIVE) {
        return;
    }

    // Download Completed
    if (_downloadState == COMPLETED) {
        _downloadSequenceIdx = 0;
        _nrOfSamplesRequested = 0;
        _numberOfSamplesToDownload = 0;
        _numberOfSamplePacketsToDownload = 0;
        _downloadState = INACTIVE;
        return;
    }

    // Start Download
    if (_downloadState == START) {
        if (_nrOfSamplesRequested > 0 &&
            _nrOfSamplesRequested < _sampleHistory.numberOfSamplesInHistory()) {
            _numberOfSamplesToDownload = _nrOfSamplesRequested;
        } else {
            _numberOfSamplesToDownload =
                _sampleHistory.numberOfSamplesInHistory();
        }
        _numberOfSamplePacketsToDownload =
            _numberOfPacketsRequired(_numberOfSamplesToDownload);
        DownloadHeader header = _buildDownloadHeader();
        _BLELibrary.characteristicSetValue(DOWNLOAD_PACKET_UUID,
                                           header.getDataArray().data(),
                                           header.getDataArray().size());
        _downloadState = DOWNLOADING;
        _sampleHistory.startReadOut(_numberOfSamplesToDownload);

    } else if (_downloadState == DOWNLOADING) { // Continue Download
        DownloadPacket packet = _buildDownloadPacket();
        _BLELibrary.characteristicSetValue(DOWNLOAD_PACKET_UUID,
                                           packet.getDataArray().data(),
                                           packet.getDataArray().size());
    }

    _BLELibrary.characteristicNotify(DOWNLOAD_PACKET_UUID);

    ++_downloadSequenceIdx;
    if (_downloadSequenceIdx >= _numberOfSamplePacketsToDownload + 1) {
        _downloadState = COMPLETED;
    }
}

void DataProvider::setSampleConfig(DataType dataType) {
    _sampleConfig = sampleConfigSelector.at(dataType);
    _sampleHistory.setSampleSize(_sampleConfig.sampleSizeBytes);
}

String DataProvider::getDeviceIdString() const {
    char cDevId[6];
    std::string macAddress = _BLELibrary.getDeviceAddress();
    snprintf(cDevId, sizeof(cDevId), "%s:%s", macAddress.substr(12, 14).c_str(),
             macAddress.substr(15, 17).c_str());
    return cDevId;
}

bool DataProvider::isDownloading() const {
    return (_downloadState != DownloadState::INACTIVE);
}

void DataProvider::enableAltDeviceName() {
    _enableAltDeviceName = true;
}

std::string DataProvider::getAltDeviceName() {
    return _altDeviceName;
}

void DataProvider::setAltDeviceName(std::string altDeviceName) {
    _altDeviceName = altDeviceName;
    _BLELibrary.characteristicSetValue(
        ALT_DEVICE_NAME_UUID,
        reinterpret_cast<const uint8_t*>(_altDeviceName.c_str()),
        _altDeviceName.length());
}

void DataProvider::onAltDeviceNameChange(std::string altDeviceName) {
    setAltDeviceName(altDeviceName);
}

std::string DataProvider::_buildAdvertisementData() {
    _advertisementHeader.writeSampleType(_sampleConfig.sampleType);
    std::string data = _advertisementHeader.getDataString();
    data.append(_currentSample.getDataString());
    return data;
}

DownloadHeader DataProvider::_buildDownloadHeader() {
    DownloadHeader header;
    uint32_t age = static_cast<uint32_t>(
        millis() - _latestHistoryTimeStampAtDownloadStart);
    header.setDownloadSampleType(_sampleConfig.downloadType);
    header.setIntervalMilliSeconds(_historyIntervalMilliSeconds);
    header.setAgeOfLatestSampleMilliSeconds(age);
    header.setDownloadSampleCount(
        static_cast<uint16_t>(_numberOfSamplesToDownload));
    return header;
}

DownloadPacket DataProvider::_buildDownloadPacket() {
    DownloadPacket packet;
    packet.setDownloadSequenceNumber(_downloadSequenceIdx);
    bool allSamplesRead = false;
    for (int i = 0; i < _sampleConfig.sampleCountPerPacket && !allSamplesRead;
         ++i) {
        Sample sample = _sampleHistory.readOutNextSample(allSamplesRead);
        packet.writeSample(sample, _sampleConfig.sampleSizeBytes, i);
    }
    return packet;
}

int DataProvider::_numberOfPacketsRequired(int numberOfSamples) const {
    int numberOfPacketsRequired =
        numberOfSamples / _sampleConfig.sampleCountPerPacket;
    if ((numberOfSamples % _sampleConfig.sampleCountPerPacket) != 0) {
        ++numberOfPacketsRequired;
    }
    return numberOfPacketsRequired;
}

void DataProvider::_setupBLEInfrastructure() {
    _BLELibrary.init();
    _BLELibrary.createServer();

    _setupBLEServices();

    _BLELibrary.setProviderCallbacks(this);
    _BLELibrary.characteristicSetValue(SAMPLE_HISTORY_INTERVAL_UUID,
                                       _historyIntervalMilliSeconds);
    _BLELibrary.characteristicSetValue(
        NUMBER_OF_SAMPLES_UUID, _sampleHistory.numberOfSamplesInHistory());
}

void DataProvider::_setupBLEServices() {
    // Download Service
    _BLELibrary.createService(DOWNLOAD_SERVICE_UUID);
    _BLELibrary.createCharacteristic(DOWNLOAD_SERVICE_UUID,
                                     NUMBER_OF_SAMPLES_UUID,
                                     Permission::READ_PERMISSION);
    _BLELibrary.characteristicSetValue(NUMBER_OF_SAMPLES_UUID, 0);
    _BLELibrary.createCharacteristic(DOWNLOAD_SERVICE_UUID,
                                     REQUESTED_SAMPLES_UUID,
                                     Permission::WRITE_PERMISSION);
    _BLELibrary.createCharacteristic(DOWNLOAD_SERVICE_UUID,
                                     SAMPLE_HISTORY_INTERVAL_UUID,
                                     Permission::READWRITE_PERMISSION);
    _BLELibrary.createCharacteristic(DOWNLOAD_SERVICE_UUID,
                                     DOWNLOAD_PACKET_UUID,
                                     Permission::NOTIFY_PERMISSION);
    _BLELibrary.startService(DOWNLOAD_SERVICE_UUID);

    // Settings Service
    _BLELibrary.createService(SETTINGS_SERVICE_UUID);
    if (_enableWifiSettings) {
        _BLELibrary.createCharacteristic(SETTINGS_SERVICE_UUID, WIFI_SSID_UUID,
                                         Permission::READWRITE_PERMISSION);
        const char* ssid = "ssid";
        _BLELibrary.characteristicSetValue(
            WIFI_SSID_UUID, reinterpret_cast<const uint8_t*>(ssid),
            strlen(ssid));
        _BLELibrary.createCharacteristic(SETTINGS_SERVICE_UUID, WIFI_PWD_UUID,
                                         Permission::WRITE_PERMISSION);
        const char* pwd = "password";
        _BLELibrary.characteristicSetValue(
            WIFI_PWD_UUID, reinterpret_cast<const uint8_t*>(pwd), strlen(pwd));
    }
    if (_enableAltDeviceName) {
        _BLELibrary.createCharacteristic(SETTINGS_SERVICE_UUID,
                                         ALT_DEVICE_NAME_UUID,
                                         Permission::READWRITE_PERMISSION);
        setAltDeviceName(_altDeviceName);
    }
    _BLELibrary.startService(SETTINGS_SERVICE_UUID);

    // Battery Service
    if (_enableBatteryService) {
        _BLELibrary.createService(BATTERY_SERVICE_UUID);
        _BLELibrary.createCharacteristic(BATTERY_SERVICE_UUID,
                                         BATTERY_LEVEL_UUID,
                                         Permission::READ_PERMISSION);
        _BLELibrary.characteristicSetValue(BATTERY_LEVEL_UUID, 0);
        _BLELibrary.startService(BATTERY_SERVICE_UUID);
    }
    // SCD FRC Service
    if (_enableFRCService) {
        _BLELibrary.createService(SCD_SERVICE_UUID);
        _BLELibrary.createCharacteristic(SCD_SERVICE_UUID, SCD_FRC_REQUEST_UUID,
                                         Permission::WRITE_PERMISSION);
        _BLELibrary.characteristicSetValue(SCD_FRC_REQUEST_UUID, 0);
        _BLELibrary.startService(SCD_SERVICE_UUID);
    }
}

void DataProvider::onHistoryIntervalChange(int interval) {
    _historyIntervalMilliSeconds = static_cast<uint64_t>(interval);
    _sampleHistory.reset();
    _BLELibrary.characteristicSetValue(
        NUMBER_OF_SAMPLES_UUID, _sampleHistory.numberOfSamplesInHistory());
}

void DataProvider::onConnectionEvent() {
    _downloadSequenceIdx = 0;
    _downloadState = INACTIVE;
}

void DataProvider::onDownloadRequest() {
    _downloadState = START;
}

void DataProvider::onFRCRequest(uint16_t reference_co2_level) {
    _frc_requested = true;
    _reference_co2_level = reference_co2_level;
}

void DataProvider::onNrOfSamplesRequest(int nr_of_samples) {
    _nrOfSamplesRequested = nr_of_samples;
}

void DataProvider::completeFRCRequest() {
    _frc_requested = false;
    _reference_co2_level = 0;
}

bool DataProvider::isFRCRequested() const {
    return _frc_requested;
}

uint32_t DataProvider::getReferenceCO2Level() const {
    return _reference_co2_level;
}
