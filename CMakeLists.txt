cmake_minimum_required(VERSION 3.21)
project(CS_6)

set(CMAKE_CXX_STANDARD 14)

include_directories(ArpTable)
link_libraries(ws2_32 wsock32)
link_libraries(iphlpapi iphlpapi)
add_executable(CS_6
        ArpTable/ArpTable.cpp)
