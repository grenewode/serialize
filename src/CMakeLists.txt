add_library(serialize serialize.cpp)
target_sources(serialize PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/serialize.hpp")
target_include_directories(serialize PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(serialize PUBLIC cxx_std_20)

target_link_libraries(serialize PUBLIC nlohmann_json::nlohmann_json)
