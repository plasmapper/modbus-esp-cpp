#include "pl_modbus.h"

//==============================================================================

PL::EspWiFiStation wifi;
const std::string wifiSsid = CONFIG_EXAMPLE_WIFI_SSID;
const std::string wifiPassword = CONFIG_EXAMPLE_WIFI_PASSWORD;
PL::ModbusClient client (PL::IpV4Address (CONFIG_EXAMPLE_SERVER_ADDRESS), CONFIG_EXAMPLE_SERVER_PORT);

#pragma pack(push, 1)
struct Coils {
  uint8_t coil0 : 1;
  uint8_t coil1 : 1;
  uint8_t coil2 : 1;
};
#pragma pack(pop)

//==============================================================================

extern "C" void app_main(void) {
  esp_event_loop_create_default();
  esp_netif_init();

  wifi.Initialize();
  wifi.SetSsid (wifiSsid);
  wifi.SetPassword (wifiPassword);
  wifi.Enable();

  while (!wifi.GetIpV4Address().u32)  
    vTaskDelay (1);

  while (1) {
    PL::ModbusException exception;

    uint16_t holdingRegisters[10];
    printf ("Reading holding registers\n");
    esp_err_t error = client.ReadHoldingRegisters (0, sizeof (holdingRegisters) / sizeof (uint16_t), holdingRegisters, &exception);
    printf ("Error: %d, exception: %d\n", error, (int)exception);
    if (error == ESP_OK) {
      printf ("Values: ");
      for (int i = 0; i < sizeof (holdingRegisters) / sizeof (uint16_t); i++)
        printf ("%d ", holdingRegisters[i]);
      printf ("\n");
    }

    Coils coils;
    printf ("Reading coils\n");
    error = client.ReadCoils (0, 3, &coils, &exception);
    printf ("Error: %d, exception: %d\n", error, (int)exception);
    if (error == ESP_OK)
      printf ("Values: %d %d %d\n", coils.coil0, coils.coil1, coils.coil2);
    
    vTaskDelay (1000 / portTICK_PERIOD_MS);
  }
}