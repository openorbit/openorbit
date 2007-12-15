find_library(CHECK_LIB check)

if (NOT CHECK_LIB)
    message(FATAL_ERROR "Bulding tests requires that check is installed")
endif (NOT CHECK_LIB)

add_subdirectory(common)

include_directories(common ${CMAKE_SOURCE_DIR}/src)
link_libraries(testMain ${CHECK_LIB})
add_subdirectory(t000_hashtable)
add_subdirectory(t001_heap)
add_subdirectory(t002_tga)
add_subdirectory(t003_math)

add_subdirectory(t005_object_manager)
add_subdirectory(t006_list)
