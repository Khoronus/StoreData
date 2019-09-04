#Setting the output folder
# First for the generic no-config case (e.g. with mingw)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
# Second, for multi-config builds (e.g. msvc)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    message(${OUTPUTCONFIG})
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin/${OUTPUTCONFIG} )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib/${OUTPUTCONFIG} )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib/${OUTPUTCONFIG} )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )


MACRO(HEADER_DIRECTORIES return_list)
    FILE(GLOB_RECURSE new_list *.hpp *.cpp)
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()


SET( PROJ_NAME      "buffer" )
SET( PROJ_PATH      ${CMAKE_SOURCE_DIR} )
SET( PROJ_OUT_PATH  ${CMAKE_BINARY_DIR} )

#Add the files
FILE( GLOB_RECURSE PROJ_SOURCES *.cpp *.cc *.c)
FILE( GLOB_RECURSE PROJ_HEADERS *.hpp *.h)


set(PROJ_RETURNLIST "")
HEADER_DIRECTORIES(PROJ_RETURNLIST)

######################################################################
# DEFINITIONS
add_definitions(-DSTOREDATABUFFERDLL)

PROJECT( ${PROJ_NAME} )

if(USE_BUILD_AS_LIB)
set(BUILD_MODE STATIC)
else()
set(BUILD_MODE SHARED)
endif(USE_BUILD_AS_LIB)


ADD_LIBRARY( ${PROJ_NAME} ${BUILD_MODE} ${PROJ_SOURCES} ${PROJ_HEADERS})
INCLUDE_DIRECTORIES( ${PROJ_INCLUDES} ${Boost_INCLUDE_DIR} ${PROJ_OPENCV_INCLUDES})
TARGET_LINK_LIBRARIES( ${PROJ_NAME} ${PROJ_LIBRARIES} ${Boost_LIBRARIES} ${OpenCV_LIBRARIES})
# Add dependency to ZLIB (if included in the project)
if (USE_ZLIB)
ADD_DEPENDENCIES(${PROJ_NAME} zlib zlibstatic)
endif()

#Use static compiler library or dynamic
if (USE_STATIC)
  set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
  foreach(CompilerFlag ${CompilerFlags})
    string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
  endforeach()
endif (USE_STATIC)

#Install Library
install(TARGETS ${PROJ_NAME} EXPORT ${MAIN_PROJ_NAME}Targets
        RUNTIME       DESTINATION ${STOREDATA_RUNTIME_INSTALL_DIR}
        LIBRARY       DESTINATION ${STOREDATA_LIBRARY_INSTALL_DIR}
        ARCHIVE       DESTINATION ${STOREDATA_ARCHIVE_INSTALL_DIR}
        FRAMEWORK     DESTINATION ${STOREDATA_FRAMEWORK_INSTALL_DIR})