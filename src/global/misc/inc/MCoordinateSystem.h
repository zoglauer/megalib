/*
 * MCoordinateSystem.h
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


#ifndef __MCoordinateSystems__
#define __MCoordinateSystems__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Definition of the coordinate system IDs
enum class MCoordinateSystem : int { 
  c_Unknown = 0, c_Galactic = 1, c_Spheric = 2, c_Cartesian2D = 3, c_Cartesian3D = 4 
};

//! I/O for MCoordinateSystem
std::ostream& operator<<(std::ostream& os, MCoordinateSystem C);


#endif


////////////////////////////////////////////////////////////////////////////////
