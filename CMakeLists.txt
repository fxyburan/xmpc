cmake_minimum_required(VERSION 3.15)
project(crypto)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -pthread)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -maes")

include_directories(include)

find_package(OpenSSL REQUIRED)

add_library(x-scheduler
        src/scheduler/graph.cpp
        src/scheduler/parser.cpp
        src/scheduler/scheduler.cpp)
target_link_libraries(x-scheduler pthread)

add_executable(x-scheduler-test
        src/u_test/scheduler_test.cpp)
target_link_libraries(x-scheduler-test x-scheduler)

add_executable(crypto
        main.cpp
        src/common/type.cpp
        src/ops/ops.cpp)
target_link_libraries(crypto OpenSSL::Crypto x-scheduler pthread)
