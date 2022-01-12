# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.13.0] - 2022-01-19
### Added
- SEN54 Example code and tutorial
- SEN55 Example code and tutorial
- SVM41 Example code and tutorial

## [0.12.0] - 2022-01-12
### Added
- New sample types allowing PM2P5 values > 1000 (up to 6552) 
  - T_RH_CO2_PM25_V2
  - T_RH_CO2_VOC_NOX_PM25_V2
  - T_RH_VOC_NOX_PM2P5_V2

## [0.11.0] - 2021-11-10
### Added
- New sample type for T_RH_CO2_VOC_NOX_PM25

## [0.10.0] - 2021-09-08
### Added
- New sample type for T_RH_VOC_NOX
- New sample type for T_RH_VOC_NOX_PM2P5

## [0.9.0] - 2021-05-11
### Added
- Add new data type: T_RH_CO2_VOC_PM25_HCHO

### Changed
- Tutorial adjustments

## [0.8.0] - 2021-03-10
### Added
- SEN44 Example code and tutorial

## [0.7.1] - 2021-01-22
## Fixed
- Fix library.properties

## [0.7.0] - 2021-01-22
### Added
- SCD4x EvalKit BLE Gadget example using T_RH_CO2 DataType.
- SCD4x EvalKit Tutorial

### Fixed
- DataType of T_RH_V4 SampleType model

## [0.6.0] - 2020-12-18
### Added
- HCHO unit and T_RH_HCHO datatype.
- SVM40 BLE Gadget example using T_RH_VOC DataType.
- SFA30 BLE Gadget example using T_RH_HCHO DataType.
- SCD30 BLE Gadget example using T_RH_CO2 DataType.
- Setup Tutorials for SCD30, SVM40 and SFA30 examples.

## [0.5.0] - 2020-12-14
### Added
- Enable GENERIC gadget type support with MyAmbience v2.0 app.

## [0.4.2] - 2020-12-03
### Fixed
- Fix current app compatibility, which only supports DataType T_RH_CO2_ALT yet.

## [0.4.1] - 2020-12-03
### Fixed
- Compile issues on Windows.

## [0.4.0] - 2020-12-02
### Added
- Configurable Wifi credentials via BLE and usage example.
- Support for more DataTypes: T_RH_VOC, T_RH_V3, T_RH_V4, T_RH_CO2_PM25 and T_RH_VOC_PM25.
- Ability to switch the used DataType during operation.

### Changed
- Gadget name is changed to a generic identifier.
- DataType initialisation.

## [0.3.0] - 2020-11-18
### Added
- Example 3: SCD30 monitor on the LILYGO TTGO T-Display board with UI.

### Fixed
- Pass CO2 value as float in SCD30 example.

### Changed
- Type changes from int to uint16_t, whereever the BLE protocol requries it.
- Replace C style casts with their C++ counterparts.

## [0.2.0] - 2020-11-17
### Changed
- All writeXX functions expect a float as sensor value.
- Removed all unnecessary internal static variables.

### Fixed
- Datatype used in Arduino Example 1 changed to T_RH_CO2_ALT.
- Download protocol internals related to sample count characteristic.
- History buffer size and capacity computations.

## [0.1.0] - 2020-11-16
### Added
- Initial release supporting T_RH_CO2_ALT datatype only.

[Unreleased]: https://github.com/sensirion/sensirion_gadgetble_arduino_library/compare/v0.13.0...HEAD
[0.13.0]: https://github.com/sensirion/sensirion_gadgetble_arduino_library/compare/v0.12.0...v0.13.0
[0.12.0]: https://github.com/sensirion/sensirion_gadgetble_arduino_library/compare/v0.11.0...v0.12.0
[0.11.0]: https://github.com/sensirion/sensirion_gadgetble_arduino_library/compare/v0.10.0...v0.11.0
[0.10.0]: https://github.com/sensirion/sensirion_gadgetble_arduino_library/compare/v0.9.0...v0.10.0
[0.9.0]: https://github.com/sensirion/sensirion_gadgetble_arduino_library/compare/v0.8.0...v0.9.0
[0.8.0]: https://github.com/sensirion/sensirion_gadgetble_arduino_library/compare/v0.7.1...v0.8.0
[0.7.1]: https://github.com/sensirion/sensirion_gadgetble_arduino_library/compare/v0.7.0...v0.7.1
[0.7.0]: https://github.com/sensirion/sensirion_gadgetble_arduino_library/compare/v0.6.0...v0.7.0
[0.6.0]: https://github.com/sensirion/sensirion_gadgetble_arduino_library/compare/v0.5.0...v0.6.0
[0.5.0]: https://github.com/sensirion/sensirion_gadgetble_arduino_library/compare/v0.4.2...v0.5.0
[0.4.2]: https://github.com/sensirion/sensirion_gadgetble_arduino_library/compare/v0.4.1...v0.4.2
[0.4.1]: https://github.com/sensirion/sensirion_gadgetble_arduino_library/compare/v0.4.0...v0.4.1
[0.4.0]: https://github.com/sensirion/sensirion_gadgetble_arduino_library/compare/v0.3.0...v0.4.0
[0.3.0]: https://github.com/sensirion/sensirion_gadgetble_arduino_library/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/sensirion/sensirion_gadgetble_arduino_library/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/sensirion/sensirion_gadgetble_arduino_library/releases/tag/v0.1.0

