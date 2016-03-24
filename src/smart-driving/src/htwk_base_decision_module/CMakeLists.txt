set(BASE_DECISION_LIB htwk_base_decision_module)

set(HTWK_BASE_DECISION_LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

add_subdirectory(./../htwk_logger htwk_logger)
include_directories(${HTWK_LOGGER_INCLUDE_DIR})

add_subdirectory(./../htwk_structs htwk_structs)
include_directories(${HTWK_STRUCTS_INCLUDE_DIR})

add_subdirectory(./../htwk_carconfig htwk_carconfig)
include_directories(${HTWK_CARCONFIG_INCLUDE_DIR})

add_subdirectory(./../htwk_world_service_h htwk_world_service_h)
include_directories(${HTWK_WORLD_SERVICE_H_INCLUDE_DIR})

if (TARGET ${BASE_DECISION_LIB})
    message("${BASE_DECISION_LIB} is already defined")
else ()
    ADD_LIBRARY(${BASE_DECISION_LIB} STATIC
            stdafx.h
            stdafx.cpp
            BaseDecisionModule.h
            BaseDecisionModule.cpp)

    target_link_libraries(${BASE_DECISION_LIB} htwk_carconfig htwk_logger htwk_structs htwk_world_service_h)
    target_link_libraries(${BASE_DECISION_LIB} ${ADTF_LIBS} ${OPENCV_LIBS})
endif ()
