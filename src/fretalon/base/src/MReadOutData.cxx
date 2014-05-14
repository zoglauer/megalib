/*
 * MReadOutData.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MReadOutData
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutData.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MReadOutData)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Append the context as text 
ostream& operator<<(ostream& os, const MReadOutData& R)
{
  os<<R.ToString();
  return os;
}

  
// MReadOutData.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
