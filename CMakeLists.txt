project(gds-render)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr/" CACHE PATH "..." FORCE)
endif()

if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColorReset "${Esc}[m")
  set(ColorBold  "${Esc}[1m")
  set(Red         "${Esc}[31m")
  set(Green       "${Esc}[32m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Magenta     "${Esc}[35m")
  set(Cyan        "${Esc}[36m")
  set(White       "${Esc}[37m")
  set(BoldRed     "${Esc}[1;31m")
  set(BoldGreen   "${Esc}[1;32m")
  set(BoldYellow  "${Esc}[1;33m")
  set(BoldBlue    "${Esc}[1;34m")
  set(BoldMagenta "${Esc}[1;35m")
  set(BoldCyan    "${Esc}[1;36m")
  set(BoldWhite   "${Esc}[1;37m")
endif()

cmake_minimum_required(VERSION 2.8)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(CAIRO REQUIRED cairo)

include_directories(${GLIB_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS} ${CAIRO_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_subdirectory(plugins)

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message("${Yellow}Debug mode for translations used!${ColorReset}")
	add_definitions(-DGETTEXT_PACKAGE=\"gds-render\" -DLOCALEDATADIR=\"${CMAKE_CURRENT_BINARY_DIR}/translations/output\")
	message("${BoldMagenta}${CMAKE_CURRENT_BINARY_DIR}/translations/output used as data dir${ColorReset}")
else(CMAKE_BUILD_TYPE STREQUAL "Debug")
	message("Global locale directory used. Make sure files in /usr/share/locale are available")
	add_definitions(-DGETTEXT_PACKAGE=\"gds-render\" -DLOCALEDATADIR=\"/usr/share\")
ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")



aux_source_directory("widgets" LAYER_SOURCES)
aux_source_directory("cell-selector" CELL_SELECTOR_SOURCES)
aux_source_directory("gds-utils" GDS_SOURCES)
aux_source_directory("output-renderers" OUTPUT_RENDERER_SOURCES)
aux_source_directory("geometric" GEOMETRIC_SOURCES)
aux_source_directory("layer" LAYER_SELECTOR_SOURCES)
set(SOURCE "main.c" "command-line.c" "gds-render-gui.c")

set(SOURCE
  ${SOURCE}
  ${LAYER_SOURCES}
  ${CELL_SELECTOR_SOURCES}
  ${GDS_SOURCES}
  ${OUTPUT_RENDERER_SOURCES}
  ${GEOMETRIC_SOURCES}
  ${LAYER_SELECTOR_SOURCES}
)

set(SOURCE_GENERATED
  ${CMAKE_CURRENT_BINARY_DIR}/resources/resources.c
)

link_directories(${GLIB_LINK_DIRS} ${GTK3_LINK_DIRS} ${CAIRO_LINK_DIRS})
SET_SOURCE_FILES_PROPERTIES(${SOURCE_GENERATED} PROPERTIES GENERATED 1)

add_subdirectory(test)

add_compile_options(-Wall -Wextra -Wold-style-declaration -Wuninitialized -Wmaybe-uninitialized -Wunused-parameter)

add_subdirectory(resources)
add_subdirectory(doxygen)
add_subdirectory(translations)
add_subdirectory(version)

link_directories(${GLIB_LINK_DIRS} ${GTK3_LINK_DIRS} ${CAIRO_LINK_DIRS})
add_definitions(${GLIB2_CFLAGS_OTHER})

add_executable(${PROJECT_NAME} ${SOURCE} ${SOURCE_GENERATED})
add_dependencies(${PROJECT_NAME} glib-resources)
add_dependencies(${PROJECT_NAME} version)
add_dependencies(${PROJECT_NAME} translations)

install (TARGETS ${PROJECT_NAME} 
	RUNTIME	
		DESTINATION bin
	)
target_link_libraries(${PROJECT_NAME} ${GLIB_LDFLAGS} ${GTK3_LDFLAGS} ${CAIRO_LDFLAGS} m version ${CMAKE_DL_LIBS})

