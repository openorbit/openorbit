
include_directories(${CMAKE_SOURCE_DIR}/src)
set(math_SRC linalg.c quaternions.c)
set(math_FILES constants.h linalg.h quaternions.h types.h CMakeLists.txt)

add_cmake_list()

add_library(math STATIC ${math_SRC} ${math_FILES})
