cmake_minimum_required(VERSION 2.8)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(CAIRO REQUIRED cairo)

project(gds-render)

add_subdirectory(glade)

include_directories(${GLIB_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS} ${CAIRO_INCLUDE_DIRS})
link_directories(${GLIB_LINK_DIRS} ${GTK3_LINK_DIRS} ${CAIRO_LINK_DIRS})
add_definitions(${GLIB2_CFLAGS_OTHER})


aux_source_directory("layer-widget" LAYER_SOURCES)
aux_source_directory("tree-renderer" RENDERER_SOURCES)
aux_source_directory("gds-parser" PARSER_SOURCES)
aux_source_directory("latex-output" LATEX_SOURCES)
set(SOURCE "main.c" "layer-selector.c")


set(SOURCE
  ${SOURCE}
  ${LAYER_SOURCES}
  ${RENDERER_SOURCES}
  ${PARSER_SOURCES}
  ${LATEX_SOURCES}
)

add_compile_options(-Wall)

add_executable(${PROJECT_NAME} ${SOURCE} ${CMAKE_CURRENT_BINARY_DIR}/glade/resources.c)
add_dependencies(${PROJECT_NAME} glib-resources)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/glade/resources.c PROPERTIES GENERATED 1)
target_link_libraries(${PROJECT_NAME} ${GLIB_LDFLAGS} ${GTK3_LDFLAGS} ${CAIRO_LDFLAGS} m)

