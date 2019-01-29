/*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Copyright (c) 2019, Lawrence Livermore National Security, LLC.
 *
 * Produced at the Lawrence Livermore National Laboratory
 *
 * LLNL-CODE-746361
 *
 * All rights reserved. See COPYRIGHT for details.
 *
 * This file is part of the GEOSX Simulation Framework.
 *
 * GEOSX is a free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License (as published by the
 * Free Software Foundation) version 2.1 dated February 1999.
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

/**
 * @file ArraySlice.hpp
 */

#ifndef ARRAY_SLICE_HPP_
#define ARRAY_SLICE_HPP_

#include <cstring>
#include <vector>
#include <iostream>
#include <utility>
#include "Logger.hpp"
#include "CXX_UtilsConfig.hpp"


#ifdef USE_ARRAY_BOUNDS_CHECK

#undef CONSTEXPRFUNC
#define CONSTEXPRFUNC

#ifdef USE_CUDA

#include <cassert>
#define ARRAY_SLICE_CHECK_BOUNDS( index )                                        \
  assert( index >= 0 && index < m_dims[0] )

#else // USE_CUDA

#define ARRAY_SLICE_CHECK_BOUNDS( index )                                        \
  GEOS_ERROR_IF( index < 0 || index >= m_dims[0], "Array Bounds Check Failed: index=" << index << " m_dims[0]=" << m_dims[0] )

#endif // USE_CUDA

#else // USE_ARRAY_BOUNDS_CHECK

#define ARRAY_SLICE_CHECK_BOUNDS( index )

#endif // USE_ARRAY_BOUNDS_CHECK


namespace LvArray
{


/**
 * @class ArraySlice
 * @brief This class serves to provide a sliced multidimensional interface to the family of LvArray
 *        classes.
 * @tparam T    type of data that is contained by the array
 * @tparam NDIM number of dimensions in array (e.g. NDIM=1->vector, NDIM=2->Matrix, etc. )
 * @tparam INDEX_TYPE the integer to use for indexing the components of the array
 *
 * This class serves as a sliced interface to an array. This is a lightweight class that contains
 * only pointers, and provides operator[] for indexed array access. This class may be aliased to
 * a pointer for NDIM=1 in cases where range checking is off.
 *
 * In general, instantiations of ArraySlice should only result from taking a slice of an an Array or
 * an ArrayView via operator[] which it inherits from ArraySlice.
 *
 * Key features:
 * 1) ArraySlice provides operator[] array accessesor
 * 2) operator[] are all const and may be called in non-mutable lamdas
 * 3) Conversion operator to go from ArraySlice<T> to ArraySlice<T const>
 * 4) Conversion operator to go from ArraySlice<T,1> to T*
 *
 */
template< typename T, int NDIM, typename INDEX_TYPE = std::int_fast32_t >
class ArraySlice
{
public:
  /**
   * @param inputData       pointer to the beginning of the data for this slice of the array
   * @param inputDimensions pointer to the beginning of the dimensions[NDIM] c-array for this slice.
   * @param inputStrides    pointer to the beginning of the strides[NDIM] c-array for this slice
   *
   * Base constructor that takes in raw data pointers as initializers for its members.
   */
  LVARRAY_HOST_DEVICE inline explicit CONSTEXPRFUNC
  ArraySlice( T * const restrict inputData,
              INDEX_TYPE const * const restrict inputDimensions,
              INDEX_TYPE const * const restrict inputStrides ) noexcept:
    m_data( inputData ),
    m_dims( inputDimensions ),
    m_strides( inputStrides )
  {}


  /**
   * User Defined Conversion operator to move from an ArraySlice<T> to ArraySlice<T const>&. This is
   * achieved by applying a reinterpret_cast to the this pointer, which is a safe operation as the
   * only difference between the types is a const specifier.
   * @return a reference to an ArraySlice<T const>
   */
  template< typename U = T >
  LVARRAY_HOST_DEVICE inline CONSTEXPRFUNC operator
  typename std::enable_if< !std::is_const<U>::value,
                           ArraySlice<T const, NDIM, INDEX_TYPE> const & >::type
    () const restrict_this noexcept
  {
    return reinterpret_cast<ArraySlice<T const, NDIM, INDEX_TYPE> const &>(*this);
  }

  /**
   * User defined conversion to convert a ArraySlice<T,1,NDIM> to a raw pointer.
   * @return a copy of m_data.
   */
  template< int U=NDIM >
  LVARRAY_HOST_DEVICE CONSTEXPRFUNC inline operator
  typename std::enable_if< U == 1, T * const restrict >::type () const noexcept restrict_this
  {
    return m_data;
  }


  /**
   * User defined conversion to convert to a reduced dimension array. For example, converting from
   * a 2d array to a 1d array is valid if the last dimension of the 2d array is 1.
   * @return A new ArraySlice<T,NDIM-1>
   */
  template< int U=NDIM >
  LVARRAY_HOST_DEVICE CONSTEXPRFUNC inline explicit
  operator typename std::enable_if< (U>1),
                                    ArraySlice<T, NDIM-1, INDEX_TYPE> >::type () const restrict_this
  {
    GEOS_ASSERT_MSG( m_dims[NDIM-1]==1, "ManagedArray::operator ArraySlice<T,NDIM-1,INDEX_TYPE>" <<
                     " is only valid if last dimension is equal to 1." );
    return ArraySlice<T, NDIM-1, INDEX_TYPE>( m_data, m_dims + 1, m_strides + 1 );
  }

  /**
   * @brief This function provides a square bracket operator for array access.
   * @param index index of the element in array to access
   * @return a reference to the member m_childInterface, which is of type ArraySlice<T,NDIM-1>.
   *
   * This function creates a new ArraySlice<T,NDIM-1,INDEX_TYPE> by passing the member pointer
   * locations +1. Thus, the returned object has m_data pointing to the beginning of the data
   * associated with its sub-array. If used as a set of nested operators (array[][][]) the compiler
   * typically provides the pointer dereference directly and does not create the new ArraySlice
   * objects.
   *
   * @note This declaration is for NDIM>=3 when bounds checking is off, and NDIM!=1 when bounds
   * checking is on.
   */
  template< int U=NDIM >
  LVARRAY_HOST_DEVICE inline CONSTEXPRFUNC typename
#ifdef USE_ARRAY_BOUNDS_CHECK
  std::enable_if< U!=1, ArraySlice<T, NDIM-1, INDEX_TYPE> >::type
#else
  std::enable_if< U >= 3, ArraySlice<T, NDIM-1, INDEX_TYPE> >::type
#endif
  operator[]( INDEX_TYPE const index ) const noexcept restrict_this
  {
    ARRAY_SLICE_CHECK_BOUNDS( index );
    return ArraySlice<T, NDIM-1, INDEX_TYPE>( &(m_data[ index*m_strides[0] ] ), m_dims+1, m_strides+1 );
  }

  /**
   * @brief This function provides a square bracket operator for array access of a 2D array.
   * @param index index of the element in array to access
   * @return a pointer to the data location indicated by the index.
   *
   * This function returns a pointer to the data that represents the slice at the given index.
   *
   * @note This declaration is for NDIM==2 when bounds checking is OFF, as the result of that
   * operation should be a raw array if bounds checking is on.
   */
#ifndef USE_ARRAY_BOUNDS_CHECK
  template< int U=NDIM >
  LVARRAY_HOST_DEVICE inline CONSTEXPRFUNC
  typename std::enable_if< U==2, T * restrict >::type
  operator[]( INDEX_TYPE const index ) const noexcept restrict_this
  {
    return &(m_data[ index*m_strides[0] ]);
  }
#endif

  /**
   * @brief This function provides a square bracket operator for array access for a 1D array.
   * @param index index of the element in array to access
   * @return a reference to the data location indicated by the index.
   *
   * @note This declaration is for NDIM==1 when bounds checking is ON.
   */
  template< int U=NDIM >
  LVARRAY_HOST_DEVICE inline CONSTEXPRFUNC
  typename std::enable_if< U==1, T & >::type
  operator[]( INDEX_TYPE const index ) const noexcept restrict_this
  {
    ARRAY_SLICE_CHECK_BOUNDS( index );
    return m_data[ index ];
  }

  /// deleted default constructor
  ArraySlice() = delete;



protected:
  /// pointer to beginning of data for this array, or sub-array.
  T * const restrict m_data;

  /// pointer to array of length NDIM that contains the lengths of each array dimension
  INDEX_TYPE const * const restrict m_dims;

  /// pointer to array of length NDIM that contains the strides of each array dimension
  INDEX_TYPE const * const restrict m_strides;

};

#ifdef USE_ARRAY_BOUNDS_CHECK

template< typename T, typename INDEX_TYPE >
using ArraySlice1d = ArraySlice<T, 1, INDEX_TYPE> const;

#else

template< typename T, typename INDEX_TYPE = int>
using ArraySlice1d = T * const restrict;

#endif

}

#endif /* SRC_COMPONENTS_CORE_SRC_ARRAY_MULTIDIMENSIONALARRAY_HPP_ */