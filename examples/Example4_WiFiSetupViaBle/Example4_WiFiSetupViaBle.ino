#include "Sensirion_Gadget_BLE.h"
#include "WifiMultiLibraryWrapper.h"

// NOTE: The WiFi library requires quite some space on the ESP32's memory.
// Make sure the SAMPLE_BUFFER_SIZE_BYTES is low enough to make it fit.
// In sensirion-BLE Gadget-lib/src/SampleHistoryRingBuffer.h, set the 
// static const size_t SAMPLE_BUFFER_SIZE_BYTES = 30000 (default);

NimBLELibraryWrapper lib;
WifiMultiLibraryWrapper wifi;
DataProvider provider(lib, DataType::T_RH_CO2_ALT, true, false, &wifi);

void setup() {
  Serial.begin(115200);
  delay(1000); // Wait for Serial monitor to start

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
    provider.writeValueToCurrentSample(++t % 50, SignalType::TEMPERATURE_DEGREES_CELSIUS);
    provider.writeValueToCurrentSample(++rh % 100, SignalType::RELATIVE_HUMIDITY_PERCENTAGE);
    provider.writeValueToCurrentSample(++co2 % 1000, SignalType::CO2_PARTS_PER_MILLION);
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
