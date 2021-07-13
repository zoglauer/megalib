/*
 * MERCoincidence.cxx
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
// MERCoincidence
//
// Find all clusters...
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERCoincidence.h"

// Standard libs:
#include <cstdlib>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MRESE.h"
#include "MRECluster.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MERCoincidence)
#endif


////////////////////////////////////////////////////////////////////////////////


MERCoincidence::MERCoincidence()
{
  // Construct an instance of MERCoincidence

  m_Algorithm = c_None;

  m_NFoundCoincidences = 0;
}


////////////////////////////////////////////////////////////////////////////////


MERCoincidence::~MERCoincidence()
{
  // Delete this instance of MERCoincidence
}


////////////////////////////////////////////////////////////////////////////////


bool MERCoincidence::SetCoincidenceWindow(double Time)
{
  m_Algorithm = c_Window;

  m_Window.Set(Time);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MRERawEvent* MERCoincidence::Search(MRawEventIncarnations* List, bool Clear)
{
  // Search for coincidences

  //cout<<"Coincidence search..."<<endl;

  MRERawEvent* RE = 0;

  if (m_Algorithm == c_None) {
    //cout<<"No coincidence search"<<endl;
    if (List->GetNRawEvents() > 0) {
      RE = List->GetRawEventAt(0);
      List->RemoveRawEvent(RE);
    }
  } else if (m_Algorithm == c_Window) {
    //cout<<"Coincidence window: "<<List->GetNRawEvents()<<endl;
    if (List->GetNRawEvents() > 0) {
      // Find coincidences
      int Coincident = 1;
      MTime Time = List->GetRawEventAt(0)->GetEventTime();
      for (int i = 1; i < List->GetNRawEvents(); ++i) {
        //cout<<List->GetRawEventAt(i)->GetEventTime().GetAsSeconds()<<":"<<Time.GetAsSeconds()<<":"<<m_Window.GetAsSeconds()<<endl;
        
        // Protection in case a new file has started with tobs = zero
        if (List->GetRawEventAt(i)->GetEventTime() < (Time - 0.1)) {
          mout<<"Info: Next event is > 0.1 sec before current event. Skipping coincidence search since (most likely) the input files have changed..."<<endl;
          break;
        }

        if (List->GetRawEventAt(i)->GetEventTime() - Time <= m_Window) {
          ++Coincident;
        }
      }
      //cout<<"Coincident: "<<Coincident<<endl;
      // In case all events are coincident we have to wait for more events unless we are in clear mode...
      if (List->GetNRawEvents() > Coincident || (List->GetNRawEvents() == Coincident && Clear == true)) {
        MTime CoincidenceWindow(0);
        RE = List->GetRawEventAt(0);
        for (int i = 1; i < Coincident; ++i) {
          if (List->GetRawEventAt(i)->GetNRESEs() > 0) {
            if (List->GetRawEventAt(i)->GetEventTime() - Time > CoincidenceWindow) {
              CoincidenceWindow = List->GetRawEventAt(i)->GetEventTime() - Time; 
            }
            for (int h = 0; h < List->GetRawEventAt(i)->GetNRESEs(); ++h) {
              MRESE* RESE = List->GetRawEventAt(i)->GetRESEAt(h);
              RE->AddRESE(RESE);
              List->GetRawEventAt(i)->RemoveRESE(RESE);
            }
          }
        }
        RE->SetCoincidenceWindow(CoincidenceWindow);
        
        // Remove the first
        List->RemoveRawEvent(List->GetRawEventAt(0));
        // Delete the rest
        for (int i = 0; i < Coincident-1; ++i) {
          RE->SetEventID(List->GetRawEventAt(0)->GetEventID()); // Realta requires the new event ID of the new event to be the one of the youngest event
          List->DeleteRawEvent(List->GetRawEventAt(0)); // yes 0!
        }
        if (Coincident > 1) {
          ++m_NFoundCoincidences;
          //cout<<"Coincidences: "<<m_NFoundCoincidences<<endl;
        }
      }
    }
  } else {
    merr<<"Unknown coincidence algorithm with ID="<<m_Algorithm<<endl;
  }

  //cout<<"The list after:"<<endl;
  //cout<<List->ToString()<<endl;

  if (RE == 0) {
    //cout<<"No coincidence"<<endl;
  } else {
    // Give the hits a new, unique ID
    RE->ReID();
    //cout<<"Event: "<<endl;
    //cout<<RE->ToString()<<endl;
  }

  return RE;
}


////////////////////////////////////////////////////////////////////////////////


MString MERCoincidence::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:

  ostringstream out;

  out<<"# Coincidence options:"<<endl;
  out<<"# "<<endl;
  if (m_Algorithm == c_Window) {
    out<<"# Coincidence window: "<<m_Window.GetAsSeconds()<<endl;
  }
  out<<"# "<<endl;

  return out.str().c_str();
}

// MERCoincidence.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
