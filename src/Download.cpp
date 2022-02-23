#include "Download.h"

// DownloadHeader
void DownloadHeader::setDownloadSampleType(uint16_t type) {
    _write16BitLittleEndian(type, 4);
}
void DownloadHeader::setIntervalMilliSeconds(uint32_t interval) {
    _write32BitLittleEndian(interval, 6);
}
void DownloadHeader::setAgeOfLatestSampleMilliSeconds(uint32_t age) {
    _write32BitLittleEndian(age, 10);
}
void DownloadHeader::setDownloadSampleCount(uint16_t count) {
    _write16BitLittleEndian(count, 14);
}

// DownloadPacket
void DownloadPacket::setDownloadSequenceNumber(int16_t number) {
    _write16BitLittleEndian(number, 0);
}

void DownloadPacket::writeSample(Sample sample, size_t sampleSize,
                                 size_t position) {
    for (int i = 0; i < sampleSize; ++i) {
        writeSampleByte(sample.getByte(i), position * sampleSize + i);
    }
}

void DownloadPacket::writeSampleByte(uint8_t byte,
                                     size_t positionInSampleData) {
    _writeByte(
        byte,
        2 + positionInSampleData); // shift to after downloadSequenceNumber
}