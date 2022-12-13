# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.24)

project ("JSON" CXX)
set(CMAKE_CXX_STANDARD 20)

# Include sub-projects.
add_subdirectory ("JSON")
add_subdirectory ("Test")
