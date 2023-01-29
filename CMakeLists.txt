# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("BlazeTracer")

# Include sub-projects.
add_subdirectory ("BlazeTracer")

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/earthmap.jpg DESTINATION ${CMAKE_CURRENT_BINARY_DIR})