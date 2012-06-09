include_directories(${CMAKE_SOURCE_DIR}/include)

set(lib_SRC pcm.c wav.c)
add_library(auload STATIC ${lib_SRC})
