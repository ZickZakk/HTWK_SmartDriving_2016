set(FILTER htwk_imu_merger)
set(FILTER2 htwk_imu_merger_test)

adtf_add_filter(
        ${FILTER}
        stdafx.h
        stdafx.cpp
        ImuMerger.h
        ImuMerger.cpp
)
adtf_add_filter(
        ${FILTER2}
        stdafx.h
        stdafx.cpp
        ImuMergerTest.h
        ImuMergerTest.cpp
)

adtf_install_plugin(${FILTER} ${CMAKE_INSTALL_PREFIX}/filter)
adtf_install_plugin(${FILTER2} ${CMAKE_INSTALL_PREFIX}/filter)

add_subdirectory(./../htwk_logger htwk_logger)
include_directories(${HTWK_LOGGER_INCLUDE_DIR})
target_link_libraries(${FILTER} htwk_logger)
target_link_libraries(${FILTER2} htwk_logger)
