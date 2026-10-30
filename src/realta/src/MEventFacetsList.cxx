/*
 * MEventFacetsList.cxx
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
// MEventFacetsList
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MEventFacetsList.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MEventFacets.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MEventFacetsList)
#endif


////////////////////////////////////////////////////////////////////////////////


MEventFacetsList::MEventFacetsList(UInt_t MaximumSize)
{
  // Construct an instance of MEventFacetsList
  
  SetMaximumSize(MaximumSize);
  m_IterationEvent = 0;

  m_List = new TList();
}


////////////////////////////////////////////////////////////////////////////////


MEventFacetsList::~MEventFacetsList()
{
  // Delete this instance of MEventFacetsList
}


////////////////////////////////////////////////////////////////////////////////


void MEventFacetsList::SetMaximumSize(UInt_t MaximumSize)
{
  // 

  m_MaximumSize = MaximumSize;
}


////////////////////////////////////////////////////////////////////////////////


UInt_t MEventFacetsList::GetSize()
{


  return m_List->GetSize();
}

////////////////////////////////////////////////////////////////////////////////


void MEventFacetsList::Add(MEventFacets *Event)
{
  //

  if (m_List->GetSize() >= (unsigned int )m_MaximumSize) {
    delete (MEventFacets *) (m_List->Remove(m_List->First()));
  }
  m_List->AddLast(Event);
}


////////////////////////////////////////////////////////////////////////////////


void MEventFacetsList::RestartIterator(MEventFacets *Event)
{
  //

  m_IterationEvent = Event;
}


////////////////////////////////////////////////////////////////////////////////


MEventFacets* MEventFacetsList::Next()
{
  //
  
  if (m_IterationEvent == 0) {
    m_IterationEvent = (MEventFacets *) (m_List->First());
  } else {
    m_IterationEvent = (MEventFacets *) (m_List->After(m_IterationEvent));
  }
  return m_IterationEvent;
}


////////////////////////////////////////////////////////////////////////////////



// MEventFacetsList.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
