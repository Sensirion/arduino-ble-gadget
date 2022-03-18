// Please install the Sensirion I2C Arduino library for the SEN44 sensor module, before
// using this example code:
// https://github.com/Sensirion/arduino-i2c-sen44

#include <Arduino.h>
#include <SensirionI2CSen44.h>
#include <Wire.h>

#include "DataProvider.h"
#include "NimBLELibraryWrapper.h"

SensirionI2CSen44 sen44;

// GadgetBle workflow
static int64_t lastMeasurementTimeMs = 0;
static int measurementIntervalMs = 1000;
NimBLELibraryWrapper lib;
DataProvider provider(lib, DataType::T_RH_VOC_PM25);

void printModuleVersions() {
    uint16_t error;
    char errorMessage[256];

    uint8_t firmwareMajor;
    uint8_t firmwareMinor;
    bool firmwareDebug;
    uint8_t hardwareMajor;
    uint8_t hardwareMinor;

    error = sen44.getVersion(firmwareMajor, firmwareMinor, firmwareDebug,
                             hardwareMajor, hardwareMinor);

    if (error) {
        Serial.print("Error trying to execute getVersion(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        if (firmwareDebug) {
            printf("Development firmware version: ");
        }
        Serial.print("Firmware: ");
        Serial.print(firmwareMajor);
        Serial.print(".");
        Serial.print(firmwareMinor);
        Serial.print(", ");

        Serial.print("Hardware: ");
        Serial.print(hardwareMajor);
        Serial.print(".");
        Serial.println(hardwareMinor);
    }
}

void printSerialNumber() {
    uint16_t error;
    char errorMessage[256];

    unsigned char serialNumber[32];
    uint8_t serialNumberSize = 32;

    // Note that the I2C buffer on some Arduino boards is too small to execute
    // this command.
    error = sen44.getSerialNumber(serialNumber, serialNumberSize);

    if (error) {
        Serial.print("Error trying to execute getSerialNumber(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        Serial.print("Serial number: ");
        Serial.println((char*)serialNumber);
    }
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

    uint16_t error;
    char errorMessage[256];

    sen44.begin(Wire);

    error = sen44.deviceReset();
    if (error) {
        Serial.print("Error trying to execute getSerialNumber(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }

    // Print SEN44 module information
    printSerialNumber();
    printModuleVersions();

    // Start Measurement
    error = sen44.startMeasurement();

    if (error) {
        Serial.print("Error trying to execute startMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }
}

void loop() {
  if (millis() - lastMeasurementTimeMs >= measurementIntervalMs) {
    measure_and_report();
  }

  provider.handleDownload();
  delay(3);
}

void measure_and_report() {
    uint16_t error;
    char errorMessage[256];

    delay(1000);

    // Read Measurement
    uint16_t massConcentrationPm1p0;
    uint16_t massConcentrationPm2p5;
    uint16_t massConcentrationPm4p0;
    uint16_t massConcentrationPm10p0;
    float vocIndex;
    float ambientHumidity;
    float ambientTemperature;

    error = sen44.readMeasuredMassConcentrationAndAmbientValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, vocIndex, ambientHumidity, ambientTemperature);

    if (error) {
        Serial.print("Error trying to execute "
                     "readMeasuredMassConcentrationAndAmbientValues(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        Serial.print("MassConcentrationPm1p0:");
        Serial.print(massConcentrationPm1p0);
        Serial.print("\t");
        Serial.print("MassConcentrationPm2p5:");
        Serial.print(massConcentrationPm2p5);
        Serial.print("\t");
        Serial.print("MassConcentrationPm4p0:");
        Serial.print(massConcentrationPm4p0);
        Serial.print("\t");
        Serial.print("MassConcentrationPm10p0:");
        Serial.print(massConcentrationPm10p0);
        Serial.print("\t");
        Serial.print("VocIndex:");
        Serial.print(vocIndex);
        Serial.print("\t");
        Serial.print("AmbientHumidity:");
        Serial.print(ambientHumidity);
        Serial.print("\t");
        Serial.print("AmbientTemperature:");
        Serial.println(ambientTemperature);
    }

    provider.writeValueToCurrentSample(ambientTemperature, Unit::T);
    provider.writeValueToCurrentSample(ambientHumidity, Unit::RH);
    provider.writeValueToCurrentSample(vocIndex, Unit::VOC);
    provider.writeValueToCurrentSample(massConcentrationPm2p5, Unit::PM2P5);
    
    provider.commitSample();
    lastMeasurementTimeMs = millis();
}
