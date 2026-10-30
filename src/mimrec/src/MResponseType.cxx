/*
 * MResponseType.cxx
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
#include "MResponseType.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


std::ostream& operator<<(std::ostream& os, MResponseType Type) 
{
  os<<"response type: ";
  if (Type == MResponseType::Unknown) {
    os<<"unknown"; 
  } else if (Type == MResponseType::Gauss1D) {
    os<<"1D Gaussian  with fixed value"; 
  } else if (Type == MResponseType::GaussByUncertainties) {
    os<<"1D Gaussian calculated via uncertainties"; 
  } else if (Type == MResponseType::GaussByEnergyLeakage) {
    os<<"1D Gauss assuming energy leagake"; 
  } else if (Type == MResponseType::PRM) {
    os<<"pre-calculated response matrix"; 
  } else if (Type == MResponseType::ConeShapes) {
    os<<"simulated cone shapes"; 
  } else {
    os<<static_cast<int>(Type)<<" (undefined name)";
  }
  
  return os;
}


// MResponseType.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
