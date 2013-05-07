/*
 * MEventFacetsList.cxx                                   v0.1  01/01/2001
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


#ifdef ___CINT___
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
