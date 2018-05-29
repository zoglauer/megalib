/*
 * MRawEventIncarnationList.cxx
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


// Standard libs:
#include <limits>
#include <algorithm>
using namespace std;

// MEGAlib libs:
#include "MAssert.h"
#include "MExceptions.h"
#include "MRawEventIncarnationList.h"
#include "MStreams.h"
#include "MRETrack.h"


#ifdef ___CLING___
ClassImp(MRawEventIncarnationList)
#endif


////////////////////////////////////////////////////////////////////////////////


MRawEventIncarnationList::MRawEventIncarnationList()
{
  Init();
  m_Geometry = nullptr;
}


////////////////////////////////////////////////////////////////////////////////


MRawEventIncarnationList::MRawEventIncarnationList(MGeometryRevan* Geometry)
{
  Init();
  m_Geometry = Geometry;
}


////////////////////////////////////////////////////////////////////////////////


MRawEventIncarnationList::~MRawEventIncarnationList()
{
  // This does not delete the raw events itself!
  // Call DeleteAll() before if you want to delete the MRERawEvents, too
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventIncarnationList::Init()
{
  m_Collection.clear();
}


////////////////////////////////////////////////////////////////////////////////


MString MRawEventIncarnationList::ToString(bool WithLink, int Level)
{
  // Returns a MString containing the relevant data of this object, i.e.
  // it calls ToString(...) of the raw events
  //
  // WithLink: Display the links of the rawevents sub elements
  // Level:    A level of N displays 3*N blancs before the text

  MString String("");
  for (int i = 0; i < Level; i++) {
    String += MString("   ");
  }
  String += MString("Raw event collection with the following incarnations:\n");

  for (unsigned int i = 0; i < m_Collection.size(); ++i) {
    String += m_Collection[i]->ToString(WithLink, Level+1);
  }
  
  return String;
}



////////////////////////////////////////////////////////////////////////////////


void MRawEventIncarnationList::Remove(MRawEventIncarnations* REI) 
{ 
  // Remove a raw event from the list but do NOT delete it

  m_Collection.erase(find(m_Collection.begin(), m_Collection.end(), REI)); 
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventIncarnationList::Delete(MRawEventIncarnations* REI) 
{ 
  // Remove a raw event from the list and delete it

  m_Collection.erase(find(m_Collection.begin(), m_Collection.end(), REI));
  REI->DeleteAll();
  delete REI;
}


////////////////////////////////////////////////////////////////////////////////


MRawEventIncarnations* MRawEventIncarnationList::Get(unsigned int i) 
{ 
  // Get the raw event at position i. Counting starts with zero!

  if (i < m_Collection.size()) {
    return m_Collection[i];
  } 

  throw MExceptionIndexOutOfBounds(0, m_Collection.size(), i);
  
  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventIncarnationList::DeleteAll() 
{ 
  // Delete all elements of the list

  while (m_Collection.size() != 0) {
    Delete(Get(0));
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MRawEventIncarnationList::HasOnlyOptimumEvents()
{
  // Check if we have an optimum event

  if (m_Collection.size() == 0) return false;
  
  bool AllOptimum = true;
  for (MRawEventIncarnations* REI: m_Collection) {
    if (REI->HasOptimumEvent() == false) {
      AllOptimum = false;
      break;
    }
  }

  return AllOptimum;
}


////////////////////////////////////////////////////////////////////////////////


bool MRawEventIncarnationList::HasBestTryEvents()
{
  // Check if we have a best try event...
  
  if (m_Collection.size() == 0) return false;
  
  bool AllOptimum = true;
  for (MRawEventIncarnations* REI: m_Collection) {
    if (REI->HasBestTryEvent() == false) {
      AllOptimum = false;
      break;
    }
  }
  
  return AllOptimum;
}


////////////////////////////////////////////////////////////////////////////////


vector<MRERawEvent*> MRawEventIncarnationList::GetBestTryEvents()
{
  // Return a pointer to the best try...

  vector<MRERawEvent*> V;
  
  for (MRawEventIncarnations* REI: m_Collection) {
    V.push_back(REI->GetBestTryEvent());
  }
  
  return V;
}


////////////////////////////////////////////////////////////////////////////////


vector<MRERawEvent*> MRawEventIncarnationList::GetOptimumEvents()
{
  // Return the optimum event

  vector<MRERawEvent*> V;
  
  for (MRawEventIncarnations* REI: m_Collection) {
    V.push_back(REI->GetOptimumEvent());
  }
  
  return V;
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MRawEventIncarnationList::GetOptimumPhysicalEvent()
{
  if (HasOnlyOptimumEvents() == false) return nullptr;

  if (m_Collection.size() == 0) { // In theory alreday handled in HasOnlyOptimumEvents(), but let's be safe...
    return nullptr;
  } else if (m_Collection.size() == 1) {
    return m_Collection[0]->GetOptimumEvent()->GetPhysicalEvent();
  } else {
    MMultiEvent* Event = new MMultiEvent();
    for (MRawEventIncarnations* REI: m_Collection) {
      Event->Add(REI->GetOptimumEvent()->GetPhysicalEvent());
    }
    Event->SetTime(Event->GetEvent(0)->GetTime());
    Event->SetId(Event->GetEvent(0)->GetId());
    return Event;
  }
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MRawEventIncarnationList::GetBestTryPhysicalEvent()
{
  if (HasBestTryEvents() == false) return nullptr;
  
  if (m_Collection.size() == 0) { // In theory alreday handled in HasBestTryEvents(), but let's be safe...
    return nullptr;
  } else if (m_Collection.size() == 1) {
    return m_Collection[0]->GetBestTryEvent()->GetPhysicalEvent();
  } else {
    MMultiEvent* Event = new MMultiEvent();
    for (MRawEventIncarnations* REI: m_Collection) {
      Event->Add(REI->GetBestTryEvent()->GetPhysicalEvent());
    }
    Event->SetTime(Event->GetEvent(0)->GetTime());
    Event->SetId(Event->GetEvent(0)->GetId());
    return Event;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if any of the raw event incarnations is valid
bool MRawEventIncarnationList::IsAnyEventValid() const
{
  for (MRawEventIncarnations* REI: m_Collection) {
    if (REI->IsAnyEventValid() == true) return true; 
  }
  
  return false;
}


// MRawEventIncarnationList.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
