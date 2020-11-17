#include "Sensirion_GadgetBle_Lib.h"

GadgetBle gadgetBle = GadgetBle(GadgetBle::DataType::T_RH_CO2_ALT);

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize the GadgetBle Library
  gadgetBle.begin();
  Serial.print("Sensirion GadgetBle Lib initialized with deviceId = ");
  Serial.println(gadgetBle.getDeviceIdString());
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
  }

  gadgetBle.handleEvents();
  delay(3);
}
