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
 * @file numpyHelpers.hpp
 * @brief Contains methods to help with conversion to python objects.
 */

#pragma once

// Source include
#include "pythonForwardDeclarations.hpp"
#include "pythonHelpers.hpp"
#include "../limits.hpp"

// system includes
#include <vector>
#include <typeindex>
#include <type_traits>

namespace LvArray
{
namespace python
{
namespace internal
{

template< typename T >
constexpr bool canExportToNumpy = std::is_arithmetic< T >::value;

std::nullptr_t exportError( std::string const & typeName );

/**
 *
 */
PyObject * createNumpyArrayImpl( void * const data,
                                 std::type_index const type,
                                 bool const dataIsConst,
                                 int const ndim,
                                 std::ptrdiff_t const * const dims,
                                 std::ptrdiff_t const * const strides );

PyObject * createCupyArrayImpl( void * const data,
                                 std::type_index const type,
                                 bool const dataIsConst,
                                 int const ndim,
                                 std::ptrdiff_t const * const dims,
                                 std::ptrdiff_t const * const strides );

} // namespace internal

bool import_array_wrapper();

/**
 *
 */
template< typename T, typename INDEX_TYPE >
std::enable_if_t< internal::canExportToNumpy< T >, PyObject * >
createNumPyArray( T * const data,
                  bool const modify,
                  int const ndim,
                  INDEX_TYPE const * const dimsPtr,
                  INDEX_TYPE const * const stridesPtr )
{
  std::vector< std::ptrdiff_t > dims( ndim );
  std::vector< std::ptrdiff_t > strides( ndim );

  for ( int i = 0; i < ndim; ++i )
  {
    dims[ i ] = integerConversion< std::ptrdiff_t >( dimsPtr[ i ] );
    strides[ i ] = integerConversion< std::ptrdiff_t >( stridesPtr[ i ] );
  }

  if ( false ){
    return internal::createCupyArrayImpl( const_cast< void * >( static_cast< void const * >( data ) ),
                                           std::type_index( typeid( T ) ),
                                           std::is_const< T >::value || !modify,
                                           ndim,
                                           dims.data(),
                                           strides.data() );
  }
  return internal::createNumpyArrayImpl( const_cast< void * >( static_cast< void const * >( data ) ),
                                         std::type_index( typeid( T ) ),
                                         std::is_const< T >::value || !modify,
                                         ndim,
                                         dims.data(),
                                         strides.data() );
}

template< typename T, typename INDEX_TYPE >
std::enable_if_t< !internal::canExportToNumpy< T >, PyObject * >
createNumPyArray( T * const data,
                  bool const modify,
                  int const ndim,
                  INDEX_TYPE const * const dimsPtr,
                  INDEX_TYPE const * const stridesPtr )
{
  LVARRAY_UNUSED_VARIABLE( data );
  LVARRAY_UNUSED_VARIABLE( modify );
  LVARRAY_UNUSED_VARIABLE( ndim );
  LVARRAY_UNUSED_VARIABLE( dimsPtr );
  LVARRAY_UNUSED_VARIABLE( stridesPtr );
  return internal::exportError( system::demangleType< T >() );
}

/**
 *
 */
template< typename T >
std::enable_if_t< internal::canExportToNumpy< T >, PyObject * >
create( T & value, bool const modify )
{
  std::ptrdiff_t dims = 1;
  std::ptrdiff_t strides = 1;

  return internal::createNumpyArrayImpl( const_cast< void * >( static_cast< void const * >( &value ) ),
                                         std::type_index( typeid( T ) ),
                                         std::is_const< T >::value || !modify,
                                         1,
                                         &dims,
                                         &strides );
}

/**
 *
 */
PyObject * create( std::string const & value, bool const modify );

/**
 *
 */
std::tuple< PyObjectRef< PyObject >, void const *, std::ptrdiff_t >
parseNumPyArray( PyObject * const obj, std::type_index const expectedType );

/**
 *
 */
std::type_index getTypeIndexFromNumPy( int const numpyType );

/**
 *
 */
std::string getNumPyTypeName( int const nympyType );

/**
 *
 */
std::pair< int, std::size_t > getNumPyType( std::type_index const typeIndex );

} // namespace python
} // namespace LvArray
