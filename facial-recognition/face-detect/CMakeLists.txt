cmake_minimum_required(VERSION 2.8)
add_compile_options(-std=c++11)
project(face_detect)
find_package(OpenCV REQUIRED)

set(face_detect_source_files face_detect.cpp face_detect.hpp)
add_library(face_detect_lib STATIC ${face_detect_source_files})
target_link_libraries(face_detect_lib ${OpenCV_LIBS})

add_executable(face_detect run_face_detect.cpp face_detect.cpp)
target_link_libraries(face_detect ${OpenCV_LIBS})
