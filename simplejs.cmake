add_subdirectory(${SIMPLEJS_BASE_DIR}/hook)
add_subdirectory(${SIMPLEJS_BASE_DIR}/platform)
add_subdirectory(${SIMPLEJS_BASE_DIR}/core)

if(NOT SIMPLEJS_DISABLE_TEST_PLUGIN)
    add_subdirectory(${SIMPLEJS_BASE_DIR}/test_plugin)
endif()

add_subdirectory(${SIMPLEJS_BASE_DIR}/program)
