
set(tc_TC_NAME "T007_Object_Manager2")
set(tc_SRC om2-test.c
           ${CMAKE_SOURCE_DIR}/src/libgencds/hashtable.c
           ${CMAKE_SOURCE_DIR}/src/libgencds/list.c
           ${CMAKE_SOURCE_DIR}/src/libgencds/object-manager2.c
)
set(tc_TGT t007_om_test)
set(tc_LIBS)


add_executable(${tc_TGT} ${tc_SRC})
target_link_libraries(${tc_TGT} ${tc_LIBS})

add_test(${tc_TC_NAME} ${tc_TGT})
