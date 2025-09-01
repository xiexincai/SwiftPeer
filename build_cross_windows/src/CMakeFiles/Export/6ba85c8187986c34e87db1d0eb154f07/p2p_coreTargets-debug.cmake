#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "p2p_core::p2p_core" for configuration "Debug"
set_property(TARGET p2p_core::p2p_core APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(p2p_core::p2p_core PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/p2p_core_d.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/p2p_core_d.dll"
  )

list(APPEND _cmake_import_check_targets p2p_core::p2p_core )
list(APPEND _cmake_import_check_files_for_p2p_core::p2p_core "${_IMPORT_PREFIX}/lib/p2p_core_d.lib" "${_IMPORT_PREFIX}/bin/p2p_core_d.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
