set( scripting_src scripting.c )
set( scripting_FILES scripting.h CMakeLists.txt)


include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_cmake_list()


add_library( scripting STATIC ${scripting_src}
                              ${scripting_FILES})
target_link_libraries( scripting ${PYTHON_LIBRARIES})
