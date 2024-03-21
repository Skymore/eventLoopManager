cmake_minimum_required(VERSION 3.25)
project(eventLoopManager)

set(CMAKE_CXX_STANDARD 20)

# 寻找 RapidJSON 库
find_package(RapidJSON REQUIRED)

# 将 RapidJSON 库的 include 目录添加到编译器的包含路径中
include_directories(${RAPIDJSON_INCLUDE_DIRS})
#include_directories(/opt/homebrew/Cellar/rapidjson/1.1.0/include)
include_directories(/opt/homebrew/Cellar/librdkafka/2.3.0/include)
link_directories(/opt/homebrew/Cellar/librdkafka/2.3.0/lib)
link_libraries(rdkafka rdkafka++)

add_executable(eventLoopManager main.cpp kafkaProducer.cpp)
