#include "NimBLELibraryWrapper.h"
#include "NimBLEDevice.h"

uint NimBLELibraryWrapper::_numberOfInstances = 0;

struct WrapperPrivateData {
    NimBLEAdvertising* _pNimBLEAdvertising;
    bool BLEDeviceRunning = false;
};

WrapperPrivateData* NimBLELibraryWrapper::_data = nullptr;

NimBLELibraryWrapper::NimBLELibraryWrapper() {
    if (NimBLELibraryWrapper::_numberOfInstances == 0) {
        _data = new WrapperPrivateData();
        ++NimBLELibraryWrapper::_numberOfInstances;
    }
}

NimBLELibraryWrapper::~NimBLELibraryWrapper() {
    if (NimBLELibraryWrapper::_numberOfInstances == 1) {
        delete _data;
        _deinit();
        --NimBLELibraryWrapper::_numberOfInstances;
    }
}

void NimBLELibraryWrapper::_deinit() {
    if (_data->BLEDeviceRunning) {
        NimBLEDevice::deinit(true);
        _data->BLEDeviceRunning = false;
    }
}

void NimBLELibraryWrapper::init() {
    if (_data->BLEDeviceRunning == true) {
        return;
    }
    NimBLEDevice::init(GADGET_NAME);
    _data->BLEDeviceRunning = true;

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