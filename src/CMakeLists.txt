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

#add_subdirectory(libgencds)
add_subdirectory(physics)
add_subdirectory(scripting)
add_subdirectory(parsers)
add_subdirectory(plugins)

#aux_source_directory(libgencds gencds_src)
#aux_source_directory(rendering rendering_src)
#aux_source_directory(vessel vessel_src)
#aux_source_directory(geo geo_src)
#aux_source_directory(sim sim_src)
#aux_source_directory(common common_src)

set(openorbit_SRC
  io-manager.c
  plugin-handler.c
  sim.c
  log.c
  settings.c
  startup.c
  common/lwcoord.c
  common/moduleinit.c
  common/stringextras.c
  common/mapped-file.c
  common/palloc.c
  scripting/scripting.c
  
  geo/ellipse.c

  parsers/3ds.c
  parsers/ac3d.c
  parsers/cmod.c
  parsers/collada.c
  parsers/hrml.c
  parsers/hrml3d.c
  parsers/model.c

  libgencds/array.c
  libgencds/avl-tree.c
  libgencds/hashtable.c
  libgencds/heap.c
  libgencds/list.c
  libgencds/object-manager.c
  libgencds/object-manager2.c


  physics/areodynamics.c
  physics/collision.c
  physics/mass.c
  physics/object.c
  physics/orbit.c
  physics/particles.c
  physics/rockets.c
  physics/sectors.c
  
  rendering/camera.c
  rendering/render.c
  rendering/colour.c
  rendering/scenegraph.c
  rendering/drawable.c
  rendering/shader-manager.c
  rendering/gui.c
  rendering/sky.c
  rendering/light.c
  rendering/terrain.c
  rendering/material.c
  rendering/text.c
  rendering/particles.c
  rendering/texture.c
  rendering/planet.c

  sim/actuator.c
  sim/simenvironment.c
  sim/battery.c
  sim/simevent.c
  sim/class.c
  sim/simtime.c
  sim/haps.c
  sim/spacecraft-control.c
  sim/propellant-tank.c
  sim/spacecraft.c
  sim/pubsub.c
  sim/world-loader.c
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

add_dependencies(openorbit ${ORBIT_PLUGIN_TARGETS})
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
