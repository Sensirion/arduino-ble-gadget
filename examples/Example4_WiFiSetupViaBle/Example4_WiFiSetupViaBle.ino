#include "Sensirion_GadgetBle_Lib.h"
#include <WiFi.h>
#include <WiFiMulti.h>
// NOTE: The WiFi library requires quite some space on the ESP32's memory.
// Make sure to decrease the SAMPLE_BUFFER_SIZE_BYTES, to make it fit.
// In sensirion-gadgetble-lib/src/Sensirion_GadgetBle_Lib.h, set the 
// static const size_t SAMPLE_BUFFER_SIZE_BYTES = 30000;

GadgetBle gadgetBle = GadgetBle(GadgetBle::DataType::T_RH_CO2_ALT);

WiFiMulti WiFiMulti;

void onWifiSettingsChanged(std::string ssid, std::string password) {
  Serial.print("WifiSetup: SSID = ");
  Serial.print(ssid.c_str());
  Serial.print(", Password = ");
  Serial.println(password.c_str());

  WiFiMulti.addAP(ssid.c_str(), password.c_str());
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize the GadgetBle Library
  gadgetBle.enableWifiSetupSettings(onWifiSettingsChanged);
  gadgetBle.setCurrentWifiSsid("");

  // Start the GadgetBle Library
  gadgetBle.begin();
  Serial.print("Sensirion GadgetBle Lib initialized with deviceId = ");
  Serial.println(gadgetBle.getDeviceIdString());

  WiFiMulti.addAP("defaultSSID", "defaultPwd");
}

uint16_t t = 0;
uint16_t rh = 0;
uint16_t co2 = 0;

static int64_t lastMmntTime = 0;
static int measurementIntervalMs = 1000;

void loop() {
  if (esp_timer_get_time() - lastMmntTime >= measurementIntervalMs * 1000) {
    Serial.println("Measurement");
    gadgetBle.writeTemperature(++t % 50);
    gadgetBle.writeHumidity(++rh % 100);
    gadgetBle.writeCO2(++co2 % 1000);
    gadgetBle.commit();
    lastMmntTime = esp_timer_get_time();

    
    if (WiFiMulti.run() != WL_CONNECTED) {
      Serial.println("WiFi not connected");
    } else {
      Serial.print("WiFi connected - IP = ");
      Serial.println(WiFi.localIP()); 
    }
  }

  gadgetBle.handleEvents();
  delay(3);
}
