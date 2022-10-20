# Network Modbus server example

1. Network Modbus server is created at port 502.
2. Internal ESP Wi-Fi station interface is created and initialized.
3. The Wi-Fi station SSID and password are set from project configuration (can be changed in `Example` menu of [Project Configuration](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/kconfig.html)).
4. The server parameters are set to maximum number of clients of 2, disabled Nagle algorithm and enabled keep-alive packets with 60 s idle time, 1 s interval and packet count of 5.
5. A single buffer is created with a size of 500 bytes and is mapped to 4 server memory areas:
    - coils (addresses 0..3999)
    - discrete inputs (addresses 0..3999)
    - holding registers (addresses 0..249)
    - input registers (addresses 0..249)
6. The Wi-Fi station is enabled.
7. The server is enabled when the Wi-Fi staion interface gets an IP address.
