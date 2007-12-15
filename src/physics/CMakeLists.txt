set(ph_SRC dynamics.c)
set(ph_FILES dynamics.h CMakeLists.txt)

add_cmake_list()

add_library(ph STATIC ${ph_SRC} ${ph_FILES})
