# Sensirion Gadget BLE Lib

This library enables you to create your own Do-It-Yourself BLE enabled sensor gadget on the ESP32 platform. 

### Key features are

* Live sensor value broadcasting wihtout requiring a BLE connection
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

1. Same as above, but open **Example 2**
2. Click the link in the top import statement and install the SCD30 library
	* [How to import a zip library](https://www.arduino.cc/en/Guide/Libraries#importing-a-zip-library)
3. Connect your CO2 sensor to the ESP32 (you can also choose other pins for SDA and SCL, but don't forget to adjust the Example code accordingly)
	* Connect the 3.3V to the 3.3V pin of the ESP32
	* Connect GND to GND on the ESP32
	* Connect the **SDA** pin to **GPIO 26**
	* Connect the **SCL** pin to **GPIO 27**
4. Press upload
5. Check the measured values in the Tools -> Serial Monitor or in the Tools -> Serial Plotter
	* Set the baud rate to 115200 for proper decoding
6. Download the **Sensirion MyAmbience CO2** App for [Android](https://play.google.com/store/apps/details?id=com.sensirion.myam) or [iOS](https://apps.apple.com/ch/app/sensirion-myambience-co2/id1529131572) and enjoy.

<img src="documents/images/ESP32-SCD30-HWSetup.png" width="600">

<img src="documents/images/SerialPlotterView.png" width="600">

<img src="documents/images/MyAmbienceCO2_Live.png" width="300"><img src="documents/images/MyAmbienceCO2_Plot.png" width="300">


## Repository Contents

* **/examples** - Example sketches for the library (.ino). Run these from the Arduino IDE. 
* **/src** - Source files for the library (.cpp, .h).
* **library.properties** - General library properties for the Arduino package manager. 


## License

See [LICENSE](LICENSE.txt).