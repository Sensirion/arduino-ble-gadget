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

void SampleHistoryRingBuffer::_writeSample(const Sample& sample) {
    for (int byteIndex = 0; byteIndex < _sampleSize; ++byteIndex) {
        _data[_sampleIndex + byteIndex] = sample.getByte(byteIndex);
    }
}
