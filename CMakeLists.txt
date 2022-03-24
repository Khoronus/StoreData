######################################################################
# The project name
cmake_minimum_required (VERSION 3.13)

######################################################################
# Import macro
include ("cmakemacro/cmake_macro_setvariables.txt")

######################################################################
# Read the path configuration file (global)
# get the location of the global configuration path
file(READ "configure/global_path_configuration.txt" pt_global)
string(REPLACE "\n" ";" PATH_GLOBAL_LIST ${pt_global})
GET_PATH("${PATH_GLOBAL_LIST}" "path_global " path_global)
set(PLATFORM_PATH "${path_global}")
message(STATUS "PLATFORM_PATH = (${PLATFORM_PATH})")
# get the location of the global configuration projects path
find_file (pt_location_global NAMES "global_projects_path_configuration.txt" REQUIRED 
  PATHS ${PLATFORM_PATH} 
  DOC "Path to global configuration paths.")
string(REPLACE "\n" ";" PATH_LOCATION_LIST ${pt_location_global})
message(STATUS "list = ${PATH_LOCATION_LIST}")
# read the configuration file global
file(READ "${pt_location_global}" pt_conf_global)
#string(REPLACE "\n" ";" PATH_GLOBAL_FULL_LIST ${pt_conf_global})
#message(STATUS "list = ${PATH_GLOBAL_FULL_LIST}")

######################################################################
# Read the path configuration file (local)
set(path_local_cfg "../configure/path_configuration.txt")
if (EXISTS ${path_local_cfg})
  file(READ ${path_local_cfg} pt_conf)
  message(STATUS "[+] open:${path_local_cfg}")
else()
  message(STATUS "[-] open:${path_local_cfg}")
endif()

######################################################################
# Combine global and local
string(CONCAT MULTI ${pt_conf_global} ${pt_conf})
# remove all the new lines
string(REPLACE "\n" ";" PATH_FULL_LIST ${MULTI})
message(STATUS "list = ${PATH_FULL_LIST}")

######################################################################
# Get the first variable
GET_PATH("${PATH_FULL_LIST}" "path_opencv " path_opencv)
message(STATUS "path_opencv = (${path_opencv})")
GET_PATH("${PATH_FULL_LIST}" "path_boost " path_boost)
message(STATUS "path_boost = (${path_boost})")
GET_PATH("${PATH_FULL_LIST}" "path_boost_lib " path_boost_lib)
message(STATUS "path_boost = (${path_boost_lib})")

######################################################################
# Project
SET( MAIN_PROJ_NAME      "StoreData" )
project (${MAIN_PROJ_NAME})

set(STOREDATA_VERSION_MAJOR 0 CACHE STRING "major version of storedata" FORCE)
set(STOREDATA_VERSION_MINOR 14 CACHE STRING "minor version of storedata" FORCE)
set(STOREDATA_VERSION_SUBMINOR 2 CACHE STRING "subminor version of storedata" FORCE)
set(STOREDATA_VERSION_PATCH 0 CACHE STRING "patch version of storedata" FORCE)
set(STOREDATA_VERSION "${STOREDATA_VERSION_MAJOR}.${STOREDATA_VERSION_MINOR}.${STOREDATA_VERSION_SUBMINOR}.${STOREDATA_VERSION_PATCH}"
                             CACHE STRING "version of storedata" FORCE)
set(STOREDATA_VERSION_NAME \"StoreDataAlpha\" CACHE STRING "name version of storedata" FORCE)

# set a debug postfix
set(CMAKE_DEBUG_POSTFIX "-dbg")

# installation destinations
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE STRING "Install prefix (e.g. /usr/local/)" FORCE)
endif()
if(UNIX)
    include(GNUInstallDirs)

    set(STOREDATA_INC_INSTALL_DIR "module") 
    set(STOREDATA_RUNTIME_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}") 
    set(STOREDATA_LIBRARY_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")
    set(STOREDATA_ARCHIVE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")
    set(STOREDATA_FRAMEWORK_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")

    set(STOREDATA_CMAKE_CONFIG_INSTALL_DIR "cmake")
    set(STOREDATA_ADDITIONAL_FILES_INSTALL_DIR "./")

    set(STOREDATA_RUNTIME_INSTALL_DIR "bin") # for the nodesize_dbg, just ignore version and the like
    set(STOREDATA_INC_INSTALL_DIR "module") # header filestree

elseif(WIN32)
    set(STOREDATA_INC_INSTALL_DIR "module")
    set(STOREDATA_RUNTIME_INSTALL_DIR   "bin") 
    set(STOREDATA_LIBRARY_INSTALL_DIR   "bin")
    set(STOREDATA_ARCHIVE_INSTALL_DIR   "lib")
    set(STOREDATA_FRAMEWORK_INSTALL_DIR "bin")

    set(STOREDATA_CMAKE_CONFIG_INSTALL_DIR "cmake")
    set(STOREDATA_ADDITIONAL_FILES_INSTALL_DIR "./")
else()
	message(FATAL_ERROR "Could not set install folders for this platform!")
endif()

######################################################################
# Options
option (USE_ZLIB "Use the zlib library" ON)
option (USE_BUILD_AS_LIB "Build as lib (no dll)" OFF)
option (USE_STATIC "Build as static library (/MT)" OFF)

######################################################################
# OpenCV
find_package(OpenCV REQUIRED PATHS ${path_opencv})

######################################################################
# Boost
SET(BOOST_ROOT ${path_boost})
SET(BOOST_LIBRARYDIR ${path_boost_lib})
if(UNIX)
    set(Boost_USE_STATIC_LIBS       OFF) # only find static libs
elseif(WIN32)
    set(Boost_USE_STATIC_LIBS       ON) # only find static libs
else()
    message(FATAL_ERROR "Could not setup boost for this platform!")
endif()
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost REQUIRED COMPONENTS system thread regex filesystem date_time serialization)
message("Boost_LIBRARY_DIR: " ${Boost_LIBRARIES})

######################################################################
# Add Third Library

if (USE_ZLIB)

if (CMAKE_VERSION VERSION_LESS 3.2)
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "")
else()
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
endif()

# 3rdparty Libraries
include(DownloadProject.cmake)
download_project(PROJ              zlib
                 URL      https://zlib.net/zlib1211.zip
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE}
)

add_subdirectory(${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})

# When using CMake 2.8.11 or later, header path dependencies
# are automatically added to the gtest and gmock targets.
# For earlier CMake versions, we have to explicitly add the
# required directories to the header search path ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${zlib_SOURCE_DIR}/include")
endif()


set (USE_LIB_ZLIB 1)
SET( PROJ_INCLUDES_ZLIB  
    "${CMAKE_BINARY_DIR}/zlib-src"
    "${CMAKE_BINARY_DIR}/zlib-build"
)
if (MSVC)
SET( PROJ_LIBRARIES_ZLIB
    debug "${CMAKE_BINARY_DIR}/zlib-build/Debug/zlibd.lib"
    optimized "${CMAKE_BINARY_DIR}/zlib-build/Release/zlib.lib"
)
endif (MSVC)
if (UNIX)
SET( PROJ_LIBRARIES_ZLIB
    debug "${CMAKE_BINARY_DIR}/zlib-build/libz.so"
    optimized "${CMAKE_BINARY_DIR}/zlib-build/libz.so"
)
endif (UNIX)

else()
set (USE_LIB_ZLIB 0)
endif (USE_ZLIB)

######################################################################
# Add Common Library

SET( PROJ_INCLUDES  
    "${CMAKE_SOURCE_DIR}/module"
    "${PROJ_INCLUDES_ZLIB}"
)

SET( PROJ_LIBRARIES
    "${PROJ_LIBRARIES_ZLIB}"
)

######################################################################
# SET VARIABLES

# Set the variable for the EXPORT based on the mode (static/shared)
if(USE_BUILD_AS_LIB)
set (STOREDATA_USELIB 1)
else()
set (STOREDATA_USELIB 0)
endif(USE_BUILD_AS_LIB)


######################################################################
# Definitions

# Windows' math include does not define constants by default.
# Set this definition so it does.
# Also set NOMINMAX so the min and max functions are not overwritten with macros.
IF(MSVC)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)
    add_definitions(-DUNICODE -D_UNICODE)
ENDIF()

######################################################################
# DEFINITIONS
add_definitions(-DSTOREDATADLL)

# Set the build mode (static/shared)
if(USE_BUILD_AS_LIB)
set(BUILD_MODE STATIC)
else()
set(BUILD_MODE SHARED)
endif(USE_BUILD_AS_LIB)

#Add library and link
add_library(${MAIN_PROJ_NAME} ${BUILD_MODE} "StoreData/empty_cmake.cpp;StoreData/StoreData.cpp")
target_link_libraries(${MAIN_PROJ_NAME} ${PROJ_LIBRARIES} ${Boost_LIBRARIES} ${OpenCV_LIBRARIES})
#Add include
#Possible error: https://cmake.org/cmake/help/v3.2/manual/cmake-packages.7.html#creating-packages
target_include_directories(${MAIN_PROJ_NAME} INTERFACE
  # Ok, relocatable:
  $<INSTALL_INTERFACE:module> 
  ${Boost_INCLUDE_DIR} ${PROJ_OPENCV_INCLUDES}
)
# Add dependency to ZLIB (if included in the project)
if (USE_ZLIB)
target_include_directories(${MAIN_PROJ_NAME} INTERFACE $<INSTALL_INTERFACE:include>)
ADD_DEPENDENCIES(${MAIN_PROJ_NAME} zlib zlibstatic)
endif()

#Install Library
install(TARGETS ${MAIN_PROJ_NAME} EXPORT ${MAIN_PROJ_NAME}Targets
        RUNTIME       DESTINATION ${STOREDATA_RUNTIME_INSTALL_DIR}
        LIBRARY       DESTINATION ${STOREDATA_LIBRARY_INSTALL_DIR}
        ARCHIVE       DESTINATION ${STOREDATA_ARCHIVE_INSTALL_DIR}
        FRAMEWORK     DESTINATION ${STOREDATA_FRAMEWORK_INSTALL_DIR})

######################################################################
# Add definitions
IF(MSVC)
	ADD_DEFINITIONS(/arch:SSE2)
	ADD_DEFINITIONS(-D__SSE2__)
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
ENDIF(MSVC)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()    
    
######################################################################
# Recurse into the subdirectories. 
add_subdirectory (module)
add_subdirectory (sample)

######################################################################
# install readme and license
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" "${CMAKE_CURRENT_SOURCE_DIR}/README.md" DESTINATION ${STOREDATA_ADDITIONAL_FILES_INSTALL_DIR})
# install headers
install(DIRECTORY "${CMAKE_SOURCE_DIR}/module" DESTINATION ${STOREDATA_ADDITIONAL_FILES_INSTALL_DIR}
          FILES_MATCHING PATTERN "*.hpp")
# install main headers
install(DIRECTORY "${CMAKE_SOURCE_DIR}/StoreData" DESTINATION ${STOREDATA_ADDITIONAL_FILES_INSTALL_DIR}
          FILES_MATCHING PATTERN "*.hpp")

set_target_properties(${MAIN_PROJ_NAME} PROPERTIES
                                       OUTPUT_NAME "${MAIN_PROJ_NAME}-${STOREDATA_VERSION}")
# Write/install version file
include(CMakePackageConfigHelpers)
set(version_file "${CMAKE_CURRENT_BINARY_DIR}/cmake/${MAIN_PROJ_NAME}-version.cmake")
write_basic_package_version_file(${version_file}
                                 VERSION ${STOREDATA_VERSION}
                                 COMPATIBILITY AnyNewerVersion)


#add files (if necessary)								 
install(EXPORT ${MAIN_PROJ_NAME}Targets DESTINATION ${STOREDATA_CMAKE_CONFIG_INSTALL_DIR}
		                               FILE ${MAIN_PROJ_NAME}-config.cmake
									   NAMESPACE ${MAIN_PROJ_NAME}::)
install(FILES ${version_file} DESTINATION ${STOREDATA_CMAKE_CONFIG_INSTALL_DIR})
