find_library(CHECK_LIB check)
find_path(CHECK_INCLUDES check.h)

if (NOT CHECK_LIB)
    message(FATAL_ERROR "Bulding tests requires that check is installed")
endif (NOT CHECK_LIB)

if (NOT CHECK_INCLUDES)
    message(FATAL_ERROR "Cannot find check headers")
endif (NOT CHECK_INCLUDES)

include_directories(${CHECK_INCLUDES})
add_subdirectory(common)

include_directories(common ${CMAKE_SOURCE_DIR}/src)
link_libraries(testMain ${CHECK_LIB})
add_subdirectory(t000_hashtable)
add_subdirectory(t001_heap)
add_subdirectory(t002_tga)
add_subdirectory(t003_math)
add_subdirectory(t004_physics)
add_subdirectory(t005_object_manager)
add_subdirectory(t006_list)
