#
# Misc
#

add_subdirectory(./../htwk_logger htwk_logger)
include_directories(${HTWK_LOGGER_INCLUDE_DIR})

add_subdirectory(./../htwk_utils htwk_utils)
include_directories(${HTWK_UTILS_INCLUDE_DIR})

add_subdirectory(./../htwk_carconfig htwk_carconfig)
include_directories(${HTWK_CARCONFIG_INCLUDE_DIR})

add_subdirectory(./../htwk_world_service_h htwk_world_service_h)
include_directories(${HTWK_WORLD_SERVICE_H_INCLUDE_DIR})

#
# Service
#

set(SERVICE htwk_world_service)

adtf_add_service(${SERVICE}
        stdafx.h
        stdafx.cpp
        WorldService.cpp)

adtf_install_plugin(${SERVICE} ${CMAKE_INSTALL_PREFIX}/filter)
adtf_set_folder(${SERVICE} ${CMAKE_INSTALL_PREFIX}/filter)

if (NOT ADTF_DISABLE_INSTALL)
    install(FILES htwk_world_service.manifest DESTINATION ${CMAKE_INSTALL_PREFIX}/filter)
    install(FILES debug/htwk_world_service.manifest DESTINATION ${CMAKE_INSTALL_PREFIX}/filter/debug)
endif (NOT ADTF_DISABLE_INSTALL)

target_link_libraries(${SERVICE} htwk_logger htwk_utils htwk_carconfig htwk_world_service_h)
