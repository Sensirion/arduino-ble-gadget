#include "WifiMultiLibraryWrapper.h"

void WifiMultiLibraryWrapper::setSsid(String ssid) {
    _ssid = ssid;
}

void WifiMultiLibraryWrapper::connect(String password) {
    wiFiMulti.addAP(_ssid.c_str(), password.c_str());
    wiFiMulti.run();
}

bool WifiMultiLibraryWrapper::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WifiMultiLibraryWrapper::localIP() {
    return WiFi.localIP().toString();
}