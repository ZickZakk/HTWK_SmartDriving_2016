set(FILTER htwk_trigger)

adtf_add_filter(
        ${FILTER}
        stdafx.h
        stdafx.cpp
        Trigger.h
        Trigger.cpp
)

adtf_install_plugin(${FILTER} ${CMAKE_INSTALL_PREFIX}/filter)
