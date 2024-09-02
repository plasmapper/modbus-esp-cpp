#include "pl_uart.h"
#include "pl_modbus.h"

//==============================================================================

class CustomModbusServer : public PL::ModbusServer {
public:
  const PL::ModbusFunctionCode userDefinedFunctionCode = (PL::ModbusFunctionCode)100;
  const size_t userDefinedFunctionRequestDataSize = 1;

  using PL::ModbusServer::ModbusServer;

  esp_err_t ReadRtuData(PL::Stream& stream, PL::ModbusFunctionCode functionCode, size_t& dataSize) override;
  esp_err_t HandleRequest(PL::Stream& stream, uint8_t stationAddress, PL::ModbusFunctionCode functionCode, size_t dataSize, uint16_t transactionId) override;
};

//==============================================================================

auto uart = std::make_shared<PL::Uart>(UART_NUM_0);
// Modbus server (port: UART0, protocol: RTU, address: 1)
CustomModbusServer server(uart, PL::ModbusProtocol::rtu, 1);

//==============================================================================

extern "C" void app_main(void) {
  uart->Initialize();
  uart->SetBaudRate(115200);
  uart->SetDataBits(8);
  uart->SetParity(PL::UartParity::even);
  uart->SetStopBits(PL::UartStopBits::one);
  uart->SetFlowControl(PL::UartFlowControl::none);
  uart->Enable();
  
  server.Enable();

  while (1) {
    vTaskDelay(1);
  }
}

//==============================================================================

esp_err_t CustomModbusServer::ReadRtuData(PL::Stream& stream, PL::ModbusFunctionCode functionCode, size_t& dataSize) {
  if (functionCode == userDefinedFunctionCode) {
    PL::Buffer& dataBuffer = GetDataBuffer();
    if (dataBuffer.size >= userDefinedFunctionRequestDataSize)
      return stream.Read(dataBuffer, 0, userDefinedFunctionRequestDataSize);
    else {
      stream.Read(NULL, dataSize);
      return ESP_ERR_INVALID_SIZE;
    }
  }
    
  return PL::ModbusServer::ReadRtuData(stream, functionCode, dataSize);
}

//==============================================================================

esp_err_t CustomModbusServer::HandleRequest(PL::Stream& stream, uint8_t stationAddress, PL::ModbusFunctionCode functionCode, size_t dataSize, uint16_t transactionId) {
  if (functionCode == userDefinedFunctionCode) {
    PL::Buffer& dataBuffer = GetDataBuffer();

    if (dataSize != userDefinedFunctionRequestDataSize)
      return WriteExceptionFrame(stream, stationAddress, functionCode, PL::ModbusException::illegalDataValue, transactionId);

    uint8_t requestValue = ((uint8_t*)dataBuffer.data)[0];

    if (dataBuffer.size < 1 + requestValue)
      return WriteExceptionFrame(stream, stationAddress, functionCode, PL::ModbusException::illegalDataValue, transactionId);
    
    for (int i = 0; i < requestValue; i++)
      ((uint8_t*)dataBuffer.data)[i+1] = esp_random();
    return WriteFrame(stream, stationAddress, functionCode, 1 + requestValue, transactionId);
  }

  return PL::ModbusServer::HandleRequest(stream, stationAddress, functionCode, dataSize, transactionId);
}
