cmake_minimum_required(VERSION 3.8)
project(rtea)

set(CMAKE_C_STANDARD 99)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(SOURCE_FILES main.c util.c util.h definitions.h ID3.c ID3.h LinkedList.c LinkedList.h tables.h)
add_executable(rtea ${SOURCE_FILES})
target_link_libraries(rtea m)