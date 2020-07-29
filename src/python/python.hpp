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
 * @file python.hpp
 * @brief Includes all the other python related headers.
 */

#pragma once

// source includes
#include "numpyArrayView.hpp"
#include "numpySortedArrayView.hpp"
#include "../templateHelpers.hpp"

namespace LvArray
{
namespace python
{

IS_VALID_EXPRESSION( CanCreate, T, create( std::declval< T & >(), true ) );

} // namespace python
} // namespace LvArray