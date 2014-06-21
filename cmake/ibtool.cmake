find_program(IBTOOL NAMES ibtool)

if (NOT IBTOOL)
  message(ERROR "could not find ibtool")
endif (NOT IBTOOL)

macro(ibtool_compile file)
  get_filename_component(basefile ${file} NAME_WE)
  get_filename_component(dir ${file} DIRECTORY)
  
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${dir}/${basefile}.nib
    COMMAND mkdir -p "${CMAKE_CURRENT_BINARY_DIR}/${dir}"
    COMMAND ${IBTOOL} --compile ${CMAKE_CURRENT_BINARY_DIR}/${dir}/${basefile}.nib
                      ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endmacro(ibtool_compile file)
