#include "Samples.h"

// ByteArray
void BLEByteArray::writeByte(uint8_t byte, const size_t position) {
    _data[position] = byte;
}

void BLEByteArray::write16BitLittleEndian(const uint16_t value,
                                          size_t position) {
    _data[position] = static_cast<uint8_t>(value);
    _data[position + 1] = static_cast<uint8_t>(value >> 8);
}

void BLEByteArray::write16BitBigEndian(const uint16_t value, size_t position) {
    _data[position + 1] = static_cast<uint8_t>(value);
    _data[position] = static_cast<uint8_t>(value >> 8);
}

std::string BLEByteArray::getDataString() {
    std::string stringData(reinterpret_cast<char*>(_data.data()), _data.size());
    return stringData;
}

// BLEAdvertisementSample
void BLEAdvertisementSample::writeCompanyId(const uint16_t companyID) {
    write16BitLittleEndian(companyID, COMPANY_ID_POSITION);
}

void BLEAdvertisementSample::writeSensirionAdvertisementType(
    const uint8_t advType) {
    writeByte(advType, SENSIRION_ADVERTISEMENT_TYPE_POSITION);
}

void BLEAdvertisementSample::writeSampleType(const uint8_t sampleType) {
    writeByte(sampleType, SAMPLE_TYPE_POSITION);
}

void BLEAdvertisementSample::writeDeviceId(const uint16_t deviceID) {
    write16BitBigEndian(deviceID, DEVICE_ID_POSITION);
}

void BLEAdvertisementSample::writeValue(const uint16_t value,
                                        const size_t position) {
    write16BitLittleEndian(value, position);
}