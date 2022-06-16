#include "DataProvider.h"
#include "NimBLELibraryWrapper.h"
#include "WifiMultiLibraryWrapper.h"
// NOTE: The WiFi library requires quite some space on the ESP32's memory.
// Make sure to decrease the SAMPLE_BUFFER_SIZE_BYTES, to make it fit.
// In sensirion-gadgetble-lib/src/Sensirion_GadgetBle_Lib.h, set the 
// static const size_t SAMPLE_BUFFER_SIZE_BYTES = 30000;

NimBLELibraryWrapper lib(true);
WifiMultiLibraryWrapper wifi;
DataProvider provider(lib, DataType::T_RH_CO2_ALT, &wifi);

void setup() {
  Serial.begin(115200);
  delay(100);

  // Start the GadgetBle Library
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
  if (millis() - lastMmntTime >= measurementIntervalMs) {
    Serial.println("Measurement");
    provider.writeValueToCurrentSample(++t % 50, Unit::T);
    provider.writeValueToCurrentSample(++rh % 100, Unit::RH);
    provider.writeValueToCurrentSample(++co2 % 1000, Unit::CO2);
    provider.commitSample();
    lastMmntTime = millis();

    
    if (wifi.isConnected() == false) {
      Serial.println("WiFi not connected");
    } else {
      Serial.print("WiFi connected - IP = ");
      Serial.println(wifi.localIP());
    }
  }

  provider.handleDownload();
  delay(3);
}
