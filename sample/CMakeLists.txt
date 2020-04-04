#######################################################################
macro(CREATE_EXAMPLE NAME SOURCES LIBRARIES)
	set(srcs)
	set(flsrcs)
	foreach(src ${SOURCES})
		if("${src}" MATCHES ".fl$")
         list(APPEND flsrcs ${src})
		else()
			list(APPEND srcs ${src})
		endif("${src}" MATCHES ".fl$")
	endforeach(src)

######################################################################
# Add Internal Library

	add_executable(${NAME} ${srcs})
		include_directories(${PROJ_INCLUDES} ${Boost_INCLUDE_DIR} ${PROJ_OPENCV_INCLUDES}
        )
	target_link_libraries(${NAME} ${LIBRARIES} ${PROJ_LIBRARIES} ${Boost_LIBRARIES} ${OpenCV_LIBRARIES}
	)

	# ------------------------
	# DEFINITIONS
	# ------------------------

	add_definitions(-DUNICODE -D_UNICODE)

    install(TARGETS ${NAME}
      DESTINATION ${CMAKE_INSTALL_PREFIX}/examples
    )

    IF(MSVC)
	ADD_DEFINITIONS(/arch:SSE2)
	ADD_DEFINITIONS(-D__SSE2__)
    ENDIF(MSVC)

endmacro(CREATE_EXAMPLE NAME SOURCES LIBRARIES)

#######################################################################
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  "${CMAKE_SOURCE_DIR}/module" 
)

#######################################################################
CREATE_EXAMPLE(sample_buffering "sample_buffering" "buffer;logger;record")
CREATE_EXAMPLE(sample_codify "sample_codify" "codify")
CREATE_EXAMPLE(sample_codify_codifydata_obsolete "sample_codify_codifydata_obsolete" "codify")
CREATE_EXAMPLE(sample_codify_packunpackimages "sample_codify_packunpackimages" "codify")
CREATE_EXAMPLE(sample_logger "sample_logger" "logger")
CREATE_EXAMPLE(sample_record_saveplayfile "sample_record_saveplayfile" "record;codify")
CREATE_EXAMPLE(sample_record_saveplayfile_dat "sample_record_saveplayfile_dat" "record;codify")
CREATE_EXAMPLE(sample_rawrecord "sample_rawrecord" "record;codify")
CREATE_EXAMPLE(sample_rawrecord_serialized "sample_rawrecord_serialized" "record;codify")
CREATE_EXAMPLE(sample_record_saveplayfile_avi "sample_record_saveplayfile_avi" "record;codify")
CREATE_EXAMPLE(sample_record_saveplayfile_avi_v2 "sample_record_saveplayfile_avi_v2" "record;codify;video;StoreData")
CREATE_EXAMPLE(sample_record_container "sample_record_container" "record")
CREATE_EXAMPLE(sample_record_container_video "sample_record_container_video" "record")
CREATE_EXAMPLE(sample_record_container_generic "sample_record_container_generic" "buffer;record")
CREATE_EXAMPLE(sample_volatile_timed_buffer "sample_volatile_timed_buffer" "buffer;record")


#######################################################################
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