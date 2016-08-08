/*
 * MResponseType.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseType__
#define __MResponseType__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////



//! Definition of the coordiante system IDs
enum class MResponseType : int { 
  Unknown = 0, Gauss1D = 1, GaussByUncertainties = 2, GaussByEnergyLeakage = 3, PRM = 4, ConeShapes = 5 
};

//! I/O for MResponseType
std::ostream& operator<<(std::ostream& os, MResponseType T);


#endif


////////////////////////////////////////////////////////////////////////////////
