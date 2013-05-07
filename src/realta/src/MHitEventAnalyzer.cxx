/*
 * MHitEventAnalyzer.cxx                                   v0.1  01/01/2001
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
// MHitEventAnalyzer
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MHitEventAnalyzer.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MDGeometryQuest.h"
#include "MGeometryBasic.h"
#include "MGeometryMEGA.h"
#include "MHitEvent.h"
#include "MHitData.h"
#include "MPhysicalEvent.h"
#include "MPairEventData.h"
#include "MRawEventList.h"
#include "MRERawEvent.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MHitEventAnalyzer)
#endif


////////////////////////////////////////////////////////////////////////////////


MHitEventAnalyzer::MHitEventAnalyzer(MDGeometryQuest* Geometry)
{
  // Construct an instance of MHitEventAnalyzer

  m_Geometry = Geometry; 
  m_Geo = (MGeometryBasic *) new MGeometryMEGA();
  m_RawEvents = new MRawEventList(m_Geo);
}


////////////////////////////////////////////////////////////////////////////////


MHitEventAnalyzer::~MHitEventAnalyzer()
{
  // Delete this instance of MHitEventAnalyzer

  m_RawEvents->Delete();
  delete m_RawEvents;
}



////////////////////////////////////////////////////////////////////////////////


Bool_t MHitEventAnalyzer::Analyze(MHitEvent* Event)
{
  // Analyze one raw event and return true if the event is useful
  
  MHitData *HD;
  MRERawEvent *RE = new MRERawEvent(m_Geo);

  Int_t i;
  for (i = 0; i < Event->GetNHits(); i++) {
    HD = Event->GetHitAt(i);
    RE->AddRESE(new MREHit(HD->GetPosition(), HD->GetEnergy(), HD->GetDetector()));
  }

  m_RawEvents->AddInitialRawEvent(RE, kTRUE);
  return m_RawEvents->IsValid();
  

  return kFALSE;
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MHitEventAnalyzer::GetPhysicalEvent()
{
  //

  if (m_RawEvents->IsValid() == kTRUE) {
    return m_RawEvents->GetValidEvent()->GetPhysicalEvent();
  } else {
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////




// MHitEventAnalyzer.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
