#include "DataProvider.h"

void DataProvider::onWifiSsidChange(std::string ssid) {
    if (_pWifiLibaray != nullptr) {
        _pWifiLibaray->setSsid(ssid.c_str());
    }
    Serial.print("Wifi SSID changed to: ");
    Serial.println(ssid.c_str());
    _wifiSSID = ssid.c_str();
    _wifiSettingsChanged = true;
}

void DataProvider::onWifiPasswordChange(std::string pwd) {
    if (_pWifiLibaray != nullptr) {
        _pWifiLibaray->connect(pwd.c_str());
    }
    Serial.print("Wifi password changed to: ");
    Serial.println(pwd.c_str());
    _wifiPassword = pwd.c_str();
    _wifiSettingsChanged = true;
}