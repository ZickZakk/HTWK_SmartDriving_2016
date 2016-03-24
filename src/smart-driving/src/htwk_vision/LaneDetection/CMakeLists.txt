set(HTWK_LANE_LIB htwk_lanes)

set(HTWK_LANE_LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

include_directories(${OpenCV_INCLUDE_DIR})

if (TARGET line_end)
    message("line_end is already defined")
else ()
    ADD_LIBRARY(line_end STATIC LaneLineEnd.cpp LaneLineEnd.h)
    target_link_libraries(line_end ${OpenCV_LIBS})
endif ()

if (TARGET ipm_border)
    message("ipm_border is already defined")
else ()
    ADD_LIBRARY(ipm_border STATIC IPMBorder.cpp IPMBorder.h)
    target_link_libraries(ipm_border ${OpenCV_LIBS} htwk_utils)
endif ()

if (TARGET lane_detection_data)
    message("lane_detection_data is already defined")
else ()
    ADD_LIBRARY(lane_detection_data STATIC LaneDetectionData.cpp LaneDetectionData.h)
    target_link_libraries(lane_detection_data ${OpenCV_LIBS} htwk_utils)
endif ()


if (TARGET ${HTWK_LANE_LIB})
    message("${HTWK_LANE_LIB} is already defined")
else ()
    file(GLOB FILES ${CMAKE_CURRENT_SOURCE_DIR}/*Line.cpp)
    file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*Line.h)

    ADD_LIBRARY(${HTWK_LANE_LIB} STATIC ${FILES} ${HEADERS})

    target_link_libraries(
            ${HTWK_LANE_LIB}
            ${OpenCV_LIBS}
            htwk_crossing
            htwk_utils
            ipm_border
            line_end
            lane_detection_data)

endif ()

add_subdirectory(CrossingDetection)

