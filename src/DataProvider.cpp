#include "DataProvider.h"
#include <math.h>

void DataProvider::begin() {
    _BLELibrary.init();
    _BLELibrary.setProviderCallbacks(this);
    _BLELibrary.characteristicSetValue(SAMPLE_HISTORY_INTERVAL_UUID,
                                       _historyIntervalMilliSeconds);
    _BLELibrary.characteristicSetValue(
        NUMBER_OF_SAMPLES_UUID, _sampleHistory.numberOfSamplesInBuffer());

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
    if ((currentTimeStamp - _sampleHistory.latestHistoryTimeStamp) >=
        _historyIntervalMilliSeconds) {
        _sampleHistory.addSample(_currentSample);
        _BLELibrary.characteristicSetValue(
            NUMBER_OF_SAMPLES_UUID, _sampleHistory.numberOfSamplesInBuffer());
        _sampleHistory.latestHistoryTimeStamp = currentTimeStamp;
    }

    // Update Advertising
    _BLELibrary.stopAdvertising();
    _BLELibrary.setAdvertisingData(_buildAdvertisementData());
    _BLELibrary.startAdvertising();
}

void DataProvider::handleDownload() {
    if (_isDownloading == false) {
        return;
    }

    // Download Completed
    if (_downloadSequenceIdx >=
        _numberOfSamplePacketsToDownload + 1) { // +1 because of the header
        _isDownloading = false;
        _downloadSequenceIdx = 0;
        _numberOfSamplesToDownload = 0;
        _numberOfSamplePacketsToDownload = 0;
        return;
    }

    // Start Download
    if (_downloadSequenceIdx == 0) {
        _numberOfSamplesToDownload = _sampleHistory.numberOfSamplesInBuffer();
        _numberOfSamplePacketsToDownload =
            _numberOfPacketsRequired(_numberOfSamplesToDownload);
        DownloadHeader header = _buildDownloadHeader();
        _BLELibrary.characteristicSetValue(DOWNLOAD_PACKET_UUID,
                                           header.getDataArray().data(),
                                           header.getDataArray().size());
        _BLELibrary.characteristicNotify(DOWNLOAD_PACKET_UUID);
        ++_downloadSequenceIdx;
    } else { // Continue Download
        DownloadPacket packet = _buildDownloadPacket();
        _BLELibrary.characteristicSetValue(DOWNLOAD_PACKET_UUID,
                                           packet.getDataArray().data(),
                                           packet.getDataArray().size());
        _BLELibrary.characteristicNotify(DOWNLOAD_PACKET_UUID);
        ++_downloadSequenceIdx;
    }
}

void DataProvider::setSampleConfig(DataType dataType) {
    _sampleConfig = sampleConfigSelector.at(dataType);
    _sampleHistory.setSampleSize(_sampleConfig.sampleSizeBytes);
}

std::string DataProvider::_buildAdvertisementData() {
    std::string data = _advertisementHeader.getDataString();
    data.append(_currentSample.getDataString());
    return data;
}

DownloadHeader DataProvider::_buildDownloadHeader() {
    DownloadHeader header;
    uint32_t age = static_cast<uint32_t>(std::round(
        static_cast<double>(millis() - _sampleHistory.latestHistoryTimeStamp) /
        1000.0));
    header.setDownloadSampleType(_sampleConfig.downloadType);
    header.setIntervalMilliSeconds(_historyIntervalMilliSeconds);
    header.setAgeOfLatestSampleMilliSeconds(age);
    header.setDownloadSampleCount(
        static_cast<uint16_t>(_sampleHistory.numberOfSamplesInBuffer()));
    return header;
}

DownloadPacket DataProvider::_buildDownloadPacket() {
    DownloadPacket packet;
    packet.setDownloadSequenceNumber(_downloadSequenceIdx);

    int downloadPacketIdx =
        _downloadSequenceIdx - 1; // first packet is the header
    int oldestSampleHistoryIdx = _sampleHistory.getOldestSampleIndex();
    int numberOfSentSamples =
        downloadPacketIdx * _sampleConfig.sampleCountPerPacket;
    int startSampleHistoryIdx = oldestSampleHistoryIdx + numberOfSentSamples;

    for (int i = 0; i < _sampleConfig.sampleCountPerPacket; ++i) {
        if ((numberOfSentSamples + i) > _numberOfSamplesToDownload) {
            return packet;
        }
        int currentSampleHistoryIdx =
            (startSampleHistoryIdx + i) % _sampleHistory.sampleCapacity();

        for (int j = 0; j < _sampleConfig.sampleSizeBytes; ++j) {
            int currentByteHistoryIdx =
                currentSampleHistoryIdx * _sampleConfig.sampleSizeBytes + j;
            int currentBytePacketIdx = i * _sampleConfig.sampleSizeBytes + j;
            uint8_t byte = _sampleHistory.getByte(currentByteHistoryIdx);
            packet.writeSampleByte(byte, currentBytePacketIdx);
        }
    }
    return packet;
}

int DataProvider::_numberOfPacketsRequired(int numberOfSamples) const {
    return static_cast<int>(
        std::ceil(static_cast<double>(numberOfSamples) /
                  static_cast<double>(_sampleConfig.sampleCountPerPacket)));
}

void DataProvider::onHistoryIntervalChange(int interval) {
    _historyIntervalMilliSeconds = static_cast<uint64_t>(interval);
    _sampleHistory.reset();
    _BLELibrary.characteristicSetValue(
        NUMBER_OF_SAMPLES_UUID, _sampleHistory.numberOfSamplesInBuffer());
}

void DataProvider::onConnectionEvent() {
    _downloadSequenceIdx = 0;
    _isDownloading = false;
}

void DataProvider::onDownloadRequest() {
    _isDownloading = true;
}
