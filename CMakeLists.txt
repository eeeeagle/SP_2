cmake_minimum_required(VERSION 3.23)
project(SP_2)

set(CMAKE_CXX_STANDARD 14)

add_executable(SP_2 src/check.hpp src/signal.hpp src/unnamed_pipe.hpp src/message_queue.hpp src/main.cpp)
