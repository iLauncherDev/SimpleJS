block()
    set(SIMPLEJS_LIB_NAME simplejs_core)
    set(SIMPLEJS_PATH "${SIMPLEJS_CORE_BASE_SOURCE_PATH}")
    set(SIMPLEJS_INCLUDE_PATH "${SIMPLEJS_PATH}/inc")

    list(APPEND SIMPLEJS_SOURCE
        ${SIMPLEJS_PATH}/lib/bitmap.c

        ${SIMPLEJS_PATH}/lib/list.c

        ${SIMPLEJS_PATH}/lib/utf8.c

        ${SIMPLEJS_PATH}/lib/status.c

        ${SIMPLEJS_PATH}/lib/s_string.c

        ${SIMPLEJS_PATH}/number/alu-iptr.c
        ${SIMPLEJS_PATH}/number/alu-i32.c
        ${SIMPLEJS_PATH}/number/alu-i64.c

        ${SIMPLEJS_PATH}/number/alu-uiptr.c
        ${SIMPLEJS_PATH}/number/alu-ui32.c
        ${SIMPLEJS_PATH}/number/alu-ui64.c

        ${SIMPLEJS_PATH}/number/alu-f32.c
        ${SIMPLEJS_PATH}/number/alu-f64.c

        ${SIMPLEJS_PATH}/number/alu.c
        ${SIMPLEJS_PATH}/number/utils.c
        ${SIMPLEJS_PATH}/number/number.c

        ${SIMPLEJS_PATH}/tokenizer/tokenizer.c

        ${SIMPLEJS_PATH}/parser/parser.c
        ${SIMPLEJS_PATH}/parser/pratt-parser.c

        ${SIMPLEJS_PATH}/builtin_object/dynamic_object.c

        ${SIMPLEJS_PATH}/object/object.c
        ${SIMPLEJS_PATH}/object/proxy.c

        ${SIMPLEJS_PATH}/variable/variable.c

        ${SIMPLEJS_PATH}/bytecode/vm.c
        ${SIMPLEJS_PATH}/bytecode/compiler.c
        ${SIMPLEJS_PATH}/bytecode/bytecode.c

        ${SIMPLEJS_PATH}/mm/gc.c

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
    target_link_libraries(${SIMPLEJS_LIB_NAME} PRIVATE simplejs_hook)
endblock()
