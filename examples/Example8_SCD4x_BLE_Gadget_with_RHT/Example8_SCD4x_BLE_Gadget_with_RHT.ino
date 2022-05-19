// This code is based on Sensirion's Arduino Snippets
// Check https://github.com/Sensirion/arduino-snippets for the most recent version.

#include "DataProvider.h"
#include "NimBLELibraryWrapper.h"
#include <SensirionI2CScd4x.h>
#include <Wire.h>


// GadgetBle workflow
static int64_t lastMeasurementTimeMs = 0;
static int measurementIntervalMs = 5000;
NimBLELibraryWrapper lib;
DataProvider provider(lib, DataType::T_RH_CO2);

SensirionI2CScd4x scd4x;

void setup() {
  Serial.begin(115200);
  // wait for serial connection from PC
  // comment the following line if you'd like the output
  // without waiting for the interface being ready
  while(!Serial);

  // Initialize the GadgetBle Library
  provider.begin();
  Serial.print("Sensirion GadgetBle Lib initialized with deviceId = ");
  Serial.println(provider.getDeviceIdString());
  
  // init I2C
  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  scd4x.begin(Wire);

  // stop potentially previously started measurement
  scd4x.stopPeriodicMeasurement();

  // Start Measurement
  error = scd4x.startPeriodicMeasurement();
  if (error) {
      Serial.print("Error trying to execute startPeriodicMeasurement(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
  }

  Serial.println("Waiting for first measurement... (5 sec)");
  Serial.println("CO2(ppm)\tTemperature(degC)\tRelativeHumidity(percent)");
  delay(5000);
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
    
  // Read Measurement
  uint16_t co2;
  float temperature;
  float humidity;

  error = scd4x.readMeasurement(co2, temperature, humidity);
  lastMeasurementTimeMs = millis();

  if (error) {
    Serial.print("Error trying to execute readMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
    return;
  }

  if (co2 == 0) {
    Serial.println("Invalid sample detected, skipping.");
    return;
  }
  
  Serial.print("co2:");
  Serial.print(co2);
  Serial.print("\t");
  Serial.print("temperature:");
  Serial.print(temperature);
  Serial.print("\t");
  Serial.print("humidity:");
  Serial.println(humidity);

  provider.writeValueToCurrentSample(co2, Unit::CO2);
  provider.writeValueToCurrentSample(temperature, Unit::T);
  provider.writeValueToCurrentSample(humidity, Unit::RH);

  provider.commitSample();
  
}
