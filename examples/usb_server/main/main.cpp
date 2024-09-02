#include "pl_usb.h"
#include "pl_modbus.h"

//==============================================================================

auto usbDevice = std::make_shared<PL::UsbDevice>(TINYUSB_USBDEV_0);
auto usbDeviceCdc = std::make_shared<PL::UsbDeviceCdc>(usbDevice, TINYUSB_CDC_ACM_0);
// Modbus server (port: USB, protocol: RTU, address: 1)
PL::ModbusServer server(usbDeviceCdc, PL::ModbusProtocol::rtu, 1);

//==============================================================================

extern "C" void app_main(void) {
  usbDeviceCdc->Initialize();
  usbDeviceCdc->Enable();
  
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