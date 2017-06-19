/*
 * MCoordinateSystem.cxx
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
#include "MCoordinateSystem.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MCoordinateSystem)
#endif


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
