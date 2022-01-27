#include "NimBLELibraryWrapper.h"
#include <NimBLEDevice.h>
#include <NimBLEServer.h>

uint NimBLELibraryWrapper::_numberOfInstances = 0;

struct WrapperPrivateData: public BLECharacteristicCallbacks,
                           BLEServerCallbacks {
    NimBLEAdvertising* pNimBLEAdvertising;
    bool BLEDeviceRunning = false;
    bool deviceConnected = false;

    // BLEServer
    NimBLEServer* pBLEServer;

    // BLEDownloadService
    NimBLEService* pBLEDownloadService;

    // BLECharacteristics
    NimBLECharacteristic* pTransferChararacteristic;
    NimBLECharacteristic* pNumberOfSamplesCharacteristic;
    NimBLECharacteristic* pSampleHistoryIntervalCharacteristic;

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

    // Initialize BLEServer
    _data->pBLEServer =
        NimBLEDevice::createServer(); // NimBLEDevice has ownership
    _data->pBLEServer->setCallbacks(_data);

    // Create Dwownload Service
    _createDownloadService();
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

void NimBLELibraryWrapper::_createDownloadService() {
    // Create service
    _data->pBLEDownloadService =
        _data->pBLEServer->createService(DOWNLOAD_SERVICE_UUID);

    // Create characteristics
    _data->pNumberOfSamplesCharacteristic =
        _data->pBLEDownloadService->createCharacteristic(NUMBER_OF_SAMPLES_UUID,
                                                         NIMBLE_PROPERTY::READ);
    _data->pNumberOfSamplesCharacteristic->setValue(0);

    _data->pSampleHistoryIntervalCharacteristic =
        _data->pBLEDownloadService->createCharacteristic(
            SAMPLE_HISTORY_INTERVAL_UUID,
            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    _data->pSampleHistoryIntervalCharacteristic->setCallbacks(_data);

    _data->pTransferChararacteristic =
        _data->pBLEDownloadService->createCharacteristic(
            DOWNLOAD_PACKET_UUID, NIMBLE_PROPERTY::NOTIFY);
    _data->pTransferChararacteristic->setCallbacks(_data);

    _data->pBLEDownloadService->start();
}