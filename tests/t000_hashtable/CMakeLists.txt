
set(tc_TC_NAME "T000_Hashtables")
set(tc_SRC hash-test.c
           ${CMAKE_SOURCE_DIR}/src/libgencds/hashtable.c
           ${CMAKE_SOURCE_DIR}/src/libgencds/list.c
           )
set(tc_TGT t000_hashtest)
set(tc_LIBS)
set(tc_INCDIRS)

include_directories(${tc_INCDIRS})

add_executable(${tc_TGT} ${tc_SRC})
target_link_libraries(${tc_TGT} ${tc_LIBS})

add_test(${tc_TC_NAME} ${tc_TGT})
