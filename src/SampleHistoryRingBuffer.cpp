#include "SampleHistoryRingBuffer.h"
#include <cmath>

void SampleHistoryRingBuffer::addSample(const Sample& sample) {
    // check that sample still fits
    size_t numberOfSamples = _sampleIndex + 1;
    if (numberOfSamples >= sampleCapacity()) {
        _bufferIsWrapped = true;
        _sampleIndex = 0;
    }
    _writeSample(sample);
    ++_sampleIndex;
}

void SampleHistoryRingBuffer::setSampleSize(size_t sampleSize) {
    _sampleSize = sampleSize;
}

size_t SampleHistoryRingBuffer::sampleCapacity() const {
    if (_sampleSize == 0) {
        return 0;
    }
    return std::floor(static_cast<double>(_data.size()) /
                      static_cast<double>(_sampleSize));
}

int SampleHistoryRingBuffer::getSampleIndex() const {
    return _sampleIndex;
}

DownloadPacket
SampleHistoryRingBuffer::buildDownloadPacket(const SampleConfig& config,
                                             int downloadSequenceIdx) {
    DownloadPacket packet;
    packet.setDownloadSequenceNumber(downloadSequenceIdx);

    int downloadPacketIdx =
        downloadSequenceIdx - 1; // first packet is the header
    int oldestSampleHistoryIdx = _bufferIsWrapped ? _sampleIndex : 0;
    int numberOfSentSamples = downloadPacketIdx * config.sampleCountPerPacket;
    int startSampleHistoryIdx = oldestSampleHistoryIdx + numberOfSentSamples;

    for (int i = 0; i < config.sampleCountPerPacket; ++i) {
        int currentSampleHistoryIdx =
            startSampleHistoryIdx + i % sampleCapacity();

        for (int j = 0; j < config.sampleSizeBytes; ++j) {
            int currentByteHistoryIdx =
                currentSampleHistoryIdx * config.sampleSizeBytes;
            int currentBytePacketIdx = i * config.sampleSizeBytes + j;
            uint8_t byte = getByte(currentByteHistoryIdx);
            packet.writeSampleByte(byte, currentBytePacketIdx);
        }
    }
    return packet;
}

void SampleHistoryRingBuffer::_writeSample(const Sample& sample) {
    for (int byteIndex = 0; byteIndex < _sampleSize; ++byteIndex) {
        _data[_sampleIndex + byteIndex] = sample.getByte(byteIndex);
    }
}
