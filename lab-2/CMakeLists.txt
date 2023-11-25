cmake_minimum_required(VERSION 3.14)
project(lab2)

set(CMAKE_CXX_STANDARD 20)

set (
  SRC
  ./src/host.cpp
  ./src/conn_pipe.cpp
)

add_executable(host ${SRC})

target_include_directories(host PRIVATE ./inc)
target_link_libraries(host PRIVATE pthread rt)


