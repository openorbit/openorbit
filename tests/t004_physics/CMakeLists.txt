# Just change these variables for your own test case
set(tc_TC_NAME "T666_xyz") 
set(tc_SRC test-case.c)
set(tc_TGT t666_xyz)
set(tc_LIBS)
set(tc_INCDIRS)

include_directories(${tc_INCDIRS})

add_executable(${tc_TGT} ${tc_SRC})
target_link_libraries(${tc_TGT} ${tc_LIBS})

add_test(${tc_TC_NAME} ${tc_TGT})

