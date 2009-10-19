include_directories(${CMAKE_SOURCE_DIR}/include)

include_directories(${PNG_INCLUDE_DIR})
include_directories(${JPEG_INCLUDE_DIR})

set(imgload_SRC image.c jpg.c pngfile.c tga.c)
set(imgload_FILES
    ${CMAKE_SOURCE_DIR}/include/imgload/image.h)

add_library(imgload STATIC ${imgload_SRC} ${imgload_FILES})
target_link_libraries(imgload ${PNG_LIBRARIES} ${JPEG_LIBRARIES})