# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
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

[0.2.0]: https://github.com/Sensirion/Sensirion_GadgetBle_Arduino_Library/releases/tag/v0.2.0
[0.1.0]: https://github.com/Sensirion/Sensirion_GadgetBle_Arduino_Library/releases/tag/v0.1.0

