set(SIMPLEJS_LIB_NAME simplejs_core_cpp)
set(SIMPLEJS_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
set(SIMPLEJS_INCLUDE_PATH "${SIMPLEJS_PATH}/inc")

list(APPEND SIMPLEJS_SOURCE
    ${SIMPLEJS_PATH}/main.cpp

    ${SIMPLEJS_PATH}/object.cpp
    ${SIMPLEJS_PATH}/variable.cpp
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
target_link_libraries(${SIMPLEJS_LIB_NAME} PRIVATE simplejs_hook simplejs_platform)
