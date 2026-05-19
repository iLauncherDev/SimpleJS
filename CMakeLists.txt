cmake_minimum_required(VERSION 3.20)
project(simplejs C)

set(SIMPLEJS_SDK_INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/sdk/include")

include(${CMAKE_CURRENT_LIST_DIR}/sdk/cmake/default.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/simplejs.cmake)
