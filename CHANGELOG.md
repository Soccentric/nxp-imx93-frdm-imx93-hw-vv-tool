# Changelog

All notable changes to the FRDM-IMX93 Peripheral Verification Tool will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Initial production-ready release for NXP FRDM-IMX93
- Complete implementation of all 11 peripheral testers:
  - CPU (dual Cortex-A55 performance benchmarking, thermal monitoring)
  - GPIO (5 GPIO banks, LPI2C, LPSPI, LPUART, FlexPWM, FlexIO)
  - GPU (Vivante GC520 graphics validation, OpenGL ES 3.1)
  - Memory (DDR4/LPDDR4 integrity testing, up to 2GB)
  - Storage (eMMC 5.1, SD/MMC via uSDHC controllers)
  - Display (MIPI-DSI, LVDS interface testing)
  - Camera (MIPI-CSI2, ISI interface validation)
  - USB (dual USB 2.0 host/device testing)
  - Networking (dual Gigabit Ethernet with ENET QoS controllers)
  - Power (PCA9451A PMIC monitoring, voltage rails, DVFS)
  - Form Factor (FRDM board Arduino/mikroBUS interface validation)
- Comprehensive documentation with Doxygen
- Unit testing framework with Google Test
- CI/CD workflows for automated testing and releases
- Build scripts for easy compilation and installation
- Cross-compilation support for ARM Cortex-A55
- CONTRIBUTING.md with detailed development guidelines

### Changed
- Adapted all peripheral testers from Raspberry Pi CM5 to i.MX93
- Updated CMake configurations for ARM Cortex-A55 optimization
- Enhanced build system with i.MX93-specific presets
- Updated README.md with FRDM-IMX93 specific information

### Removed
- Raspberry Pi specific hardware references
- BCM chipset specific code

## [1.0.0] - 2025-11-19

### Added
- Initial release of FRDM-IMX93 Peripheral Verification Tool
- Support for NXP FRDM-IMX93 development board
- Modular architecture with PIMPL pattern
- Short test and monitoring test modes
- Command-line interface for flexible testing
- MIT License

[Unreleased]: https://github.com/Soccentric/frdm-imx93-hardware-peripherals-verification-tool/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/Soccentric/frdm-imx93-hardware-peripherals-verification-tool/releases/tag/v1.0.0
