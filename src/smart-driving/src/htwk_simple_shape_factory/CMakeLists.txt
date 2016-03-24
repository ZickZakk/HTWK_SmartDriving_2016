set(FILTER htwk_simple_shape_filter)

adtf_add_filter(
        ${FILTER}
        stdafx.h
        stdafx.cpp
        simple_shape.h
        simple_shape.cpp
)

adtf_install_plugin(${FILTER} ${CMAKE_INSTALL_PREFIX}/filter)