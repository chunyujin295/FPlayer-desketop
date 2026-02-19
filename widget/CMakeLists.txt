project(FPlayer_Widget)

find_package(Qt6 COMPONENTS Widgets Core Gui Designer Network Multimedia REQUIRED)

add_standard_module(${PROJECT_NAME} include/widget ON ON)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui Qt6::Network Qt6::Multimedia)
target_link_libraries(${PROJECT_NAME} PRIVATE Logger)