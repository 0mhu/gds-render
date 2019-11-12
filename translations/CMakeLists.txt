add_custom_target(translations
	                DEPENDS
			        "${PROJECT_BINARY_DIR}/translations/output/"
		)

add_custom_command(DEPENDS
	                 "${CMAKE_CURRENT_SOURCE_DIR}/pot/po/*"
		   OUTPUT
		         "${PROJECT_BINARY_DIR}/translations/output/"
		   COMMAND
		       COMMAND  ./generate-mo.sh "${PROJECT_BINARY_DIR}/translations/output"
		   WORKING_DIRECTORY
		        ${CMAKE_CURRENT_SOURCE_DIR}
		   COMMENT
		         "Generating translation locales"
		   )
