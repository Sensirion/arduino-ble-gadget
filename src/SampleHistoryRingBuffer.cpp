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

bool SampleHistoryRingBuffer::isWrapped() const {
    return _bufferIsWrapped;
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
    int oldestSampleIdx = _bufferIsWrapped ? _sampleIndex : 0;
    int numberOfSentSamples = downloadPacketIdx * config.sampleCountPerPacket;
    int firstSampleForPacketIdx = oldestSampleIdx + numberOfSentSamples;

    for (int i = 0; i < config.sampleCountPerPacket; ++i) {
        int sampleIdx = firstSampleForPacketIdx + i % sampleCapacity();

        for (int j = 0; j < config.sampleSizeBytes; ++j) {
            int byteIdx = sampleIdx * config.sampleSizeBytes;
            uint8_t byte = getByte(byteIdx);
            packet.writeSampleByte(byte, j);
        }
    }
    return packet;
}

void SampleHistoryRingBuffer::_writeSample(const Sample& sample) {
    for (int byteIndex = 0; byteIndex < _sampleSize; ++byteIndex) {
        _data[_sampleIndex + byteIndex] = sample.getByte(byteIndex);
    }
}
