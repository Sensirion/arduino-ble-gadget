#include "NimBLELibraryWrapper.h"

void NimBLELibraryWrapper::init(const std::string& deviceName) {
    NimBLEDevice::init(deviceName);
    _deviceName = deviceName;

    _pNimBLEAdvertising = NimBLEDevice::getAdvertising();
    // Helps with iPhone connection issues (copy/paste)
    _pNimBLEAdvertising->setMinPreferred(0x06);
    _pNimBLEAdvertising->setMaxPreferred(0x12);
}

void NimBLELibraryWrapper::setAdvertisingData(const std::string& data) {
    NimBLEAdvertisementData advert;
    advert.setName(_deviceName);
    advert.setManufacturerData(data);
    _pNimBLEAdvertising->setAdvertisementData(advert);
}

void NimBLELibraryWrapper::startAdvertising() {
    _pNimBLEAdvertising->start();
}

void NimBLELibraryWrapper::stopAdvertising() {
    _pNimBLEAdvertising->stop();
}
std::string NimBLELibraryWrapper::getDeviceAddress() {
    return NimBLEDevice::getAddress().toString();
}