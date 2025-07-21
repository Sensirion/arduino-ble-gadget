#ifndef _SCD4X_DATA_PROVIDER_H_
#define _SCD4X_DATA_PROVIDER_H_

#include "SCDDataProvider.h"
#include <functional>

class SCD4xDataProvider: public SCDDataProvider {
  public:
    explicit SCD4xDataProvider(IBLELibraryWrapper& libraryWrapper,
                               DataType dataType = T_RH_V3,
                               bool enableWifiSettings = false,
                               bool enableBatteryService = false,
                               IWifiLibraryWrapper* pWifiLibrary = nullptr)
        : SCDDataProvider(libraryWrapper, dataType, enableWifiSettings,
                          enableBatteryService, pWifiLibrary) {};
    void enableASCIntervalCharacteristic(
        std::function<void(std::string)> callbackFunction);
    void enableASCInitIntervalCharacteristic(
        std::function<void(std::string)> callbackFunction);
    void enableASCTargetCharacteristic(
        std::function<void(std::string)> callbackFunction);
    void setASCInitInterval(uint16_t value);
    void setASCInterval(uint16_t value);
    void setASCTarget(uint16_t value);

  private:
    void _setupSCDBLECharacteristics() override;
    bool _enableASCIntervalCharacteristic = false;
    bool _enableASCInitIntervalCharacteristic = false;
    bool _enableASCTargetCharacteristic = false;
};

#endif /* _SCD4X_DATA_PROVIDER_H_ */
