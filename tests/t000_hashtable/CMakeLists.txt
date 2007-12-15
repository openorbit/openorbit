
set(tc_TC_NAME "T000_Hashtables")
set(tc_SRC hash-test.c)
set(tc_TGT t000_hashtest)
set(tc_LIBS gencds)
set(tc_INCDIRS)

include_directories(${tc_INCDIRS})

add_executable(${tc_TGT} ${tc_SRC})
target_link_libraries(${tc_TGT} ${tc_LIBS})

add_test(${tc_TC_NAME} ${tc_TGT})
