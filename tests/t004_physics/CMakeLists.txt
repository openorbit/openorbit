# Just change these variables for your own test case
set(tc_TC_NAME "T004_physics") 
set(tc_SRC test-case.c
    ../../src/physics/object.c
    ../../src/physics/world.c
    ../../src/physics/octtree.c
    ../../src/physics/collision.c
    ../../src/physics/celestial-object.c
    ../../src/physics/mass.c
    ../../src/common/palloc.c
    ../../src/libgencds/array.c
    ../../src/libgencds/avl-tree.c
    ../../src/log.c
)

set(tc_TGT t004_physics)
set(tc_LIBS vmath celmek m)

include_directories(${tc_INCDIRS})

add_executable(${tc_TGT} ${tc_SRC})
target_link_libraries(${tc_TGT} ${tc_LIBS})

add_test(${tc_TC_NAME} ${tc_TGT})

