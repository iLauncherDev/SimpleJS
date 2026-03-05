block()
    set(SIMPLEJS_PATH "${TEST_PLUGIN_BASE_SOURCE_PATH}")
    set(SIMPLEJS_INCLUDE_PATH "${SIMPLEJS_PATH}/inc")

    list(APPEND SIMPLEJS_SOURCE
        ${SIMPLEJS_PATH}/main.c
    )

    add_library(test_plugin SHARED ${SIMPLEJS_SOURCE})
    set_target_properties(test_plugin PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        C_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )

    target_include_directories(test_plugin PRIVATE ${SIMPLEJS_INCLUDE_PATH})
    target_include_directories(test_plugin PUBLIC ${SIMPLEJS_SDK_INCLUDE_PATH})

    target_link_libraries(test_plugin PUBLIC simplejs_core)
endblock()
