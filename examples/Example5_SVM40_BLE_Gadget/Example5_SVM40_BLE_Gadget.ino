// This code is based on Sensirion's Arduino Snippets
// Check https://github.com/Sensirion/arduino-snippets for the most recent version.

#include "esp_timer.h"
#include "Sensirion_GadgetBle_Lib.h"

#include <Wire.h>

// SVM40
const int16_t SVM40_ADDRESS = 0x6A;

// Temperature offset in degC
static int temperature_offset = -5;

// GadgetBle workflow
static int64_t lastMmntTime = 0;
static int mmntInterval = 1000000;
GadgetBle gadgetBle = GadgetBle(GadgetBle::DataType::T_RH_VOC);

void setup() {
  int16_t t_offset;
  uint8_t data[3], counter;

  Serial.begin(115200);
  // wait for serial connection from PC
  // comment the following line if you'd like the output
  // without waiting for the interface being ready
  while (!Serial);

  // Initialize the GadgetBle Library
  gadgetBle.begin();
  Serial.print("Sensirion GadgetBle Lib initialized with deviceId = ");
  Serial.println(gadgetBle.getDeviceIdString());

  // output format
  Serial.println("VOC_Index\tRH\tT");

  // init I2C
  Wire.begin();

  // wait until sensors startup, > 1 ms according to datasheet
  delay(100);

  // read t offset from flash memory
  Wire.beginTransmission(SVM40_ADDRESS);
  Wire.write(0x60);
  Wire.write(0x14);
  Wire.endTransmission();

  // wait 10 ms to allow the sensor to fill the internal buffer
  delay(10);

  // read offset data, after two bytes a CRC follows
  Wire.requestFrom(SVM40_ADDRESS, 3);
  counter = 0;
  while (Wire.available()) {
    data[counter++] = Wire.read();
  }

  t_offset = (uint16_t)data[0] << 8 | data[1];

  // print value, as all temperature values, this one is also scaled by 200
  Serial.print("Default T Offset: ");
  Serial.print(String(float(t_offset) / 200));
  Serial.println();

  // set t offset to new value (degC) and scale it accordingly by a factor of 200
  t_offset = temperature_offset * 200;

  // prepare buffer with t offset data
  // calculate CRC for each 2 bytes of data
  data[0] = (t_offset & 0xff00) >> 8;
  data[1] = t_offset & 0x00ff;
  data[2] = CalcCrc(data);

  // send new value for t offset to sensor (will be hold in RAM, not persistent)
  Wire.beginTransmission(SVM40_ADDRESS);
  Wire.write(0x60);
  Wire.write(0x14);
  Wire.write(data[0]);
  Wire.write(data[1]);
  Wire.write(data[2]);
  Wire.endTransmission();

  // wait 10 ms to allow the sensor to fill the internal buffer
  delay(10);

  // now send command to save parameter in the flash (NVM memory) of the sensor to have persistence
  Wire.beginTransmission(SVM40_ADDRESS);
  Wire.write(0x60);
  Wire.write(0x02);
  Wire.endTransmission();

  // wait 20 ms to allow the sensor to write the data into the flash
  delay(20);

  // repeat read offset data to make sure that the values are applied correctly
  Wire.beginTransmission(SVM40_ADDRESS);
  Wire.write(0x60);
  Wire.write(0x14);
  Wire.endTransmission();

  // wait 10 ms to allow the sensor to fill the internal buffer
  delay(10);

  // read offset data, after two bytes a CRC follows
  Wire.requestFrom(SVM40_ADDRESS, 3);
  counter = 0;
  while (Wire.available()) {
    data[counter++] = Wire.read();
  }

  t_offset = (uint16_t)data[0] << 8 | data[1];

  Serial.print("New T Offset: ");
  Serial.print(String(float(t_offset) / 200));
  Serial.println();

  // start up sensor, sensor will go to continous measurement mode
  // each second there will be new measurement values
  Wire.beginTransmission(SVM40_ADDRESS);
  Wire.write(0x00);
  Wire.write(0x10);
  Wire.endTransmission();

  // wait until sensors is ready, fan is initialized
  delay(2000);
}

void loop() {
  if (esp_timer_get_time() - lastMmntTime >= mmntInterval) {
    measure_and_report();
  }

  gadgetBle.handleEvents();
  delay(3);
}

void measure_and_report() {
  int16_t voc, humidity, temperature;
  uint8_t data[9], counter;

  // read measurement data
  Wire.beginTransmission(SVM40_ADDRESS);
  Wire.write(0x03);
  Wire.write(0xA6);
  Wire.endTransmission();

  // wait 10 ms to allow the sensor to fill the internal buffer
  delay(10);

  // read measurement data sen44, after two bytes a CRC follows
  Wire.requestFrom(SVM40_ADDRESS, 9);
  counter = 0;
  while (Wire.available()) {
    data[counter++] = Wire.read();
  }

  // VOC level is a signed int and scaled by a factor of 10 and needs to be divided by 10
  // VOC raw value is an uint16_t and has no scaling
  // humidity is a signed int and scaled by 100 and need to be divided by 100
  // temperature is a signed int and scaled by 200 and need to be divided by 200
  voc = (uint16_t)data[0] << 8 | data[1];
  humidity = (uint16_t)data[3] << 8 | data[4];
  temperature = (uint16_t)data[6] << 8 | data[7];

  Serial.print("VOC_Index:");
  Serial.print(String(float(voc) / 10));
  Serial.print("\t");
  Serial.print("RH:");
  Serial.print(String(float(humidity) / 100));
  Serial.print("\t");
  Serial.print("T:");
  Serial.print(String(float(temperature) / 200));
  Serial.println();

  gadgetBle.writeVOC(float(voc) / 10);
  gadgetBle.writeHumidity(float(humidity) / 100);
  gadgetBle.writeTemperature(float(temperature) / 200);

  gadgetBle.commit();
  lastMmntTime = esp_timer_get_time();
}

// calculate CRC according to datasheet
uint8_t CalcCrc(uint8_t data[2]) {
  uint8_t crc = 0xFF;
  for (int i = 0; i < 2; i++) {
    crc ^= data[i];
    for (uint8_t bit = 8; bit > 0; --bit) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ 0x31u;
      } else {
        crc = (crc << 1);
      }
    }
  }
  return crc;
}
