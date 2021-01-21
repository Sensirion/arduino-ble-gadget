# Sensirion Gadget BLE Lib

This library enables you to create your own Do-It-Yourself BLE enabled sensor gadget on the ESP32 platform. 

### Key features are

* Live sensor value broadcasting without requiring a BLE connection
* Simultaneous monitoring of sensor data from multiple BLE clients
* **Sensirion MyAmbience CO2** App compatibility - allows you to monitor your gadgets, plot the sensor values and share/export the data.
* Buffering of sensor data for later download (sensor history download)

**Please see the examples provided as reference on how to use this library.**

Important points to keep in mind:

* `writeXX` writes the corresponding sensor values to the library. But the values are not published to the BLE stack until `commit` has been called
* Keep the loop delay at 3ms, to allow the library to provide history data to the smart phone applications and other BLE clients nearby. Increasing this delay will slow down the download process.
* To allow history data downloads, you need to have the `handleEvents` function within the loop.

### Recommended Hardware

This project was developed and tested on Espressif [ESP32 DevKitC](https://www.espressif.com/en/products/devkits/esp32-devkitc) hardware (see e.g. [ESP32-DevKitC-32D](https://www.digikey.com/en/products/detail/espressif-systems/ESP32-DEVKITC-32D/9356990))

### Mobile App Support

Download the **Sensirion MyAmbience CO2** App to monitor your gadget, plot the sensor values and share/export the data:

* [Download for Android](https://play.google.com/store/apps/details?id=com.sensirion.myam)
* [Download for iOS](https://apps.apple.com/ch/app/sensirion-myambience-co2/id1529131572) 

## Getting Started

1. Download Arduino IDE and setup the environment for ESP32 platform
	* Follow [this guide](https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md)
	* Detailed Instructions for advanced users: [Arduino-ESP32](https://github.com/espressif/arduino-esp32)
2. Connect your ESP32 DevKit to your computer and launch the Arduino IDE
3. In the Arduino IDE, make sure you have the `ESP32 Dev Module` selected and the PSRAM `Disabled`
4. Download the library's release zip and unpack it into your [Arduino's library path](https://www.arduino.cc/en/Guide/Libraries#importing-a-zip-library).
	* Download the latest release
	* [How to import a zip library](https://www.arduino.cc/en/Guide/Libraries#importing-a-zip-library)
5. Select File -> Examples -> Sensirion Gadget BLE Lib -> **Example 1**
6. Click the upload button


### Create your own CO2 Monitor

Follow the [instructions in this tutorial](documents/SCD30_Monitor_Tutorial.md).


## Repository Contents

* **/examples** - Example sketches for the library (.ino). Run these from the Arduino IDE. 
* **/src** - Source files for the library (.cpp, .h).
* **library.properties** - General library properties for the Arduino package manager. 


## License

See [LICENSE](LICENSE.txt).
