cmake_minimum_required(VERSION 4.0)

set(CMAKE_CXX_COMPILER "x86_64-linux-musl-g++")

project(
  Pace_Challenge_2026
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(pace utils.cpp node.cpp tree.cpp main.cpp ilp.cpp)

target_link_directories(pace PRIVATE ~/.musl/zlib/)
target_link_directories(pace PRIVATE ~/.musl/highs/lib/)

target_link_options(pace PRIVATE -static -march=haswell)
target_link_libraries(pace PRIVATE highs z)

target_compile_features(pace PUBLIC cxx_std_20)
