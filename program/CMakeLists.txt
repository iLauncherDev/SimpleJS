set(SIMPLEJS_EXEC_NAME simplejs)
set(SIMPLEJS_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
set(SIMPLEJS_INCLUDE_PATH "${SIMPLEJS_PATH}/inc")

list(APPEND SIMPLEJS_SOURCE
    ${SIMPLEJS_PATH}/main.c
)

add_executable(${SIMPLEJS_EXEC_NAME} ${SIMPLEJS_SOURCE})

target_include_directories(${SIMPLEJS_EXEC_NAME} PRIVATE ${SIMPLEJS_INCLUDE_PATH})

target_include_directories(${SIMPLEJS_EXEC_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../core/inc)
target_include_directories(${SIMPLEJS_EXEC_NAME} PUBLIC ${SIMPLEJS_SDK_INCLUDE_PATH})

target_link_libraries(${SIMPLEJS_EXEC_NAME} simplejs_core simplejs_platform simplejs_hook)
