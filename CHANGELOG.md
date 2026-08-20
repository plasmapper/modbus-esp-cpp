# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.4.1] - 2026-08-20
### Changed
- usb_server example's pl_usb dependency to 2.0.0.

### Fixed
- server_user_defined_function example not setting dataSize in ReadRtuData.

## [1.4.0] - 2026-08-18
### Added
- Write operation timeout to ModbusBase (used by both ModbusClient and ModbusServer).

### Fixed
- ModbusClient::Command TCP transaction ID matching loop having no overall timeout.
- ModbusClient::ReadRtuData not flushing the read buffer on an unrecognized function code.
- ModbusServer reading from stream while having a short read timeout.

## [1.3.0] - 2026-08-11
### Changed
- Lock timeout handling.
- Static const members to constexpr.

### Fixed
- Reading coils and discrete inputs at the end of a memory area.
- Reading multiple coils at the end of the request data buffer.
- Command response size not being initialized.
- TCP client transaction ID never changing.
- Client read/write commands not validating a zero item count.
- Unsafe pointer casts to the transaction buffer and memory areas.

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