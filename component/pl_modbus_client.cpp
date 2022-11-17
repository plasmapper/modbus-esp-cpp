#include "pl_modbus_client.h"
#include "esp_check.h"

//==============================================================================

static const char* TAG = "pl_modbus_client";

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
  esp_err_t error = mutex.Lock (timeout);
  if (error == ESP_OK)
    return ESP_OK;
  if (error == ESP_ERR_TIMEOUT && timeout == 0)
    return ESP_ERR_TIMEOUT;
  ESP_RETURN_ON_ERROR (error, TAG, "mutex lock failed");
  return ESP_OK;
}

//==============================================================================

esp_err_t ModbusClient::Unlock() {
  ESP_RETURN_ON_ERROR (mutex.Unlock(), TAG, "mutex unlock failed");
  return ESP_OK;
}

//==============================================================================

esp_err_t ModbusClient::Command (ModbusFunctionCode functionCode, const void* requestData, size_t requestDataSize, void* responseData, size_t maxResponseDataSize, size_t* responseDataSize, ModbusException* exception) {
  LockGuard lgClient (*this);
  Buffer& dataBuffer = GetDataBuffer();
  LockGuard lgBuffer (dataBuffer);

  if (exception)
    *exception = ModbusException::noException;

  ESP_RETURN_ON_FALSE (dataBuffer.size >= requestDataSize, ESP_ERR_INVALID_SIZE, TAG, "buffer is too small");
  if (requestData)
    memcpy (dataBuffer.data, requestData, requestDataSize);
  size_t tempResponseDataSize; 
  ESP_RETURN_ON_ERROR (Command (functionCode, requestDataSize, tempResponseDataSize, exception), TAG, "command failed");
  ESP_RETURN_ON_FALSE (tempResponseDataSize <= maxResponseDataSize, ESP_ERR_INVALID_SIZE, TAG, "buffer is too small");
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

  ESP_RETURN_ON_FALSE (dataBuffer.size >= 4, ESP_ERR_INVALID_SIZE, TAG, "buffer is too small"); 
  ((uint16_t*)dataBuffer.data)[0] = __builtin_bswap16 (address);
  uint16_t u16Value = value ? 0x00FF : 0;
  ((uint16_t*)dataBuffer.data)[1] = u16Value;

  size_t responseDataSize; 
  ESP_RETURN_ON_ERROR (Command (ModbusFunctionCode::writeSingleCoil, 4, responseDataSize, exception), TAG, "command failed");
  if (stationAddress == 0)
    return ESP_OK;
  ESP_RETURN_ON_FALSE (responseDataSize == 4, ESP_ERR_INVALID_RESPONSE, TAG, "invalid response data size");
  ESP_RETURN_ON_FALSE (__builtin_bswap16 (((uint16_t*)dataBuffer.data)[0]) == address, ESP_ERR_INVALID_RESPONSE, TAG, "invalid response memory address");
  ESP_RETURN_ON_FALSE (((uint16_t*)dataBuffer.data)[1] == u16Value, ESP_ERR_INVALID_RESPONSE, TAG, "invalid response memory value");
  return ESP_OK;
}

//==============================================================================

esp_err_t ModbusClient::WriteSingleHoldingRegister (uint16_t address, uint16_t value, ModbusException* exception) {
  LockGuard lgClient (*this);
  Buffer& dataBuffer = GetDataBuffer();
  LockGuard lgBuffer (dataBuffer);

  if (exception)
    *exception = ModbusException::noException;

  ESP_RETURN_ON_FALSE (dataBuffer.size >= 4, ESP_ERR_INVALID_SIZE, TAG, "buffer is too small");
  ((uint16_t*)dataBuffer.data)[0] = __builtin_bswap16 (address);
  ((uint16_t*)dataBuffer.data)[1] = __builtin_bswap16 (value);

  size_t responseDataSize; 
  ESP_RETURN_ON_ERROR (Command (ModbusFunctionCode::writeSingleHoldingRegister, 4, responseDataSize, exception), TAG, "command failed");

  if (stationAddress == 0)
    return ESP_OK;
  ESP_RETURN_ON_FALSE (responseDataSize == 4, ESP_ERR_INVALID_RESPONSE, TAG, "invalid response data size");
  ESP_RETURN_ON_FALSE (__builtin_bswap16 (((uint16_t*)dataBuffer.data)[0]) == address, ESP_ERR_INVALID_RESPONSE, TAG, "invalid response memory address");
  ESP_RETURN_ON_FALSE (__builtin_bswap16 (((uint16_t*)dataBuffer.data)[1]) == value, ESP_ERR_INVALID_RESPONSE, TAG, "invalid response memory value");
  return ESP_OK;
}

//==============================================================================

esp_err_t ModbusClient::WriteMultipleCoils (uint16_t address, uint16_t numberOfItems, const void* requestData, ModbusException* exception) {
  LockGuard lgClient (*this);
  Buffer& dataBuffer = GetDataBuffer();
  LockGuard lgBuffer (dataBuffer);

  if (exception)
    *exception = ModbusException::noException;
  ESP_RETURN_ON_FALSE (requestData, ESP_ERR_INVALID_ARG, TAG, "requestData is null");

  for (auto& addressRange : SplitAddressRange (address, numberOfItems, maxNumberOfModbusBitsToWrite)) {
    size_t memoryDataSize = (addressRange.numberOfItems - 1) / 8 + 1;

    ESP_RETURN_ON_FALSE (dataBuffer.size >= memoryDataSize + 5, ESP_ERR_INVALID_SIZE, TAG, "buffer is too small");

    ((uint16_t*)dataBuffer.data)[0] = __builtin_bswap16 (addressRange.address);
    ((uint16_t*)dataBuffer.data)[1] = __builtin_bswap16 (addressRange.numberOfItems);
    ((uint8_t*)dataBuffer.data)[4] = memoryDataSize;
    if (requestData)
      memcpy ((uint8_t*)dataBuffer.data + 5, (uint8_t*)requestData + (addressRange.address - address) / 8, memoryDataSize);

    size_t responseDataSize;
    ESP_RETURN_ON_ERROR (Command (ModbusFunctionCode::writeMultipleCoils, memoryDataSize + 5, responseDataSize, exception), TAG, "command failed");
    if (stationAddress == 0)
      return ESP_OK;
    ESP_RETURN_ON_FALSE (responseDataSize == 4, ESP_ERR_INVALID_RESPONSE, TAG, "invalid response data size");
    ESP_RETURN_ON_FALSE (__builtin_bswap16 (((uint16_t*)dataBuffer.data)[0]) == addressRange.address, ESP_ERR_INVALID_RESPONSE, TAG, "invalid response memory address");
    ESP_RETURN_ON_FALSE (__builtin_bswap16 (((uint16_t*)dataBuffer.data)[1]) == addressRange.numberOfItems, ESP_ERR_INVALID_RESPONSE, TAG, "invalid response number of items");
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
  ESP_RETURN_ON_FALSE (requestData, ESP_ERR_INVALID_ARG, TAG, "requestData is null");

  for (auto& addressRange : SplitAddressRange (address, numberOfItems, maxNumberOfModbusRegistersToWrite)) {
    size_t memoryDataSize = addressRange.numberOfItems * 2;

    ESP_RETURN_ON_FALSE (dataBuffer.size >= memoryDataSize + 5, ESP_ERR_INVALID_SIZE, TAG, "buffer is too small");

    ((uint16_t*)dataBuffer.data)[0] = __builtin_bswap16 (addressRange.address);
    ((uint16_t*)dataBuffer.data)[1] = __builtin_bswap16 (addressRange.numberOfItems);
    ((uint8_t*)dataBuffer.data)[4] = memoryDataSize;

    if (requestData) {
      for (uint_fast16_t i = 0; i < addressRange.numberOfItems; i++)
        ((uint16_t*)((uint8_t*)dataBuffer.data + 5))[i] = __builtin_bswap16 (requestData[addressRange.address - address + i]);
    }

    size_t responseDataSize; 
    ESP_RETURN_ON_ERROR (Command (ModbusFunctionCode::writeMultipleHoldingRegisters, memoryDataSize + 5, responseDataSize, exception), TAG, "command failed");
    if (stationAddress == 0)
      return ESP_OK;
    ESP_RETURN_ON_FALSE (responseDataSize == 4, ESP_ERR_INVALID_RESPONSE, TAG, "invalid response data size");
    ESP_RETURN_ON_FALSE (__builtin_bswap16 (((uint16_t*)dataBuffer.data)[0]) == addressRange.address, ESP_ERR_INVALID_RESPONSE, TAG, "invalid response memory address");
    ESP_RETURN_ON_FALSE (__builtin_bswap16 (((uint16_t*)dataBuffer.data)[1]) == addressRange.numberOfItems, ESP_ERR_INVALID_RESPONSE, TAG, "invalid response number of items");
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
    if (dataBuffer.size >= 1) {
      ESP_RETURN_ON_ERROR (stream.Read (dataBuffer, 0, 1), TAG, "read exception failed");
      return ESP_OK;
    }
    else {
      stream.Read (NULL, 1);
      ESP_RETURN_ON_ERROR (ESP_ERR_INVALID_SIZE, TAG, "buffer is too small");
    }
  }

  switch (functionCode) {
    case ModbusFunctionCode::readCoils:
    case ModbusFunctionCode::readDiscreteInputs:
    case ModbusFunctionCode::readHoldingRegisters:
    case ModbusFunctionCode::readInputRegisters:
      if (dataBuffer.size >= 1) {
        ESP_RETURN_ON_ERROR (stream.Read (dataBuffer, 0, 1), TAG, "read byte size failed");
        dataSize = 1 + ((uint8_t*)dataBuffer.data)[0];
        if (dataBuffer.size >= dataSize) {
          ESP_RETURN_ON_ERROR (stream.Read (dataBuffer, 1, dataSize - 1), TAG, "read data failed");
          return ESP_OK;
        }
        else {
          stream.Read (NULL, dataSize - 1);
          ESP_RETURN_ON_ERROR (ESP_ERR_INVALID_SIZE, TAG, "buffer is too small");
          return ESP_OK;       
        }
      }
      else {
        uint8_t byteSize;
        if (stream.Read (&byteSize, 1) == ESP_OK)
          stream.Read (NULL, byteSize);
        ESP_RETURN_ON_ERROR (ESP_ERR_INVALID_SIZE, TAG, "buffer is too small");
        return ESP_OK;
      }
      break;
      
    case ModbusFunctionCode::writeSingleCoil:
    case ModbusFunctionCode::writeSingleHoldingRegister:
    case ModbusFunctionCode::writeMultipleCoils:
    case ModbusFunctionCode::writeMultipleHoldingRegisters:
      dataSize = 4;
      if (dataBuffer.size >= dataSize) {
        ESP_RETURN_ON_ERROR (stream.Read (dataBuffer, 0, dataSize), TAG, "read data failed");
        return ESP_OK;
      }
      else {
        stream.Read (NULL, dataSize);
        ESP_RETURN_ON_ERROR (ESP_ERR_INVALID_SIZE, TAG, "buffer is too small");
        return ESP_OK;
      }
      break;
            
    default:
      ESP_RETURN_ON_ERROR (ESP_ERR_NOT_SUPPORTED, TAG, "function code (%d) is not supported", (int)functionCode);
      return ESP_OK;
  }
}

//==============================================================================

esp_err_t ModbusClient::Command (ModbusFunctionCode functionCode, size_t requestDataSize, size_t& responseDataSize, ModbusException* exception) {
  if (interface == ModbusInterface::network) {
    ESP_RETURN_ON_ERROR (tcpClient->Connect(), TAG, "TCP client connect failed");
  }
  
  Stream& stream = (interface == ModbusInterface::uart) ? (Stream&)*uartPort : (Stream&)*tcpClient->GetStream();
  LockGuard lgStream (stream);

  Buffer& dataBuffer = GetDataBuffer();

  stream.Read (NULL, stream.GetReadableSize());

  ESP_RETURN_ON_ERROR (WriteFrame (stream, stationAddress, functionCode, requestDataSize, transactionId), TAG, "write frame failed");
  if (stationAddress == 0)
    return ESP_OK;

  uint8_t responseStationAddress;
  ModbusFunctionCode responseFunctionCode;
  uint16_t responseTransactionId;
  do {
    ESP_RETURN_ON_ERROR (ReadFrame (stream, responseStationAddress, responseFunctionCode, responseDataSize, responseTransactionId), TAG, "read frame failed");
  } while (responseTransactionId != transactionId);

  ESP_RETURN_ON_FALSE (responseStationAddress == stationAddress, ESP_ERR_INVALID_RESPONSE, TAG, "invalid response station address");
  ESP_RETURN_ON_FALSE ((uint8_t)functionCode == ((uint8_t)responseFunctionCode & 0x7F), ESP_ERR_INVALID_RESPONSE, TAG, "invalid response function code");
  
  if ((uint8_t)responseFunctionCode & 0x80) {
    ESP_RETURN_ON_FALSE (responseDataSize == 1, ESP_ERR_INVALID_RESPONSE, TAG, "invalid exception data size");
    if (exception)
      *exception = (ModbusException)(((uint8_t*)dataBuffer.data)[0]);
    ESP_RETURN_ON_ERROR (ESP_FAIL, TAG, "Modbus exception (%d)", (int)*exception);
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
  ESP_RETURN_ON_FALSE (stationAddress != 0, ESP_ERR_INVALID_ARG, TAG, "invalid station address");
  ESP_RETURN_ON_FALSE (dataBuffer.size >= 4, ESP_ERR_INVALID_SIZE, TAG, "buffer is too small");
  
  for (auto& addressRange : SplitAddressRange (address, numberOfItems, maxNumberOfModbusBitsToRead)) {
    ((uint16_t*)dataBuffer.data)[0] = __builtin_bswap16 (addressRange.address);
    ((uint16_t*)dataBuffer.data)[1] = __builtin_bswap16 (addressRange.numberOfItems);

    size_t responseDataSize; 
    size_t memoryDataSize = (addressRange.numberOfItems - 1) / 8 + 1;
    ESP_RETURN_ON_ERROR (Command (functionCode, 4, responseDataSize, exception), TAG, "command failed");
    ESP_RETURN_ON_FALSE (responseDataSize == memoryDataSize + 1, ESP_ERR_INVALID_RESPONSE, TAG, "invalid response data size");
    ESP_RETURN_ON_FALSE (((uint8_t*)dataBuffer.data)[0] == memoryDataSize, ESP_ERR_INVALID_RESPONSE, TAG, "invalid response byte size"); 
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
  ESP_RETURN_ON_FALSE (stationAddress != 0, ESP_ERR_INVALID_ARG, TAG, "invalid station address");
  ESP_RETURN_ON_FALSE (dataBuffer.size >= 4, ESP_ERR_INVALID_SIZE, TAG, "buffer is too small");
  
  for (auto& addressRange : SplitAddressRange (address, numberOfItems, maxNumberOfModbusRegistersToRead)) {
    ((uint16_t*)dataBuffer.data)[0] = __builtin_bswap16 (addressRange.address);
    ((uint16_t*)dataBuffer.data)[1] = __builtin_bswap16 (addressRange.numberOfItems);

    size_t responseDataSize; 
    size_t memoryDataSize = addressRange.numberOfItems * 2;
    ESP_RETURN_ON_ERROR (Command (functionCode, 4, responseDataSize, exception), TAG, "command failed");
    ESP_RETURN_ON_FALSE (responseDataSize == memoryDataSize + 1, ESP_ERR_INVALID_RESPONSE, TAG, "invalid response data size");
    ESP_RETURN_ON_FALSE (((uint8_t*)dataBuffer.data)[0] == memoryDataSize, ESP_ERR_INVALID_RESPONSE, TAG, "invalid response byte size"); 

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