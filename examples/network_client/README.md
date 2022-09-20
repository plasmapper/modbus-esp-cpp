# Network Modbus client example

1. Internal ESP Wi-Fi station interface is created and initialized.
2. Network Modbus client is created with IP address and port from project configuration (can be changed in `Example` menu of [Project Configuration](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/kconfig.html)).
3. Wi-Fi station SSID and password are set from project configuration (can be changed in `Example` menu of [Project Configuration](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/kconfig.html)).
4. Wi-Fi station is enabled.
5. After the interface gets an IP address 10 holding registers starting at address 0 and 3 coils starting at address 0 are read and received values are printed every second.
