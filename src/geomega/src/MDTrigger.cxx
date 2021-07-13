/*
 * MDTrigger.cxx
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
// MDTrigger
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDTrigger.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MDDetector.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDTrigger)
#endif


////////////////////////////////////////////////////////////////////////////////


const int MDTrigger::c_Detector = 0;
const int MDTrigger::c_GuardRing = 1;


////////////////////////////////////////////////////////////////////////////////


MDTrigger::MDTrigger(const MString& Name)
{
  // Construct an instance of MDTrigger

  m_Name = Name;
  m_TriggerType = MDTriggerType::c_Unknown;
  m_IgnoreVetoes = false;
}


////////////////////////////////////////////////////////////////////////////////


MDTrigger::~MDTrigger()
{
  // Delete this instance of MDTrigger
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, const MDTrigger& Trigger)
{
  os<<Trigger.ToString();
  
  return os;
}


// MDTrigger.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
