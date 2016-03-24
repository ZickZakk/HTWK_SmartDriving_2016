#
# Misc
#

add_subdirectory(./../htwk_logger htwk_logger)
include_directories(${HTWK_LOGGER_INCLUDE_DIR})

add_subdirectory(./../htwk_utils htwk_utils)
include_directories(${HTWK_UTILS_INCLUDE_DIR})

add_subdirectory(./../htwk_structs htwk_strcuts)
include_directories(${HTWK_STRUCTS_INCLUDE_DIR})

#
# Filter
#

set(FILTER htwk_state_controller_new)

adtf_add_filter(${FILTER}
        stdafx.cpp
        stdafx.h
        StateControllerNew.cpp
        StateControllerNew.h)

adtf_install_plugin(${FILTER} ${CMAKE_INSTALL_PREFIX}/filter)

target_link_libraries(${FILTER} htwk_logger htwk_structs htwk_utils)
