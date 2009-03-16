MACRO(AUTOWRAP file)
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/aw_${file}
                     COMMAND autowrap
                             ${CMAKE_CURRENT_SOURCE_DIR}/${file}
                             -o ${CMAKE_CURRENT_BINARY_DIR}/aw_${file}
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file}
                     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
ENDMACRO(AUTOWRAP file)
