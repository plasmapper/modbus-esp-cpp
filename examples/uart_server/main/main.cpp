#include "pl_modbus.h"

//==============================================================================

auto port = std::make_shared<PL::UartPort>(UART_NUM_0);
// Modbus server (port: UART0, protocol: RTU, address: 1)
PL::ModbusServer server (port, PL::ModbusProtocol::rtu, 1);

//==============================================================================

extern "C" void app_main(void) {
  port->Initialize();
  port->SetBaudRate (115200);
  port->SetDataBits (8);
  port->SetParity (PL::UartParity::even);
  port->SetStopBits (PL::UartStopBits::one);
  port->SetFlowControl (PL::UartFlowControl::none);
  port->Enable();
  
  // Coils, digital inputs, holding registers and input registers all mapped to the same area.
  // 250 holding/input registers and 4000 coils/digital inputs).
  auto buffer = std::make_shared<PL::Buffer>(500);
  server.AddMemoryArea (PL::ModbusMemoryType::coils, 0, buffer);
  server.AddMemoryArea (PL::ModbusMemoryType::discreteInputs, 0, buffer);
  server.AddMemoryArea (PL::ModbusMemoryType::holdingRegisters, 0, buffer);
  server.AddMemoryArea (PL::ModbusMemoryType::inputRegisters, 0, buffer);
  server.Enable();

  while (1) {
    vTaskDelay (1);
  }
}