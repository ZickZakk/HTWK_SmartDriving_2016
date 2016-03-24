cmake_minimum_required(VERSION 2.6)

set(TEST_DIR ${PROJECT_SOURCE_DIR}/test)

PROJECT(Test)

#
# Misc
#

add_subdirectory(./../src/htwk_logger htwk_logger)
include_directories(${HTWK_LOGGER_INCLUDE_DIR})

add_subdirectory(./../src/htwk_utils htwk_utils)
include_directories(${HTWK_UTILS_INCLUDE_DIR})

add_subdirectory(./../src/htwk_carconfig htwk_carconfig)
include_directories(${HTWK_CARCONFIG_INCLUDE_DIR})

add_subdirectory(./../src/htwk_vision htwk_vision)
include_directories(${HTWK_VISION_INCLUDE_DIR})

include_directories(${OpenCV_INCLUDE_DIR})

#
# Test exe
#

file(GLOB_RECURSE TEST_FILES ${TEST_DIR}/*.cpp)
file(GLOB_RECURSE TEST_HEADERS ${TEST_DIR}/*.h)
add_executable(run_tests ${TEST_FILES} ${TEST_HEADERS})

SET_TARGET_PROPERTIES(run_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test")

ADD_CUSTOM_TARGET(tests "${CMAKE_BINARY_DIR}/test/run_tests" DEPENDS run_tests COMMENT "Running CPPUNIT tests...")

install_programs(/test FILES run_tests)

file(GLOB_RECURSE RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources/*)
install(FILES ${RESOURCES} DESTINATION ${CMAKE_INSTALL_PREFIX}/test/resources)

target_link_libraries(run_tests htwk_logger htwk_utils htwk_vision htwk_carconfig)
target_link_libraries(run_tests cppunit)
target_link_libraries(run_tests ${OpenCV_LIBS} ${ADTF_LIBS})
