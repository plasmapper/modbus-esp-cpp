#include "pl_modbus_server.h"

//==============================================================================

namespace PL {

//==============================================================================

const uint_fast8_t lowerBits[] =  {0, 0b1, 0b11, 0b111, 0b1111, 0b11111, 0b111111, 0b1111111, 0b11111111};

const std::string ModbusServer::defaultName = "Modbus Server";

//==============================================================================

ModbusServer::ModbusServer (std::shared_ptr<UartPort> port, ModbusProtocol protocol, uint8_t stationAddress, std::shared_ptr<Buffer> buffer) :
    ModbusBase (protocol, buffer, defaultReadTimeout), interface (ModbusInterface::uart), uartServer (std::make_shared<UartServer>(port, *this)),
    stationAddress (stationAddress) {
  SetName (defaultName);
}

//==============================================================================

ModbusServer::ModbusServer (std::shared_ptr<UartPort> port, ModbusProtocol protocol, uint8_t stationAddress, size_t bufferSize) :
    ModbusBase (protocol, bufferSize, defaultReadTimeout), interface (ModbusInterface::uart), uartServer (std::make_shared<UartServer>(port, *this)),
    stationAddress (stationAddress) {
  SetName (defaultName);
}

//==============================================================================

ModbusServer::ModbusServer (uint16_t port, std::shared_ptr<Buffer> buffer) :
    ModbusBase (defaultNetworkProtocol, buffer, defaultReadTimeout), interface (ModbusInterface::network), tcpServer (std::make_shared<TcpServer>(port, *this)),
    stationAddress (defaultNetworkStationAddress) {
  SetName (defaultName);
}

//==============================================================================

ModbusServer::ModbusServer (uint16_t port, size_t bufferSize) :
    ModbusBase (defaultNetworkProtocol, bufferSize, defaultReadTimeout), interface (ModbusInterface::network), tcpServer (std::make_shared<TcpServer>(port, *this)),
    stationAddress (defaultNetworkStationAddress) {
  SetName (defaultName);
}

//==============================================================================

esp_err_t ModbusServer::Lock (TickType_t timeout) {
  return mutex.Lock (timeout);
}

//==============================================================================

esp_err_t ModbusServer::Unlock() {
  return mutex.Unlock();
}

//==============================================================================

esp_err_t ModbusServer::Enable() {
  return interface == ModbusInterface::uart ? uartServer->Enable() : tcpServer->Enable();
}

//==============================================================================

esp_err_t ModbusServer::Disable() {
  return interface == ModbusInterface::uart ? uartServer->Disable() : tcpServer->Disable();
}

//==============================================================================

esp_err_t ModbusServer::AddMemoryArea (std::shared_ptr<ModbusMemoryArea> memoryArea) {
  LockGuard lg (*this);
  memoryAreas.push_back (memoryArea);
  return ESP_OK;
}

//==============================================================================

esp_err_t ModbusServer::AddMemoryArea (ModbusMemoryType type, uint16_t address, std::shared_ptr<Buffer> buffer) {
  return AddMemoryArea (type, address, buffer, 0, buffer->size);
}

//==============================================================================

esp_err_t ModbusServer::AddMemoryArea (ModbusMemoryType type, uint16_t address, std::shared_ptr<Buffer> buffer, size_t offset, size_t size) {
  if (offset >= buffer->size)
    return ESP_ERR_INVALID_ARG;
  if (size > buffer->size - offset)
    return ESP_ERR_INVALID_SIZE;
  return AddMemoryArea (std::make_shared<ModbusMemoryArea> (type, address, (uint8_t*)buffer->data + offset, size, buffer));
}

//==============================================================================

esp_err_t ModbusServer::AddMemoryArea (ModbusMemoryType type, uint16_t address, std::shared_ptr<ModbusMemoryArea> baseMemoryArea, size_t offset, size_t size) {
  if (offset >= baseMemoryArea->size)
    return ESP_ERR_INVALID_ARG;
  if (size > baseMemoryArea->size - offset)
    return ESP_ERR_INVALID_SIZE;
  return AddMemoryArea (std::make_shared<ModbusMemoryArea> (type, address, (uint8_t*)baseMemoryArea->data + offset, size, baseMemoryArea));
}  

//==============================================================================

bool ModbusServer::IsEnabled() {
  return interface == ModbusInterface::uart ? uartServer->IsEnabled() : tcpServer->IsEnabled();
}

//==============================================================================

uint8_t ModbusServer::GetStationAddress() {
  LockGuard lg (*this);
  return stationAddress;
}

//==============================================================================

esp_err_t ModbusServer::SetStationAddress (uint8_t stationAddress) {
  LockGuard lg (*this);
  this->stationAddress = stationAddress;
  return ESP_OK;
}

//==============================================================================

size_t ModbusServer::GetMaxNumberOfClients() {
  return interface == ModbusInterface::uart ? 1 : tcpServer->GetMaxNumberOfClients();
}

//==============================================================================

esp_err_t ModbusServer::SetMaxNumberOfClients (size_t maxNumberOfClients) {
  return interface == ModbusInterface::uart ? ESP_ERR_NOT_SUPPORTED : tcpServer->SetMaxNumberOfClients (maxNumberOfClients);
}

//==============================================================================

esp_err_t ModbusServer::SetTaskParameters (const TaskParameters& taskParameters) {
  if (interface == ModbusInterface::uart)
    return uartServer->SetTaskParameters (taskParameters);
  else
    return tcpServer->SetTaskParameters (taskParameters);
}

//==============================================================================

std::weak_ptr<Server> ModbusServer::GetBaseServer() {
  if (interface == ModbusInterface::uart)
    return uartServer;
  else
    return tcpServer;
}

//==============================================================================

esp_err_t ModbusServer::ReadRtuData (Stream& stream, ModbusFunctionCode functionCode, size_t& dataSize) {
  Buffer& dataBuffer = GetDataBuffer();
  
  switch (functionCode) {
    case ModbusFunctionCode::readCoils:
    case ModbusFunctionCode::readDiscreteInputs:
    case ModbusFunctionCode::readHoldingRegisters:
    case ModbusFunctionCode::readInputRegisters:
    case ModbusFunctionCode::writeSingleCoil:
    case ModbusFunctionCode::writeSingleHoldingRegister:
      dataSize = 4;
      if (dataBuffer.size >= dataSize)
        return stream.Read (dataBuffer, 0, dataSize);
      else {
        stream.Read (NULL, dataSize);
        return ESP_ERR_INVALID_SIZE;
      }        

    case ModbusFunctionCode::writeMultipleCoils:
    case ModbusFunctionCode::writeMultipleHoldingRegisters:
      if (dataBuffer.size >= 5) {
        PL_RETURN_ON_ERROR (stream.Read (dataBuffer, 0, 5));
        dataSize = 5 + ((uint8_t*)dataBuffer.data)[4];
        if (dataBuffer.size >= dataSize)
          return stream.Read (dataBuffer, 5, dataSize - 5);
        else {
          stream.Read (NULL, dataSize - 5);
          return ESP_ERR_INVALID_SIZE;          
        }
      }
      else {
        uint8_t byteSize;
        if (stream.Read (NULL, 4) == ESP_OK && stream.Read (&byteSize, 1) == ESP_OK)
          stream.Read (NULL, byteSize);
        return ESP_ERR_INVALID_SIZE;
      }

    default:
      stream.SetReadTimeout (2);
      stream.Read (NULL, SIZE_MAX);
      return ESP_ERR_NOT_SUPPORTED;  
  }
}

//==============================================================================

esp_err_t ModbusServer::HandleRequest (Stream& stream, uint8_t stationAddress, ModbusFunctionCode functionCode, size_t dataSize, uint16_t transactionId) {
  Buffer& dataBuffer = GetDataBuffer();

  if (functionCode == ModbusFunctionCode::readCoils || functionCode == ModbusFunctionCode::readDiscreteInputs) {
    if (stationAddress == 0)
      return ESP_ERR_INVALID_RESPONSE;
    if (dataSize != 4)
      return WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalDataValue, transactionId);
    uint_fast16_t memoryAddress = __builtin_bswap16 (((uint16_t*)dataBuffer.data)[0]);
    uint_fast16_t numberOfMemoryItems = __builtin_bswap16 (((uint16_t*)dataBuffer.data)[1]);
    if (numberOfMemoryItems == 0 || numberOfMemoryItems > maxNumberOfModbusBitsToRead || dataBuffer.size < (numberOfMemoryItems - 1) / 8 + 2)
      return WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalDataValue, transactionId);
    if (memoryAddress > 0xFFFF - numberOfMemoryItems + 1)
      return WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalDataAddress, transactionId);
    
    ModbusMemoryType memoryType = (functionCode == ModbusFunctionCode::readCoils)?(ModbusMemoryType::coils):(ModbusMemoryType::discreteInputs);
    if (auto memoryArea = FindMemoryArea (memoryType, memoryAddress, numberOfMemoryItems)) {
      LockGuard lg (*memoryArea);
      memoryArea->OnRead();

      uint8_t* memoryData = (uint8_t*)memoryArea->data + (memoryAddress - memoryArea->address) / 8;
      uint_fast8_t memoryBitOffset = (memoryAddress - memoryArea->address) % 8;
      uint_fast8_t memorySize = (numberOfMemoryItems - 1) / 8 + 1;
      ((uint8_t*)dataBuffer.data)[0] = memorySize;
      for (uint_fast8_t i = 0; i < memorySize; i++) {
        uint_fast8_t byte = (memoryData[i] >> memoryBitOffset) | (memoryData[i + 1] << (8 -  memoryBitOffset));
        if (i == (memorySize - 1) && (numberOfMemoryItems % 8))
          byte &= lowerBits[numberOfMemoryItems % 8];
        ((uint8_t*)dataBuffer.data)[i + 1] = byte;
      }

      return WriteFrame (stream, stationAddress, functionCode, memorySize + 1, transactionId);
    }
    else
      return WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalDataAddress, transactionId);
  }

  if (functionCode == ModbusFunctionCode::readHoldingRegisters || functionCode == ModbusFunctionCode::readInputRegisters) {
    if (stationAddress == 0)
      return ESP_ERR_INVALID_RESPONSE;
    if (dataSize != 4)
      return WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalDataValue, transactionId);
    uint_fast16_t memoryAddress = __builtin_bswap16 (((uint16_t*)dataBuffer.data)[0]);
    uint_fast16_t numberOfMemoryItems = __builtin_bswap16 (((uint16_t*)dataBuffer.data)[1]);
    if (numberOfMemoryItems == 0 || numberOfMemoryItems > maxNumberOfModbusRegistersToRead || dataBuffer.size < numberOfMemoryItems * 2 + 1)
      return WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalDataValue, transactionId);
    if (memoryAddress > 0xFFFF - numberOfMemoryItems + 1)
      return WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalDataAddress, transactionId);
    
    ModbusMemoryType memoryType = (functionCode == ModbusFunctionCode::readHoldingRegisters)?(ModbusMemoryType::holdingRegisters):(ModbusMemoryType::inputRegisters);
    if (auto memoryArea = FindMemoryArea (memoryType, memoryAddress, numberOfMemoryItems)) {
      LockGuard lg (*memoryArea);
      memoryArea->OnRead();

      uint16_t* memoryData = (uint16_t*)memoryArea->data + (memoryAddress - memoryArea->address);
      ((uint8_t*)dataBuffer.data)[0] = numberOfMemoryItems * 2;
      for (uint_fast16_t i = 0; i < numberOfMemoryItems; i++)
        ((uint16_t*)((uint8_t*)dataBuffer.data + 1))[i] = __builtin_bswap16 (memoryData[i]);

      return WriteFrame (stream, stationAddress, functionCode, numberOfMemoryItems * 2 + 1, transactionId);
    }
    else
      return WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalDataAddress, transactionId);
  }

  if (functionCode == ModbusFunctionCode::writeSingleCoil || functionCode == ModbusFunctionCode::writeSingleHoldingRegister) {
    if (dataSize != 4)
      return stationAddress == 0 ? ESP_OK : WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalDataValue, transactionId);
    uint_fast16_t memoryAddress = __builtin_bswap16 (((uint16_t*)dataBuffer.data)[0]);
    uint_fast16_t memoryValue = __builtin_bswap16 (((uint16_t*)dataBuffer.data)[1]);
    if ((functionCode == ModbusFunctionCode::writeSingleCoil && memoryValue != 0 && memoryValue != 0xFF00) || dataBuffer.size < 4)
      return stationAddress == 0 ? ESP_OK : WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalDataValue, transactionId);
    
    ModbusMemoryType memoryType = (functionCode == ModbusFunctionCode::writeSingleCoil)?(ModbusMemoryType::coils):(ModbusMemoryType::holdingRegisters);
    if (auto memoryArea = FindMemoryArea (memoryType, memoryAddress, 1)) {
      LockGuard lg (*memoryArea);
      memoryArea->OnRead();

      if (memoryType == ModbusMemoryType::coils) {
        void* memoryData = (uint8_t*)memoryArea->data + (memoryAddress - memoryArea->address) / 8;
        uint_fast8_t memoryBitOffset = (memoryAddress - memoryArea->address) % 8;
        if (memoryValue == 0xFF00)
          *(uint8_t*)memoryData |= (1 << memoryBitOffset);
        else
          *(uint8_t*)memoryData &= ~(1 << memoryBitOffset);
      }        
      else
        *((uint16_t*)memoryArea->data + (memoryAddress - memoryArea->address)) = memoryValue;
    
      esp_err_t error = stationAddress == 0 ? ESP_OK : WriteFrame (stream, stationAddress, functionCode, 4, transactionId);
      memoryArea->OnWrite();
      return error;
    }
    else
      return stationAddress == 0 ? ESP_OK : WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalDataAddress, transactionId);
  }

  if (functionCode == ModbusFunctionCode::writeMultipleCoils) {
    if (dataSize < 5)
      return stationAddress == 0 ? ESP_OK : WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalDataValue, transactionId);
    uint_fast16_t memoryAddress = __builtin_bswap16 (((uint16_t*)dataBuffer.data)[0]);
    uint_fast16_t numberOfMemoryItems = __builtin_bswap16 (((uint16_t*)dataBuffer.data)[1]);
    uint_fast8_t memorySize = ((uint8_t*)dataBuffer.data)[4];
    if (numberOfMemoryItems == 0 || numberOfMemoryItems > maxNumberOfModbusBitsToWrite || dataSize != memorySize + 5 || memorySize != (numberOfMemoryItems - 1) / 8 + 1)
      return stationAddress == 0 ? ESP_OK : WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalDataValue, transactionId);
    if (memoryAddress > 0xFFFF - numberOfMemoryItems + 1)
      return stationAddress == 0 ? ESP_OK : WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalDataAddress, transactionId);
    
    ModbusMemoryType memoryType = ModbusMemoryType::coils;
    if (auto memoryArea = FindMemoryArea (memoryType, memoryAddress, numberOfMemoryItems)) {
      LockGuard lg (*memoryArea);
      memoryArea->OnRead();

      uint8_t* memoryData = (uint8_t*)memoryArea->data + (memoryAddress - memoryArea->address) / 8;
      uint_fast8_t memoryBitOffset = (memoryAddress - memoryArea->address) % 8;
      memorySize = (memoryBitOffset + numberOfMemoryItems - 1) / 8 + 1;
      for (uint_fast8_t i = 0; i < memorySize; i++) {
        uint_fast8_t memoryValue = 0;
        if (i)
          memoryValue = *(uint16_t*)((uint8_t*)dataBuffer.data + i + 4) >> (8 - memoryBitOffset);
        else
          memoryValue = (((uint8_t*)dataBuffer.data)[5] << memoryBitOffset) | (*memoryData & lowerBits[memoryBitOffset]);
        if (i == memorySize - 1 && (memoryBitOffset + numberOfMemoryItems) % 8) {
          memoryValue &= lowerBits[(memoryBitOffset + numberOfMemoryItems) % 8];
          memoryValue |= (memoryData[i] & ~(lowerBits[(memoryBitOffset + numberOfMemoryItems) % 8]));
        }
        memoryData[i] = memoryValue;
      }

      esp_err_t error = stationAddress == 0 ? ESP_OK : WriteFrame (stream, stationAddress, functionCode, 4, transactionId);
      memoryArea->OnWrite();
      return error;
    }
    else
      return stationAddress == 0 ? ESP_OK : WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalDataAddress, transactionId);
  }

  if (functionCode == ModbusFunctionCode::writeMultipleHoldingRegisters) {
    if (dataSize < 5)
      return stationAddress == 0 ? ESP_OK : WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalDataValue, transactionId);
    uint_fast16_t memoryAddress = __builtin_bswap16 (((uint16_t*)dataBuffer.data)[0]);
    uint_fast16_t numberOfMemoryItems = __builtin_bswap16 (((uint16_t*)dataBuffer.data)[1]);
    uint_fast8_t memorySize = ((uint8_t*)dataBuffer.data)[4];
    if (numberOfMemoryItems == 0 || numberOfMemoryItems > maxNumberOfModbusRegistersToWrite || dataSize != memorySize + 5 || memorySize != numberOfMemoryItems * 2)
      return stationAddress == 0 ? ESP_OK : WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalDataValue, transactionId);
    if (memoryAddress > 0xFFFF - numberOfMemoryItems + 1)
      return stationAddress == 0 ? ESP_OK : WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalDataAddress, transactionId);
    
    ModbusMemoryType memoryType = ModbusMemoryType::holdingRegisters;
    if (auto memoryArea = FindMemoryArea (memoryType, memoryAddress, numberOfMemoryItems)) {
      LockGuard lg (*memoryArea);
      memoryArea->OnRead();

      uint16_t* memoryData = (uint16_t*)memoryArea->data + (memoryAddress - memoryArea->address);
      for (uint_fast16_t i = 0; i < numberOfMemoryItems; i++)
          memoryData[i] = __builtin_bswap16 (((uint16_t*)((uint8_t*)dataBuffer.data + 5))[i]);

      esp_err_t error = stationAddress == 0 ? ESP_OK : WriteFrame (stream, stationAddress, functionCode, 4, transactionId);
      memoryArea->OnWrite();
      return error;
    }
    else
      return stationAddress == 0 ? ESP_OK : WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalDataAddress, transactionId);
  }
  
  return stationAddress == 0 ? ESP_OK : WriteExceptionFrame (stream, stationAddress, functionCode, ModbusException::illegalFunction, transactionId);
}

//==============================================================================

esp_err_t ModbusServer::WriteExceptionFrame (Stream& stream, uint8_t stationAddress, ModbusFunctionCode functionCode, ModbusException exception, uint16_t transactionId) {
  Buffer& dataBuffer = GetDataBuffer();
  if (dataBuffer.size < 1)
    return ESP_ERR_INVALID_SIZE;
  ((uint8_t*)dataBuffer.data)[0] = (uint8_t)exception;
  return WriteFrame (stream, stationAddress, (ModbusFunctionCode)((uint8_t)functionCode | 0x80), 1, transactionId);
}

//==============================================================================

ModbusServer::UartServer::UartServer (std::shared_ptr<UartPort> port, ModbusServer& modbusServer) : PL::UartServer (port), modbusServer (modbusServer) {}

//==============================================================================

esp_err_t ModbusServer::UartServer::HandleRequest (UartPort& port) {
  return modbusServer.HandleRequest (port);
}

//==============================================================================

ModbusServer::TcpServer::TcpServer (uint16_t port, ModbusServer& modbusServer) : PL::TcpServer (port), modbusServer (modbusServer) {}

//==============================================================================

esp_err_t ModbusServer::TcpServer::HandleRequest (NetworkStream& stream) {
  return modbusServer.HandleRequest (stream);
}

//==============================================================================

esp_err_t ModbusServer::HandleRequest (Stream& stream) {
  LockGuard lgServer (*this);
  uint8_t stationAddress;
  ModbusFunctionCode functionCode;
  Buffer& dataBuffer = GetDataBuffer();
  size_t dataSize;
  uint16_t transactionId;
  LockGuard lgBuffer (dataBuffer);

  esp_err_t error = ReadFrame (stream, stationAddress, functionCode, dataSize, transactionId);
  if ((error == ESP_OK || error == ESP_ERR_INVALID_SIZE) && stationAddress != this->stationAddress && stationAddress != 0)
    return ESP_OK;

  return error == ESP_OK ? HandleRequest (stream, stationAddress, functionCode, dataSize, transactionId) : error;
}

//==============================================================================

std::shared_ptr<ModbusMemoryArea> ModbusServer::FindMemoryArea (ModbusMemoryType memoryType, uint16_t address, uint16_t numberOfItems) {
  for (auto& memoryArea : memoryAreas) {
    if (memoryArea->type == memoryType && memoryArea->address <= address && memoryArea->address + memoryArea->numberOfItems >= address + numberOfItems)
      return memoryArea;
  }
  return NULL;
}

//==============================================================================

}