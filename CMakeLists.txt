cmake_minimum_required(VERSION 3.20)
project(simplejs C ASM)

set(SIMPLEJS_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SIMPLEJS_SDK_INCLUDE_PATH "${SIMPLEJS_BASE_DIR}/sdk/include")

include(${SIMPLEJS_BASE_DIR}/sdk/cmake/default.cmake)
include(${SIMPLEJS_BASE_DIR}/simplejs.cmake)
