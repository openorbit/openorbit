include_directories(${CMAKE_SOURCE_DIR}/include)

set(vmath_SRC vmath-matvec.c vmath-quaternions.c)
set(vmath_FILES
    ${CMAKE_SOURCE_DIR}/include/vmath/vmath.h
    ${CMAKE_SOURCE_DIR}/include/vmath/vmath-constants.h
    ${CMAKE_SOURCE_DIR}/include/vmath/vmath-matvec.h
    ${CMAKE_SOURCE_DIR}/include/vmath/vmath-quaternions.h
    ${CMAKE_SOURCE_DIR}/include/vmath/vmath-types.h
    ${CMAKE_SOURCE_DIR}/include/vmath/vmath-fixed.h)

add_library(vmath STATIC ${vmath_SRC} ${vmath_FILES})
