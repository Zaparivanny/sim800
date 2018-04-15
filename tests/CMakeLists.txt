cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(gtest-demo)

enable_language(C)
enable_language(CXX)

set(CMAKE_C_FLAGS "-std=c99") 

add_definitions(-D___NOP)

if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
    set(CMAKE_CXX_FLAGS         "-Wall -Wno-unknown-pragmas -Wno-sign-compare -Woverloaded-virtual -Wwrite-strings -Wno-unused")
    set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g3")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/../src
    )

add_library(
    example
    ${PROJECT_SOURCE_DIR}/../src/sim800.c
    )

SET(GOOGLETEST_DIR "external/googletest")
SET(GTestSrc ${GOOGLETEST_DIR}/googletest)
SET(GMockSrc ${GOOGLETEST_DIR}/googlemock)

include_directories(
    ${PROJECT_SOURCE_DIR}/../${GTestSrc}
    ${PROJECT_SOURCE_DIR}/../${GTestSrc}/include
    ${PROJECT_SOURCE_DIR}/../${GMockSrc}
    ${PROJECT_SOURCE_DIR}/../${GMockSrc}/include
    )

set(GOOGLETEST_SOURCES
    ${PROJECT_SOURCE_DIR}/../${GTestSrc}/src/gtest-all.cc
    ${PROJECT_SOURCE_DIR}/../${GMockSrc}/src/gmock-all.cc
    ${PROJECT_SOURCE_DIR}/../${GTestSrc}/src/gtest_main.cc
    )

foreach(_source ${GOOGLETEST_SOURCES})
    set_source_files_properties(${_source} PROPERTIES GENERATED 1)
endforeach()

add_library(googletest ${GOOGLETEST_SOURCES})

add_executable(
    unit_tests
    ${PROJECT_SOURCE_DIR}/Autotest/main.cpp
    ${PROJECT_SOURCE_DIR}/Autotest/tst_autotestsim800.h
    )

add_dependencies(unit_tests googletest)

target_link_libraries(
    unit_tests
    googletest
    example
    pthread
    )

include(CTest)
enable_testing()

add_test(unit ${PROJECT_BINARY_DIR}/unit_tests)