#pragma once
#include "pl_modbus_types.h"
#include "pl_common.h"
#include "pl_network.h"

//==============================================================================

namespace PL {

//==============================================================================

/// @brief Base class for both Modbus client and server
class ModbusBase : public virtual Lockable {
public:
  /// @brief Default transaction buffer size
  /// @note Sized for the largest RTU/TCP transaction.
  static constexpr size_t defaultBufferSize = 260;
  /// @brief Default protocol for network interface
  static constexpr ModbusProtocol defaultNetworkProtocol = ModbusProtocol::tcp;
  /// @brief Default station address for network interface
  static constexpr uint8_t defaultNetworkStationAddress = 255;

  /// @brief Maximum number of coils or discrete inputs that can be read in one request
  static constexpr uint16_t maxNumberOfModbusBitsToRead = 2000;
  /// @brief Maximum number of coils that can be written in one request
  static constexpr uint16_t maxNumberOfModbusBitsToWrite = 1968;
  /// @brief Maximum number of holding or input registers that can be read in one request
  static constexpr uint16_t maxNumberOfModbusRegistersToRead = 125;
  /// @brief Maximum number of holding registers that can be written in one request
  static constexpr uint16_t maxNumberOfModbusRegistersToWrite = 123;

  /// @brief Gets Modbus protocol
  /// @return protocol
  ModbusProtocol GetProtocol();
  
  /// @brief Sets Modbus protocol
  /// @param protocol protocol
  /// @return error code
  virtual esp_err_t SetProtocol(ModbusProtocol protocol);

  /// @brief Gets the read operation timeout 
  /// @return timeout in FreeRTOS ticks
  TickType_t GetReadTimeout();

  /// @brief Sets the read operation timeout
  /// @param timeout timeout in FreeRTOS ticks
  /// @return error code
  esp_err_t SetReadTimeout(TickType_t timeout);

  /// @brief Gets the write operation timeout
  /// @return timeout in FreeRTOS ticks
  TickType_t GetWriteTimeout();

  /// @brief Sets the write operation timeout
  /// @param timeout timeout in FreeRTOS ticks
  /// @return error code
  esp_err_t SetWriteTimeout(TickType_t timeout);

  /// @brief Gets the delay between the end of the read operation and unlocking the stream
  /// @return delay in FreeRTOS ticks
  TickType_t GetDelayAfterRead();

  /// @brief Sets the delay between the end of the read operation and unlocking the stream
  /// @param delay delay in FreeRTOS ticks
  /// @return error code
  esp_err_t SetDelayAfterRead(TickType_t delay);

protected:
  ModbusBase(ModbusProtocol protocol, std::shared_ptr<Buffer> buffer, TickType_t readTimeout, TickType_t writeTimeout);
  ModbusBase(ModbusProtocol protocol, size_t bufferSize, TickType_t readTimeout, TickType_t writeTimeout);

  /// @brief Reads the Modbus frame
  /// @param stream stream to read from
  /// @param stationAddress frame station address
  /// @param functionCode frame function code
  /// @param dataSize frame data size
  /// @param transactionId frame transaction ID (for Modbus TCP protocol)
  /// @return error code
  esp_err_t ReadFrame(Stream& stream, uint8_t& stationAddress, ModbusFunctionCode& functionCode, size_t& dataSize, uint16_t& transactionId);
  
  /// @brief Writes the Modbus frame
  /// @param stream stream to write to
  /// @param stationAddress frame station address
  /// @param functionCode frame function code
  /// @param dataSize frame data size
  /// @param transactionId frame transaction ID (for Modbus TCP protocol)
  /// @return error code
  esp_err_t WriteFrame(Stream& stream, uint8_t stationAddress, ModbusFunctionCode functionCode, size_t dataSize, uint16_t transactionId);

  /// @brief Reads data from the stream (overriden in ModbusServer to read one byte at a time)
  /// @param stream stream to read from
  /// @param dest destination (can be NULL)
  /// @param size number of bytes to read
  /// @return error code
  virtual esp_err_t StreamRead(Stream& stream, void* dest, size_t size);
  
  /// @brief Reads data from the stream into the buffer (overriden in ModbusServer to read one byte at a time)
  /// @param stream stream to read from
  /// @param dest destination buffer
  /// @param offset destination buffer offset
  /// @param size number of bytes to read
  /// @return error code
  virtual esp_err_t StreamRead(Stream& stream, Buffer& dest, size_t offset, size_t size);

  /// @brief Reads the data from the stream up to the specified termination character (overriden in ModbusServer to read one byte at a time)
  /// @param stream stream to read from
  /// @param termChar termination character
  /// @return error code
  virtual esp_err_t StreamReadUntil(Stream& stream, char termChar);

  /// @brief Reads the data for the specified function code (for Modbus RTU protocol)
  /// @param stream stream to read from
  /// @param functionCode frame function code
  /// @param dataSize frame data size
  /// @return error code
  virtual esp_err_t ReadRtuData(Stream& stream, ModbusFunctionCode functionCode, size_t& dataSize) = 0;

  /// @brief Gets the data part of the transaction buffer with offset and size based on the Modbus protocol
  /// @return data buffer
  Buffer& GetDataBuffer();
  
private:
  ModbusProtocol protocol;
  std::shared_ptr<Buffer> buffer;
  std::shared_ptr<Buffer> dataBuffer;
  TickType_t readTimeout;
  TickType_t writeTimeout;
  TickType_t delayAfterRead = 0;

  uint16_t Crc(size_t size);
  uint8_t Lrc(size_t size);
  void InitializeDataBuffer();
};

//==============================================================================

}