#include "DataProvider.h"

void DataProvider::onWifiSsidChange(std::string ssid) {
    if (_pWifiLibaray != nullptr) {
        _pWifiLibaray->setSsid(ssid.c_str());
        _wifiSSID = ssid.c_str();
    }
}

void DataProvider::onWifiPasswordChange(std::string pwd) {
    if (_pWifiLibaray != nullptr) {
        _pWifiLibaray->connect(pwd.c_str());
        _wifiPassword = pwd.c_str();
        _wifiSettingsChanged = true;
    }
}