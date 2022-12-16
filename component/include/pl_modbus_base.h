#pragma once
#include "pl_modbus_types.h"
#include "pl_common.h"
#include "pl_uart.h"
#include "pl_network.h"

//==============================================================================

namespace PL {

//==============================================================================

/// @brief Base class for both Modbus client and server
class ModbusBase : public virtual Lockable {
public:
  /// @brief Default transaction buffer size
  static const size_t defaultBufferSize = 260;
  /// @brief Default protocol for network interface
  static const ModbusProtocol defaultNetworkProtocol = ModbusProtocol::tcp;
  /// @brief Default station address for network interface
  static const uint8_t defaultNetworkStationAddress = 255;

  /// @brief Maximum number of coils or discrete inputs that can be read in one request
  static const uint16_t maxNumberOfModbusBitsToRead = 2000;
  /// @brief Maximum number of coils that can be written in one request
  static const uint16_t maxNumberOfModbusBitsToWrite = 1968;
  /// @brief Maximum number of holding or input registers that can be read in one request
  static const uint16_t maxNumberOfModbusRegistersToRead = 125;
  /// @brief Maximum number of holding registers that can be written in one request
  static const uint16_t maxNumberOfModbusRegistersToWrite = 123;

  /// @brief Get Modbus protocol
  /// @return protocol
  ModbusProtocol GetProtocol();
  
  /// @brief Set Modbus protocol
  /// @param protocol protocol
  /// @return error code
  virtual esp_err_t SetProtocol (ModbusProtocol protocol);

  /// @brief Get the read operation timeout 
  /// @return timeout in FreeRTOS ticks
  TickType_t GetReadTimeout();

  /// @brief Set the read operation timeout 
  /// @param timeout timeout in FreeRTOS ticks
  /// @return error code
  esp_err_t SetReadTimeout (TickType_t timeout);

  /// @brief Get the delay between the end of the read operation and unlocking the stream
  /// @return delay in FreeRTOS ticks
  TickType_t GetDelayAfterRead();

  /// @brief Set the delay between the end of the read operation and unlocking the stream
  /// @param delay delay in FreeRTOS ticks
  /// @return error code
  esp_err_t SetDelayAfterRead (TickType_t delay);

protected:
  ModbusBase (ModbusProtocol protocol, std::shared_ptr<Buffer> buffer, TickType_t readTimeout);
  ModbusBase (ModbusProtocol protocol, size_t bufferSize, TickType_t readTimeout);

  /// @brief Read the Modbus frame
  /// @param stream stream to read from
  /// @param stationAddress frame station address
  /// @param functionCode frame function code
  /// @param dataSize frame data size
  /// @param transactionId frame transaction ID (for Modbus TCP protocol)
  /// @return error code
  esp_err_t ReadFrame (Stream& stream, uint8_t& stationAddress, ModbusFunctionCode& functionCode, size_t& dataSize, uint16_t& transactionId);
  
  /// @brief Write the Modbus frame
  /// @param stream stream to write to
  /// @param stationAddress frame station address
  /// @param functionCode frame function code
  /// @param dataSize frame data size
  /// @param transactionId frame transaction ID (for Modbus TCP protocol)
  /// @return error code
  esp_err_t WriteFrame (Stream& stream, uint8_t stationAddress, ModbusFunctionCode functionCode, size_t dataSize, uint16_t transactionId);
  
  /// @brief Read the data for the specified function code (for Modbus RTU protocol).
  /// @param stream stream to read from
  /// @param functionCode frame function code
  /// @param dataSize frame data size
  /// @return error code
  virtual esp_err_t ReadRtuData (Stream& stream, ModbusFunctionCode functionCode, size_t& dataSize) = 0;

  /// @brief Get the data part of the transaction buffer with offset and size based on the Modbus protocol
  /// @return data buffer
  Buffer& GetDataBuffer();
  
private:
  ModbusProtocol protocol;
  std::shared_ptr<Buffer> buffer;
  std::shared_ptr<Buffer> dataBuffer;
  TickType_t readTimeout;
  TickType_t delayAfterRead = 0;

  uint16_t Crc (size_t size);
  uint8_t Lrc (size_t size);
  void InitializeDataBuffer();
};

//==============================================================================

}