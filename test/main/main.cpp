#include "unity.h"
#include "pl_modbus.h"

//==============================================================================

class Server : public PL::ModbusServer {
public:
  using PL::ModbusServer::ModbusServer;

  esp_err_t ReadRtuData(PL::Stream& stream, PL::ModbusFunctionCode functionCode, size_t& dataSize) override;
  esp_err_t HandleRequest(PL::Stream& stream, uint8_t stationAddress, PL::ModbusFunctionCode functionCode, size_t dataSize, uint16_t transactionId) override;
};

//==============================================================================

class Client : public PL::ModbusClient {
public:
  using PL::ModbusClient::ModbusClient;

  esp_err_t ReadRtuData(PL::Stream& stream, PL::ModbusFunctionCode functionCode, size_t& dataSize) override;
};

//==============================================================================

const uint16_t port = 502;
const size_t serverBufferSize = 1000;
const uint8_t stationAddress = 100;

Server server(port, serverBufferSize);
Client client(PL::IpV4Address(127, 0, 0, 1), port, serverBufferSize);
std::vector<PL::ModbusProtocol> protocols = {PL::ModbusProtocol::rtu, PL::ModbusProtocol::ascii, PL::ModbusProtocol::tcp};
std::vector<std::string> protocolNames = {"RTU", "ASCII", "TCP"};

// Coils, digital inputs, holding registers and input registers all mapped to the same area.
// 250 holding/input registers and 4000 coils/digital inputs).
const size_t numberOfRegisters = 250;
const size_t numberOfBits = numberOfRegisters * 16;
auto serverHR = std::make_shared<PL::ModbusMemoryArea>(PL::ModbusMemoryType::holdingRegisters, 0, numberOfRegisters * 2);
const size_t numberOfIterations = 100;

const PL::ModbusFunctionCode userDefinedFunctionCode = (PL::ModbusFunctionCode)100;
uint32_t userDefinedFunctionRequest;
uint32_t userDefinedFunctionResponse[10];

//==============================================================================

void TestErrors();
void TestReadCoils();
void TestReadDiscreteInputs();
void TestReadHoldingRegisters();
void TestReadInputRegisters();
void TestWriteSingleCoil();
void TestWriteSingleHoldingRegister();
void TestWriteMultipleCoils();
void TestWriteMultipleHoldingRegisters();
void TestUserDefinedFunctionCode();

//==============================================================================

extern "C" void app_main(void) {
  ESP_ERROR_CHECK(esp_netif_init());
  
  UNITY_BEGIN();
  TEST_ASSERT_EQUAL(PL::ModbusServer::defaultNetworkProtocol, server.GetProtocol());
  TEST_ASSERT_EQUAL(PL::ModbusServer::defaultNetworkProtocol, client.GetProtocol());
  TEST_ASSERT_EQUAL(PL::ModbusServer::defaultNetworkStationAddress, server.GetStationAddress());
  TEST_ASSERT_EQUAL(PL::ModbusServer::defaultNetworkStationAddress, client.GetStationAddress());
  TEST_ASSERT_EQUAL(PL::ModbusServer::defaultReadTimeout, server.GetReadTimeout());
  TEST_ASSERT_EQUAL(PL::ModbusClient::defaultReadTimeout, client.GetReadTimeout());
  TEST_ASSERT_EQUAL(0, server.GetDelayAfterRead());
  TEST_ASSERT_EQUAL(0, client.GetDelayAfterRead());

  TEST_ASSERT(server.SetStationAddress(stationAddress) == ESP_OK);
  TEST_ASSERT_EQUAL(stationAddress, server.GetStationAddress());
  TEST_ASSERT(client.SetStationAddress(stationAddress) == ESP_OK);
  TEST_ASSERT_EQUAL(stationAddress, client.GetStationAddress());

  TEST_ASSERT(server.SetReadTimeout(PL::ModbusServer::defaultReadTimeout + 1) == ESP_OK);
  TEST_ASSERT_EQUAL(PL::ModbusServer::defaultReadTimeout + 1, server.GetReadTimeout());
  TEST_ASSERT(client.SetReadTimeout(PL::ModbusClient::defaultReadTimeout + 1) == ESP_OK);
  TEST_ASSERT_EQUAL(PL::ModbusClient::defaultReadTimeout + 1, client.GetReadTimeout());

  TEST_ASSERT(server.SetDelayAfterRead(1) == ESP_OK);
  TEST_ASSERT_EQUAL(1, server.GetDelayAfterRead());
  TEST_ASSERT(client.SetDelayAfterRead(1) == ESP_OK);
  TEST_ASSERT_EQUAL(1, client.GetDelayAfterRead());

  TEST_ASSERT(server.SetDelayAfterRead(0) == ESP_OK);
  TEST_ASSERT(client.SetDelayAfterRead(0) == ESP_OK);

  esp_fill_random(serverHR->data, numberOfRegisters * 2);
  server.AddMemoryArea(std::make_shared<PL::ModbusMemoryArea>(PL::ModbusMemoryType::coils, 0, serverHR->data, serverHR->size, serverHR));
  server.AddMemoryArea(std::make_shared<PL::ModbusMemoryArea>(PL::ModbusMemoryType::discreteInputs, 0, serverHR->data, serverHR->size, serverHR));
  server.AddMemoryArea(serverHR);
  server.AddMemoryArea(std::make_shared<PL::ModbusMemoryArea>(PL::ModbusMemoryType::inputRegisters, 0, serverHR->data, serverHR->size, serverHR));
  TEST_ASSERT(server.Enable() == ESP_OK);
  vTaskDelay(10);
  TEST_ASSERT(server.IsEnabled());

  for (int i = 0; i < protocols.size(); i++) {
    printf("%s protocol:\n", protocolNames[i].c_str());
    TEST_ASSERT(server.SetProtocol(protocols[i]) == ESP_OK);
    TEST_ASSERT(client.SetProtocol(protocols[i]) == ESP_OK);

    RUN_TEST(TestErrors);
    RUN_TEST(TestReadCoils);
    RUN_TEST(TestReadDiscreteInputs);
    RUN_TEST(TestReadHoldingRegisters);
    RUN_TEST(TestReadInputRegisters);
    RUN_TEST(TestWriteSingleCoil);
    RUN_TEST(TestWriteSingleHoldingRegister);
    RUN_TEST(TestWriteMultipleCoils);
    RUN_TEST(TestWriteMultipleHoldingRegisters);
    RUN_TEST(TestUserDefinedFunctionCode);
  }

  TEST_ASSERT(server.Disable() == ESP_OK);
  TEST_ASSERT(!server.IsEnabled());

  UNITY_END();
}

//==============================================================================

void TestErrors() {
  TEST_ASSERT(client.SetStationAddress(stationAddress + 1) == ESP_OK);
  TEST_ASSERT(client.WriteSingleCoil(0, false, NULL) == ESP_ERR_TIMEOUT);
  TEST_ASSERT(client.SetStationAddress(stationAddress) == ESP_OK);

  PL::ModbusException exception;
  TEST_ASSERT(client.Command(PL::ModbusFunctionCode::unknown, NULL, 0, NULL, 0, NULL, &exception) == ESP_FAIL);
  TEST_ASSERT_EQUAL(PL::ModbusException::illegalFunction, exception);
  TEST_ASSERT(client.ReadCoils(numberOfBits, 1, NULL, &exception) == ESP_FAIL);
  TEST_ASSERT_EQUAL(PL::ModbusException::illegalDataAddress, exception);
  TEST_ASSERT(client.ReadDiscreteInputs(numberOfBits, 1, NULL, &exception) == ESP_FAIL);
  TEST_ASSERT_EQUAL(PL::ModbusException::illegalDataAddress, exception);
  TEST_ASSERT(client.ReadHoldingRegisters(numberOfRegisters, 1, NULL, &exception) == ESP_FAIL);
  TEST_ASSERT_EQUAL(PL::ModbusException::illegalDataAddress, exception);
  TEST_ASSERT(client.ReadInputRegisters(numberOfRegisters, 1, NULL, &exception) == ESP_FAIL);
  TEST_ASSERT_EQUAL(PL::ModbusException::illegalDataAddress, exception);
  TEST_ASSERT(client.WriteSingleCoil(numberOfBits, false, &exception) == ESP_FAIL);
  TEST_ASSERT_EQUAL(PL::ModbusException::illegalDataAddress, exception);
  TEST_ASSERT(client.WriteSingleHoldingRegister(numberOfRegisters, false, &exception) == ESP_FAIL);
  TEST_ASSERT_EQUAL(PL::ModbusException::illegalDataAddress, exception);
  uint16_t testValue;
  TEST_ASSERT(client.WriteMultipleCoils(numberOfBits, 1, &testValue, &exception) == ESP_FAIL);
  TEST_ASSERT_EQUAL(PL::ModbusException::illegalDataAddress, exception);
  TEST_ASSERT(client.WriteMultipleHoldingRegisters(numberOfRegisters, 1, &testValue, &exception) == ESP_FAIL);
  TEST_ASSERT_EQUAL(PL::ModbusException::illegalDataAddress, exception);
  uint16_t testData[] = {0, 0x1111};
  TEST_ASSERT(client.Command(PL::ModbusFunctionCode::writeSingleCoil, testData, sizeof(testData), NULL, 0, NULL, &exception) == ESP_FAIL);
  TEST_ASSERT_EQUAL(PL::ModbusException::illegalDataValue, exception);
}

//==============================================================================

void TestReadCoils() {
  for (int i = 0; i < numberOfIterations; i++) {
    uint16_t testNumberOfBits = esp_random() % numberOfBits + 1;
    uint16_t testAddress = esp_random() % (numberOfBits - testNumberOfBits + 1);
    uint8_t* dest = new uint8_t[(testNumberOfBits - 1) / 8 + 1]; 
    PL::ModbusException exception;
    TEST_ASSERT(client.ReadCoils(testAddress, testNumberOfBits, dest, &exception) == ESP_OK);
    TEST_ASSERT_EQUAL(PL::ModbusException::noException, exception);
    for (int j = 0; j < testNumberOfBits; j++) {
      TEST_ASSERT_EQUAL((((uint8_t*)serverHR->data)[(testAddress + j) / 8] >> ((testAddress + j) % 8)) & 1, (dest[j / 8] >> (j % 8)) & 1);
    }
    delete[] dest;
  }
}

//==============================================================================

void TestReadDiscreteInputs() {
  for (int i = 0; i < numberOfIterations; i++) {
    uint16_t testNumberOfBits = esp_random() % numberOfBits + 1;
    uint16_t testAddress = esp_random() % (numberOfBits - testNumberOfBits + 1);
    uint8_t* dest = new uint8_t[(testNumberOfBits - 1) / 8 + 1]; 
    PL::ModbusException exception;
    TEST_ASSERT(client.ReadDiscreteInputs(testAddress, testNumberOfBits, dest, &exception) == ESP_OK);
    TEST_ASSERT_EQUAL(PL::ModbusException::noException, exception);
    for (int j = 0; j < testNumberOfBits; j++) {
      TEST_ASSERT_EQUAL((((uint8_t*)serverHR->data)[(testAddress + j) / 8] >> ((testAddress + j) % 8)) & 1, (dest[j / 8] >> (j % 8)) & 1);
    }
    delete[] dest;
  }
}

//==============================================================================

void TestReadHoldingRegisters() {
  for (int i = 0; i < numberOfIterations; i++) {
    uint16_t testNumberOfRegisters = esp_random() % numberOfRegisters + 1;
    uint16_t testAddress = esp_random() % (numberOfRegisters - testNumberOfRegisters + 1);
    uint16_t* dest = new uint16_t[testNumberOfRegisters * 2]; 
    PL::ModbusException exception;
    TEST_ASSERT(client.ReadHoldingRegisters(testAddress, testNumberOfRegisters, dest, &exception) == ESP_OK);
    TEST_ASSERT_EQUAL(PL::ModbusException::noException, exception);
    for (int j = 0; j < testNumberOfRegisters; j++) {
      TEST_ASSERT_EQUAL(((uint16_t*)serverHR->data)[testAddress + j], dest[j]);
    }
    delete[] dest;
  }
}

//==============================================================================

void TestReadInputRegisters() {
  for (int i = 0; i < numberOfIterations; i++) {
    uint16_t testNumberOfRegisters = esp_random() % numberOfRegisters + 1;
    uint16_t testAddress = esp_random() % (numberOfRegisters - testNumberOfRegisters + 1);
    uint16_t* dest = new uint16_t[testNumberOfRegisters * 2]; 
    PL::ModbusException exception;
    TEST_ASSERT(client.ReadInputRegisters(testAddress, testNumberOfRegisters, dest, &exception) == ESP_OK);
    TEST_ASSERT_EQUAL(PL::ModbusException::noException, exception);
    for (int j = 0; j < testNumberOfRegisters; j++) {
      TEST_ASSERT_EQUAL(((uint16_t*)serverHR->data)[testAddress + j], dest[j]);
    }
    delete[] dest;
  }
}

//==============================================================================

void TestWriteSingleCoil() {
  for (int i = 0; i < numberOfIterations; i++) {
    uint16_t testAddress = esp_random() % numberOfBits;
    bool testValue = esp_random() % 2;
    PL::ModbusException exception;
    TEST_ASSERT(client.WriteSingleCoil(testAddress, testValue, &exception) == ESP_OK);
    TEST_ASSERT_EQUAL(PL::ModbusException::noException, exception);
    TEST_ASSERT_EQUAL(testValue, (((uint8_t*)serverHR->data)[testAddress / 8] >> (testAddress % 8)) & 1);
  }
}

//==============================================================================

void TestWriteSingleHoldingRegister() {
  for (int i = 0; i < numberOfIterations; i++) {
    uint16_t testAddress = esp_random() % numberOfRegisters;
    uint16_t testValue = esp_random() % 0x10000;
    PL::ModbusException exception;
    TEST_ASSERT(client.WriteSingleHoldingRegister(testAddress, testValue, &exception) == ESP_OK);
    TEST_ASSERT_EQUAL(PL::ModbusException::noException, exception);
    TEST_ASSERT_EQUAL(testValue, ((uint16_t*)serverHR->data)[testAddress]);
  }
}

//==============================================================================

void TestWriteMultipleCoils() {
  for (int i = 0; i < numberOfIterations; i++) {
    uint16_t testNumberOfBits = esp_random() % numberOfBits + 1;
    uint16_t testAddress = esp_random() % (numberOfBits - testNumberOfBits + 1);
    uint8_t* src = new uint8_t[(testNumberOfBits - 1) / 8 + 1]; 
    esp_fill_random(src, (testNumberOfBits - 1) / 8 + 1);
    PL::ModbusException exception;
    TEST_ASSERT(client.WriteMultipleCoils(testAddress, testNumberOfBits, src, &exception) == ESP_OK);
    TEST_ASSERT_EQUAL(PL::ModbusException::noException, exception);
    for (int j = 0; j < testNumberOfBits; j++) {
      TEST_ASSERT_EQUAL((src[j / 8] >> (j % 8)) & 1, (((uint8_t*)serverHR->data)[(testAddress + j) / 8] >> ((testAddress + j) % 8)) & 1);
    }
    delete[] src;
  }
}

//==============================================================================

void TestWriteMultipleHoldingRegisters() {
  for (int i = 0; i < numberOfIterations; i++) {
    uint16_t testNumberOfRegisters = esp_random() % numberOfRegisters + 1;
    uint16_t testAddress = esp_random() % (numberOfRegisters - testNumberOfRegisters + 1);
    uint16_t* src = new uint16_t[testNumberOfRegisters * 2]; 
    esp_fill_random(src, testNumberOfRegisters * 2);
    PL::ModbusException exception;
    TEST_ASSERT(client.WriteMultipleHoldingRegisters(testAddress, testNumberOfRegisters, src, &exception) == ESP_OK);
    TEST_ASSERT_EQUAL(PL::ModbusException::noException, exception);
    for (int j = 0; j < testNumberOfRegisters; j++) {
      TEST_ASSERT_EQUAL(src[j], ((uint16_t*)serverHR->data)[testAddress + j]);
    }
    delete[] src;
  }
}

//==============================================================================

void TestUserDefinedFunctionCode() {
  userDefinedFunctionRequest = esp_random();
  size_t responseDataSize;
  TEST_ASSERT(client.Command(userDefinedFunctionCode, &userDefinedFunctionRequest, sizeof(userDefinedFunctionRequest), userDefinedFunctionResponse, sizeof(userDefinedFunctionResponse), &responseDataSize, NULL) == ESP_OK);
  TEST_ASSERT_EQUAL(sizeof(userDefinedFunctionResponse), responseDataSize);
  for (int i = 0; i < sizeof(userDefinedFunctionResponse) / sizeof(uint32_t); i++)
    TEST_ASSERT_EQUAL(userDefinedFunctionRequest, userDefinedFunctionResponse[i]);
}

//==============================================================================

esp_err_t Server::ReadRtuData(PL::Stream& stream, PL::ModbusFunctionCode functionCode, size_t& dataSize) {
  PL::Buffer& dataBuffer = GetDataBuffer();

  if (functionCode == PL::ModbusFunctionCode::unknown) {
    dataSize = 0;
    return ESP_OK;
  }

  if (functionCode == userDefinedFunctionCode) {
    dataSize = sizeof(userDefinedFunctionRequest);
    if (dataBuffer.size >= dataSize)
      return stream.Read(dataBuffer, 0, dataSize);
    else {
      stream.Read(NULL, dataSize);
      return ESP_ERR_INVALID_SIZE;
    }
  }
    
  return PL::ModbusServer::ReadRtuData(stream, functionCode, dataSize);
}

//==============================================================================

esp_err_t Server::HandleRequest(PL::Stream& stream, uint8_t stationAddress, PL::ModbusFunctionCode functionCode, size_t dataSize, uint16_t transactionId) {
  PL::Buffer& dataBuffer = GetDataBuffer();

  if (functionCode == userDefinedFunctionCode) {
    if (dataSize != sizeof(userDefinedFunctionRequest) || dataBuffer.size < sizeof(userDefinedFunctionResponse))
      return WriteExceptionFrame(stream, stationAddress, functionCode, PL::ModbusException::illegalDataValue, transactionId);
    uint32_t request = ((uint32_t*)dataBuffer.data)[0];
    for (int i = 0; i < sizeof(userDefinedFunctionResponse) / sizeof(uint32_t); i++)
      ((uint32_t*)dataBuffer.data)[i] = request;
    return WriteFrame(stream, stationAddress, functionCode, sizeof(userDefinedFunctionResponse), transactionId);
  }
  return PL::ModbusServer::HandleRequest(stream, stationAddress, functionCode, dataSize, transactionId);
}

//==============================================================================

esp_err_t Client::ReadRtuData(PL::Stream& stream, PL::ModbusFunctionCode functionCode, size_t& dataSize) {
  PL::Buffer& dataBuffer = GetDataBuffer();

  if (functionCode == userDefinedFunctionCode) {
    dataSize = sizeof(userDefinedFunctionResponse);
    if (dataBuffer.size >= dataSize)
      return stream.Read(dataBuffer, 0, dataSize);
    else {
      stream.Read(NULL, dataSize);
      return ESP_ERR_INVALID_SIZE;
    }        
  }

  return PL::ModbusClient::ReadRtuData(stream, functionCode, dataSize);
}