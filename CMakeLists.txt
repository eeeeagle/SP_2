cmake_minimum_required(VERSION 3.24)
project(SP_2)

set(CMAKE_CXX_STANDARD 14)

add_executable(SP_2 src/main.cpp src/check.hpp src/common.hpp src/message_queue.hpp src/signal.hpp src/unnamed_pipe.hpp)
