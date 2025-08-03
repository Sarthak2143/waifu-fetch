cmake_minimum_required(VERSION 3.10)
project(waifu-fetch)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finput-charset=UTF-8 -fexec-charset=UTF-8")

find_package(OpenCV REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(cpr REQUIRED)

add_library(ImageRenderer STATIC
  ImageRenderer.cpp
)

target_include_directories(ImageRenderer PUBLIC
  ${OPENCV_INCLUDE_DIRS}
)

target_link_libraries(ImageRenderer PRIVATE
  cpr::cpr
  opencv_core
  opencv_imgcodecs
  opencv_imgproc
)

# add_subdirectory(cpr)

add_executable(waifu-fetch
  main.cpp
)

target_include_directories(waifu-fetch PRIVATE
  ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(waifu-fetch PRIVATE
  ImageRenderer
  cpr::cpr
  opencv_core
  opencv_imgcodecs
  opencv_imgproc
)
