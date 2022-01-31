#include "Samples.h"



// ByteBuffer
template<size_t SIZE>
std::string ByteBuffer<SIZE>::getDataString() {
    std::string stringData(reinterpret_cast<char*>(_data.data()), _data.size());
    return stringData;
}

template<size_t SIZE>
void ByteBuffer<SIZE>::writeValue(uint16_t value, size_t position) {
    _write16BitLittleEndian(value, position);
}

template<size_t SIZE>
void ByteBuffer<SIZE>::_writeByte(uint8_t byte, size_t position) {
    assert(position >= 0 && position < SIZE);
    _data[position] = byte;
}

template<size_t SIZE>
void ByteBuffer<SIZE>::_write16BitLittleEndian(uint16_t value, size_t position) {
    assert(position >= 0 && position < SIZE - 1);
    _data[position] = static_cast<uint8_t>(value);
    _data[position + 1] = static_cast<uint8_t>(value >> 8);
}

template<size_t SIZE>
void ByteBuffer<SIZE>::_write16BitBigEndian(uint16_t value, size_t position) {
    assert(position >= 0 && position < SIZE - 1);
    _data[position + 1] = static_cast<uint8_t>(value);
    _data[position] = static_cast<uint8_t>(value >> 8);
}

// Explicit template instantiation (so that template methods may be 
//  defined in implementation file)
template class ByteBuffer<SAMPLE_BUFFER_SIZE_BYTES>;
template class ByteBuffer<SAMPLE_HISTORY_BUFFER_SIZE_BYTES>;
template class ByteBuffer<ADVERTISEMENT_HEADER_BUFFER_SIZE_BYTES>;
template class ByteBuffer<DOWNLOAD_PACKET_BUFFER_SIZE_BYTES>;

// AdvertisementHeaderBuffer
void AdvertisementHeaderBuffer::writeCompanyId(uint16_t companyID) {
    _write16BitLittleEndian(companyID, 0);
}

void AdvertisementHeaderBuffer::writeSensirionAdvertisementType(uint8_t advType) {
    _writeByte(advType, 2);
}

void AdvertisementHeaderBuffer::writeSampleType(uint8_t sampleType) {
    _writeByte(sampleType, 3);
}

void AdvertisementHeaderBuffer::writeDeviceId(uint16_t deviceID) {
    _write16BitBigEndian(deviceID, 4);
}
