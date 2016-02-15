cmake_minimum_required(VERSION 2.6)
project(LIBSEEK)
#find_package(libusb REQUIRED)
find_path(LIBUSB_INCLUDE_PATH libusb.h /usr/include/libusb-1.0/)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -fpic --std=c++11")

add_library(seek STATIC seek.cpp)
add_executable(seek-test seek-test.cpp)
add_executable(seek-test-calib seek-test-calib.cpp)

target_include_directories(seek PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${LIBUSB_INCLUDE_PATH})

target_link_libraries(seek LINK_PUBLIC usb-1.0)
target_link_libraries(seek-test LINK_PUBLIC seek)
target_link_libraries(seek-test-calib LINK_PUBLIC seek)