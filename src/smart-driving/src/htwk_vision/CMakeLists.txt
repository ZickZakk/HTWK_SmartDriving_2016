add_subdirectory(LaneDetection)

set(VISION_LIB htwk_vision)

set(HTWK_VISION_LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

if (TARGET ${VISION_LIB})
    message("${VISION_LIB} is already defined")
else ()
    ADD_LIBRARY(${VISION_LIB} STATIC Vision.cpp Vision.h)

    include_directories(${OpenCV_INCLUDE_DIR})

    target_link_libraries(${VISION_LIB} htwk_lanes)
    target_link_libraries(${VISION_LIB} ${OpenCV_LIBS})
endif ()


