/*
 * MResponseBuilder.cxx
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
// MResponseBuilder
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseBuilder.h"

// Standard libs:
#include <limits>
#include <algorithm>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MFile.h"
#include "MRawEventIncarnationList.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MResponseBuilder)
#endif


////////////////////////////////////////////////////////////////////////////////


const float MResponseBuilder::c_NoBound = numeric_limits<float>::max()/3.1;


////////////////////////////////////////////////////////////////////////////////


MResponseBuilder::MResponseBuilder()
{
  // Construct an instance of MResponseBuilder

  m_DataFileName = g_StringNotDefined;
  m_GeometryFileName = g_StringNotDefined;
  m_ResponseName = g_StringNotDefined;
  m_ResponseNameSuffix = "";

  m_StartEventID = 0;
  m_MaxNEvents = numeric_limits<unsigned int>::max();
  m_SaveAfter = 10000;

  m_ReReader = nullptr;
  m_SiReader = nullptr;
  m_ReaderFinished = false;
  
  m_ReEvent = nullptr;
  m_SiEvent = nullptr;
  
  m_RevanEventID = 0;
  m_RevanLevel = 0;
  m_SivanEventID = 0;
  m_SivanLevel = 0;
 
  m_SiGeometry = nullptr;
  m_ReGeometry = nullptr;

  m_NumberOfSimulatedEventsClosedFiles = 0;
  m_NumberOfSimulatedEventsThisFile = 0;

  m_Interrupt = false;
  
  m_Suffix = ".rsp";

  m_Mode = MResponseBuilderReadMode::EventByEvent;
  
  m_OnlyINITRequired = false;

  mout<<"Comment to myself:"<<endl;
  mout<<"There is currently a problem in revan that requires coincidence search to be deactivated in order to be able to get a response"<<endl;
  mout<<"There is no easy fix for this..."<<endl;
}


////////////////////////////////////////////////////////////////////////////////


MResponseBuilder::~MResponseBuilder()
{
  // Delete this instance of MResponseBuilder
}


////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseBuilder::Initialize() 
{ 
  // Load revan geometry
  if ((m_ReGeometry = LoadGeometry(true, 0.0)) == 0) return false;
  
  // Create the raw event analyzer
  m_ReReader = new MRawEventAnalyzer();
  m_ReReader->SetGeometry(m_ReGeometry);
  
  
  // Load revan settings file
  if (m_RevanSettingsFileName != g_StringNotDefined && m_RevanSettingsFileName != "") {
    if (m_RevanSettings.Read(m_RevanSettingsFileName) == false) {
      merr<<"Unable to open revan settings file \""<<m_RevanSettingsFileName<<"\""<<endl; 
      return false;
    }
    m_ReReader->SetSettings(&m_RevanSettings);  
  }
  
  if (m_Mode == MResponseBuilderReadMode::File) {
    if (m_ReReader->SetInputModeFile(m_DataFileName) == false) {
      merr<<"Unable to open data file \""<<m_DataFileName<<"\" in revan"<<endl; 
      return false;
    }
  }

  // ... and initialize it
  if (m_ReReader->PreAnalysis() == false) {
    merr<<"Unable to initialize event reconstruction."<<endl;     
    return false;
  }
  
  
  // Load the mimrec configuration file ...
  if (m_MimrecSettingsFileName != g_StringNotDefined && m_MimrecSettingsFileName != "") {
    if (m_MimrecSettings.Read(m_MimrecSettingsFileName) == false) {
      merr<<"Unable to open mimrec settings file \""<<m_MimrecSettingsFileName<<"\""<<endl; 
      return false;
    }
    // ... and initialize the event selector 
    m_MimrecEventSelector.SetSettings(&m_MimrecSettings);
  }
  
  // Load the sivan geometry
  if ((m_SiGeometry = LoadGeometry(false, 0.0)) == 0) return false;

  // and the sivan file reader
  if (m_Mode == MResponseBuilderReadMode::File) {
    m_SiReader = new MFileEventsSim(m_SiGeometry);
    if (m_SiReader->Open(m_DataFileName) == false) {
      merr<<"Unable to open simulation file \""<<m_DataFileName<<"\" in sivan"<<endl; 
      return false;
    }
  }

  m_Counter = 0;
  
  return true; 
}
 

////////////////////////////////////////////////////////////////////////////////


//! Analyze one events
bool MResponseBuilder::Analyze()
{
  if (m_Counter > 0 && m_Counter % m_SaveAfter == 0) {
    if (Save() == false) return false;
  }  
   
  // Nothing to initialize
  if (m_Mode == MResponseBuilderReadMode::File) {
    if (InitializeNextMatchingEvent() == false) return false;
  }
  
  if (m_ReEvent == nullptr) {
    merr<<"No revan event available"<<endl;
    return false;
  }
  
  if (m_SiEvent == nullptr) {
    merr<<"No sivan event available"<<endl;
    return false;
  }
  
  ++m_Counter;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseBuilder::Finalize() 
{ 
  if (Save() == false) return false;
  
  return true; 
}


////////////////////////////////////////////////////////////////////////////////


//! Get the output file prefix
MString MResponseBuilder::GetFilePrefix() const
{
  MString Prefix = m_ResponseName;
  if (m_ResponseNameSuffix != "") {
    Prefix += ".";
    Prefix += m_ResponseNameSuffix;
  }
  
  return Prefix;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Save the response matrices
bool MResponseBuilder::Save() 
{ 
  if (m_RevanSettingsFileName != "") {
    m_RevanSettings.Write(GetFilePrefix() + ".revan.cfg");
  }
  if (m_MimrecSettingsFileName != "") {
    m_MimrecSettings.Write(GetFilePrefix() + ".mimrec.cfg");
  }
  
  return false; 
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseBuilder::SetEvent(const MString String, bool NeedsNoising, int Version)
{
  // Set and verify the simulation file name

  m_Mode = MResponseBuilderReadMode::EventByEvent;

  if (m_ReReader->AddRawEvent(String, NeedsNoising, Version) == false) {
    cout<<"Unable to add raw event"<<endl;
    return false;
  }
  
  m_ReReader->AnalyzeEvent();
  // automatically deleted: m_ReEvent and the content in m_ReEvents
  m_ReEvent = nullptr;
  m_ReEvents.clear();
  if (m_ReReader->GetRawEventList() != nullptr) {
    MRawEventIncarnationList* REIL = m_ReReader->GetRawEventList();
    for (unsigned int i = 0; i < REIL->Size(); ++i) {
      if (REIL->Get(i)->GetNRawEvents() > 0) {
        m_ReEvents.push_back(REIL->Get(i)->GetRawEventAt(0)); // why not the optimum event?
      }
    }
  }
  if (m_ReEvents.size() > 0) m_ReEvent = m_ReEvents[0];
  
  if (m_ReEvent == nullptr) {
    cout<<"We have no good raw event"<<endl;
    return false;
  }
  
  if (m_SiEvent == nullptr) {
    m_SiEvent = new MSimEvent();
    m_SiEvent->SetGeometry(m_SiGeometry);
  }
  if (m_SiEvent->ParseEvent(String, Version) == false) {
    cout<<"Unable to add event to sivan"<<endl;
    return false;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseBuilder::SetDataFileName(const MString FileName)
{
  // Set and verify the simulation file name

  if (MFile::Exists(FileName) == false) {
    mout<<"*** Error: \""<<FileName<<"\" does not exist"<<endl;
    return false;
  }
  m_DataFileName = FileName;

  m_Mode = MResponseBuilderReadMode::File;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseBuilder::SetGeometryFileName(const MString FileName)
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


void MResponseBuilder::SetResponseName(const MString Name)
{
  // Set the response name (used for the file name suffix)

  m_ResponseName = Name;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseBuilder::SetRevanSettingsFileName(const MString FileName)
{
  // Set and verify the simulation file name

  if (MFile::Exists(FileName) == false) {
    mout<<"*** Error: \""<<FileName<<"\" does not exist"<<endl;
    return false;
  }
  m_RevanSettingsFileName = FileName;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseBuilder::SetMimrecSettingsFileName(const MString FileName)
{
  // Set and verify the simulation file name

  if (MFile::Exists(FileName) == false) {
    mout<<"*** Error: \""<<FileName<<"\" does not exist"<<endl;
    return false;
  }
  m_MimrecSettingsFileName = FileName;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MGeometryRevan* MResponseBuilder::LoadGeometry(bool ActivateNoise, double GlobalFailureRate)
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


bool MResponseBuilder::InitializeNextMatchingEvent()
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
      m_ReEvent = nullptr;

      // Load/Analyze
      ERReturnCode = m_ReReader->AnalyzeEvent();
      if (ERReturnCode == MRawEventAnalyzer::c_AnalysisNoEventsLeftInFile) {
        //mout<<"Response: No more events available in file!"<<endl;
        m_ReaderFinished = true;
        MoreEvents = false;
        break;
      }

      //if (m_ReReader->GetRawEventList() != 0) {
      //  if (m_ReReader->GetRawEventList()->GetNRawEvents() > 0) {
      //    m_ReEvent = m_ReReader->GetRawEventList()->GetRawEventAt(0);
      //  }
      //}
      
      m_ReEvents.clear();
      if (m_ReReader->GetRawEventList() != nullptr) {
        MRawEventIncarnationList* REIL = m_ReReader->GetRawEventList();
        for (unsigned int i = 0; i < REIL->Size(); ++i) {
          if (REIL->Get(i)->GetNRawEvents() > 0) {
            m_ReEvents.push_back(REIL->Get(i)->GetRawEventAt(0)); // why not the optimum event?
          }
        }
      }
      if (m_ReEvents.size() > 0) m_ReEvent = m_ReEvents[0];
      
      
      
      // Decide future:
      if (m_ReEvent != nullptr && m_ReEvent->GetEventType() != MRERawEvent::c_PairEvent) {
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
      m_SiEvent = nullptr;
      
      // Read:
      m_SiEvent = m_SiReader->GetNextEvent(false);

      // Decide:
      if (m_SiEvent != nullptr) {  
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
          mout<<"Response: Sivan found NO good event (Id="<<m_SiEvent->GetID()<<") TRNC or not enough IAs!"<<endl;
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


bool MResponseBuilder::SanityCheckSimulations()
{
  // Do a sanity check if the simulation is ok
  // E.g. all remaining clusters need to have on sim origin != 1 !!
 
  // There is nothing to sanity check...
  if (m_OnlyINITRequired == true) return true;
  
  if (m_SiEvent->GetNIgnoredHTs() > 0) {
    mout<<"Response sanity check (event "<<m_SiEvent->GetID()<<"): We have ignored HTs in the sim file -> "<<
      " check your geometry, because noising was deactivated!"<<endl;
    return false;
  }

  for (auto RE: m_ReEvents) {  
    //cout<<"Sanitycheck"<<endl;

    if (RE->GetVertex() != 0) continue;

    if (int(m_SiEvent->GetNHTs()) < RE->GetNRESEs()) {
      mout<<"Response sanity check (event "<<m_SiEvent->GetID()<<"): The simulation has less hits than the raw event!!!"<<endl;
      return false;
    }

    for (int i = 0; i < RE->GetNRESEs(); ++i) {
      MRESE* RESE = RE->GetRESEAt(i);
      //cout<<"SC: "<<(long) RESE<<endl;
      
      vector<int> EndOriginIds = GetOriginIds(RESE);

      // Check that all origin IDs are > 2, i.e. belong to a process and not an init
      bool HasOriginIDs = false;
      for (unsigned int i = 0; i < EndOriginIds.size(); ++i) {
        if (EndOriginIds[i] <= 1) {
          mout<<"Response sanity check (event "<<m_SiEvent->GetID()<<"): One of the HT's has the following non-compliant origin ID: "<<EndOriginIds[i]<<endl;
          return false;
        } else {
          HasOriginIDs = true;
        }
      }
      if (HasOriginIDs == false) {
        mout<<"Response sanity check (event "<<m_SiEvent->GetID()<<"): One of the HT's has no origin IDs: Your simulations most like do not contain the full required IA information block!"<<endl;
        return false;       
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


vector<int> MResponseBuilder::GetReseIds(MRESE* Start)
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


vector<int> MResponseBuilder::GetOriginIds(MRESE* RESE)
{
  // Extremely time critical function!

  // ID offset between hits in revan and sivan
  const int IdOffset = 2;

  // First check if it already exits
  map<MRESE*, vector<int> >::iterator RIter = m_OriginIds.find(RESE);
  if (RIter != m_OriginIds.end()) {
    return (*RIter).second;
  } else {
    // If not find them...
    
    // Get the revan RESE Ids
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
            m_SiEvent->GetIAAt(Origin-1)->GetProcess() != "ANNI" && 
            m_SiEvent->GetIAAt(Origin-1)->GetProcess() != "DECA") {
            OriginIds.push_back(Origin);
          }
        }
      }
    }

    // ... finally store them
    sort(OriginIds.begin(), OriginIds.end());
    m_OriginIds[RESE] = OriginIds;

    return OriginIds;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseBuilder::AreIdsInSequence(const vector<int>& Ids)
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


//! Determine the original mother ID of the gamma ray: mother is either INIT, ANNI, or DECA -- and not: BREM, PHOT, IONI, COMP 
vector<int> MResponseBuilder::GetMotherIds(const vector<int>& AllSimIds)
{
  // 
  
  vector<int> MotherIDs;
  
  for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
    int ID = AllSimIds[i];
    int HighestOriginID = ID;
    while (true) {
      // Get new origin ID
      ID = m_SiEvent->GetIAAt(HighestOriginID-1)->GetOriginID();

      // Check if we are done
      bool IsGood = false;
      if (m_SiEvent->GetIAAt(ID-1)->GetProcess() == "INIT") {
        IsGood = true;
      } else if (m_SiEvent->GetIAAt(ID-1)->GetProcess() == "ANNI") {
        IsGood = true;
      } else if (m_SiEvent->GetIAAt(ID-1)->GetProcess() == "DECA") {
        IsGood = true;
      } else if (m_SiEvent->GetIAAt(ID-1)->GetProcess() == "BREM") {
        IsGood = false;
      } else if (m_SiEvent->GetIAAt(ID-1)->GetProcess() == "COMP") {
        IsGood = false;
      } else if (m_SiEvent->GetIAAt(ID-1)->GetProcess() == "PAIR") {
        IsGood = false;
      } else if (m_SiEvent->GetIAAt(ID-1)->GetProcess() == "PHOT") {
        IsGood = false;
      } else if (m_SiEvent->GetIAAt(ID-1)->GetProcess() == "IONI") {
        IsGood = false;
      } else {
        mout<<"Error: Unhandled process: "<<m_SiEvent->GetIAAt(ID-1)->GetProcess()<<endl;
        mout<<"       Make sure you run the simulation only with a EM physics list, not a hadronics physics list!"<<endl;
        mout<<"       This event cannot be handled correctly!"<<endl;
        IsGood = false; 
      }
      
      HighestOriginID = ID;
      
      if (IsGood == true) {
        if (find(MotherIDs.begin(), MotherIDs.end(), HighestOriginID) == MotherIDs.end()) {
          MotherIDs.push_back(HighestOriginID); 
        }
        break;
      }
    }
  }

  
  return MotherIDs;
}


////////////////////////////////////////////////////////////////////////////////


void MResponseBuilder::Shuffle(vector<MRESE*>& RESEs)
{
  //! Shuffle the RESEs around...
  
  unsigned int size = RESEs.size();
  for (unsigned int i = 0; i < 2*size; ++i) {
    unsigned int From = gRandom->Integer(size);
    unsigned int To = gRandom->Integer(size);
    MRESE* Temp = RESEs[To];
    RESEs[To] = RESEs[From];
    RESEs[From] = Temp;
  }
}

////////////////////////////////////////////////////////////////////////////////


vector<float> MResponseBuilder::CreateLogDist(float Min, float Max, int Bins, 
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


vector<float> MResponseBuilder::CreateThresholdedLogDist(float Min, float Max, int Bins, float Threshold, 
                                                      float MinBound, float MaxBound,
                                                      float Offset, bool Inverted)
{
  // Create axis with bins in logaritmic distance

  vector<float> Axis;

  if (MinBound != c_NoBound) {
    Axis.push_back(MinBound);
  }

  if (Min <= 0) {
    merr<<"Minimum has to be positive, setting it to 1"<<endl;
    Min = 1;
  }
  
  if (Threshold >= (Max - Min)/Bins) {
    merr<<"Threshold has to be smaller than: (Max - Min)/NBins. Setting it to this value -> bins will be equally spaced instead of logarithmically spaced."<<endl;
    Threshold = (Max - Min)/Bins;
  }
  
  Min = log(Min);
  Max = log(Max - Bins*Threshold);
  float Dist = (Max-Min)/(Bins);

  for (int i = 0; i < Bins+1; ++i) {
    Axis.push_back(exp(Min+i*Dist) + i*Threshold);
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


vector<float> MResponseBuilder::CreateEquiDist(float Min, float Max, int Bins, 
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


// MResponseBuilder.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
