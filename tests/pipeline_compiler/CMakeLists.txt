if (TARGET "PipelineCompiler")
	file( GLOB_RECURSE SOURCES "*.*" )

	if (DEFINED ANDROID)
		add_library( "Tests.PipelineCompiler" STATIC ${SOURCES} )
	else ()
		add_executable( "Tests.PipelineCompiler" ${SOURCES} )
	endif ()

	source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} )
	set_property( TARGET "Tests.PipelineCompiler" PROPERTY FOLDER "Tests" )
	target_link_libraries( "Tests.PipelineCompiler" "PipelineCompiler" )
	
	add_test( NAME "Tests.PipelineCompiler" COMMAND "Tests.PipelineCompiler" )
endif ()