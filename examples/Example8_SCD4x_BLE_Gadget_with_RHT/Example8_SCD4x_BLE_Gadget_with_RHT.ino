// This code is based on Sensirion's Arduino Snippets
// Check https://github.com/Sensirion/arduino-snippets for the most recent version.

#include "esp_timer.h"
#include "Sensirion_GadgetBle_Lib.h"
#include <SensirionI2CScd4x.h>
#include <Wire.h>


// GadgetBle workflow
static int64_t lastMmntTime = 0;
static int mmntIntervalUs = 5000000;
GadgetBle gadgetBle = GadgetBle(GadgetBle::DataType::T_RH_CO2);

SensirionI2CScd4x scd4x;

void setup() {
  Serial.begin(115200);
  // wait for serial connection from PC
  // comment the following line if you'd like the output
  // without waiting for the interface being ready
  while(!Serial);

  // Initialize the GadgetBle Library
  gadgetBle.begin();
  Serial.print("Sensirion GadgetBle Lib initialized with deviceId = ");
  Serial.println(gadgetBle.getDeviceIdString());
  
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
  if (esp_timer_get_time() - lastMmntTime >= mmntIntervalUs) {
    measure_and_report();
  }

  gadgetBle.handleEvents();
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
  lastMmntTime = esp_timer_get_time();

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

  Serial.print(co2);
  Serial.print("\t");
  Serial.print(temperature);
  Serial.print("\t");
  Serial.println(humidity);

  gadgetBle.writeCO2(co2);
  gadgetBle.writeTemperature(temperature);
  gadgetBle.writeHumidity(humidity);
  gadgetBle.commit();
}