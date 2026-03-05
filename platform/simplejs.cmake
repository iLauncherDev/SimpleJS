block()
    set(SIMPLEJS_LIB_NAME simplejs_platform)
    set(SIMPLEJS_PATH "${SIMPLEJS_PLATFORM_BASE_SOURCE_PATH}")
    set(SIMPLEJS_INCLUDE_PATH "${SIMPLEJS_PATH}/inc")

    if(WIN32)
        set(SIMPLEJS_PLATFORM "windows")
    else()
        set(SIMPLEJS_PLATFORM "posix")
    endif()

    list(APPEND SIMPLEJS_SOURCE
        ${SIMPLEJS_PATH}/lib/platform/sleep-${SIMPLEJS_PLATFORM}.c
        ${SIMPLEJS_PATH}/lib/sleep.c

        ${SIMPLEJS_PATH}/lib/platform/shared_lib-${SIMPLEJS_PLATFORM}.c
        ${SIMPLEJS_PATH}/lib/shared_lib.c

        ${SIMPLEJS_PATH}/lib/platform/thread-${SIMPLEJS_PLATFORM}.c
        ${SIMPLEJS_PATH}/lib/thread.c
    )

    add_library(${SIMPLEJS_LIB_NAME} SHARED ${SIMPLEJS_SOURCE})
    set_target_properties(${SIMPLEJS_LIB_NAME} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        C_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )

    target_include_directories(${SIMPLEJS_LIB_NAME} PRIVATE ${SIMPLEJS_INCLUDE_PATH})
    target_include_directories(${SIMPLEJS_LIB_NAME} PUBLIC ${SIMPLEJS_SDK_INCLUDE_PATH})
    target_link_libraries(${SIMPLEJS_LIB_NAME} PRIVATE simplejs_hook)
endblock()
