#ifndef _SCD_DATA_PROVIDER_H_
#define _SCD_DATA_PROVIDER_H_

#include "DataProvider.h"
#include <functional>

class SCD4xDataProvider;

class SCDDataProvider: public DataProvider {
  public:
    explicit SCDDataProvider(IBLELibraryWrapper& libraryWrapper,
                             DataType dataType = T_RH_V3,
                             bool enableWifiSettings = false,
                             bool enableBatteryService = false,
                             IWifiLibraryWrapper* pWifiLibrary = nullptr)
        : DataProvider(libraryWrapper, dataType, enableWifiSettings,
                       enableBatteryService, false, pWifiLibrary) {};

    void enableMeasurementIntervalCharacteristic(
        std::function<void(std::string)> callbackFunction);
    void enableTempOffsetCharacteristic(
        std::function<void(std::string)> callbackFunction);
    void enableAltitudeCharacteristic(
        std::function<void(std::string)> callbackFunction);
    void enableForcedRecalibrationCharacteristic(
        std::function<void(std::string)> callbackFunction);
    void
    enableASCCharacteristic(std::function<void(std::string)> callbackFunction);
    void setASCStatus(uint16_t value);
    void setMeasurementInterval(uint16_t value);
    void setTempOffset(uint16_t value);
    void setAltitude(uint16_t value);

  private:
    friend class SCD4xDataProvider;
    virtual void _setupSCDBLECharacteristics();
    void _setupBLEServices() override;
    bool _enableAltitudeCharacteristic = false;
    bool _enableTempOffsetCharacteristic = false;
    bool _enableMeasurementIntervalCharacteristic = false;
    bool _enableForcedRecalibrationCharacteristic = false;
    bool _enableASCCharacteristic = false;
    bool _enableSCDService = false;
};

#endif /* _SCD_DATA_PROVIDER_H_ */