#include "DataProvider.h"
#include <math.h>

void DataProvider::begin() {
    _BLELibrary.init();
    _BLELibrary.setProviderCallbacks(this);
    _BLELibrary.characteristicSetValue(SAMPLE_HISTORY_INTERVAL_UUID,
                                       _historyIntervalMilliSeconds);
    _BLELibrary.characteristicSetValue(
        NUMBER_OF_SAMPLES_UUID, _sampleHistory.numberOfSamplesInHistory());

    _sampleHistory.setSampleSize(_sampleConfig.sampleSizeBytes);

    // Fill advertisement header
    _advertisementHeader.writeCompanyId(0x06D5);
    _advertisementHeader.writeSensirionAdvertisementType(0x00);

    // Use part of MAC address as device id
    std::string macAddress = _BLELibrary.getDeviceAddress();
    _advertisementHeader.writeDeviceId(
        strtol(macAddress.substr(12, 17).c_str(), NULL, 16));

    _BLELibrary.setAdvertisingData(_buildAdvertisementData());
    _BLELibrary.startAdvertising();
}

void DataProvider::writeValueToCurrentSample(float value, Unit unit) {
    // Check for valid value
    if (isnan(value)) {
        return;
    }

    // Check for correct unit
    if (_sampleConfig.sampleSlots.count(unit) ==
        0) { // implies unit is not part of sample
        return;
    }

    // Get relevant metaData
    uint16_t (*converterFunction)(float value) =
        _sampleConfig.sampleSlots.at(unit).converterFunction;
    size_t offset = _sampleConfig.sampleSlots.at(unit).offset;

    // Convert float to 16 bit int
    uint16_t convertedValue = converterFunction(value);
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

void DataProvider::handleDownload() {
    if (_downloadState == INACTIVE) {
        return;
    }

    // Download Completed
    if (_downloadState == COMPLETED) {
        _downloadSequenceIdx = 0;
        _numberOfSamplesToDownload = 0;
        _numberOfSamplePacketsToDownload = 0;
        _downloadState = INACTIVE;
        return;
    }

    // Start Download
    if (_downloadState == START) {
        _numberOfSamplesToDownload = _sampleHistory.numberOfSamplesInHistory();

        _numberOfSamplePacketsToDownload =
            _numberOfPacketsRequired(_numberOfSamplesToDownload);
        _BLELibrary.characteristicSetValue(NUMBER_OF_SAMPLES_UUID,
                                           _numberOfSamplesToDownload);
        DownloadHeader header = _buildDownloadHeader();
        _BLELibrary.characteristicSetValue(DOWNLOAD_PACKET_UUID,
                                           header.getDataArray().data(),
                                           header.getDataArray().size());
        _downloadState = DOWNLOADING;
        _sampleHistory.startReadOut();

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

std::string DataProvider::_buildAdvertisementData() {
    _advertisementHeader.writeSampleType(_sampleConfig.sampleType);
    std::string data = _advertisementHeader.getDataString();
    data.append(_currentSample.getDataString());
    return data;
}

DownloadHeader DataProvider::_buildDownloadHeader() {
    DownloadHeader header;
    uint32_t age = static_cast<uint32_t>(
        ((millis() - _latestHistoryTimeStampAtDownloadStart) / 1000));
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
