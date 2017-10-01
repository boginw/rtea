cmake_minimum_required(VERSION 3.8)
project(rtea)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c util.c util.h definitions.c definitions.h ID3.c ID3.h)
add_executable(rtea ${SOURCE_FILES})