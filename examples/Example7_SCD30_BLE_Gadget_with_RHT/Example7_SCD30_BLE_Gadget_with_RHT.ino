#include "esp_timer.h"
#include <Wire.h>

// Download the SeeedStudio SCD30 Arduino driver here:
//  => https://github.com/Seeed-Studio/Seeed_SCD30/releases/latest
#include "SCD30.h"

#include "Sensirion_GadgetBle_Lib.h"

#define SDA_pin 26  // Define the SDA pin used for the SCD30
#define SCL_pin 27  // Define the SCL pin used for the SCD30

static int64_t lastMmntTime = 0;
static int startCheckingAfterUs = 1900000;

GadgetBle gadgetBle = GadgetBle(GadgetBle::DataType::T_RH_CO2);

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize the GadgetBle Library
  gadgetBle.begin();
  Serial.print("Sensirion GadgetBle Lib initialized with deviceId = ");
  Serial.println(gadgetBle.getDeviceIdString());

  // Initialize the SCD30 driver
  Wire.begin(SDA_pin, SCL_pin);
  scd30.initialize();
  scd30.setAutoSelfCalibration(1);
  scd30.setTemperatureOffset(3);
}

void loop() {
  float result[3] = {0};

  if (esp_timer_get_time() - lastMmntTime >= startCheckingAfterUs) {

    if (scd30.isAvailable()) {
      scd30.getCarbonDioxideConcentration(result);

      gadgetBle.writeCO2(result[0]);
      gadgetBle.writeTemperature(result[1]);
      gadgetBle.writeHumidity(result[2]);

      gadgetBle.commit();
      lastMmntTime = esp_timer_get_time();

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

  gadgetBle.handleEvents();
  delay(3);
}
