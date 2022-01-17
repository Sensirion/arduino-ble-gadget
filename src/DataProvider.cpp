#include "DataProvider.h"

void DataProvider::begin() {
    _BLELibrary.initDevice(GADGET_NAME);

    // Fill advertisedData byte array
    _advertisementSample.writeCompanyId(0x06D5);
    _advertisementSample.writeSensirionAdvertisementType(0x00);

    // Use part of MAC address as device id
    std::string macAddress = _BLELibrary.getDeviceAddress();
    _advertisementSample.writeDeviceId(
        strtol(macAddress.substr(12, 17).c_str(), NULL, 16));

    _BLELibrary.initAdvertising();

    _BLELibrary.setAdvertisingData(_advertisementSample.getDataString());

    _BLELibrary.startAdvertising();
}

void DataProvider::writeValue(float value, Unit unit) {
    // check for valid value
    if (isnan(value)) {
        return;
    }

    // check for correct unit
    if (_sampleDataScheme.sampleSlots.count(unit) ==
        0) { // implies unit is not part of sample
        return;
    }

    // get relevant metaData
    uint16_t (*converterFunction)(float value) =
        _sampleDataScheme.sampleSlots.at(unit).converterFunction;
    size_t offset = _sampleDataScheme.sampleSlots.at(unit).offset;

    // convert float to 16 bit int
    uint16_t convertedValue = converterFunction(value);
    _advertisementSample.writeValue(
        convertedValue,
        BLEAdvertisementSample::FIRST_SAMPLE_SLOT_POSITION + offset);
}

void DataProvider::commit() {
    _BLELibrary.stopAdvertising();
    _BLELibrary.setAdvertisingData(_advertisementSample.getDataString());
    _BLELibrary.startAdvertising();
}

void DataProvider::handleEvents() {
    // future feature;
}

void DataProvider::setSampleDataScheme(DataType dataType) {
    _sampleDataScheme = sampleDataSchemeSelector.at(dataType);
}