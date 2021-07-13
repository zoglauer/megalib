/*
 * MExposureMode.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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
