add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/hook)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/platform)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/core)

if(NOT SIMPLEJS_DISABLE_TEST_PLUGIN)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test_plugin)
endif()

if(NOT SIMPLEJS_DISABLE_MAIN_PROGRAM)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/program)
endif()
