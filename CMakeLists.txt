cmake_minimum_required(VERSION 3.9)

project(Pipik VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CGAL_DIR /usr/local/lib/cmake/CGAL)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(CGAL)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
#find_package( OpenCV REQUIRED )
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(  yaml-cpp REQUIRED )
# Find OSQP library and headers.
find_package(OSQP QUIET)
#include(CGAL_OSQP_support)
#if(TARGET CGAL::OSQP_support)
#  message(STATUS "Found OSQP")

#  set(osqp_targets
#  regularize_15_segments
#  regularize_100_segments_angles
#  regularize_100_segments_offsets
#  regularize_simple)

#  foreach(osqp_target ${osqp_targets})
##    create_single_source_cgal_program("${osqp_target}.cpp")
##    if(TARGET ${osqp_target})
##      target_link_libraries(${osqp_target} PUBLIC CGAL::OSQP_support)
##    endif()
#  endforeach()

#  # Use Eigen.
#  find_package(Eigen3 3.1.0 QUIET) # (3.1.0 or greater)
#  include(CGAL_Eigen3_support)
#  if(TARGET CGAL::Eigen3_support)
#    message(STATUS "Found Eigen")

##    create_single_source_cgal_program("regularize_real_data_2.cpp")
##    target_link_libraries(regularize_real_data_2 PUBLIC CGAL::Eigen3_support CGAL::OSQP_support)
#  else()
#    message(STATUS "NOTICE: Eigen was not found. Eigen examples won't be available.")
#  endif()
#else()
#  message(STATUS "NOTICE: OSQP was not found. OSQP examples won't be available.")
#endif()

#if(${osqp_FOUND})
#    message(STATUS "Found osqp")
#endif

set (AUTOGEN_BUILD_DIR
    ./ui
    )

set(INC_DIRS
    ./src
    ./include
    ./ui
)



#include_directories( ${OpenCV_INCLUDE_DIRS} )
#include_directories(../yaml-cpp/build)
#link_directories(/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/lib)

INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
include_directories(${INC_DIRS})

link_directories(./build/libdxfrw)
#link_directories(../libdxfrw/build)

#add_subdirectory(BackServer)

set(PROJECT_SOURCES
        ./src/main.cpp
        ./src/mainwindow.cpp
        ./include/mainwindow.h
        ./ui/mainwindow.ui

        include/dxf_parser.h
        src/dxf_parser.cpp
        include/qdxfviewer.h
        src/qdxfviewer.cpp

#        ./src/dxf/file_element.cpp
#        ./src/dxf/header.cpp
#        ./src/dxf/layer.cpp
#        ./src/dxf/table.cpp
#        ./src/dxf/entity.cpp
#        ./src/dxf/file_image.cpp
#        ./src/dxf/section.cpp

#        ./src/dxf/entities
#        ./src/dxf/input
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Pipik
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Pipik APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Pipik SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Pipik
            ${PROJECT_SOURCES}
        )
    endif()
endif()
add_subdirectory (./src/PipeAlgo)
add_subdirectory(./libdxfrw)
add_subdirectory(./BackServer)
target_link_libraries(Pipik
        PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
        PipeAlgo
#        libyaml-cpp.a
#        libiconv.tbd
        dxfrw
#        ${OpenCV_LIBS}
        CGAL::CGAL
        Eigen3::Eigen)

target_include_directories(Pipik PRIVATE ${INC_DIRS})

set_target_properties(Pipik PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Pipik)
endif()
