project(libversion)

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

add_library(version STATIC "version.c")
execute_process(COMMAND bash ./generate-version-string.sh
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}		
		OUTPUT_VARIABLE GIT_VER)
execute_process(COMMAND bash ./generate-git-commit-string.sh
	        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_COMMIT)
message("${BoldGreen}Commit: ${GIT_COMMIT}")
message("Version: ${GIT_VER}${ColorReset}")

IF(GIT_VER MATCHES "-dirty")
        message("${BoldRed}Build is dirty! Commit your changes before releasing this version!${ColorReset}")
ENDIF(GIT_VER MATCHES "-dirty")
target_compile_definitions(version PRIVATE PROJECT_GIT_VERSION=${GIT_VER} PROJECT_GIT_COMMIT=${GIT_COMMIT})
