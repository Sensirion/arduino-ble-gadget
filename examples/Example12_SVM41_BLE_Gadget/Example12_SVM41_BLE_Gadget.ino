/*
 * Copyright (c) 2022, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <Arduino.h>
#include <SensirionI2CSvm41.h>
#include <Wire.h>

#include "DataProvider.h"
#include "NimBLELibraryWrapper.h"

SensirionI2CSvm41 svm41;

// GadgetBle workflow
static int64_t lastMeasurementTimeMs = 0;
static int measurementIntervalMs = 1000;
NimBLELibraryWrapper lib;
DataProvider provider(lib, DataType::T_RH_VOC_NOX);

void setup() {

    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }

    // Initialize the GadgetBle Library
    provider.begin();
    Serial.print("Sensirion GadgetBle Lib initialized with deviceId = ");
    Serial.println(provider.getDeviceIdString());

    Wire.begin();

    uint16_t error;
    char errorMessage[256];

    svm41.begin(Wire);

    error = svm41.deviceReset();
    if (error) {
        Serial.print("Error trying to execute deviceReset(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }

    // Start Measurement
    error = svm41.startMeasurement();
    if (error) {
        Serial.print("Error trying to execute startMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }
}

void loop() {
  if (millis() - lastMeasurementTimeMs >= measurementIntervalMs) {
    measure_and_report();
  }

  provider.handleDownload();
  delay(3);
}

void measure_and_report() {
    uint16_t error;
    char errorMessage[256];

    // Read Measurement
    float humidity;
    float temperature;
    float vocIndex;
    float noxIndex;
    error = svm41.readMeasuredValues(humidity, temperature, vocIndex, noxIndex);
    if (error) {
        Serial.print("Error trying to execute readMeasuredValues(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        Serial.print("Humidity:");
        Serial.print(humidity);
        Serial.print("\t");
        Serial.print("Temperature:");
        Serial.print(temperature);
        Serial.print("\t");
        Serial.print("VocIndex:");
        Serial.print(vocIndex);
        Serial.print("\t");
        Serial.print("NoxIndex:");
        Serial.println(noxIndex);
    }

    provider.writeValueToCurrentSample(temperature, Unit::T);
    provider.writeValueToCurrentSample(humidity, Unit::RH);
    provider.writeValueToCurrentSample(vocIndex, Unit::VOC);
    provider.writeValueToCurrentSample(noxIndex, Unit::NOX);

    provider.commitSample();
    lastMeasurementTimeMs = millis();
}
