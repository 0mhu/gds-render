project(gds-render-test)

add_custom_target(test "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}" "-r compact" "-s" DEPENDS ${PROJECT_NAME})

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

cmake_minimum_required(VERSION 2.8...3.18)
find_package(PkgConfig REQUIRED)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/catch-framework")

aux_source_directory("geometric" GEOMETRIC_TEST_SOURCES)
set(TEST_SOURCES
	${GEOMETRIC_TEST_SOURCES}
)

set(DUT_SOURCES
	"../geometric/vector-operations.c"
)

add_executable(${PROJECT_NAME} EXCLUDE_FROM_ALL "test-main.cpp" ${TEST_SOURCES} ${DUT_SOURCES})
target_link_libraries(${PROJECT_NAME} ${GLIB_LDFLAGS} ${GTK3_LDFLAGS} ${CAIRO_LDFLAGS} m version ${CMAKE_DL_LIBS})

