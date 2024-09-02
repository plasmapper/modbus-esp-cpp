#include "pl_uart.h"
#include "pl_modbus.h"

//==============================================================================

auto uart = std::make_shared<PL::Uart>(UART_NUM_0);
// Modbus server (port: UART0, protocol: RTU, address: 1)
PL::ModbusServer server(uart, PL::ModbusProtocol::rtu, 1);

//==============================================================================

extern "C" void app_main(void) {
  uart->Initialize();
  uart->SetBaudRate(115200);
  uart->SetDataBits(8);
  uart->SetParity(PL::UartParity::even);
  uart->SetStopBits(PL::UartStopBits::one);
  uart->SetFlowControl(PL::UartFlowControl::none);
  uart->Enable();
  
  // Coils, digital inputs, holding registers and input registers all mapped to the same area.
  // 250 holding/input registers and 4000 coils/digital inputs).
  auto holdingRegisters = std::make_shared<PL::ModbusMemoryArea>(PL::ModbusMemoryType::holdingRegisters, 0, 500);
  server.AddMemoryArea(std::make_shared<PL::ModbusMemoryArea>(PL::ModbusMemoryType::coils, 0, holdingRegisters->data, holdingRegisters->size, holdingRegisters));
  server.AddMemoryArea(std::make_shared<PL::ModbusMemoryArea>(PL::ModbusMemoryType::discreteInputs, 0, holdingRegisters->data, holdingRegisters->size, holdingRegisters));
  server.AddMemoryArea(holdingRegisters);
  server.AddMemoryArea(std::make_shared<PL::ModbusMemoryArea>(PL::ModbusMemoryType::inputRegisters, 0, holdingRegisters->data, holdingRegisters->size, holdingRegisters));
  server.Enable();

  while (1) {
    vTaskDelay(1);
  }
}