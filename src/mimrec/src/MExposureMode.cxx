/*
 * MExposureMode.cxx
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
#include "MExposureMode.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MExposureMode)
#endif


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
