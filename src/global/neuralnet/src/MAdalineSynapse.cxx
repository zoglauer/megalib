/*
 * MAdalineSynapse.cxx
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
// MAdalineSynapse
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MAdalineSynapse.h"

// Standard libs:
#include <sstream>
#include <iostream>
#include <list>
using namespace std;

// ROOT libs:
#include "TRandom.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MAdalineSynapse)
#endif


////////////////////////////////////////////////////////////////////////////////


MAdalineSynapse::MAdalineSynapse() : 
MSynapse() 
{
  // Default constructor
  
  m_Weight=2*gRandom->Rndm()-1;
}


// MAdalineSynapse.cxx: the end...
////////////////////////////////////////////////////////////////////////////////

