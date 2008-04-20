# Root CMakeList file

project(openorbit)
cmake_minimum_required(VERSION 2.6)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release."
        FORCE)
endif (NOT CMAKE_BUILD_TYPE)

# If we are to enable buildinc the docs, locate the doxygen and LaTeX programs
# Doxygen is mandatory for this, LaTeX is optional
if (ENABLE_DOCS)
    find_package(Doxygen REQUIRED)
    find_package(LATEX)	
endif (ENABLE_DOCS)

# Require SDL and OpenGL
find_package(SDL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

# Enable later when sound system has been added
# find_package(OpenAL REQUIRED)

# Some binary I/O must know if byte-swapping is needed
include(TestBigEndian)
test_big_endian(BIG_ENDIAN)

# Need python libraries, and also the pyrexc compiler
option(WITH_PYTHON "Enable Python Scripting" on)

if (WITH_PYTHON)
    find_package(PythonLibs REQUIRED)
    add_definitions(-DWITH_PYTHON)
    include(cmake/Pyrex.txt)
endif (WITH_PYTHON)

if (APPLE)
    include(cmake/InstallMacRes.txt)
endif (APPLE)

option(WITH_GUILE "Enable Scheme Scripting with Guile (DEPRECATED)" off)

find_program(FLEX flex)
if (NOT FLEX)
    message(FATAL_ERROR "Could not locate flex")
endif (NOT FLEX)

# It is possible to build supporting both single and double math,
# note that binary plugins will need to match the precision of the
# open orbit application
option(WITH_DOUBLE_PRECISION "Select fp precision as double" off)
if (WITH_DOUBLE_PRECISION)
    add_definitions(-DUSE_DOUBLE_FP_MATH)
else (WITH_DOUBLE_PRECISION)
    add_definitions(-DUSE_SINGLE_FP_MATH)
endif (WITH_DOUBLE_PRECISION)

option(ENABLE_VECTORISE "Enable SIMD vectorisation" on)


# Turn on testing system, our tests are located in the test directory
option(ENABLE_TESTS "Enable the testing system" off)
if (ENABLE_TESTS)
    enable_testing()
endif (ENABLE_TESTS)

# TODO: Support more compilers
# TODO: Move compiler flags to file in cmake/arch-flags.cmake
if (CMAKE_COMPILER_IS_GNUCC)
    include(cmake/arch-flags.cmake)
else (CMAKE_COMPILER_IS_GNUCC)
    message(FATAL_ERROR "Only GCC supported for the moment")
endif (CMAKE_COMPILER_IS_GNUCC)



# Support command when working with Xcode
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/AddCMakeList.txt)

if (HAVE_ALTIVEC)
    add_definitions(-DENABLE_VECTORISE)
endif (HAVE_ALTIVEC)



include_directories(include)
add_subdirectory(rsrc)
add_subdirectory(src)
add_subdirectory(include)
#add_subdirectory(rsrc)

if (ENABLE_TESTS)
    add_subdirectory(tests)
endif (ENABLE_TESTS)


set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open Orbit, an interactive modular space flight simulator")
set(CPACK_PACKAGE_VENDOR "Mattias Holm")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "1")

include(CPack)
