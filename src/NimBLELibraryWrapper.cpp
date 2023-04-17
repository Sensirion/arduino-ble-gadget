#include "NimBLELibraryWrapper.h"
#include <NimBLEDevice.h>
#include <NimBLEServer.h>

uint NimBLELibraryWrapper::_numberOfInstances = 0;

struct WrapperPrivateData: public BLECharacteristicCallbacks,
                           BLEServerCallbacks {
    NimBLEAdvertising* pNimBLEAdvertising;
    bool BLEDeviceRunning = false;

    // owned by NimBLE
    NimBLEServer* pBLEServer;
    NimBLEService* services[MAX_NUMBER_OF_SERVICES] = {nullptr};
    NimBLECharacteristic* characteristics[MAX_NUMBER_OF_CHARACTERISTICS] = {
        nullptr};

    // BLEServerCallbacks
    void onConnect(BLEServer* serverInst);
    void onDisconnect(BLEServer* serverInst);

    // BLECharacteristicCallbacks
    void onWrite(BLECharacteristic* characteristic);
    void onSubscribe(NimBLECharacteristic* pCharacteristic,
                     ble_gap_conn_desc* desc, uint16_t subValue);

    // DataProvider Callbacks
    IProviderCallbacks* providerCallbacks = nullptr;
};

void WrapperPrivateData::onConnect(NimBLEServer* serverInst) {
    if (providerCallbacks != nullptr) {
        providerCallbacks->onConnectionEvent();
    }
}

void WrapperPrivateData::onDisconnect(BLEServer* serverInst) {
    if (providerCallbacks != nullptr) {
        providerCallbacks->onConnectionEvent();
    }
}

void WrapperPrivateData::onSubscribe(NimBLECharacteristic* pCharacteristic,
                                     ble_gap_conn_desc* desc,
                                     uint16_t subValue) {
    if ((providerCallbacks != nullptr) && (subValue == 1)) {
        providerCallbacks->onDownloadRequest();
    }
}

void WrapperPrivateData::onWrite(BLECharacteristic* characteristic) {
    if (characteristic->getUUID().toString().compare(
            SAMPLE_HISTORY_INTERVAL_UUID) == 0) {
        std::string value = characteristic->getValue();
        uint32_t sampleIntervalMs =
            value[0] | (value[1] << 8) | (value[2] << 16) | (value[3] << 24);
        if (providerCallbacks != nullptr) {
            providerCallbacks->onHistoryIntervalChange(sampleIntervalMs);
        }
    } else if (characteristic->getUUID().toString().compare(WIFI_SSID_UUID) ==
               0) {
        providerCallbacks->onWifiSsidChange(characteristic->getValue());
    } else if (characteristic->getUUID().toString().compare(WIFI_PWD_UUID) ==
               0) {
        providerCallbacks->onWifiPasswordChange(characteristic->getValue());
    }
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

void NimBLELibraryWrapper::createServer() {
    _data->pBLEServer = NimBLEDevice::createServer();
    _data->pBLEServer->setCallbacks(_data);
}

bool NimBLELibraryWrapper::createService(const char* uuid) {
    for (int i = 0; i < MAX_NUMBER_OF_SERVICES; ++i) {
        if (_data->services[i] == nullptr) {
            _data->services[i] = _data->pBLEServer->createService(uuid);
            return true;
        }
    }
    return false; // no space in services[]
}

bool NimBLELibraryWrapper::createCharacteristic(const char* serviceUuid,
                                                const char* characteristicUuid,
                                                Permission permission) {
    NimBLEService* service = _lookupService(serviceUuid);
    if (service == nullptr) { // invalid service uuid
        return false;
    }
    for (int i = 0; i < MAX_NUMBER_OF_CHARACTERISTICS; ++i) {
        if (_data->characteristics[i] == nullptr) {
            switch (permission) {
                case (Permission::READWRITE_PERMISSION):
                    _data->characteristics[i] =
                        service->createCharacteristic(characteristicUuid);
                    _data->characteristics[i]->setCallbacks(_data);
                    return true;
                case (Permission::READ_PERMISSION):
                    _data->characteristics[i] = service->createCharacteristic(
                        characteristicUuid, NIMBLE_PROPERTY::READ);
                    return true;
                case (Permission::WRITE_PERMISSION):
                    _data->characteristics[i] = service->createCharacteristic(
                        characteristicUuid, NIMBLE_PROPERTY::WRITE);
                    _data->characteristics[i]->setCallbacks(_data);
                    return true;
                case (Permission::NOTIFY_PERMISSION):
                    _data->characteristics[i] = service->createCharacteristic(
                        characteristicUuid, NIMBLE_PROPERTY::NOTIFY);
                    return true;
                default:
                    return false;
            }
        }
    }
    return false; // no space in characteristics[]
}

bool NimBLELibraryWrapper::startService(const char* uuid) {
    NimBLEService* service = _lookupService(uuid);
    if (service == nullptr) {
        return false;
    }
    bool success = service->start();
    return success;
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

bool NimBLELibraryWrapper::characteristicSetValue(const char* uuid,
                                                  const uint8_t* data,
                                                  size_t size) {
    NimBLECharacteristic* pCharacteristic = _lookupCharacteristic(uuid);
    if (nullptr == pCharacteristic) {
        return false;
    }
    pCharacteristic->setValue(data, size);
    return true;
}

bool NimBLELibraryWrapper::characteristicSetValue(const char* uuid, int value) {
    NimBLECharacteristic* pCharacteristic = _lookupCharacteristic(uuid);
    if (nullptr == pCharacteristic) {
        return false;
    }
    pCharacteristic->setValue(value);
    return true;
}

std::string NimBLELibraryWrapper::characteristicGetValue(const char* uuid) {
    NimBLECharacteristic* pCharacteristic = _lookupCharacteristic(uuid);
    if (nullptr == pCharacteristic) {
        return "";
    }
    return pCharacteristic->getValue();
}

bool NimBLELibraryWrapper::characteristicNotify(const char* uuid) {
    NimBLECharacteristic* pCharacteristic = _lookupCharacteristic(uuid);
    if (nullptr == pCharacteristic) {
        return false;
    }
    pCharacteristic->notify(true);
    return true;
}

void NimBLELibraryWrapper::setProviderCallbacks(
    IProviderCallbacks* providerCallbacks) {
    _data->providerCallbacks = providerCallbacks;
}

NimBLECharacteristic*
NimBLELibraryWrapper::_lookupCharacteristic(const char* uuid) {
    // NimBLECharacteristic* pCharacteristic = nullptr;
    for (int i = 0; i < MAX_NUMBER_OF_CHARACTERISTICS; ++i) {
        if (_data->characteristics[i] == nullptr) {
            continue;
        }
        if (strcmp(_data->characteristics[i]->getUUID().toString().c_str(),
                   uuid) == 0) {
            return _data->characteristics[i];
        }
    }
    return nullptr;
}

NimBLEService* NimBLELibraryWrapper::_lookupService(const char* uuid) {
    for (int i = 0; i < MAX_NUMBER_OF_SERVICES; ++i) {
        if (_data->services[i] == nullptr) {
            continue;
        }
        if (strcmp(_data->services[i]->getUUID().toString().c_str(), uuid) ==
            0) {
            return _data->services[i];
        }
    }
    return nullptr;
}