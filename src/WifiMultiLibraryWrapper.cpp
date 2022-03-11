#include "WifiMultiLibraryWrapper.h"

void WifiMultiLibraryWrapper::setSsid(std::string ssid) {
    _ssid = ssid;
}

void WifiMultiLibraryWrapper::connect(std::string password) {
    wiFiMulti.addAP(_ssid.c_str(), password.c_str());
    wiFiMulti.run();
}

bool WifiMultiLibraryWrapper::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

std::string WifiMultiLibraryWrapper::localIP() {
    return WiFi.localIP().toString().c_str();
}