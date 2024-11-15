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
#ifndef _DATA_PROVIDER_H_
#define _DATA_PROVIDER_H_

#include "AdvertisementHeader.h"
#include "Download.h"
#include "IBLELibraryWrapper.h"
#include "IProviderCallbacks.h"
#include "IWifiLibraryWrapper.h"
#include "SampleHistoryRingBuffer.h"
#include "Sensirion_UPT_Core.h"
#include <string>

class DataProvider: public IProviderCallbacks {
  public:
    explicit DataProvider(IBLELibraryWrapper& libraryWrapper,
                          DataType dataType = T_RH_V3,
                          bool enableWifiSettings = false,
                          bool enableBatteryService = false,
                          bool enableFRCService = false,
                          IWifiLibraryWrapper* pWifiLibrary = nullptr)
        : _BLELibrary(libraryWrapper), _enableWifiSettings(enableWifiSettings),
          _enableBatteryService(enableBatteryService),
          _enalbeFRCService(enableFRCService),
          _enableAltDeviceName(false),
          _sampleConfig(sampleConfigSelector.at(dataType)),
          _pWifiLibaray(pWifiLibrary){};
    ~DataProvider(){};
    void begin();
    void writeValueToCurrentSample(float value, SignalType signalType);
    void commitSample();
    void handleDownload();
    void setBatteryLevel(int value);
    void setSampleConfig(DataType dataType);
    String getDeviceIdString() const;
    bool isDownloading() const;
    bool isFRCRequested() const;
    uint32_t getReferenceCO2Level() const;
    void completeFRCRequest();
    /*
     * enableAltDeviceName must be called before begin() to ensure
     * the characteristic is created.
     * Initially the alternative device name is empty.
     * Use setAltDeviceName to change the device name.
     */
    void enableAltDeviceName();
    std::string getAltDeviceName();
    void setAltDeviceName(std::string altDeviceName);

    private: 
    std::string _buildAdvertisementData();
    DownloadHeader _buildDownloadHeader();
    DownloadPacket _buildDownloadPacket();
    int _numberOfPacketsRequired(int numberOfSamples) const;
    IBLELibraryWrapper& _BLELibrary;
    void _setupBLEInfrastructure();
    Sample _currentSample;
    AdvertisementHeader _advertisementHeader;
    SampleHistoryRingBuffer _sampleHistory;
    int _nrOfSamplesRequested = 0;
    DownloadState _downloadState = INACTIVE;
    int _downloadSequenceIdx = 0; // first packet is the header
    int _numberOfSamplesToDownload = 0;
    int _numberOfSamplePacketsToDownload = 0;
    bool _frc_requested = false;
    uint32_t _reference_co2_level = 0;
    std::string _altDeviceName = "";

    bool _enableWifiSettings;
    bool _enableBatteryService;
    bool _enalbeFRCService;
    bool _enableAltDeviceName;

    SampleConfig _sampleConfig;
    uint64_t _historyIntervalMilliSeconds = 600000; // = 10 minutes
    uint64_t _latestHistoryTimeStamp = 0;
    uint64_t _latestHistoryTimeStampAtDownloadStart = 0;
    IWifiLibraryWrapper* _pWifiLibaray;

    // ProviderCallbacks
    void onHistoryIntervalChange(int interval) override;
    void onConnectionEvent() override;
    void onDownloadRequest() override;
    void onWifiSsidChange(std::string ssid) override;
    void onWifiPasswordChange(std::string pwd) override;
    void onFRCRequest(uint16_t reference_co2_level) override;
    void onNrOfSamplesRequest(int nr_of_samples) override;
    void onAltDeviceNameChange(std::string altDeviceName) override;
};

#endif /* _DATA_PROVIDER_H_ */