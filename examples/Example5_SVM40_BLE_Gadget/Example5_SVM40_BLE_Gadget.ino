// This code is based on Sensirion's Arduino Snippets
// Check https://github.com/Sensirion/arduino-snippets for the most recent version.

#include "DataProvider.h"
#include "NimBLELibraryWrapper.h"
#include <SensirionI2CSvm40.h>
#include <Wire.h>

SensirionI2CSvm40 svm40;

// Temperature offset in degC
static int temperature_offset = -1;

// GadgetBle workflow
static int64_t lastMeasurementTimeMs = 0;
static int measurementIntervalMs = 1000;
NimBLELibraryWrapper lib;
DataProvider provider(lib, DataType::T_RH_VOC);

void setup() {

  Serial.begin(115200);
  // wait for serial connection from PC
  // comment the following lines if you'd like the output
  // without waiting for the interface being ready
  while (!Serial) {
      delay(100);
  }

  // Initialize the GadgetBle Library
  provider.begin();
  Serial.print("Sensirion GadgetBle Lib initialized with deviceId = ");
  Serial.println(provider.getDeviceIdString());

  // init I2C
  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  svm40.begin(Wire);

  // wait until sensors startup, > 1 ms according to datasheet
  delay(100);

  int16_t t_offset;
  
  // read t offset from flash memory
  svm40.getTemperatureOffsetForRhtMeasurements(t_offset);
  
  // print value
  Serial.print("Default T Offset: ");
  Serial.print(t_offset/200);
  Serial.println();

  // set t offset to new value (degC) and scale it accordingly by a factor of 200
  t_offset = temperature_offset * 200;

  // send new value for t offset to sensor (will be hold in RAM, not persistent)
  svm40.setTemperatureOffsetForRhtMeasurements(t_offset);

  // repeat read offset data to make sure that the values are applied correctly
  svm40.getTemperatureOffsetForRhtMeasurements(t_offset);
  
  Serial.print("New T Offset: ");
  Serial.println(t_offset/200);
  Serial.println();

  // output format
  Serial.println("VOC_Index\tRH\tT");

  // Start continous measurement
  // each second there will be new measurement values
  error = svm40.startContinuousMeasurement();
  if (error) {
      Serial.print("Error trying to execute startContinuousMeasurement(): ");
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

  int16_t vocIndex;
  int16_t humidity;
  int16_t temperature;
  
  // wait 10 ms to allow the sensor to fill the internal buffer
  delay(10);

  // VOC level is a signed int and scaled by a factor of 10 and needs to be divided by 10
  // VOC raw value is an uint16_t and has no scaling
  // humidity is a signed int and scaled by 100 and need to be divided by 100
  // temperature is a signed int and scaled by 200 and need to be divided by 200
  error = svm40.readMeasuredValuesAsIntegers(vocIndex, humidity, temperature);
  if (error) {
      Serial.print(
          "Error trying to execute readMeasuredValuesAsIntegers(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
  } else {
      // Provide the sensor values for Tools -> Serial Monitor or Serial Plotter
      Serial.print("VocIndex:");
      Serial.print(vocIndex / 10.0);
      Serial.print("\t");
      Serial.print("Humidity[%]:");
      Serial.print(humidity / 100.0);
      Serial.print("\t");
      Serial.print("Temperature[℃]:");
      Serial.println(temperature / 200.0);
  }

  provider.writeValueToCurrentSample(float(vocIndex) / 10, Unit::VOC);
  provider.writeValueToCurrentSample(float(humidity) / 100, Unit::RH);
  provider.writeValueToCurrentSample(float(temperature) / 200, Unit::T);

  provider.commitSample();
  lastMeasurementTimeMs = millis();
}
