/*
 * Copyright (c) 2022, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _NIM_BLE_LIBRARY_WRAPPER_H_
#define _NIM_BLE_LIBRARY_WRAPPER_H_

#include "IBLELibraryWrapper.h"
#include <NimBLECharacteristic.h>

struct WrapperPrivateData;

class NimBLELibraryWrapper: public IBLELibraryWrapper {
  public:
    /**
     * @brief Construct a new Nim BLE Library Wrapper
     * @note To allow the client, e.g. MyAmbience app, to discover a change in
     * the provided services switch Bluetooth off and then on again on your
     * client device.
     *
     * @param enableWifiSettings if true, wifi settings service will be
     * initialized
     * @param enableBatteryService if true, battery level service will be
     * initilized
     */
    NimBLELibraryWrapper();
    NimBLELibraryWrapper(const NimBLELibraryWrapper& other) = delete;
    NimBLELibraryWrapper& operator=(const NimBLELibraryWrapper& other) = delete;
    virtual ~NimBLELibraryWrapper();
    void init() override;
    void createServer() override;
    bool createService(const char* uuid) override;
    bool createCharacteristic(const char* serviceUuid,
                              const char* characteristicUuid,
                              Permission permission) override;
    bool startService(const char* uuid) override;
    void setAdvertisingData(const std::string& data) override;
    void startAdvertising() override;
    void stopAdvertising() override;
    std::string getDeviceAddress() override;
    bool characteristicSetValue(const char* uuid, const uint8_t* data,
                                size_t size) override;
    bool characteristicSetValue(const char* uuid, int value) override;
    std::string characteristicGetValue(const char* uuid) override;
    bool characteristicNotify(const char* uuid) override;
    void setProviderCallbacks(IProviderCallbacks* providerCallbacks) override;

  private:
    void _deinit();
    void _createDownloadService();
    void _createSettingsService();
    void _createBatteryService();
    NimBLECharacteristic* _lookupCharacteristic(const char* uuid);
    NimBLEService* _lookupService(const char* uuid);

    static WrapperPrivateData* _data;
    static uint _numberOfInstances;
};

#endif /* _NIM_BLE_LIBRARY_WRAPPER_H_ */