// This code is based on Sensirion's Arduino Snippets
// Check https://github.com/Sensirion/arduino-snippets for the most recent version.

#include "esp_timer.h"
#include "Sensirion_GadgetBle_Lib.h"
#include <SensirionI2CSfa3x.h>
#include <Wire.h>

SensirionI2CSfa3x sfa3x;

// GadgetBle workflow
static int64_t lastMmntTime = 0;
static int mmntInterval = 1000000;
GadgetBle gadgetBle = GadgetBle(GadgetBle::DataType::T_RH_HCHO);

void setup() {
  Serial.begin(115200);
  // wait for serial connection from PC
  // comment the following line if you'd like the output
  // without waiting for the interface being ready
  while(!Serial){
    delay(100);
  }

  // Initialize the GadgetBle Library
  gadgetBle.begin();
  Serial.print("Sensirion GadgetBle Lib initialized with deviceId = ");
  Serial.println(gadgetBle.getDeviceIdString());

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
  if (esp_timer_get_time() - lastMmntTime >= mmntInterval) {
    measure_and_report();
  }

  gadgetBle.handleEvents();
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
  lastMmntTime = esp_timer_get_time();

  if (error) {
      Serial.print("Error trying to execute readMeasuredValues(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
      return;
  }

  // Applying scale factors before printing measured values
  Serial.print(hcho_raw/5.0);
  Serial.print("\t");
  Serial.print(temperature_raw/200.0);
  Serial.print("\t");
  Serial.print(humidity_raw/100.0);
  Serial.println();

  gadgetBle.writeHCHO(hcho_raw/5.0);
  gadgetBle.writeHumidity(humidity_raw/100.0);
  gadgetBle.writeTemperature(temperature_raw/200.0);

  gadgetBle.commit();
}