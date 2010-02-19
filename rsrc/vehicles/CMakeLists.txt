set(ORBIT_RES_VEHICLES "${CMAKE_SOURCE_DIR}/rsrc/vehicles/spacecrafts/mercury/mercury.hrml"
                       "${CMAKE_SOURCE_DIR}/rsrc/vehicles/spacecrafts/mercury/mercury.ac"
                       "${CMAKE_SOURCE_DIR}/rsrc/vehicles/spacecrafts/mercury/redstone.ac")

set_source_files_properties("${CMAKE_SOURCE_DIR}/rsrc/vehicles/spacecrafts/mercury/mercury.hrml"
                            "${CMAKE_SOURCE_DIR}/rsrc/vehicles/spacecrafts/mercury/mercury.ac"
                            "${CMAKE_SOURCE_DIR}/rsrc/vehicles/spacecrafts/mercury/redstone.ac"
                            PROPERTIES
                            MACOSX_PACKAGE_LOCATION Resources/spacecrafts/mercury/)

