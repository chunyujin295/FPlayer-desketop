project(FPlayer_Core)

find_package(Qt6 COMPONENTS Core Gui Designer Network Multimedia REQUIRED)

add_standard_module(${PROJECT_NAME} include/core ON ON)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Network Qt6::Multimedia)
target_link_libraries(${PROJECT_NAME} PRIVATE Logger)

qt_deploy_runtime(FPlayer_Core)