/*
 * MCCrossSections.hh
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */



/******************************************************************************
 *
 * Class to calculate cross sections!
 *
 */

#ifndef ___MCCrossSections___
#define ___MCCrossSections___

// Geant4:
#include "globals.hh"

// Cosima:

// MEGAlib:
#include "MString.h"

// Standard lib:

// Forward declarations:


/******************************************************************************/

class MCCrossSections
{
  // public interface:
public:
  /// Default constructor
  MCCrossSections();
  /// Default destructor
  virtual ~MCCrossSections();

  /// Create all absorption files in the correct directory
  bool CreateCrossSectionFiles(MString Path);

  // protected methods:
protected:
  

  // protected members:
protected:


  // private members:
private:


};

#endif


/*
 * MCCrossSections.hh: the end...
 ******************************************************************************/
