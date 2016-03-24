set(FILTER2 htwk_median_values_filter)

adtf_add_filter(
        ${FILTER2}
        stdafx.h
        stdafx.cpp
        Median_values.h
        Median_values.cpp
)

adtf_install_plugin(${FILTER2} ${CMAKE_INSTALL_PREFIX}/filter)

add_subdirectory(./../htwk_median_values htwk_median_lib)
add_subdirectory(./../htwk_logger htwk_logger)
include_directories(${HTWK_MEDIAN_LIB_INCLUDE_DIR})
include_directories(${HTWK_LOGGER_INCLUDE_DIR})
target_link_libraries(${FILTER2} htwk_median_lib)
target_link_libraries(${FILTER2} htwk_logger)
