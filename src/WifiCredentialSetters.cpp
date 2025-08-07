#include "DataProvider.h"

void DataProvider::onWifiSsidChange(std::string ssid) {
    if (_pWifiLibrary != nullptr) {
        _pWifiLibrary->setSsid(ssid.c_str());
    }
}

void DataProvider::onWifiPasswordChange(std::string pwd) {
    if (_pWifiLibrary != nullptr) {
        _pWifiLibrary->connect(pwd.c_str());
    }
}