/*
 * MCoordinateSystems.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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
