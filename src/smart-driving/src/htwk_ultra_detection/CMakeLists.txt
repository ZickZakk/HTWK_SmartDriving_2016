#
# Misc
#

add_subdirectory(./../htwk_logger htwk_logger)
add_subdirectory(./../htwk_structs htwk_structs)

include_directories(${HTWK_LOGGER_INCLUDE_DIR})
include_directories(${HTWK_STRUCTS_INCLUDE_DIR})
include_directories(${OpenCV_INCLUDE_DIR})

#
# Filter
#

set(FILTER htwk_ultra_detection)

adtf_add_filter(
        ${FILTER}
        stdafx.h
        stdafx.cpp
        htwk_ultra_detection.h
        htwk_ultra_detection.cpp
        htwk_ultra_grid.h
        htwk_ultra_grid.cpp
)

adtf_install_plugin(${FILTER} ${CMAKE_INSTALL_PREFIX}/filter)

target_link_libraries(${FILTER} htwk_logger)
target_link_libraries(${FILTER} htwk_structs)
target_link_libraries(${FILTER} ${OpenCV_LIBS})
