set(FILTER htwk_distance_sensor_merger)

adtf_add_filter(
        ${FILTER}
        stdafx.h
        stdafx.cpp
        ../htwk_depth_detection/ConnectedComponentFinder.h
        ../htwk_depth_detection/ConnectedComponentFinder.cpp
        DistanceSensorMerger.h
        DistanceSensorMerger.cpp
)

adtf_install_plugin(${FILTER} ${CMAKE_INSTALL_PREFIX}/filter)

add_subdirectory(./../htwk_logger htwk_logger)
add_subdirectory(./../htwk_vision htwk_vision)

include_directories(${HTWK_LOGGER_INCLUDE_DIR})
include_directories(${OpenCV_INCLUDE_DIR})

target_link_libraries(${FILTER} htwk_logger)
target_link_libraries(${FILTER} ${OpenCV_LIBS})
