/*
 * MHitEvent.cxx
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
// MHitEvent
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MHitEvent.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MHitData.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MHitEvent)
#endif


////////////////////////////////////////////////////////////////////////////////


MHitEvent::MHitEvent()
{
  // default constructor

  m_ID = 98;

  m_Hits = 0;
}


////////////////////////////////////////////////////////////////////////////////


MHitEvent::~MHitEvent()
{
  //

  if (m_Hits != 0) m_Hits->Delete();
  delete m_Hits;
}


////////////////////////////////////////////////////////////////////////////////


void MHitEvent::AddHit(MHitData* Hit)
{
  //

  if (m_Hits == 0) {
    m_Hits = new TObjArray();
  }
  m_Hits->AddLast(Hit);
}


////////////////////////////////////////////////////////////////////////////////


Int_t MHitEvent::GetNHits()
{
  //
  if (m_Hits == 0) {
    return 0;
  }
  return m_Hits->GetLast() + 1;
}


////////////////////////////////////////////////////////////////////////////////


MHitData* MHitEvent::GetHitAt(Int_t i)
{
  //

  if (i >= 0 && i < GetNHits()) {
    return (MHitData* ) m_Hits->At(i);
  } else {
    Fatal("MHitData* MHitEvent::GetHitAt(Int_t i)",
          "Index out of bounds: this = %d, min = 0, max = %d", i, GetNHits());
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////



void MHitEvent::SetID(UInt_t ID)
{
  //

  m_ID = ID;
}
 

////////////////////////////////////////////////////////////////////////////////


UInt_t MHitEvent::GetID()
{
  //

  return m_ID;
}


////////////////////////////////////////////////////////////////////////////////



// MHitEvent.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
