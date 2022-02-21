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
#include "Config.h"
#include "Download.h"
#include "IBLELibraryWrapper.h"
#include "Sample.h"
#include "SampleHistoryRingBuffer.h"

class DataProvider {
  public:
    explicit DataProvider(IBLELibraryWrapper& libraryWrapper,
                          DataType dataType = T_RH_V3)
        : _BLELibrary(libraryWrapper),
          _sampleConfig(sampleConfigSelector.at(dataType)){};
    void begin();
    void writeValueToCurrentSample(float value, Unit unit);
    void commitSample();
    // void handleEvents();
    void setSampleConfig(DataType dataType);

  private:
    std::string _buildAdvertisementData();
    DownloadPacket _buildDownloadPacket();
    // void _handleDownload();
    // void _saveSampleToHistoryBuffer();
    IBLELibraryWrapper& _BLELibrary;
    Sample _currentSample;
    AdvertisementHeader _advertisementHeader;
    SampleHistoryRingBuffer _sampleHistory;
    int _sampleHistoryIndex;
    int _downloadSequenceIdx = 0; // first packet is the header

    SampleConfig _sampleConfig;
    uint64_t _historyIntervalMilliSeconds = 60000; // = 10 minutes
};

#endif /* _DATA_PROVIDER_H_ */