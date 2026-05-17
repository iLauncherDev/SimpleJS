block()
    set(SIMPLEJS_LIB_NAME test_plugin)
    set(SIMPLEJS_PATH "${TEST_PLUGIN_BASE_SOURCE_PATH}")
    set(SIMPLEJS_INCLUDE_PATH "${SIMPLEJS_PATH}/inc")

    list(APPEND SIMPLEJS_SOURCE
        ${SIMPLEJS_PATH}/main.c
    )

    add_library(${SIMPLEJS_LIB_NAME} SHARED ${SIMPLEJS_SOURCE})
    set_target_properties(${SIMPLEJS_LIB_NAME} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        C_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )

    target_include_directories(${SIMPLEJS_LIB_NAME} PRIVATE ${SIMPLEJS_INCLUDE_PATH})
    target_include_directories(${SIMPLEJS_LIB_NAME} PUBLIC ${SIMPLEJS_SDK_INCLUDE_PATH})

    target_link_libraries(${SIMPLEJS_LIB_NAME} PUBLIC simplejs_core)
endblock()
