set(STRUCTS htwk_structs)

set(HTWK_STRUCTS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

if (TARGET ${STRUCTS})
    message("${STRUCTS} is already defined")
else ()
    file(GLOB_RECURSE FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
    file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

    ADD_LIBRARY(${STRUCTS} STATIC ${FILES} ${HEADERS})
endif ()
