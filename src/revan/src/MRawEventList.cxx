/*
 * MRawEventList.cxx
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
// MRawEventList.cxx
//
// This is a list of MRERawEvent-objects.
// The different objects are different expressions (different hit sequenence,
// track direction) of the same underlying (one and only) event.
//
// The destructor does not delete the objects, but you have to call
// DeleteAll() before destructing.
//
////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <limits>
#include <algorithm>
using namespace std;

// MEGAlib libs:
#include "MAssert.h"
#include "MRawEventList.h"
#include "MStreams.h"
#include "MRETrack.h"


#ifdef ___CINT___
ClassImp(MRawEventList)
#endif


////////////////////////////////////////////////////////////////////////////////


MRawEventList::MRawEventList()
{
  // Create a new MRawEventList object

  Init();
}


////////////////////////////////////////////////////////////////////////////////


MRawEventList::MRawEventList(MGeometryRevan* Geometry)
{
  // Create a new MRawEventList object

  Init();
  m_Geometry = Geometry;
}


////////////////////////////////////////////////////////////////////////////////


MRawEventList::~MRawEventList()
{
  // This does not delete the raw events itself!
  // Call DeleteAll() before if you want to delete the MRERawEvents, too
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventList::Init()
{
  // Some initialisations equal for all constructors:

  m_RawEventList.clear();

  m_InitialEvent = 0;
  m_OptimumEvent = 0;
  m_BestTryEvent = 0;

  m_EventCounter = 0;
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventList::SetGeometry(MGeometryRevan* Geometry)
{
  // Set the geometry description

  m_Geometry = Geometry;
}


////////////////////////////////////////////////////////////////////////////////


MString MRawEventList::ToString(bool WithLink, int Level)
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
  String += MString("Raw event with the following possible combinations:\n");

  char Text[100];
  for (int e = 0; e < GetNRawEvents(); e++) {
    for (int i = 0; i < Level; i++) {
      String += MString("   ");
    }
    sprintf(Text, "Combination %d:\n", e+1);
    String += MString(Text);

    String += GetRawEventAt(e)->ToString(WithLink, Level+1);
  }
  
  return String;
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventList::AddRawEvent(MRERawEvent* RE) 
{ 
  // Add a raw event to the list

  m_RawEventList.push_back(RE); 
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventList::RemoveRawEvent(MRERawEvent* RE) 
{ 
  // Remove a raw event from the list but do NOT delete it

  if (RE == m_InitialEvent) m_InitialEvent = 0;
  if (RE == m_OptimumEvent) m_OptimumEvent = 0;
  if (RE == m_BestTryEvent) m_BestTryEvent = 0;

  m_RawEventList.erase(find(m_RawEventList.begin(), m_RawEventList.end(), RE)); 
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventList::DeleteRawEvent(MRERawEvent* RE) 
{ 
  // Remove a raw event from the list and delete it

  if (RE == m_InitialEvent) m_InitialEvent = 0;
  if (RE == m_OptimumEvent) m_OptimumEvent = 0;
  if (RE == m_BestTryEvent) m_BestTryEvent = 0;

  m_RawEventList.erase(find(m_RawEventList.begin(), m_RawEventList.end(), RE)); 
  delete RE;
}


////////////////////////////////////////////////////////////////////////////////


MRERawEvent* MRawEventList::GetRawEventAt(int i) 
{ 
  // Get the raw event at position i. Counting starts with zero!

  if (i < int(m_RawEventList.size())) {
    return m_RawEventList[i];
  } 

  merr<<"Index ("<<i<<") out of bounds (0, "<<m_RawEventList.size()-1<<")"<<endl;
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventList::SetRawEventAt(MRERawEvent* RE, int i)
{
  // Set the raw event at position i. Counting starts with zero!

  if (i < int(m_RawEventList.size())) {
    m_RawEventList[i] = RE;
  } else {
    merr<<"Index ("<<i<<") out of bounds (0, "<<m_RawEventList.size()-1<<")"<<endl;
    massert(i < int(m_RawEventList.size()));
  }
}


////////////////////////////////////////////////////////////////////////////////


int MRawEventList::GetNRawEvents() 
{ 
  // Get the number of raw events in the list

  return m_RawEventList.size(); 
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventList::DeleteAll() 
{ 
  // Delete all elements of the list

  while (m_RawEventList.size() != 0) {
    DeleteRawEvent(GetRawEventAt(0));
  }

  m_InitialEvent = 0;
  m_OptimumEvent = 0;
  m_BestTryEvent = 0;
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventList::SetInitialRawEvent(MRERawEvent* RE)
{
  // add the first raw event and start the analysis of the event

  massert(RE != 0);

  //! Delete all previous events
  DeleteAll();

  m_InitialEvent = RE;
  m_OptimumEvent = 0;
  m_BestTryEvent = 0;

  AddRawEvent(RE);
}


////////////////////////////////////////////////////////////////////////////////


bool MRawEventList::HasOptimumEvent()
{
  // Check if we have an optimum event

  if (m_OptimumEvent != 0) {
    mdebug<<"Optimum event!"<<endl;
    return true;
  }

  mdebug<<"No optimum event!"<<endl;
  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventList::SetBestTryEvent(MRERawEvent* BestTryEvent)
{
  // Set the event which is the best shot, but is definitely not the correct one

  m_BestTryEvent = BestTryEvent;
}


////////////////////////////////////////////////////////////////////////////////


bool MRawEventList::HasBestTry()
{
  // Check if we have abest try event...

  if (m_BestTryEvent != 0) {
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////


MRERawEvent* MRawEventList::GetBestTryEvent()
{
  // Return a pointer to the best try...

  return m_BestTryEvent;
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventList::SetOptimumEvent(MRERawEvent* OptimumEvent)
{
  // Set the optimum event

  if (OptimumEvent != 0) {
    massert(OptimumEvent->IsGoodEvent() == true);
  }

  m_OptimumEvent = OptimumEvent;
}


////////////////////////////////////////////////////////////////////////////////


MRERawEvent* MRawEventList::GetOptimumEvent()
{
  // Return the optimum event

  return m_OptimumEvent;
}


////////////////////////////////////////////////////////////////////////////////


MRERawEvent* MRawEventList::GetInitialRawEvent()
{
  // Return the inital event, i.e. the mere hits event...

  return m_InitialEvent;
}


////////////////////////////////////////////////////////////////////////////////


bool MRawEventListTrackCompareGoodAreHigh(MRERawEvent* a, MRERawEvent* b) 
{
  // No quality factor is a large number, so make sure it is at the end of the list!
  if (a->GetTrackQualityFactor() == MRERawEvent::c_NoQualityFactor) {
    return false;
  }

  if (a->GetTrackQualityFactor() > b->GetTrackQualityFactor()) {
    return true;
  } else if (a->GetTrackQualityFactor() == b->GetTrackQualityFactor()) {
    if (a->GetRESEAt(0)->GetEnergy() < b->GetRESEAt(0)->GetEnergy()) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
};


////////////////////////////////////////////////////////////////////////////////


bool MRawEventListTrackCompareGoodAreLow(MRERawEvent* a, MRERawEvent* b) 
{

  if (a->GetTrackQualityFactor() < b->GetTrackQualityFactor()) {
    return true;
  } else if (a->GetTrackQualityFactor() == b->GetTrackQualityFactor()) {
    if (a->GetRESEAt(0)->GetEnergy() < b->GetRESEAt(0)->GetEnergy()) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
};


////////////////////////////////////////////////////////////////////////////////


void MRawEventList::SortByTrackQualityFactor(bool GoodAreHigh)
{
  // This is some kind of basic quicksort algorithm:

  if (GoodAreHigh == true) {
    sort(m_RawEventList.begin(), m_RawEventList.end(), MRawEventListTrackCompareGoodAreHigh);
  } else {
    sort(m_RawEventList.begin(), m_RawEventList.end(), MRawEventListTrackCompareGoodAreLow);
  }
}


// MRawEventList.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
