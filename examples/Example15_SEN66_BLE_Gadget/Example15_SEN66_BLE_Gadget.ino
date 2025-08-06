// Please install the Sensirion I2C Arduino library for the SEN66 sensor module, before
// using this example code:
// https://github.com/Sensirion/arduino-i2c-sen66
#include "Sensirion_Gadget_BLE.h"
#include <SensirionI2cSen66.h>

SensirionI2cSen66 sen66;

// Error definitions
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0
static char errorMessage[64];
static int16_t error;


// GadgetBle workflow
static int64_t lastMeasurementTimeMs = 0;
static int measurementIntervalMs = 1000;
NimBLELibraryWrapper lib;
DataProvider provider(lib, DataType::T_RH_CO2_VOC_NOX_PM25);


void printSerialNumber() {
    int8_t serialNumber[32] = {0};
    error = sen66.getSerialNumber(serialNumber, 32);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute getSerialNumber(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    Serial.print("serialNumber: ");
    Serial.print((const char*)serialNumber);
    Serial.println();
}


void setup() {

    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }

    // Initialize the GadgetBle Library
    provider.begin();
    Serial.print("Sensirion GadgetBle Lib initialized with deviceId = ");
    Serial.println(provider.getDeviceIdString());

    Wire.begin();
    sen66.begin(Wire, SEN66_I2C_ADDR_6B);

    error = sen66.deviceReset();
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute deviceReset(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    delay(1200);

    // Print SEN66 Serial Number
    printSerialNumber();

    // Start Measurement
    error = sen66.startContinuousMeasurement();
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute startContinuousMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
}

void loop() {
  if (millis() - lastMeasurementTimeMs >= measurementIntervalMs) {
      measureAndReport();
  }

  provider.handleDownload();
  delay(20); 
}

/**
* Return true if all measured values are valid (not unknown)
*/
bool checkMeasuredValues(
    float massConcentrationPm2p5,
    float humidity,
    float temperature,
    float vocIndex,
    float noxIndex,
    uint16_t co2
    ) {
    // handle unknown samples
    const float unknownPm = 0xffff / 10.0;
    const float unknownGasIndex = 0x7fff / 10.0;
    const float unknownHumidity = 0x7fff / 100.0;
    const float unknownTemperature = 0x7fff / 200.0;
    const uint16_t unknownCo2 = 0xffff;

    if (massConcentrationPm2p5 == unknownPm) {
        return false;
    }

    if (humidity == unknownHumidity) {
        return false;
    }
    if (temperature == unknownTemperature) {
        return false;
    }
    if (vocIndex == unknownGasIndex) {
        return false;
    }
    if (noxIndex == unknownGasIndex) {
        return false;
    }
    if (co2 == unknownCo2) {
        return false;
    }
    return true;
}

void measureAndReport() {
    // Measured values
    float massConcentrationPm1p0 = 0.0;
    float massConcentrationPm2p5 = 0.0;
    float massConcentrationPm4p0 = 0.0;
    float massConcentrationPm10p0 = 0.0;
    float humidity = 0.0;
    float temperature = 0.0;
    float vocIndex = 0.0;
    float noxIndex = 0.0;
    uint16_t co2 = 0;

    error = sen66.readMeasuredValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, humidity, temperature, vocIndex, noxIndex,
        co2);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute readMeasuredValues(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }

    // update last measurement time
    lastMeasurementTimeMs = millis();

    if (!checkMeasuredValues(massConcentrationPm2p5, humidity, temperature,
                               vocIndex, noxIndex, co2)) {
        return;
    }

    Serial.print("massConcentrationPm1p0: ");
    Serial.print(massConcentrationPm1p0);
    Serial.print("\t");
    Serial.print("massConcentrationPm2p5: ");
    Serial.print(massConcentrationPm2p5);
    Serial.print("\t");
    Serial.print("massConcentrationPm4p0: ");
    Serial.print(massConcentrationPm4p0);
    Serial.print("\t");
    Serial.print("massConcentrationPm10p0: ");
    Serial.print(massConcentrationPm10p0);
    Serial.print("\t");
    Serial.print("humidity: ");
    Serial.print(humidity);
    Serial.print("\t");
    Serial.print("temperature: ");
    Serial.print(temperature);
    Serial.print("\t");
    Serial.print("vocIndex: ");
    Serial.print(vocIndex);
    Serial.print("\t");
    Serial.print("noxIndex: ");
    Serial.print(noxIndex);
    Serial.print("\t");
    Serial.print("co2: ");
    Serial.print(co2);
    Serial.println();

    provider.writeValueToCurrentSample(temperature, SignalType::TEMPERATURE_DEGREES_CELSIUS);
    provider.writeValueToCurrentSample(humidity, SignalType::RELATIVE_HUMIDITY_PERCENTAGE);
    provider.writeValueToCurrentSample(co2, SignalType::CO2_PARTS_PER_MILLION);
    provider.writeValueToCurrentSample(vocIndex, SignalType::VOC_INDEX);
    provider.writeValueToCurrentSample(noxIndex, SignalType::NOX_INDEX);
    provider.writeValueToCurrentSample(massConcentrationPm2p5, SignalType::PM2P5_MICRO_GRAMM_PER_CUBIC_METER);

    provider.commitSample();
}
