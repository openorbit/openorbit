
set(tc_TC_NAME "T001_Heap")
set(tc_SRC heap-test.c
           ${CMAKE_SOURCE_DIR}/src/libgencds/heap.c)
set(tc_TGT t001_heaptest)
set(tc_LIBS)
set(tc_INCDIRS)



include_directories(${tc_INCDIRS})
add_executable(${tc_TGT} ${tc_SRC})
target_link_libraries(${tc_TGT} ${tc_LIBS})

add_test(${tc_TC_NAME} ${tc_TGT})
