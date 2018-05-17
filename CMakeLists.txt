cmake_minimum_required(VERSION 2.8)

aux_source_directory("layer-widget" SUB_SOURCES)
set(SOURCE "main.c" "gdsparse.c" "layer-selector.c")


set(SOURCE
  ${SOURCE}
  ${SUB_SOURCES}
)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GLIB_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS} "layer-widget")
link_directories(${GLIB_LINK_DIRS} ${GTK3_LINK_DIRS})
configure_file(glade/main.glade glade/main.glade COPYONLY)
configure_file(glade/layer-widget.glade glade/layer-widget.glade COPYONLY)

project(gds-render)
add_executable(${PROJECT_NAME} ${SOURCE})

target_link_libraries(${PROJECT_NAME} ${GLIB_LDFLAGS} ${GTK3_LDFLAGS} m)
