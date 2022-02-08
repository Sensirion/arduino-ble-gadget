/*
 * Copyright (c) 2022, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _BYTE_ARRAY_H_
#define _BYTE_ARRAY_H_

#include "Arduino.h"
#include <array>

// Must explicitly instantiate template in Samples.cpp before usage
template <size_t SIZE> class ByteArray {
  public:
    std::string getDataString() {
        std::string stringData(reinterpret_cast<char*>(_data.data()),
                               _data.size());
        return stringData;
    }
    const std::array<uint8_t, SIZE>& getDataArray() const {
        return _data;
    }

    uint8_t getByte(int index) const {
        return _data[index];
    }

  protected:
    void _writeByte(uint8_t byte, size_t position) {
        assert(position >= 0 && position < SIZE);
        _data[position] = byte;
    }
    void _write16BitLittleEndian(uint16_t value, size_t position) {
        assert(position >= 0 && position < SIZE - 1);
        _data[position] = static_cast<uint8_t>(value);
        _data[position + 1] = static_cast<uint8_t>(value >> 8);
    }
    void _write16BitBigEndian(uint16_t value, size_t position) {
        assert(position >= 0 && position < SIZE - 1);
        _data[position + 1] = static_cast<uint8_t>(value);
        _data[position] = static_cast<uint8_t>(value >> 8);
    }
    void _write32BitLittleEndian(uint32_t value, size_t position) {
        assert(position >= 0 && position < SIZE - 3);
        _data[position] = static_cast<uint8_t>(value);
        _data[position + 1] = static_cast<uint8_t>(value >> 8);
        _data[position + 2] = static_cast<uint8_t>(value >> 16);
        _data[position + 3] = static_cast<uint8_t>(value >> 24);
    }
    std::array<uint8_t, SIZE> _data = {};
};

#endif /* _BYTE_ARRAY_H_ */