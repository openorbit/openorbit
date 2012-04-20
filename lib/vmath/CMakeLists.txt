include_directories(${CMAKE_SOURCE_DIR}/include)

set(vmath_SRC vmath-matvec.c vmath-quaternions.c)

add_library(vmath STATIC ${vmath_SRC})
