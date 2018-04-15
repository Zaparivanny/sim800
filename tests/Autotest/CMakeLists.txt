CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(AutoTestSim800
        LANGUAGES C CXX)


add_definitions(-DGTEST_LANGUAGE_CXX11)
add_definitions(-D___NOP)

find_package(Threads REQUIRED)

if ($ENV{GOOGLETEST_DIR})
    SET(GOOGLETEST_DIR $ENV{GOOGLETEST_DIR})
else ()
    message(WARNING "Using googletest src dir specified at Qt Creator wizard")
    SET(GOOGLETEST_DIR ${PROJECT_SOURCE_DIR}/../../external/googletest)
endif ()
if (EXISTS ${GOOGLETEST_DIR})
    SET(GTestSrc ${GOOGLETEST_DIR}/googletest)
    SET(GMockSrc ${GOOGLETEST_DIR}/googlemock)
else ()
    message( FATAL_ERROR "No googletest src dir found - set GOOGLETEST_DIR to enable!")
endif ()

set(SIM_DIR ${CMAKE_CURRENT_SOURCE_DIR}../../../src)
include_directories(${GTestSrc} ${GTestSrc}/include ${GMockSrc} ${GMockSrc}/include)

message (${SIM_DIR})

include_directories(${SIM_DIR})
add_library(SIM_LIB ${SIM_DIR}/sim800.c ${SIM_DIR}/sim800.h)


add_executable(${PROJECT_NAME} main.cpp tst_autotestsim800.h
               ${GTestSrc}/src/gtest-all.cc
               ${GMockSrc}/src/gmock-all.cc)
add_test(${PROJECT_NAME} COMMAND ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads SIM_LIB)

