file(GLOB ORBIT_SCRIPTS_LOCAL "*.py")

set(ORBIT_SCRIPTS ${ORBIT_SCRIPTS_LOCAL} PARENT_SCOPE)
set_source_files_properties(${ORBIT_SCRIPTS}
                            PROPERTIES
                            MACOSX_PACKAGE_LOCATION Resources/)
