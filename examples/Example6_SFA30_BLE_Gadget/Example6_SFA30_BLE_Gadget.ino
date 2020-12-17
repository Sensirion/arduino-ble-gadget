// This code is based on Sensirion's Arduino Snippets
// Check https://github.com/Sensirion/arduino-snippets for the most recent version.

#include "esp_timer.h"
#include "Sensirion_GadgetBle_Lib.h"

#include <Wire.h>

// SFA30
const int16_t SFA_ADDRESS = 0x5D;

// GadgetBle workflow
static int64_t lastMmntTime = 0;
static int mmntInterval = 1000000;
GadgetBle gadgetBle = GadgetBle(GadgetBle::DataType::T_RH_HCHO);

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

  // wait until sensor is ready
  delay(10);
  
  // start SFA measurement in periodic mode, will update every 0.5 s
  Wire.beginTransmission(SFA_ADDRESS);
  Wire.write(0x00);
  Wire.write(0x06);
  Wire.endTransmission();

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
  float hcho, temperature, humidity;
  uint8_t data[9], counter;

  // send read data command
  Wire.beginTransmission(SFA_ADDRESS);
  Wire.write(0x03);
  Wire.write(0x27);
  Wire.endTransmission();

  //wait time before reading for the values should be more than 2ms
  delay(10);
  
  // read measurement data: 
  // 2 bytes formaldehyde, 1 byte CRC, scale factor 5
  // 2 bytes RH, 1 byte CRC, scale factor 100
  // 2 bytes T, 1 byte CRC, scale factor 200
  // stop reading after 9 bytes (not used)
  Wire.requestFrom(SFA_ADDRESS, 9);
  counter = 0;
  while (Wire.available()) {
    data[counter++] = Wire.read();
  }
  
  // floating point conversion according to datasheet
  hcho = (float)((int16_t)data[0] << 8 | data[1])/5;
  // convert RH in %
  humidity = (float)((int16_t)data[3] << 8 | data[4])/100;
  // convert T in degC
  temperature = (float)((int16_t)data[6] << 8 | data[7])/200;

  Serial.print(hcho);
  Serial.print("\t");
  Serial.print(temperature);
  Serial.print("\t");
  Serial.print(humidity);
  Serial.println();

  gadgetBle.writeHCHO(hcho);
  gadgetBle.writeHumidity(humidity);
  gadgetBle.writeTemperature(temperature);

  gadgetBle.commit();
  lastMmntTime = esp_timer_get_time();
}
