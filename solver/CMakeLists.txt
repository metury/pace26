cmake_minimum_required(VERSION 4.0)

set(CMAKE_CXX_COMPILER "x86_64-linux-musl-g++")

project(
  Pace_Challenge_2026
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SHARED_SOURCES utils.cpp node.cpp tree.cpp ilp.cpp)

add_executable(pace_exact main_exact.cpp ${SHARED_SOURCES})

target_compile_features(pace_exact PUBLIC cxx_std_20)

target_compile_options(pace_exact PUBLIC -O3)

target_include_directories(pace_exact PRIVATE
    $ENV{HOME}/.musl/include
)

target_link_directories(pace_exact PRIVATE
    $ENV{HOME}/.musl/lib
    $ENV{HOME}/.musl/zlib
)

target_link_options(pace_exact PRIVATE -static -march=haswell)

target_link_libraries(pace_exact PRIVATE
    -Wl,--start-group
    scip
    soplex
    mpfr
    gmp
    z
    zimpl
    -Wl,--end-group
)
