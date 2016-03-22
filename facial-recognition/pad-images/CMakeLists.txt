cmake_minimum_required(VERSION 2.8)
add_compile_options(-std=c++11)
project(pad_images)
find_package(OpenCV REQUIRED )

set(pad_images_source_files pad_images.cpp pad_images.hpp)
add_library(pad_images_lib STATIC ${pad_images_source_files})
target_link_libraries(pad_images_lib ${OpenCV_LIBS})

add_executable(pad_images run_pad_images.cpp pad_images.cpp)
target_link_libraries(pad_images ${OpenCV_LIBS})
