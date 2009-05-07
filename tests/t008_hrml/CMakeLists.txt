# Just change these variables for your own test case
set(tc_TC_NAME "T008_hrml") 
set(tc_SRC hrml-test.c ${CMAKE_SOURCE_DIR}/src/parsers/hrml.c)
set(tc_TGT t008_hrml)
set(tc_LIBS)
set(tc_INCDIRS ${CMAKE_SOURCE_DIR}/src/parsers)

include_directories(${tc_INCDIRS})

add_executable(${tc_TGT} ${tc_SRC})
target_link_libraries(${tc_TGT} ${tc_LIBS})

add_test(${tc_TC_NAME} ${tc_TGT}
         ${CMAKE_CURRENT_SOURCE_DIR}/valid.hrml)
