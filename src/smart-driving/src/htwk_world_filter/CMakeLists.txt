#
# Misc
#

include_directories(${OpenCV_INCLUDE_DIR})

add_subdirectory(./../htwk_logger htwk_logger)
include_directories(${HTWK_LOGGER_INCLUDE_DIR})

add_subdirectory(./../htwk_structs htwk_structs)
include_directories(${HTWK_STRUCTS_INCLUDE_DIR})

add_subdirectory(./../htwk_utils htwk_utils)
include_directories(${HTWK_UTILS_INCLUDE_DIR})

add_subdirectory(./../htwk_world_service_h htwk_world_service_h)
include_directories(${HTWK_WORLD_SERVICE_H_INCLUDE_DIR})

#
# Filter In
#

set(FILTER_IN htwk_world_filter_in)

adtf_add_filter(${FILTER_IN}
        stdafx.h
        stdafx.cpp
        WorldFilterIn.h
        WorldFilterIn.cpp)

adtf_install_plugin(${FILTER_IN} ${CMAKE_INSTALL_PREFIX}/filter)

adtf_set_folder(${FILTER_IN} ${CMAKE_INSTALL_PREFIX}/filter)

target_link_libraries(${FILTER_IN} htwk_logger htwk_utils htwk_structs htwk_world_service_h)

#
# Filter Out
#

#set(FILTER_OUT htwk_world_filter_out)
#
#adtf_add_filter(${FILTER_OUT}
#        stdafx.h
#        stdafx.cpp
#        WorldFilterOut.h
#        WorldFilterOut.cpp)
#
#adtf_install_plugin(${FILTER_OUT} ${CMAKE_INSTALL_PREFIX}/filter)
#
#adtf_set_folder(${FILTER_OUT} ${CMAKE_INSTALL_PREFIX}/filter)
#
#target_link_libraries(${FILTER_OUT} htwk_logger)
#target_link_libraries(${FILTER_OUT} htwk_world_service_h)
#target_link_libraries(${FILTER_OUT} ${OpenCV_LIBS})
