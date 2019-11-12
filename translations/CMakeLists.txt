add_custom_target(translations ALL
		COMMAND  ./generate-mo.sh "${PROJECT_BINARY_DIR}/translations/output"
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Generating translation locales")
