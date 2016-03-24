set(FILTER htwk_movement_analyzer)

adtf_add_filter(
        ${FILTER}
        stdafx.h
        stdafx.cpp
        Movement_analyzer.h
        Movement_analyzer.cpp
)

adtf_install_plugin(${FILTER} ${CMAKE_INSTALL_PREFIX}/filter)

add_subdirectory(./../htwk_logger htwk_logger)
add_subdirectory(./../htwk_calc_movement htwk_calc_movement)

include_directories(${HTWK_CALC_MOVEMENT_INCLUDE_DIR})
include_directories(${HTWK_LOGGER_INCLUDE_DIR})
include_directories(${OpenCV_INCLUDE_DIR})

target_link_libraries(${FILTER} htwk_logger)
target_link_libraries(${FILTER} htwk_calc_movement)
target_link_libraries(${FILTER} ${OpenCV_LIBS})
