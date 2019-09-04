# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/module/version/version.hpp.in"
  "${PROJECT_SOURCE_DIR}/module/version/version.hpp"
  )

configure_file (
  "${PROJECT_SOURCE_DIR}/module/core/storedata_defines.hpp.in"
  "${PROJECT_SOURCE_DIR}/module/core/storedata_defines.hpp"
  )

configure_file (
  "${PROJECT_SOURCE_DIR}/module/record/inc/record/lib_configuration.hpp.in"
  "${PROJECT_SOURCE_DIR}/module/record/inc/record/lib_configuration.hpp"
  )

######################################################################
# Subdirectories

add_subdirectory (buffer)
add_subdirectory (logger)
add_subdirectory (codify)
add_subdirectory (record)
add_subdirectory (video)
