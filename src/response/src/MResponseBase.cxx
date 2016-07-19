/*
 * MResponseBase.cxx
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
// MResponseBase
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseBase.h"

// Standard libs:
#include <limits>
#include <algorithm>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MFile.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseBase)
#endif


////////////////////////////////////////////////////////////////////////////////


const float MResponseBase::c_NoBound = numeric_limits<float>::max()/3.1;


////////////////////////////////////////////////////////////////////////////////


MResponseBase::MResponseBase()
{
  // Construct an instance of MResponseBase

  m_SimulationFileName = g_StringNotDefined;
  m_GeometryFileName = g_StringNotDefined;
  m_ResponseName = g_StringNotDefined;

  m_StartEventID = 0;
  m_MaxNEvents = numeric_limits<unsigned int>::max();
  m_SaveAfter = 10000;

  m_ReReader = 0;
  m_SiReader = 0;
  m_ReaderFinished = false;
  
  m_ReEvent = 0;
  m_SiEvent = 0;
  
  m_RevanEventID = 0;
  m_RevanLevel = 0;
  m_SivanEventID = 0;
  m_SivanLevel =0;
 
  m_SiGeometry = 0;
  m_ReGeometry = 0;

  m_NumberOfSimulatedEventsClosedFiles = 0;
  m_NumberOfSimulatedEventsThisFile = 0;

  m_Interrupt = false;
  
  m_Suffix = ".rsp";

  m_OnlyINITRequired = false;

  mout<<"Comment to myself:"<<endl;
  mout<<"There is currently a problem in revan that requires coincidence search to be deactivated in order to be able to get a response"<<endl;
  mout<<"There is no easy fix for this..."<<endl;
}


////////////////////////////////////////////////////////////////////////////////


MResponseBase::~MResponseBase()
{
  // Delete this instance of MResponseBase
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseBase::SetSimulationFileName(const MString FileName)
{
  // Set and verify the simulation file name

  if (MFile::Exists(FileName) == false) {
    mout<<"*** Error: \""<<FileName<<"\" does not exist"<<endl;
    return false;
  }
  m_SimulationFileName = FileName;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseBase::SetGeometryFileName(const MString FileName)
{
  // Set and verify the geometry file name

  if (MFile::Exists(FileName) == false) {
    mout<<"*** Error: \""<<FileName<<"\" does not exist"<<endl;
    return false;
  }
  m_GeometryFileName = FileName;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MResponseBase::SetResponseName(const MString Name)
{
  // Set the response name (used for the file name suffix)

  m_ResponseName = Name;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseBase::SetRevanConfigurationFileName(const MString FileName)
{
  // Set and verify the simulation file name

  if (MFile::Exists(FileName) == false) {
    mout<<"*** Error: \""<<FileName<<"\" does not exist"<<endl;
    return false;
  }
  m_RevanCfgFileName = FileName;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseBase::SetMimrecConfigurationFileName(const MString FileName)
{
  // Set and verify the simulation file name

  if (MFile::Exists(FileName) == false) {
    mout<<"*** Error: \""<<FileName<<"\" does not exist"<<endl;
    return false;
  }
  m_MimrecCfgFileName = FileName;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MGeometryRevan* MResponseBase::LoadGeometry(bool ActivateNoise, double GlobalFailureRate)
{
  MGeometryRevan* ReGeometry = new MGeometryRevan();
  if (ReGeometry->ScanSetupFile(m_GeometryFileName) == true) {
    mout<<"Response: Geometry "<<ReGeometry->GetName()<<" loaded!"<<endl;
    ReGeometry->ActivateNoising(ActivateNoise);
    ReGeometry->SetGlobalFailureRate(GlobalFailureRate);
  } else {
    mout<<"Response: Loading of geometry "<<ReGeometry->GetName()<<" failed!!"<<endl;
    delete ReGeometry;
    ReGeometry = 0;
  } 

  return ReGeometry;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseBase::InitializeNextMatchingEvent()
{
  // Initialize the next sivan/revan matching event for response creation

  if (m_Interrupt == true) return false;

  unsigned int ERReturnCode;

 
  bool MoreEvents = true;
  bool TryNextEvent = true;

  bool Restart = true;
  while (m_RevanEventID != m_SivanEventID || TryNextEvent == true) {
    if (m_SivanEventID > m_MaxNEvents) return false;
    
    //cout<<"Response: Levels: r="<<m_RevanLevel<<" s="<<m_SivanLevel<<" IDs: r="<<m_RevanEventID<<" s="<<m_SivanEventID<<endl;
    
    
    if (m_RevanLevel < m_SivanLevel || m_RevanEventID < m_SivanEventID || Restart == true || TryNextEvent == true) {
      Restart = false;
      // Read revan
      //mout<<"Response: Searching revan event..."<<endl;

      // delete m_ReEvent; // automatically deleted!
      m_ReEvent = 0;

      // Load/Analyze
      ERReturnCode = m_ReReader->AnalyzeEvent();
      if (ERReturnCode == MRawEventAnalyzer::c_AnalysisNoEventsLeftInFile) {
        //mout<<"Response: No more events available in file!"<<endl;
        m_ReaderFinished = true;
        MoreEvents = false;
        break;
      }

      if (m_ReReader->GetRawEventList() != 0) {
        if (m_ReReader->GetRawEventList()->GetNRawEvents() > 0) {
          m_ReEvent = m_ReReader->GetRawEventList()->GetRawEventAt(0);
        }
      }

      // Decide future:
      if (m_ReEvent != 0 && m_ReEvent->GetEventType() != MRERawEvent::c_PairEvent) {
        if (m_ReEvent->GetEventID() < m_RevanEventID) {
          m_RevanLevel++; 
        }
        m_RevanEventID = m_ReEvent->GetEventID();
        TryNextEvent = false;
        //mout<<"Response: ER found good solution (Id="<<m_ReEvent->GetEventID()<<")!"<<endl;
      } else {
        TryNextEvent = true;
        //mout<<"Response: Did not find good raw event!"<<endl;
      }
      
    }
    
    if (m_SivanLevel < m_RevanLevel || m_SivanEventID < m_RevanEventID) {
      //mout<<"Response: Searching sivan event..."<<endl;

      // Clean:
      delete m_SiEvent;
      m_SiEvent = 0;
      
      // Read:
      m_SiEvent = m_SiReader->GetNextEvent(false);

      // Decide:
      if (m_SiEvent != 0) {
        // Test if it is not truncated:
        if ((m_OnlyINITRequired == true && m_SiEvent->GetNIAs() == 1 && m_SiEvent->GetIAAt(0)->GetProcess() == "INIT") || 
            (m_SiEvent->GetNIAs() > 1 && m_SiEvent->GetIAAt(m_SiEvent->GetNIAs()-1)->GetProcess() != "TRNC")) {
          
          if ((unsigned int) m_SiEvent->GetID() < m_SivanEventID) {
            m_SivanLevel++; 
          }
          m_SivanEventID = m_SiEvent->GetID();
          TryNextEvent = false;   
          //mout<<"Response: Sivan found good event (Id="<<m_SiEvent->GetID()<<")!"<<endl;
        } else {
          // Ignore this event...
          //mout<<"Response: Sivan found NO good event (Id="<<m_SiEvent->GetID()<<") TRNC or not enough IAs!"<<endl;
          TryNextEvent = true;
        }
      } else {
        TryNextEvent = true;
        mout<<"Response: No more events!"<<endl;
        m_ReaderFinished = true;
        MoreEvents = false;
        break;
      }
    }

    // Before we jump out here, we have to do some sanity checks:
    if (!(m_RevanEventID != m_SivanEventID || TryNextEvent == true)) {
      m_Ids.clear();
      m_OriginIds.clear();
      
      if (SanityCheckSimulations() == false) {
        TryNextEvent = true;
        mout<<"Response: Something is wrong with your simulation! Posibilities are"<<endl;
        mout<<"          * You do not have interaction information (IA)"<<endl;
        mout<<"          * The step length is too long (e.g. longer than your pitch)"<<endl;
        mout<<"          * You have too high production thresholds"<<endl;
        mout<<"          * You have coincidence search turned on"<<endl;
        mout<<"          * Something else..."<<endl;
      }
    }
  }
  
  if (MoreEvents == true) {
    if (m_SiEvent->GetSimulationEventID() < m_NumberOfSimulatedEventsThisFile) {
      m_NumberOfSimulatedEventsClosedFiles = m_SiReader->GetSimulatedEvents();
    }
    m_NumberOfSimulatedEventsThisFile = m_SiEvent->GetSimulationEventID();
  } else {
    m_NumberOfSimulatedEventsClosedFiles = m_SiReader->GetSimulatedEvents();
    m_NumberOfSimulatedEventsThisFile = 0;
  }
  //mout<<"Response: Match Sivan ID="<<m_SivanEventID<<"  Revan ID="<<m_RevanEventID<<endl;
  
  return MoreEvents;  
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseBase::SanityCheckSimulations()
{
  // Do a sanity check if the simulation is ok
  // E.g. all remaining clusters need to have on sim origin != 1 !!
 
  // There is nothing to sanity check...
  if (m_OnlyINITRequired == true) return true;
  
  if (m_SiEvent->GetNIgnoredHTs() > 0) {
    mout<<"Response: We have ignored HTs in the sim file -> "<<
      " check your geometry, because noising was deactivated!"<<endl;
    return false;
  }

  MRawEventList* REList = m_ReReader->GetRawEventList();

  int r_max = REList->GetNRawEvents();
  for (int r = 0; r < r_max; ++r) {
    MRERawEvent* RE = REList->GetRawEventAt(r);
    if (RE->GetVertex() != 0) continue;

    if (int(m_SiEvent->GetNHTs()) < RE->GetNRESEs()) {
      mout<<"The simulation has less hits than the raw event!!!"<<endl;
      return false;
    }

    for (int i = 0; i < RE->GetNRESEs(); ++i) {
      MRESE* RESE = RE->GetRESEAt(i);

      vector<int> EndOriginIds = GetOriginIds(RESE);

      bool More = false;
      for (unsigned int i = 0; i < EndOriginIds.size(); ++i) {
        if (EndOriginIds[i] > 1) {
          More = true;
          break;
        }
      }

      if (More == false) {
        return false;
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


vector<int> MResponseBase::GetReseIds(MRESE* Start)
{ 
  // Return the IDs of all involved base! RESEs, i.e. split clusters and tracks

  massert(Start->GetType() == MRESE::c_Track || 
          Start->GetType() == MRESE::c_Cluster || 
          Start->GetType() == MRESE::c_Hit);

  map<MRESE*, vector<int> >::iterator Iter = m_Ids.find(Start);

  if (Iter != m_Ids.end()) {
    return (*Iter).second;
  } else {
    vector<int> Ids;
    MRESE* RESE = 0;
    MRESE* SubRESE = 0;

    if (Start->GetType() == MRESE::c_Track) {
      int r_max = Start->GetNRESEs();
      for (int r = 0; r < r_max; ++r) {
        RESE = Start->GetRESEAt(r);
        massert(RESE->GetType() == MRESE::c_Hit || RESE->GetType() == MRESE::c_Cluster);
        if (RESE->GetType() == MRESE::c_Cluster) {
          for (int c = 0; c < RESE->GetNRESEs(); ++c) {
            SubRESE = RESE->GetRESEAt(c);
            massert(SubRESE->GetType() == MRESE::c_Hit);
            Ids.push_back(SubRESE->GetID());
          }
        } else {
          Ids.push_back(RESE->GetID());
        }
      }
    } else if (Start->GetType() == MRESE::c_Cluster) {
      int r_max = Start->GetNRESEs();
      for (int r = 0; r < r_max; ++r) {
        RESE = Start->GetRESEAt(r);
        massert(RESE->GetType() == MRESE::c_Hit);
        Ids.push_back(RESE->GetID());
      }
    } else {
      Ids.push_back(Start->GetID());
    }
    
    sort(Ids.begin(), Ids.end());
 
    m_Ids[Start] = Ids;

    return Ids;
  }
}


////////////////////////////////////////////////////////////////////////////////


vector<int> MResponseBase::GetOriginIds(MRESE* RESE)
{
  // Extremely time critical function!

  const int IdOffset = 2;

  map<MRESE*, vector<int> >::iterator RIter = m_OriginIds.find(RESE);

  if (RIter != m_OriginIds.end()) {
    return (*RIter).second;
  } else {
    
    vector<int> Ids = GetReseIds(RESE);

    vector<int> OriginIds;
    OriginIds.reserve(10);

    // Generate sim IDs:
    for (vector<int>::iterator Iter = Ids.begin(); Iter != Ids.end(); ++Iter) {
      unsigned int HTID = (*Iter)-IdOffset;
      if (HTID >= m_SiEvent->GetNHTs()) {
        merr<<"The RESE has higher IDs "<<HTID<<" than the sim file HTs!"<<endl;
        return OriginIds;
      }
      MSimHT* HT = m_SiEvent->GetHTAt(HTID);
      if (HT == 0) {
        merr<<"Hit not found. ID's don't match. Something is badly wrong..."<<endl;
        return OriginIds;
      }

      for (unsigned int o = 0; o < HT->GetNOrigins(); ++o) {
        int Origin = int(HT->GetOriginAt(o));
        if (find(OriginIds.begin(), OriginIds.end(), Origin) == OriginIds.end()) { // not found
          if (Origin >= 1 && 
              m_SiEvent->GetIAAt(Origin-1)->GetProcess() != "INIT" && 
              m_SiEvent->GetIAAt(Origin-1)->GetProcess() != "ANNI") {
            OriginIds.push_back(Origin);
          }
        }
      }
    }

    sort(OriginIds.begin(), OriginIds.end());
    m_OriginIds[RESE] = OriginIds;

    return OriginIds;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseBase::AreIdsInSequence(const vector<int>& Ids)
{
  // Return true if the given Ids are in sequence without holes

  const int IdOffset = 2;

//   for (unsigned int i = 0; i < Ids.size()-1; ++i) {
//     if (Ids[i+1] - Ids[i] != 1) return false;
//   }

  vector<int> Origins;
  for (unsigned int i = 0; i < Ids.size(); ++i) {
    for (unsigned int h = 0; h < m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetNOrigins(); ++h) {
      bool Contained = false;
      for (unsigned int o = 0; o < Origins.size(); ++o) {
        if (Origins[o] == m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetOriginAt(h)) {
          Contained = true;
          break;
        }
      }
      if (Contained == false) {
        Origins.push_back(m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetOriginAt(h));
      }
    }
  }

  double MinTime = numeric_limits<double>::max();
  double MaxTime = -numeric_limits<double>::max();
  for (unsigned int i = 0; i < Ids.size(); ++i) {
    if (m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetTime() > MaxTime) {
      MaxTime = m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetTime();
    }
    if (m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetTime() < MinTime) {
      MinTime = m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetTime();
    }  
  }

  // No hit with a listed origin is allowed to be between min and max time:
  for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
    if (m_SiEvent->GetHTAt(h)->GetTime() > MinTime &&
        m_SiEvent->GetHTAt(h)->GetTime() < MaxTime) {
      for (unsigned int o = 0; o < Origins.size(); ++o) {
        if (m_SiEvent->GetHTAt(h)->IsOrigin(Origins[o]) == true) {
          bool Found = false;
          for (unsigned int i = 0; i < Ids.size(); ++i) {
            if (int(h) == Ids[i]-IdOffset) {
              Found = true;
              break;
            }
          }
          if (Found == false) {
            return false;
          }
        }
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


vector<float> MResponseBase::CreateLogDist(float Min, float Max, int Bins, 
                                           float MinBound, float MaxBound,
                                           float Offset, bool Inverted)
{
  // Create axis with bins in logaritmic distance

  vector<float> Axis;

  if (MinBound != c_NoBound) {
    Axis.push_back(MinBound);
  }

  Min = log(Min);
  Max = log(Max);
  float Dist = (Max-Min)/(Bins);

  for (int i = 0; i < Bins+1; ++i) {
    Axis.push_back(exp(Min+i*Dist));
  }

  if (MaxBound != c_NoBound) {
    Axis.push_back(MaxBound);
  }

  if (Inverted == true) {
    vector<float> Temp = Axis;
    for (unsigned int i = 1; i < Temp.size()-1; ++i) {
      Axis[i] = Axis[i-1] + (Temp[Temp.size()-i]-Temp[Temp.size()-i-1]);
    }
  }

  for (unsigned int i = 0; i < Axis.size(); ++i) {
    Axis[i] += Offset;
  }

  return Axis;
}


////////////////////////////////////////////////////////////////////////////////


vector<float> MResponseBase::CreateEquiDist(float Min, float Max, int Bins, 
                                            float MinBound, float MaxBound,
                                            float Offset, bool Inverted)
{
  // Create axis with bins in logaritmic distance

  vector<float> Axis;

  if (MinBound != c_NoBound) {
    Axis.push_back(MinBound);
  }

  float Dist = (Max-Min)/(Bins);

  for (int i = 0; i < Bins+1; ++i) {
    Axis.push_back(Min+i*Dist);
  }

  if (MaxBound != c_NoBound) {
    Axis.push_back(MaxBound);
  }

  if (Inverted == true) {
    vector<float> Temp = Axis;
    for (unsigned int i = 1; i < Temp.size()-1; ++i) {
      Axis[i] = Axis[i-1] + (Temp[Temp.size()-i]-Temp[Temp.size()-i-1]);
    }
  }

  for (unsigned int i = 0; i < Axis.size(); ++i) {
    Axis[i] += Offset;
  }

  return Axis;
}


// MResponseBase.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
