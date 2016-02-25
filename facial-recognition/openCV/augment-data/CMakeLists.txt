cmake_minimum_required(VERSION 2.8)
add_compile_options(-std=c++11)
project(augment_data)
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

add_executable(augment_data augment_data.cpp)
target_link_libraries(augment_data ${OpenCV_LIBS})
target_link_libraries(augment_data ${CMAKE_THREAD_LIBS_INIT})