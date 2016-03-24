set(FILTER_DECISON htwk_drive_decision)

add_subdirectory(./../htwk_logger htwk_logger)
include_directories(${HTWK_LOGGER_INCLUDE_DIR})

add_subdirectory(./../htwk_structs htwk_structs)
include_directories(${HTWK_STRUCTS_INCLUDE_DIR})

adtf_add_filter(
        ${FILTER_DECISON}
        stdafx.h
        stdafx.cpp
        DriveDecision.cpp
        DriveDecision.h
)

adtf_install_plugin(${FILTER_DECISON} ${CMAKE_INSTALL_PREFIX}/filter)

target_link_libraries(${FILTER_DECISON} htwk_logger htwk_structs)
