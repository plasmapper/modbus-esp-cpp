#pragma once
#include "pl_modbus_base.h"
#include "pl_modbus_memory_area.h"

//==============================================================================

namespace PL {

//==============================================================================

/// @brief Modbus server class
class ModbusServer : public ModbusBase, public Server {
public:
  /// @brief Default server name
  static const std::string defaultName; 
  /// @brief Default read operation timeout in FreeRTOS ticks
  static const TickType_t defaultReadTimeout = 2;

  /// @brief Create an UART Modbus server with shared transaction buffer
  /// @param port UART port
  /// @param protocol Modbus protocol
  /// @param stationAddress station address
  /// @param buffer transaction buffer
  ModbusServer (std::shared_ptr<Uart> uart, ModbusProtocol protocol, uint8_t stationAddress, std::shared_ptr<Buffer> buffer);

  /// @brief Create an UART Modbus server and allocate transaction buffer
  /// @param port UART port
  /// @param protocol Modbus protocol
  /// @param stationAddress station address
  /// @param bufferSize transaction buffer size
  ModbusServer (std::shared_ptr<Uart> uart, ModbusProtocol protocol, uint8_t stationAddress, size_t bufferSize = defaultBufferSize);
  
  /// @brief Create a network Modbus server with shared transaction buffer
  /// @param port network port
  /// @param buffer transaction buffer
  ModbusServer (uint16_t port, std::shared_ptr<Buffer> buffer);
  
  /// @brief Create a network Modbus server and allocate transaction buffer
  /// @param port network port
  /// @param bufferSize transaction buffer size
  ModbusServer (uint16_t port, size_t bufferSize = defaultBufferSize);

  esp_err_t Lock (TickType_t timeout = portMAX_DELAY) override;
  esp_err_t Unlock() override;

  esp_err_t Enable() override;
  esp_err_t Disable() override;

  /// @brief Add a Modbus memory area to the server
  /// @param memoryArea memory area
  /// @return error code
  void AddMemoryArea (std::shared_ptr<ModbusMemoryArea> memoryArea);
  
  bool IsEnabled() override;

  /// @brief Get the Modbus station address
  /// @return station address
  uint8_t GetStationAddress();

  /// @brief Set the Modbus station address
  /// @param stationAddress station address
  /// @return error code
  esp_err_t SetStationAddress (uint8_t stationAddress);

  /// @brief Set the server task parameters
  /// @param taskParameters task parameters
  /// @return error code
  esp_err_t SetTaskParameters (const TaskParameters& taskParameters);

  /// @brief Get the base server (UartServer or TcpServer)
  /// @return base server
  std::weak_ptr<Server> GetBaseServer();

protected:
  esp_err_t ReadRtuData (Stream& stream, ModbusFunctionCode functionCode, size_t& dataSize) override;
  
  /// @brief Handle the Modbus client request
  /// @param stream client stream
  /// @param stationAddress request station address
  /// @param functionCode request function code
  /// @param dataSize request data size
  /// @param transactionId request transaction ID (for Modbus TCP protocol)
  /// @return error code
  virtual esp_err_t HandleRequest (Stream& stream, uint8_t stationAddress, ModbusFunctionCode functionCode, size_t dataSize, uint16_t transactionId);
  
  /// @brief Write the Modbus exception frame
  /// @param stream client stream
  /// @param stationAddress frame station address
  /// @param functionCode frame function code
  /// @param exception Modbus exception
  /// @param transactionId frame transaction ID (for Modbus TCP protocol)
  /// @return error code
  esp_err_t WriteExceptionFrame (Stream& stream, uint8_t stationAddress, ModbusFunctionCode functionCode, ModbusException exception, uint16_t transactionId);

private:
  class UartServer : public PL::UartServer {
  public:
    UartServer (std::shared_ptr<Uart> uart, ModbusServer& modbusServer);
    esp_err_t HandleRequest (Uart& uart) override;
  private:
    ModbusServer& modbusServer;
  };

  class TcpServer : public PL::TcpServer {
  public:
    TcpServer (uint16_t port, ModbusServer& modbusServer);
    esp_err_t HandleRequest (NetworkStream& stream) override;
  private:
    ModbusServer& modbusServer;
  };

  Mutex mutex;
  ModbusInterface interface;
  std::shared_ptr<UartServer> uartServer;
  std::shared_ptr<TcpServer> tcpServer;
  uint8_t stationAddress;
  std::vector<std::shared_ptr<ModbusMemoryArea>> memoryAreas;

  esp_err_t HandleRequest (Stream& stream);
  std::shared_ptr<ModbusMemoryArea> FindMemoryArea (ModbusMemoryType memoryType, uint16_t memoryAddress, uint16_t numberOfItems);
};

//==============================================================================
  
}