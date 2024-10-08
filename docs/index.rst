Modbus Component
================

.. |COMPONENT| replace:: modbus

.. |ESP_IDF_VERSION| replace:: 5.3
  
.. |VERSION| replace:: 1.2.1

.. include:: ../../../installation.rst

.. include:: ../../../sdkconfig_network.rst

Features
--------

1. :cpp:class:`PL::ModbusClient` - a Modbus client class.

   * RTU, ASCII and TCP protocols via a single stream (UART, USB etc) or a network connection.
   * Implemented read/write functions (Modbus function codes):
   
     * :cpp:func:`PL::ModbusClient::ReadCoils` / :cpp:func:`PL::ModbusClient::ReadDiscreteInputs` /
       :cpp:func:`PL::ModbusClient::ReadHoldingRegisters` / :cpp:func:`PL::ModbusClient::ReadInputRegisters` (1/2/3/4)
     * :cpp:func:`PL::ModbusClient::WriteSingleCoil` / :cpp:func:`PL::ModbusClient::WriteSingleHoldingRegister` (5/6)
     * :cpp:func:`PL::ModbusClient::WriteMultipleCoils` / :cpp:func:`PL::ModbusClient::WriteMultipleHoldingRegisters` (15/16)
     
   * Splitting single read/write requests into multiple requests with valid number of memory elements. 
   * Automatic reconnection to the device.
   * Support of multiple devices on the same stream or TCP client.
   * To implement other Modbus function codes:
   
     * Inherit :cpp:class:`PL::ModbusClient` and override :cpp:func:`PL::ModbusClient::ReadRtuData` method to read custom function response data.
     * Use public or protected :cpp:func:`PL::ModbusClient::Command` method (see the implemented read/write methods).
     
2. :cpp:class:`PL::ModbusServer` - a Modbus server class.
   
   * RTU, ASCII and TCP protocols via a single stream (UART, USB etc) or a network connection.
   * Several :cpp:func:`PL::ModbusServer::AddMemoryArea` methods, :cpp:class:`PL::ModbusMemoryArea` and :cpp:class:`PL::ModbusTypedMemoryArea`
     classes to create simple and complex combinations of Modbus server memory areas.  
   * Same implemented read/write functions as for the client.
   * To implement other Modbus function codes:
   
     * Inherit :cpp:class:`PL::ModbusServer` class and override :cpp:func:`PL::ModbusServer::ReadRtuData` method to read custom function request data. 
     * Override :cpp:func:`PL::ModbusServer::HandleRequest` method to handle the client request with a custom function code.

Thread safety
-------------

Class method thread safety is implemented by having the :cpp:class:`PL::Lockable` as a base class and creating the class object lock guard at the beginning of the methods.

The stream :cpp:class:`PL::ModbusClient` locks both the :cpp:class:`PL::ModbusClient` and the :cpp:class:`PL::Stream` objects for the duration of the transaction.
The network :cpp:class:`PL::ModbusClient` locks both the :cpp:class:`PL::ModbusClient` and the :cpp:class:`PL::TcpClient` objects for the duration of the transaction.

The stream :cpp:class:`PL::ModbusServer` task method locks both the underlying :cpp:class:`PL::StreamServer` and the :cpp:class:`PL::Stream` objects for the duration of the transaction.
The network :cpp:class:`PL::ModbusServer` task method locks both the underlying :cpp:class:`PL::TcpServer` and the client :cpp:class:`PL::NetworkStream` objects for the duration of the transaction.
The default :cpp:func:`PL::ModbusServer::HandleRequest` locks the accessed :cpp:class:`PL::ModbusMemoryArea` for the duration of the transaction.

Examples
--------
| `UART client <https://components.espressif.com/components/plasmapper/pl_modbus/versions/1.2.1/examples/uart_client>`_
| `Network client <https://components.espressif.com/components/plasmapper/pl_modbus/versions/1.2.1/examples/network_client>`_
| `UART server <https://components.espressif.com/components/plasmapper/pl_modbus/versions/1.2.1/examples/uart_server>`_
| `USB server <https://components.espressif.com/components/plasmapper/pl_modbus/versions/1.2.1/examples/usb_server>`_
| `Network server <https://components.espressif.com/components/plasmapper/pl_modbus/versions/1.2.1/examples/network_server>`_
| `Server memory areas <https://components.espressif.com/components/plasmapper/pl_modbus/versions/1.2.1/examples/server_memory_areas>`_
| `Server user-defined function <https://components.espressif.com/components/plasmapper/pl_modbus/versions/1.2.1/examples/server_user_defined_function>`_

API reference
-------------

.. toctree::
  
  api/types      
  api/modbus_client
  api/modbus_server
  api/modbus_memory_area
  api/modbus_typed_memory_area