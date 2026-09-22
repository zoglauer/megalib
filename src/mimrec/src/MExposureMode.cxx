/*
 * MExposureMode.cxx
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
#include "MExposureMode.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


std::ostream& operator<<(std::ostream& os, MExposureMode Type)
{
  os<<"exposure mode: ";
  if (Type == MExposureMode::Unknown) {
    os<<"unknown";
  } else if (Type == MExposureMode::Flat) {
    os<<"flat";
  } else if (Type == MExposureMode::CalculateFromEfficiency) {
    os<<"calculated from efficiency (near or far field, moving or static)";
  } else if (Type == MExposureMode::CalculateFromEfficiencyFarFieldMoving) {
    os<<"calculated from efficiency (far field, moving)";
  } else if (Type == MExposureMode::CalculateFromEfficiencyNearFieldStatic) {
    os<<"calculated from efficiency (near field, static)";
  } else {
    os<<static_cast<int>(Type)<<" (undefined name)";
  }

  return os;
}


// MExposureMode.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
