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
  return os<<static_cast<int>(C); 
}



// MCoordinateSystem.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
