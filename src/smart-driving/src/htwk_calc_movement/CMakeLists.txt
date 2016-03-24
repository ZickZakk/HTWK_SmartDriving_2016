set(MOVEMENT htwk_calc_movement)

set(HTWK_CALC_MOVEMENT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

include_directories(${OpenCV_INCLUDE_DIR})

if (TARGET ${MOVEMENT})
    message("${MOVEMENT} is already defined")
else()
    ADD_LIBRARY(${MOVEMENT} STATIC
            MovementCalculator.cpp
            MovementCalculator.h)

    target_link_libraries(${MOVEMENT} ${OpenCV_LIBS})
endif ()
