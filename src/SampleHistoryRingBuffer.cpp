#include "SampleHistoryRingBuffer.h"
#include <cmath>

void SampleHistoryRingBuffer::putSample(const Sample& sample) {
    // iterate outSampleIndex if overwriting
    if (isFull()) {
        _tail = _nextIndex(_tail);
    }
    // copy byte wise
    _writeSample(sample);

    // iterate _head
    _head = _nextIndex(_head);
}

void SampleHistoryRingBuffer::setSampleSize(size_t sampleSize) {
    _sampleSizeBytes = sampleSize;
    reset();
}

int SampleHistoryRingBuffer::numberOfSamplesInHistory() const {
    int diff = _head - _tail;
    if (diff >= 0) {
        return diff;
    } else {
        return _sizeInSamples() + diff;
    }
}

bool SampleHistoryRingBuffer::isFull() const {
    return _nextIndex(_head) == _tail;
}

void SampleHistoryRingBuffer::startReadOut() {
    _sampleReadOutIndex = _tail;
}

// May give out invalid sample if called on an empty sample history
Sample SampleHistoryRingBuffer::readOutNextSample(bool& allSamplesRead) {
    Sample sample = _readSample(_sampleReadOutIndex);
    if (!allSamplesRead) {
        _sampleReadOutIndex = _nextIndex(_sampleReadOutIndex);
    }
    allSamplesRead = (_sampleReadOutIndex == _head);
    return sample;
}

void SampleHistoryRingBuffer::reset() {
    _head = 0;
    _tail = 0;
    _sampleReadOutIndex = 0;
}

int SampleHistoryRingBuffer::_nextIndex(int index) const {
    return (index + 1) % _sizeInSamples();
}

size_t SampleHistoryRingBuffer::_sizeInSamples() const {
    if (_sampleSizeBytes == 0) {
        return 0;
    } else {
        return (SAMPLE_HISTORY_RING_BUFFER_SIZE_BYTES / _sampleSizeBytes);
    }
}

void SampleHistoryRingBuffer::_writeSample(const Sample& sample) {
    for (int byteIndex = 0; byteIndex < _sampleSizeBytes; ++byteIndex) {
        _data[_head * _sampleSizeBytes + byteIndex] = sample.getByte(byteIndex);
    }
}

Sample SampleHistoryRingBuffer::_readSample(int sampleIndex) const {
    Sample sample;
    for (int i = 0; i < _sampleSizeBytes; ++i) {
        uint8_t byte = getByte((sampleIndex * _sampleSizeBytes) + i);
        sample.setByte(byte, i);
    }
    return sample;
}
