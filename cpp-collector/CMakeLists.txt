cmake_minimum_required(VERSION 3.16)

project(flowlog_qt_collector)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core Network)

add_executable(flow_collector
    src/main.cpp

    src/model/NormalizedFlow.cpp

    src/sinks/HttpSink.cpp

    src/collectors/FakeCollector.cpp
    src/collectors/FileCollector.cpp

    src/parsers/ZeekParser.cpp
    src/parsers/SuricataParser.cpp

    src/collectors/NetflowUdpCollector.cpp
    src/parsers/NetflowV5Parser.cpp
)

target_include_directories(flow_collector PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(flow_collector PRIVATE
    Qt5::Core
    Qt5::Network
)
