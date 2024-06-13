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
//#include <iostream>
//using namespace std;

// ROOT libs:

// MEGAlib:
#include "MStreams.h"
#include "MAssert.h"
#include "MREAM.h"
#include "MREAMDriftChamberEnergy.h"
#include "MREAMGuardRingHit.h"
#include "MREAMDirectional.h"
#include "MDDriftChamber.h"
#include "MDStrip2D.h"
#include "MDStrip3D.h"
#include "MDVoxel3D.h"
#include "MDStrip3DDirectional.h"
#include "MDGuardRing.h"
#include "MBinaryStore.h"
#include "MSimEvent.h"
#include "MSimHT.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MFileEventsEvta)
#endif


////////////////////////////////////////////////////////////////////////////////

const long MFileEventsEvta::c_NoId = -1;

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
  m_ReachedBinarySection = false;
  
  m_Noising = new MERNoising();
  m_Noising->SetGeometry(m_Geometry);
  
  m_SaveOI = false;
}


////////////////////////////////////////////////////////////////////////////////


MFileEventsEvta::~MFileEventsEvta()
{
  // Delete this instance of MFileEventsEvta

  delete m_Noising;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEventsEvta::Open(MString FileName, unsigned int Way, bool IsBinary)
{
  // Open the file, check if the type is correct

  m_IncludeFileUsed = false;
  m_IncludeFile = new MFileEventsEvta(m_Geometry);
  m_IncludeFile->SetIsIncludeFile(true);
  
  if (FileName.EndsWith("sim") == false && FileName.EndsWith("sim.gz") == false && FileName.EndsWith("evta") == false && FileName.EndsWith("evta.gz") == false) {
    merr<<"This file is neither sim nor evta file: "<<FileName<<endl;
    return false;    
  }
  
  if (FileName.EndsWith("sim") == true || FileName.EndsWith("sim.gz") == true) {
    m_IsSimulation = true;
  } else {
    m_IsSimulation = false;
  }
  
  if (MFileEvents::Open(FileName, Way, IsBinary) == false) {
    return false;
  }
  
  if (m_Geometry->IsScanned() == false) {
    merr<<"We do not have a properly initialized geometry!"<<endl;
    return false;
  }

  m_EventId = -1;
  m_IsFirstEvent = true;
  m_ReachedBinarySection = false;
  
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
  if (m_IsBinary == true) {
    return GetNextEventBinary();
  } else {
    return GetNextEventASCII();
  }
}


////////////////////////////////////////////////////////////////////////////////


MRERawEvent* MFileEventsEvta::GetNextEventASCII()
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
  if (UpdateProgress() == false) return nullptr;

  // This takes care of (SC1)
  if (m_IncludeFileUsed == true) {
    MFileEventsEvta* IncludedFile = dynamic_cast<MFileEventsEvta*>(m_IncludeFile);
    IncludedFile->SaveOI(m_SaveOI); // Inherit the SaveOI status
    MRERawEvent* RE = IncludedFile->GetNextEvent();
    if (RE == nullptr) {
      m_Noising->AddStatistics(IncludedFile->GetERNoising());
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
  int nInteraction; bool firstLine = false;
  //char MCInteraction[5];
  double x, y, z, dx, dy, dz, px, py, pz, e; // For OI fields
  while (IsGood() == true) {
    if (ReadLine(Line) == false) break;
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
        // First set the old event id - this is for backward compatibility, were we did not have the ID keyword
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
        if (sscanf(Line.Data(), "SE%lu", &m_EventId) != 1) {
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
      // Reset OI fields
      x = 0.; y = 0.; z = 0.; dx = 0.; dy = 0.; dz = 0.; px = 0.; py = 0.; pz = 0.; e = 0.;
      nInteraction = 0; firstLine = false;
    } else if ((Line[0] == 'N' && Line[1] == 'F') ||
               (Line[0] == 'E' && Line[1] == 'N')) {
      ReadFooter(true);
    } else if (Line[0] == 'I' && Line[1] == 'N') {
      if (m_IncludeFileUsed == true) {
        MFileEventsEvta* IncludedFile = dynamic_cast<MFileEventsEvta*>(m_IncludeFile);
        IncludedFile->SaveOI(m_SaveOI); // Inherit the SaveOI status
        MRERawEvent* RE = IncludedFile->GetNextEvent();
        //MRERawEvent* RE = dynamic_cast<MFileEventsEvta*>(m_IncludeFile)->GetNextEvent();
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
      
      
      if (m_SaveOI == true) {
        if (Line[0] == 'I' && Line[1] == 'A') {
          if (Line[3] == 'I' && Line[4] == 'N' && Line[5] == 'I' && Line[6] == 'T') { // Retain the original direction, energy and polarization
            if (sscanf(Line.Data(), "IA INIT %*d;%*d;%*d;%*f;%*f;%*f;%*f;%*d;%*f;%*f;%*f;%*f;%*f;%*f;%*f;%*d;%lf;%lf;%lf;%lf;%lf;%lf;%lf",
              &dx, &dy, &dz, &px, &py, &pz, &e) == 7) {
              firstLine = true;
            }
          }
          if (firstLine) {
            if (sscanf(Line.Data(), "IA %*s %d;%*d;%*d;%*f;%lf;%lf;%lf;%*d;%*f;%*f;%*f;%*f;%*f;%*f;%*f;%*d;%*f;%*f;%*f;%*f;%*f;%*f;%*f",
                &nInteraction, &x, &y, &z) == 4) {
              if (nInteraction == 2) { // Retain interaction position instead of original position
                //MCInteraction[4] = '\0';
                ostringstream out;
                out<<"OI "<</*MCInteraction<<" "<<*/x<<";"<<y<<";"<<z<<";"<<dx<<";"<<dy<<";"<<dz<<";"<<px<<";"<<py<<";"<<pz<<";"<<e<<endl;
                Line = out.str().c_str(); 
                firstLine = false;
              }
            }
          }
        }
      }
      

      // All other keywords need to be handled by the current event itself
      Event->ParseLine(Line, m_Version);
    }
  }

  // We are done --- no more new events
  m_EventId = -1;
  m_IsFirstEvent = true;

  delete Event;
  ShowProgress(false);

  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


MRERawEvent* MFileEventsEvta::GetNextEventBinary()
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
  if (UpdateProgress() == false) return nullptr;
  
  // This takes care of (SC1)
  if (m_IncludeFileUsed == true) {
    MFileEventsEvta* IncludedFile = dynamic_cast<MFileEventsEvta*>(m_IncludeFile);
    IncludedFile->SaveOI(m_SaveOI); // Inherit the SaveOI status
    MRERawEvent* RE = IncludedFile->GetNextEvent();
    //MRERawEvent* RE = dynamic_cast<MFileEventsEvta*>(m_IncludeFile)->GetNextEvent();
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
  MRERawEvent* Event = nullptr;
  
  
  if (m_ReachedBinarySection == false) {
    
    MString Line;
    while (IsGood() == true) {
      if (ReadLine(Line) == false) break;
      if (Line == "STARTBINARYSTREAM") {
        m_ReachedBinarySection = true;
        break;
      }
      // Include another file:
      else if (Line[0] == 'I' && Line[1] == 'N') {
        
        if (OpenIncludeFile(Line) == true) {
          MFileEventsEvta* IncludedFile = dynamic_cast<MFileEventsEvta*>(m_IncludeFile);
          IncludedFile->SaveOI(m_SaveOI); // Inherit the SaveOI status
          Event = IncludedFile->GetNextEvent();
          if (Event == nullptr) {
            m_Noising->AddStatistics(IncludedFile->GetERNoising());
            m_IncludeFile->Close();
            m_IncludeFileUsed = false;
            break;
          } else {
            UpdateObservationTimes(Event);
            return Event;
          }
        } else {
          m_IncludeFile->Close();
          m_IncludeFileUsed = false;
        }
      } 
    }
  }
  
  
  
  // Max size of the binary store
  const unsigned long MaxFill = 500000;
  
  // We are in the binary file stream, let's make sure we have at least 1 MB in the data store
  if (IsGood()) {
    if (m_BinaryStore.GetArraySizeUnread() < 0.8*MaxFill) {
      m_BinaryStore.Truncate();
      Read(m_BinaryStore, MaxFill - m_BinaryStore.GetArraySize());
    }
  }
  
  while (m_BinaryStore.GetArraySizeUnread() >= 2) {
    MString Flag = m_BinaryStore.GetString(2);
    m_BinaryStore.ProgressPosition(-2);
    if (Flag != "SE" && Flag != "EN") {
      cout<<"ERROR: File parsing error. Expected SE or EN, but got: "<<Flag<<". Progressing to next sync flag"<<endl;
      while (Flag != "SE" && Flag != "EN") {
        m_BinaryStore.ProgressPosition(+1);
        Flag = m_BinaryStore.GetString(2);
        m_BinaryStore.ProgressPosition(-2);
        
        if (m_BinaryStore.GetArraySizeUnread() < 0.8*MaxFill) {
          m_BinaryStore.Truncate();
          Read(m_BinaryStore, MaxFill - m_BinaryStore.GetArraySize());
        }
      }
      cout<<"INFO: Recovered. Found flag: "<<Flag<<endl;
    }
    
    
    if (Flag == "SE") {
      MSimEvent* SimEvent = new MSimEvent();
      SimEvent->SetGeometry(m_Geometry);
      
      try {
        if (SimEvent->ParseBinary(m_BinaryStore) == false) {
          delete SimEvent;
          SimEvent = nullptr;
        }
      } catch (...) {
        cout<<"ERROR: Unable to read a binary event!"<<endl;
        if (SimEvent != nullptr) {
          delete SimEvent;
          SimEvent = nullptr;
        }
      }
      
      
      if (SimEvent != nullptr) {
        MRESE::ResetIDCounter();
        Event = new MRERawEvent(m_Geometry);
        Event->SetEventID(SimEvent->GetID());
        Event->SetEventTime(SimEvent->GetTime());
        for (unsigned int h = 0; h < SimEvent->GetNHTs(); ++h) {
          MREHit* Hit = new MREHit();
          Hit->SetDetector(SimEvent->GetHTAt(h)->GetDetectorType());
          Hit->SetPosition(SimEvent->GetHTAt(h)->GetPosition());
          Hit->SetEnergy(SimEvent->GetHTAt(h)->GetEnergy());
          Hit->SetTime(SimEvent->GetHTAt(h)->GetTime());
          Hit->RetrieveResolutions(m_Geometry);
          Event->AddRESE(Hit);
        }
        for (unsigned int h = 0; h < SimEvent->GetNGRs(); ++h) {
          MDVolumeSequence* V = m_Geometry->GetVolumeSequencePointer(SimEvent->GetGRAt(h)->GetPosition(), true, true);
          
          MREAMGuardRingHit* GR = new MREAMGuardRingHit();
          GR->SetVolumeSequence(V);
          GR->SetEnergy(SimEvent->GetGRAt(h)->GetEnergy());
          GR->SetEnergyResolution(V->GetDetector()->GetGuardRing()->GetEnergyResolution(SimEvent->GetGRAt(h)->GetEnergy()));
          Event->AddREAM(GR);
        }
        if (m_SaveOI == true && SimEvent->GetNIAs() > 0) {
          Event->SetOriginInformation(SimEvent->GetIAAt(1)->GetPosition(), SimEvent->GetIAAt(0)->GetSecondaryDirection(), SimEvent->GetIAAt(0)->GetSecondaryPolarization(), SimEvent->GetIAAt(0)->GetSecondaryEnergy());
        }
        
        delete SimEvent;
        SimEvent = nullptr;
        
        // If this is simulation, then noise all hits:
        if (m_IsSimulation == true && m_Geometry != nullptr) {
          m_Noising->Analyze(Event);
        } // Is simulation
        
        if (Event->GetNRESEs() == 0) {
          delete Event;
          Event = nullptr;
          MRESE::ResetIDCounter();
        } else {
          UpdateObservationTimes(Event);
          return Event;
        }
      }
      
    } else if (Flag == "EN") {
      ReadFooter(true);
      break;
    }
  }
  
  // We are done --- no more new events
  m_EventId = -1;
  m_IsFirstEvent = true;
  
  delete Event;
  ShowProgress(false);
  
  return nullptr;  
}



// MFileEventsEvta.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
