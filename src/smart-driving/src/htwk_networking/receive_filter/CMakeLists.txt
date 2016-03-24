
#install(FILES data_exchange_raw_filter.plb.plugindescription DESTINATION src/examples/bin/debug CONFIGURATIONS Debug)
#install(FILES data_exchange_raw_filter.plb.plugindescription DESTINATION src/examples/bin CONFIGURATIONS Release RelWithDebInfo)

set(FILTER htwk_newtork_reciver)

adtf_add_filter(
        ${FILTER}
        stdafx.h
        stdafx.cpp
        raw_filter.h
        raw_filter.cpp
)

adtf_install_plugin(${FILTER} ${CMAKE_INSTALL_PREFIX}/filter)

add_subdirectory(./../../htwk_logger htwk_logger)
include_directories(${HTWK_LOGGER_INCLUDE_DIR})
add_subdirectory(./../../htwk_utils htwk_utils)
include_directories(${HTWK_UTILS_INCLUDE_DIR})

target_link_libraries(${FILTER} htwk_logger)
target_link_libraries(${FILTER} htwk_utils)
target_link_libraries(${FILTER} ${OpenCV_LIBS})
