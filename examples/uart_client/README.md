# UART Modbus Client Example

1. UART Modbus client is created with UART0 as a port, Modbus RTU protocol and station address 1.
2. UART0 is initialized.
3. UART0 parameters are set to 115200 bps, 8 data bits, even parity, 1 stop bit and no flow control.
4. UART0 is enabled.
5. 10 holding registers starting at address 0 are read, incremented and written back every second. 
6. 3 coils starting at address 0 are read, inverted and written back. 
