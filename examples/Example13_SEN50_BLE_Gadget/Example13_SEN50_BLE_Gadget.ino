// Please install the Sensirion I2C Arduino library for the SEN50 sensor module,
// before using this example code:
// https://github.com/Sensirion/arduino-i2c-sen5x

#include <Arduino.h>
#include <SensirionI2CSen5x.h>
#include <Wire.h>

#include "DataProvider.h"
#include "NimBLELibraryWrapper.h"

SensirionI2CSen5x sen5x;

// GadgetBle workflow
static int64_t lastMeasurementTimeMs = 0;
static int measurementIntervalMs = 1000;
NimBLELibraryWrapper lib;
DataProvider provider(lib, DataType::PM10_PM25_PM40_PM100);

void printModuleVersions() {
    uint16_t error;
    char errorMessage[256];

    unsigned char productName[32];
    uint8_t productNameSize = 32;

    error = sen5x.getProductName(productName, productNameSize);

    if (error) {
        Serial.print("Error trying to execute getProductName(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        Serial.print("ProductName:");
        Serial.println((char*)productName);
    }

    uint8_t firmwareMajor;
    uint8_t firmwareMinor;
    bool firmwareDebug;
    uint8_t hardwareMajor;
    uint8_t hardwareMinor;
    uint8_t protocolMajor;
    uint8_t protocolMinor;

    error = sen5x.getVersion(firmwareMajor, firmwareMinor, firmwareDebug,
                             hardwareMajor, hardwareMinor, protocolMajor,
                             protocolMinor);
    if (error) {
        Serial.print("Error trying to execute getVersion(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
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

    error = sen5x.getSerialNumber(serialNumber, serialNumberSize);
    if (error) {
        Serial.print("Error trying to execute getSerialNumber(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        Serial.print("SerialNumber:");
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

    sen5x.begin(Wire);

    error = sen5x.deviceReset();
    if (error) {
        Serial.print("Error trying to execute deviceReset(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }

    // Print SEN50 module information
    printSerialNumber();
    printModuleVersions();

    // Start Measurement
    error = sen5x.startMeasurement();

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
    float massConcentrationPm1p0;
    float massConcentrationPm2p5;
    float massConcentrationPm4p0;
    float massConcentrationPm10p0;

    error = sen5x.readMeasuredValuesSen50(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0);

    if (error) {
        Serial.print("Error trying to execute "
                     "readMeasuredValuesSen50(): ");
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
        Serial.print("\n");
    }

    provider.writeValueToCurrentSample(massConcentrationPm1p0, Unit::PM1P0);
    provider.writeValueToCurrentSample(massConcentrationPm2p5, Unit::PM2P5);
    provider.writeValueToCurrentSample(massConcentrationPm4p0, Unit::PM4P0);
    provider.writeValueToCurrentSample(massConcentrationPm10p0, Unit::PM10);
    provider.commitSample();
    lastMeasurementTimeMs = millis();
}
