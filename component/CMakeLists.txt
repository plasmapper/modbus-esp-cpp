cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "pl_modbus_base.cpp" "pl_modbus_memory_area.cpp" "pl_modbus_client.cpp" "pl_modbus_server.cpp" INCLUDE_DIRS "include"
                       REQUIRES "pl_common" "pl_uart" "pl_network")