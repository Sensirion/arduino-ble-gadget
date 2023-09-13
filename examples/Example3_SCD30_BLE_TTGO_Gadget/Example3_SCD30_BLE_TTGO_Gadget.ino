// Go to TTGO T-Display's Github Repository
// Download the code as zip, extract it and copy the Folder TFT_eSPI
//  => https://github.com/Xinyuan-LilyGO/TTGO-T-Display/archive/master.zip
// to your Arduino library path
#include <SPI.h>
#include <TFT_eSPI.h>

#include "resources/fonts/ArchivoNarrow_Regular10pt7b.h"
#include "resources/fonts/ArchivoNarrow_Regular50pt7b.h"
#include "resources/fonts/SensirionSimple25pt7b.h"
#include <SensirionI2cScd30.h>

#include "Sensirion_Gadget_BLE.h"

static int64_t lastMeasurementTimeMs = 0;
static int measurementIntervalMs = 1900;

NimBLELibraryWrapper lib;
DataProvider provider(lib, DataType::T_RH_CO2_ALT);

SensirionI2cScd30 sensor;
static char errorMessage[128];
static int16_t error;

// Display related
#define SENSIRION_GREEN 0x6E66
#define sw_version "v1.0"

#define GFXFF 1
#define FF99 &SensirionSimple25pt7b
#define FF90 &ArchivoNarrow_Regular10pt7b
#define FF95 &ArchivoNarrow_Regular50pt7b

TFT_eSPI tft =
    TFT_eSPI(135, 240); // Invoke library, pins defined in User_Setup.h

void displayInit() {
    tft.init();
    tft.setRotation(1);
}

void displaySplashScreen() {
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(SENSIRION_GREEN, TFT_WHITE);

    uint8_t defaultDatum = tft.getTextDatum();
    tft.setTextDatum(1); // Top centre

    tft.setTextSize(1);
    tft.setFreeFont(FF99);
    tft.drawString("B", 120, 40);

    tft.setTextSize(1);
    tft.drawString(sw_version, 120, 90, 2);

    // Revert datum setting
    tft.setTextDatum(defaultDatum);
}

void displayCo2(uint16_t co2) {
    if (co2 > 9999) {
        co2 = 9999;
    }

    tft.fillScreen(TFT_BLACK);

    uint8_t defaultDatum = tft.getTextDatum();

    tft.setTextSize(1);
    tft.setFreeFont(FF90);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.setTextDatum(6); // bottom left
    tft.drawString("CO2", 10, 125);

    tft.setTextDatum(8); // bottom right
    tft.drawString(provider.getDeviceIdString(), 230, 125);

    // Draw CO2 number
    if (co2 >= 1600) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
    } else if (co2 >= 1000) {
        tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    } else {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
    }

    tft.setTextDatum(8); // bottom right
    tft.setTextSize(1);
    tft.setFreeFont(FF95);
    tft.drawString(String(co2), 195, 105);

    // Draw CO2 unit
    tft.setTextSize(1);
    tft.setFreeFont(FF90);
    tft.drawString("ppm", 230, 90);

    // Revert datum setting
    tft.setTextDatum(defaultDatum);
}

void setup() {
    Serial.begin(115200);
    delay(100);

    // Initialize the GadgetBle Library
    provider.begin();
    Serial.print("Sensirion GadgetBle Lib initialized with deviceId = ");
    Serial.println(provider.getDeviceIdString());

    // Initialize the SCD30 driver
    Wire.begin();
    sensor.begin(Wire, SCD30_I2C_ADDR_61);

    sensor.stopPeriodicMeasurement();
    sensor.softReset();
    sensor.activateAutoCalibration(1);

    // Display init and splash screen
    displayInit();
    displaySplashScreen();

    // Start measurements on SCD30
    error = sensor.startPeriodicMeasurement(0);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute startPeriodicMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }

    // Enjoy the splash screen for 2 seconds
    delay(2000);
}

void loop() {
    float co2Concentration = 0.0;
    float temperature = 0.0;
    float humidity = 0.0;
    uint16_t data_ready = 0;

    if (millis() - lastMeasurementTimeMs >= measurementIntervalMs) {

        sensor.getDataReady(data_ready);
        if (bool(data_ready)) {
            error = sensor.readMeasurementData(co2Concentration, temperature,
                                               humidity);
            if (error != NO_ERROR) {
                Serial.print("Error trying to execute readMeasurementData(): ");
                errorToString(error, errorMessage, sizeof errorMessage);
                Serial.println(errorMessage);
                return;
            }

            provider.writeValueToCurrentSample(
                co2Concentration, SignalType::CO2_PARTS_PER_MILLION);
            provider.writeValueToCurrentSample(
                temperature, SignalType::TEMPERATURE_DEGREES_CELSIUS);
            provider.writeValueToCurrentSample(
                humidity, SignalType::RELATIVE_HUMIDITY_PERCENTAGE);
            provider.commitSample();

            lastMeasurementTimeMs = millis();

            // Provide the sensor values for Tools -> Serial Monitor or Serial
            // Plotter
            Serial.print("CO2[ppm]:");
            Serial.print(co2Concentration);
            Serial.print("\t");
            Serial.print("Temperature[℃]:");
            Serial.print(temperature);
            Serial.print("\t");
            Serial.print("Humidity[%]:");
            Serial.println(humidity);

            // display CO2 value
            displayCo2((uint16_t)std::round(co2Concentration));
        }
    }

    provider.handleDownload();
    delay(3);
}
