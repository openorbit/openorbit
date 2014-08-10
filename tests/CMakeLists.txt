include_directories(${LIBCHECK_INCLUDE_DIR})
add_subdirectory(common)

include_directories(
  common
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/lib/vmath/include
  ${CMAKE_SOURCE_DIR}/lib/celmek/include)

link_libraries(testMain ${LIBCHECK_LIBRARY})
add_subdirectory(t000_hashtable)
add_subdirectory(t001_heap)
add_subdirectory(t002_tga)
add_subdirectory(t004_physics)
add_subdirectory(t005_object_manager)
add_subdirectory(t006_list)
add_subdirectory(t007_object_manager2)
add_subdirectory(t008_hrml)
