cmake_minimum_required(VERSION 3.5)
project(waifu-fetch)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)

add_subdirectory(cpr)

add_executable(waifu-fetch main.cpp)

target_link_libraries(waifu-fetch PRIVATE
  cpr::cpr
  opencv_core
  opencv_imgproc
  opencv_imgcodecs
  opencv_highgui
)

target_include_directories(
  waifu-fetch PRIVATE
  ${OpenCV_INCLUDE_DIRS}
)
