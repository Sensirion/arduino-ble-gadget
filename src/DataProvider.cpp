#include "DataProvider.h"

void DataProvider::begin() {
    _BLELibrary.init();

    // Fill advertisedData byte array
    _advertisementHeader.writeCompanyId(0x06D5);
    _advertisementHeader.writeSensirionAdvertisementType(0x00);

    // Use part of MAC address as device id
    std::string macAddress = _BLELibrary.getDeviceAddress();
    _advertisementHeader.writeDeviceId(
        strtol(macAddress.substr(12, 17).c_str(), NULL, 16));

    _BLELibrary.setAdvertisingData(_advertisementHeader.getDataString());

    _BLELibrary.startAdvertising();
}

void DataProvider::writeValueToCurrentSample(float value, Unit unit) {
    // check for valid value
    if (isnan(value)) {
        return;
    }

    // check for correct unit
    if (_sampleConfig.sampleSlots.count(unit) ==
        0) { // implies unit is not part of sample
        return;
    }

    // get relevant metaData
    uint16_t (*converterFunction)(float value) =
        _sampleConfig.sampleSlots.at(unit).converterFunction;
    size_t offset = _sampleConfig.sampleSlots.at(unit).offset;

    // convert float to 16 bit int
    uint16_t convertedValue = converterFunction(value);
    _currentSample.writeValue(
        convertedValue,
        offset);
}

void DataProvider::commitSample() {
    // add sample to sampleBuffer: TODO

    // Create Advertising Packet
    std::string advertisementPacket = _buildAdvertisementData();

    // Update Advertising
    _BLELibrary.stopAdvertising();
    _BLELibrary.setAdvertisingData(advertisementPacket);
    _BLELibrary.startAdvertising();
}

void DataProvider::handleEvents() {
    // future feature: TODO
}

void DataProvider::setSampleConfig(DataType dataType) {
    _sampleConfig = sampleConfigSelector.at(dataType);
}

std::string DataProvider::_buildAdvertisementData() {
    std::string data = _advertisementHeader.getDataString();
    data.append(_currentSample.getDataString());
    return data;
}