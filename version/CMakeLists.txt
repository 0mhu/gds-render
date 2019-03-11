add_library(version STATIC "version.c")
execute_process(COMMAND bash ./generate-version-string.sh
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}		
		OUTPUT_VARIABLE GIT_VER)
target_compile_definitions(version PUBLIC PROJECT_GIT_VERSION=${GIT_VER})
