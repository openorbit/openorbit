
# Add any sources of tested units
set(tc_SRC test-case.c)

add_executable(t666_testname ${tc_SRC})

add_test("T666_Testname" t666_testname)
