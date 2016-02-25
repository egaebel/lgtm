cmake_minimum_required(VERSION 2.8)
add_compile_options(-std=c++11)
project(full_preprocessing)
find_package(OpenCV REQUIRED)

add_executable(full_preprocessing full_preprocessing.cpp)
target_link_libraries(full_preprocessing ${OpenCV_LIBS})
target_link_libraries(full_preprocessing face_detect_lib)
target_link_libraries(full_preprocessing pad_images_lib)
