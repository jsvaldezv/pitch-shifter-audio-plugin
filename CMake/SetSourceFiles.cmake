# Source Files
macro(setSourceFiles sourceFiles)
    file(GLOB_RECURSE SOURCES
	    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    )       
    set(${sourceFiles} ${SOURCES})
endmacro()