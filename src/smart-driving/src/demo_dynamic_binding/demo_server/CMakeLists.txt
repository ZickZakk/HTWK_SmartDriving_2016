set(FILTER demo_server)

adtf_add_filter(
        ${FILTER}
        ../include/demo_published_interface.h
        stdafx.h
        stdafx.cpp
        demo_server_filter.h
        demo_server_filter.cpp
)

adtf_install_plugin(${FILTER} ${CMAKE_INSTALL_PREFIX}/filter)

add_subdirectory(./../../htwk_logger htwk_logger)
include_directories(${HTWK_LOGGER_INCLUDE_DIR})
add_subdirectory(./../../htwk_utils htwk_utils)
include_directories(${HTWK_UTILS_INCLUDE_DIR})

target_link_libraries(${FILTER} htwk_logger)
target_link_libraries(${FILTER} htwk_utils)
target_link_libraries(${FILTER} ${OpenCV_LIBS})

#adtf_add_filter(demo_binding_server stdafx.h
#        stdafx.cpp
#        demo_server_filter.h
#        demo_server_filter.cpp
#        ../include/demo_published_interface.h)
#adtf_install_plugin(demo_binding_server src/examples/bin)
#adtf_set_folder(demo_binding_server examples/filters/demo_binding)

#install(FILES demo_binding_server.plb.plugindescription DESTINATION src/examples/bin/debug CONFIGURATIONS Debug)
#install(FILES demo_binding_server.plb.plugindescription DESTINATION src/examples/bin CONFIGURATIONS Release RelWithDebInfo)
