cmake_minimum_required(VERSION 2.8)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)


include_directories(${GLIB_INCLUDE_DIRS})
link_directories(${GLIB_LINK_DIRS})

project(gds-render)
add_executable(${PROJECT_NAME} "main.c" "gdsparse.c")

target_link_libraries(${PROJECT_NAME} ${GLIB_LDFLAGS})
