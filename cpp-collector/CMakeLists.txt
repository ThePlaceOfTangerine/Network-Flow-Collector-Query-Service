cmake_minimum_required(VERSION 3.16)

project(flowlog_cpp_collector)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)

add_executable(flow_collector
    src/main.cpp
)

target_include_directories(flow_collector PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(flow_collector PRIVATE
    nlohmann_json::nlohmann_json
    CURL::libcurl
)
