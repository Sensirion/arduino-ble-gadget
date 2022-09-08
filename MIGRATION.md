# Code Migration Guide to Version 1.0.0

This guide is intended to support you in the migration of Arduino sketches using the Sensirion Gadget BLE Lib version 0.14.0 or earlier to the new and improved version 1.0.0.
The necessary information may also be gathered from the updated examples and tutorials, but this should be your one-stop document.

If you are unsure about the notation used here, see the last section for some hints.

## Include Dependencies

The library header file has been renamed.

Concretely, replace
```C++
#include "Sensirion_GadgetBle_Lib.h"
```
by
```C++
#include "Sensirion_Gadget_BLE.h"
```

## Instantiate Objects

The `GadgetBle` object from verion 0.14.0 and below has been renamed to `DataProvider` (as it provides sensor data via BLE to a peripheral, e.g. a Smartphone). On instantiation, the `DataProvider` object requires an instance of a `NimBLELibraryWrapper` object as an argument, so the latter needs to be created first. Don't forget to replace `$YOUR_DATATYPE` by your preferred datatype and mind that `DataType` is now independent of `GadgetBLE`/`DataProvider`.

Replace 
```C++
GadgetBle gadgetBle = GadgetBle(GadgetBle::DataType::$YOUR_DATATYPE);
```
by
```C++
NimBLELibraryWrapper lib;
DataProvider provider(lib, DataType::$YOUR_DATATYPE);
```

*Note:*
*- Names of objects like `provider`/`gadgetBle` or `lib` can be chosen arbitrarily, but we'll stick to these as they are also used in the exampleUsages.*
*- NimBLELibraryWrapper is an implementation of the IBLELibraryWrapper interface for the NimBLE library, which is more lightweight and allows for more features to be added to a gadget. If one wants to use an alternative BLE library, a corresponding wrapper needs to be implemented and used inplace of NimBLEWrapper.*

## Methods
Methods not listed here are unchanged. Just make sure to call them on the `DataProvider` object instead of the `GadgetBle` one.

The various write$SOME_UNIT methods (e.g. `writeTemperature`, `writeHumidity`) are now unified in `writeValueToCurrentSample`. 
This however needs a `Unit` to be specified for its second parameter. 
As before, replace `$YOUR_UNIT` in the snippet below by the unit of the value you want to write.

Replace
```C++
gadgetBle.write$SOME_UNIT(value);
```
by
```C++
provider.writeValueToCurrentSample(value, Unit::$YOUR_UNIT);
```  

The `commit` method has been renamed to `commitSample`:

Replace
```C++
gadgetBle.commit();
```
by
```C++
provider.commitSample();
```  

`handleEvents` has been renamed to `handleDownload`.

Replace
```C++
gadgetBle.handleEvents();
```
by
```C++
provider.handleDownload();
```  

## exampleUsage Changes (optional)

Your code should run with the patches above, but if you adapted and exampleUsage from v0.14.0 or older, consider applying these changes to your code. 

The `esp_timer` dependency is dropped in favor of Arduino's own `millis` function. 

Delete
```C++
#include "esp_timer.h"
```
and replace all instances of 
```C++
esp_timer_get_time()
```
by
```C++
millis()
```

All variables tracking time now use milliseconds exclusively (just like the `millis` function). Values given previously in microseconds need to be reduced by a factor of 1000.

For example, replace
```C++
static int measurementIntervalUs = 1000000;
```
by
```C++
static int measurementIntervalMs = 1000;
```

## WiFi Settings Feature (optional)
It is possible to connect the gadget to WiFi during runtime by setting the WiFi SSID and Password in the **Sensirion MyAmbiance App**.
This functionality, which previously needed to be implemented in the Sketch (see `Example4_WiFiSetupViaBle.ino` from a pre-`1.0.0` version), is now integrated into the library.

To enable the this feature you need to use the provided wrapper for the `WifiMulti` Library.

Add
```C++
#include "WifiMultiLibraryWrapper.h"
```
after the includes at the top of the sketch.

When instantiating the NimBLELibraryWrapper, it's first parameter, `enableWifiSettings`, needs to be set to `True`. `WifiMultiLibraryWrapper` needs to be instantiated and a reference thereof then passed to the `DataProvider` object for the third parameter.

Concretely, the previous snippet 
```C++
NimBLELibraryWrapper lib;
DataProvider provider(lib, DataType::$YOUR_DATATYPE);
```

needs to be replaced by

```C++
NimBLELibraryWrapper lib(true);
WifiMultiLibraryWrapper wifi;
DataProvider provider(lib, DataType::$YOUR_DATATYPE, &wifi);
```
For additional information on how to use this feature in your code, have a look at the updated example `Example4_WiFiSetupViaBl.ino` and the IWifiLibraryWrapper interface.

*Note: As with the BLE library before, the WiFi library is wrapped to allow for alternative libraries to be used inplace. For that the `IWifiLibraryWrapper` needs to be implemented for the library at hand.*

### Some Notes on the Notation in this Document

Each section explains how the library changed and gives concrete code snippets that look like this:

```C++
This is a code snippet
```

Any part of the code snippets starting with `$` followed by capitalized letters is a placeholder variable and needs to be specified for your code to be valid.
So when you see
```C++
writeValue(value, Unit::$UNIT)
```
you need to replace `$UNIT` by `T` for example (`T` for Temperature) and thus write
```C++
writeValue(value, Unit::T)
```
in your code.

