#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ddsenabler_yaml" for configuration "Debug"
set_property(TARGET ddsenabler_yaml APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(ddsenabler_yaml PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libddsenabler_yaml.so.0.1.0"
  IMPORTED_SONAME_DEBUG "libddsenabler_yaml.so.0"
  )

list(APPEND _cmake_import_check_targets ddsenabler_yaml )
list(APPEND _cmake_import_check_files_for_ddsenabler_yaml "${_IMPORT_PREFIX}/lib/libddsenabler_yaml.so.0.1.0" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
