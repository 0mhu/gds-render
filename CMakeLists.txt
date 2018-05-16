cmake_minimum_required(VERSION 2.8)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GLIB_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS})
link_directories(${GLIB_LINK_DIRS} ${GTK3_LINK_DIRS})
configure_file(glade/main.glade glade/main.glade COPYONLY)
configure_file(glade/layer-widget.glade glade/layer-widget.glade COPYONLY)

project(gds-render)
add_executable(${PROJECT_NAME} "main.c" "gdsparse.c" "layer-element.c" "layer-selector.c")

target_link_libraries(${PROJECT_NAME} ${GLIB_LDFLAGS} ${GTK3_LDFLAGS} m)
