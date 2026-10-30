/*
 * MDTrigger.cxx
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
