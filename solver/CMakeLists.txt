cmake_minimum_required(VERSION 4.0)

project(
  Pace_Challenge_2026
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(highs CONFIG REQUIRED)

add_executable(pace utils.cpp node.cpp tree.cpp main.cpp ilp.cpp)

target_link_libraries(pace PRIVATE highs)

target_compile_features(pace PUBLIC cxx_std_20)
