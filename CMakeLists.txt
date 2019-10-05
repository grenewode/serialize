project(serialize)
cmake_minimum_required(VERSION 3.1)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/third-party/json")


add_subdirectory(src)
add_subdirectory(example)