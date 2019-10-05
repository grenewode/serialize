add_library(serialize INTERFACE)
target_sources(serialize INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/serialize.hpp")
target_include_directories(serialize INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")