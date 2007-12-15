# Root CMakeList file

project(openorbit)


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

# Turn on testing system, our tests are located in the test directory
option(ENABLE_TESTS "Enable the testing system" off)
if (ENABLE_TESTS)
    enable_testing()
endif (ENABLE_TESTS)

# TODO: Support more compilers
if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS -std=c99)
    set(CMAKE_C_FLAGS_DEBUG -std=c99)
else (CMAKE_COMPILER_IS_GNUCC)
    message(FATAL_ERROR "Only GCC supported for the moment")
endif (CMAKE_COMPILER_IS_GNUCC)

# Support command when working with Xcode
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/AddCMakeList.txt)

add_subdirectory(src)
# add_subdirectory(rsrc)

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
