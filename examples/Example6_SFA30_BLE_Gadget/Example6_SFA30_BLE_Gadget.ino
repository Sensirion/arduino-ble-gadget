// This code is based on Sensirion's Arduino Snippets
// Check https://github.com/Sensirion/arduino-snippets for the most recent version.

#include "DataProvider.h"
#include "NimBLELibraryWrapper.h"
#include <SensirionI2CSfa3x.h>
#include <Wire.h>

SensirionI2CSfa3x sfa3x;

// GadgetBle workflow
static int64_t lastMeasurementTimeMs = 0;
static int measurementIntervalMs = 1000;
NimBLELibraryWrapper lib;
DataProvider provider(lib, DataType::T_RH_HCHO);

void setup() {
  Serial.begin(115200);
  // wait for serial connection from PC
  // comment the following line if you'd like the output
  // without waiting for the interface being ready
  while(!Serial){
    delay(100);
  }

  // Initialize the GadgetBle Library
  provider.begin();
  Serial.print("Sensirion GadgetBle Lib initialized with deviceId = ");
  Serial.println(provider.getDeviceIdString());

  // output format
  Serial.println();
  Serial.println("HCHO\tT\tRH");

  uint16_t error;
  char errorMessage[256];

  Wire.begin();
  
  // init I2C
  sfa3x.begin(Wire);

  // stop a potentially ongoing measurement
  sfa3x.stopMeasurement();
 
  // start SFA measurement in periodic mode
  error = sfa3x.startContinuousMeasurement();
  if (error) {
      Serial.print("Error trying to execute startContinuousMeasurement(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
  }
  
  // module is not outputing HCHO for the first 10 s after powering up
  delay(10000);
}

void loop() {
  if (millis() - lastMeasurementTimeMs >= measurementIntervalMs) {
    measure_and_report();
  }

  provider.handleDownload();
  delay(3);
}

void measure_and_report() {
  int16_t hcho_raw;
  int16_t humidity_raw;
  int16_t temperature_raw;
  

  delay(10);
  uint16_t error;
  char errorMessage[256];
  error = sfa3x.readMeasuredValues(hcho_raw, humidity_raw, temperature_raw);
  lastMeasurementTimeMs = millis();

  if (error) {
      Serial.print("Error trying to execute readMeasuredValues(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
      return;
  }

  // Applying scale factors before printing measured values
  Serial.print("Hcho:");
  Serial.print(hcho_raw/5.0);
  Serial.print("\t");
  Serial.print("Temperature:");
  Serial.print(temperature_raw/200.0);
  Serial.print("\t");
  Serial.print("Humididty:");
  Serial.print(humidity_raw/100.0);
  Serial.println();

  provider.writeValueToCurrentSample(hcho_raw/5.0, Unit::HCHO);
  provider.writeValueToCurrentSample(humidity_raw/100.0, Unit::RH);
  provider.writeValueToCurrentSample(temperature_raw/200.0, Unit::T);

  provider.commitSample();
}
