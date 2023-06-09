cmake_minimum_required(VERSION 3.9)
project(BackPP)
#link_libraries(${PROJECT_NAME})

find_package(CGAL)
find_package(Drogon REQUIRED)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

set(INC_DIRS
    ./src
    ../include
    ../src
    ../src/PipeAlgo/include
)

#include_directories( ${OpenCV_INCLUDE_DIRS} )
#include_directories(../yaml-cpp/build)
#link_directories(/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/lib)

INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
include_directories(${INC_DIRS})

add_executable(BackPP src/main.cpp
    ../include/dxf_parser.h
    ../src/dxf_parser.cpp
    )
#    ../src/PipeAlgo/include/PipeAlgo.h
#[[    ../src/PipeAlgo/src/PipeAlgo.cpp]]

# Add warnings for our example targets--some warnings (such as -Wunused-parameter) only appear
# when the templated functions are instantiated at their point of use.

#target_compile_options(BackPP PRIVATE -Wall -Wextra -Werror)
target_link_libraries(BackPP PRIVATE Drogon::Drogon
#    libyaml-cpp.a
    PipeAlgo
    libdxfrw.a
    yaml-cpp
    ${OpenCV_LIBS}
    CGAL::CGAL
    Eigen3::Eigen)
target_include_directories(BackPP PRIVATE ${INC_DIRS})


set_property(TARGET BackPP
             PROPERTY CXX_STANDARD ${DROGON_CXX_STANDARD})
set_property(TARGET BackPP PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET BackPP PROPERTY CXX_EXTENSIONS OFF)

get_property(ALL_BUILDSYSTEM_TARGETS DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
add_custom_target(Backend DEPENDS ${ALL_BUILDSYSTEM_TARGETS})
