add_custom_target(translations
		COMMAND  ./generate-mo.sh "${PROJECT_BINARY_DIR}/translations"
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Generating translation locales")
