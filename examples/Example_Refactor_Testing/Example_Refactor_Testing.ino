#include "DataProvider.h"
#include "NimBLELibraryWrapper.h"

NimBLELibraryWrapper lib;
DataProvider provider(lib);

float mockTemp = 0;
float mockHumi = 0;

void setup(){
    Serial.begin(115200);
    provider.begin();
    Serial.println(offsetof(SampleSlot, offset));
}

void loop(){
    delay(100);
    provider.writeValue(++mockTemp, Unit::T);
    provider.commit();
}