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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <map>

enum Unit {
    T,
    RH,
    VOC,
    NOX,
    CO2,
    PM1P0,
    PM2P5,
    PM4P0,
    PM10,
    HCHO /* Add new unit here */
};

enum DataType {
    T_RH_V3,
    T_RH_V4,
    T_RH_VOC,
    T_RH_CO2,
    T_RH_CO2_ALT,
    T_RH_CO2_PM25,
    T_RH_VOC_PM25,
    T_RH_VOC_NOX,
    T_RH_VOC_NOX_PM25,
    T_RH_HCHO,
    T_RH_CO2_VOC_PM25_HCHO,
    T_RH_CO2_VOC_NOX_PM25,
    T_RH_CO2_PM25_V2,
    T_RH_VOC_PM25_V2,
    T_RH_CO2_VOC_PM25_HCHO_V2,
    PM10_PM25_PM40_PM100,
    CO2_DataType
    /* Add new DataType here. Don't forget to add corresponding
    SampleConfig in Config.cpp */
};

// Converter functions
uint16_t convertSimple(float value);
uint16_t convertTemperatureV1(float value);
uint16_t convertHumidityV1(float value);
uint16_t convertHumidityV2(float value);
uint16_t convertPM2p5V1(float value);
uint16_t convertPMV2(float value);
uint16_t convertHCHOV1(float value);
/* Declare new converter function here, define in Config.cpp */

struct SampleSlot {
    Unit unit;
    size_t offset;
    uint16_t (*converterFunction)(float value);
};

struct SampleConfig {
    DataType dataType;
    uint16_t downloadType;
    uint8_t sampleType;
    size_t sampleSizeBytes;
    size_t sampleCountPerPacket;
    uint8_t sensirionAdvertisementSampleType;
    std::map<Unit, SampleSlot> sampleSlots;
};

extern std::map<DataType, SampleConfig> sampleConfigSelector;

#endif /* _CONFIG_H_ */
