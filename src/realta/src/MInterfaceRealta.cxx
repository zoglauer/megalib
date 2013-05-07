/*
 * MInterfaceRealta.cxx
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


#ifdef ___CINT___
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
