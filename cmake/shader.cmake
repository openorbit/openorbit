include(CMakeParseArguments)

function(add_shader)
  set(options PREPROCESS)
  set(oneValueArgs TARGET NAME)
  set(multiValueArgs SOURCE INCLUDE)
  
  cmake_parse_argements(SHADER
    "${options}" "${oneValueArgs}" "${multiValueArgs}"
    ${ARGN})
  
  # Record the added target in the global spacecraft targets list and
  # record the rsrc provided by the target

  if (SHADER_PREPROCESS)
    # Build include string
    set(INCLUDES "")
    foreach (inc "${SHADER_INCLUDE}")
      set(INCLUDES "${INCLUDES} -I${inc}")
    endforeach (inc)

    # Add CPP targets
    foreach (src "${SHADER_SOURCE}")
      add_custom_command(
	OUTPUT 
	COMMAND cpp "${INCLUDES}" "${CMAKE_CURRENT_SOURCE_DIR}/${src}"
	             -o "${CMAKE_CURRENT_BINARY_DIR}/${src}"
	MAIN_DEPENDENCY "${src}")
    endforeach (src)

    
  endif (SHADER_PREPROCESS)

  set(STATIC_PLUGIN_TARGETS "${STATIC_PLUGIN_TARGETS};${PLUGIN_TARGET}"
    CACHE INTERNAL FORCE)
  set(SPACECRAFT_${PLUGIN_TARGET}_RSRC "${PLUGIN_RSRC}"
    CACHE INTERNAL FORCE)
  
  add_library(${PLUGIN_TARGET} STATIC ${PLUGIN_SOURCE})
  
endfunction(add_shader)
