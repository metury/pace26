cmake_minimum_required(VERSION 4.0)

set(CMAKE_CXX_COMPILER "x86_64-linux-musl-g++")

project(
  Pace_Challenge_2026
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(pace utils.cpp node.cpp tree.cpp main.cpp ilp.cpp)

target_compile_features(pace PUBLIC cxx_std_20)

target_compile_options(pace PUBLIC -O3)

target_include_directories(pace PRIVATE
    $ENV{HOME}/.musl/include
    $ENV{HOME}/.musl/highs
)

target_link_directories(pace PRIVATE
    $ENV{HOME}/.musl/lib
    $ENV{HOME}/.musl/zlib
    $ENV{HOME}/.musl/highs/lib
)

target_link_options(pace PRIVATE -static -march=haswell)

target_link_libraries(pace PRIVATE
    -Wl,--start-group
    scip
    soplex
    mpfr
    gmp
    z
    zimpl
    highs
    -Wl,--end-group
)
