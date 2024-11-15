#include "AdvertisementHeader.h"

void AdvertisementHeader::writeCompanyId(uint16_t companyID) {
    _write16BitLittleEndian(companyID, 0);
}

void AdvertisementHeader::writeSensirionAdvertisementType(uint8_t advType) {
    _writeByte(advType, 2);
}

void AdvertisementHeader::writeSampleType(uint8_t sampleType) {
    _writeByte(sampleType, 3);
}

void AdvertisementHeader::writeDeviceId(uint8_t deviceIDHigh,
                                        uint8_t deviceIDLow) {
    _writeByte(deviceIDHigh, 4);
    _writeByte(deviceIDLow, 5);
}
