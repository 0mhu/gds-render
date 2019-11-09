project(libversion)
add_library(version STATIC "version.c")
execute_process(COMMAND bash ./generate-version-string.sh
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}		
		OUTPUT_VARIABLE GIT_VER)
execute_process(COMMAND bash ./generate-git-commit-string.sh
	        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_COMMIT)
message("Commit: ${GIT_COMMIT}")
message("Version: ${GIT_VER}")
target_compile_definitions(version PRIVATE PROJECT_GIT_VERSION=${GIT_VER} PROJECT_GIT_COMMIT=${GIT_COMMIT})
