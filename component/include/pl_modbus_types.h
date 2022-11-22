#pragma once
#include "stdint.h"

//==============================================================================

namespace PL {

//==============================================================================

/// @brief Modbus interface
enum class ModbusInterface {
  /// @brief UART interface
  uart,
  /// @brief network interface
  network
};

/// @brief Modbus protocol
enum class ModbusProtocol : uint8_t {
  /// @brief Modbus RTU
  rtu = 0,
  /// @brief Modbus ASCII
  ascii = 1,
  /// @brief Modbus TCP
  tcp = 2
};

/// @brief Modbus memory type
enum class ModbusMemoryType {
  /// @brief coils
  coils,
  /// @brief discrete inputs
  discreteInputs,
  /// @brief holding registers
  holdingRegisters,
  /// @brief input registers
  inputRegisters
};

/// @brief Modbus function code
enum class ModbusFunctionCode : uint8_t {
  /// @brief unknown
  unknown = 0,
  /// @brief read coils
  readCoils = 1,
  /// @brief read discrete inputs
  readDiscreteInputs = 2,
  /// @brief read holding registers
  readHoldingRegisters = 3,
  /// @brief read input registers
  readInputRegisters = 4,
  /// @brief write single coil
  writeSingleCoil = 5,
  /// @brief write single holding register
  writeSingleHoldingRegister = 6,
  /// @brief write multiple coils
  writeMultipleCoils = 15,
  /// @brief write multiple holding registers
  writeMultipleHoldingRegisters = 16
};

// Modbus exception
enum class ModbusException : uint8_t {
  /// @brief no exception
  noException = 0,
  /// @brief illegal function
  illegalFunction = 1,
  /// @brief illegal data address
  illegalDataAddress = 2,
  /// @brief illegal data value
  illegalDataValue = 3,
  /// @brief server device failure
  serverDeviceFailure = 4,
  /// @brief acknowledge
  acknowledge = 5,
  /// @brief server device busy
  serverDeviceBusy = 6,
  /// @brief negative acknowledge
  negativeAcknowledge = 7,
  /// @brief memory parity error
  memoryParityError = 8,
  /// @brief gateway path unavailable
  gatewayPathUnavailable = 10,
  /// @brief gateway target device failed to respond
  gatewayTargetDeviceFailedToRespond = 11
};

//==============================================================================
  
}