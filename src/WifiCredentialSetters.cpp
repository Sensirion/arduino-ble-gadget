#include "DataProvider.h"

void DataProvider::onWifiSsidChange(std::string ssid) {
    if (_pWifiLibaray != nullptr) {
        _pWifiLibaray->setSsid(ssid);
    }
}

void DataProvider::onWifiPasswordChange(std::string pwd) {
    if (_pWifiLibaray != nullptr) {
        _pWifiLibaray->connect(pwd);
    }
}