set(CARCONFIG htwk_carconfig)
set(HTWK_CARCONFIG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

add_subdirectory(./../htwk_logger htwk_logger)
include_directories(${HTWK_LOGGER_INCLUDE_DIR})

if (TARGET ${CARCONFIG})
    message("${CARCONFIG} is already defined")
else ()
    file(GLOB_RECURSE FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
    file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

    ADD_LIBRARY(${CARCONFIG} STATIC ${FILES} ${HEADERS})

    target_link_libraries(${CARCONFIG} htwk_logger htwk_utils)
endif ()
