
include_directories(${CMAKE_SOURCE_DIR}/src)
set(math_SRC vmath-matvec.c vmath-quaternions.c)
set(math_FILES vmath-constants.h vmath-matvec.h vmath-quaternions.h
               vmath-types.h CMakeLists.txt)

add_cmake_list()

add_library(math STATIC ${math_SRC} ${math_FILES})
