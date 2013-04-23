include_directories(../lib/vmath/include)
include_directories(../lib/celmek/include)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${PYTHON_INCLUDE_PATH})
if (${SDLTTF_FOUND})
  include_directories(${SDLTTF_INCLUDE_DIR})
endif (${SDLTTF_FOUND})

include_directories(${CONFIG_INC})
include_directories(${PNG_INCLUDE_DIR})
include_directories(${JPEG_INCLUDE_DIR})

add_definitions(${PNG_DEFINITIONS})
# Set proper shared lib extension
set(so_ext "\".so\"")
if (APPLE)
    SET(so_ext "\".dylib\"")
endif (APPLE)
add_definitions(-DSO_EXT=${so_ext})

include_directories(${OPENGL_INCLUDE_DIR} ${SDL_INCLUDE_DIR})
include_directories(rendering)

if (APPLE)
    include_directories(platform/macosx/)
elseif(UNIX)
    include_directories(platform/posix/)
    include_directories(platform/sdl/)
endif (APPLE)

set(openorbit_SRC
  io-manager.c
  plugin-handler.c
  sim.c
  terminal.c
  log.c
  settings.c
  startup.c

  parsers/hrml.c
  geo/ellipse.c

  common/mapped-file.c
  common/monotonic-time.c
  common/stringextras.c
  common/moduleinit.c
  common/palloc.c

  rendering/3ds.c
  rendering/overlay.c
  rendering/ac3d.c
  rendering/particles.c
  rendering/camera.c
  rendering/scene.c
  rendering/cmod.c
  rendering/scenegraph.c
  rendering/collada.c
  rendering/shader-manager.c
  rendering/colour.c
  rendering/sky.c
  rendering/gui.c
  rendering/terrain.c
  rendering/light.c
  rendering/text.c
  rendering/location.c
  rendering/texture.c
  rendering/material.c
  rendering/viewport.c
  rendering/object.c
  rendering/window.c

  physics/areodynamics.c
  physics/mass.c
  physics/particles.c
  physics/barneshut.c
  physics/object.c
  physics/rockets.c
  physics/celestial-object.c
  physics/octtree.c
  physics/sectors.c
  physics/collision.c
  physics/orbit.c
  physics/world.c

  sim/actuator.c
  sim/propellant-tank.c
  sim/simtime.c
  sim/battery.c
  sim/pubsub.c
  sim/spacecraft-control.c
  sim/class.c
  sim/simenvironment.c
  sim/spacecraft.c
  sim/haps.c
  sim/simevent.c
  sim/world-loader.c

  sim/spacecraft/mercury/mercury.c
  )

set(darwin_src
  platform/macosx/OOAppDelegate.m
  platform/macosx/OOOpenGLView.m
  platform/macosx/cgbitmaps.c
  platform/macosx/hid.m
  platform/macosx/mouse.c
  platform/macosx/osx-main.m
  platform/macosx/res-manager.c
)
set(unix_src
  platform/sdl/sdl-events.c
  platform/sdl/sdl-main.c
  platform/sdl/sdl-mouse.c
  platform/sdl/sdl-window.c
  platform/posix/res-manager.c)

  #${rendering_src} ${vessel_src} ${geo_src} ${sim_src} ${gencds_src} ${common_src})

#set(openorbit_FILES
#	sim.h io-manager.h settings.h plugin-handler.h
#	CMakeLists.txt)

#add_cmake_list()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
add_definitions(-DHAVE_CONFIG)

# Default target names for open orbit app
set(open_orbit_name "openorbit")
set(open_orbit_debug_name "openorbit-dbg")

if (APPLE)
    list(APPEND openorbit_SRC ${darwin_src})
    
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
elseif(UNIX)
# Unix system that is not darwin

    list(APPEND openorbit_SRC ${unix_src})

endif (APPLE)

set_source_files_properties(${ORBIT_SCRIPTS}
                            PROPERTIES
                            MACOSX_PACKAGE_LOCATION Resources/script/)

set_source_files_properties(${ORBIT_DEP_YAML}
                            PROPERTIES
                            MACOSX_PACKAGE_LOCATION Resources/python/yaml)

set_source_files_properties(${ORBIT_PLUGINS}
                            PROPERTIES
                            GENERATED true
                            MACOSX_PACKAGE_LOCATION Resources/plugins)

include(../rsrc/CMakeLists.txt)

add_executable(openorbit MACOSX_BUNDLE
               ${openorbit_SRC}
               ${openorbit_FILES}
               ${ORBIT_RES_TEX}
               ${ORBIT_RES_DATA}
               ${ORBIT_RES_FONT}
               ${ORBIT_RES_VEHICLES}
               ${ORBIT_SCRIPTS}
               ${ORBIT_DEP_YAML}
               ${ORBIT_PLUGINS}
             )

set_target_properties(openorbit
                      PROPERTIES
                      RELEASE_OUTPUT_NAME "${open_orbit_name}"
                      DEBUG_OUTPUT_NAME "${open_orbit_debug_name}")

target_link_libraries(openorbit
                      vmath imgload auload
                      dl ${LIBJANSSON}
                      ${PNG_LIBRARIES} ${JPEG_LIBRARIES}
                      ${OPENGL_LIBRARIES} ${OPENAL_LIBRARY}
		      ${SDL_LIBRARY}
		      ${GLUT_LIBRARIES} ${LIBXML2_LIBRARIES}
		      ${PYTHON_LIBRARIES} ${FREETYPE_LIBRARIES}
		      ${IOKIT}
		      )

if (${SDLTTF_FOUND})
  target_link_libraries(openorbit ${SDLTTF_LIBRARY})
endif (${SDLTTF_FOUND})
#target_link_libraries(openorbit ${ORBIT_PLUGINS})

install(TARGETS openorbit
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        RESOURCE DESTINATION share/openorbit/
        BUNDLE DESTINATION Applications )
