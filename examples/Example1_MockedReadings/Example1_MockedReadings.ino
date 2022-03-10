#include "DataProvider.h"
#include "NimBLELibraryWrapper.h"

#include "esp_timer.h"

NimBLELibraryWrapper lib;
DataProvider provider(lib, DataType::T_RH_CO2_ALT);

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize the GadgetBle Library
  provider.begin();
  Serial.print("Sensirion GadgetBle Lib initialized with deviceId = ");
  Serial.println(provider.getDeviceIdString());
}

uint16_t t = 0;
uint16_t rh = 0;
uint16_t co2 = 0;

static int64_t lastMmntTime = 0;
static int measurementIntervalMs = 1000;

void loop() {
  if (esp_timer_get_time() - lastMmntTime >= measurementIntervalMs * 1000) {
    Serial.println("Measurement");
    provider.writeValueToCurrentSample(++t % 50, Unit::T);
    provider.writeValueToCurrentSample(++rh % 100, Unit::RH);
    provider.writeValueToCurrentSample(++co2 % 1000, Unit::CO2);
    provider.commitSample();
    lastMmntTime = esp_timer_get_time();
  }

  provider.handleDownload();
  delay(3);
}
