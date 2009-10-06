# Builds library of file readers, the tga loader loads binary data and is
# written in C. In order to keep stuff clean, place all file loaders here (this
# does not include wrappers for loading abstract data, only the raw file loaders
# them self).

set( lex_files ) # set to the base name of any lexfiles that shall be built here
set( lex_c_files ) # make sure that this is empty
set( parser_src tga.c pngfile.c jpg.c image.c hrml.c)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

foreach (file ${lex_files})
	list(APPEND lex_c_files ${CMAKE_CURRENT_BINARY_DIR}/lex.${file}.c)
	list(APPEND lex_l_files ${file}.l)
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lex.${file}.c
		COMMAND ${FLEX}
			-o${CMAKE_CURRENT_BINARY_DIR}/lex.${file}.c
			-P${file}
			${CMAKE_CURRENT_SOURCE_DIR}/${file}.l
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file}.l
	)
endforeach(file)

if(CMAKE_GENERATOR MATCHES "Xcode")
    source_group("CMake" FILES "CMakeLists.txt")
    source_group("Lex" FILES ${lex_l_files})
#    source_group("Yacc" FILES ${yacc_y_files})
    liST(APPEND parser_src "CMakeLists.txt")
#    list(APPEND parser_src ${lex_l_files})
    set_source_files_properties( CMakeLists.txt ${lex_l_files}
                                 PROPERTIES HEADER_FILE_ONLY yes )
endif(CMAKE_GENERATOR MATCHES "Xcode")

add_library( parsers STATIC ${parser_src} ${lex_c_files} )
