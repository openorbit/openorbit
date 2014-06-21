include(swig)
include(ibtool)

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

  libgencds/array.c
  libgencds/avl-tree.c
  libgencds/hashtable.c
  libgencds/heap.c
  libgencds/list.c

  rendering/3ds.c
  rendering/overlay.c
  rendering/ac3d.c
  rendering/particles.c
  rendering/camera.c
  rendering/cmod.c
  rendering/collada.c
  rendering/colour.c
  rendering/gui.c
  rendering/light.c
  rendering/location.c
  rendering/material.c
  rendering/object.c
  rendering/scene.c
  rendering/scenegraph.c
  rendering/shader-manager.c
  rendering/sky.c
  rendering/terrain.c
  rendering/text.c
  rendering/texture.c
  rendering/viewport.c
  rendering/window.c

  physics/areodynamics.c
#  physics/barneshut.c
  physics/celestial-object.c
  physics/collision.c
  physics/mass.c
  physics/object.c
  physics/octtree.c
  physics/orbit.c
  physics/particles.c
  physics/rockets.c
  physics/sectors.c
  physics/world.c

  scripting/scripting.c

  sim/actuator.c
  sim/battery.c
  sim/class.c
  sim/haps.c
  sim/propellant-tank.c
  sim/pubsub.c
  sim/simenvironment.c
  sim/simevent.c
  sim/simtime.c
  sim/spacecraft-control.c
  sim/spacecraft.c
  sim/world-loader.c

  sim/mfd/mfd.c

  sim/spacecraft/mercury/mercury.c
  )

swig_wrap(python scripting/openorbit/config.i)
swig_wrap(python scripting/openorbit/event.i)
set(scripting_src
    ${CMAKE_CURRENT_BINARY_DIR}/scripting/openorbit/config.i.c
    ${CMAKE_CURRENT_BINARY_DIR}/scripting/openorbit/event.i.c
)

set(darwin_src
  platform/macosx/OOAppDelegate.m
  platform/macosx/OOOpenGLView.m
  platform/macosx/cgbitmaps.c
  platform/macosx/hid.m
  platform/macosx/menu-manager.m
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


set(ORBIT_SCRIPTS
  ${CMAKE_SOURCE_DIR}/rsrc/script/init.py
  ${CMAKE_SOURCE_DIR}/rsrc/script/postinit.py
)

set(ORBIT_SHADERS
  rendering/shaders/fixed/fixed.vertex
  rendering/shaders/fixed/fixed.fragment
  rendering/shaders/flat/flat.vertex
  rendering/shaders/flat/flat.fragment
  rendering/shaders/overlay/overlay.vertex
  rendering/shaders/overlay/overlay.fragment
  rendering/shaders/planet/planet.vertex
  rendering/shaders/planet/planet.fragment
  rendering/shaders/shared/shader-types.glsl
  rendering/shaders/sky/sky.vertex
  rendering/shaders/sky/sky.fragment
  rendering/shaders/spacecraft/spacecraft.vertex
  rendering/shaders/spacecraft/spacecraft.fragment
  rendering/shaders/star/star.vertex
  rendering/shaders/star/star.fragment
)

  #${rendering_src} ${vessel_src} ${geo_src} ${sim_src} ${gencds_src} ${common_src})

#set(openorbit_FILES
#	sim.h io-manager.h settings.h plugin-handler.h
#	CMakeLists.txt)

#add_cmake_list()

list(APPEND openorbit_SRC ${scripting_src})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
add_definitions(-DHAVE_CONFIG)

# Default target names for open orbit app
set(open_orbit_name "openorbit")
set(open_orbit_debug_name "openorbit-dbg")

if (APPLE)
  ibtool_compile(platform/macosx/MainMenu.xib)
  set(ORBIT_NIBS
      ${CMAKE_CURRENT_BINARY_DIR}/platform/macosx/MainMenu.nib)
 
  list(APPEND openorbit_SRC ${darwin_src})
    
  set(MACOSX_BUNDLE_INFO_STRING "Open Orbit, an interactive space flight simulator")
#    SET(MACOSX_BUNDLE_ICON_FILE)
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.openorbit.openorbit")
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

set_source_files_properties(${ORBIT_NIBS}
                            PROPERTIES
                            GENERATED true
                            MACOSX_PACKAGE_LOCATION Resources/)

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
	       ${ORBIT_NIBS}
             )

set_target_properties(openorbit
                      PROPERTIES
                      RELEASE_OUTPUT_NAME "${open_orbit_name}"
                      DEBUG_OUTPUT_NAME "${open_orbit_debug_name}")

target_link_libraries(openorbit
                      vmath celmek imgload auload
                      dl ${LIBEDIT} ${LIBJANSSON}
                      ${PNG_LIBRARIES} ${JPEG_LIBRARIES}
                      ${OPENGL_LIBRARIES} ${OPENAL_LIBRARY}
		      ${SDL_LIBRARY}
		      ${GLUT_LIBRARIES} ${LIBXML2_LIBRARIES}
		      ${PYTHON_LIBRARIES} ${FREETYPE_LIBRARIES}
		      ${IOKIT} ${OBJC} ${COCOA}
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
