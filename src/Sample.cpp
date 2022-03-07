#include "Sample.h"

void Sample::writeValue(uint16_t value, size_t position) {
    _write16BitLittleEndian(value, position);
}

void Sample::setByte(uint8_t byte, size_t position) {
    _data[position] = byte;
}
