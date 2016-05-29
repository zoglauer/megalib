/*
 * MFileEventsEvta.cxx
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
// MFileEventsEvta
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFileEventsEvta.h"

// Standard libs:

// ROOT libs:

// MEGAlib:
#include "MStreams.h"
#include "MAssert.h"
#include "MREAM.h"
#include "MREAMDriftChamberEnergy.h"
#include "MREAMGuardringHit.h"
#include "MREAMDirectional.h"
#include "MDDriftChamber.h"
#include "MDStrip2D.h"
#include "MDStrip3D.h"
#include "MDVoxel3D.h"
#include "MDStrip3DDirectional.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MFileEventsEvta)
#endif


////////////////////////////////////////////////////////////////////////////////

const int MFileEventsEvta::c_NoId = -1;

////////////////////////////////////////////////////////////////////////////////


MFileEventsEvta::MFileEventsEvta(MGeometryRevan* Geometry) : MFileEvents()
{
  // Construct an instance of MFileEventsEvta

  m_Geometry = Geometry;
  if (Geometry == 0) {
    merr<<"Geometry pointer is zero! Aborting!"<<fatal;
  }

  m_EventId = -1;
  m_IsFirstEvent = true;

  m_Noising = new MERNoising();
  m_Noising->SetGeometry(m_Geometry);
}


////////////////////////////////////////////////////////////////////////////////


MFileEventsEvta::~MFileEventsEvta()
{
  // Delete this instance of MFileEventsEvta

  delete m_Noising;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEventsEvta::Open(MString FileName, unsigned int Way)
{
  // Open the file, check if the type is correct

  m_IncludeFileUsed = false;
  m_IncludeFile = new MFileEventsEvta(m_Geometry);
  m_IncludeFile->SetIsIncludeFile(true);

  if (FileName.EndsWith("sim") == true || FileName.EndsWith("sim.gz") == true) {
    m_IsSimulation = true;
  } else {
    m_IsSimulation = false;
  }

  if (MFileEvents::Open(FileName, Way) == false) {
    return false;
  }

  if (m_Geometry->IsScanned() == false) {
    mout<<"We do not have a properly initialized geometry!"<<endl;
    return false;
  }

  m_EventId = -1;
  m_IsFirstEvent = true;

  m_Noising->PreAnalysis();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEventsEvta::Close()
{
  // Close the file

  return MFileEvents::Close();
}
  

////////////////////////////////////////////////////////////////////////////////


void MFileEventsEvta::UpdateObservationTimes(MRERawEvent* Event)
{
  //! Update the observation times using the given event
  
  // If the overall observation time has alreday been set, don't change anything
  if (m_HasObservationTime == true) return;
  
  // Otherwise set everything
  if (m_HasStartObservationTime == false) {
    m_StartObservationTime = Event->GetTime();
    m_HasStartObservationTime = true;
  }
  m_EndObservationTime = Event->GetTime();
  m_HasEndObservationTime = true;
  
  // Do not set m_HasObservationTime
}


////////////////////////////////////////////////////////////////////////////////


MRERawEvent* MFileEventsEvta::GetNextEvent()
{
  // Return the next event... or 0 if it is the last one
  // So remember to test for more events!

  massert(m_IsOpen == true);

  // The general reading plan:
  // We have to consider several special cases:
  // (SC0) If the user has pressed cancel, we stop any reading
  // (SC1) If there is an active INclude-file we have to get the event from this file
  //       If there are no more events in the include file, we continue reading this file  
  

  // This takes care of (SC0)
  if (UpdateProgress() == false) return 0;

  // This takes care of (SC1)
  if (m_IncludeFileUsed == true) {
    MRERawEvent* RE = dynamic_cast<MFileEventsEvta*>(m_IncludeFile)->GetNextEvent();
    if (RE == nullptr) {
      m_Noising->AddStatistics(dynamic_cast<MFileEventsEvta*>(m_IncludeFile)->GetERNoising());
      if (m_IncludeFile->IsCanceled() == true) m_Canceled = true;
      m_IncludeFile->Close();
      m_IncludeFileUsed = false;
      if (m_Canceled == true) return nullptr;
    } else {
      UpdateObservationTimes(RE);
      return RE;
    }
  }

  // The standard loop:


  // Create a new MRERawEvent...
  MRESE::ResetIDCounter();
  MRERawEvent* Event = new MRERawEvent(m_Geometry);

  MString Line;
  while (IsGood() == true) {
    ReadLine(Line);
    if (Line.Length() < 2) continue;

    // Round 1: Take care of new files --- but do not yet read them or new events in them:
    if (Line[0] == 'N' && Line[1] == 'F') {

      if (OpenNextFile(Line) == 0) {
        mgui<<"Your current file contains a \"NF\" -- next file -- directive."<<endl
            <<"However, the file could not be found or read!!"<<show;
        return 0;
      }
    } else if (Line[0] == 'I' && Line[1] == 'N') {

      if (OpenIncludeFile(Line) == true) {
        mdebug<<"Switched to new include file: "<<m_IncludeFile->GetFileName()<<endl;
      } else {
        mgui<<"Your current file contains a \"IN\" -- include file -- directive."<<endl
            <<"However, the file could not be found or read: "<<m_IncludeFile->GetFileName()<<show;
      }
    }


    // Round 2: The event is completed
    if ((Line[0] == 'S' && Line[1] == 'E') ||
        (Line[0] == 'E' && Line[1] == 'N') ||
        (Line[0] == 'N' && Line[1] == 'F') ||
        (Line[0] == 'I' && Line[1] == 'N') ) {

      if (m_IsFirstEvent == false) {
        // First set the old event id - this is for backeard compatibility, were we did not have the ID keyword
        if (m_EventId != c_NoId) {
          Event->SetEventID(m_EventId);
        }

        // If this is simulation, then noise all hits:
        if (m_IsSimulation == true && m_Geometry != 0) {
          m_Noising->Analyze(Event);
        } // Is simulation
      } // not first event
      
      // Backward compatibility: The SE keyword may contain the event ID
      if (Line[0] == 'S' && Line[1] == 'E') {
        if (sscanf(Line.Data(), "SE%i", &m_EventId) != 1) {
          m_EventId = c_NoId;
        }
      }
      
      // If the event is empty, then we ignore it and prepare for the next event:
      if (Event->GetNRESEs() == 0) {
        delete Event;
        MRESE::ResetIDCounter();
        Event = new MRERawEvent(m_Geometry);
        m_IsFirstEvent = false;
      } else {
        if (m_IsFirstEvent == false) {
          UpdateObservationTimes(Event);
          return Event;
        } else {
          m_IsFirstEvent = false;
        }
      }
    }


    // Round 3:
    if ((Line[0] == 'S' && Line[1] == 'E')) {
      // Everything already handled!
    } else if ((Line[0] == 'N' && Line[1] == 'F') ||
               (Line[0] == 'E' && Line[1] == 'N')) {
      ReadFooter(true);
    } else if (Line[0] == 'I' && Line[1] == 'N') {
      if (m_IncludeFileUsed == true) {
        MRERawEvent* RE = dynamic_cast<MFileEventsEvta*>(m_IncludeFile)->GetNextEvent();
        if (RE == 0) {
          m_Noising->AddStatistics(dynamic_cast<MFileEventsEvta*>(m_IncludeFile)->GetERNoising());
          m_IncludeFile->Close();
          m_IncludeFileUsed = false;
        } else {
          UpdateObservationTimes(RE);
          return RE;
        }
      }
    } else {
      // This information is only transfered during hacks --- no guarantee that it still works
      // We parse the IA information (if present) and transfer the position and direction of the first hit 
      // as OI information to the event
      
      /*
      if (Line[0] == 'I' && Line[1] == 'A') {
        if (Line[3] == 'I' && Line[4] == 'N' && Line[5] == 'I' && Line[6] == 'T') {
          double x, y, z, dx, dy, dz, px, py, pz, e;
          if (sscanf(Line.Data(), "IA INIT %*d;%*d;%*d;%*lf;%lf;%lf;%lf;%*d;%*lf;%*lf;%*lf;%*lf;%*lf;%*lf;%*lf;%*d;%lf;%lf;%lf;%lf;%lf;%lf;%lf",
                     &x, &y, &z, &dx, &dy, &dz, &px, &py, &pz, &e) == 10) {
            ostringstream out;
            out<<"OI "<<x<<";"<<y<<";"<<z<<";"<<dx<<";"<<dy<<";"<<dz<<";"<<px<<";"<<py<<";"<<pz<<";"<<e<<endl;
            Line = out.str().c_str();
          }
        }
      }
      */

      // All other keywords need to be handled by the current event itself
      Event->ParseLine(Line, m_Version);
    }
  }

  // We are done --- no more new events
  m_EventId = -1;
  m_IsFirstEvent = true;

  delete Event;
  ShowProgress(false);

  return 0;
}



// MFileEventsEvta.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
