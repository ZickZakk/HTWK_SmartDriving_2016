set(MEDIAN_LIB htwk_median_lib)

set(HTWK_MEDIAN_LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

if (TARGET ${MEDIAN_LIB})
    message("${MEDIAN_LIB} is already defined")
else()
    ADD_LIBRARY(${MEDIAN_LIB} STATIC
            Median_values_lib.cpp
            Median_values_lib.h)

endif ()
