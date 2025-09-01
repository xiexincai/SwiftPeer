#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "p2p_core::p2p_core" for configuration "Release"
set_property(TARGET p2p_core::p2p_core APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(p2p_core::p2p_core PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/p2p_core.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/p2p_core.dll"
  )

list(APPEND _cmake_import_check_targets p2p_core::p2p_core )
list(APPEND _cmake_import_check_files_for_p2p_core::p2p_core "${_IMPORT_PREFIX}/lib/p2p_core.lib" "${_IMPORT_PREFIX}/bin/p2p_core.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
