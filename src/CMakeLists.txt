add_library(serialize INTERFACE)
target_sources(serialize INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/serialize.hpp")
target_include_directories(serialize INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(serialize INTERFACE cxx_std_17)

target_link_libraries(serialize INTERFACE nlohmann_json::nlohmann_json)