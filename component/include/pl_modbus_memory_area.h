#pragma once
#include "pl_modbus_types.h"
#include "pl_common.h"

//==============================================================================

namespace PL {

//==============================================================================

/// @brief Modbus memory area
class ModbusMemoryArea : public Buffer {
public:
  /// @brief Memory area type
  const ModbusMemoryType type;
  /// @brief Memory area address
  const uint16_t address;
  /// @brief Number of memory area items (bits or 16-bit registers)
  const size_t numberOfItems;

  /// @brief Create a Modbus memory area and allocate memory 
  /// @param type memory area type
  /// @param address memory area address
  /// @param size memory area data size (in bytes)
  ModbusMemoryArea (ModbusMemoryType type, uint16_t address, size_t size);

  /// @brief Create a Modbus memory area from preallocated memory 
  /// @param type memory area type
  /// @param address memory area address
  /// @param data memory area data pointer
  /// @param size memory area data size (in bytes)
  ModbusMemoryArea (ModbusMemoryType type, uint16_t address, void* data, size_t size);

  /// @brief Create a Modbus memory area from preallocated memory with shared lockable
  /// @param type memory area type
  /// @param address memory area address
  /// @param data memory area data pointer
  /// @param size memory area data size (in bytes)
  /// @param lockable lockable object that is locked when this memory area is locked
  ModbusMemoryArea (ModbusMemoryType type, uint16_t address, void* data, size_t size, std::shared_ptr<Lockable> lockable);
  
  /// @brief Callback method that is called when memory area is about to be read
  virtual esp_err_t OnRead();
  /// @brief Callback method that is called when memory area has just been written
  virtual esp_err_t OnWrite();

private:
  size_t GetNumberOfItems();
};

//==============================================================================

}