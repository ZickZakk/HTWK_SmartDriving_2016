set(FILTER htwk_lane_detection)

adtf_add_filter(
        ${FILTER}
        stdafx.h
        stdafx.cpp
        lane_detection.h
        lane_detection.cpp
)

adtf_install_plugin(${FILTER} ${CMAKE_INSTALL_PREFIX}/filter)

add_subdirectory(./../htwk_logger htwk_logger)
add_subdirectory(./../htwk_vision htwk_vision)

include_directories(${HTWK_LOGGER_INCLUDE_DIR})
include_directories(${HTWK_VISION_INCLUDE_DIR})
include_directories(${OpenCV_INCLUDE_DIR})

target_link_libraries(${FILTER} htwk_logger)
target_link_libraries(${FILTER} htwk_vision)
target_link_libraries(${FILTER} ${OpenCV_LIBS})

#install(FILES left.bmp DESTINATION ${CMAKE_INSTALL_PREFIX}/filter/debug)
#install(FILES left.bmp DESTINATION ${CMAKE_INSTALL_PREFIX}/filter/)
