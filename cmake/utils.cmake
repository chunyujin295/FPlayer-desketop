# \brief 宏名称 : config_project【CXI公共配置逻辑】
# @is_qt_module : 是否为Qt依赖模块 True Or False
macro(config_project is_qt_module)
    if (MSVC)
        add_compile_options("/utf-8")
        add_compile_options("-DUNICODE -D_UNICODE")
    endif ()
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    if (${is_qt_module})
        set(CMAKE_AUTORCC ON)
        set(CMAKE_AUTOMOC ON)
        list(APPEND CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/uis")
        set(CMAKE_AUTOUIC ON)
        set(CMAKE_INCLUDE_CURRENT_DIR ON)
    endif ()
endmacro()

# \brief 宏名称    : add_standard_module【CXI添加标准模块宏】
# @target         : 库目标
# @export_location: 导出位置
# @is_shared      : 是否为动态库
# @is_qt_module   : 是否依赖Qt
macro(add_standard_module target export_location is_shared is_qt_module)
    if (${target} STREQUAL "")
        message(FATAL_ERROR "${target}目标不存在")
    endif ()
    if (NOT ${is_shared})
        message(FATAL_ERROR "动静态库配置参数不存在")
    endif ()

    config_project(${is_qt_module})
    if (${is_shared})
        add_library(${target} SHARED)
    else ()
        add_library(${target})
    endif ()
    if (NOT ${export_location} STREQUAL "")
        export_symbol(${target} ${export_location})
    endif ()
    file(GLOB_RECURSE srcs CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
    file(GLOB_RECURSE srcs_p CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/private/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/private/*.h")
    file(GLOB_RECURSE uis CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/uis/*.ui")
    file(GLOB_RECURSE qrcs CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/res/*.qrc")
    target_include_directories(${target} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/private")
    target_sources(${target} PRIVATE ${srcs} ${uis} ${srcs_p} ${qrcs})
endmacro()

# ========================= 方法 =========================
# \brief 函数名称 : export_symbol【导出逻辑】
# @target      : 库目标
# @location    : 生成位置
function(export_symbol target location)
    if (NOT TARGET ${target})
        message(FATAL_ERROR "目标 ${target} 不存在，无法导出符号")
    endif ()
    if (NOT location)
        message(FATAL_ERROR "缺少导出文件路径参数")
    endif ()
    string(TOUPPER ${target} TARGET_NAME)

    set(dst "${CMAKE_CURRENT_SOURCE_DIR}/${location}/export.h")
    if (NOT EXISTS ${dst})
        configure_file("../cmake/global_template.h.in"
                "${CMAKE_CURRENT_SOURCE_DIR}/${location}/export.h"
                @ONLY)
    endif ()
    target_compile_definitions(${target} PRIVATE "${TARGET_NAME}_LIBRARY")
endfunction()

# \brief 函数名称 : install_qt_libs【安装Qt库】
# @QT_LIBS      : Qt库列表
# @dest_dir     : 安装目录
function(install_qt_libs dest_dir)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs QT_LIBS)
    cmake_parse_arguments(INSTALL_QT_LIBS
            "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        set(DEBUG_SUFFIX)
        if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
            set(DEBUG_SUFFIX "d")
        endif ()

        # 推导 Qt 安装目录
        set(QT_INSTALL_PATH "${Qt6_DIR}/../../../")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
            if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
                set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
            endif ()
        endif ()

        # 平台插件 (Windows 必需)
        if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
            execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
                    "${dest_dir}/plugins/platforms/")
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                    "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                    "${dest_dir}/plugins/platforms/")
        endif ()

        # Qt 库和插件目录映射（相对完整）
        set(QT_PLUGIN_MAP
                Gui "platforms\;imageformats\;iconengines\;platforminputcontexts\;styles"
                Widgets "platforms\;imageformats\;iconengines\;platforminputcontexts\;styles"
                Network "bearer"
                Sql "sqldrivers"
                Svg "imageformats"
                Multimedia "mediaservice\;audio"
                MultimediaWidgets "mediaservice\;audio"
                Qml "qmltooling"
                Quick "scenegraph\;qmltooling"
                Positioning "position"
                Sensors "sensors"
                Bluetooth "bluetooth"
                PrintSupport "printsupport"
                WebEngineCore "resources\;translations"
                WebView "resources\;translations"
                TextToSpeech "texttospeech"
                VirtualKeyboard "virtualkeyboard"
        )

        # 拷贝库和插件
        foreach (QT_LIB ${INSTALL_QT_LIBS_QT_LIBS})
            # DLL
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                    "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                    "${dest_dir}")

            # 查找插件映射
            list(FIND QT_PLUGIN_MAP ${QT_LIB} plugin_index)
            if (NOT plugin_index EQUAL -1)
                math(EXPR plugin_value_index "${plugin_index} + 1")
                list(GET QT_PLUGIN_MAP ${plugin_value_index} plugin_dirs_str)
                string(REPLACE ";" "|" plugin_dirs_pattern "${plugin_dirs_str}") # 防止 split 被误解
                string(REPLACE ";" ";" plugin_dirs "${plugin_dirs_str}")

                foreach (plugin_dir ${plugin_dirs})
                    set(src_plugin_dir "${QT_INSTALL_PATH}/plugins/${plugin_dir}")
                    set(dst_plugin_dir "${dest_dir}/plugins/${plugin_dir}")

                    if (EXISTS "${src_plugin_dir}")
                        execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${dst_plugin_dir}")
                        file(GLOB plugin_files "${src_plugin_dir}/*${DEBUG_SUFFIX}.dll")
                        foreach (plugin_file ${plugin_files})
                            execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                                    "${plugin_file}" "${dst_plugin_dir}")
                        endforeach ()
                    endif ()
                endforeach ()
            endif ()
        endforeach ()
    endif ()
endfunction()