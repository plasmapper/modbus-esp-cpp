#include "pl_modbus.h"

//==============================================================================

auto uart = std::make_shared<PL::Uart>(UART_NUM_0);
// Modbus client (port: UART0, protocol: RTU, station address: 255)
PL::ModbusClient client(uart, PL::ModbusProtocol::rtu, 1);

#pragma pack(push, 1)
struct Coils {
  uint8_t coil0 : 1;
  uint8_t coil1 : 1;
  uint8_t coil2 : 1;
};
#pragma pack(pop)

//==============================================================================

extern "C" void app_main(void) {
  uart->Initialize();
  uart->SetBaudRate(115200);
  uart->SetDataBits(8);
  uart->SetParity(PL::UartParity::even);
  uart->SetStopBits(PL::UartStopBits::one);
  uart->SetFlowControl(PL::UartFlowControl::none);
  uart->Enable();

  while (1) {
    uint16_t holdingRegisters[10];
    if (client.ReadHoldingRegisters(0, sizeof(holdingRegisters) / sizeof(uint16_t), holdingRegisters, NULL) == ESP_OK) {
      for (int i = 0; i < sizeof(holdingRegisters) / sizeof(uint16_t); i++)
        holdingRegisters[i]++;
      client.WriteMultipleHoldingRegisters(0, sizeof(holdingRegisters) / sizeof(uint16_t), holdingRegisters, NULL);
    }

    Coils coils;
    if (client.ReadCoils(0, 3, &coils, NULL) == ESP_OK) {
      coils.coil0 = !coils.coil0;
      coils.coil1 = !coils.coil1;
      coils.coil2 = !coils.coil2;
      client.WriteMultipleCoils(0, 3, &coils, NULL);
    }
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}