find_package(SWIG REQUIRED)

macro(swig_wrap lang file)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file}.c ${CMAKE_CURRENT_BINARY_DIR}/${file}.py
    COMMAND mkdir -p `dirname ${CMAKE_CURRENT_BINARY_DIR}/${file}.c`
    COMMAND ${SWIG_EXECUTABLE} -${lang} -o ${CMAKE_CURRENT_BINARY_DIR}/${file}.c
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endmacro(swig_wrap lang file)
