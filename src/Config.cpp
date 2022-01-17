#include "Config.h"

std::map<DataType, SampleDataScheme> sampleDataSchemeSelector = {
    {
        T_RH_V3,
        {DataType::T_RH_V3,
         0,
         4,
         {{Unit::T, {Unit::T, 0, &convertTemperatureV1}},
          {Unit::RH, {Unit::RH, 2, &convertHumidityV1}}}},
    },
    {
        T_RH_V4,
        {DataType::T_RH_V4,
         0,
         6,
         {{Unit::T, {Unit::T, 0, &convertTemperatureV1}},
          {Unit::RH, {Unit::RH, 2, &convertHumidityV2}}}},
    }
    /* Add new SampleDataSchemes here */
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