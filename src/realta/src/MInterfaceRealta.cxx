/*
 * MInterfaceRealta.cxx
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
// MInterfaceRealta
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MInterfaceRealta.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MDGeometryQuest.h"
#include "MHitEventAnalyzer.h"
#include "MPhysicalEventAnalyzer.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MInterfaceRealta)
#endif


////////////////////////////////////////////////////////////////////////////////


MInterfaceRealta::MInterfaceRealta(MSettingsRealta *GUIData)
{
  // standard constructor

  m_GUIData = GUIData;

  //m_ControlCenter = new MControlCenter();
  //m_ControlCenter->SetPort(9090);
}


////////////////////////////////////////////////////////////////////////////////


MInterfaceRealta::~MInterfaceRealta()
{
  // default destructor

}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceRealta::AnalyzeEvents()
{
  //
  
  //m_ControlCenter->Connect();
  //m_ControlCenter->StartControlLoop();
}




// MInterfaceRealta.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
