cmake_minimum_required(VERSION 3.21)
project(daemon)
set(CMAKE_CXX_STANDARD 17)

add_executable(
  test
  ./main.cpp
  ./daemon.cpp
  )
