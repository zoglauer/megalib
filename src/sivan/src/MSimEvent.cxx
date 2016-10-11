/*
 * MSimEvent.cxx
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
// MSimEvent
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSimEvent.h"

// Standard libs:
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <limits>
#include <algorithm>
using namespace std;

// ROOT libs:
#include <TMath.h>

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MDDetector.h"
#include "MDACS.h"
#include "MDStrip2D.h"
#include "MDStrip3D.h"
#include "MDCalorimeter.h"
#include "MDDriftChamber.h"
#include "MDVolume.h"
#include "MDGridPointCollection.h"
#include "MDGridPoint.h"
#include "MComptonEvent.h"
#include "MPairEvent.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MSimEvent)
#endif


// This number has to be increased, whenever there is a significant change
// in any of the default output ToSimString(...) of the Sim classes 
const int MSimEvent::s_Version = 25;

// Some constants indicating what to store
const int MSimEvent::c_StoreSimulationInfoAll = 0;
const int MSimEvent::c_StoreSimulationInfoDepositsOnly = 1;
const int MSimEvent::c_StoreSimulationInfoInitOnly = 2;
const int MSimEvent::c_StoreSimulationInfoNone = 3;


////////////////////////////////////////////////////////////////////////////////


MSimEvent::MSimEvent()
{
  // Default constructor

  m_Geometry = 0;
 
  m_NEvent = 0;
  m_NStartedEvent = 0;

  m_StoreSimulationData = false;
  
  Reset();
}


////////////////////////////////////////////////////////////////////////////////


MSimEvent::MSimEvent(const MSimEvent& Event)
{
  // Default constructor

  m_Geometry = Event.m_Geometry;

  for (unsigned int i = 0; i < Event.m_IAs.size(); ++i) {
    MSimIA* IA = new MSimIA(*Event.m_IAs[i]);
    m_IAs.push_back(IA);
  }

  for (unsigned int i = 0; i < Event.m_HTs.size(); ++i) {
    MSimHT* HT = new MSimHT(*Event.m_HTs[i]);
    m_HTs.push_back(HT);
  }

  for (unsigned int i = 0; i < Event.m_DRs.size(); ++i) {
    MSimDR* DR = new MSimDR(*Event.m_DRs[i]);
    m_DRs.push_back(DR);
  }

  for (unsigned int i = 0; i < Event.m_Clusters.size(); ++i) {
    MSimCluster* C = new MSimCluster(*Event.m_Clusters[i]);
    m_Clusters.push_back(C);
  }

  for (unsigned int i = 0; i < Event.m_GRs.size(); ++i) {
    MSimGR* GR = new MSimGR(*Event.m_GRs[i]);
    m_GRs.push_back(GR);
  }

  for (unsigned int i = 0; i < Event.m_PMs.size(); ++i) {
    MSimPM* PM = new MSimPM(*Event.m_PMs[i]);
    m_PMs.push_back(PM);
  }

  m_NEvent = Event.m_NEvent;
  m_NStartedEvent = Event.m_NStartedEvent;

  m_Time = Event.m_Time;
  m_TOF = Event.m_TOF;

  m_Veto = Event.m_Veto;
  m_TotalDetectorEnergy = Event.m_TotalDetectorEnergy;
  m_StoreSimulationData = Event.m_StoreSimulationData;
  m_SimulationData = Event.m_SimulationData;
  m_EventType = Event.m_EventType;
  m_EventLocation = Event.m_EventLocation;
  m_REventRetrieval = Event.m_REventRetrieval;
  m_IEventRetrieval = Event.m_IEventRetrieval;
  m_Version = Event.m_Version;
  m_NIgnoredHTs = Event.m_NIgnoredHTs;
}


////////////////////////////////////////////////////////////////////////////////


MSimEvent::~MSimEvent()
{
  // default destructor

  for (unsigned int i = 0; i < m_IAs.size(); ++i) delete m_IAs[i];
  for (unsigned int i = 0; i < m_HTs.size(); ++i) delete m_HTs[i];
  for (unsigned int i = 0; i < m_DRs.size(); ++i) delete m_DRs[i];
  for (unsigned int i = 0; i < m_GRs.size(); ++i) delete m_GRs[i];
  for (unsigned int i = 0; i < m_PMs.size(); ++i) delete m_PMs[i];
  for (unsigned int i = 0; i < m_Clusters.size(); ++i) delete m_Clusters[i];
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::Add(const MSimEvent& Event)
{
  // Add another simulated event...

  // This is bad, but I must have had a good reason to do it this way...
  MSimEvent& Local = const_cast<MSimEvent&>(Event);

  if (m_Geometry == 0) {
    m_Geometry = Local.m_Geometry;
  }

  // If they are from different geometry, stop
  if (Local.m_Geometry != 0 && (Local.m_Geometry != m_Geometry || Local.m_Geometry->GetName() != m_Geometry->GetName())) {
    return false;
  }
  
  int Offset = GetNIAs();

  for (unsigned int i = 0; i < Local.GetNIAs(); ++i) {
    MSimIA* IA = new MSimIA(*Local.GetIAAt(i));
    IA->OffsetOrigins(Offset);
    m_IAs.push_back(IA);
  }

  for (unsigned int i = 0; i < Local.GetNHTs(); ++i) {
    MSimHT* HT = new MSimHT(*Local.GetHTAt(i));
    HT->OffsetOrigins(Offset);
    m_HTs.push_back(HT);
  }

  for (unsigned int i = 0; i < Local.GetNDRs(); ++i) {
    MSimDR* DR = new MSimDR(*Local.GetDRAt(i));
    m_DRs.push_back(DR);
  }

  for (unsigned int i = 0; i < Local.GetNGRs(); ++i) {
    MSimGR* GR = new MSimGR(*Local.GetGRAt(i));
    m_GRs.push_back(GR);
  }

  for (unsigned int i = 0; i < Local.GetNPMs(); ++i) {
    MSimPM* PM = new MSimPM(*Local.GetPMAt(i));
    m_PMs.push_back(PM);
  }

  map<MVector, double>::const_iterator Iter;
  for (Iter = Event.m_TotalDetectorEnergy.begin();
       Iter != Event.m_TotalDetectorEnergy.end(); 
       Iter++) {
    m_TotalDetectorEnergy[(*Iter).first] += (*Iter).second; 
  }

  if (Local.m_Veto == true) m_Veto = true;
  m_SimulationData += Local.m_SimulationData;

  mimp<<"Add is not complete!"<<show;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::AddHT(const MSimHT& HT)
{
  // Add an hit - no error checks are performed...

  MSimHT* pHT = new MSimHT(HT); 
  m_HTs.push_back(pHT);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::AddIA(const MSimIA& IA)
{
  // Add an interaction - no error checks are performed...

  MSimIA* pIA = new MSimIA(IA); 
  m_IAs.push_back(pIA);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::AddDR(const MSimDR& DR)
{
  // Add an guard ring hit - no error checks are performed...

  MSimDR* pDR = new MSimDR(DR); 
  m_DRs.push_back(pDR);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::AddGR(const MSimGR& GR)
{
  // Add an guard ring hit - no error checks are performed...

  MSimGR* pGR = new MSimGR(GR); 
  m_GRs.push_back(pGR);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::AddPM(const MSimPM& PM)
{
  // Add an guard ring hit - no error checks are performed...

  MSimPM* pPM = new MSimPM(PM); 
  m_PMs.push_back(pPM);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::AddBD(const MString& BD)
{
  // Add a bad event flag - but only if it does not yet exist
  
  for (unsigned int b = 0; b < m_BDs.size(); ++b) {
    if (m_BDs[b] == BD) return true; 
  }

  m_BDs.push_back(BD);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::AddCC(const MString& CC)
{
  // Add a comment
  
  m_CCs.push_back(CC);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSimEvent::SetGeometry(MDGeometryQuest* Geo)
{
  m_Geometry = Geo;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::ParseEvent(MString Line, int Version)
{
  // Parse the whole event at once
  
  Reset();
  
  vector<MString> Lines = Line.Tokenize("\n");
  
  for (MString L: Lines) {
    if (ParseLine(L, Version) == false) {
      return false; 
    }
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::ParseLine(MString LineBuffer, int Version)
{
  // Analyze one line of text input...
  // Return true if all event data is available
  //
  // We have the following types:
  // SE (int) event number; (int) started event number
  // IA
  // HTsim
  // NS
  // VT
  // TF
  // ES

  bool Ret = true;

  m_Version = Version;

  if (m_StoreSimulationData == true) {
    m_SimulationData += LineBuffer;
    m_SimulationData += '\n';
  }

  if (LineBuffer.Length() < 2) {
    return true;
  }

  // Start Event:
  if (LineBuffer[0] == 'I' && LineBuffer[1] == 'D') {
    if (sscanf(LineBuffer.Data(),"ID %d %d\n",&m_NEvent, &m_NStartedEvent) != 2) {
      if (sscanf(LineBuffer.Data(),"ID %d\n",&m_NEvent) != 1) {
        mout<<"Error during scanning of sim file in token ID!"<<endl;
        mout<<"  "<<LineBuffer<<endl;
        Ret = false;
      } else {
        m_NStartedEvent = m_NEvent;
      }
    }
    Reset();
  }
  // Add an interaction
  else if (LineBuffer[0] == 'I' && LineBuffer[1] == 'A') {
    MSimIA* IA = new MSimIA();
    if (IA->AddRawInput(LineBuffer, Version) == true) {
      m_IAs.push_back(IA);
    } else {
      mout<<"Error during scanning of sim file in token IA:"<<endl;
      mout<<"  "<<LineBuffer<<endl;
      Ret = false;
      delete IA;
    }
  }
//   // Add deposited energy in not sensitive material:
//   else if (LineBuffer[0] == 'N' && LineBuffer[1] == 'S') {
//     if (sscanf(LineBuffer,"NS%lf\n", &m_EnergyNS) != 1) {
//       mout<<"Error during scanning of sim file in token NS:"<<endl;
//       mout<<"  "<<LineBuffer<<endl;
//       Ret = false;
//     }
//   }
  // Add deposited energy in not sensitive material:
  else if (LineBuffer[0] == 'P' && LineBuffer[1] == 'M') {
    MSimPM* PM = new MSimPM();
    if (PM->AddRawInput(LineBuffer, Version) == true) {
      m_PMs.push_back(PM);
    } else {
      mout<<"Error during scanning of sim file in token GR:"<<endl;
      mout<<"  "<<LineBuffer<<endl;
      Ret = false;
      delete PM;
    }
  }
  // Add deposited energy in guard ring:
  else if (LineBuffer[0] == 'G' && LineBuffer[1] == 'R') {
    MSimGR* GR = new MSimGR(m_Geometry);
    if (GR->AddRawInput(LineBuffer, Version) == true) {
      m_GRs.push_back(GR);
    } else {
      //mout<<"Error during scanning of sim file in token GR:"<<endl;
      //mout<<"  "<<LineBuffer<<endl;
      Ret = false;
      delete GR;
    }
  }
  // Add bad event flags
  else if (LineBuffer[0] == 'B' && LineBuffer[1] == 'D') {
    istringstream iss(LineBuffer.GetString());
    string s;
    while (getline(iss, s, ' ')) {
      if (s != "BD" && s != " " && s != "") {
        m_BDs.push_back(s);
      }
    }
  }
  // Add comment
  else if (LineBuffer[0] == 'C' && LineBuffer[1] == 'C') {
    m_CCs.push_back(LineBuffer.GetSubString(3)); // Don't safe the inital "CC " 
  }
  // Add total detector energy:
  else if (LineBuffer[0] == 'X' && LineBuffer[1] == 'E') {
    double x, y, z, E;
    if (sscanf(LineBuffer.Data(), "XE %lf;%lf;%lf;%lf\n", &x, &y, &z, &E) != 4) {
      mout<<"Error during scanning of sim file in token XE:"<<endl;
      mout<<"  "<<LineBuffer<<endl;
      Ret = false;
    }
    m_TotalDetectorEnergy[MVector(x, y, z)] += E;
  }
  // Add the time:
  else if (LineBuffer[0] == 'T' && LineBuffer[1] == 'I') {
    if (m_Time.Set(LineBuffer) == false) {
      mout<<"Error during scanning of sim file in token TI:"<<endl;
      mout<<"  "<<LineBuffer<<endl;
      Ret = false;
    }
  }
  // Add TOF:
  else if (LineBuffer[0] == 'T' && LineBuffer[1] == 'F') {
    if (sscanf(LineBuffer.Data(), "TF%lf\n",&m_TOF) != 1) {
      mout<<"Error during scanning of sim file in token TF:"<<endl;
      mout<<"  "<<LineBuffer<<endl;
      Ret = false;
    }
  }
  // Add Veto:
  else if (LineBuffer[0] == 'V' && LineBuffer[1] == 'T') {
    double E1, E2, E3, E4;
    if (sscanf(LineBuffer.Data(), "VT%lf;%lf;%lf;%lf\n", &E1, &E2, &E3, &E4) == 4) {
      if (E1 > 100 || E2 > 100 || E3 > 100 || E4 > 100) {
        m_Veto = true;
      } else {
        m_Veto = false;
      }
    } else {
      m_Veto = true;
    }
  }
  // Add a hit
  else if (LineBuffer[0] == 'H' && LineBuffer[1] == 'T') {
    MSimHT* HT = new MSimHT(m_Geometry);
    if (HT->AddRawInput(LineBuffer, Version) == true) {
      m_HTs.push_back(HT);
    } else {
      // No error message here, because false also means below threshold
      // mout<<"Error during scanning of sim file in token HT!"<<endl;
      // mout<<"  "<<LineBuffer<<endl;
      Ret = false;
      m_NIgnoredHTs++;
      delete HT;
    }
  }
  // Add a DR statement
  else if (LineBuffer[0] == 'D' && LineBuffer[1] == 'R') {
    MSimDR* DR = new MSimDR();
    if (DR->AddRawInput(LineBuffer, Version) == true) {
      m_DRs.push_back(DR);
    } else {
      mout<<"Error during scanning of sim file in token DR!"<<endl;
      Ret = false;
      delete DR;
    }
  }
  // Add the coordinate system 
  else if ((LineBuffer[0] == 'R' && (LineBuffer[1] == 'X' || LineBuffer[1] == 'Z')) ||
           (LineBuffer[0] == 'H' && (LineBuffer[1] == 'X' || LineBuffer[1] == 'Z')) ||
           (LineBuffer[0] == 'G' && (LineBuffer[1] == 'X' || LineBuffer[1] == 'Z'))) {
    MRotationInterface::ParseLine(LineBuffer);
  }
  
  return Ret;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::Analyze()
{

  if (CheckEvent() == false) {
    //cout<<ToString()<<endl;
    Error("bool MSimEvent::AddRawInput(char* LineBuffer)",
          "Something went wrong during read of event. Skipping event...");
    Reset();
    return false;
  }
  
  // Test the trigger condition of the simulation
  MDTriggerUnit* Trigger = m_Geometry->GetTriggerUnit();
  Trigger->Reset();
  for (unsigned int h = 0; h < GetNHTs(); ++h) {
    Trigger->AddHit(GetHTAt(h)->GetPosition(), GetHTAt(h)->GetEnergy());
  }
  if (Trigger->HasTriggered() == false) {
    mdebug<<"MSimEvent: Event "<<GetID()<<" has not triggered!"<<endl;
    for (unsigned int h = 0; h < m_HTs.size(); ++h) delete m_HTs[h];
    m_HTs.clear();
  }

  DetermineEventType();
  DetermineEventLocation();
  DetermineEventRetrieval();

  return true; 
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::CheckEvent()
{
  // Check the event if it has been read correctly, i.e. if there are hits
  // and no interactions, something is wrong...
  //
  // Not all possible types of errors are checked...

//   if (GetNIAs() == 0) {
//     mout<<"Event has no interactions!"<<endl;
//     return false;
//   }
//   if (GetIAAt(0)->GetOriginID() != 0) {
//     mout<<"First IA has no origin!"<<endl;
//     return false;
//   }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSimEvent::DetermineEventType()
{
  // Determine the event type

  if (GetNIAs() <= 1)  {
    m_EventType = Unknown;
    return;
  }

  if (GetIAAt(1)->GetProcess() == "COMP") {
    m_EventType = Compton;
  }
  // Pair event:
  else if (GetIAAt(1)->GetProcess() == "PAIR") {
    m_EventType = Pair;
  }
  // Unkown event:
  else {
    m_EventType = Unknown;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MSimEvent::DetermineEventLocation()
{
  // Determine the place of the first interaction:

  if (GetNIAs() <= 1)  {
    m_EventLocation = NotSensitive;
    return;
  }

  // First interaction in D1:
  if (GetIAAt(1)->GetDetectorType() == 1) {
    m_EventLocation = D1;
  } 
  // First interaction in D2:
  else if (GetIAAt(1)->GetDetectorType() == 2) {
    m_EventLocation = D2;
  } 
  // First interaction in D3:
  else if (GetIAAt(1)->GetDetectorType() == 3) {
    m_EventLocation = D3;
  } 
  // First interaction in D3:
  else if (GetIAAt(1)->GetDetectorType() == 4) {
    m_EventLocation = D4;
  } 
  // First interaction in D3:
  else if (GetIAAt(1)->GetDetectorType() == 5) {
    m_EventLocation = D5;
  } 
  // First interaction in D3:
  else if (GetIAAt(1)->GetDetectorType() == 6) {
    m_EventLocation = D6;
  } 
  // First interaction in D3:
  else if (GetIAAt(1)->GetDetectorType() == 7) {
    m_EventLocation = D7;
  } 
  // First interaction in D3:
  else if (GetIAAt(1)->GetDetectorType() == 8) {
    m_EventLocation = D8;
  } 
  // Unkown location
  else {
    m_EventLocation = NotSensitive;
  } 
}


////////////////////////////////////////////////////////////////////////////////


MVector MSimEvent::GetIPVertex()
{
  return GetIAAt(1)->GetPosition();
}


////////////////////////////////////////////////////////////////////////////////


MVector MSimEvent::GetIPElectronDir()
{
  return GetIAAt(1)->GetSecondaryDirection();
}


////////////////////////////////////////////////////////////////////////////////


MVector MSimEvent::GetIPPositronDir()
{
  return GetIAAt(2)->GetSecondaryDirection();
}


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetIPElectronEnergy()
{
  return GetIAAt(1)->GetSecondaryEnergy();
}


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetIPPositronEnergy()
{
  return GetIAAt(2)->GetSecondaryEnergy();
}


////////////////////////////////////////////////////////////////////////////////


void MSimEvent::DetermineEventRetrieval()
{
  // Determine if the ideal event can be retrieved, i.e.
  // if the first two interactions come from the initial gamma-ray
  // and 
  // Determine if the real event can be retrieved, i.e.
  // If there are hits from the first and the second IA

  unsigned int i, j;

  // Start with the hpythesis everything's ok: 
  m_IEventRetrieval = IdealRetrievalGood;
  m_REventRetrieval = RealRetrievalGood;


  if (m_EventType == Compton) {
    // Ideal case:
    // The first three interactions have to be from the initial gamma-ray
    
    if (GetNIAs() < 3) {
      m_IEventRetrieval = IdealRetrievalBad;
      //cout<<"Event Retrieval: Not enough IAs: "<<GetNIAs()<<endl;
      //massert(false);
    } else {
      // Test if their origin is 1
      if (GetIAAt(1)->GetOriginID() != 1 || GetIAAt(2)->GetOriginID() != 1) {
        m_IEventRetrieval = IdealRetrievalBad;
        //cout<<"Event Retrieval: Wrong IA origin: "<<GetIAAt(1)->GetOriginID()<<endl;
      } else {
        //It is a good one now ... I believe!
      }
      if (GetIAAt(1)->GetProcess() == "ESCP" || GetIAAt(2)->GetProcess() == "ESCP") {
        m_IEventRetrieval = IdealRetrievalBad;
        //cout<<"Event Retrieval: We cannot have an escape in the first two intercations."<<endl;
      }
    }
    
    // Real case:
    if (m_IEventRetrieval == IdealRetrievalBad) {
      m_REventRetrieval = RealRetrievalBad;
    } else {
      // there has to be at least one hit with origin IA2 and one with IA3
      // if not the hit was in insensitive material or fell below the energy threshold
      j = 0;
      for (i = 0; i < GetNHTs(); ++i) {
        if (GetHTAt(i)->IsOrigin(2) == true) {
          //cout<<"Event Retrieval: Origin 2 found"<<endl;
          ++j;
          break;
        }
      }
      for (i = 0; i < GetNHTs(); ++i) {
        if (GetHTAt(i)->IsOrigin(3) == true) {
          //cout<<"Event Retrieval: Origin 3 found"<<endl;
          ++j;
          break;
        }
      }
      
      if (j != 2) {
        m_REventRetrieval = RealRetrievalBad;
        //cout<<"Event Retrieval: Wrong hit origin: "<<j<<endl;
      } else {
        //It is a good one now ... I believe!
      }

      
      // Now check for clustering - the first and the second hit are not allowed 
      // to be in the same cluster!
      CreateClusters();
      if (GetRCFirstIAClustered() == GetRCSecondIAClustered() ||
          GetRCFirstIA() == GetRCSecondIA()) {
        m_REventRetrieval = RealRetrievalBad;
        
        //cout<<"Event Retrieval: Bad clusters!"<<endl;
        /*      
        cout<<GetRCFirstIAClustered().X()<<"!"<<GetRCFirstIAClustered().Y()<<"!"<<GetRCFirstIAClustered().Z()<<endl;
        cout<<GetRCSecondIAClustered().X()<<"!"<<GetRCSecondIAClustered().Y()<<"!"<<GetRCSecondIAClustered().Z()<<endl;
        cout<<GetRCFirstIA().X()<<"!"<<GetRCFirstIA().Y()<<"!"<<GetRCFirstIA().Z()<<endl;
        cout<<GetRCSecondIA().X()<<"!"<<GetRCSecondIA().Y()<<"!"<<GetRCSecondIA().Z()<<endl;
        cout<<GetSimulationData()<<endl;
        */
      }
    }
  } else if (m_EventType == Pair) {
    if (GetNIAs() < 3) {
      m_IEventRetrieval = IdealRetrievalBad;
      //cout<<"Event Retrieval: Not enough IAs: "<<GetNIAs()<<endl;
    } else {
      // Test if their origin is 1
      if (GetIAAt(1)->GetOriginID() != 1 || GetIAAt(2)->GetOriginID() != 1) {
        m_IEventRetrieval = IdealRetrievalBad;
        //cout<<"Event Retrieval: Wrong IA origin: "<<GetIAAt(1)->GetOriginID()<<endl;
      } else {
        //It is a good one now ... I believe!
      }
    }
    
    // Real case:
    if (m_IEventRetrieval == IdealRetrievalBad) {
      m_REventRetrieval = RealRetrievalBad;
    } else {
      // there has to be at least one hit with origin IA2 and one with IA3
      // if not the hit was in insensitive material or fell below the energy threshold
      j = 0;
      for (i = 0; i < GetNHTs(); ++i) {
        if (GetHTAt(i)->IsOrigin(2) == true) {
          //cout<<"Event Retrieval: Origin 2 found"<<endl;
          ++j;
          break;
        }
      }
      for (i = 0; i < GetNHTs(); ++i) {
        if (GetHTAt(i)->IsOrigin(3) == true) {
          //cout<<"Event Retrieval: Origin 3 found"<<endl;
          ++j;
          break;
        }
      }
      
      if (j != 2) {
        m_REventRetrieval = RealRetrievalBad;
        //cout<<"Event Retrieval: Wrong hit origin: "<<j<<endl;
      } else {
        //It is a good one now ... I believe!
      }

      
      // Now check for clustering - the first and the second hit are not allowed 
      // to be in the same cluster!
      CreateClusters();
      if (GetRPElectronD() == MVector(0.0, 0.0, 0.0) || 
          GetRPPositronD() == MVector(0.0, 0.0, 0.0)) {
        m_REventRetrieval = RealRetrievalBad;
        //cout<<"Event Retrieval: One direction is missing!"<<endl;
      }

    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MSimEvent::CreateClusters()
{
  // Here we create clusters out of neighboring pixels which can not be 
  // resolved in a realistic reconstruction environment...

  mimp<<"Call of AreNear() is suspicious..."<<show;

  if (m_Geometry != 0) {

    unsigned int i, j, k;
    MSimCluster* C = 0;
    MSimCluster* CC = 0;
    vector<MSimCluster*>::iterator ClusterIter;

    MDVolumeSequence* VS1 = 0;
    MDVolumeSequence* VS2 = 0;
    for (i = 0; i < GetNHTs(); ++i) {
      if (GetHTAt(i)->GetCluster() == 0) {
        C = new MSimCluster();
        C->AddHT(GetHTAt(i));
        GetHTAt(i)->SetCluster(C);
        AddCluster(C);
      }
      //cout<<endl<<endl;
      VS1 = GetHTAt(i)->GetVolumeSequence();
      //if (VS1 == 0) continue;
      if (VS1->GetDetectorVolume() == 0) continue;
      //cout<<"Checking "<<GetHTAt(i)->ToString()<<" ..."<<endl;
      //cout<<VS1->ToString()<<endl;
      for (j = i+1; j < GetNHTs(); ++j) {
        VS2 = GetHTAt(j)->GetVolumeSequence();
        //if (VS2 == 0) continue;
        if (VS2->GetDetectorVolume() == 0) continue;
        //cout<<VS2->ToString()<<endl;
        //cout<<"DTypes: "<<VS1->GetDetectorType()->GetName()<<"!"<<VS2->GetDetectorType()->GetName()<<endl;
//         if (VS1->GetDetectorVolume()->GetName().CompareTo(VS2->GetDetectorVolume()->GetName()) == 0 &&
//             GetHTAt(i)->GetDetectorType() == GetHTAt(j)->GetDetectorType()) {
        if (VS1->HasSameDetector(VS2) == true) {
          //cout<<" ... with "<<GetHTAt(j)->ToString()<<":"<<endl;
          if (VS1->GetDetector()->AreNear(VS1->GetPositionInDetector(), MVector(0, 0, 0),
                                          VS2->GetPositionInDetector(), MVector(0, 0, 0), 0, 3) == true) {
            //cout<<"Are beside!!"<<endl;
            // Ok, they have to be clustered...


            // Check if second has already a cluster...
            if (GetHTAt(i)->GetCluster() != GetHTAt(j)->GetCluster()) {
              //cout<<"The second has a different or no cluster ..."<<endl;
              //cout<<"First: "<<GetHTAt(i)->ToString()<<endl;;
              ///cout<<"Second: "<<GetHTAt(j)->ToString()<<endl;
              if (GetHTAt(j)->GetCluster() != 0) {
                //cout<<"... and its a different one"<<endl;
                // Get the cluster and move its content to the latest cluster...
                CC = GetHTAt(j)->GetCluster();
                //cout<<"Reordering...!"<<CC->GetNHTs()<<endl;
                for (k = 0; k < CC->GetNHTs(); k++) {
                  //cout<<"New cluster for: "<<CC->GetHTAt(k)->ToString()<<endl;
                  CC->GetHTAt(k)->SetCluster(GetHTAt(i)->GetCluster());
                  GetHTAt(i)->GetCluster()->AddHT(CC->GetHTAt(k));
                }
                // Remove the cluster from the events array:
                ClusterIter = find(m_Clusters.begin(), m_Clusters.end(), CC);
                m_Clusters.erase(ClusterIter);
                delete CC;
              } else {
                //cout<<"... no cluster"<<endl;
                GetHTAt(j)->SetCluster(GetHTAt(i)->GetCluster());
                GetHTAt(i)->GetCluster()->AddHT(GetHTAt(j));
              }
            } // no else, do nothing
            else {
              //cout<<"The second has the same cluster!"<<endl;
            }
          }
        }
      }
    }
  } else {
    Warning("void MSimEvent::CreateClusters()",
            "Can't create clusters, because geometry is missing...");
  }
}


////////////////////////////////////////////////////////////////////////////////


int MSimEvent::GetIEventRetrieval()
{
  // Return if the ideal event can be retrieved, i.e.
  // if the first two interactions come from the initial gamma-ray

  return m_IEventRetrieval;
}


////////////////////////////////////////////////////////////////////////////////


int MSimEvent::GetREventRetrieval()
{
  // Return if the real event can be retrieved, i.e.
  // if there are hits from the first and the second IA

  return m_REventRetrieval;
}


////////////////////////////////////////////////////////////////////////////////


int MSimEvent::GetEventType()
{
  // Return the event type:
  //
  // SET_Unkown  = 0
  // SET_Compton = 1
  // SET_Pair    = 2

  return m_EventType;
}


////////////////////////////////////////////////////////////////////////////////


int MSimEvent::GetEventLocation()
{
  // Return the location of the first IA
  //
  // SEL_Unkown  = 0
  // SEL_D1      = 1
  // SEL_D2      = 2
  // SEL_D3      = 3
  // SEL_D4      = 4

  return m_EventLocation;
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MSimEvent::GetPhysicalEvent(bool Ideal)
{
  MPhysicalEvent* Phys = 0;
  if (GetEventType() == MSimEvent::Compton) {
    MComptonEvent* C = new MComptonEvent();
    if (Ideal == true) {
      C->SetDe(GetICElectronD().Unit());
          
      C->SetC1(GetICFirstIA());
      C->SetEe(GetICEnergyElectron());
          
      C->SetC2(GetICSecondIA());
      C->SetEg(GetICEnergyGamma());
    } else {
      C->SetDe(GetRCElectronD());
          
      C->SetC1(GetRCFirstIAClustered());
      C->SetEe(GetRCEnergyElectron());
          
      C->SetC2(GetRCSecondIAClustered());
      C->SetEg(GetRCEnergyGamma());
    }
    C->SetSequenceLength(2);
    C->SetId(GetID());
    C->SetTime(GetTime());

    C->Validate();

    Phys = C;
  } else if (GetEventType() == MSimEvent::Pair) {
    MPairEvent* P = new MPairEvent();
    if (Ideal == true) {
      P->SetPairCreationIA(GetRPIA());
      P->SetElectronDirection(GetRPElectronD());
      P->SetPositronDirection(GetRPPositronD());
      P->SetEnergyElectron(GetRPEnergyElectron());
      P->SetEnergyPositron(GetRPEnergyPositron());
      P->SetInitialEnergyDeposit(GetRPInitialEnergyDeposit());
    } else {
      P->SetPairCreationIA(GetIPVertex());
      P->SetElectronDirection(GetIPElectronDir());
      P->SetPositronDirection(GetIPPositronDir());
      P->SetEnergyElectron(GetIPElectronEnergy());
      P->SetEnergyPositron(GetIPPositronEnergy());
      P->SetInitialEnergyDeposit(0.0);
    }
    P->SetId(GetID());
    P->SetTime(GetTime());

    P->Validate();

    Phys = (MPhysicalEvent*) P;
  }
  
  return Phys;
}


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetEnergyDepositNotSensitiveMaterial()
{
  // return the energy deposited in not sensitive material

  double Energy = 0;
  for (unsigned int i = 0; i < m_PMs.size(); ++i) {
    Energy += m_PMs[i]->GetEnergy();
  }

  return Energy;
}


////////////////////////////////////////////////////////////////////////////////


MVector MSimEvent::GetICFirstIA()
{
  // Return the location of the ideal first Compton interaction

  return GetIAAt(1)->GetPosition();
}


////////////////////////////////////////////////////////////////////////////////


MVector MSimEvent::GetICSecondIA()
{
  // Return the location of the ideal second Compton interaction

  return GetIAAt(2)->GetPosition();
}


////////////////////////////////////////////////////////////////////////////////


int MSimEvent::GetICFirstIADetector()
{
  // Return the detector of the first interaction

  return GetIAAt(1)->GetDetectorType();
}


////////////////////////////////////////////////////////////////////////////////


int MSimEvent::GetICSecondIADetector()
{
  // Return the detector of the second interaction

  if (GetNIAs() >= 2) {
    return GetIAAt(2)->GetDetectorType();
  }

  return -1;
}

////////////////////////////////////////////////////////////////////////////////


int MSimEvent::GetICThirdIADetector()
{
  if (GetNIAs() > 3) {
    return GetIAAt(3)->GetDetectorType();    
  }

  return -1;
}

////////////////////////////////////////////////////////////////////////////////


MVector MSimEvent::GetICElectronD()
{
  // Return the ideal direction of the recoil electron

  return GetIAAt(1)->GetSecondaryDirection().Unit();
}


////////////////////////////////////////////////////////////////////////////////


MVector MSimEvent::GetICPhotonD()
{

  return (GetIAAt(2)->GetPosition() - GetIAAt(1)->GetPosition()).Unit();
}


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetICEnergyGamma()
{
  // Return the ideal energy of the scattered gamma ray

  return GetIAAt(0)->GetSecondaryEnergy() - GetIAAt(1)->GetSecondaryEnergy();
}


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetICEnergyElectron()
{
  // Return the ideal energy of the recoil electron

  return GetIAAt(1)->GetSecondaryEnergy();
}



////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetICEnergy()
{
  // Return the ideal energy of the incoming gamma-ray

  return GetIAAt(0)->GetSecondaryEnergy();
}


////////////////////////////////////////////////////////////////////////////////


MVector MSimEvent::GetICOrigin()
{
  // Return the origin of the incoming gamma-ray

  return GetIAAt(0)->GetSecondaryDirection();
}


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetICScatterAngle()
{
  // Return the ideal Compton scatter angle

  double Value = 1 - 511 * 
    (1/GetICEnergyGamma() - 1/(GetICEnergyElectron() + GetICEnergyGamma()));

  if (Value <= -1 || Value >= 1) {
    return 0;
  }

  return acos(Value);
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::IsTriggered()
{
  // Return true if there is at least one hit in D1 and D2
  
  unsigned int i;
  int NHits = 0;
  for (i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->GetDetectorType() == D1) {
      NHits++;
    }
  }
  if (NHits == 0) {
    return false;
  }

  NHits = 0;
  for (i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->GetDetectorType() == D2) {
      NHits++;
    }
  }
  if (NHits == 0) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MVector MSimEvent::GetRCFirstIA()
{
  // Return the real first hit of the scattered gamma-ray

  MVector Ideal = GetICFirstIA();
  MVector Candidate = MVector(DBL_MAX, DBL_MAX, DBL_MAX);
  double Distance = DBL_MAX; 

  unsigned int i;
  for (i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->IsOrigin(2) == true) {
      if ((GetHTAt(i)->GetPosition() - Ideal).Mag() < Distance) {
        Candidate = GetHTAt(i)->GetPosition();
        Distance = (Candidate-Ideal).Mag();
      }
    }
  }

  return Candidate;
}


////////////////////////////////////////////////////////////////////////////////


MVector MSimEvent::GetRCFirstIAClustered()
{
  //

  massert(m_Geometry != 0);

  MVector Ideal = GetICFirstIA();
  MVector Candidate = MVector(DBL_MAX, DBL_MAX, DBL_MAX);
  double Distance = DBL_MAX; 

  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->IsOrigin(2) == true) {
      if ((GetHTAt(i)->GetCluster()->GetPosition() - Ideal).Mag() < Distance) {
        Candidate = GetHTAt(i)->GetCluster()->GetPosition();
        Distance = (Candidate-Ideal).Mag();
      }
    }
  }

  return Candidate;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::IsCSecondIAD1()
{
  // Return true, if the second compton interactionis in D1

  unsigned int i;
  for (i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->IsOrigin(3) == true) {
      if (GetHTAt(i)->GetDetectorType() == 1) {
        return true;
      } else {
        return false;
      }
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


MVector MSimEvent::GetRCSecondIA()
{
  // Return the real second compton IA of the scattered gamma-ray

  if (GetIAAt(2)->GetOriginID() != 1 ) {
    return MVector(DBL_MAX, DBL_MAX, DBL_MAX);
  }

  MVector Ideal = GetICSecondIA();
  MVector Candidate = MVector(DBL_MAX, DBL_MAX, DBL_MAX);
  double Distance = DBL_MAX; 

  unsigned int i;
  for (i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->IsOrigin(3) == true) {
      if ((GetHTAt(i)->GetPosition() - Ideal).Mag() < Distance) {
        Candidate = GetHTAt(i)->GetPosition();
        Distance = (Candidate-Ideal).Mag();
      }
    }
  }

  return Candidate;
}


////////////////////////////////////////////////////////////////////////////////


MVector MSimEvent::GetRCSecondIAClustered()
{
  // Return the real second compton IA of the scattered gamma-ray

  massert(m_Geometry != 0);

  if (GetIAAt(2)->GetOriginID() != 1 ) {
    return MVector(DBL_MAX, DBL_MAX, DBL_MAX);
  }

  MVector Ideal = GetICSecondIA();
  MVector Candidate = MVector(DBL_MAX, DBL_MAX, DBL_MAX);
  double Distance = DBL_MAX; 

  unsigned int i;
  for (i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->IsOrigin(3) == true) {
      if ((GetHTAt(i)->GetCluster()->GetPosition() - Ideal).Mag() < Distance) {
        Candidate = GetHTAt(i)->GetCluster()->GetPosition();
        Distance = (Candidate-Ideal).Mag();
      }
    }
  }

  return Candidate;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::HasTrack(int Layers)
{
  // Return true if there is some kind of track,
  // i.e. at least NLayers of the tracker must have been hit 

  // --->acz---> Something geometry dependent:

  unsigned int i;
  int NLayers = 0;
  double zPos = numeric_limits<double>::max();
  int Origin = numeric_limits<int>::max();

  // Check all hits if they are in D1
  for (i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->GetDetectorType() == 1) {
      // If this hit and the last one belong to the same Origin and 
      // are in different layers, we have some kind of track...
      if (Origin == GetHTAt(i)->GetOriginAt(0)) {
        if (fabs(zPos - (GetHTAt(i)->GetPosition())[2]) > 0.01) {
          NLayers++;
          if (NLayers >= Layers) {
            return true;
          }
        }
      } else {
        NLayers = 1;
      }
      // Store the z-Position and the origin
      zPos = (GetHTAt(i)->GetPosition())[2];
      Origin = GetHTAt(i)->GetOriginAt(0);
    } else {
      NLayers = 0;
      zPos = numeric_limits<double>::max();
      Origin = numeric_limits<int>::max();
    }
  }
 
  return false;
}


////////////////////////////////////////////////////////////////////////////////


MVector MSimEvent::GetRCElectronD(int Layers)
{
  // Return the real direction of the electron

  // Direction of the recoil electron:
  // a. at least "Layers" layers must have been hit...
  // b. compute the center of energy in theses layers
  // c. compute the direction from layer 1 to layer 2

  // a & b
  unsigned int i;
  int NLayers = 0;
  double Lastz = 9999;
  MVector CooFirst(0.0, 0.0, 0.0), CooSecond(0.0, 0.0, 0.0);
  double EnergyFirst = 0, EnergySecond = 0;
  for (i = 0; i < GetNHTs(); ++i) {
    // If we have a hit of the electron in the first detector... 
    if (GetHTAt(i)->IsOrigin(2) == true &&
        GetHTAt(i)->GetDetectorType() == 1 ) {
      // If we entered a new layer ...
      //cout<<"New layer? "<<(GetHTAt(i)->GetPosition())[2]<<"!"<<Lastz<<endl;
      //cout<<GetHTAt(i)->ToString()<<endl;
      if (fabs((GetHTAt(i)->GetPosition())[2] - Lastz) > 0.01 ) {
        NLayers++;
        Lastz = (GetHTAt(i)->GetPosition())[2];
        
        if (NLayers == 1) {
          CooFirst = GetHTAt(i)->GetPosition();
          EnergyFirst = GetHTAt(i)->GetEnergy();
        }
        else if (NLayers == 2) {
          CooSecond = GetHTAt(i)->GetPosition();
          EnergySecond = GetHTAt(i)->GetEnergy();
        }
      }
      else {
        Lastz = (GetHTAt(i)->GetPosition())[2];
        
        if (NLayers == 1) {
          CooFirst = (1.0/(EnergyFirst+GetHTAt(i)->GetEnergy()))*
            (EnergyFirst*CooFirst + GetHTAt(i)->GetEnergy()*
             GetHTAt(i)->GetPosition());
          EnergyFirst = GetHTAt(i)->GetEnergy()+EnergyFirst;
        }
        else if (NLayers == 2) {
          CooSecond = (1.0/(EnergySecond+GetHTAt(i)->GetEnergy()))*
            (EnergySecond*CooSecond + GetHTAt(i)->GetEnergy()*
             GetHTAt(i)->GetPosition());
          EnergySecond = GetHTAt(i)->GetEnergy()+EnergySecond;
        }
      }
    }
  }

  //cout<<"NLayers: "<<NLayers<<endl;
  
  //cout<<CooFirst.X()<<"!"<<CooFirst.Y()<<"!"<<CooFirst.Z()<<endl;
  //cout<<CooSecond.X()<<"!"<<CooSecond.Y()<<"!"<<CooSecond.Z()<<endl;

  if (NLayers < Layers) {
    return MVector(0.0, 0.0, 0.0);
  } else {
    return CooSecond - CooFirst;
  }
}


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetRCEnergyGamma()
{
  // The energy of the scattered gamma-ray:

  // a. create a list of all IAs originating in IA 2
  // b. sum up the energy of all hits in this list 

  bool FoundIt;
  double EnergyElectron = 0, EnergyGamma = 0;

  // a. 
  int* IAList = new int[m_IAs.size()];
  int NIAList = 0;
  IAList[NIAList++] = 2;
  
  // 
  for (int j = 0; j < NIAList; ++j) {
    for (unsigned int i = 2; i < m_IAs.size(); ++i) {
      if (GetIAAt(i)->GetOriginID() == IAList[j]) {
        IAList[NIAList++] = i+1;
      }
    }
  }

  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    GetHTAt(i)->SetAddFlag(false);
  }

  // b.
  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    // if the origin is in the list, add it the the electron energy otherwise to the gamma
    FoundIt = false;
    for (int j = 0; j < NIAList; ++j) {
      if (GetHTAt(i)->IsOrigin(IAList[j]) == true && 
          GetHTAt(i)->IsAdded() == false) {
        GetHTAt(i)->SetAddFlag();
        FoundIt = true;
        EnergyElectron += GetHTAt(i)->GetEnergy();
      }  
    }
    if (FoundIt == false && GetHTAt(i)->IsAdded() == false) {
      GetHTAt(i)->SetAddFlag();
      EnergyGamma += GetHTAt(i)->GetEnergy();
    }
  }

  delete [] IAList;

  return EnergyGamma;
}


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetRCEnergyElectron()
{
  // The energy of the recoil electron

  // a. create a list of all IAs originating in IA 2
  // b. sum up the energy of all hits in this list 

  bool FoundIt;
  double EnergyElectron = 0, EnergyGamma = 0;

  // a. 
  int* IAList = new int[m_IAs.size()];
  int NIAList = 0;
  IAList[NIAList++] = 2;
  
  // 
  for (int j = 0; j < NIAList; ++j) {
    for (unsigned int i = 2; i < m_IAs.size(); ++i) {
      if (GetIAAt(i)->GetOriginID() == IAList[j]) {
        IAList[NIAList++] = i+1;
      }
    }
  }

  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    GetHTAt(i)->SetAddFlag(false);
  }

  // b.
  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    // if the origin is in the list, add it the the electron energy otherwise to the gamma
    FoundIt = false;
    for (int j = 0; j < NIAList; ++j) {
      if (GetHTAt(i)->IsOrigin(IAList[j]) == true && 
          GetHTAt(i)->IsAdded() == false) {
        GetHTAt(i)->SetAddFlag();
        FoundIt = true;
        EnergyElectron += GetHTAt(i)->GetEnergy();
      }  
    }
    if (FoundIt == false && GetHTAt(i)->IsAdded() == false) {
      GetHTAt(i)->SetAddFlag();
      EnergyGamma += GetHTAt(i)->GetEnergy();
    }
  }

  delete [] IAList;

  return EnergyElectron;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::IsElectronContainedInD1()
{
  // Check if the electron (or some bremsstrahlung) leaves the tracker... 

  // a. 
  int* IAList = new int[m_IAs.size()];
  int NIAList = 0;
  IAList[NIAList++] = 2;
  
  // 
  for (int j = 0; j < NIAList; ++j) {
    for (unsigned int i = 2; i < m_IAs.size(); ++i) {
      if (GetIAAt(i)->GetOriginID() == IAList[j]) {
        IAList[NIAList++] = i+1;
      }
    }
  }

  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    GetHTAt(i)->SetAddFlag(false);
  }

  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    // if the origin is in the list, add it the the electron energy otherwise to the gamma
    for (int j = 0; j < NIAList; ++j) {
      if (GetHTAt(i)->IsOrigin(IAList[j]) == true && 
          GetHTAt(i)->IsAdded() == false) {
        GetHTAt(i)->SetAddFlag();
        if (GetHTAt(i)->GetDetectorType() != D1) {
          delete [] IAList;
          return false;
        }
      }  
    }
  }

  delete [] IAList;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetRFirstEnergyDepositElectron()
{
  // Return the first energy deposit of the first electron in the first layer 
  // Return DBL_MIN if there is no hit, fulfilling the requirements
  //
  // --> not geometry independent!!!

  unsigned int i, j;
  int Vertex = -1;
  double Energy = DBL_MIN;
  double zLayer = 0.0;

  // Search first electron
  for (i = 0; i < GetNIAs(); ++i) {
    if (GetIAAt(i)->GetSecondaryParticleID() == 3) {
      Vertex = GetIAAt(i)->GetID();
    }
  }
  if (Vertex == -1) return DBL_MIN;

  // Test if the first energy deposit is in a D1:
  for (i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->IsOrigin(Vertex) == true) {
      if (GetHTAt(i)->GetDetectorType() == 1) {
        Energy = GetHTAt(i)->GetEnergy();
        zLayer = GetHTAt(i)->GetPosition().Z();
        for (j = i+1; j < GetNHTs(); ++j) {
          if (GetHTAt(j)->IsOrigin(Vertex) == true && GetHTAt(j)->GetDetectorType() == 1) {
            if (fabs(GetHTAt(j)->GetPosition().Z() - zLayer) < 0.5) {
              Energy += GetHTAt(j)->GetEnergy();
            } else {
              break;
            }
          }
        }
        break;
      }
    }
  }

  return Energy;
}


////////////////////////////////////////////////////////////////////////////////


int MSimEvent::GetLengthFirstTrack()
{
  // Return the length of the first track, 0 otherwise

  unsigned int i, j;
  int Vertex = -1;
  int Length = 0;
  double zLayer = 0.0;

  // Search first electron
  for (i = 0; i < GetNIAs(); ++i) {
    if (GetIAAt(i)->GetSecondaryParticleID() == 3) {
      Vertex = GetIAAt(i)->GetID();
      break;
    }
  }
  if (Vertex == -1) return -1;

  // Test if the first energy deposit is in a D1:
  for (i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->IsOrigin(Vertex) == true) {
      if (GetHTAt(i)->GetDetectorType() == D1) {
        Length++;
        zLayer = GetHTAt(i)->GetPosition().Z();
        for (j = i+1; j < GetNHTs(); ++j) {
          if (GetHTAt(j)->IsOrigin(Vertex) == true && GetHTAt(j)->GetDetectorType() == D1) {
            if (fabs(GetHTAt(j)->GetPosition().Z() - zLayer) > 0.5) {
              Length++;
              zLayer = GetHTAt(j)->GetPosition().Z();
              //cout<<zLayer<<endl;
            }
          }
        }
        break;
      }
    }
  }

  return Length;
}


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetRCEnergy()
{
  //

  // a. create a list of all IAs originating in IA 2
  // b. sum up the energy of all hits in this list 

  bool FoundIt;
  double EnergyElectron = 0, EnergyGamma = 0;

  // a. 
  int* IAList = new int[m_IAs.size()];
  int NIAList = 0;
  IAList[NIAList++] = 2;
  
  // 
  for (int j = 0; j < NIAList; ++j) {
    for (unsigned int i = 2; i < m_IAs.size(); ++i) {
      if (GetIAAt(i)->GetOriginID() == IAList[j]) {
        IAList[NIAList++] = i+1;
      }
    }
  }

  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    GetHTAt(i)->SetAddFlag(false);
  }

  // b.
  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    // if the origin is in the list, add it the the electron energy otherwise to the gamma
    FoundIt = false;
    for (int j = 0; j < NIAList; ++j) {
      if (GetHTAt(i)->IsOrigin(IAList[j]) == true && 
          GetHTAt(i)->IsAdded() == false) {
        GetHTAt(i)->SetAddFlag();
        FoundIt = true;
        EnergyElectron += GetHTAt(i)->GetEnergy();
      }  
    }
    if (FoundIt == false && GetHTAt(i)->IsAdded() == false) {
      GetHTAt(i)->SetAddFlag();
      EnergyGamma += GetHTAt(i)->GetEnergy();
    }
  }

  delete [] IAList;

  return EnergyElectron + EnergyGamma;
}


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetREnergy()
{
  // Return the energy measured - this is NOT the energy of the scattered
  // gamma ray

  double Energy = 0;

  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    Energy += GetHTAt(i)->GetEnergy();
  }

  return Energy;
}


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetREnergyD2()
{
  // Return the energy measured in D2 - this is NOT the energy of the scattered
  // gamma ray

  double Energy = 0;

  for (unsigned i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->GetDetectorType() == 2) {
      Energy += GetHTAt(i)->GetEnergy();
    }
  }

  return Energy;
}


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetREnergyD1()
{
  // Return the energy measured in D1 - 
  // this is NOT the energy of the recoil electron

  double Energy = 0;

  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->GetDetectorType() == 1) {
      Energy += GetHTAt(i)->GetEnergy();
    }
  }

  return Energy;
}


////////////////////////////////////////////////////////////////////////////////


MVector MSimEvent::GetRCentralHitD1()
{
  // Return the center of energy of hits in D1
  // NOT finished

  MVector Pos;
  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->GetDetectorType() == 1) {
      Pos += GetHTAt(i)->GetPosition();
    }
  }

  return Pos;
}


////////////////////////////////////////////////////////////////////////////////


MVector MSimEvent::GetRCentralHitD2()
{
  // Return the center of energy of hits in D2
  // NOT finished

  MVector Pos;

  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->GetDetectorType() == 2) {
      Pos += GetHTAt(i)->GetPosition();
    }
  }

  return Pos;  
}


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetAverageClusterSize(int Detector)
{
  // Return the average cluster size in the detctor

  double NHitsInClusters = 0;
  double NClusters = 0;
  for (unsigned int c = 0; c < GetNClusters(); c++) {
    //cout<<GetClusterAt(c)->ToString()<<endl;
    if (GetClusterAt(c)->GetDetector() == Detector) {
      NClusters++;
      NHitsInClusters += GetClusterAt(c)->GetNHTs();
      //cout<<"Found cluster: "<<GetClusterAt(c)->GetNHTs()<<endl;
      //cout<<NClusters<<": "<<NHitsInClusters<<endl;
    }
  }

  if (NClusters == 0) return 0;

  return NHitsInClusters/NClusters;
}  


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetAverageEnergyDeposit(int Detector)
{
  // Return the average energy deposit per voxel in the specified detector

  double Energy = 0;
  int NHits = 0;
  for (unsigned int h = 0; h < GetNHTs(); h++) {
    if (GetHTAt(h)->GetDetectorType() == Detector) {
      NHits++;
      Energy += GetHTAt(h)->GetEnergy();
    }
  }

  if (NHits == 0) return -1;

  return Energy/NHits;
}


////////////////////////////////////////////////////////////////////////////////


int MSimEvent::GetRNHitsD1()
{
  // Return the number of real hit detector modules in D1

  int N = 0;
  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->GetDetectorType() == 1) {
      N++;
    }
  }

  return N;
}


////////////////////////////////////////////////////////////////////////////////


int MSimEvent::GetRNHitsD2()
{
  // Return the number of real hit detector modulesin D2

  int N = 0;
  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->GetDetectorType() == 2) {
      N++;
    }
  }

  return N;
}


////////////////////////////////////////////////////////////////////////////////


MVector MSimEvent::GetRPIA()
{
  // The real pair interaction position

  massert(m_Geometry != 0);

  MVector Ideal = GetIPVertex();
  MVector Candidate = MVector(numeric_limits<double>::max(), numeric_limits<double>::max(), numeric_limits<double>::max());
  double Distance = numeric_limits<double>::max(); 

  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->IsOrigin(2) == true || GetHTAt(i)->IsOrigin(3)) {
      if ((GetHTAt(i)->GetCluster()->GetPosition() - Ideal).Mag() < Distance) {
        Candidate = GetHTAt(i)->GetCluster()->GetPosition();
        Distance = (Candidate-Ideal).Mag();
      }
    }
  }

  return Candidate;
}


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetRPInitialEnergyDeposit()
{
  // The real pair interaction position

  massert(m_Geometry != 0);

  MVector Ideal = GetIPVertex();
  double Distance = numeric_limits<double>::max(); 
  double Energy = 0;

  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->IsOrigin(2) == true || GetHTAt(i)->IsOrigin(3) == true) {
      if ((GetHTAt(i)->GetCluster()->GetPosition() - Ideal).Mag() < Distance) {
        Energy = GetHTAt(i)->GetCluster()->GetEnergy();
        Distance = (GetHTAt(i)->GetCluster()->GetPosition()-Ideal).Mag();
      }
    }
  }

  return Energy;
}


////////////////////////////////////////////////////////////////////////////////


MVector MSimEvent::GetRPElectronD()
{
  // Return the real direction of the electron

  // Direction of the recoil electron:
  // a. at least "Layers" layers must have been hit...
  // b. compute the center of energy in theses layers
  // c. compute the direction from layer 1 to layer 2

  // a & b
  double Tolerance = 0.0001;
  double Distance = numeric_limits<double>::max();
  MVector Start = GetRPIA();
  MVector Stop(0, 0, 0);

  // Search closest neighbor
  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->IsOrigin(2) == true && 
        GetHTAt(i)->IsOrigin(3) == false &&
        GetHTAt(i)->GetDetectorType() == 1 ) {
      if ((GetHTAt(i)->GetCluster()->GetPosition() - Start).Mag() < Distance &&
          (GetHTAt(i)->GetCluster()->GetPosition() - Start).Mag() > Tolerance) {
        // Check if the cluster is seperated:
        if (GetHTAt(i)->GetCluster()->HasOrigin(2) == true &&
            GetHTAt(i)->GetCluster()->HasOrigin(3) == false) {
          Distance = (GetHTAt(i)->GetCluster()->GetPosition() - Start).Mag();
          Stop = GetHTAt(i)->GetCluster()->GetPosition();
        }
      }
    }
  }

  return (Stop-Start).Unit();
}


////////////////////////////////////////////////////////////////////////////////


MVector MSimEvent::GetRPPositronD()
{
  // Return the real direction of the electron

  // Direction of the recoil electron:
  // a. at least "Layers" layers must have been hit...
  // b. compute the center of energy in theses layers
  // c. compute the direction from layer 1 to layer 2

  // a & b
  double Tolerance = 0.0001;
  double Distance = numeric_limits<double>::max();
  MVector Start = GetRPIA();
  MVector Stop(0, 0, 0);

  // Search closest neighbor
  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->IsOrigin(3) == true && 
        GetHTAt(i)->IsOrigin(2) == false &&
        GetHTAt(i)->GetDetectorType() == 1 ) {
      if ((GetHTAt(i)->GetCluster()->GetPosition() - Start).Mag() < Distance &&
          (GetHTAt(i)->GetCluster()->GetPosition() - Start).Mag() > Tolerance) {
       // Check if the cluster is seperated:
        if (GetHTAt(i)->GetCluster()->HasOrigin(3) == true &&
            GetHTAt(i)->GetCluster()->HasOrigin(2) == false) {
          Distance = (GetHTAt(i)->GetCluster()->GetPosition() - Start).Mag();
          Stop = GetHTAt(i)->GetCluster()->GetPosition();
        }
      }
    }
  }

  return (Stop-Start).Unit();
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::IsIAOrigin(MSimIA* IA, int Origin)
{
  while (IA->GetOriginID() != 0) {
    if (IA->GetID() == Origin) {
      return true;
    } else if (IA->GetOriginID() != 0) {
      IA = GetIAAt(IA->GetOriginID()-1);
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetRPEnergyElectron()
{
  // The energy of the recoil electron

  // a. Create a list of all IAs originating in IA 2
  vector<int> IAIds;
  for (unsigned int i = 0; i < GetNIAs(); ++i) {
    if (IsIAOrigin(GetIAAt(i), 2) == true) {
      IAIds.push_back(GetIAAt(i)->GetID());
    }
  }

  // Add the energy of all these origins:
  double Energy = 0;
  for (unsigned int i = 0; i < IAIds.size(); ++i) {
    for (unsigned int h = 0; h < GetNHTs(); ++h) {
      if (GetHTAt(h)->IsOrigin(IAIds[i]) == true && 
          GetHTAt(h)->GetNOrigins() > 0) {
        Energy += GetHTAt(h)->GetEnergy()/GetHTAt(h)->GetNOrigins();
      }
    }
  }

  return Energy;
}


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetRPEnergyPositron()
{
  // The energy of the recoil electron

  // a. Create a list of all IAs originating in IA 2
  vector<int> IAIds;
  for (unsigned int i = 0; i < GetNIAs(); ++i) {
    if (IsIAOrigin(GetIAAt(i), 3) == true) {
      IAIds.push_back(GetIAAt(i)->GetID());
    }
  }

  // Add the energy of all these origins:
  double Energy = 0;
  for (unsigned int i = 0; i < IAIds.size(); ++i) {
    for (unsigned int h = 0; h < GetNHTs(); ++h) {
      if (GetHTAt(h)->IsOrigin(IAIds[i]) == true && 
          GetHTAt(h)->GetNOrigins() > 0) {
        Energy += GetHTAt(h)->GetEnergy()/GetHTAt(h)->GetNOrigins();
      }
    }
  }

  return Energy;
}


////////////////////////////////////////////////////////////////////////////////


int MSimEvent::GetNTriggeredLayers()
{
  // Return the number of layers with hits in it:

  vector<double> Zs;

  for (unsigned int j = 0; j < GetNHTs(); ++j) {
    if (GetHTAt(j)->GetDetectorType() != 1) continue;
    bool Add = true;
    for (unsigned int z = 0; z < Zs.size(); ++z) {
      if (fabs(GetHTAt(j)->GetPosition().Z() - Zs[z]) < 0.5) {
        Add = false;
      }
    }

    if (Add == true) {
      Zs.push_back(GetHTAt(j)->GetPosition().Z());
      //cout<<"Adding: "<<GetHTAt(j)->GetPosition().Z()<<endl;
    }
  }

  return (int) Zs.size();
}


////////////////////////////////////////////////////////////////////////////////
  

bool SortHTByTime(MSimHT* A, MSimHT* B) {
  return (A->GetTime() < B->GetTime());
}


int MSimEvent::GetLengthOfFirstComptonTrack()
{
  // Calculate the length of the first track
  // -3: No Hit
  // -2: No Compton
  // -1: Not in 2D-Strip 
  //  0: Hit in passive material i
  //  n: Track length

  int Length = 0;

  if (GetNIAs() <= 1) return -3;
  if (GetIAAt(1)->GetProcess() != "COMP") return -2;
  if (GetIAAt(1)->GetDetectorType() == 0) return -1;

  // Gather all hits which originate from the first Compton interaction:
  vector<MSimHT*> HTs;
  for (unsigned int h = 0; h < GetNHTs(); ++h) {
    if (GetHTAt(h)->IsOrigin(2) == true) {
      if (GetHTAt(h)->GetVolumeSequence()->GetDetector()->GetDetectorType() == MDDetector::c_Strip2D) {
        HTs.push_back(GetHTAt(h));
      }
    }
  }

  if (HTs.size() == 0) return -1;

  // Sort the hits in time sequence:
  sort(HTs.begin(), HTs.end(), SortHTByTime);

  // Whenever we have a new detector, we have entered a new layer:
  MDVolumeSequence* Last = HTs[0]->GetVolumeSequence();
  Length++;
  for (unsigned int h = 1; h < HTs.size(); ++h) {
    if (HTs[h]->GetVolumeSequence()->HasSameDetector(Last) == false) {
      Last = HTs[h]->GetVolumeSequence();
      Length++;
    }
  }

  return Length;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::GetVeto()
{
  // Return the Veto-flag

  // In Comptel-case we have a real veto flag:
  if (m_Veto == true) {
    return m_Veto;
  }


  // In MEGA-case, we have hits in Detector 4 (= ACS)
  double VetoEnergy = 0;
  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    if (GetHTAt(i)->GetDetectorType() == 4) {
      VetoEnergy += GetHTAt(i)->GetEnergy();
    }
  }

  if (VetoEnergy > 100) {
    return true;
  } else {
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetTOF()
{
  // Return the time of flight between D1 and D2 (in ns)

  return m_TOF;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MSimEvent::GetNIAs()
{
  // return the number of stored IA

  return m_IAs.size();
}


////////////////////////////////////////////////////////////////////////////////


MSimIA* MSimEvent::GetIAAt(unsigned int i)
{
  // return IA number i

  if (i < m_IAs.size()) {
    return m_IAs[i];
  } else {
    merr<<"Event "<<m_NEvent<<": IA index ("<<i<<") out of bounds! Max: "<<GetNIAs()<<endl;
    massert(false);
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


MSimIA* MSimEvent::GetIAById(int Id)
{
  // return IA with Id Id

  Id -= 1;
  if (Id >= 0 && Id < int(m_IAs.size())) {
    return m_IAs[Id];
  } else {
    merr<<"Event "<<m_NEvent<<": IA index ("<<Id<<") out of bounds! Max: "<<GetNIAs()<<endl;
    massert(false);
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MSimEvent::GetNDRs()
{
  // return the number of stored DR

  return m_DRs.size();
}


////////////////////////////////////////////////////////////////////////////////


MSimDR* MSimEvent::GetDRAt(unsigned int i)
{
  // return DR number i

  if (i < m_DRs.size()) {
    return m_DRs[i];
  } else {
    merr<<"Event "<<m_NEvent<<": DR index ("<<i<<") out of bounds! Max: "<<GetNDRs()<<endl;
    massert(false);
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MSimEvent::GetNPMs()
{
  // Return the number of PM IDs in this event

  return m_PMs.size();
}


////////////////////////////////////////////////////////////////////////////////


MSimPM* MSimEvent::GetPMAt(unsigned int i)
{
  // return PM number i

  if (i < m_PMs.size()) {
    return m_PMs[i];
  } else {
    merr<<"Event "<<m_NEvent<<": PM index ("<<i<<") out of bounds! Max: "<<GetNPMs()<<endl;
    massert(false);
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MSimEvent::GetNBDs()
{
  // Return the number of BD flags in this event

  return m_BDs.size();
}


////////////////////////////////////////////////////////////////////////////////


MString MSimEvent::GetBDAt(unsigned int i)
{
  // return BD number i

  if (i < m_BDs.size()) {
    return m_BDs[i];
  } else {
    merr<<"Event "<<m_NEvent<<": BD index ("<<i<<") out of bounds! Max: "<<GetNBDs()<<endl;
    massert(false);
    return "";
  }
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MSimEvent::GetNCCs()
{
  // Return the number of comments

  return m_CCs.size();
}


////////////////////////////////////////////////////////////////////////////////


MString MSimEvent::GetCCAt(unsigned int i)
{
  // Return Comment number i

  if (i < m_CCs.size()) {
    return m_CCs[i];
  } else {
    merr<<"Event "<<m_NEvent<<": CC index ("<<i<<") out of bounds! Max: "<<GetNCCs()<<endl;
    massert(false);
    return "";
  }
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MSimEvent::GetNGRs()
{
  // return the number of stored GR

  return m_GRs.size();
}


////////////////////////////////////////////////////////////////////////////////


MSimGR* MSimEvent::GetGRAt(unsigned int i)
{
  // return GR number i

  if (i < m_GRs.size()) {
    return m_GRs[i];
  } else {
    merr<<"Event "<<m_NEvent<<": GR index ("<<i<<") out of bounds! Max: "<<GetNGRs()<<endl;
    massert(false);
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MSimEvent::GetNHTs()
{
  // return the number of stored HT

  return m_HTs.size();
}


////////////////////////////////////////////////////////////////////////////////


MSimHT* MSimEvent::GetHTAt(unsigned int i)
{
  // return HT number i

  if (i < m_HTs.size()) {
    return m_HTs[i];
  } else {
    merr<<"Event "<<m_NEvent<<": HT index ("<<i<<") out of bounds! Max: "<<GetNHTs()<<endl;
    massert(false);
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MSimEvent::RemoveHT(MSimHT* HT)
{
  // remove a hit from this event:

  vector<MSimHT*>::iterator I = find(m_HTs.begin(), m_HTs.end(), HT);
  if (I != m_HTs.end()) {
    m_HTs.erase(I);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MSimEvent::RemoveAllHTsBut(MSimHT* HT)
{
  //! Remove all but the given hit from the event  

  vector<MSimHT*>::iterator I = m_HTs.begin();
  while (I != m_HTs.end()) {
    if ((*I) != HT) {
      I = m_HTs.erase(I);
    } else {
      ++I;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


vector<int> MSimEvent::GetAllHTOrigins()
{
  // 

  vector<int> O;

  for (unsigned int h = 0; h < GetNHTs(); ++h) {
    for (unsigned int o = 0; o < GetHTAt(h)->GetNOrigins(); ++o) {
      if (find(O.begin(), O.end(), GetHTAt(h)->GetOriginAt(o)) != O.end()) {
        O.push_back(GetHTAt(h)->GetOriginAt(o));
      }
    }
  }

  massert(false);

  return O;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MSimEvent::GetNClusters()
{
  // return the number of stored HT

  return m_Clusters.size();
}


////////////////////////////////////////////////////////////////////////////////


MSimCluster* MSimEvent::GetClusterAt(unsigned int i)
{
  // return cluster number i

  if (i < GetNClusters()) {
    return m_Clusters[i];
  } else {
    merr<<"Event "<<m_NEvent<<": Index ("<<i<<") out of bounds! Max: "<<GetNClusters()<<endl;
    massert(false);
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MSimEvent::AddCluster(MSimCluster* Cluster)
{
  // Add a cluster 

  m_Clusters.push_back(Cluster);
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::IsIAResolved(int IAID)
{
  // Returns true, if in all hits originating from the IA "IAID" only descendents
  // interactions are present, e.g. deposits from the recoil electron and eventually
  // Bremsstrahlung, but not another Compton interaction
  // Attention: IA-ID "1" does of course not count!

  massert(IAID >= 1);

  // GetDescendent IDs
  vector<int> IAIDs = GetDescendents(IAID);

  // Get a list of all HTs which contain IAID
  vector<MSimHT*> HTs = GetHTListIncludingDescendents(IAID);

  if (HTs.size() == 0) {
    return false;
  }

  // If one of those HTs contains other IDs than in IAIDs, then the interaction is not resolved! 
  for (unsigned int h = 0; h < HTs.size(); ++h) {
    for (unsigned int o = 0; o < HTs[h]->GetNOrigins(); ++o) {
      if (HTs[h]->GetOriginAt(o) == 1) continue;
      bool Found = false;
      for (unsigned int d = 0; d < IAIDs.size(); ++d) {
        if (HTs[h]->GetOriginAt(o) == IAIDs[d]) {
          Found = true;
          break;
        }
      }
      if (Found == false) {
        return false;
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


vector<int> MSimEvent::GetDescendents(int IAID) 
{
  // Get all descendents, i.e. all interactions originating from this one
  // Returns a list of IA IDs including the original one!

  vector<int> IAIDs;
  IAIDs.push_back(IAID);

  for (unsigned int g = 1; g < GetNIAs(); ++g) {
    for (unsigned int a = 0; a < IAIDs.size(); ++a) {
      if (GetIAAt(g)->GetOriginID() == IAIDs[a]) {
        IAIDs.push_back(GetIAAt(g)->GetID());
        break;
      }
    }
  }

  //cout<<"Descendents: ";
  //for (unsigned int d = 0; d < IAIDs.size(); ++d) {
  //  cout<<IAIDs[d]<<" ";
  //}
  //cout<<endl;

  return IAIDs;
}


////////////////////////////////////////////////////////////////////////////////


vector<MSimHT*> MSimEvent::GetHTListIncludingDescendents(int IAID)
{
  // Returns a list of all HTs originating from IA and its descendents

  massert(IAID >= 1);

  // Get all descendents:
  vector<int> IAIDs = GetDescendents(IAID);

  vector<MSimHT*> HTList;
  for (unsigned int h = 0; h < GetNHTs(); ++h) {
    for (unsigned int i = 0; i < IAIDs.size(); ++i) {
      if (GetHTAt(h)->IsOrigin(IAIDs[i]) == true) {
        HTList.push_back(GetHTAt(h));
        break;
      }
    }
  }

  return HTList;
}


////////////////////////////////////////////////////////////////////////////////


int MSimEvent::IsIACompletelyAbsorbed(int IAID, double AbsoluteTolerance, double RelativeTolerance)
{
  // Returns 0 if this IA (including its decendents) is completely absorbed.
  // Returns 1 if this IA is partly absorbed
  // Returns 2 if this IA has not deposited energy at all
  //
  // If there are multiple absorptions in one voxel of the detector, then 
  // the algorithm might fail!
  // Make sure the hits are NOT noised!
  // This algorithm is very sensitive to the numbering scheme within the sim file...

  massert(IAID >= 1);
  massert(m_Version >= 21);

  mimp<<"Stupid tolerance handling and problems identifying IA in same voxels..."<<show;

  // Get all descendents:
  vector<int> IAIDs = GetDescendents(IAID);

  double EnergyReal = 0;
  for (unsigned int h = 0; h < GetNHTs(); ++h) {
    for (unsigned int i = 0; i < IAIDs.size(); ++i) {
      if (GetHTAt(h)->IsOrigin(IAIDs[i]) == true) {
        EnergyReal += GetHTAt(h)->GetEnergy();
        break;
      }
    }
  }

  double EnergyIdeal = 0;

  // If we do have a Compton event, we do have to take special precautions:
  if (GetIAAt(IAID-1)->GetProcess() == "COMP") {
    //cout<<"Compton: "<<endl;
    MSimIA* Top = 0;
    MSimIA* Bottom = 0;
    if (GetIAAt(IAID-2)->GetOriginID() == GetIAAt(IAID-1)->GetOriginID()) {
      Top = GetIAAt(IAID-2);
      Bottom = GetIAAt(IAID-1);
    } else {
      Top = GetIAAt(GetIAAt(IAID-1)->GetOriginID()-1);
      Bottom = GetIAAt(IAID-1);
    }
    
    //cout<<"Top:"<<Top->GetSecondaryEnergy()<<"  Bottom:"<<Bottom->GetSecondaryEnergy()<<endl;

    if (GetIAAt(IAID-2)->GetProcess() == "COMP") {
      EnergyIdeal = Top->GetMotherEnergy()-Bottom->GetMotherEnergy();
      //cout<<"v1: "<<EnergyIdeal<<endl;
    } else {
      EnergyIdeal = Top->GetSecondaryEnergy()-Bottom->GetMotherEnergy();
      //cout<<"v2: "<<EnergyIdeal<<endl;
    }
  } else {
    EnergyIdeal = GetIAAt(IAID-1)->GetSecondaryEnergy();
  }

  //mout<<"Ideal: "<<EnergyIdeal<<"   Real: "<<EnergyReal<<endl;

  double Tolerance = AbsoluteTolerance + RelativeTolerance*EnergyIdeal;
  if (EnergyReal < Tolerance) return 2;
  if (fabs(EnergyIdeal - EnergyReal) > Tolerance) return 1;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::Discretize(int Detector)
{
  // Discretize this event and combine hits in same voxels:

  // Grids for each detector type
  map<int, vector<MDGridPointCollection> > DetectorGrids;

  // Add all hits to the grids:
  for (vector<MSimHT*>::iterator Iter = m_HTs.begin(); 
       Iter != m_HTs.end(); /* erase! */ ) {
    MSimHT* Hit = (*Iter);

    //cout<<"Hit: "<<Hit->GetPosition()<<endl;
    if (Hit->GetDetectorType() != Detector) {
      Iter++; 
      continue;
    }

    // Let's do some sanity checks:
    if (Hit->GetVolumeSequence()->GetDetector() == 0) {
      mout<<" *** Really bad error *** We do not have a detector for this hit!"<<endl;
      mout<<"     This should have been checked during reading of event..."<<endl;
      return false;
    }

    if (Hit->GetDetectorType() >= MDDetector::c_MinDetector && 
        Hit->GetDetectorType() <= MDDetector::c_MaxDetector) {
      vector<MDGridPointCollection>& Grids = DetectorGrids[Hit->GetDetectorType()];

      // Check if we have a suitable grid:
      const unsigned int NoGrid = numeric_limits<unsigned int>::max();
      unsigned int CorrectGrid = NoGrid;
      for (unsigned g = 0; g < Grids.size(); ++g) {
        if (Grids[g].HasSameDetector(Hit->GetVolumeSequence()) == true) {
          CorrectGrid = g;
          break;
        }
      }
      if (CorrectGrid == NoGrid) {
        // Create a new Grid
        MDGridPointCollection Grid(*(Hit->GetVolumeSequence())); 
        Grids.push_back(Grid);
        CorrectGrid = Grids.size()-1;
      } 
      // Add hit to Grid - Grid does correct discretization
      //cout<<"Grid: "<<Grids.size()<<"!"<<CorrectGrid<<endl;
      Grids[CorrectGrid].Add(Hit->GetVolumeSequence()->GetPositionInDetector(), 
                             Hit->GetEnergy(), Hit->GetTime(), Hit->GetOrigins());
    } else {
      merr<<"Event "<<m_NEvent<<": We have a hit without detector type: "<<Hit->GetDetectorType()<<endl;
    }

    // We delete the old hits:
    Iter = m_HTs.erase(Iter++);
    delete Hit;
  }

  // Finalize Gridding - discretize z position of drifting voxels:
  map<int, vector<MDGridPointCollection> >::iterator Iter;
  for (Iter = DetectorGrids.begin();
       Iter != DetectorGrids.end();
       ++Iter) {
    vector<MDGridPointCollection>& Grids = (*Iter).second;
    for (unsigned int g = 0; g < Grids.size(); ++g) {
      Grids[g].FinalizeGridding();
    }
  }

  // Read the data from the Grids into MSimHTs:
  // m_HTs.clear();
  for (Iter = DetectorGrids.begin();
       Iter != DetectorGrids.end();
       ++Iter) {
    vector<MDGridPointCollection>& Grids = (*Iter).second;
    for (unsigned int g = 0; g < Grids.size(); ++g) {
      MDGridPointCollection& Grid = Grids[g];
      double DetectorEnergy = 0;
      for (unsigned int p = 0; p < Grid.GetNGridPoints(); ++p) {
        const MDGridPoint& Point = Grid.GetGridPointAt(p);
        //cout<<"New hit: "<<Point<<endl;
        if (Point.GetType() == MDGridPoint::c_Voxel || 
            Point.GetType() == MDGridPoint::c_VoxelDrift ||
            Point.GetType() == MDGridPoint::c_XYAnger ||
            Point.GetType() == MDGridPoint::c_XYZAnger) {
          MSimHT* Hit = new MSimHT((*Iter).first,
                                   Grid.GetWorldPositionGridPointAt(p),
                                   Point.GetEnergy(),
                                   Point.GetTime(),
                                   Point.GetOrigins(),
                                   m_Geometry);
          //cout<<"Hit (2): "<<Hit->GetPosition()<<endl;

          if (Hit->GetDetectorType() == MDDetector::c_NoDetectorType) {
            merr<<"Event "<<m_NEvent<<": Something went badly wrong: The new hit has no detector! Ignoring it right now"<<endl;
            delete Hit;
          } else {
            m_HTs.push_back(Hit); 
            DetectorEnergy += Point.GetEnergy();
          }
        } else if (Point.GetType() == MDGridPoint::c_Guardring) {
          MSimGR* GR = new MSimGR((*Iter).first,
                                  Grid.GetWorldPositionGridPointAt(p),
                                  Point.GetEnergy(),
                                  m_Geometry);
          if (GR->GetDetectorType() == MDDetector::c_NoDetectorType) {
            merr<<"Event "<<m_NEvent<<": Something went badly wrong: The new guard hit has no detector! Ignoring it right now"<<endl;
            delete GR;
          } else {
            m_GRs.push_back(GR); 
          }
        } else {
          merr<<"Event "<<m_NEvent<<": GridPoint type "<<Point.GetType()<<" not implemented!"<<endl;
        }
      }
      // Store the total energy for drift chambers: 
      if ((*Iter).first == MDDetector::c_DriftChamber) {
        // ... if it has a light sensitive detector
        if (dynamic_cast<MDDriftChamber*>(Grid.GetDetector())->GetLightDetectorPosition() != 0) {
          m_TotalDetectorEnergy[Grid.GetWorldPosition()] = Grid.GetEnergy(); 
        }
      }
    }
  }

  if (Detector == MDDetector::c_Strip3DDirectional) {
    for (unsigned int i = 0; i < m_DRs.size(); ++i) delete m_DRs[i];
    m_DRs.clear();

    for (unsigned int i = 0; i < GetNIAs(); ++i) {
      if (GetIAAt(i)->GetDetectorType() == MDDetector::c_Strip3DDirectional) {
        // Disretize position:
        MDVolumeSequence VS = m_Geometry->GetVolumeSequence(GetIAAt(i)->GetPosition());
        MDGridPointCollection Grid(VS);
        vector<int> Origins;
        Grid.Add(VS.GetPositionInDetector(), 0, 0, Origins);
        massert(Grid.GetNGridPoints() == 1);
        MVector DiscretizedPosition = Grid.GetWorldPositionGridPointAt(0);
        
        MSimDR* DR = new MSimDR(DiscretizedPosition, GetIAAt(i)->GetSecondaryDirection(), GetIAAt(i)->GetSecondaryEnergy());
        m_DRs.push_back(DR);
      }
    }
  }

  return true;
}



////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetGuardringEnergy()
{
  //! Return the total energy deposit in all guard rings:

  double Energy = 0.0;

  for (unsigned int g = 0; g < GetNGRs(); ++g) {
    Energy += GetGRAt(g)->GetEnergy();
  }

  return Energy;
}


////////////////////////////////////////////////////////////////////////////////


vector<MSimEvent*> MSimEvent::CreateSingleHitEvents()
{
  //! Create a set of sim events which contain only one hit per event

  vector<MSimEvent*> Events;

  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    Events.push_back(new MSimEvent(*this));
  }

  for (unsigned int e = 0; e < Events.size(); ++e) {
    Events[e]->RemoveAllHTsBut(Events[e]->GetHTAt(e));
  }

  return Events;
}


////////////////////////////////////////////////////////////////////////////////


double MSimEvent::GetTotalEnergyDeposit()
{
  //! Return the total energy deposit (after noising)

  double Deposit = 0;
  for (unsigned int h = 0; h < GetNHTs(); ++h) {
    Deposit += GetHTAt(h)->GetEnergy();
  }
  
  return Deposit;
}

  
////////////////////////////////////////////////////////////////////////////////

double MSimEvent::GetTotalEnergyDepositBeforeNoising()
{
  //! Return the total energy deposit (before noising)
 
  double Deposit = 0;
  for (unsigned int h = 0; h < GetNHTs(); ++h) {
    Deposit += GetHTAt(h)->GetOriginalEnergy();
  }
  
  return Deposit; 
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEvent::IsCompletelyAbsorbed(double AbsoluteTolerance)
{
  //! Return true if the event is completely absorbed 

 double Start = 0;
 for (unsigned int i = 0; i < GetNIAs(); ++i) {
   if (GetIAAt(i)->GetProcess() == "INIT") {
     Start += GetIAAt(i)->GetSecondaryEnergy();
   }
 }
 
 if (fabs(Start - GetTotalEnergyDepositBeforeNoising()) < AbsoluteTolerance) {
   return true; 
 }
 
 return false;
}
  
  
////////////////////////////////////////////////////////////////////////////////


void MSimEvent::Reset()
{  
  // Delete all info of this event:

  for (unsigned int i = 0; i < m_IAs.size(); ++i) delete m_IAs[i];
  for (unsigned int i = 0; i < m_HTs.size(); ++i) delete m_HTs[i];
  for (unsigned int i = 0; i < m_Clusters.size(); ++i) delete m_Clusters[i];
  for (unsigned int i = 0; i < m_DRs.size(); ++i) delete m_DRs[i];
  for (unsigned int i = 0; i < m_GRs.size(); ++i) delete m_GRs[i];
  for (unsigned int i = 0; i < m_PMs.size(); ++i) delete m_PMs[i];
  m_IAs.clear();
  m_HTs.clear();
  m_Clusters.clear();  
  m_DRs.clear();  
  m_GRs.clear();  
  m_PMs.clear();  
  m_BDs.clear();
  m_CCs.clear();

  m_TotalDetectorEnergy.clear();

  m_EventType = Unknown;
  m_EventLocation = NotSensitive;
  m_TOF = 0;
  m_Veto = false;
  m_SimulationData = "";
  m_Time.Set(0, 0);

  m_NIgnoredHTs = 0;
}


////////////////////////////////////////////////////////////////////////////////


MString MSimEvent::GetSimulationData()
{
  // Return the input strings of this event
  
  if (m_StoreSimulationData == false) {
    mout<<"Info: Simulation data has not been stored."<<endl;
  }

  return m_SimulationData;
}


////////////////////////////////////////////////////////////////////////////////


MString MSimEvent::ToString()
{
  // Dump a simple stream of this event...

  ostringstream out;

  out<<"Event: "<<m_NEvent<<endl;
  for (unsigned int i = 0; i < GetNIAs(); ++i) {
    out<<GetIAAt(i)->ToString()<<endl;
  }
  for (unsigned int i = 0; i < GetNHTs(); ++i) {
    out<<GetHTAt(i)->ToString()<<endl;
  }

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MSimEvent::ToSimString(const int WhatToStore, const int Precision, const int Version)
{
  // Convert this SimEvent to the original *.sim file format...

  ostringstream out;
  out<<"SE"<<endl;
  out<<"ID "<<m_NEvent<<" "<<m_NStartedEvent<<endl;
  if (m_Veto == true) {
    out<<"VT"<<endl;
  } else {
    out<<"TI "<<m_Time.GetLongIntsString()<<endl;
    
    if (m_BDs.size() > 0) {
      out<<"BD ";
      for (unsigned int b = 0; b < m_BDs.size(); ++b) {
        out<<m_BDs[b]<<" "; 
      }
      out<<endl;
    }
    
    MRotationInterface::Stream(out);
    
    if (WhatToStore == c_StoreSimulationInfoAll) {
      // The ED keyword (Total energy deposit in active material)
      double ED = 0.0;
      for (unsigned int i = 0; i < GetNHTs(); ++i) {
        ED += GetHTAt(i)->GetEnergy();
      }
      for (unsigned int i = 0; i < GetNGRs(); ++i) {
        ED += GetGRAt(i)->GetEnergy();
      }
      out<<"ED "<<ED<<endl;
      
      // The EC keyword (Escapes)
      double EC = 0.0;
      for (unsigned int i = 0; i < GetNIAs(); ++i) {
        if (GetIAAt(i)->GetProcess() == "ESCP") {
          EC += GetIAAt(i)->GetMotherEnergy();
        }
      }
      out<<"EC "<<EC<<endl;
      
      // Deposits in not sensitive material - summary
      double NS = 0.0;
      for (unsigned int i = 0; i < GetNPMs(); ++i) {
        NS += GetPMAt(i)->GetEnergy();
      }
      out<<"NS "<<NS<<endl;
      
      // Deposits in not sensitive material - detailed
      for (unsigned int i = 0; i < GetNPMs(); ++i) {
        out<<GetPMAt(i)->ToSimString(WhatToStore, Precision, Version)<<endl;;
      }
      
      // Comments
      for (unsigned int c = 0; c < m_CCs.size(); ++c) {
        out<<"CC "<<m_CCs[c]<<endl; 
      }
    }


    if (WhatToStore == c_StoreSimulationInfoAll ||
        WhatToStore == c_StoreSimulationInfoInitOnly) {
      for (unsigned int i = 0; i < GetNIAs(); ++i) {
        if ((GetIAAt(i)->GetProcess() != "INIT" && GetIAAt(i)->GetProcess() != "ENTR" && GetIAAt(i)->GetProcess() != "EXIT") && 
            WhatToStore == c_StoreSimulationInfoInitOnly) {
          continue;
        }
        out<<GetIAAt(i)->ToSimString(WhatToStore, Precision, Version)<<endl;
      }
    }
    for (unsigned int i = 0; i < GetNHTs(); ++i) {
      out<<GetHTAt(i)->ToSimString(WhatToStore, Precision, Version)<<endl;
    }
    for (unsigned int i = 0; i < GetNGRs(); ++i) {
      out<<GetGRAt(i)->ToSimString(WhatToStore, Precision, Version)<<endl;
    }
    map<MVector, double>::iterator Iter;
    for (Iter = m_TotalDetectorEnergy.begin();
         Iter != m_TotalDetectorEnergy.end(); ++Iter) {
      mimp<<"XE only hacked!!! --- But anyway not used..."<<endl;
      out<<"XE "<<(*Iter).first.X()<<";"<<(*Iter).first.Y()<<";"<<(*Iter).first.Z()<<";"<<(*Iter).second<<endl;
    }

    for (unsigned int i = 0; i < GetNDRs(); ++i) {
      out<<GetDRAt(i)->ToSimString(WhatToStore, Precision, Version)<<endl;
    }
  }

  return out;
}


// MSimEvent.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
