project(pluginexample)
cmake_minimum_required(VERSION 2.8...3.18)
find_package(PkgConfig REQUIRED)
pkg_search_module(PYTHON REQUIRED python3)

aux_source_directory(src SOURCES)

include_directories(${PYTHON_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
link_libraries(${PYTHON_LDFLAGS} version)

add_library(${PROJECT_NAME} SHARED EXCLUDE_FROM_ALL ${SOURCES})
add_dependencies(${PROJECT_NAME} version)
set_target_properties(${PROJECT_NAME} PROPERTIES C_VISIBILITY_PRESET hidden)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
