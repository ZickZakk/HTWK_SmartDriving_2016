set(FILTER htwk_signal_restrictor)

adtf_add_filter(
        ${FILTER}
        stdafx.h
        stdafx.cpp
        SignalRestrictor.h
        SignalRestrictor.cpp
)

adtf_install_plugin(${FILTER} ${CMAKE_INSTALL_PREFIX}/filter)

add_subdirectory(./../htwk_logger htwk_logger)

include_directories(${HTWK_LOGGER_INCLUDE_DIR})

target_link_libraries(${FILTER} htwk_logger)