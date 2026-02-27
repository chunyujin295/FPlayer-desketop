project(FPlayer_Common)

find_package(Qt6 COMPONENTS Core REQUIRED)

add_standard_module(${PROJECT_NAME} include/common ON ON)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core)
target_link_libraries(${PROJECT_NAME} PRIVATE Logger)

# 设置安装项目
install(TARGETS ${PROJECT_NAME}
        #        EXPORT CxiCommonPcTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})