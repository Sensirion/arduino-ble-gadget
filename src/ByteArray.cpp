#include "ByteArray.h"

// Explicit template instantiation (so that template methods may be
// defined in implementation file)
template class ByteArray<SAMPLE_SIZE_BYTES>;
template class ByteArray<ADVERTISEMENT_HEADER_SIZE_BYTES>;
template class ByteArray<SAMPLE_HISTORY_RING_BUFFER_SIZE_BYTES>;
template class ByteArray<DOWNLOAD_PACKET_SIZE_BYTES>;

// ByteArray
template <size_t SIZE> std::string ByteArray<SIZE>::getDataString() {
    std::string stringData(reinterpret_cast<char*>(_data.data()), _data.size());
    return stringData;
}

template <size_t SIZE>
void ByteArray<SIZE>::_writeByte(uint8_t byte, size_t position) {
    assert(position >= 0 && position < SIZE);
    _data[position] = byte;
}

template <size_t SIZE>
void ByteArray<SIZE>::_write16BitLittleEndian(uint16_t value, size_t position) {
    assert(position >= 0 && position < SIZE - 1);
    _data[position] = static_cast<uint8_t>(value);
    _data[position + 1] = static_cast<uint8_t>(value >> 8);
}

template <size_t SIZE>
void ByteArray<SIZE>::_write16BitBigEndian(uint16_t value, size_t position) {
    assert(position >= 0 && position < SIZE - 1);
    _data[position + 1] = static_cast<uint8_t>(value);
    _data[position] = static_cast<uint8_t>(value >> 8);
}