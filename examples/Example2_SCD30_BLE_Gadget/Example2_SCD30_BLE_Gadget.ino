#include "Sensirion_Gadget_BLE.h"
#include <SensirionI2cScd30.h>

static int64_t lastMeasurementTimeMs = 0;
static int measurementIntervalMs = 1900;

NimBLELibraryWrapper lib;
DataProvider provider(lib, DataType::T_RH_CO2_ALT);

SensirionI2cScd30 sensor;
static char errorMessage[128];
static int16_t error;

void setup() {
    Serial.begin(115200);
    delay(100);

    // Initialize the GadgetBle Library
    provider.begin();
    Serial.print("Sensirion GadgetBle Lib initialized with deviceId = ");
    Serial.println(provider.getDeviceIdString());

    // Initialize the SCD30 driver
    Wire.begin();
    sensor.begin(Wire, SCD30_I2C_ADDR_61);

    sensor.stopPeriodicMeasurement();
    sensor.softReset();
    sensor.activateAutoCalibration(1);

    error = sensor.startPeriodicMeasurement(0);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute startPeriodicMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
}

void loop() {
    float co2Concentration = 0.0;
    float temperature = 0.0;
    float humidity = 0.0;
    uint16_t data_ready = 0;
    if (millis() - lastMeasurementTimeMs >= measurementIntervalMs) {
        sensor.getDataReady(data_ready);
        if (bool(data_ready)) {
            error = sensor.readMeasurementData(co2Concentration, temperature,
                                               humidity);
            if (error != NO_ERROR) {
                Serial.print("Error trying to execute readMeasurementData(): ");
                errorToString(error, errorMessage, sizeof errorMessage);
                Serial.println(errorMessage);
                return;
            }

            provider.writeValueToCurrentSample(
                co2Concentration, SignalType::CO2_PARTS_PER_MILLION);
            provider.writeValueToCurrentSample(
                temperature, SignalType::TEMPERATURE_DEGREES_CELSIUS);
            provider.writeValueToCurrentSample(
                humidity, SignalType::RELATIVE_HUMIDITY_PERCENTAGE);
            provider.commitSample();

            lastMeasurementTimeMs = millis();

            // Provide the sensor values for Tools -> Serial Monitor or Serial
            // Plotter
            Serial.print("CO2[ppm]:");
            Serial.print(co2Concentration);
            Serial.print("\t");
            Serial.print("Temperature[℃]:");
            Serial.print(temperature);
            Serial.print("\t");
            Serial.print("Humidity[%]:");
            Serial.println(humidity);
        }
    }

    provider.handleDownload();
    delay(3);
}