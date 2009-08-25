set( pyrex_files ooscript )
set( pyrex_c_files )
set( scripting_src scripting.c )
set( scripting_FILES scripting.h CMakeLists.txt)


include_directories(${CMAKE_CURRENT_BINARY_DIR})

foreach (file ${pyrex_files})
	list(APPEND pyrex_c_files ${CMAKE_CURRENT_BINARY_DIR}/${file}.c)
	list(APPEND pyrex_pyx_files ${file}.pyx)
  use_pyrex(${file})
endforeach(file)

add_cmake_list()


add_library( scripting STATIC ${scripting_src} ${pyrex_c_files}
                              ${pyrex_pyx_files} ${scripting_FILES})
target_link_libraries( scripting ${PYTHON_LIBRARIES})
