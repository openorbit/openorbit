
set(tc_TC_NAME "T007_Object_Manager2")
set(tc_SRC om2-test.c)
set(tc_TGT t007_om_test)
set(tc_LIBS gencds)


add_executable(${tc_TGT} ${tc_SRC})
target_link_libraries(${tc_TGT} ${tc_LIBS})

add_test(${tc_TC_NAME} ${tc_TGT})
