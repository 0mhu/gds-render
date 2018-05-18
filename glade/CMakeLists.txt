add_custom_target(glib-resources DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/resources.c)
add_custom_command(DEPENDS
			 ${CMAKE_CURRENT_SOURCE_DIR}/resources.xml
		   OUTPUT
			${CMAKE_CURRENT_BINARY_DIR}/resources.c
		   COMMAND
			echo "$(pwd)"
		   COMMAND 
			glib-compile-resources --target="${CMAKE_CURRENT_BINARY_DIR}/resources.c" --sourcedir="${CMAKE_CURRENT_SOURCE_DIR}" --generate-source "${CMAKE_CURRENT_SOURCE_DIR}/resources.xml"
		   )
