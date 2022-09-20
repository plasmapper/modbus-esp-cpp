#include "pl_modbus.h"

//==============================================================================

auto port = std::make_shared<PL::UartPort>(UART_NUM_0);
// Modbus server (port: UART0, protocol: RTU, address: 1)
PL::ModbusServer server (port, PL::ModbusProtocol::rtu, 1);

#pragma pack(push, 1)
struct TypedRegisterData {
  uint16_t reg0 = 0;
  uint16_t reg1 = 0;
  uint16_t reg2 = 0;
  uint16_t reg3 = 0;
  uint16_t reg4 = 0;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct TypedRegisterWithCoilData {
  union {
    struct {
      uint8_t flag0 : 1;
      uint8_t flag1 : 1;
      uint8_t flag2 : 1;
    };
    uint16_t reg0 = 0;
  };
  uint16_t reg1 = 0;
  uint16_t reg2 = 0;
  uint16_t reg3 = 0;
  uint16_t reg4 = 0;
};
#pragma pack(pop)

class SecondCounterInputRegister : public PL::ModbusMemoryArea {
public:
  SecondCounterInputRegister (uint16_t address);
  void OnRead() override;

private:
  uint16_t data;
};

//==============================================================================

extern "C" void app_main(void) {
  port->Initialize();
  port->SetBaudRate (115200);
  port->SetDataBits (8);
  port->SetParity (PL::UartParity::even);
  port->SetStopBits (PL::UartStopBits::one);
  port->SetFlowControl (PL::UartFlowControl::none);
  port->Enable();
  
  // 5 holding registers (addresses 0..4)
  auto holdingRegisters = std::make_shared<PL::ModbusMemoryArea>(PL::ModbusMemoryType::holdingRegisters, 0, 10);
  server.AddMemoryArea (holdingRegisters);
  // Example of data access in program
  if (1) {
    PL::LockGuard lg (*holdingRegisters);
    ((uint16_t*)holdingRegisters->data)[0] = 123;
  }

  // 5 holding registers (addresses 5..9) with typed access
  auto typedHoldingRegisters = std::make_shared<PL::ModbusTypedMemoryArea<TypedRegisterData>>(PL::ModbusMemoryType::holdingRegisters, 5);
  server.AddMemoryArea (typedHoldingRegisters);
  // Example of data access in program
  if (1) {
    PL::LockGuard lg (*typedHoldingRegisters);
    typedHoldingRegisters->data->reg0 = 123;
  }

  // 5 holding and 5 input registers (addresses 10..14) mapped to the same memory area with typed access
  auto typedCombinedRegisters = std::make_shared<PL::TypedBuffer<TypedRegisterData>>();
  server.AddMemoryArea (PL::ModbusMemoryType::holdingRegisters, 10, typedCombinedRegisters);
  server.AddMemoryArea (PL::ModbusMemoryType::inputRegisters, 10, typedCombinedRegisters);
  // Example of data access in program
  if (1) {
    PL::LockGuard lg (*typedCombinedRegisters);
    typedCombinedRegisters->data->reg0 = 123;
  }

  // 5 holding registers (addresses 15..19) with first register LSB accessible as coils (addresses 15..22) with typed access
  auto typedRegistersWithCoils = std::make_shared<PL::TypedBuffer<TypedRegisterWithCoilData>>();
  server.AddMemoryArea (PL::ModbusMemoryType::holdingRegisters, 15, typedRegistersWithCoils);
  server.AddMemoryArea (PL::ModbusMemoryType::coils, 15, typedRegistersWithCoils, 0, 1);
  // Example of data access in program
  if (1) {
    PL::LockGuard lg (*typedRegistersWithCoils);
    typedRegistersWithCoils->data->flag1 = true;
    typedRegistersWithCoils->data->reg1 = 123;
  }

  // Dynamic input register (address 20) that contains the uptime value in seconds
  std::shared_ptr<PL::ModbusMemoryArea> secondCounterInputRegister = std::make_shared<SecondCounterInputRegister>(20);
  server.AddMemoryArea (secondCounterInputRegister);

  // Dynamic input register (address 21) that duplicates register 20
  server.AddMemoryArea (PL::ModbusMemoryType::inputRegisters, 21, secondCounterInputRegister, 0, secondCounterInputRegister->size);

  server.Enable();

  while (1) {
    vTaskDelay (1);
  }
}

//==============================================================================

SecondCounterInputRegister::SecondCounterInputRegister (uint16_t address) :
  PL::ModbusMemoryArea (PL::ModbusMemoryType::inputRegisters, address, &data, sizeof (data)) {}

//==============================================================================

void SecondCounterInputRegister::OnRead() {
  data = xTaskGetTickCount() * portTICK_PERIOD_MS / 1000;
}
