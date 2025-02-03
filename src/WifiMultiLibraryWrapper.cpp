#include "WifiMultiLibraryWrapper.h"

void WifiMultiLibraryWrapper::setSsid(String ssid) {
    _ssid = ssid;
}

void WifiMultiLibraryWrapper::connect(String password) {
    wiFiMulti.addAP(_ssid.c_str(), password.c_str());
    wiFiMulti.run();

    preferences.begin("wifiCreds", false);
    preferences.putString("ssid", _ssid);
    preferences.putString("pass", password);
    preferences.end();
}

void WifiMultiLibraryWrapper::loadCredentials() {
    preferences.begin("wifiCreds", true);
    String ssid = preferences.getString("ssid", "");
    String pass = preferences.getString("pass", "");
    preferences.end();

    if (ssid != "") {
        wiFiMulti.addAP(ssid.c_str(), pass.c_str());
        wiFiMulti.run(1000);
    }
}

bool WifiMultiLibraryWrapper::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WifiMultiLibraryWrapper::localIP() {
    return WiFi.localIP().toString();
}