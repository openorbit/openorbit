set(ph_SRC orbit.c)
set(ph_FILES orbit.h CMakeLists.txt)

add_cmake_list()

add_library(ph STATIC ${ph_SRC} ${ph_FILES})
