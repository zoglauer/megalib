/*
 * MCoordinateSystem.cxx
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


// Include the header:
#include "MCoordinateSystem.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


std::ostream& operator<<(std::ostream& os, MCoordinateSystem C) {
  if (C == MCoordinateSystem::c_Galactic) {
    return os<<"Galactic";
  } else if (C == MCoordinateSystem::c_Spheric) {
    return os<<"spheric";
  } else if (C == MCoordinateSystem::c_Cartesian2D) {
    return os<<"Cartesian 2D";    
  } else if (C == MCoordinateSystem::c_Cartesian3D) {
    return os<<"Cartesian 3D";
  } else {
    return os<<"unknown";
  }
}


// MCoordinateSystem.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
