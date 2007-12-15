
set(tc_TC_NAME "T003_Math")
set(tc_SRC math-test.c)
set(tc_TGT t003_math_test)
set(tc_LIBS math m)


add_executable(${tc_TGT} ${tc_SRC})
target_link_libraries(${tc_TGT} ${tc_LIBS})

add_test(${tc_TC_NAME} ${tc_TGT})

