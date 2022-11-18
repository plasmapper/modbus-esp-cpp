#pragma once
#include "pl_modbus_base.h"

//==============================================================================

namespace PL {

//==============================================================================

/// @brief Modbus client class
class ModbusClient : public ModbusBase {
public:
  /// @brief Default read operation timeout in FreeRTOS ticks
  static const TickType_t defaultReadTimeout = 300 / portTICK_PERIOD_MS;

  /// @brief Create an UART Modbus client
  /// @param port UART port
  /// @param protocol Modbus protocol
  /// @param stationAddress station address
  /// @param bufferSize transaction buffer size
  ModbusClient (std::shared_ptr<UartPort> port, ModbusProtocol protocol, uint8_t stationAddress, size_t bufferSize = defaultBufferSize);

  /// @brief Create a network Modbus client with IPv4 remote address
  /// @param address remote IPv4 address
  /// @param port remote port
  /// @param bufferSize transaction buffer size
  ModbusClient (IpV4Address address, uint16_t port, size_t bufferSize = defaultBufferSize);

  /// @brief Create a network Modbus client with IPv6 remote address
  /// @param address remote IPv6 address
  /// @param port remote port
  /// @param bufferSize transaction buffer size
  ModbusClient (IpV6Address address, uint16_t port, size_t bufferSize = defaultBufferSize);

  /// @brief Create a network Modbus client using shared TCP client
  /// @param tcpClient TCP client
  /// @param bufferSize transaction buffer size
  ModbusClient (std::shared_ptr<TcpClient> tcpClient, size_t bufferSize = defaultBufferSize);

  esp_err_t Lock (TickType_t timeout = portMAX_DELAY) override;
  esp_err_t Unlock() override;

  /// @brief Send a Modbus request and return response data
  /// @param functionCode request function code
  /// @param requestData request data pointer
  /// @param requestDataSize request data size
  /// @param responseData response data pointer
  /// @param maxResponseDataSize maximum response data size
  /// @param responseDataSize response data size
  /// @param exception Modbus exception
  /// @return error code
  esp_err_t Command (ModbusFunctionCode functionCode, const void* requestData, size_t requestDataSize, void* responseData, size_t maxResponseDataSize, size_t* responseDataSize, ModbusException* exception);
  
  /// @brief Read coils
  /// @param address first coil address
  /// @param numberOfItems number of coils
  /// @param responseData coil values (8 values per byte)
  /// @param exception Modbus exception
  /// @return error code
  esp_err_t ReadCoils (uint16_t address, uint16_t numberOfItems, void* responseData, ModbusException* exception);

  /// @brief Read discrete inputs
  /// @param address first discrete input address
  /// @param numberOfItems number of discrete inputs
  /// @param responseData discrete input values (8 values per byte)
  /// @param exception Modbus exception
  /// @return error code
  esp_err_t ReadDiscreteInputs (uint16_t address, uint16_t numberOfItems, void* responseData, ModbusException* exception);

  /// @brief Read holding registers
  /// @param address first holding register address
  /// @param numberOfItems number of holding registers
  /// @param responseData holding register values
  /// @param exception Modbus exception
  /// @return error code
  esp_err_t ReadHoldingRegisters (uint16_t address, uint16_t numberOfItems, void* responseData, ModbusException* exception);

  /// @brief Read input registers
  /// @param address first input register address
  /// @param numberOfItems number of input registers
  /// @param responseData input register values
  /// @param exception Modbus exception
  /// @return error code
  esp_err_t ReadInputRegisters (uint16_t address, uint16_t numberOfItems, void* responseData, ModbusException* exception);

  /// @brief Write single coil
  /// @param address coil address
  /// @param value coil value
  /// @param exception Modbus exception
  /// @return error code 
  esp_err_t WriteSingleCoil (uint16_t address, bool value, ModbusException* exception);

  /// @brief Write single holding register
  /// @param address holding register address
  /// @param value holding register value
  /// @param exception Modbus exception
  /// @return error code
  esp_err_t WriteSingleHoldingRegister (uint16_t address, uint16_t value, ModbusException* exception);

  /// @brief Write multiple coils
  /// @param address first coil address
  /// @param numberOfItems number of coils
  /// @param requestData coil values (8 values per byte)
  /// @param exception Modbus exception
  /// @return error code  
  esp_err_t WriteMultipleCoils (uint16_t address, uint16_t numberOfItems, const void* requestData, ModbusException* exception);

  /// @brief Write multiple holding registers
  /// @param address first holding register address
  /// @param numberOfItems number of holding registers
  /// @param requestData holding register values
  /// @param exception Modbus exception
  /// @return error code  
  esp_err_t WriteMultipleHoldingRegisters (uint16_t address, uint16_t numberOfItems, const void* requestData, ModbusException* exception);

  /// @brief Get the Modbus station address
  /// @return station address
  uint8_t GetStationAddress();

  /// @brief Set the Modbus station address
  /// @param address station address
  /// @return error code
  esp_err_t SetStationAddress (uint8_t address);

protected:
  esp_err_t ReadRtuData (Stream& stream, ModbusFunctionCode functionCode, size_t& dataSize) override;

private:
  Mutex mutex;
  ModbusInterface interface;
  std::shared_ptr<UartPort> uartPort;
  std::shared_ptr<TcpClient> tcpClient;
  uint8_t stationAddress;
  std::shared_ptr<Buffer> buffer;
  uint16_t transactionId = 0;
  
  esp_err_t Command (ModbusFunctionCode functionCode, size_t requestDataSize, size_t& responseDataSize, ModbusException* exception);
  esp_err_t ReadBits (ModbusFunctionCode functionCode, uint16_t address, uint16_t numberOfItems, void* responseData, ModbusException* exception);
  esp_err_t ReadRegisters (ModbusFunctionCode functionCode, uint16_t address, uint16_t numberOfItems, void* responseData, ModbusException* exception);

  struct AddressRange {
    uint16_t address;
    uint16_t numberOfItems;
  };

  std::vector<AddressRange> SplitAddressRange (uint16_t address, uint16_t numberOfItems, uint16_t maxNumberOfItems);
};

//==============================================================================
  
}