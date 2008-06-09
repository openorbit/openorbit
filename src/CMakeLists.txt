include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${PYTHON_INCLUDE_PATH})

# Set proper shared lib extension
set(so_ext "\\\".so\\\"")
if (APPLE)
    SET(so_ext "\\\".dylib\\\"")
endif (APPLE)
add_definitions(-DSO_EXT=${so_ext})

include_directories(${OPENGL_INCLUDE_DIR} ${SDL_INCLUDE_DIR})

if (APPLE)
    include_directories(platform/macosx/)
endif (APPLE)

add_subdirectory(libgencds)
add_subdirectory(vmath)
add_subdirectory(physics)
add_subdirectory(scripting)
add_subdirectory(parsers)

aux_source_directory(rendering rendering_src)
aux_source_directory(vessel vessel_src)

set(openorbit_SRC
	main.c sim.c error.c settings.c io-manager.c plugin-handler.c
	${rendering_src} ${vessel_src} )

set(openorbit_FILES
	sim.h error.h io-manager.h settings.h plugin-handler.h
	CMakeLists.txt)

add_cmake_list()

# Default target names for open orbit app
set(open_orbit_name "openorbit")
set(open_orbit_debug_name "openorbit-dbg")

if (APPLE)
    aux_source_directory(platform/macosx macosx_SRC)
    
    list(APPEND openorbit_SRC ${macosx_SRC})
    
    set(MACOSX_BUNDLE_INFO_STRING "Open Orbit, an interactive space flight simulator")
#    SET(MACOSX_BUNDLE_ICON_FILE)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "Open Orbit")
#    SET(MACOSX_BUNDLE_LONG_VERSION_STRING)
    set(MACOSX_BUNDLE_BUNDLE_NAME "Open Orbit")
#    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING)
#    SET(MACOSX_BUNDLE_BUNDLE_VERSION)
    set(MACOSX_BUNDLE_COPYRIGHT "Mattias Holm")

    # update name for the mac
    set(open_orbit_name "Open Orbit")
    set(open_orbit_debug_name "Open Orbit Debug")
endif (APPLE)

set_source_files_properties(${ORBIT_RES_TEX}
                            PROPERTIES
                            MACOSX_PACKAGE_LOCATION Resources/)

set_source_files_properties(${ORBIT_RES_DATA}
                            PROPERTIES
                            MACOSX_PACKAGE_LOCATION Resources/)

set_source_files_properties(${ORBIT_SCRIPTS}
                            PROPERTIES
                            MACOSX_PACKAGE_LOCATION Resources/)


add_executable(openorbit MACOSX_BUNDLE
               ${openorbit_SRC}
               ${openorbit_FILES}
               ${ORBIT_RES_TEX}
               ${ORBIT_RES_DATA}
               ${ORBIT_SCRIPTS})

set_target_properties(openorbit
                      PROPERTIES
                      RELEASE_OUTPUT_NAME "${open_orbit_name}"
                      DEBUG_OUTPUT_NAME "${open_orbit_debug_name}")

target_link_libraries(openorbit
                      math scripting parsers dl gencds
                      ${OPENGL_LIBRARIES} ${SDL_LIBRARY} ${GLUT_LIBRARIES})




install(TARGETS openorbit
        RUNTIME DESTINATION bin
        RESOURCE DESTINATION share/openorbit/
        BUNDLE DESTINATION Applications )
