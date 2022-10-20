# Modbus Server Memory Areas Example

1. UART Modbus server is created with UART0 as a port, Modbus RTU protocol and station address 1.
2. UART0 is initialized.
3. UART0 parameters are set to 115200 bps, 8 data bits, even parity, 1 stop bit and no flow control.
4. UART0 is enabled.
5. Serveral memory areas are added to the server (and in-program locked data access is shown):
   - 5 holding registers (addresses 0..4).
   - 5 holding registers (addresses 5..9) with typed access (addresses 0..4).
   - 5 holding and 5 input registers (addresses 10..14) mapped to the same memory area with typed access.
   - 5 holding registers (addresses 15..19) with first register LSB accessible as coils (addresses 15..22) with typed access.
   - Dynamic input register (address 20) that contains the uptime value in seconds.
   - Dynamic input register (address 21) that duplicates register 20.
6. UART Modbus server is enabled.