project(FPlayer_Backend_Media_FFmpeg)

find_package(Qt6 COMPONENTS Core OpenGL OpenGLWidgets REQUIRED)

add_standard_module(${PROJECT_NAME} include/fplayer/backend/media_ffmpeg ON ON)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::OpenGL Qt6::OpenGLWidgets)
target_link_libraries(${PROJECT_NAME} PUBLIC FPlayer_Api)
target_link_libraries(${PROJECT_NAME} PRIVATE Logger)
target_link_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE avdevice avfilter avformat avcodec swscale swresample avutil)
# windows下依赖DirectShow库
if (WIN32)
    target_link_libraries(FPlayer_Backend_Media_FFmpeg
            PRIVATE
            strmiids
            ole32
            oleaut32
    )
endif ()
target_include_directories(${PROJECT_NAME} PUBLIC ${FFMPEG_HEADER})

# 设置安装项目
install(TARGETS ${PROJECT_NAME}
        #        EXPORT CxiCommonPcTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})