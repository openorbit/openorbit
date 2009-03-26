#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -undefined dynamic_lookup")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -bundle -undefined dynamic_lookup")
set(LINK_FLAGS "${LINK_FLAGS} -bundle -undefined dynamic_lookup")
add_subdirectory(example)
#-bundle_loader myapp
#add_dependencies(${ORBIT_PLUGIN_TARGET} openorbit)

set(ORBIT_PLUGINS ${ORBIT_PLUGIN} PARENT_SCOPE)
set(ORBIT_PLUGIN_TARGETS ${ORBIT_PLUGIN_TARGET} PARENT_SCOPE)
