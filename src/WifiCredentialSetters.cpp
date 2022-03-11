#include "DataProvider.h"

void DataProvider::onWifiSsidChange(std::string ssid) {
    Serial.print("Ssid changed to ");
    Serial.println(ssid.c_str());
    if (_pWifiLibaray != nullptr) {
        _pWifiLibaray->setSsid(ssid);
    }
}

void DataProvider::onWifiPasswordChange(std::string pwd) {
    Serial.println("Connecting to wifi using given password");
    if (_pWifiLibaray != nullptr) {
        _pWifiLibaray->connect(pwd);
    }
}