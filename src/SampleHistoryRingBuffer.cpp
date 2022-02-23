#include "SampleHistoryRingBuffer.h"
#include <cmath>

void SampleHistoryRingBuffer::putSample(const Sample& sample) {
    // iterate outSampleIndex if overwriting
    if (_empty) { // but not if it is the very first sample
        _empty = false;
    } else if (_inSampleIndex == _outSampleIndex) {
        _outSampleIndex = (_outSampleIndex + 1) % sizeInSamples();
    }
    // copy byte wise
    _writeSample(sample);

    // iterate _inSampleIndex
    _inSampleIndex = (_inSampleIndex + 1) % sizeInSamples();
}

Sample SampleHistoryRingBuffer::getSample() {
    Sample sample = _fetchSample();

    ++_outSampleIndex;
    _outSampleIndex %= sizeInSamples();

    if (_outSampleIndex == _inSampleIndex) {
        _empty = true;
    }
    return sample;
}

void SampleHistoryRingBuffer::setSampleSize(size_t sampleSize) {
    _sampleSizeBytes = sampleSize;
    reset();
}

size_t SampleHistoryRingBuffer::sizeInSamples() const {
    return SAMPLE_HISTORY_RING_BUFFER_SIZE_BYTES / _sampleSizeBytes;
}

int SampleHistoryRingBuffer::numberOfSamplesInHistory() const {
    if (_empty) {
        return 0;
    }

    int diff = _inSampleIndex - _outSampleIndex;
    if (diff > 0) {
        return diff;
    } else {
        return sizeInSamples() - diff;
    }
}

bool SampleHistoryRingBuffer::isEmpty() const {
    return _empty;
}

void SampleHistoryRingBuffer::reset() {
    _inSampleIndex = 0;
    _outSampleIndex = 0;
    _empty = true;
}

void SampleHistoryRingBuffer::_writeSample(const Sample& sample) {
    for (int byteIndex = 0; byteIndex < _sampleSizeBytes; ++byteIndex) {
        _data[_inSampleIndex * _sampleSizeBytes + byteIndex] =
            sample.getByte(byteIndex);
    }
}

Sample SampleHistoryRingBuffer::_fetchSample() {
    Sample sample;
    for (int i = 0; i < _sampleSizeBytes; ++i) {
        uint8_t byte = getByte((_outSampleIndex * _sampleSizeBytes) + i);
        sample.setByte(byte, i);
    }
    return sample;
}
