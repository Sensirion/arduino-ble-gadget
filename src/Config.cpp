#include "Config.h"

std::map<DataType, SampleConfig> sampleConfigSelector = {
    {T_RH_V3,
     {.dataType = DataType::T_RH_V3,
      .downloadType = 0,
      .sampleType = 4,
      .sampleSize = 4,
      .sensirionAdvertisementSampleType = 0,
      .sampleSlots = {{Unit::T,
                       {.unit = Unit::T,
                        .offset = 0,
                        .converterFunction = &convertTemperatureV1}},
                      {Unit::RH,
                       {.unit = Unit::RH,
                        .offset = 2,
                        .converterFunction = &convertHumidityV1}}}}},
    {T_RH_V4,
     {.dataType = DataType::T_RH_V4,
      .downloadType = 5,
      .sampleType = 6,
      .sampleSize = 4,
      .sensirionAdvertisementSampleType = 0,
      .sampleSlots = {{Unit::T,
                       {.unit = Unit::T,
                        .offset = 0,
                        .converterFunction = &convertTemperatureV1}},
                      {Unit::RH,
                       {.unit = Unit::RH,
                        .offset = 2,
                        .converterFunction = &convertHumidityV2}}}}}

    /* Add new SampleConfigs here */
};

// Converter functions
uint16_t convertSimple(float value) {
    return static_cast<uint16_t>(value + 0.5f);
}

uint16_t convertTemperatureV1(float value) {
    return static_cast<uint16_t>((((value + 45) / 175) * 65535) + 0.5f);
}

uint16_t convertHumidityV1(float value) {
    return static_cast<uint16_t>(((value / 100) * 65535) + 0.5f);
}

uint16_t convertHumidityV2(float value) {
    return static_cast<uint16_t>((((value + 6.0) * 65535) / 125.0) + 0.5f);
}

uint16_t convertPM2p5V1(float value) {
    return static_cast<uint16_t>(((value / 1000) * 65535) + 0.5f);
}

uint16_t convertPM2p5V2(float value) {
    return static_cast<uint16_t>((value * 10) + 0.5f);
}

uint16_t convertHCHOV1(float value) {
    return static_cast<uint16_t>((value * 5) + 0.5f);
}
/* Define new converter function here */