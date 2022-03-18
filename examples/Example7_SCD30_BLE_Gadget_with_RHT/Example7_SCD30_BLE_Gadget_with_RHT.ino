#include <Wire.h>

// Download the SeeedStudio SCD30 Arduino driver here:
//  => https://github.com/Seeed-Studio/Seeed_SCD30/releases/latest
#include "SCD30.h"

#include "DataProvider.h"
#include "NimBLELibraryWrapper.h"

static int64_t lastMeasurementTimeMs = 0;
static int measurementIntervalMs = 1900;

NimBLELibraryWrapper lib;
DataProvider provider(lib, DataType::T_RH_CO2);

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize the GadgetBle Library
  provider.begin();
  Serial.print("Sensirion GadgetBle Lib initialized with deviceId = ");
  Serial.println(provider.getDeviceIdString());

  // Initialize the SCD30 driver
  Wire.begin();
  scd30.initialize();
  scd30.setAutoSelfCalibration(1);
  scd30.setTemperatureOffset(3);
}

void loop() {
  float result[3] = {0};

  if (millis() - lastMeasurementTimeMs >= measurementIntervalMs) {

    if (scd30.isAvailable()) {
      scd30.getCarbonDioxideConcentration(result);

      provider.writeValueToCurrentSample(result[0], Unit::CO2);
      provider.writeValueToCurrentSample(result[1], Unit::T);
      provider.writeValueToCurrentSample(result[2], Unit::RH);

      provider.commitSample();
      lastMeasurementTimeMs = millis();

      // Provide the sensor values for Tools -> Serial Monitor or Serial Plotter
      Serial.print("CO2[ppm]:");
      Serial.print(result[0]);
      Serial.print("\t");
      Serial.print("Temperature[℃]:");
      Serial.print(result[1]);
      Serial.print("\t");
      Serial.print("Humidity[%]:");
      Serial.println(result[2]);
    }
  }

  provider.handleDownload();
  delay(3);
}
