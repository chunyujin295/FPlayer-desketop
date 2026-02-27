cmake_minimum_required(VERSION 3.15)
project(FPlayer_Desketop)

set(CMAKE_CXX_STANDARD 17)

include(cmake/3rd.cmake)
include(cmake/utils.cmake)

set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(common)
add_subdirectory(api)
add_subdirectory(widget)
add_subdirectory(app)




