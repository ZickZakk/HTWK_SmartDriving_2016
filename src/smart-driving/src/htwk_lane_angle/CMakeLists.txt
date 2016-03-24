set(LANE_ANGLE_LIB htwk_lane_angle)

set(HTWK_LANE_ANGLE_LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

if (TARGET ${LANE_ANGLE_LIB})
    message("${LANE_ANGLE_LIB} is already defined")
else()
    ADD_LIBRARY(${LANE_ANGLE_LIB} STATIC
            LaneAngle.cpp
            LaneAngle.h)

    target_link_libraries(${LANE_ANGLE_LIB} ${OpenCV_LIBS})
    target_link_libraries(${LANE_ANGLE_LIB} htwk_logger)

endif ()


