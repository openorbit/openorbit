include(CMakeParseArguments)

function(static_plugin)
  set(options)
  set(oneValueArgs TARGET NAME)
  set(multiValueArgs SOURCE RSRC)
  cmake_parse_argements(PLUGIN
    "${options}" "${oneValueArgs}" "${multiValueArgs}"
    ${ARGN})
  
  # Record the added target in the global spacecraft targets list and
  # record the rsrc provided by the target
  set(STATIC_PLUGIN_TARGETS "${STATIC_PLUGIN_TARGETS};${PLUGIN_TARGET}"
    CACHE INTERNAL FORCE)
  set(SPACECRAFT_${PLUGIN_TARGET}_RSRC "${PLUGIN_RSRC}"
    CACHE INTERNAL FORCE)
  
  add_library(${PLUGIN_TARGET} STATIC ${PLUGIN_SOURCE})
  
endfunction(static_plugin)
