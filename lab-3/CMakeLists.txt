cmake_minimum_required(VERSION 3.14)
project(lab-3)

set(CMAKE_CXX_STANDARD 17)

set (
  SRC
  main.cpp
)

add_executable(lab3 ${SRC})

target_include_directories(lab3 PRIVATE ./src ./src/connections)
target_link_libraries(lab3 PRIVATE pthread rt)


