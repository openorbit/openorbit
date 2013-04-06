# Root CMakeList file

# Force use of clang for vector extensions
set(CMAKE_C_COMPILER_INIT "clang")
set(CMAKE_CXX_COMPILER_INIT "clang")

project(openorbit)
cmake_minimum_required(VERSION 2.8)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Analyse Release."
        FORCE)
endif (NOT CMAKE_BUILD_TYPE)

# Require SDL and OpenGL on non OS X targets
if (NOT APPLE)
  find_package(SDL REQUIRED)
  find_package(SDL_ttf QUIET)
endif (NOT APPLE)

if (APPLE)
  find_library(IOKIT IOKit)
  if (NOT IOKIT)
    message(FATAL_ERROR "cannot find IOKit.framework")
  endif (NOT IOKIT)
endif (APPLE)

find_package(OpenGL REQUIRED)
find_package(OpenAL REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(SWIG REQUIRED)
find_package(freetype REQUIRED)
find_package(libxml2 REQUIRED)

find_library(LIBJANSSON jansson)
if (NOT LIBJANSSON)
  message(FATAL_ERROR "cannot find libjansson")
endif (NOT LIBJANSSON)
# TODO: Download missing packages, i.e. sdl, ode et.al.
# file(DOWNLOAD url file [TIMEOUT timeout] [STATUS status] [LOG log])

# Enable later when sound system has been added
# find_package(OpenAL REQUIRED)

# Some binary I/O must know if byte-swapping is needed
include(TestBigEndian)
test_big_endian(BIG_ENDIAN)

# Need python libraries, and also the pyrexc compiler
find_package(PythonLibs REQUIRED)

if (APPLE)
    include(cmake/InstallMacRes.txt)
endif (APPLE)

# TODO: Support more compilers
# TODO: Move compiler flags to file in cmake/arch-flags.cmake
include(cmake/arch-flags.cmake)

include_directories(include)
include_directories(${FREETYPE_INCLUDE_DIRS})
include_directories(${LIBXML2_INCLUDE_DIR})
add_subdirectory(lib)
add_subdirectory(scripts)
add_subdirectory(src)
add_subdirectory(include)

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
  set(DOXYGEN_CONFIG "${CMAKE_BINARY_DIR}/Doxyfile")

  configure_file( "${CMAKE_SOURCE_DIR}/Doxyfile.in"
    "${DOXYGEN_CONFIG}"
    @ONLY)
  if (NOT TARGET docs)
    add_custom_target(docs ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIG}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  endif (NOT TARGET docs)
endif (DOXYGEN_FOUND)


# Locate libcheck, we cannot build tests without it.
find_path(LIBCHECK_INCLUDE_DIR check.h)
find_library(LIBCHECK_LIBRARY check)

if (LIBCHECK_LIBRARY AND LIBCHECK_INCLUDE_DIR)
  enable_testing()

  if(NOT TARGET check)
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
  endif(NOT TARGET check)
  add_subdirectory(tests)
endif (LIBCHECK_LIBRARY AND LIBCHECK_INCLUDE_DIR)


set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open Orbit, an interactive modular space flight simulator")
set(CPACK_PACKAGE_VENDOR "Mattias Holm")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.org")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.org")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "1")

include(CPack)
