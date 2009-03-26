add_library(examplePlugin MODULE "example-plugin.c")
get_target_property(loc examplePlugin LOCATION)
set(ORBIT_PLUGIN ${loc} PARENT_SCOPE)
set(ORBIT_PLUGIN_TARGET examplePlugin PARENT_SCOPE)
