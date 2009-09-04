set(ph_SRC orbit.c sectors.c rockets.c object.c)
set(ph_FILES orbit.h CMakeLists.txt)

add_cmake_list()

add_library(ph STATIC ${ph_SRC} ${ph_FILES})
