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

#ifndef _SAMPLES_H_
#define _SAMPLES_H_

#include <Arduino.h>
#include <map>
#include <vector>

struct BLEByteArray {
    static const size_t DATA_LENGTH = 18;
    void writeByte(uint8_t byte, const size_t position);
    void write16BitLittleEndian(const uint16_t value, size_t position);
    void write16BitBigEndian(const uint16_t value, size_t position);
    std::string getDataString();
    std::array<uint8_t, DATA_LENGTH> _data = {};
};

struct BLEAdvertisementSample: BLEByteArray {
    static const size_t COMPANY_ID_POSITION = 0;
    static const size_t SENSIRION_ADVERTISEMENT_TYPE_POSITION = 2;
    static const size_t SAMPLE_TYPE_POSITION = 3;
    static const size_t DEVICE_ID_POSITION = 4;
    static const size_t FIRST_SAMPLE_SLOT_POSITION = 6;

    void writeCompanyId(const uint16_t companyID);
    void writeSensirionAdvertisementType(const uint8_t advType);
    void writeSampleType(const uint8_t sampleType);
    void writeDeviceId(const uint16_t deviceID);
    void writeValue(const uint16_t value, const size_t position);
};

class BLEDataLoggerSample: public BLEByteArray {
    // todo
};

#endif /* _SAMPLES_H_ */