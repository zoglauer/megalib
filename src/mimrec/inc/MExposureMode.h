/*
 * MExposureMode.h
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


#ifndef __MExposureMode__
#define __MExposureMode__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////



//! Definition of the coordiante system IDs
enum class MExposureMode : int {
  Unknown = 0, Flat = 1, CalculateFromEfficiency = 2,  CalculateFromEfficiencyFarFieldMoving = 3, CalculateFromEfficiencyNearFieldStatic = 4 
};

//! I/O for MExposureMode
std::ostream& operator<<(std::ostream& os, MExposureMode T);


#endif


////////////////////////////////////////////////////////////////////////////////
