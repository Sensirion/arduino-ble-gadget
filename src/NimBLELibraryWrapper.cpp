#include "NimBLELibraryWrapper.h"
#include "NimBLEDevice.h"

struct WrapperPrivateData {
    NimBLEAdvertising* _pNimBLEAdvertising;
};

NimBLELibraryWrapper::~NimBLELibraryWrapper() {
    NimBLEDevice::deinit();
}

void NimBLELibraryWrapper::init() {
    NimBLEDevice::init(GADGET_NAME);

    _data->_pNimBLEAdvertising = NimBLEDevice::getAdvertising();
    // Helps with iPhone connection issues (copy/paste)
    _data->_pNimBLEAdvertising->setMinPreferred(0x06);
    _data->_pNimBLEAdvertising->setMaxPreferred(0x12);
}

void NimBLELibraryWrapper::setAdvertisingData(const std::string& data) {
    NimBLEAdvertisementData advert;
    advert.setName(GADGET_NAME);
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