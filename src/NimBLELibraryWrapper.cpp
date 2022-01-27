#include "NimBLELibraryWrapper.h"
#include <NimBLEDevice.h>
#include <NimBLEServer.h>

uint NimBLELibraryWrapper::_numberOfInstances = 0;

struct WrapperPrivateData: public BLECharacteristicCallbacks,
                           BLEServerCallbacks {
    NimBLEAdvertising* pNimBLEAdvertising;
    bool BLEDeviceRunning = false;
    bool deviceConnected = false;

    // BLEServerCallbacks
    void onConnect(BLEServer* serverInst);
    void onDisconnect(BLEServer* serverInst);

    // BLECharacteristicCallbacks
    void onWrite(BLECharacteristic* characteristic);
};

void WrapperPrivateData::onConnect(NimBLEServer* serverInst) {
    deviceConnected = true;
}

void WrapperPrivateData::onDisconnect(BLEServer* serverInst) {
    deviceConnected = false;
}

void WrapperPrivateData::onWrite(BLECharacteristic* characteristic) {
    // todo: download service (upcoming story)
}

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

    _data->pNimBLEAdvertising = NimBLEDevice::getAdvertising();
    // Helps with iPhone connection issues (copy/paste)
    _data->pNimBLEAdvertising->setMinPreferred(0x06);
    _data->pNimBLEAdvertising->setMaxPreferred(0x12);
}

void NimBLELibraryWrapper::setAdvertisingData(const std::string& data) {
    NimBLEAdvertisementData advert;
    advert.setName(GADGET_NAME);
    advert.setManufacturerData(data);
    _data->pNimBLEAdvertising->setAdvertisementData(advert);
}

void NimBLELibraryWrapper::startAdvertising() {
    _data->pNimBLEAdvertising->start();
}

void NimBLELibraryWrapper::stopAdvertising() {
    _data->pNimBLEAdvertising->stop();
}
std::string NimBLELibraryWrapper::getDeviceAddress() {
    return NimBLEDevice::getAddress().toString();
}