cmake_minimum_required(VERSION 4.1)
project(FPlay_desketop)

set(CMAKE_CXX_STANDARD 17)

include(cmake/3rd.cmake)

set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(FPlay_desketop main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE Logger)



