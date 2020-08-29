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

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "python/PyArray.hpp"
#include "MallocBuffer.hpp"

static LvArray::Array< int, 1, RAJA::PERM_I, std::ptrdiff_t, LvArray::MallocBuffer > array1DOfInts{10};
static LvArray::Array< double, 1, RAJA::PERM_I, int, LvArray::MallocBuffer > array1DOfDoubles{10};
static LvArray::Array< long, 2, RAJA::PERM_IJ, std::ptrdiff_t, LvArray::MallocBuffer > array2DIJOfLongs{10, 10};
static LvArray::Array< float, 2, RAJA::PERM_JI, long long int, LvArray::MallocBuffer > array2DJIOfFloats{10, 10};
static LvArray::Array< double, 4, RAJA::PERM_KILJ, std::ptrdiff_t, LvArray::MallocBuffer > array4DKILJOfDoubles{10, 10, 10, 10};


static PyObject * getArray1DOfInts( PyObject * const self, PyObject * const args )
{
  LVARRAY_UNUSED_VARIABLE( self );

  int modify;
  if( !PyArg_ParseTuple( args, "p", &modify ) )
  { return nullptr; }

  return LvArray::python::create( array1DOfInts, modify );
}

static PyObject * getArray1DOfDoubles( PyObject * const self, PyObject * const args )
{
  LVARRAY_UNUSED_VARIABLE( self );

  int modify;
  if( !PyArg_ParseTuple( args, "p", &modify ) )
  { return nullptr; }

  return LvArray::python::create( array1DOfDoubles, modify );
}

static PyObject * getArray2DIJOfLongs( PyObject * const self, PyObject * const args )
{
  LVARRAY_UNUSED_VARIABLE( self );

  int modify;
  if( !PyArg_ParseTuple( args, "p", &modify ) )
  { return nullptr; }

  return LvArray::python::create( array2DIJOfLongs, modify );
}

static PyObject * getArray2DJIOfFloats( PyObject * const self, PyObject * const args )
{
  LVARRAY_UNUSED_VARIABLE( self );

  int modify;
  if( !PyArg_ParseTuple( args, "p", &modify ) )
  { return nullptr; }

  return LvArray::python::create( array2DJIOfFloats, modify );
}

static PyObject * getArray4DKILJOfDoubles( PyObject * const self, PyObject * const args )
{
  LVARRAY_UNUSED_VARIABLE( self );

  int modify;
  if( !PyArg_ParseTuple( args, "p", &modify ) )
  { return nullptr; }

  return LvArray::python::create( array4DKILJOfDoubles, modify );
}

BEGIN_ALLOW_DESIGNATED_INITIALIZERS

/**
 * Array of functions and docstrings to export to Python
 */
static PyMethodDef testPyArrayFuncs[] = {
  {"get_array1d_int", getArray1DOfInts, METH_VARARGS, ""},
  {"get_array1d_double", getArray1DOfDoubles, METH_VARARGS, ""},
  {"get_array2d_ij_long", getArray2DIJOfLongs, METH_VARARGS, ""},
  {"get_array2d_ji_float", getArray2DJIOfFloats, METH_VARARGS, ""},
  {"get_array4d_kilj_double", getArray4DKILJOfDoubles, METH_VARARGS, ""},
  {nullptr, nullptr, 0, nullptr}        /* Sentinel */
};

/**
 * Initialize the module object for Python with the exported functions
 */
static struct PyModuleDef testPyArrayModule = {
  PyModuleDef_HEAD_INIT,
  .m_name = "testPyArray",
  .m_doc = "",
  .m_size = -1,
  .m_methods = testPyArrayFuncs,
};

END_ALLOW_DESIGNATED_INITIALIZERS

PyMODINIT_FUNC
PyInit_testPyArray(void)
{
  LvArray::python::PyObjectRef<> module = PyModule_Create( &testPyArrayModule );

  if ( !LvArray::python::addTypeToModule( module, LvArray::python::getPyArrayType(), "Array" ) )
  { return nullptr; }

  return module.release();
}
