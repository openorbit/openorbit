add_library(examplePlugin SHARED "example-plugin.c")
get_target_property(loc examplePlugin LOCATION)
set(ORBIT_PLUGIN ${loc} PARENT_SCOPE)
