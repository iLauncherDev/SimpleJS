cmake_minimum_required(VERSION 3.20)
project(simplejs C ASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options("/experimental:c11atomics")
endif()

set(SIMPLEJS_SDK_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/sdk/include")

set(SIMPLEJS_HOOK_BASE_SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/hook")
include(${SIMPLEJS_HOOK_BASE_SOURCE_PATH}/simplejs.cmake)

set(SIMPLEJS_PLATFORM_BASE_SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/platform")
include(${SIMPLEJS_PLATFORM_BASE_SOURCE_PATH}/simplejs.cmake)

set(SIMPLEJS_CORE_BASE_SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/core")
include(${SIMPLEJS_CORE_BASE_SOURCE_PATH}/simplejs.cmake)

set(TEST_PLUGIN_BASE_SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/test_plugin")
include(${TEST_PLUGIN_BASE_SOURCE_PATH}/test_plugin.cmake)

set(PROGRAM_BASE_SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/program")
list(APPEND PROGRAM_SOURCE
    ${PROGRAM_BASE_SOURCE_PATH}/main.c
)

add_executable(${PROJECT_NAME} ${PROGRAM_SOURCE})

target_link_libraries(${PROJECT_NAME} PUBLIC simplejs_core simplejs_hook simplejs_platform)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROGRAM_BASE_SOURCE_PATH}/inc)
target_include_directories(${PROJECT_NAME} PUBLIC ${SIMPLEJS_CORE_BASE_SOURCE_PATH}/inc)
