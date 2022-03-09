#include "DataProvider.h"

void DataProvider::onWifiSsidChange(std::string ssid) {
    Serial.print("Ssid changed to "); // mock
    Serial.println(ssid.c_str());
    // Call relevant methods of wifi library of choice to change ssid
}

void DataProvider::onWifiPasswordChange(std::string pwd) {
    Serial.println("Password Changed!"); // mock
    // Call relevant methods of wifi library of choice to handle password
    // input. Best not to save it but to directly use the pwd argument
    // to establish a connection.
}