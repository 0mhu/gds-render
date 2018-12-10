find_package(Doxygen)
if (DOXYGEN_FOUND)
 	add_custom_target(doxygen
		COMMAND ./build-doxygen.sh "${PROJECT_BINARY_DIR}"
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Generating documentation with Doxygen")
		
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
