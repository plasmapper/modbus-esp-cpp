# USB Modbus Server Example

1. USB Modbus server is created with USB as a port, Modbus RTU protocol and station address 1.
2. USB device and USB device CDC endpoint are initialized.
3. USB device CDC endpoint is enabled.
4. A single buffer is created with a size of 500 bytes and is mapped to 4 server memory areas:
    - coils (addresses 0..3999)
    - discrete inputs (addresses 0..3999)
    - holding registers (addresses 0..249)
    - input registers (addresses 0..249)
5. USB Modbus server is enabled.
