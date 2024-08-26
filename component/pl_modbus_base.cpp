#include "pl_modbus_base.h"
#include "esp_check.h"

//==============================================================================

static const char* TAG = "pl_modbus_base";

//==============================================================================

namespace PL {

//==============================================================================

const uint16_t crcTable[] = {
  0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
  0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
  0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
  0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
  0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
  0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
  0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
  0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
  0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
  0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
  0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
  0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
  0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
  0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
  0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
  0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
  0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
  0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
  0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
  0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
  0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
  0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
  0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
  0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
  0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
  0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
  0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
  0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
  0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
  0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
  0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
  0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040
};

//==============================================================================

ModbusProtocol ModbusBase::GetProtocol() {
  LockGuard lg(*this);
  return protocol;
}

//==============================================================================

esp_err_t ModbusBase::SetProtocol(ModbusProtocol protocol) {
  LockGuard lg(*this);
  ESP_RETURN_ON_FALSE(protocol == ModbusProtocol::rtu || protocol == ModbusProtocol::ascii || protocol == ModbusProtocol::tcp, ESP_ERR_INVALID_ARG, TAG, "invalid protocol");
  this->protocol = protocol;
  InitializeDataBuffer();
  return ESP_OK;
}

//==============================================================================

TickType_t ModbusBase::GetReadTimeout() {
  LockGuard lg(*this);
  return readTimeout;
}

//==============================================================================

esp_err_t ModbusBase::SetReadTimeout(TickType_t timeout) {
  LockGuard lg(*this);
  readTimeout = timeout;
  return ESP_OK;
}

//==============================================================================

TickType_t ModbusBase::GetDelayAfterRead() {
  LockGuard lg(*this);
  return delayAfterRead;
}

//==============================================================================

esp_err_t ModbusBase::SetDelayAfterRead(TickType_t delay) {
  LockGuard lg(*this);
  delayAfterRead = delay;
  return ESP_OK;
}

//==============================================================================

ModbusBase::ModbusBase(ModbusProtocol protocol, std::shared_ptr<Buffer> buffer, TickType_t readTimeout) : protocol(protocol), buffer(buffer), readTimeout(readTimeout) {
  if (protocol != ModbusProtocol::rtu && protocol != ModbusProtocol::ascii && protocol != ModbusProtocol::tcp)
    this->protocol = ModbusProtocol::rtu;
  InitializeDataBuffer();
}

//==============================================================================

ModbusBase::ModbusBase(ModbusProtocol protocol, size_t bufferSize, TickType_t readTimeout) : ModbusBase(protocol, std::make_shared<Buffer>(bufferSize), readTimeout) {}

//==============================================================================

esp_err_t ModbusBase::ReadFrame(Stream& stream, uint8_t& stationAddress, ModbusFunctionCode& functionCode, size_t& dataSize, uint16_t& transactionId) {
  esp_err_t error;
  ESP_RETURN_ON_ERROR(stream.SetReadTimeout(readTimeout), TAG, "set timeout failed");

  if (protocol == ModbusProtocol::rtu) {
    transactionId = 0;
    ESP_RETURN_ON_ERROR(stream.Read(&stationAddress, 1), TAG, "read station address failed");
    ESP_RETURN_ON_ERROR(stream.Read(&functionCode, 1), TAG, "read function code failed");
    if ((error = ReadRtuData(stream, functionCode, dataSize)) == ESP_OK) {
      uint16_t crc;
      ESP_RETURN_ON_ERROR(stream.Read(&crc, 2), TAG, "read crc failed");
      vTaskDelay(delayAfterRead);
      if (buffer->size >= dataSize + 2) {
        ((uint8_t*)buffer->data)[0] = stationAddress;
        ((uint8_t*)buffer->data)[1] = (uint8_t)functionCode;
        ESP_RETURN_ON_FALSE(Crc(dataSize + 2) == crc, ESP_ERR_INVALID_CRC, TAG, "invalid crc");
        return ESP_OK;
      }
      else {
        ESP_RETURN_ON_ERROR(ESP_ERR_INVALID_SIZE, TAG, "buffer is too small");
      }
    }
    else {
      if (error == ESP_ERR_INVALID_SIZE)
        stream.Read(NULL, 2);
      vTaskDelay(delayAfterRead);
      ESP_RETURN_ON_ERROR(error, TAG, "read RTU data failed");
      return ESP_OK;
    }
  }

  if (protocol == ModbusProtocol::ascii) {
    transactionId = 0;
    ESP_RETURN_ON_ERROR(stream.ReadUntil(':'), TAG, "read ':' failed");
    for (size_t i = 0; i < buffer->size; i++) {
      uint8_t asciiData[2];
      ESP_RETURN_ON_ERROR(stream.Read(asciiData, 2), TAG, "read ASCII failed");
      if (asciiData[0] == '\r' && asciiData[1] == '\n') {
        dataSize = (i >= 3) ? (i - 3) : 0;
        vTaskDelay(delayAfterRead);
        ESP_RETURN_ON_FALSE(i >= 3, ESP_ERR_INVALID_RESPONSE, TAG, "invalid request");
        ESP_RETURN_ON_FALSE(Lrc(i) == 0, ESP_ERR_INVALID_CRC, TAG, "invalid crc");
        return ESP_OK;
      }
      else {
        if (asciiData[0] < '0' || (asciiData[0] > '9' && asciiData[0] < 'A') || asciiData[0] > 'F' ||
            asciiData[1] < '0' || (asciiData[1] > '9' && asciiData[1] < 'A') || asciiData[1] > 'F') {
          ESP_RETURN_ON_ERROR(stream.ReadUntil('\n'), TAG, "read until \\n failed");
          vTaskDelay(delayAfterRead);
          ESP_RETURN_ON_ERROR(ESP_ERR_INVALID_RESPONSE, TAG, "invalid ASCII data");
        }
        ((uint8_t*)buffer->data)[i] = ((asciiData[0] - ((asciiData[0] < 'A')?('0'):('A' - 10))) << 4);
        ((uint8_t*)buffer->data)[i] += (asciiData[1] - ((asciiData[1] < 'A')?('0'):('A' - 10)));
        if (i == 0)
          stationAddress = ((uint8_t*)buffer->data)[0];
        if (i == 1)
          functionCode = (ModbusFunctionCode)((uint8_t*)buffer->data)[1];
      }
    }
    ESP_RETURN_ON_ERROR(stream.ReadUntil('\n'), TAG, "read until \\n failed");
    vTaskDelay(delayAfterRead);
    ESP_RETURN_ON_ERROR(ESP_ERR_INVALID_SIZE, TAG, "buffer is too small");
  }

  if (protocol == ModbusProtocol::tcp) {
    uint16_t protocolId;
    uint16_t tcpDataLength;
    ESP_RETURN_ON_ERROR(stream.Read(&transactionId, 2), TAG, "read transaction ID failed");
    transactionId = __builtin_bswap16(transactionId);
    
    ESP_RETURN_ON_ERROR(stream.Read(&protocolId, 2), TAG, "read protocol ID failed");
    if (protocolId != 0) {
      stream.FlushReadBuffer(2);
      vTaskDelay(delayAfterRead);
      ESP_RETURN_ON_ERROR(ESP_ERR_INVALID_RESPONSE, TAG, "invalid protocol id");
    }
    
    ESP_RETURN_ON_ERROR(stream.Read(&tcpDataLength, 2), TAG, "read data length failed");
    tcpDataLength = __builtin_bswap16(tcpDataLength);
    if (tcpDataLength < 2) {
      stream.Read(NULL, tcpDataLength);
      vTaskDelay(delayAfterRead);
      ESP_RETURN_ON_ERROR(ESP_ERR_INVALID_RESPONSE, TAG, "invalid data length");
    }

    ESP_RETURN_ON_ERROR(stream.Read(&stationAddress, 1), TAG, "read station address failed");
    ESP_RETURN_ON_ERROR(stream.Read(&functionCode, 1), TAG, "read function code failed");
    dataSize = tcpDataLength - 2;
    if (buffer->size >= dataSize + 8) {
      ESP_RETURN_ON_ERROR(stream.Read(*buffer, 8, dataSize), TAG, "read data failed");
      vTaskDelay(delayAfterRead);
      return ESP_OK;
    }
    else {
      ESP_RETURN_ON_ERROR(stream.Read(NULL, dataSize), TAG, "read data failed");
      vTaskDelay(delayAfterRead);
      ESP_RETURN_ON_ERROR(ESP_ERR_INVALID_SIZE, TAG, "buffer is too small");
    }
  }

  ESP_RETURN_ON_ERROR(ESP_ERR_NOT_SUPPORTED, TAG, "protocol is not supported");
  return ESP_OK;
}

//==============================================================================

esp_err_t ModbusBase::WriteFrame(Stream& stream, uint8_t stationAddress, ModbusFunctionCode functionCode, size_t dataSize, uint16_t transactionId) {
  if (protocol != ModbusProtocol::tcp && stream.GetReadableSize())
    return ESP_ERR_INVALID_STATE;

  if (protocol == ModbusProtocol::rtu) {
    ESP_RETURN_ON_FALSE(buffer->size >= dataSize + 4, ESP_ERR_INVALID_SIZE, TAG, "buffer is too small");
    ((uint8_t*)buffer->data)[0] = stationAddress;
    ((uint8_t*)buffer->data)[1] = (uint8_t)functionCode;
    *(uint16_t*)((uint8_t*)buffer->data + 2 + dataSize) = Crc(2 + dataSize);

    /*uint32_t msDelay = 2;
    if (auto uart = dynamic_cast<Uart*>(&stream)) {
      uint32_t baudrate = uart->GetBaudRate();
      if (baudrate > 0 && baudrate < 19200)
        msDelay = 28000 / baudrate + 1;
    }
    ets_delay_us(msDelay * 1000);*/
    ESP_RETURN_ON_ERROR(stream.Write(*buffer, 0, dataSize + 4), TAG, "stream write error");
    return ESP_OK;
  }

  if (protocol == ModbusProtocol::ascii) {
    ESP_RETURN_ON_FALSE(buffer->size >= dataSize * 2 + 9, ESP_ERR_INVALID_SIZE, TAG, "buffer is too small");
    ((uint8_t*)buffer->data)[0] = stationAddress;
    ((uint8_t*)buffer->data)[1] = (uint8_t)functionCode;
    ((uint8_t*)buffer->data)[dataSize + 2] = Lrc(dataSize + 2);
    for (int i = dataSize + 2; i >= 0; i--) {
      uint8_t byteData = ((uint8_t*)buffer->data)[i] >> 4;
      ((uint8_t*)buffer->data)[i * 2 + 1] = (byteData > 9)?(byteData - 10 + 'A'):(byteData + '0');
      byteData = ((uint8_t*)buffer->data)[i] & 0x0F;
      ((uint8_t*)buffer->data)[i * 2 + 2] = (byteData > 9)?(byteData - 10 + 'A'):(byteData + '0');
    }
    ((uint8_t*)buffer->data)[0] = ':';
    ((uint8_t*)buffer->data)[dataSize * 2 + 7] = '\r';
    ((uint8_t*)buffer->data)[dataSize * 2 + 8] = '\n';

    ESP_RETURN_ON_ERROR(stream.Write(*buffer, 0, dataSize * 2 + 9), TAG, "stream write error");
    return ESP_OK;
  }

  if (protocol == ModbusProtocol::tcp) {
    ESP_RETURN_ON_FALSE(buffer->size >= dataSize + 8 && dataSize <= 0xFFFD, ESP_ERR_INVALID_SIZE, TAG, "buffer is too small");
    ((uint16_t*)buffer->data)[0] = __builtin_bswap16(transactionId);
    ((uint16_t*)buffer->data)[1] = 0;
    ((uint16_t*)buffer->data)[2] = __builtin_bswap16((uint16_t)(dataSize + 2));
    ((uint8_t*)buffer->data)[6] = stationAddress;
    ((uint8_t*)buffer->data)[7] = (uint8_t)functionCode;

    ESP_RETURN_ON_ERROR(stream.Write(*buffer, 0, dataSize + 8), TAG, "stream write error");
    return ESP_OK;
  }

  return ESP_ERR_NOT_SUPPORTED;
}

//==============================================================================

Buffer& ModbusBase::GetDataBuffer() {
  return *dataBuffer;
}

//==============================================================================

uint16_t ModbusBase::Crc(size_t size) {
  uint_fast16_t crc = 0xFFFF;
  uint_fast8_t crcTableIndex;
  for (size_t i = 0; i < size; i++) {
    crcTableIndex = (crc & 0xFF) ^ (((uint8_t*)buffer->data)[i]);
    crc >>= 8;
    crc ^= crcTable[crcTableIndex];
  }
  return crc;  
}

//==============================================================================

uint8_t ModbusBase::Lrc(size_t size) {
  uint_fast8_t lrc = 0;
  for (size_t i = 0; i < size; i++)
    lrc += ((uint8_t*)buffer->data)[i];
  return ~lrc + 1;
}

//==============================================================================

void ModbusBase::InitializeDataBuffer() {
  if (protocol == ModbusProtocol::rtu)
    dataBuffer = std::make_shared<Buffer>((uint8_t*)buffer->data + 2, buffer->size >= 4 ? (buffer->size - 4) : 0, buffer);
  if (protocol == ModbusProtocol::ascii)
    dataBuffer = std::make_shared<Buffer>((uint8_t*)buffer->data + 2, buffer->size >= 9 ? ((buffer->size - 9) / 2) : 0, buffer);
  if (protocol == ModbusProtocol::tcp)
    dataBuffer = std::make_shared<Buffer>((uint8_t*)buffer->data + 8, buffer->size >= 8 ? (buffer->size - 8) : 0, buffer);
}

//==============================================================================

}