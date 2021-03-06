set( lvarray_headers
     Array.hpp
     ArrayOfArrays.hpp
     ArrayOfArraysView.hpp
     ArrayOfSets.hpp
     ArrayOfSetsView.hpp
     ArraySlice.hpp
     ArrayView.hpp
     CRSMatrix.hpp
     CRSMatrixView.hpp
     ChaiBuffer.hpp
     Macros.hpp
     MallocBuffer.hpp
     SortedArray.hpp
     SortedArrayView.hpp
     SparsityPattern.hpp
     SparsityPatternView.hpp
     StackBuffer.hpp
     arrayManipulation.hpp
     bufferManipulation.hpp
     fixedSizeSquareMatrixOps.hpp
     fixedSizeSquareMatrixOpsImpl.hpp
     genericTensorOps.hpp
     indexing.hpp
     input.hpp
     limits.hpp
     math.hpp
     output.hpp
     sliceHelpers.hpp
     sortedArrayManipulation.hpp
     sortedArrayManipulationHelpers.hpp
     system.hpp
     tensorOps.hpp
     totalview/tv_data_display.h
     typeManipulation.hpp )

set( lvarray_sources
     system.cpp
     totalview/tv_data_display.c )

blt_add_library( NAME             lvarray
                 SOURCES          ${lvarray_sources}
                 HEADERS          ${lvarray_headers}
                 DEPENDS_ON       ${lvarray_dependencies} )

target_include_directories( lvarray
                            PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
                            $<INSTALL_INTERFACE:include> )

install( TARGETS lvarray
         EXPORT lvarray
         ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib
         RUNTIME DESTINATION lib )

install( EXPORT lvarray
         DESTINATION share/lvarray/cmake/ )

install( DIRECTORY . DESTINATION include/LvArray FILES_MATCHING PATTERN *.hpp )
install( DIRECTORY . DESTINATION include/LvArray FILES_MATCHING PATTERN *.h )

lvarray_add_code_checks( PREFIX lvarray
                         EXCLUDES "blt/*" )
