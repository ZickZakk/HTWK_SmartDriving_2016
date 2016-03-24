set(FILTER htwk_crossing_detection)

adtf_add_filter(
        ${FILTER}
        ../htwk_base_decision_module/BaseDecisionModule.cpp
        ../htwk_base_decision_module/BaseDecisionModule.h
        stdafx.h
        stdafx.cpp
        CrossingDetection.h
        CrossingDetection.cpp
)

adtf_install_plugin(${FILTER} ${CMAKE_INSTALL_PREFIX}/filter)

add_subdirectory(./../htwk_logger htwk_logger)
include_directories(${HTWK_LOGGER_INCLUDE_DIR})
add_subdirectory(./../htwk_utils htwk_utils)
include_directories(${HTWK_UTILS_INCLUDE_DIR})

target_link_libraries(${FILTER} htwk_logger)
target_link_libraries(${FILTER} htwk_utils)
target_link_libraries(${FILTER} ${OpenCV_LIBS})
