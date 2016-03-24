set(LOGGER htwk_logger)

set(HTWK_LOGGER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIR} PARENT_SCOPE)

if (TARGET ${LOGGER})
    message("${LOGGER} is already defined")
else()
    ADD_LIBRARY(${LOGGER} STATIC
            Logger.cpp
            Logger.h)

    include_directories(${OpenCV_INCLUDE_DIR})

    target_link_libraries(${LOGGER} htwk_utils)
    target_link_libraries(${LOGGER} ${OpenCV_LIBS})
endif ()
