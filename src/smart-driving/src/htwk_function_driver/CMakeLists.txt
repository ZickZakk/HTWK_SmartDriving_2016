set(FILTER htwk_function_driver)

adtf_add_filter(
        ${FILTER}
        stdafx.h
        stdafx.cpp
        FunctionDriver.h
        FunctionDriver.cpp
)

adtf_install_plugin(${FILTER} ${CMAKE_INSTALL_PREFIX}/filter)

add_subdirectory(./../htwk_logger htwk_logger)
include_directories(${HTWK_LOGGER_INCLUDE_DIR})
target_link_libraries(${FILTER} htwk_logger)

add_subdirectory(./../htwk_utils htwk_utils)
include_directories(${HTWK_UTILS_INCLUDE_DIR})
target_link_libraries(${LOGGER} htwk_utils)