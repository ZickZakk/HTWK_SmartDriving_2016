set(FILTER htwk_messagesbus_netio)

adtf_add_filter(
        ${FILTER}
        stdafx.h
        stdafx.cpp
        demonetiofilter.h
        demonetiofilter.cpp
)

adtf_install_plugin(${FILTER} ${CMAKE_INSTALL_PREFIX}/filter)

add_subdirectory(./../htwk_logger htwk_logger)

include_directories(${HTWK_LOGGER_INCLUDE_DIR})
include_directories(${OpenCV_INCLUDE_DIR})

target_link_libraries(${FILTER} htwk_logger)
target_link_libraries(${FILTER} ${OpenCV_LIBS})
