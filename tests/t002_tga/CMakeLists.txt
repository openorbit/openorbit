
set(tc_TC_NAME "T002_Tga")
set(tc_SRC tga-test.c ${CMAKE_SOURCE_DIR}/src/parsers/tga.c)
set(tc_TGT t002_tgatest)
set(tc_LIBS)
set(tc_INCDIRS ${CMAKE_SOURCE_DIR}/src/parsers)

include_directories(${tc_INCDIRS})

add_executable(${tc_TGT} ${tc_SRC})
target_link_libraries(${tc_TGT} ${tc_LIBS})

add_test(${tc_TC_NAME} ${tc_TGT}
	${CMAKE_CURRENT_SOURCE_DIR}/rle.tga
	${CMAKE_CURRENT_SOURCE_DIR}/uncomp.tga)
