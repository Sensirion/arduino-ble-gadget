// This code is based on Sensirion's Arduino Snippets
// Check https://github.com/Sensirion/arduino-snippets for the most recent version.

#include "Sensirion_Gadget_BLE.h"
#include <SensirionI2CScd4x.h>

// GadgetBle workflow
static int64_t lastMeasurementTimeMs = 0;
static int measurementIntervalMs = 5000;
NimBLELibraryWrapper lib;
// initialize DataProvider with SCD FRC BLE Servie enabled
DataProvider provider(lib, DataType::T_RH_CO2, false, false, true);
char errorMessage[256];

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
    measureAndReport();
  }

  provider.handleDownload();
  handleFrcRequest();
  delay(3);
}

void measureAndReport() {
  uint16_t error;
    
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

  provider.writeValueToCurrentSample(co2, SignalType::CO2_PARTS_PER_MILLION);
  provider.writeValueToCurrentSample(temperature, SignalType::TEMPERATURE_DEGREES_CELSIUS);
  provider.writeValueToCurrentSample(humidity, SignalType::RELATIVE_HUMIDITY_PERCENTAGE);

  provider.commitSample();
  
}

void handleFrcRequest() {
  if(!provider.isFRCRequested()) {
    return;
  }
  uint16_t reference_co2_level = provider.getReferenceCO2Level();
  Serial.print("Performing FRC with CO2 reference level [ppm]: ");
  Serial.println(reference_co2_level);
  uint16_t frcCorrection;
  uint16_t error = 0;

  // FRC can only be performed when no measurement is running
  error = scd4x.stopPeriodicMeasurement();
  if(error) {
    Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
    Serial.println("FRC could not be performed.");
    provider.completeFRCRequest();
    return;
  }

  error = scd4x.performForcedRecalibration(reference_co2_level, frcCorrection);
  if(error) {
    Serial.print("Error trying to execute performForcedRecalibration(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else {
    Serial.print("FRC performed successfully. Correction value is now at: ");
    Serial.println(frcCorrection);
  }

  provider.completeFRCRequest();

  error = scd4x.startPeriodicMeasurement();
  if (error) {
      Serial.print("Error trying to execute startPeriodicMeasurement(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
  }
  delay(5000);
}
