cmake_minimum_required(VERSION 3.10)
project(GLTFWriter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/ThirdParty)
include_directories(${CMAKE_SOURCE_DIR}/SimpleViewer/include)

add_executable(GLTFWriter main.cpp)