Modbus Component
================

.. |COMPONENT| replace:: modbus

.. |ESP_IDF_VERSION| replace:: 5.0
  
.. |VERSION| replace:: 1.0.1

.. include:: ../../../installation.rst

.. include:: ../../../sdkconfig_network.rst

Features
--------

1. :cpp:class:`PL::ModbusClient` - a Modbus client class.

   * RTU, ASCII and TCP protocols via UART or network connection.
   * Implemented read/write functions (Modbus function codes):
   
     * :cpp:func:`PL::ModbusClient::ReadCoils` / :cpp:func:`PL::ModbusClient::ReadDiscreteInputs` /
       :cpp:func:`PL::ModbusClient::ReadHoldingRegisters` / :cpp:func:`PL::ModbusClient::ReadInputRegisters` (1/2/3/4)
     * :cpp:func:`PL::ModbusClient::WriteSingleCoil` / :cpp:func:`PL::ModbusClient::WriteSingleHoldingRegister` (5/6)
     * :cpp:func:`PL::ModbusClient::WriteMultipleCoils` / :cpp:func:`PL::ModbusClient::WriteMultipleHoldingRegisters` (15/16)
     
   * Splitting single read/write requests into multiple requests with valid number of memory elements. 
   * Automatic reconnection to the device.
   * Support of multiple devices on the same UART or TCP client.
   * To implement other Modbus function codes:
   
     * Inherit :cpp:class:`PL::ModbusClient` and override :cpp:func:`PL::ModbusClient::ReadRtuData` method to read custom function response data.
     * Use public or protected :cpp:func:`PL::ModbusClient::Command` method (see the implemented read/write methods).
     
2. :cpp:class:`PL::ModbusServer` - a Modbus server class.
   
   * RTU, ASCII and TCP protocols via UART or network connection.
   * Several :cpp:func:`PL::ModbusServer::AddMemoryArea` methods, :cpp:class:`PL::ModbusMemoryArea` and :cpp:class:`PL::ModbusTypedMemoryArea`
     classes to create simple and complex combinations of Modbus server memory areas.  
   * Same implemented read/write functions as for the client.
   * To implement other Modbus function codes:
   
     * Inherit :cpp:class:`PL::ModbusServer` class and override :cpp:func:`PL::ModbusServer::ReadRtuData` method to read custom function request data. 
     * Override :cpp:func:`PL::ModbusServer::HandleRequest` method to handle the client request with a custom function code.

Examples
--------
| `UART client <https://components.espressif.com/components/plasmapper/pl_modbus/versions/1.0.1/examples/uart_client>`_
| `Network client <https://components.espressif.com/components/plasmapper/pl_modbus/versions/1.0.1/examples/network_client>`_
| `UART server <https://components.espressif.com/components/plasmapper/pl_modbus/versions/1.0.1/examples/uart_server>`_
| `Network server <https://components.espressif.com/components/plasmapper/pl_modbus/versions/1.0.1/examples/network_server>`_
| `Server memory areas <https://components.espressif.com/components/plasmapper/pl_modbus/versions/1.0.1/examples/server_memory_areas>`_
| `Server user-defined function <https://components.espressif.com/components/plasmapper/pl_modbus/versions/1.0.1/examples/server_user_defined_function>`_

API reference
-------------

.. toctree::
  
  api/types      
  api/modbus_client
  api/modbus_server
  api/modbus_memory_area
  api/modbus_typed_memory_area