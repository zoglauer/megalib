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


void MRawEventIncarnationList::RemoveAll() 
{ 
  // Remove all elements of the list
  
  while (m_Collection.size() != 0) {
    Remove(Get(0));
  }
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
    // Check for 511's:
    MPETEvent* P = nullptr;
    vector<unsigned int> Annihilations;
    for (unsigned int i = 0; i < m_Collection.size(); ++i) {
      if (fabs(m_Collection[i]->GetOptimumEvent()->GetEnergy() - c_E0) < 2*m_Collection[i]->GetOptimumEvent()->GetEnergyResolution()) {
        Annihilations.push_back(i);
      }
    }
    
    // If we have exactly 2 511's we create a PET event out of them 
    if (Annihilations.size() == 2) {
      MVector Start1;
      if (m_Collection[Annihilations[0]]->GetOptimumEvent()->GetStartPoint() != nullptr) {
        Start1 = m_Collection[Annihilations[0]]->GetOptimumEvent()->GetStartPoint()->GetPosition();
      } else {
        Start1 = m_Collection[Annihilations[0]]->GetOptimumEvent()->GetRESEAt(0)->GetPosition();
        if (m_Collection[Annihilations[0]]->GetOptimumEvent()->GetNRESEs() != 1) {
          //mout<<m_Collection[Annihilations[0]]->GetOptimumEvent()->ToString()<<endl;
          merr<<"GetOptimumPhysicalEvent: Error missing start point of event (did you require a start in the tracker, but have no trigger criteria which enforces that?)"<<endl;
          return nullptr;
        }
      }
      MVector Start2;
      if (m_Collection[Annihilations[1]]->GetOptimumEvent()->GetStartPoint() != nullptr) {
        Start2 = m_Collection[Annihilations[1]]->GetOptimumEvent()->GetStartPoint()->GetPosition();
      } else {
        Start2 = m_Collection[Annihilations[1]]->GetOptimumEvent()->GetRESEAt(0)->GetPosition();
        if (m_Collection[Annihilations[1]]->GetOptimumEvent()->GetNRESEs() != 1) {
          //mout<<m_Collection[Annihilations[0]]->GetOptimumEvent()->ToString()<<endl;
          merr<<"GetOptimumPhysicalEvent: Error missing start point of event (did you require a start in the tracker, but have no trigger criteria which enforces that?)"<<endl;
          return nullptr;
        }
      }
      
      P = new MPETEvent();
      P->SetEnergy1(m_Collection[Annihilations[0]]->GetOptimumEvent()->GetEnergy());
      P->SetEnergy2(m_Collection[Annihilations[1]]->GetOptimumEvent()->GetEnergy());
      P->SetPosition1(Start1);
      P->SetPosition2(Start2);
      P->SetTiming1(m_Collection[Annihilations[0]]->GetOptimumEvent()->GetRESEAt(0)->GetTime());
      P->SetTiming2(m_Collection[Annihilations[1]]->GetOptimumEvent()->GetRESEAt(0)->GetTime());
      P->SetTimingResolution1(m_Collection[Annihilations[0]]->GetOptimumEvent()->GetRESEAt(0)->GetTimeResolution());
      P->SetTimingResolution2(m_Collection[Annihilations[1]]->GetOptimumEvent()->GetRESEAt(0)->GetTimeResolution());
      P->SetTime(m_Collection[Annihilations[1]]->GetOptimumEvent()->GetEventTime());
      P->SetId(m_Collection[Annihilations[1]]->GetOptimumEvent()->GetEventId());
      
    } else {
      Annihilations.clear();
    }
    
    
    // If we have a PET event and just two incarnations then we just return the PET event
    if (P != nullptr && m_Collection.size() == 2) {
      return P;
    } 
    // Otherwise we return a multi-event
    else {
      MMultiEvent* Event = new MMultiEvent();
      for (unsigned int i = 0; i < m_Collection.size(); ++i) {
        if (find(Annihilations.begin(), Annihilations.end(), i) == Annihilations.end()) {
          Event->Add(m_Collection[i]->GetOptimumEvent()->GetPhysicalEvent());
        }
      }
      if (P != nullptr) {
        Event->Add(P);
      }
      
      Event->SetTime(Event->GetEvent(0)->GetTime());
      Event->SetId(Event->GetEvent(0)->GetId());
      
      return Event;
    }
  }
  
  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MRawEventIncarnationList::GetBestTryPhysicalEvent()
{
  if (HasBestTryEvents() == false) return nullptr;
  
  if (m_Collection.size() == 0) { // In theory already handled in HasBestTryEvents(), but let's be safe...
    return nullptr;
  } else if (m_Collection.size() == 1) {
    return m_Collection[0]->GetBestTryEvent()->GetPhysicalEvent();
  } else {
    // Check for 511's:
    MPETEvent* P = nullptr;
    vector<unsigned int> Annihilations;
    for (unsigned int i = 0; i < m_Collection.size(); ++i) {
      if (fabs(m_Collection[i]->GetBestTryEvent()->GetEnergy() - c_E0) < 2*m_Collection[i]->GetBestTryEvent()->GetEnergyResolution()) {
        Annihilations.push_back(i);
      }
    }
    
    // If we have exactly 2 511's we create a PET event out of them 
    if (Annihilations.size() == 2) {
      MVector Start1;
      if (m_Collection[Annihilations[0]]->GetBestTryEvent()->GetStartPoint() != nullptr) {
        Start1 = m_Collection[Annihilations[0]]->GetBestTryEvent()->GetStartPoint()->GetPosition();
      } else {
        Start1 = m_Collection[Annihilations[0]]->GetBestTryEvent()->GetRESEAt(0)->GetPosition();
        if (m_Collection[Annihilations[0]]->GetBestTryEvent()->GetNRESEs() != 1) {
          merr<<"GetBestTryPhysicalEvent: Error missing start point of event (did you require a start in the tracker, but have no trigger criteria which enforces a hit in the tracker?)"<<endl;
          //mout<<m_Collection[Annihilations[0]]->GetBestTryEvent()->ToString()<<endl;
          return nullptr;
        }
      }
      MVector Start2;
      if (m_Collection[Annihilations[1]]->GetBestTryEvent()->GetStartPoint() != nullptr) {
        Start2 = m_Collection[Annihilations[1]]->GetBestTryEvent()->GetStartPoint()->GetPosition();
      } else {
        Start2 = m_Collection[Annihilations[1]]->GetBestTryEvent()->GetRESEAt(0)->GetPosition();
        if (m_Collection[Annihilations[1]]->GetBestTryEvent()->GetNRESEs() != 1) {
          merr<<"GetBestTryPhysicalEvent: Error missing start point of event (did you require a start in the tracker, but have no trigger criteria which enforces a hit in the tracker?)"<<endl;
          //mout<<m_Collection[Annihilations[0]]->GetBestTryEvent()->ToString()<<endl;
          return nullptr;
        }
      }
      
      P = new MPETEvent();
      P->SetEnergy1(m_Collection[Annihilations[0]]->GetBestTryEvent()->GetEnergy());
      P->SetEnergy2(m_Collection[Annihilations[1]]->GetBestTryEvent()->GetEnergy());
      P->SetPosition1(Start1);
      P->SetPosition2(Start2);
      P->SetTiming1(m_Collection[Annihilations[0]]->GetBestTryEvent()->GetRESEAt(0)->GetTime());
      P->SetTiming2(m_Collection[Annihilations[1]]->GetBestTryEvent()->GetRESEAt(0)->GetTime());
      P->SetTimingResolution1(m_Collection[Annihilations[0]]->GetBestTryEvent()->GetRESEAt(0)->GetTimeResolution());
      P->SetTimingResolution2(m_Collection[Annihilations[1]]->GetBestTryEvent()->GetRESEAt(0)->GetTimeResolution());
      P->SetTime(m_Collection[Annihilations[1]]->GetBestTryEvent()->GetEventTime());
      P->SetId(m_Collection[Annihilations[1]]->GetBestTryEvent()->GetEventId());
      
    } else {
      Annihilations.clear();
    }
    
    
    // If we have a PET event and just two incarnations then we just return the PET event
    if (P != nullptr && m_Collection.size() == 2) {
      return P;
    } 
    // Otherwise we return a multi-event
    else {
      MMultiEvent* Event = new MMultiEvent();
      for (unsigned int i = 0; i < m_Collection.size(); ++i) {
        if (find(Annihilations.begin(), Annihilations.end(), i) == Annihilations.end()) {
          Event->Add(m_Collection[i]->GetBestTryEvent()->GetPhysicalEvent());
        }
      }
      if (P != nullptr) {
        Event->Add(P);
      }
      
      Event->SetTime(Event->GetEvent(0)->GetTime());
      Event->SetId(Event->GetEvent(0)->GetId());
      
      return Event;
    }
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
