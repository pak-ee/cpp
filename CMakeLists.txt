cmake_minimum_required(VERSION 3.15)

project(minimal_arkanoid LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(mini
  src/main.cpp
  src/Entities.hpp
  src/Bonus.hpp
  src/Physics.hpp
)

target_compile_features(mini PRIVATE cxx_std_20)
target_link_libraries(mini PRIVATE sfml-graphics sfml-window sfml-system)

if(UNIX AND NOT APPLE)
  target_compile_options(mini PRIVATE -Wall -Wextra -Wpedantic)
endif()


