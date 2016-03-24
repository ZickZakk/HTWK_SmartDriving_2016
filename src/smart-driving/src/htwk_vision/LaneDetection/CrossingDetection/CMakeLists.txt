set(HTWK_CROSSING_LIB htwk_crossing)

set(HTWK_CROSSING_LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

include_directories(${OpenCV_INCLUDE_DIR})

if (TARGET ${HTWK_CROSSING_LIB})
    message("${HTWK_CROSSING_LIB} is already defined")
else ()
    file(GLOB_RECURSE FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
    file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

    ADD_LIBRARY(${HTWK_CROSSING_LIB} STATIC ${FILES} ${HEADERS})

    target_link_libraries(
            ${HTWK_CROSSING_LIB}
            ${OpenCV_LIBS}
            htwk_utils
            htwk_logger)

    if (NOT ADTF_DISABLE_INSTALL)
        file(GLOB_RECURSE TEMPLATE_PICS ${CMAKE_CURRENT_SOURCE_DIR}/*.bmp)

        install(FILES ${TEMPLATE_PICS} DESTINATION ${CMAKE_INSTALL_PREFIX}/filter/resources)
        install(FILES ${TEMPLATE_PICS} DESTINATION ${CMAKE_INSTALL_PREFIX}/filter/debug/resources)
    endif (NOT ADTF_DISABLE_INSTALL)
endif ()
