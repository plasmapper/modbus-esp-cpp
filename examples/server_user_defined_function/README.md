# Modbus Server User-Defined Function Example

1. CustomModbusServer class inherits ModbusServer class and overrides ReadRtuData and HandleRequest methods to implement user-defined function with function code 100 that receives 1 byte and responds with the same byte + data array of random byte values (the array size is determined by the received value).
2. CustomModbusServer is created with UART0 as a port, Modbus RTU protocol and station address 1.
3. UART0 is initialized.
4. UART0 parameters are set to 115200 bps, 8 data bits, even parity, 1 stop bit and no flow control.
5. UART0 is enabled.
6. CustomModbusServer is enabled.
