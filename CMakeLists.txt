cmake_minimum_required(VERSION 3.15)
project(FPlayer_desketop)

set(CMAKE_CXX_STANDARD 17)

include(cmake/3rd.cmake)
include(cmake/utils.cmake)

set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(core)
add_subdirectory(widget)
add_subdirectory(sample)

install_qt_libs(${CMAKE_RUNTIME_OUTPUT_DIRECTORY} QT_LIBS
        Core
        Gui
        Widgets
        Qml
        Xml
        Svg
        Multimedia
        MultimediaWidgets
)



