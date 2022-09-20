#include "pl_modbus_client.h"

//==============================================================================

namespace PL {

//==============================================================================

ModbusClient::ModbusClient (std::shared_ptr<UartPort> port, ModbusProtocol protocol, uint8_t stationAddress, size_t bufferSize) :
    ModbusBase (protocol, bufferSize, defaultReadTimeout), interface (ModbusInterface::uart), uartPort (port),
    stationAddress (stationAddress) {}

//==============================================================================

ModbusClient::ModbusClient (IpV4Address address, uint16_t port, size_t bufferSize) :
    ModbusBase (defaultNetworkProtocol, bufferSize, defaultReadTimeout), interface (ModbusInterface::network), tcpClient (std::make_shared<TcpClient> (address, port)),
    stationAddress (defaultNetworkStationAddress) {
  tcpClient->DisableNagleAlgorithm();
}

//==============================================================================

ModbusClient::ModbusClient (IpV6Address address, uint16_t port, size_t bufferSize) :
    ModbusBase (defaultNetworkProtocol, bufferSize, defaultReadTimeout), interface (ModbusInterface::network), tcpClient (std::make_shared<TcpClient> (address, port)),
    stationAddress (defaultNetworkStationAddress) {
  tcpClient->DisableNagleAlgorithm();
}

//==============================================================================

ModbusClient::ModbusClient (std::shared_ptr<TcpClient> tcpClient, size_t bufferSize) :
    ModbusBase (defaultNetworkProtocol, bufferSize, defaultReadTimeout), interface (ModbusInterface::network), tcpClient (tcpClient),
    stationAddress (defaultNetworkStationAddress) {
  tcpClient->DisableNagleAlgorithm();
}


//==============================================================================

esp_err_t ModbusClient::Lock (TickType_t timeout) {
  return mutex.Lock (timeout);
}

//==============================================================================

esp_err_t ModbusClient::Unlock() {
  return mutex.Unlock();
}

//==============================================================================

esp_err_t ModbusClient::Command (ModbusFunctionCode functionCode, const void* requestData, size_t requestDataSize, void* responseData, size_t maxResponseDataSize, size_t* responseDataSize, ModbusException* exception) {
  LockGuard lgClient (*this);
  Buffer& dataBuffer = GetDataBuffer();
  LockGuard lgBuffer (dataBuffer);

  if (exception)
    *exception = ModbusException::noException;

  if (dataBuffer.size < requestDataSize)
    return ESP_ERR_INVALID_SIZE;
  if (requestData)
    memcpy (dataBuffer.data, requestData, requestDataSize);
  size_t tempResponseDataSize; 
  PL_RETURN_ON_ERROR (Command (functionCode, requestDataSize, tempResponseDataSize, exception));
  if (tempResponseDataSize > maxResponseDataSize)
    return ESP_ERR_INVALID_SIZE;
  if (responseDataSize)
    *responseDataSize = tempResponseDataSize;
  if (responseData)
    memcpy (responseData, dataBuffer.data, tempResponseDataSize);
  return ESP_OK;
}

//==============================================================================

esp_err_t ModbusClient::ReadCoils (uint16_t address, uint16_t numberOfItems, void* responseData, ModbusException* exception) {
  return ReadBits (ModbusFunctionCode::readCoils, address, numberOfItems, responseData, exception);
}

//==============================================================================

esp_err_t ModbusClient::ReadDiscreteInputs (uint16_t address, uint16_t numberOfItems, void* responseData, ModbusException* exception) {
  return ReadBits (ModbusFunctionCode::readDiscreteInputs, address, numberOfItems, responseData, exception);
}

//==============================================================================

esp_err_t ModbusClient::ReadHoldingRegisters (uint16_t address, uint16_t numberOfItems, uint16_t* responseData, ModbusException* exception) {
  return ReadRegisters (ModbusFunctionCode::readHoldingRegisters, address, numberOfItems, responseData, exception);
}

//==============================================================================

esp_err_t ModbusClient::ReadInputRegisters (uint16_t address, uint16_t numberOfItems, uint16_t* responseData, ModbusException* exception) {
  return ReadRegisters (ModbusFunctionCode::readInputRegisters, address, numberOfItems, responseData, exception);
}

//==============================================================================

esp_err_t ModbusClient::WriteSingleCoil (uint16_t address, bool value, ModbusException* exception) {
  LockGuard lgClient (*this);
  Buffer& dataBuffer = GetDataBuffer();
  LockGuard lgBuffer (dataBuffer);

  if (exception)
    *exception = ModbusException::noException;

  if (dataBuffer.size < 4)
    return ESP_ERR_INVALID_SIZE;
  ((uint16_t*)dataBuffer.data)[0] = __builtin_bswap16 (address);
  uint16_t u16Value = value ? 0x00FF : 0;
  ((uint16_t*)dataBuffer.data)[1] = u16Value;

  size_t responseDataSize; 
  PL_RETURN_ON_ERROR (Command (ModbusFunctionCode::writeSingleCoil, 4, responseDataSize, exception));

  if (stationAddress != 0 && (responseDataSize != 4 || __builtin_bswap16 (((uint16_t*)dataBuffer.data)[0]) != address || ((uint16_t*)dataBuffer.data)[1] != u16Value))
    return ESP_ERR_INVALID_RESPONSE;
  return ESP_OK;
}

//==============================================================================

esp_err_t ModbusClient::WriteSingleHoldingRegister (uint16_t address, uint16_t value, ModbusException* exception) {
  LockGuard lgClient (*this);
  Buffer& dataBuffer = GetDataBuffer();
  LockGuard lgBuffer (dataBuffer);

  if (exception)
    *exception = ModbusException::noException;

  if (dataBuffer.size < 4)
    return ESP_ERR_INVALID_SIZE;
  ((uint16_t*)dataBuffer.data)[0] = __builtin_bswap16 (address);
  ((uint16_t*)dataBuffer.data)[1] = __builtin_bswap16 (value);

  size_t responseDataSize; 
  PL_RETURN_ON_ERROR (Command (ModbusFunctionCode::writeSingleHoldingRegister, 4, responseDataSize, exception));

  if (stationAddress != 0 && (responseDataSize != 4 || __builtin_bswap16 (((uint16_t*)dataBuffer.data)[0]) != address || __builtin_bswap16 (((uint16_t*)dataBuffer.data)[1]) != value))
    return ESP_ERR_INVALID_RESPONSE;
  return ESP_OK;
}

//==============================================================================

esp_err_t ModbusClient::WriteMultipleCoils (uint16_t address, uint16_t numberOfItems, const void* requestData, ModbusException* exception) {
  LockGuard lgClient (*this);
  Buffer& dataBuffer = GetDataBuffer();
  LockGuard lgBuffer (dataBuffer);

  if (exception)
    *exception = ModbusException::noException;
  if (!requestData)
    return ESP_ERR_INVALID_ARG;

  for (auto& addressRange : SplitAddressRange (address, numberOfItems, maxNumberOfModbusBitsToWrite)) {
    size_t memoryDataSize = (addressRange.numberOfItems - 1) / 8 + 1;

    if (dataBuffer.size < memoryDataSize + 5)
      return ESP_ERR_INVALID_SIZE;

    ((uint16_t*)dataBuffer.data)[0] = __builtin_bswap16 (addressRange.address);
    ((uint16_t*)dataBuffer.data)[1] = __builtin_bswap16 (addressRange.numberOfItems);
    ((uint8_t*)dataBuffer.data)[4] = memoryDataSize;
    if (requestData)
      memcpy ((uint8_t*)dataBuffer.data + 5, (uint8_t*)requestData + (addressRange.address - address) / 8, memoryDataSize);

    size_t responseDataSize; 
    PL_RETURN_ON_ERROR (Command (ModbusFunctionCode::writeMultipleCoils, memoryDataSize + 5, responseDataSize, exception));
    if (stationAddress != 0 && (responseDataSize != 4 || __builtin_bswap16 (((uint16_t*)dataBuffer.data)[0]) != addressRange.address || __builtin_bswap16 (((uint16_t*)dataBuffer.data)[1]) != addressRange.numberOfItems))
      return ESP_ERR_INVALID_RESPONSE;
  }
  return ESP_OK;
}

//==============================================================================

esp_err_t ModbusClient::WriteMultipleHoldingRegisters (uint16_t address, uint16_t numberOfItems, const uint16_t* requestData, ModbusException* exception) {
  LockGuard lgClient (*this);
  Buffer& dataBuffer = GetDataBuffer();
  LockGuard lgBuffer (dataBuffer);

  if (exception)
    *exception = ModbusException::noException;
  if (!requestData)
    return ESP_ERR_INVALID_ARG;

  for (auto& addressRange : SplitAddressRange (address, numberOfItems, maxNumberOfModbusRegistersToWrite)) {
    size_t memoryDataSize = addressRange.numberOfItems * 2;

    if (dataBuffer.size < memoryDataSize + 5)
      return ESP_ERR_INVALID_SIZE;

    ((uint16_t*)dataBuffer.data)[0] = __builtin_bswap16 (addressRange.address);
    ((uint16_t*)dataBuffer.data)[1] = __builtin_bswap16 (addressRange.numberOfItems);
    ((uint8_t*)dataBuffer.data)[4] = memoryDataSize;

    if (requestData) {
      for (uint_fast16_t i = 0; i < addressRange.numberOfItems; i++)
        ((uint16_t*)((uint8_t*)dataBuffer.data + 5))[i] = __builtin_bswap16 (requestData[addressRange.address - address + i]);
    }

    size_t responseDataSize; 
    PL_RETURN_ON_ERROR (Command (ModbusFunctionCode::writeMultipleHoldingRegisters, memoryDataSize + 5, responseDataSize, exception));
    if (stationAddress != 0 && (responseDataSize != 4 || __builtin_bswap16 (((uint16_t*)dataBuffer.data)[0]) != addressRange.address || __builtin_bswap16 (((uint16_t*)dataBuffer.data)[1]) != addressRange.numberOfItems))
      return ESP_ERR_INVALID_RESPONSE;

  }
  return ESP_OK;
}

//==============================================================================

uint8_t ModbusClient::GetStationAddress() {
  LockGuard lg (*this);
  return stationAddress;
}

//==============================================================================

esp_err_t ModbusClient::SetStationAddress (uint8_t stationAddress) {
  LockGuard lg (*this);
  this->stationAddress = stationAddress;
  return ESP_OK;
}

//==============================================================================

esp_err_t ModbusClient::ReadRtuData (Stream& stream, ModbusFunctionCode functionCode, size_t& dataSize) {
  Buffer& dataBuffer = GetDataBuffer();

  if ((uint8_t)functionCode & 0x80) {
    dataSize = 1;
    if (dataBuffer.size >= 1)
      return stream.Read (dataBuffer, 0, 1);
    else {
      stream.Read (NULL, 1);
      return ESP_ERR_INVALID_SIZE;
    }
  }

  switch (functionCode) {
    case ModbusFunctionCode::readCoils:
    case ModbusFunctionCode::readDiscreteInputs:
    case ModbusFunctionCode::readHoldingRegisters:
    case ModbusFunctionCode::readInputRegisters:
      if (dataBuffer.size >= 1) {
        PL_RETURN_ON_ERROR (stream.Read (dataBuffer, 0, 1));
        dataSize = 1 + ((uint8_t*)dataBuffer.data)[0];
        if (dataBuffer.size >= dataSize)
          return stream.Read (dataBuffer, 1, dataSize - 1);
        else {
          stream.Read (NULL, dataSize - 1);
          return ESP_ERR_INVALID_SIZE;          
        }
      }
      else {
        uint8_t byteSize;
        if (stream.Read (&byteSize, 1) == ESP_OK)
          stream.Read (NULL, byteSize);
        return ESP_ERR_INVALID_SIZE;
      }
      
    case ModbusFunctionCode::writeSingleCoil:
    case ModbusFunctionCode::writeSingleHoldingRegister:
    case ModbusFunctionCode::writeMultipleCoils:
    case ModbusFunctionCode::writeMultipleHoldingRegisters:
      dataSize = 4;
      if (dataBuffer.size >= dataSize)
        return stream.Read (dataBuffer, 0, dataSize);
      else {
        stream.Read (NULL, dataSize);
        return ESP_ERR_INVALID_SIZE;
      }
            
    default:
      return ESP_ERR_NOT_SUPPORTED;  
  }
}

//==============================================================================

esp_err_t ModbusClient::Command (ModbusFunctionCode functionCode, size_t requestDataSize, size_t& responseDataSize, ModbusException* exception) {
  if (interface == ModbusInterface::network) {
    PL_RETURN_ON_ERROR (tcpClient->Connect());
  }
  
  Stream& stream = (interface == ModbusInterface::uart) ? (Stream&)*uartPort : (Stream&)*tcpClient->GetStream();
  LockGuard lgStream (stream);

  Buffer& dataBuffer = GetDataBuffer();

  stream.Read (NULL, stream.GetReadableSize());

  PL_RETURN_ON_ERROR (WriteFrame (stream, stationAddress, functionCode, requestDataSize, transactionId));
  if (stationAddress == 0)
    return ESP_OK;

  uint8_t responseStationAddress;
  ModbusFunctionCode responseFunctionCode;
  uint16_t responseTransactionId;
  do {
    PL_RETURN_ON_ERROR (ReadFrame (stream, responseStationAddress, responseFunctionCode, responseDataSize, responseTransactionId));
  } while (responseTransactionId != transactionId);

  if ((uint8_t)functionCode != ((uint8_t)responseFunctionCode & 0x7F) || responseStationAddress != stationAddress)
    return ESP_ERR_INVALID_RESPONSE;
  
  if ((uint8_t)responseFunctionCode & 0x80) {
    if (responseDataSize != 1)
      return ESP_ERR_INVALID_RESPONSE;
    if (exception)
      *exception = (ModbusException)(((uint8_t*)dataBuffer.data)[0]);
    return ESP_FAIL;
  }

  return ESP_OK;
}

//==============================================================================

esp_err_t ModbusClient::ReadBits (ModbusFunctionCode functionCode, uint16_t address, uint16_t numberOfItems, void* responseData, ModbusException* exception) {
  LockGuard lgClient (*this);
  Buffer& dataBuffer = GetDataBuffer();
  LockGuard lgBuffer (dataBuffer);

  if (exception)
    *exception = ModbusException::noException;
  if (stationAddress == 0)
    return ESP_ERR_INVALID_ARG;
  if (dataBuffer.size < 4)
    return ESP_ERR_INVALID_SIZE;
  
  for (auto& addressRange : SplitAddressRange (address, numberOfItems, maxNumberOfModbusBitsToRead)) {
    ((uint16_t*)dataBuffer.data)[0] = __builtin_bswap16 (addressRange.address);
    ((uint16_t*)dataBuffer.data)[1] = __builtin_bswap16 (addressRange.numberOfItems);

    size_t responseDataSize; 
    size_t memoryDataSize = (addressRange.numberOfItems - 1) / 8 + 1;
    PL_RETURN_ON_ERROR (Command (functionCode, 4, responseDataSize, exception));

    if (responseDataSize != memoryDataSize + 1 || ((uint8_t*)dataBuffer.data)[0] != memoryDataSize)
      return ESP_ERR_INVALID_RESPONSE;

    if (responseData)
      memcpy ((uint8_t*)responseData + (addressRange.address - address) / 8, (uint8_t*)dataBuffer.data + 1, memoryDataSize);
  }
  return ESP_OK;
}

//==============================================================================

esp_err_t ModbusClient::ReadRegisters (ModbusFunctionCode functionCode, uint16_t address, uint16_t numberOfItems, uint16_t* responseData, ModbusException* exception) {
  LockGuard lgClient (*this);
  Buffer& dataBuffer = GetDataBuffer();
  LockGuard lgBuffer (dataBuffer);

  if (exception)
    *exception = ModbusException::noException;
  if (stationAddress == 0)
    return ESP_ERR_INVALID_ARG;
  if (dataBuffer.size < 4)
    return ESP_ERR_INVALID_SIZE;
  
  for (auto& addressRange : SplitAddressRange (address, numberOfItems, maxNumberOfModbusRegistersToRead)) {
    ((uint16_t*)dataBuffer.data)[0] = __builtin_bswap16 (addressRange.address);
    ((uint16_t*)dataBuffer.data)[1] = __builtin_bswap16 (addressRange.numberOfItems);

    size_t responseDataSize; 
    size_t memoryDataSize = addressRange.numberOfItems * 2;
    PL_RETURN_ON_ERROR (Command (functionCode, 4, responseDataSize, exception));

    if (responseDataSize != memoryDataSize + 1 || ((uint8_t*)dataBuffer.data)[0] != memoryDataSize)
      return ESP_ERR_INVALID_RESPONSE;

    if (responseData) {
      for (uint_fast16_t i = 0; i < addressRange.numberOfItems; i++)
        responseData[addressRange.address - address + i] = __builtin_bswap16 (((uint16_t*)((uint8_t*)dataBuffer.data + 1))[i]);
    }
  }
  return ESP_OK;
}

//==============================================================================

std::vector<ModbusClient::AddressRange> ModbusClient::SplitAddressRange (uint16_t address, uint16_t numberOfItems, uint16_t maxNumberOfItems) {
  std::vector<ModbusClient::AddressRange> addressRanges;
  numberOfItems = std::min ((int)numberOfItems, 0xFFFF - address + 1);

  if (numberOfItems <= maxNumberOfItems) {
    addressRanges.push_back ({address, numberOfItems});
    return addressRanges;
  }

  int numberOfAddressRanges = (numberOfItems - 1) / maxNumberOfItems + 1;
  for (int i = 0; i < numberOfAddressRanges; i++)
    addressRanges.push_back ({(uint16_t)(address + i * maxNumberOfItems), (uint16_t)(i < (numberOfAddressRanges - 1) ? maxNumberOfItems : ((numberOfItems - 1) % maxNumberOfItems + 1))});
  return addressRanges;
}

//==============================================================================
  
}