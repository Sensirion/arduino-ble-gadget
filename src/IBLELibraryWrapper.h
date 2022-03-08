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
#ifndef _I_BLE_LIBRARY_WRAPPER_H_
#define _I_BLE_LIBRARY_WRAPPER_H_

#include "IProviderCallbacks.h"
#include <string>

const char* const GADGET_NAME = "S";

static const char* const DOWNLOAD_SERVICE_UUID =
    "00008000-b38d-4985-720e-0f993a68ee41";
static const char* const SAMPLE_HISTORY_INTERVAL_UUID =
    "00008001-b38d-4985-720e-0f993a68ee41";
static const char* const NUMBER_OF_SAMPLES_UUID =
    "00008002-b38d-4985-720e-0f993a68ee41";
static const char* const DOWNLOAD_PACKET_UUID =
    "00008004-b38d-4985-720e-0f993a68ee41";

static const char* const SETTINGS_SERVICE_UUID =
    "00008100-b38d-4985-720e-0f993a68ee41";
static const char* const WIFI_SSID_UUID =
    "00008171-b38d-4985-720e-0f993a68ee41";
static const char* const WIFI_PWD_UUID = "00008172-b38d-4985-720e-0f993a68ee41";

// abstract class
class IBLELibraryWrapper {
  public:
    virtual ~IBLELibraryWrapper() = default;
    virtual void init() = 0;
    // set device name and manufacturer data
    virtual void setAdvertisingData(const std::string& data) = 0;
    virtual void startAdvertising() = 0;
    virtual void stopAdvertising() = 0;
    virtual std::string getDeviceAddress() = 0;
    virtual void characteristicSetValue(const char* uuid, const uint8_t* data,
                                        size_t size) = 0;
    virtual void characteristicSetValue(const char* uuid, int value);
    virtual std::string characteristicGetValue(const char* uuid) = 0;
    virtual void characteristicNotify(const char* uuid) = 0;
    virtual void
    setProviderCallbacks(IProviderCallbacks* providerCallbacks) = 0;
};

#endif /* _I_BLE_LIBRARY_WRAPPER_H_ */