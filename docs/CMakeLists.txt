# Add docs target if we have doxygen
# Optionally add documentation target if we have Doxygen available
find_package(Doxygen)

if (DOXYGEN_FOUND)
  # Try to locate dot, if available we use it to generate call graphs
  find_program(DOT_EXECUTABLE dot)
  if (DOT_EXECUTABLE)
    set(DOT_FOUND "YES")
    set(DOT_PATH "${DOT_EXECUTABLE}")
  else (DOT_EXECUTABLE)
    set(DOT_FOUND "NO")
    set(DOT_PATH "")
  endif (DOT_EXECUTABLE)

  # Configure the doxygen file
  set(DOXYGEN_CONFIG "${CMAKE_BINARY_DIR}/doxygen.conf")

  configure_file( "${CMAKE_SOURCE_DIR}/doxygen.conf.in"
    "${DOXYGEN_CONFIG}"
    @ONLY)

  if (NOT TARGET docs)
    add_custom_target(docs ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIG}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  endif (NOT TARGET docs)
endif (DOXYGEN_FOUND)
