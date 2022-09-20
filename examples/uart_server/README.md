# UART Modbus server example

1. UART Modbus server is created with UART0 as a port, Modbus RTU protocol and station address 1.
2. UART0 is initialized.
3. UART0 parameters are set to 115200 bps, 8 data bits, even parity, 1 stop bit and no flow control.
4. UART0 is enabled.
5. A single buffer is created with a size of 500 bytes and is mapped to 4 server memory areas:
    - coils (addresses 0..3999)
    - discrete inputs (addresses 0..3999)
    - holding registers (addresses 0..249)
    - input registers (addresses 0..249)
6. UART Modbus server is enabled.
