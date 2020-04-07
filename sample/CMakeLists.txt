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
CREATE_EXAMPLE(sample_rawrecorder "sample_rawrecorder" "record;codify")
CREATE_EXAMPLE(sample_rawrecorder_serialized "sample_rawrecorder_serialized" "record;codify")
CREATE_EXAMPLE(sample_EnhanceAsyncRecorderManager "sample_EnhanceAsyncRecorderManager" "record;codify;video;StoreData")
CREATE_EXAMPLE(sample_record_container_file "sample_record_container_file" "record")
CREATE_EXAMPLE(sample_record_container_video "sample_record_container_video" "buffer;record")
CREATE_EXAMPLE(sample_DataDesynchronizerGeneric "sample_DataDesynchronizerGeneric" "buffer;record")
CREATE_EXAMPLE(sample_DataDesynchronizerGenericFaster "sample_DataDesynchronizerGenericFaster" "buffer;record")
CREATE_EXAMPLE(sample_volatile_timed_buffer "sample_volatile_timed_buffer" "buffer;record")
CREATE_EXAMPLE(sample_create_file "sample_create_file" "record")
CREATE_EXAMPLE(sample_create_video "sample_create_video" "record")
CREATE_EXAMPLE(sample_PlayerRecorder "sample_PlayerRecorder" "record")
CREATE_EXAMPLE(sample_rawrecorder_zip "sample_rawrecorder_zip" "record")

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