#include "pl_modbus.h"

//==============================================================================

class WiFiGotIpEventHandler {
public:
  void OnGotIpV4Address (PL::NetworkInterface& wifi);
  void OnGotIpV6Address (PL::NetworkInterface& wifi);
};

//==============================================================================

PL::EspWiFiStation wifi;
const std::string wifiSsid = CONFIG_EXAMPLE_WIFI_SSID;
const std::string wifiPassword = CONFIG_EXAMPLE_WIFI_PASSWORD;
auto wiFiGotIpEventHandler = std::make_shared<WiFiGotIpEventHandler>();
// Modbus server (port: 502, protocol: TCP, address: 255)
const uint16_t port = 502;
PL::ModbusServer server (port);

//==============================================================================

extern "C" void app_main(void) {
  esp_event_loop_create_default();
  esp_netif_init();

  wifi.Initialize();
  wifi.SetSsid (wifiSsid);
  wifi.SetPassword (wifiPassword);
  wifi.gotIpV4AddressEvent.AddHandler (wiFiGotIpEventHandler, &WiFiGotIpEventHandler::OnGotIpV4Address);
  wifi.gotIpV6AddressEvent.AddHandler (wiFiGotIpEventHandler, &WiFiGotIpEventHandler::OnGotIpV6Address);

  if (auto baseServer = server.GetBaseServer().lock()) {
    PL::TcpServer& tcpServer = (PL::TcpServer&)*baseServer;
    tcpServer.SetMaxNumberOfClients (2);
    tcpServer.DisableNagleAlgorithm();
    tcpServer.EnableKeepAlive();
    tcpServer.SetKeepAliveIdleTime (60);
    tcpServer.SetKeepAliveInterval (1);
    tcpServer.SetKeepAliveCount (5);
  }
  // Coils, digital inputs, holding registers and input registers all mapped to the same area.
  // 250 holding/input registers and 4000 coils/digital inputs.
  auto buffer = std::make_shared<PL::Buffer>(500);
  server.AddMemoryArea (PL::ModbusMemoryType::coils, 0, buffer);
  server.AddMemoryArea (PL::ModbusMemoryType::discreteInputs, 0, buffer);
  server.AddMemoryArea (PL::ModbusMemoryType::holdingRegisters, 0, buffer);
  server.AddMemoryArea (PL::ModbusMemoryType::inputRegisters, 0, buffer);

  wifi.Enable();

  while (1) {
    vTaskDelay (1);
  }
}

//==============================================================================

void WiFiGotIpEventHandler::OnGotIpV4Address (PL::NetworkInterface& wifi) {
  if (server.Enable() == ESP_OK) 
    printf ("Listening (address: %s, port: %d)\n", wifi.GetIpV4Address().ToString().c_str(), port);
}

//==============================================================================

void WiFiGotIpEventHandler::OnGotIpV6Address (PL::NetworkInterface& wifi) {
  if (server.Enable() == ESP_OK)
    printf ("Listening (address: %s, port: %d)\n", wifi.GetIpV6LinkLocalAddress().ToString().c_str(), port);
}