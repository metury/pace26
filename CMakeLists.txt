cmake_minimum_required(VERSION 3.31)

# In Order to compile it for optil.
# set(CMAKE_CXX_COMPILER "x86_64-linux-musl-g++")

project(
  Pace_Challenge_2026
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_library(SCIP_LIBRARY NAMES scip REQUIRED)
find_path(SCIP_INCLUDE_DIR NAMES scip/scip.h REQUIRED)

add_executable(pace main.cpp utils.cpp node.cpp tree.cpp ilp.cpp)

target_compile_features(pace PUBLIC cxx_std_20)
target_compile_options(pace PUBLIC -O3)

target_link_libraries(pace PUBLIC ${SCIP_LIBRARY})
target_include_directories(pace PUBLIC ${SCIP_INCLUDE_DIR})

# Also for compiling by hand for optil.
# target_include_directories(pace PRIVATE
#     $ENV{HOME}/.musl/include
# )
#
# target_link_directories(pace PRIVATE
#     $ENV{HOME}/.musl/lib
#     $ENV{HOME}/.musl/zlib
# )
#
# target_link_options(pace PRIVATE -static -march=haswell)
#
# target_link_libraries(pace PRIVATE
#     -Wl,--start-group
#     scip
#     soplex
#     mpfr
#     gmp
#     z
#     zimpl
#     -Wl,--end-group
# )
