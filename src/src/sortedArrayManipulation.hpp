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
 * @file sortedArrayManipulator.hpp
 * This file contains common sorted array manipulation routines.
 * Aside from the functions that take a callback every function assumes that
 * the array has a capacity large enough for the given operation.
 */

#ifndef SORTEDARRAYMANIPULATION_HPP_
#define SORTEDARRAYMANIPULATION_HPP_

#include "arrayManipulation.hpp"
#include "sortedArrayManipulationHelpers.hpp"
#include "Logger.hpp"
#include <cstdlib>      // for malloc / free.

namespace sortedArrayManipulation
{

/**
 * @class CallBacks
 * @brief This class provides a no-op callbacks interface for the ArrayManipulation sorted routines.
 */
template <class T, class INDEX_TYPE>
class CallBacks
{
public:

    /**
     * @brief Callback signaling that the size of the array has increased.
     * @param [in] nToAdd the increase in the size of the array.
     * @return a pointer to the array.
     */
    LVARRAY_HOST_DEVICE inline
    T * incrementSize( INDEX_TYPE const nToAdd ) restrict_this
    { return nullptr; }

    /**
     * @brief Callback signaling that a value was inserted at the given position.
     * @param [in] pos the position the value was inserted at.
     */
    LVARRAY_HOST_DEVICE inline
    void insert( INDEX_TYPE const pos ) restrict_this
    { (void) pos; }

    /**
     * @brief Callback signaling that the entry of the array at the first position was set to the value
     *        at the second position.
     * @param [in] pos the position in the array that was set.
     * @param [in] valuePos the position of the value that the entry in the array was set to.
     */
    LVARRAY_HOST_DEVICE inline
    void set( INDEX_TYPE const pos, INDEX_TYPE const valuePos ) restrict_this
    { (void) pos, (void) valuePos; }

    /**
     * @brief Callback signaling that the that the given value was inserted at the given position.
     *        Further information is provided in order to make the insertion efficient.
     * @param [in] nLeftToInsert the number of insertions that occur after this one.
     * @param [in] valuePos the position of the value that was inserted.
     * @param [in] pos the position in the array the value was inserted at.
     * @param [in] prevPos the position the previous value was inserted at or the size of the array
     *             if it is the first insertion.
     */
    LVARRAY_HOST_DEVICE inline
    void insert( INDEX_TYPE const nLeftToInsert, INDEX_TYPE const valuePos,
                 INDEX_TYPE const pos, INDEX_TYPE const prevPos ) restrict_this
    { (void) nLeftToInsert, (void) valuePos, (void) pos, void (prevPos); }

    /**
     * @brief Callback signaling that an entry was removed from the array at given position.
     * @param [in] pos the position of the entry that was removed.
     */
    LVARRAY_HOST_DEVICE inline
    void remove( INDEX_TYPE const pos ) restrict_this
    { (void) pos; }

    /**
     * @brief Callback signaling that the given entry was removed from the given position. Further information
     *        is provided in order to make the removal efficient.
     * @param [in] nRemoved the number of entries removed, starts at 1.
     * @param [in] curPos the position in the array the entry was removed at.
     * @param [in] nextPos the position the next entry will be removed at or the original size of the array
     *             if this was the last entry removed.
     */
    LVARRAY_HOST_DEVICE inline
    void remove( INDEX_TYPE const nRemoved, INDEX_TYPE const curPos, INDEX_TYPE const nextPos ) restrict_this
    { (void) nRemoved, (void) curPos, (void) nextPos; }
};

/**
 * @tparam T the type of the values to compare.
 * @class less
 * @brief This class operates as functor similar to std::less.
 */
template <class T>
struct less
{
  /**
   * @brief Return true iff lhs < rhs.
   */
  DISABLE_HD_WARNING
  CONSTEXPRFUNC LVARRAY_HOST_DEVICE inline bool operator() (T const & lhs, T const & rhs) const restrict_this
  { return lhs < rhs; }
};

/**
 * @tparam T the type of the values to compare.
 * @class less
 * @brief This class operates as functor similar to std::greater.
 */
template <class T>
struct greater
{
  /**
   * @brief Return true iff lhs > rhs.
   */
  DISABLE_HD_WARNING
  CONSTEXPRFUNC LVARRAY_HOST_DEVICE inline bool operator() (T const & lhs, T const & rhs) const restrict_this
  { return lhs > rhs; }
};

/**
 * @tparam RandomAccessIterator an iterator type that provides random access.
 * @tparam Compare the type of the comparison function.
 * @brief Sort the given values in place using the given comparator.
 * @param [in/out] first a RandomAccessIterator to the beginning of the values to sort.
 * @param [in/out] last a RandomAccessIterator to the end of the values to sort.
 * @param [in/out] comp a function that does the comparison between two objects.
 *
 * @note should be equivalent to std::sort(first, last, comp).
 */
DISABLE_HD_WARNING
template<class RandomAccessIterator, class Compare>
LVARRAY_HOST_DEVICE inline void makeSorted(RandomAccessIterator first, RandomAccessIterator last, Compare comp)
{
  if (last - first > internal::INTROSORT_THRESHOLD)
  {
    internal::introsortLoop(first, last, comp);
  }

  internal::insertionSort(first, last - first, comp);
}

/**
 * @tparam RandomAccessIterator an iterator type that provides random access.
 * @brief Sort the given values in place from least to greatest.
 * @param [in/out] first a RandomAccessIterator to the beginning of the values to sort.
 * @param [in/out] last a RandomAccessIterator to the end of the values to sort.
 *
 * @note should be equivalent to std::sort(first, last).
 */
DISABLE_HD_WARNING
template<class RandomAccessIterator>
LVARRAY_HOST_DEVICE inline void makeSorted(RandomAccessIterator first, RandomAccessIterator last)
{ return makeSorted(first, last, less<typename std::remove_reference<decltype(*first)>::type>()); }

/**
 * @tparam RandomAccessIteratorA an iterator type that provides random access.
 * @tparam RandomAccessIteratorB an iterator type that provides random access.
 * @tparam Compare the type of the comparison function.
 * @brief Sort the given values in place using the given comparator and perform the same operations
 *        on the data array thus preserving the mapping between values[i] and data[i].
 * @param [in/out] valueFirst a RandomAccessIterator to the beginning of the values to sort.
 * @param [in/out] valueLast a RandomAccessIterator to the end of the values to sort.
 * @param [in/out] dataFirst a RandomAccessIterator to the beginning of the data.
 * @param [in/out] comp a function that does the comparison between two objects.
 */
DISABLE_HD_WARNING
template<class RandomAccessIteratorA, class RandomAccessIteratorB, class Compare>
LVARRAY_HOST_DEVICE inline void dualSort(RandomAccessIteratorA valueFirst, RandomAccessIteratorA valueLast,
                                         RandomAccessIteratorB dataFirst, Compare comp)
{
  std::ptrdiff_t const size = valueLast - valueFirst;
  internal::DualIterator<RandomAccessIteratorA, RandomAccessIteratorB> dualIter(valueFirst, dataFirst);
  return makeSorted(dualIter, dualIter + size, dualIter.createComparator(comp));
}

/**
 * @tparam RandomAccessIteratorA an iterator type that provides random access.
 * @tparam RandomAccessIteratorB an iterator type that provides random access.
 * @brief Sort the given values in place from least to greatest and perform the same operations
 *        on the data array thus preserving the mapping between values[i] and data[i].
 * @param [in/out] valueFirst a RandomAccessIterator to the beginning of the values to sort.
 * @param [in/out] valueLast a RandomAccessIterator to the end of the values to sort.
 * @param [in/out] dataFirst a RandomAccessIterator to the beginning of the data.
 */
DISABLE_HD_WARNING
template<class RandomAccessIteratorA, class RandomAccessIteratorB>
LVARRAY_HOST_DEVICE inline void dualSort(RandomAccessIteratorA valueFirst, RandomAccessIteratorA valueLast,
                                         RandomAccessIteratorB dataFirst)
{ return dualSort(valueFirst, valueLast, dataFirst, less<typename std::remove_reference<decltype(*valueFirst)>::type>()); }

/**
 * @tparam T the type of the values stored in the buffer.
 * @tparam N the size of the local buffer.
 * @brief Create a copy of the values array in localBuffer if possible. If not an allocation is
 *        made and the values are copied into that.
 * @param [in] values pointer to the values to copy.
 * @param [in] nVals the number of values to copy.
 * @param [in/out] localBuffer a reference to an array T[N].
 * @return A pointer to the buffer containing a copy of the values array.
 *
 * @note freeTemporaryBuffer must be called with the pointer returned from this method, nVals, and localBuffer to
 *       deallocate the created buffer.
 */
DISABLE_HD_WARNING
template <class T, int N>
LVARRAY_HOST_DEVICE inline T * createTemporaryBuffer(T const * const values, std::ptrdiff_t const nVals,
                                                     T (&localBuffer)[N])
{
  T * buffer = localBuffer;
  if (nVals <= N)
  {
    for (std::ptrdiff_t i = 0; i < nVals; ++i)
    {
      localBuffer[i] = values[i];
    }
  }
  else
  {
    buffer = static_cast<T*>(std::malloc(sizeof(T) * nVals));

    for (std::ptrdiff_t i = 0; i < nVals; ++i)
    {
      new (buffer + i) T(values[i]);
    }
  }

  return buffer;
}

/**
 * @tparam T the type of the values stored in the buffer.
 * @tparam N the size of the local buffer.
 * @brief Deallocate a buffer created from createTemporaryBuffer.
 * @param [in] buffer the buffer returned by createTemporaryBuffer.
 * @param [in] nVals the number of values in the buffer.
 * @param [in] localBuffer a reference to an array T[N].
 */
DISABLE_HD_WARNING
template <class T, int N>
LVARRAY_HOST_DEVICE inline void freeTemporaryBuffer(T * const buffer, std::ptrdiff_t const nVals,
                                                    T const (&localBuffer)[N])
{
  if (buffer == localBuffer)
  {
    return;
  }

  for (std::ptrdiff_t i = 0; i < nVals; ++i)
  {
    buffer[i].~T();
  }
  std::free(buffer);
}

/**
 * @tparam T the type of values in the array.
 * @tparam INDEX_TYPE the integer type used to index into the array.
 * @tparam Compare the type of the comparison function, defaults to less<T>.
 * @brief Return true if the given array is sorted using comp.
 * @param [in] ptr pointer to the array.
 * @param [in] size the size of the array.
 * @param [in/out] the comparison method to use.
 *
 * @note Should be equivalent to std::is_sorted(ptr, ptr + size, comp).
 */
DISABLE_HD_WARNING
template <class T, class INDEX_TYPE, class Compare=less<T>>
LVARRAY_HOST_DEVICE inline
INDEX_TYPE isSorted( T const * const ptr, INDEX_TYPE const size, Compare comp=Compare() )
{
  GEOS_ASSERT( ptr != nullptr || size == 0 );
  GEOS_ASSERT( arrayManipulation::isPositive( size ) );

  for( INDEX_TYPE i = 0 ; i < size - 1 ; ++i )
  {
    if( comp(ptr[i + 1], ptr[i]) ) 
    {
      return false;
    }
  }

  return true;
}

/**
 * @tparam T the type of values in the array.
 * @tparam INDEX_TYPE the integer type used to index into the array.
 * @tparam Compare the type of the comparison function, defaults to less<T>.
 * @brief Return the index of the first value in the array greater than or equal to value.
 * @param [in] ptr pointer to the array, must be sorted under comp.
 * @param [in] size the size of the array.
 * @param [in] value the value to find.
 * @param [in/out] comp the comparison method to use.
 *
 * @note Should be equivalent to std::lower_bound(ptr, ptr + size, value, comp).
 */
DISABLE_HD_WARNING
template <class T, class INDEX_TYPE, class Compare=less<T>>
LVARRAY_HOST_DEVICE inline
INDEX_TYPE find( T const * const ptr, INDEX_TYPE const size, T const & value, Compare comp=Compare() )
{
  GEOS_ASSERT( ptr != nullptr || size == 0 );
  GEOS_ASSERT( arrayManipulation::isPositive( size ) );
  GEOS_ASSERT( isSorted( ptr, size, comp ) );

  INDEX_TYPE lower = 0;
  INDEX_TYPE upper = size;
  while( lower != upper )
  {
    INDEX_TYPE const guess = (lower + upper) / 2;
    if (comp(ptr[guess], value))
    {
      lower = guess + 1;
    }
    else
    {
      upper = guess;
    }
  }

  return lower;
}

/**
 * @tparam T the type of values in the array.
 * @tparam INDEX_TYPE the integer type used to index into the array.
 * @tparam Compare the type of the comparison function, defaults to less<T>.
 * @brief Return true if the array contains value.
 * @param [in] ptr pointer to the array, must be sorted under comp.
 * @param [in] size the size of the array.
 * @param [in] value the value to find.
 * @param [in/out] comp the comparison method to use.
 *
 * @note Should be equivalent to std::binary_search(ptr, ptr + size, value, comp).
 */
DISABLE_HD_WARNING
template <class T, class INDEX_TYPE, class Compare=less<T>>
LVARRAY_HOST_DEVICE inline
bool contains( T const * const ptr, INDEX_TYPE const size, T const & value, Compare comp=Compare() )
{
  INDEX_TYPE const pos = find( ptr, size, value, comp );
  return (pos != size) && (ptr[pos] == value);
}

/**
 * @tparam T the type of values in the array.
 * @tparam INDEX_TYPE the integer type used to index into the array.
 * @tparam CALLBACKS the type of the callBacks class.
 * @brief Remove the given value from the array if it exists.
 * @param [in] ptr pointer to the array, must be sorted under less<T>.
 * @param [in] size the size of the array.
 * @param [in] value the value to remove.
 * @param [in/out] callBacks class which must define at least a method remove(INDEX_TYPE).
 *                 If the value is found it is removed then this method is called with the index the value
 *                 was found at.
 * @return True iff the value was removed.
 */
DISABLE_HD_WARNING
template <class T, class INDEX_TYPE, class CALLBACKS>
LVARRAY_HOST_DEVICE inline
bool remove( T * const ptr, INDEX_TYPE const size, T const & value, CALLBACKS && callBacks )
{
  GEOS_ASSERT( ptr != nullptr || size == 0 );
  GEOS_ASSERT( arrayManipulation::isPositive( size ));

  // Find the position of value.
  INDEX_TYPE const index = find( ptr, size, value );

  // If it's not in the array we can return.
  if( index == size || ptr[index] != value )
  {
    return false;
  }

  // Remove the value and call the callback.
  arrayManipulation::erase( ptr, size, index );
  callBacks.remove( index );
  return true;
}

/**
 * @tparam T the type of values in the array.
 * @tparam INDEX_TYPE the integer type used to index into the array.
 * @tparam CALLBACKS the type of the callBacks class.
 * @brief Remove the given values from the array if they exist.
 * @param [in] ptr pointer to the array, must be sorted under less<T>.
 * @param [in] size the size of the array.
 * @param [in] values the values to remove, must be sorted under less<T>.
 * @param [in] nVals the number of values to remove.
 * @param [in/out] callBacks class which must define at least a method remove(INDEX_TYPE, INDEX_TYPE, INDEX_TYPE).
 *                 If a value is found it is removed then this method is called with the number of values removed so far,
 *                 the position of the last value removed and the position of the next value to remove or size if there are no more
 *                 values to remove.
 * @return The number of values removed.
 */
DISABLE_HD_WARNING
template <class T, class INDEX_TYPE, class CALLBACKS>
LVARRAY_HOST_DEVICE inline
INDEX_TYPE removeSorted( T * const ptr, INDEX_TYPE const size, T const * const values,
                         INDEX_TYPE const nVals, CALLBACKS && callBacks )
{
  GEOS_ASSERT( ptr != nullptr || size == 0 );
  GEOS_ASSERT( arrayManipulation::isPositive( size ));
  GEOS_ASSERT( values != nullptr || nVals == 0 );
  GEOS_ASSERT( arrayManipulation::isPositive( nVals ));
  GEOS_ASSERT( isSorted( values, nVals ));

  // If there are no values to remove we can return.
  if( nVals == 0 )
    return 0;

  // Find the position of the first value to remove and the position it's at in the array.
  INDEX_TYPE firstValuePos = nVals;
  INDEX_TYPE curPos = size;
  for( INDEX_TYPE i = 0 ; i < nVals ; ++i )
  {
    curPos = find( ptr, size, values[i] );

    // If the current value is larger than the largest value in the array we can return.
    if( curPos == size )
      return 0;

    if( ptr[curPos] == values[i] )
    {
      firstValuePos = i;
      break;
    }
  }

  // If we didn't find a value to remove we can return.
  if( firstValuePos == nVals )
    return 0;

  // Loop over the values
  INDEX_TYPE nRemoved = 0;
  for( INDEX_TYPE curValuePos = firstValuePos ; curValuePos < nVals ; )
  {
    // Find the next value to remove
    INDEX_TYPE nextValuePos = nVals;
    INDEX_TYPE nextPos = size;
    for( INDEX_TYPE j = curValuePos + 1 ; j < nVals ; ++j )
    {
      // Skip over duplicate values
      if( values[j] == values[j - 1] )
        continue;

      // Find the position
      INDEX_TYPE const pos = find( ptr + curPos, size - curPos, values[j] ) + curPos;

      // If it's not in the array then neither are any of the rest of the values.
      if( pos == size )
        break;

      // If it's in the array then we can exit this loop.
      if( ptr[pos] == values[j] )
      {
        nextValuePos = j;
        nextPos = pos;
        break;
      }
    }

    // Shift the values down and call the callback.
    nRemoved += 1;
    arrayManipulation::shiftDown( ptr, nextPos, curPos + 1, nRemoved );
    callBacks.remove( nRemoved, curPos, nextPos );

    curValuePos = nextValuePos;
    curPos = nextPos;

    // If we've reached the end of the array we can exit the loop.
    if( curPos == size )
      break;
  }

  // Destroy the values moved out of at the end of the array.
  for( INDEX_TYPE i = size - nRemoved ; i < size ; ++i )
  {
    ptr[i].~T();
  }

  return nRemoved;
}

/**
 * @tparam T the type of values in the array.
 * @tparam INDEX_TYPE the integer type used to index into the array.
 * @tparam CALLBACKS the type of the callBacks class.
 * @brief Remove the given values from the array if they exist.
 * @param [in] ptr pointer to the array, must be sorted under less<T>.
 * @param [in] size the size of the array.
 * @param [in] values the values to remove.
 * @param [in] nVals the number of values to remove.
 * @param [in/out] callBacks class which must define at least a method remove(INDEX_TYPE, INDEX_TYPE, INDEX_TYPE).
 *                 If a value is found it is removed then this method is called with the number of values removed so far,
 *                 the position of the last value removed and the position of the next value to remove or size if there are no more
 *                 values to remove.
 * @return The number of values removed.
 */
DISABLE_HD_WARNING
template <class T, class INDEX_TYPE, class CALLBACKS>
LVARRAY_HOST_DEVICE inline
INDEX_TYPE remove( T * const ptr, INDEX_TYPE const size, T const * const values,
                   INDEX_TYPE const nVals, CALLBACKS && callBacks )
{
  GEOS_ASSERT( ptr != nullptr || size == 0 );
  GEOS_ASSERT( arrayManipulation::isPositive( size ));
  GEOS_ASSERT( values != nullptr || nVals == 0 );
  GEOS_ASSERT( arrayManipulation::isPositive( nVals ));

  constexpr INDEX_TYPE LOCAL_SIZE = 16;
  T localBuffer[LOCAL_SIZE];
  T * const buffer = createTemporaryBuffer(values, nVals, localBuffer);
  makeSorted(buffer, buffer + nVals);

  INDEX_TYPE const nInserted = removeSorted(ptr, size, buffer, nVals, std::move(callBacks));
    
  freeTemporaryBuffer(buffer, nVals, localBuffer);
  return nInserted;
}

/**
 * @tparam T the type of values in the array.
 * @tparam INDEX_TYPE the integer type used to index into the array.
 * @tparam CALLBACKS the type of the callBacks class.
 * @brief Insert the given value into the array if it doesn't already exist.
 * @param [in] ptr pointer to the array, must be sorted under less<T>.
 * @param [in] size the size of the array.
 * @param [in] value the value to insert.
 * @param [in/out] callBacks class which must define at least a method T * incrementSize(INDEX_TYPE)
 *                 and insert(INDEX_TYPE). incrementSize is called with the number of values to insert and returns a new
 *                 pointer to the array. If an insert has occurred insert is called with the position in the array
 *                 at which the insert took place.
 * @return True iff the value was inserted.
 */
DISABLE_HD_WARNING
template <class T, class INDEX_TYPE, class CALLBACKS>
LVARRAY_HOST_DEVICE inline
bool insert( T const * const ptr, INDEX_TYPE const size, T const & value, CALLBACKS && callBacks )
{
  GEOS_ASSERT( ptr != nullptr || size == 0 );
  GEOS_ASSERT( arrayManipulation::isPositive( size ));

  INDEX_TYPE insertPos = INDEX_TYPE(-1);
  if (size == 0 || value < ptr[0]) // Take care of the case of an empty array or inserting at the beginning.
  {
    insertPos = 0;
  }
  else if (ptr[size - 1] < value) // The case of inserting at the end.
  {
    insertPos = size;
  }
  else // otherwise do a binary search
  {
    // We've already checked the first and last values.
    insertPos = find( ptr, size, value );
  }

  // If it's in the array we call the incrementSize callback and return false.
  if( insertPos != size && ptr[insertPos] == value )
  {
    callBacks.incrementSize( 0 );
    return false;
  }

  // Otherwise call the incrementSize callback, get the new pointer, insert and call the insert callback.
  T * const newPtr = callBacks.incrementSize( 1 );
  arrayManipulation::insert( newPtr, size, insertPos, value );
  callBacks.insert( insertPos );
  return true;
}

/**
 * @tparam T the type of values in the array.
 * @tparam INDEX_TYPE the integer type used to index into the array.
 * @tparam CALLBACKS the type of the callBacks class.
 * @brief Insert the given values into the array if they don't already exist.
 * @param [in] ptr pointer to the array, must be sorted under less<T>.
 * @param [in] size the size of the array.
 * @param [in] values the values to insert, must be sorted under less<T>.
 * @param [in] nVals the number of values to insert.
 * @param [in/out] callBacks class which must define at least a method T * incrementSize(INDEX_TYPE),
 *                 set(INDEX_TYPE, INDEX_TYPE) and insert(INDEX_TYPE, INDEX_TYPE, INDEX_TYPE, INDEX_TYPE).
 *                 incrementSize is called with the number of values to insert and returns a new pointer to the array.
 *                 After an insert has occurred insert is called with the number of values left to insert, the
 *                 index of the value inserted, the index at which it was inserted and the index at
 *                 which the the previous value was inserted or size if it is the first value inserted.
 *                 set is called when the array is empty and it takes a position in the array and the position of the
 *                 value that will occupy that position.
 * @return The number of values inserted.
 */
DISABLE_HD_WARNING
template <class T, class INDEX_TYPE, class CALLBACKS>
LVARRAY_HOST_DEVICE inline
INDEX_TYPE insertSorted( T const * const ptr, INDEX_TYPE const size, T const * const values,
                         INDEX_TYPE const nVals, CALLBACKS && callBacks )
{
  GEOS_ASSERT( ptr != nullptr || size == 0 );
  GEOS_ASSERT( arrayManipulation::isPositive( size ));
  GEOS_ASSERT( values != nullptr || nVals == 0 );
  GEOS_ASSERT( nVals >= 0 );
  GEOS_ASSERT( isSorted( values, nVals ));

  // If there are no values to insert.
  if( nVals == 0 )
  {
    callBacks.incrementSize( 0 );
    return 0;
  }

  INDEX_TYPE nToInsert = 0; // The number of values that will actually be inserted.

  // Special case for inserting into an empty array.
  if( size == 0 )
  {
    // Count up the number of unique values.
    nToInsert = 1;
    for( INDEX_TYPE i = 1 ; i < nVals ; ++i )
    {
      if( values[i] != values[i - 1] )
        ++nToInsert;
    }

    // Call callBack with the number to insert and get a new pointer back.
    T * const newPtr = callBacks.incrementSize( nToInsert );

    // Insert the first value.
    new (&newPtr[0]) T( values[0] );
    callBacks.set( 0, 0 );

    // Insert the remaining values, checking for duplicates.
    INDEX_TYPE curInsertPos = 1;
    for( INDEX_TYPE i = 1 ; i < nVals ; ++i )
    {
      if( values[i] != values[i - 1] )
      {
        new (&newPtr[curInsertPos]) T( values[i] );
        callBacks.set( curInsertPos, i );
        ++curInsertPos;
      }
    }

    return nToInsert;
  }

  // We need to count up the number of values that will actually be inserted.
  // Doing so involves finding the position the value will be inserted at.
  // Below we store the first MAX_PRE_CALCULATED positions so that we don't need
  // to find them again.

  constexpr int MAX_PRE_CALCULATED = 32;
  INDEX_TYPE valuePositions[MAX_PRE_CALCULATED];  // Positions of the first 32 values to insert.
  INDEX_TYPE insertPositions[MAX_PRE_CALCULATED]; // Position at which to insert the first 32 values.

  // Loop over the values in reverse (from largest to smallest).
  INDEX_TYPE curPos = size;
  for( INDEX_TYPE i = nVals - 1 ; i >= 0 ; --i )
  {
    // Skip duplicate values.
    if( i != 0 && values[i] == values[i - 1] )
      continue;

    curPos = find( ptr, curPos, values[i] );

    // If values[i] isn't in the array.
    if( curPos == size || ptr[curPos] != values[i] )
    {
      // Store the value position and insert position if there is space left.
      if( nToInsert < MAX_PRE_CALCULATED )
      {
        valuePositions[nToInsert] = i;
        insertPositions[nToInsert] = curPos;
      }

      nToInsert += 1;
    }
  }

  // Call callBack with the number to insert and get a new pointer back.
  T * const newPtr = callBacks.incrementSize( nToInsert );

  // If there are no values to insert we can return.
  if( nToInsert == 0 )
    return 0;

  //
  INDEX_TYPE const nPreCalculated = (nToInsert < MAX_PRE_CALCULATED) ?
                                    nToInsert : MAX_PRE_CALCULATED;

  // Insert pre-calculated values.
  INDEX_TYPE prevInsertPos = size;
  for( INDEX_TYPE i = 0 ; i < nPreCalculated ; ++i )
  {
    // Shift the values up...
    arrayManipulation::shiftUp( newPtr, prevInsertPos, insertPositions[i], INDEX_TYPE( nToInsert - i ));

    // and insert.
    INDEX_TYPE const curValuePos = valuePositions[i];
    new (&newPtr[insertPositions[i] + nToInsert - i - 1]) T( values[curValuePos] );
    callBacks.insert( nToInsert - i, curValuePos, insertPositions[i], prevInsertPos );

    prevInsertPos = insertPositions[i];
  }

  // If all the values to insert were pre-calculated we can return.
  if( nToInsert <= MAX_PRE_CALCULATED )
    return nToInsert;

  // Insert the rest of the values.
  INDEX_TYPE const prevValuePos = valuePositions[MAX_PRE_CALCULATED - 1];
  INDEX_TYPE nInserted = MAX_PRE_CALCULATED;
  for( INDEX_TYPE i = prevValuePos - 1 ; i >= 0 ; --i )
  {
    // Skip duplicates
    if( values[i] == values[i + 1] )
      continue;

    INDEX_TYPE const pos = find( newPtr, prevInsertPos, values[i] );

    // If values[i] is already in the array skip it.
    if( pos != prevInsertPos && newPtr[pos] == values[i] )
      continue;

    // Else shift the values up and insert.
    arrayManipulation::shiftUp( newPtr, prevInsertPos, pos, INDEX_TYPE( nToInsert - nInserted ));
    new (&newPtr[pos + nToInsert - nInserted - 1]) T( values[i] );
    callBacks.insert( nToInsert - nInserted, i, pos, prevInsertPos );

    nInserted += 1;
    prevInsertPos = pos;

    // If all the values have been inserted then exit the loop.
    if( nInserted == nToInsert )
      break;
  }

  GEOS_ASSERT_MSG( nInserted == nToInsert, nInserted << " " << nToInsert );

  return nToInsert;
}

/**
 * @tparam T the type of values in the array.
 * @tparam INDEX_TYPE the integer type used to index into the array.
 * @tparam CALLBACKS the type of the callBacks class.
 * @brief Insert the given values into the array if they don't already exist.
 * @param [in] ptr pointer to the array, must be sorted under less<T>.
 * @param [in] size the size of the array.
 * @param [in] values the values to insert.
 * @param [in] nVals the number of values to insert.
 * @param [in/out] callBacks class which must define at least a method T * incrementSize(INDEX_TYPE),
 *                 set(INDEX_TYPE, INDEX_TYPE) and insert(INDEX_TYPE, INDEX_TYPE, INDEX_TYPE, INDEX_TYPE).
 *                 incrementSize is called with the number of values to insert and returns a new pointer to the array.
 *                 After an insert has occurred insert is called with the number of values left to insert, the
 *                 index of the value inserted, the index at which it was inserted and the index at
 *                 which the the previous value was inserted or size if it is the first value inserted.
 *                 set is called when the array is empty and it takes a position in the array and the position of the
 *                 value that will occupy that position.
 * @return The number of values inserted.
 */
DISABLE_HD_WARNING
template <class T, class INDEX_TYPE, class CALLBACKS>
LVARRAY_HOST_DEVICE inline
INDEX_TYPE insert( T const * const ptr, INDEX_TYPE const size, T const * const values,
                   INDEX_TYPE const nVals, CALLBACKS && callBacks )
{
  GEOS_ASSERT( ptr != nullptr || size == 0 );
  GEOS_ASSERT( arrayManipulation::isPositive( size ));
  GEOS_ASSERT( values != nullptr || nVals == 0 );
  GEOS_ASSERT( nVals >= 0 );

  constexpr INDEX_TYPE LOCAL_SIZE = 16;
  T localBuffer[LOCAL_SIZE];
  T * const buffer = createTemporaryBuffer(values, nVals, localBuffer);
  makeSorted(buffer, buffer + nVals);

  INDEX_TYPE const nInserted = insertSorted(ptr, size, buffer, nVals, std::move(callBacks));

  freeTemporaryBuffer(buffer, nVals, localBuffer);
  return nInserted;
}

} // namespace sortedArrayManipulation

#endif // SORTEDARRAYMANIPULATION_HPP_