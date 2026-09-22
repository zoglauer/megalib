/*
 * MHitEventAnalyzer.cxx
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
#include "MRawEventIncarnations.h"
#include "MRERawEvent.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MHitEventAnalyzer)
#endif


////////////////////////////////////////////////////////////////////////////////


MHitEventAnalyzer::MHitEventAnalyzer(MDGeometryQuest* Geometry)
{
  // Construct an instance of MHitEventAnalyzer

  m_Geometry = Geometry; 
  m_Geo = (MGeometryBasic *) new MGeometryMEGA();
  m_RawEvents = new MRawEventIncarnations(m_Geo);
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
