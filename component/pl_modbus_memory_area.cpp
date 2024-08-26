#include "pl_modbus_memory_area.h"
#include "string.h"

//==============================================================================

namespace PL {

//==============================================================================

ModbusMemoryArea::ModbusMemoryArea(ModbusMemoryType type, uint16_t address, size_t size) : 
    Buffer(size), type(type), address(address), numberOfItems(GetNumberOfItems()) {
  memset(data, 0, size);
}

//==============================================================================

ModbusMemoryArea::ModbusMemoryArea(ModbusMemoryType type, uint16_t address, void* data, size_t size) :
  Buffer(data, size), type(type), address(address), numberOfItems(GetNumberOfItems()) {}

//==============================================================================

ModbusMemoryArea::ModbusMemoryArea(ModbusMemoryType type, uint16_t address, void* data, size_t size, std::shared_ptr<Lockable> lockable) :
  Buffer(data, size, lockable), type(type), address(address), numberOfItems(GetNumberOfItems()) {}

//==============================================================================

esp_err_t ModbusMemoryArea::OnRead() {
  return ESP_OK;
}

//==============================================================================

esp_err_t ModbusMemoryArea::OnWrite() {
  return ESP_OK;
}

//==============================================================================

size_t ModbusMemoryArea::GetNumberOfItems() {
  if (type == ModbusMemoryType::coils || type == ModbusMemoryType::discreteInputs)
    return std::min(size * 8, (size_t)(0xFFFF - address) + 1);
  if (type == ModbusMemoryType::holdingRegisters || type == ModbusMemoryType::inputRegisters)
    return  std::min(size / 2, (size_t)(0xFFFF - address) + 1);
  return 0;
}

//==============================================================================

}