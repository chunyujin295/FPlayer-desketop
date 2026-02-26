project(FPlayer_Sample)

include(../cmake/utils.cmake)
config_project(ON)
add_executable(${PROJECT_NAME} "" )
find_package(Qt6 COMPONENTS Widgets Core Gui Designer REQUIRED)

file(GLOB_RECURSE "src" CONFIGURE_DEPENDS "*.cpp" "*.h")
file(GLOB_RECURSE "uis" CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.ui")
file(GLOB_RECURSE "qrcs" CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/res/*.qrc")

target_sources(${PROJECT_NAME} PRIVATE ${src} ${uis} ${qrcs})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui)
target_link_libraries(${PROJECT_NAME} PRIVATE FPlayer_Core)

#copy_imported_dependencies(${PROJECT_NAME} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
