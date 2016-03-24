set(FILTER_DECISON_TEST htwk_drive_decision_test)

adtf_add_filter(
        ${FILTER_DECISON_TEST}
        stdafx.h
        stdafx.cpp
        DriveDecisionTest.cpp
        DriveDecisionTest.h
)

adtf_install_plugin(${FILTER_DECISON_TEST} ${CMAKE_INSTALL_PREFIX}/filter)

add_subdirectory(./../htwk_logger htwk_logger)
include_directories(${HTWK_LOGGER_INCLUDE_DIR})
target_link_libraries(${FILTER_DECISON_TEST} htwk_logger)
