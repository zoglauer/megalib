/*
 * MResponseType.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
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
