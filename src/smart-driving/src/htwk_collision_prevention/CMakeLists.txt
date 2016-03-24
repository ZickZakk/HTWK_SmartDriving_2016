set(FILTER htwk_collision_prevention)

add_subdirectory(./../htwk_logger htwk_logger)
include_directories(${HTWK_LOGGER_INCLUDE_DIR})

add_subdirectory(./../htwk_carconfig htwk_carconfig)
include_directories(${HTWK_CARCONFIG_INCLUDE_DIR})

add_subdirectory(./../htwk_utils htwk_utils)
include_directories(${HTWK_UTILS_INCLUDE_DIR})

add_subdirectory(./../htwk_structs htwk_structs)
include_directories(${HTWK_STRUCTS_INCLUDE_DIR})

add_subdirectory(./../htwk_base_decision_module htwk_base_decision_module)
include_directories(${HTWK_BASE_DECISION_LIB_INCLUDE_DIR})

add_subdirectory(./../htwk_world_service_h htwk_world_service_h)
include_directories(${HTWK_WORLD_SERVICE_H_INCLUDE_DIR})

adtf_add_filter(
        ${FILTER}
        stdafx.h
        stdafx.cpp
        CollisionPrevention.h
        CollisionPrevention.cpp
)

adtf_install_plugin(${FILTER} ${CMAKE_INSTALL_PREFIX}/filter)

target_link_libraries(${FILTER} htwk_base_decision_module htwk_carconfig htwk_logger htwk_structs htwk_utils htwk_world_service_h)
target_link_libraries(${FILTER} ${OpenCV_LIBS})
