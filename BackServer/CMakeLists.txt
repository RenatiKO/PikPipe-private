#project(BackPP)
#link_libraries(${PROJECT_NAME})

find_package(Drogon REQUIRED)
add_executable(BackPP src/main.cpp
    ../include/dxf_parser.h
    ../src/dxf_parser.cpp
    ../include/PipeAlgo/PipeAlgo.h
    ../src/PipeAlgo/PipeAlgo.cpp)

# Add warnings for our example targets--some warnings (such as -Wunused-parameter) only appear
# when the templated functions are instantiated at their point of use.

#target_compile_options(BackPP PRIVATE -Wall -Wextra -Werror)
target_link_libraries(BackPP PRIVATE Drogon::Drogon
    libyaml-cpp.a
    libdxfrw.a
    ${OpenCV_LIBS}
    CGAL::CGAL
    Eigen3::Eigen)
target_include_directories(BackPP PRIVATE ${INC_DIRS})


set_property(TARGET BackPP
             PROPERTY CXX_STANDARD ${DROGON_CXX_STANDARD})
set_property(TARGET BackPP PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET BackPP PROPERTY CXX_EXTENSIONS OFF)
