/*
 * MRawEventIncarnations.cxx
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
#include "MRawEventIncarnations.h"
#include "MStreams.h"
#include "MRETrack.h"


#ifdef ___CLING___
ClassImp(MRawEventIncarnations)
#endif


////////////////////////////////////////////////////////////////////////////////


MRawEventIncarnations::MRawEventIncarnations()
{
  Init();
  m_Geometry = nullptr;
}


////////////////////////////////////////////////////////////////////////////////


MRawEventIncarnations::MRawEventIncarnations(MGeometryRevan* Geometry)
{
  Init();
  m_Geometry = Geometry;
}


////////////////////////////////////////////////////////////////////////////////


MRawEventIncarnations::~MRawEventIncarnations()
{
  // This does not delete the raw events itself!
  // Call DeleteAll() before if you want to delete the MRERawEvents, too
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventIncarnations::Init()
{
  m_Incarnations.clear();

  m_InitialEvent = 0;
  m_OptimumEvent = 0;
  m_BestTryEvent = 0;
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventIncarnations::SetGeometry(MGeometryRevan* Geometry)
{
  m_Geometry = Geometry;
}


////////////////////////////////////////////////////////////////////////////////


MString MRawEventIncarnations::ToString(bool WithLink, int Level)
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


void MRawEventIncarnations::AddRawEvent(MRERawEvent* RE) 
{ 
  m_Incarnations.push_back(RE); 
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventIncarnations::RemoveRawEvent(MRERawEvent* RE) 
{ 
  // Remove a raw event from the list but do NOT delete it

  if (RE == m_InitialEvent) m_InitialEvent = 0;
  if (RE == m_OptimumEvent) m_OptimumEvent = 0;
  if (RE == m_BestTryEvent) m_BestTryEvent = 0;

  m_Incarnations.erase(find(m_Incarnations.begin(), m_Incarnations.end(), RE)); 
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventIncarnations::DeleteRawEvent(MRERawEvent* RE) 
{ 
  // Remove a raw event from the list and delete it

  if (RE == m_InitialEvent) m_InitialEvent = 0;
  if (RE == m_OptimumEvent) m_OptimumEvent = 0;
  if (RE == m_BestTryEvent) m_BestTryEvent = 0;

  m_Incarnations.erase(find(m_Incarnations.begin(), m_Incarnations.end(), RE)); 
  delete RE;
}


////////////////////////////////////////////////////////////////////////////////


MRERawEvent* MRawEventIncarnations::GetRawEventAt(int i) 
{ 
  // Get the raw event at position i. Counting starts with zero!

  if (i < int(m_Incarnations.size())) {
    return m_Incarnations[i];
  } 

  merr<<"Index ("<<i<<") out of bounds (0, "<<m_Incarnations.size()-1<<")"<<endl;
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventIncarnations::SetRawEventAt(MRERawEvent* RE, int i)
{
  // Set the raw event at position i. Counting starts with zero!

  if (i < int(m_Incarnations.size())) {
    m_Incarnations[i] = RE;
  } else {
    merr<<"Index ("<<i<<") out of bounds (0, "<<m_Incarnations.size()-1<<")"<<endl;
    massert(i < int(m_Incarnations.size()));
  }
}


////////////////////////////////////////////////////////////////////////////////


int MRawEventIncarnations::GetNRawEvents() 
{ 
  // Get the number of raw events in the list

  return m_Incarnations.size(); 
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventIncarnations::DeleteAll() 
{ 
  // Delete all elements of the list

  while (m_Incarnations.size() != 0) {
    DeleteRawEvent(GetRawEventAt(0));
  }

  m_InitialEvent = 0;
  m_OptimumEvent = 0;
  m_BestTryEvent = 0;
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventIncarnations::SetInitialRawEvent(MRERawEvent* RE)
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


bool MRawEventIncarnations::HasOptimumEvent()
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


void MRawEventIncarnations::SetBestTryEvent(MRERawEvent* BestTryEvent)
{
  // Set the event which is the best shot, but is definitely not the correct one

  m_BestTryEvent = BestTryEvent;
}


////////////////////////////////////////////////////////////////////////////////


bool MRawEventIncarnations::HasBestTry()
{
  // Check if we have abest try event...

  if (m_BestTryEvent != 0) {
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////


MRERawEvent* MRawEventIncarnations::GetBestTryEvent()
{
  // Return a pointer to the best try...

  return m_BestTryEvent;
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventIncarnations::SetOptimumEvent(MRERawEvent* OptimumEvent)
{
  // Set the optimum event

  if (OptimumEvent != 0) {
    massert(OptimumEvent->IsGoodEvent() == true);
  }

  m_OptimumEvent = OptimumEvent;
}


////////////////////////////////////////////////////////////////////////////////


MRERawEvent* MRawEventIncarnations::GetOptimumEvent()
{
  // Return the optimum event

  return m_OptimumEvent;
}


////////////////////////////////////////////////////////////////////////////////


MRERawEvent* MRawEventIncarnations::GetInitialRawEvent()
{
  // Return the inital event, i.e. the mere hits event...

  return m_InitialEvent;
}


////////////////////////////////////////////////////////////////////////////////


bool MRawEventIncarnationsTrackCompareGoodAreHigh(MRERawEvent* a, MRERawEvent* b) 
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


bool MRawEventIncarnationsTrackCompareGoodAreLow(MRERawEvent* a, MRERawEvent* b) 
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


void MRawEventIncarnations::SortByTrackQualityFactor(bool GoodAreHigh)
{
  // This is some kind of basic quicksort algorithm:

  if (GoodAreHigh == true) {
    sort(m_Incarnations.begin(), m_Incarnations.end(), MRawEventIncarnationsTrackCompareGoodAreHigh);
  } else {
    sort(m_Incarnations.begin(), m_Incarnations.end(), MRawEventIncarnationsTrackCompareGoodAreLow);
  }
}


// MRawEventIncarnations.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
