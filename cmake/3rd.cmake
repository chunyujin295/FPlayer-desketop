include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake)
set(SPDLOG_BUILD_SHARED ON CACHE BOOL "spdlog Build Shared Lib" FORCE) # 覆盖spdlog的Option，生成静态库
# Logger
CPMAddPackage(
        NAME Logger
        GIT_REPOSITORY git@github.com:chunyujin295/Logger.git
        GIT_TAG v1.0.10
)
