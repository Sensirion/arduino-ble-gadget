#include "DataProvider.h"
#include "NimBLELibraryWrapper.h"

NimBLELibraryWrapper lib;
DataProvider provider(lib);

float mockTemp = 0;
float mockHumi = 0;
uint64_t lastCommitTimeMS = 0;
uint64_t commitIntervalMS = 2000; 

void setup(){
    Serial.begin(115200);
    provider.begin();
}

void loop(){
    delay(10);
    uint64_t now = millis();
    if (now - lastCommitTimeMS >= commitIntervalMS) {
        provider.writeValueToCurrentSample(++mockTemp, Unit::T);
        provider.commitSample();
        lastCommitTimeMS = now;
    }
    provider.handleDownload();
}