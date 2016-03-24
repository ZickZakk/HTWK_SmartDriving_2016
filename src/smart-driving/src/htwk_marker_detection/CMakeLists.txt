set(FILTER htwk_marker_detection)

adtf_add_filter(
        ${FILTER}
        stdafx.h
        stdafx.cpp
        MarkerDetection.h
        MarkerDetection.cpp
)

adtf_install_plugin(${FILTER} ${CMAKE_INSTALL_PREFIX}/filter)

add_subdirectory(./../htwk_logger htwk_logger)
add_subdirectory(./../htwk_utils htwk_utils)

include_directories(${HTWK_LOGGER_INCLUDE_DIR})
include_directories(${HTWK_UTILS_INCLUDE_DIR})
include_directories(${OpenCV_INCLUDE_DIR})
include_directories(${aruco_INCLUDE_DIR})

target_link_libraries(${FILTER} htwk_logger)
target_link_libraries(${FILTER} htwk_utils)
target_link_libraries(${FILTER} ${OpenCV_LIBS})
target_link_libraries(${FILTER} ${aruco_LIBS})
