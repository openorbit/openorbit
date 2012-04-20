include_directories(${CMAKE_SOURCE_DIR}/include)

include_directories(${PNG_INCLUDE_DIR})
include_directories(${JPEG_INCLUDE_DIR})

set(imgload_SRC image.c jpg.c pngfile.c tga.c)

add_library(imgload STATIC ${imgload_SRC})
target_link_libraries(imgload ${PNG_LIBRARIES} ${JPEG_LIBRARIES})