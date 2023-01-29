# CMakeList.txt : CMake project for BlazeTracer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (BlazeTracer "main.cpp"  "image.h" "util.h" "vec3.h" "color.h" "ray.h" "hittable.h" "sphere.h" "hittable_list.h" "rtweekend.h" "camera.h" "material.h" "moving_sphere.h" "aabb.h" "bvh.h" "texture.h" "perlin.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET BlazeTracer PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
