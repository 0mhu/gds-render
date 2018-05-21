cmake_minimum_required(VERSION 2.8)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

project(gds-render)

add_subdirectory(glade)

include_directories(${GLIB_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS} "layer-widget")
link_directories(${GLIB_LINK_DIRS} ${GTK3_LINK_DIRS})
add_definitions(${GLIB2_CFLAGS_OTHER})


aux_source_directory("layer-widget" SUB_SOURCES)
aux_source_directory("tree-renderer" RENDERER_SOURCES)
set(SOURCE "main.c" "gdsparse.c" "layer-selector.c")


set(SOURCE
  ${SOURCE}
  ${SUB_SOURCES}
  ${RENDERER_SOURCES}
)


add_executable(${PROJECT_NAME} ${SOURCE} ${CMAKE_CURRENT_BINARY_DIR}/glade/resources.c)
add_dependencies(${PROJECT_NAME} glib-resources)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/glade/resources.c PROPERTIES GENERATED 1)
target_link_libraries(${PROJECT_NAME} ${GLIB_LDFLAGS} ${GTK3_LDFLAGS} m)

