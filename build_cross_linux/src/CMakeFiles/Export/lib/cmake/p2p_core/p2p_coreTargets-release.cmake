#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "p2p_core::p2p_core" for configuration "Release"
set_property(TARGET p2p_core::p2p_core APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(p2p_core::p2p_core PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libp2p_core.so.1.0.0"
  IMPORTED_SONAME_RELEASE "libp2p_core.so.1"
  )

list(APPEND _IMPORT_CHECK_TARGETS p2p_core::p2p_core )
list(APPEND _IMPORT_CHECK_FILES_FOR_p2p_core::p2p_core "${_IMPORT_PREFIX}/lib/libp2p_core.so.1.0.0" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
