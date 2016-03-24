#
# Misc
#

add_subdirectory(./../htwk_logger htwk_logger)
include_directories(${HTWK_LOGGER_INCLUDE_DIR})

add_subdirectory(./../htwk_structs htwk_structs)
include_directories(${HTWK_STRUCTS_INCLUDE_DIR})

add_subdirectory(./../htwk_utils htwk_utils)
include_directories(${HTWK_UTILS_INCLUDE_DIR})

add_subdirectory(./../htwk_carconfig htwk_carconfig)
include_directories(${HTWK_CARCONFIG_INCLUDE_DIR})

add_subdirectory(./../htwk_base_decision_module htwk_base_decision_module)
include_directories(${HTWK_BASE_DECISION_LIB_INCLUDE_DIR})

add_subdirectory(./../htwk_world_service_h htwk_world_service_h)
include_directories(${HTWK_WORLD_SERVICE_H_INCLUDE_DIR})

include_directories(${OpenCV_INCLUDE_DIR})

#
# Cross Parking
#

set(FILTER_CROSS htwk_cross_parking)

adtf_add_filter(
        ${FILTER_CROSS}
        stdafx.h
        stdafx.cpp
        ParkingBase.h
        ParkingBase.cpp
        CrossParking.h
        CrossParking.cpp
)

adtf_install_plugin(${FILTER_CROSS} ${CMAKE_INSTALL_PREFIX}/filter)

target_link_libraries(${FILTER_CROSS} htwk_base_decision_module htwk_carconfig htwk_logger htwk_structs htwk_utils htwk_world_service_h)
target_link_libraries(${FILTER_CROSS} ${OpenCV_LIBS})

#
# Parallel Parking
#

set(FILTER_PARALLEL htwk_parallel_parking)

adtf_add_filter(
        ${FILTER_PARALLEL}
        stdafx.h
        stdafx.cpp
        ParkingBase.h
        ParkingBase.cpp
        ParallelParking.h
        ParallelParking.cpp
)

adtf_install_plugin(${FILTER_PARALLEL} ${CMAKE_INSTALL_PREFIX}/filter)

target_link_libraries(${FILTER_PARALLEL} htwk_logger htwk_structs htwk_utils htwk_carconfig htwk_base_decision_module)
target_link_libraries(${FILTER_PARALLEL} ${OpenCV_LIBS})

#
# Pull Out
#

set(FILTER_PULL htwk_pull_out)

adtf_add_filter(
        ${FILTER_PULL}
        stdafx.h
        stdafx.cpp
        ParkingBase.h
        ParkingBase.cpp
        PullOut.h
        PullOut.cpp
)

adtf_install_plugin(${FILTER_PULL} ${CMAKE_INSTALL_PREFIX}/filter)

target_link_libraries(${FILTER_PULL} htwk_logger htwk_structs htwk_utils htwk_carconfig htwk_base_decision_module)
target_link_libraries(${FILTER_PULL} ${OpenCV_LIBS})
