MACRO(INSTALL_MAC_RES target files)
  if (APPLE)
    string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE_CAPS)
    
    get_target_property(outName_conf ${target} ${BUILD_TYPE_CAPS}_OUTPUT_NAME)
    foreach (file ${files})
      add_custom_command(TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/install-mac-res.sh
        ARGS -f ${file} -a ${outName_conf}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endforeach (file)
    
  endif (APPLE)
ENDMACRO(INSTALL_MAC_RES target config files)
