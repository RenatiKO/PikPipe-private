cmake_minimum_required(VERSION 3.9)


find_package(Eigen3 3.3 REQUIRED NO_MODULE)
add_definitions (-Wall -pedantic)
find_package(CGAL)
find_package(  yaml-cpp REQUIRED )

set (PROJECT
        PipeAlgo)

set (HEADERS
        ./include/PipeAlgo.h)

set (SOURCES
        ./src/PipeAlgo.cpp)

set (TESTS_SOURCES
        ./tests/tests.cpp)

project (${PROJECT})
source_group ("Header Files" FILES ${HEADERS})
source_group ("Source Files" FILES ${SOURCES})

add_library (${PROJECT} ${HEADERS} ${SOURCES})
#include_directories(../../..yaml-cpp/build)
target_link_libraries(${PROJECT} yaml-cpp)


if (TESTS_SOURCES)
    set (Boost_USE_STATIC_LIBS ON)
    set (Boost_USE_MULTITHREADED ON)
    find_package (Boost
            COMPONENTS
            unit_test_framework
            REQUIRED)
    include_directories(${Boost_INCLUDE_DIRS})
    set (TEST test_${PROJECT})
    add_executable (${TEST} ${TESTS_SOURCES})
    target_link_libraries (${TEST} ${PROJECT} ${Boost_LIBRARIES})
    add_test (${TEST} ${TEST})
endif ()
