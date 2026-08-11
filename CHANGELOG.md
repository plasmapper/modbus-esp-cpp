# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Changed
- Lock timeout handling.
- Static const members to constexpr.
- CMake minimum required version to 3.22.
- ESP-IDF, pl_common and pl_network dependency version requirements.

### Fixed
- Reading coils and discrete inputs at the end of a memory area.
- Reading multiple coils at the end of the request data buffer.
- Command response size not being initialized.
- TCP client transaction ID never changing.
- Client read/write commands not validating a zero item count.

## [1.2.1] - 2024-09-12
### Fixed
- ModbusTypedMemoryArea data pointer not being const.

## [1.2.0] - 2024-09-02
### Added
- USB Modbus server example.

### Changed
- All UART references to a more generic Stream.

## [1.1.0] - 2024-08-27
### Changed
- Modbus client and server locking.
- ESP-IDF dependency to 5.3.

## [1.0.1] - 2024-06-12
### Added
- Copying examples to component folder on upload.

## [1.0.0] - 2024-06-12
Initial release.