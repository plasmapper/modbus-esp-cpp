#pragma once
#include "pl_modbus_memory_area.h"

//==============================================================================

namespace PL {

//==============================================================================

/// @brief Class template for modbus memory area with typed data member
/// @tparam Type data type
template <class Type>
class ModbusTypedMemoryArea : public ModbusMemoryArea {
public:
  /// @brief typed modbus memory area data
  Type* data;

  /// @brief Create a typed Modbus memory area and allocate memory 
  /// @param type memory area type
  /// @param address memory area address
  ModbusTypedMemoryArea (ModbusMemoryType type, uint16_t address) :
    ModbusMemoryArea (type, address, sizeof (Type)), data ((Type*)ModbusMemoryArea::data) {}

  /// @brief Create a typed Modbus memory area from preallocated memory 
  /// @param type memory area type
  /// @param address memory area address
  /// @param data memory area data pointer
  ModbusTypedMemoryArea (ModbusMemoryType type, uint16_t address, Type* data) :
    ModbusMemoryArea (type, address, data, sizeof (Type)), data (data) {}

  /// @brief Create a typed Modbus memory area from preallocated memory with shared lockable
  /// @param type memory area type
  /// @param address memory area address
  /// @param data memory area data pointer
  /// @param lockable lockable object that is locked when this memory area is locked
  ModbusTypedMemoryArea (ModbusMemoryType type, uint16_t address, Type* data, std::shared_ptr<Lockable> lockable) :
    ModbusMemoryArea (type, address, data, sizeof (Type), lockable), data (data) {}
};

//==============================================================================

}