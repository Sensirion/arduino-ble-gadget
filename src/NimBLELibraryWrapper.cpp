#include "NimBLELibraryWrapper.h"
#include "NimBLEDevice.h"

struct WrapperPrivateData {
    NimBLEAdvertising* _pNimBLEAdvertising;
    std::string _deviceName;
};

NimBLELibraryWrapper::~NimBLELibraryWrapper() {
    NimBLEDevice::deinit();
}

void NimBLELibraryWrapper::init(const std::string& deviceName) {
    NimBLEDevice::init(deviceName);
    _data->_deviceName = deviceName;

    _data->_pNimBLEAdvertising = NimBLEDevice::getAdvertising();
    // Helps with iPhone connection issues (copy/paste)
    _data->_pNimBLEAdvertising->setMinPreferred(0x06);
    _data->_pNimBLEAdvertising->setMaxPreferred(0x12);
}

void NimBLELibraryWrapper::setAdvertisingData(const std::string& data) {
    NimBLEAdvertisementData advert;
    advert.setName(_data->_deviceName);
    advert.setManufacturerData(data);
    _data->_pNimBLEAdvertising->setAdvertisementData(advert);
}

void NimBLELibraryWrapper::startAdvertising() {
    _data->_pNimBLEAdvertising->start();
}

void NimBLELibraryWrapper::stopAdvertising() {
    _data->_pNimBLEAdvertising->stop();
}
std::string NimBLELibraryWrapper::getDeviceAddress() {
    return NimBLEDevice::getAddress().toString();
}