/*
 * MResponseMultipleCompton.cxx
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
// MResponseMultipleCompton
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseMultipleCompton.h"

// Standard libs:
#include <limits>
#include <algorithm>
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MSettingsRevan.h"
#include "MRESEIterator.h"
#include "MERCSRBayesian.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MResponseMultipleCompton)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MResponseMultipleCompton::MResponseMultipleCompton()
{  
  m_ResponseNameSuffix = "mc";
  
  m_DoAbsorptions = true;
  m_MaxAbsorptions = 5;
  m_MaxNInteractions = 7;

  m_MaxEnergyDifference = 5; // keV
  m_MaxEnergyDifferencePercent = 0.02;

  m_MaxTrackEnergyDifference = 30; // keV
  m_MaxTrackEnergyDifferencePercent = 0.1;

  m_EnergyMinimum = 100; // keV
  m_EnergyMaximum = 10000; // keV
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MResponseMultipleCompton::~MResponseMultipleCompton()
{
  // Nothing to delete
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsComptonTrack(MRESE& Start, MRESE& Center, 
                                       int PreviousPosition, double Etot, double Eres)
{
  // A good start point of the track consists of the following:
  // (1) Start is a track
  // (2) Start is only one Compton interaction
  // (3) One of Centers Compton interactions directly follows the IA of start
  // (4) The first hit of start is the first hit of the track
  //     The second hit of start is the second hit of the track

  mdebug<<"IsComptonTrack: Looking at: "<<Start.GetID()<<"("<<Etot<<")"<<endl;

  // (1) 
  if (Start.GetType() != MRESE::c_Track) {
    mdebug<<"IsComptonTrack: No track!"<<endl;
    return false;    
  }

  // (2)
  if (IsSingleCompton(Start) == false) {
    mdebug<<"IsComptonTrack: Start has multiple Comptons!"<<endl;
    return false;        
  }

  // (3)
  if (IsComptonSequence(Start, Center, PreviousPosition, Etot-Start.GetEnergy(), Eres) == false) {
    // Attention: Eres is a little to large...
    mdebug<<"IsComptonTrack: No Compton sequence!"<<endl;
    return false;        
  }

  // (4)
  MRESEIterator Iter;
  MRETrack* Track = (MRETrack*) &Start;
  Iter.Start(Track->GetStartPoint());
  Iter.GetNextRESE();
  if (IsTrackStart(*Iter.GetCurrent(), *Iter.GetNext(), Track->GetEnergy()) == false) {
    mdebug<<"IsComptonTrack: Track is wrong!"<<endl;
    return false;    
  }
    
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsTrackStart(MRESE& Start, MRESE& Central, double Energy)
{
  // According to the current simulation (GMega), the RESEs are in sequence if
  // (1) the IDs of their hits are in increasing order without holes
  // (2) all originate from the same IA
  // (3) The start condition is fullfilled, when the start hit has the lowest 
  //     time-tag of all hits with this origin

  const int IdOffset = 2;

  vector<int> StartIds = GetReseIds(&Start);
  vector<int> CentralIds = GetReseIds(&Central);
  if (StartIds.size() == 0) return false;
  if (CentralIds.size() == 0) return false;


  // Test (1)
  if (AreIdsInSequence(StartIds) == false) {
    mdebug<<"   Is track start: Start IDs not in sequence"<<endl;
    return false;
  }
  if (AreIdsInSequence(CentralIds) == false) {
    mdebug<<"Is track Start: Central IDs not in sequence"<<endl;
    return false;
  }

  // Test for holes:
  // The hit after the last one from start is the first from central:
  double TimeLastFirst = m_SiEvent->GetHTAt(StartIds.back()-IdOffset)->GetTime();
  double TimeFirstLast = m_SiEvent->GetHTAt(CentralIds.front()-IdOffset)->GetTime();
  for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
    if (m_SiEvent->GetHTAt(h)->GetTime() > TimeLastFirst && 
        m_SiEvent->GetHTAt(h)->GetTime() < TimeFirstLast) {
      mdebug<<"   Is track start: Hit between first and central according to time: "
            <<h<<endl;
      mdebug<<"LF: "<<TimeLastFirst<<" FL: "<<TimeFirstLast<<endl;
      return false;      
    }
  } 


//   if (abs(CentralIds[0] - StartIds[StartIds.size()-1]) != 1 && 
//       abs(CentralIds[CentralIds.size()-1] - StartIds[0]) != 1) {
//     mdebug<<"   Is track start: Distance not equal 1: "
//           <<abs(CentralIds[0] - StartIds[StartIds.size()-1])<<", "
//           <<abs(CentralIds[CentralIds.size()-1] - StartIds[0])<<endl;
//     return false;
//   }
  
  // Test (2)
  int CommonOrigin = -1;
  for (unsigned int i = 0; i < StartIds.size(); ++i) {
    for (unsigned int j = 0; j < CentralIds.size(); ++j) {
      for (unsigned int oi = 0; oi < m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetNOrigins(); ++oi) {
        for (unsigned int oj = 0; oj < m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetNOrigins(); ++oj) {
          if (m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetOriginAt(oi) == 
              m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetOriginAt(oj)) {
            CommonOrigin = m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetOriginAt(oj);
            break;
          }
        }
      }
    }
  }

  if (CommonOrigin == -1) {
    mdebug<<"   Is track start: No common origin!"<<endl;
    return false;
  }

  // Test (3)
  double Timing = numeric_limits<double>::max();
  for (unsigned int i = 0; i < StartIds.size(); ++i) {
    if (m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetTime() < Timing) {
      Timing = m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetTime();
    }
  }
  unsigned int i_max = m_SiEvent->GetNHTs();
  for (unsigned int i = 0; i < i_max; ++i) {
    if (m_SiEvent->GetHTAt(i)->IsOrigin(CommonOrigin)) {
      if (m_SiEvent->GetHTAt(i)->GetTime() < Timing) {
        mdebug<<"   Is track start: Not first hit (timing)"<<endl;
        return false;
      }
    }
  }

  if (Energy > 0) {
    if (IsTrackCompletelyAbsorbed(StartIds, Energy) == false) {
      mdebug<<"   Is track start: Not completely absorbed"<<endl;
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsTrackStop(MRESE& Central, 
                                           MRESE& Stop, double Energy)
{
  // According to the current simulation (GMega), the RESEs are in sequence if
  // (1) the IDs of their hits are in increasing order without holes
  // (2) all originate from the same IA (espec. the first and last hit in the 
  //     sequence)
  // (3) The start condition is fullfilled, when the start hit has the highest 
  //     time-tag of all hits with this origin

  const int IdOffset = 2;

  vector<int> CentralIds = GetReseIds(&Central);
  vector<int> StopIds = GetReseIds(&Stop);
  
  // Test (1)
  if (AreIdsInSequence(CentralIds) == false) {
    mdebug<<"   IsStop: Central IDs not in sequence"<<endl;
    return false;
  }
  if (AreIdsInSequence(StopIds) == false) {
    mdebug<<"   IsStop: Stop IDs not in sequence"<<endl;
    return false;
  }
  // Test for holes:
  // The hit after the last one from start is the first from central:
  double TimeLastFirst = m_SiEvent->GetHTAt(CentralIds.back()-IdOffset)->GetTime();
  double TimeFirstLast = m_SiEvent->GetHTAt(StopIds.front()-IdOffset)->GetTime();
  for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
    if (m_SiEvent->GetHTAt(h)->GetTime() > TimeLastFirst && 
        m_SiEvent->GetHTAt(h)->GetTime() < TimeFirstLast) {
      mdebug<<"   IsStop: Hit between first and central according to time!"<<endl;
      return false;      
    }
  } 
//   if (abs(StopIds[0] - CentralIds[CentralIds.size()-1]) != 1 && 
//       abs(StopIds[StopIds.size()-1] - CentralIds[0]) != 1) {
//     mdebug<<"   IsStop: Distance not equal 1"<<endl;
//     return false;
//   }


  // Test (2)
  int CommonOrigin = -1;
  for (unsigned int i = 0; i < CentralIds.size(); ++i) {
    for (unsigned int j = 0; j < StopIds.size(); ++j) {
      for (unsigned int oi = 0; oi < m_SiEvent->GetHTAt(CentralIds[i]-IdOffset)->GetNOrigins(); ++oi) {
        for (unsigned int oj = 0; oj < m_SiEvent->GetHTAt(StopIds[j]-IdOffset)->GetNOrigins(); ++oj) {
          if (m_SiEvent->GetHTAt(CentralIds[i]-IdOffset)->GetOriginAt(oi) == 
              m_SiEvent->GetHTAt(StopIds[j]-IdOffset)->GetOriginAt(oj)) {
            CommonOrigin = m_SiEvent->GetHTAt(StopIds[j]-IdOffset)->GetOriginAt(oj);
            break;
          }
        }
      }
    }
  }

  if (CommonOrigin == -1) {
    mdebug<<"   IsStop: No common origin!"<<endl;
    return false;
  }

  // Test (3)
  double Timing = -numeric_limits<double>::max();
  for (unsigned int i = 0; i < StopIds.size(); ++i) {
    if (m_SiEvent->GetHTAt(StopIds[i]-IdOffset)->GetTime() > Timing) {
      Timing = m_SiEvent->GetHTAt(StopIds[i]-IdOffset)->GetTime();
    }
  }
  unsigned int i_max = m_SiEvent->GetNHTs();
  for (unsigned int i = 0; i < i_max; ++i) {
    if (m_SiEvent->GetHTAt(i)->IsOrigin(CommonOrigin)) {
      if (m_SiEvent->GetHTAt(i)->GetTime() > Timing) {
        mdebug<<"   IsStop: Not last hit (timing)"<<endl;
        return false;
      }
    }
  }

  if (IsTrackCompletelyAbsorbed(StopIds, Energy) == false) {
    mdebug<<"   Is track stop: Not completely absorbed"<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::AreReseInSequence(MRESE& Start, 
                                                 MRESE& Central, 
                                                 MRESE& Stop, 
                                                 double Energy)
{
  // Return true if the given RESEs are in sequence
  //
  // According to the current simulation (GMega), the RESEs are in sequence if
  // (1) the IDs of their hits are in increasing order without holes
  // (2) all originate from the same IA (espec. the first and last hit in the 
  //     sequence)

  const int IdOffset = 2;

  vector<int> StartIds = GetReseIds(&Start);
  vector<int> CentralIds = GetReseIds(&Central);
  vector<int> StopIds = GetReseIds(&Stop);
  
  // Test (1)
  if (AreIdsInSequence(StartIds) == false) {
    mdebug<<"   IsCentral: Start IDs not in sequence"<<endl;
    return false;
  }
  if (AreIdsInSequence(CentralIds) == false) {
    mdebug<<"   IsCentral: Central IDs not in sequence"<<endl;
    return false;
  }
  if (AreIdsInSequence(StopIds) == false) {
    mdebug<<"   IsCentral: Stop IDs not in sequence"<<endl;
    return false;
  }
  // Test for holes:
  // The hit after the last one from start is the first from central:
  double TimeLastFirst = m_SiEvent->GetHTAt(StartIds.back()-IdOffset)->GetTime();
  double TimeFirstLast = m_SiEvent->GetHTAt(CentralIds.front()-IdOffset)->GetTime();
  for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
    if (m_SiEvent->GetHTAt(h)->GetTime() > TimeLastFirst && 
        m_SiEvent->GetHTAt(h)->GetTime() < TimeFirstLast) {
      mdebug<<"   IsCentral: Hit between first and central according to time!"<<endl;
      return false;      
    }
  } 
  TimeLastFirst = m_SiEvent->GetHTAt(CentralIds.back()-IdOffset)->GetTime();
  TimeFirstLast = m_SiEvent->GetHTAt(StopIds.front()-IdOffset)->GetTime();
  for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
    if (m_SiEvent->GetHTAt(h)->GetTime() > TimeLastFirst && 
        m_SiEvent->GetHTAt(h)->GetTime() < TimeFirstLast) {
      mdebug<<"   IsCentral: Hit between first and central according to time!"<<endl;
      return false;      
    }
  } 


  // Test (2)
  int CommonOrigin = -1;
  for (unsigned int i = 0; i < StartIds.size(); ++i) {
    for (unsigned int j = 0; j < CentralIds.size(); ++j) {
      for (unsigned int k = 0; k < StopIds.size(); ++k) {
        for (unsigned int oi = 0; oi < m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetNOrigins(); ++oi) {
          for (unsigned int oj = 0; oj < m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetNOrigins(); ++oj) {
            for (unsigned int ok = 0; ok < m_SiEvent->GetHTAt(StopIds[k]-IdOffset)->GetNOrigins(); ++ok) {
              if (m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetOriginAt(oi) == 
                  m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetOriginAt(oj) && 
                  m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetOriginAt(oi) == 
                  m_SiEvent->GetHTAt(StopIds[k]-IdOffset)->GetOriginAt(ok)) {
                CommonOrigin = m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetOriginAt(oj);
                break;
              }
            }
          }
        }
      }
    }
  }
  if (CommonOrigin == -1) {
    mdebug<<"   IsCentral: No common origin!"<<endl;
    return false;
  }

  // (3) Timing:
  if (m_SiEvent->GetHTAt(StartIds[0]-IdOffset)->GetTime() > 
      m_SiEvent->GetHTAt(CentralIds[0]-IdOffset)->GetTime() ||
      m_SiEvent->GetHTAt(CentralIds[0]-IdOffset)->GetTime() >
      m_SiEvent->GetHTAt(StopIds[0]-IdOffset)->GetTime()) {
    mdebug<<"   IsCentral: Timing wrong"<<endl;
    return false;
  }


  if (IsTrackCompletelyAbsorbed(CentralIds, Energy) == false) {
    mdebug<<"   IsCentral: Not completely absorbed"<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsComptonStart(MRESE& Start, double Etot, double Eres)
{
  // Return true if the given RESEs are in sequence
  //
  // A good start point for tripple Comptons requires:
  // (1) An absorption better than 3 sigma energy resolution + 2 keV for the fuirst hitr and the while sequence...
  // (2) The origin is or produced a photon
  // (3) We have exactly one Compton interaction and maybe some other, secondary interaction

  if (g_Verbosity >= c_Chatty) mout<<"IsComptonStart: Looking at: "<<Start.GetID()<<endl;


  // Get the origin IDs of the RESEs
  vector<int> StartOriginIds = GetOriginIds(&Start);
  if (StartOriginIds.size() == 0) {
    if (g_Verbosity >= c_Chatty) mout<<"IsComptonStart: Start has no Sim IDs!"<<endl;
    return false;
  }
  
  // Find the smallest origin ID in the sequence
  int SmallestOriginID = numeric_limits<int>::max();
  for (unsigned int i = 0; i < StartOriginIds.size(); ++i) {
    if (StartOriginIds[i] < SmallestOriginID) SmallestOriginID = StartOriginIds[i];
  }
  
  // Find the real Origin
  int TrueOrigin = m_SiEvent->GetIAAt(SmallestOriginID-1)->GetOriginID();
  
  // Test (1) - Absorption:
  if (Etot > 0) {
    // First hit
    if (IsAbsorbed(StartOriginIds, Start.GetEnergy(), Start.GetEnergyResolution()) == false) {
      if (g_Verbosity >= c_Chatty) mout<<"IsComptonStart: IA not completely absorbed!"<<endl;
      return false;
    }
    // Full sequence
    if (IsTotalAbsorbed(StartOriginIds, Etot, Eres) == false) {
      if (g_Verbosity >= c_Chatty) mout<<"IsComptonStart: Not completely absorbed!"<<endl;
      return false;
    }
  }
  
  // Test (2) - the start IA must be a photon (and create nothing) or be anything and create a photon
  MSimIA* OriginIA = m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(SmallestOriginID-1)->GetOriginID()-1);
  if (OriginIA->GetSecondaryParticleID() == 1 || // create photon
     (OriginIA->GetSecondaryParticleID() == 0 && OriginIA->GetMotherParticleID() == 1)) { // create nothing and be a photon
    // good...
  } else {
    // Only photons can be good...
    if (g_Verbosity >= c_Chatty) mout<<"IsComptonStart: IA which triggered first RESE did not create a photon or is not photon and create nothing"<<endl;
    return false;
  }
  
  // Check if the IA only contains Compton dependants
  if (ContainsOnlyComptonDependants(StartOriginIds) == false) {
    if (g_Verbosity >= c_Chatty) mout<<"IsComptonStart: Start contains not only Compton dependants"<<endl;
    return false;
  }
  
  // Check if we really have only one Compton interaction
  if (NumberOfComptonInteractions(StartOriginIds, TrueOrigin) != 1) {
    if (g_Verbosity >= c_Chatty) mout<<"IsComptonStart: Not exactly one Compton in interaction"<<endl;
      return false;       
  }
  
  if (g_Verbosity >= c_Chatty) mout<<"IsComptonStart: Passed!"<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MResponseMultipleCompton::NumberOfComptonInteractions(vector<int> AllSimIds, int Origin)
{
  unsigned int N = 0;
  
  for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
    if (m_SiEvent->GetIAAt(AllSimIds[i]-1)->GetProcess() == "COMP" && m_SiEvent->GetIAAt(AllSimIds[i]-1)->GetOriginID() == Origin)  {
      ++N;
    }
  }
  
  return N;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsComptonSequence(MRESE& Start, MRESE& Central, 
                                          int StartPosition, double Etot, 
                                          double Eres)
{
  // Return true if the given RESEs are in sequence
  //
  // A good start point for double Comptons requires:
  // (1) An absorption better than 99%
  // (2) the IDs of their hits are in increasing order without holes
  // (3) Start is the StartPosition's Compton IA

  mdebug<<"IsComptonSequence2: Looking at: "<<Start.GetID()<<" - "<<Central.GetID()<<endl;


  // Test (2)
  //vector<int> StartIds = GetReseIds(Start);
  vector<int> StartOriginIds = GetOriginIds(&Start);
  if (StartOriginIds.size() == 0) {
    mdebug<<"IsComptonSequence2: Start has no Sim IDs!"<<endl;
    return false;
  }
  
  //vector<int> CentralIds = GetReseIds(Central);
  vector<int> CentralOriginIds = GetOriginIds(&Central);
  if (CentralOriginIds.size() == 0) {
    mdebug<<"IsComptonSequence2: Central has no Sim IDs!"<<endl;
    return false;
  }
  

  if (ContainsOnlyComptonDependants(CentralOriginIds) == false) {
    mdebug<<"IsComptonSequence2: Central contains not only Compton dependants"<<endl;
    return false;
  }
  if (AreInComptonSequence(StartOriginIds, CentralOriginIds, StartPosition) == false) {
    mdebug<<"IsComptonSequence2: Not in Compton sequence!"<<endl;
    return false;
  }

  if (Etot > 0) {
    if (IsTotalAbsorbed(CentralOriginIds, Etot, Eres) == false) {
      mdebug<<"IsComptonSequence2: Not completely absorbed!"<<endl;
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsComptonSequence(MRESE& Start, MRESE& Central, 
                                          MRESE& Stop, int StartPosition, 
                                          double Etot, double Eres)
{
  // Return true if the given RESEs are in sequence

  // A good start point for tripple Comptons requires:
  // (1) An absorption better than 98%
  // (2) the IDs of their hits are in increasing order without holes

  mdebug<<"IsComptonSequence3: Looking at: "
        <<Start.GetID()<<" - "<<Central.GetID()<<" - "<<Stop.GetID()<<endl;


  // Test (2)
  //vector<int> StartIds = GetReseIds(Start);
  vector<int> StartOriginIds = GetOriginIds(&Start);
  if (StartOriginIds.size() == 0) {
    mdebug<<"IsComptonSequence3: Start has no Sim IDs!"<<endl;
    return false;
  }
  
//   // Absorption:
//   if (IsAbsorbed(StartOriginIds, Start.GetEnergy()) == false) {
//     mdebug<<"IsComptonSequence3: Start not completely absorbed!"<<endl;
//     return false;
//   }
  
  //vector<int> CentralIds = GetReseIds(Central);
  vector<int> CentralOriginIds = GetOriginIds(&Central);
  if (CentralOriginIds.size() == 0) {
    mdebug<<"IsComptonSequence3: Central has no Sim IDs!"<<endl;
    return false;
  }
  
  // Absorption:
  if (IsAbsorbed(CentralOriginIds, Central.GetEnergy(), Central.GetEnergyResolution()) == false) {
    mdebug<<"IsComptonSequence3: Central not completely absorbed!"<<endl;
    return false;
  }
  
  //vector<int> StopIds = GetReseIds(Stop);
  vector<int> StopOriginIds = GetOriginIds(&Stop);
  if (StopOriginIds.size() == 0) {
    mdebug<<"IsComptonSequence3: Stop has no Sim IDs!"<<endl;
    return false;
  }
  
//   // Absorption:
//   if (IsAbsorbed(StopOriginIds, Stop.GetEnergy()) == false) {
//     mdebug<<"IsComptonSequence3: Stop not completely absorbed!"<<endl;
//     return false;
//   }
  
  //cout<<"Cont only Start!"<<endl;
//   if (ContainsOnlyComptonDependants(StartOriginIds) == false) {
//     mdebug<<"IsComptonSequence3: Start contains not only Compton dependants"<<endl;
//     return false;
//   }
  //cout<<"Cont only Central!"<<endl;
  if (ContainsOnlyComptonDependants(CentralOriginIds) == false) {
    mdebug<<"IsComptonSequence3: Central contains not only Compton dependants"<<endl;
    return false;
  }
  if (ContainsOnlyComptonDependants(StopOriginIds) == false) {
    mdebug<<"IsComptonSequence3: Stop contains not only Compton dependants"<<endl;
    return false;
  }
  if (AreInComptonSequence(StartOriginIds, CentralOriginIds, StartPosition) == false) {
    mdebug<<"IsComptonSequence3: Not in Compton sequence!"<<endl;
    return false;
  }
  if (AreInComptonSequence(CentralOriginIds, StopOriginIds, 0) == false) {
    mdebug<<"IsComptonSequence3: Not in Compton sequence!"<<endl;
    return false;
  }

  if (IsTotalAbsorbed(CentralOriginIds, Etot, Eres) == false) {
    mdebug<<"IsComptonSequence3: Not completely absorbed!"<<endl;
    return false;
  }

  bool FoundBehind = false;
  for (unsigned int i = 0; i < StartOriginIds.size(); ++i) {
    if (StopOriginIds[0] - StartOriginIds[i] == 2) {
      FoundBehind = true;
    }
  }
  if (FoundBehind == false) {
    mdebug<<"IsComptonSequence3: More than one Compton between start and stop!"<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsComptonEnd(MRESE& End)
{
  // A good start point for tripple Comptons requires:
  // (1) An absorption better than 98%
  // (2) the IDs of their hits are in increasing order without holes
  // (3) We have one Compton interaction and some other interaction
  // (4) The origin is only the first Compton interaction

  mdebug<<"IsComptonEnd: Looking at: "<<End.GetID()<<endl;



  // Test (2)
//   vector<int> CentralIds = GetReseIds(Central);
//   vector<int> CentralOriginIds = GetOriginIds(CentralIds);
//   if (CentralOriginIds.size() == 0) {
//     mdebug<<"IsComptonEnd: Central has no Sim IDs!"<<endl;
//     return false;
//   }
  
  //vector<int> EndIds = GetReseIds(End);
  vector<int> EndOriginIds = GetOriginIds(&End);
  if (EndOriginIds.size() == 0) {
    mdebug<<"IsComptonEnd: End has no Sim IDs!"<<endl;
    return false;
  }
  
  // Absorption:
  if (IsTotalAbsorbed(EndOriginIds, End.GetEnergy(), End.GetEnergyResolution()) == false) {
    mdebug<<"IsComptonEnd: Not completely absorbed!"<<endl;
    return false;
  }

//   if (ContainsOnlyComptonDependants(CentralOriginIds) == false) {
//     mdebug<<"IsComptonEnd: Central contains not only Compton dependants"<<endl;
//     return false;
//   }
  if (ContainsOnlyComptonDependants(EndOriginIds) == false) {
    mdebug<<"IsComptonEnd: End contains not only Compton dependants"<<endl;
    return false;
  }
//   if (AreInComptonSequence(CentralOriginIds, EndOriginIds) == false) {
//     mdebug<<"IsComptonEnd: Not in Compton sequence!"<<endl;
//     return false;
//   }
  
  // Test (3)
  if (m_SiEvent->GetNIAs() < 3) { 
    mdebug<<"IsComptonEnd: Not enough interactions!"<<endl;
    return false;    
  }
  
//   // Test (4)
//   // End needs to contain the last interaction of track 1 
//   int LastIA = 0;
//   for (unsigned int i = 1; i < m_SiEvent->GetNIAs(); ++i) {
//     if (m_SiEvent->GetIAAt(i)->GetOriginID() == 1) {
//       LastIA = i+1;
//     } else {     
//       break;
//     }
//   }
//   bool FoundLastIA = false;
//   for (unsigned int i = 0; i < EndOriginIds.size(); ++i) {
//     if (EndOriginIds[i] == LastIA) {
//       FoundLastIA = true;
//       break;
//     }
//   }
//   if (FoundLastIA == false) {
//     mdebug<<"IsComptonEnd: Last interaction of track 1 not part of last hit"<<endl;
//     return false;    
//   }

  // Motherparticle of smallest ID needs to be a photon:
  int Smallest = EndOriginIds[0];
  for (unsigned int i = 1; i < EndOriginIds.size(); ++i) {
    if (EndOriginIds[i] < Smallest) Smallest = EndOriginIds[i];
  }

  if (m_SiEvent->GetIAAt(Smallest-1)->GetOriginID() == 0) {
    if (m_SiEvent->GetIAAt(Smallest-1)->GetProcess() != "ANNI" &&
        m_SiEvent->GetIAAt(Smallest-1)->GetProcess() != "INIT") {
      return false;
    }
  } else {
    if (m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOriginID()-1)->GetProcess() != "ANNI" &&
        m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOriginID()-1)->GetProcess() != "INIT") {
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsSingleCompton(MRESE& Start)
{
  // Check if all interaction in start are from one single Compton interaction:

  //cout<<Start.ToString()<<endl;

  //vector<int> StartIds = GetReseIds(Start);
  vector<int> StartOriginIds = GetOriginIds(&Start);
  if (StartOriginIds.size() == 0) {
    mdebug<<"IsSingleCompton: Start has no Sim IDs!"<<endl;
    return false;
  }

  int NPhotos = 0;
  int NComptons = 0;
  for (unsigned int i = 0; i < StartOriginIds.size(); ++i) {
    //cout<<"Id:"<<StartOriginIds[i]<<endl;
    if (m_SiEvent->GetIAAt(StartOriginIds[i]-1)->GetProcess() == "COMP") {
      NComptons++;
    } else if (m_SiEvent->GetIAAt(StartOriginIds[i]-1)->GetProcess() == "PHOT") {
      NPhotos++;
    }
  }

  if (NComptons == 1 && NPhotos >= 0) {
    mdebug<<"IsSingleCompton: Single Compton: C="<<NComptons<<" P="<<NPhotos<<endl;
    return true;
  } else if (NComptons == 0 && NPhotos >= 1) {
    mdebug<<"IsSingleCompton: No Compton but photo: C="<<NComptons<<" P="<<NPhotos<<endl;
    return true;
  } else {
    mdebug<<"IsSingleCompton: No single Compton or photo: C="<<NComptons<<" P="<<NPhotos<<endl;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::AreInComptonSequence(const vector<int>& StartOriginIds, 
                                                    const vector<int>& CentralOriginIds,
                                                    int StartPosition)
{
  // They are in good sequence if 
  //  (1) Both smallest IDs originate from 1
  //  (2) The IDs are in sequence

  massert(StartOriginIds.size() > 0);
  massert(CentralOriginIds.size() > 0);
  
  if (m_SiEvent->GetIAAt(StartOriginIds[0]-1)->GetProcess() != "COMP") {
    mdebug<<"CS: No COMP"<<endl;
    return false;
  }

  if (m_SiEvent->GetIAAt(StartOriginIds[0]-1)->GetOriginID() != 
      m_SiEvent->GetIAAt(CentralOriginIds[0]-1)->GetOriginID()) {
    mdebug<<"CS: No common origin"<<endl;
    return false;
  }

//   if (m_SiEvent->GetIAAt(StartOriginIds[0]-1)->GetMotherParticleNumber() != 1) {
//     mdebug<<"CS: Mother is no photon: "<<m_SiEvent->GetIAAt(StartOriginIds[0]-1)->GetMotherParticleNumber()<<endl;
//     return false;
//   }

  bool FoundBehind = false;
  for (unsigned int i = 0; i < StartOriginIds.size(); ++i) {
    //cout<<"testing: "<<CentralOriginIds[0]<<"-"<<StartOriginIds[i]<<endl;
    if (CentralOriginIds[0] - StartOriginIds[i] == 1) {
      FoundBehind = true;
      break;
    }
  }
  if (FoundBehind == false) {
    mdebug<<"CS: Not behind each other!"<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::ContainsOnlyComptonDependants(vector<int> AllSimIds)
{
  // We do two checks here, one down the tree and one up the tree
  
  // First check upwards: Is everything in there originating from somewhere else in there or is it an initial process (COMP, INIT, produced a photon)
  for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
    int ID = AllSimIds[i];
    int HighestOriginID = ID;
    while (true) {
      // Get new origin ID
      ID = m_SiEvent->GetIAAt(ID-1)->GetOriginID();
      // Check if included
      for (unsigned int j = 0; j < AllSimIds.size(); ++j) {
        if (i != j && ID == AllSimIds[j]) {
          HighestOriginID = ID;
          break;
        }
      }
      
      // Check if we are done
      if (m_SiEvent->GetIAAt(ID-1)->GetProcess() == "INIT") break; // Compton dependents, remember?
    }
    
    // We are good if the end point is a Compton scatter or a photo effect which is preceeded by a Compton in the list:
    bool IsGood = false;
    if (m_SiEvent->GetIAAt(HighestOriginID-1)->GetProcess() == "COMP") {
      IsGood = true;
    } else if (m_SiEvent->GetIAAt(HighestOriginID-1)->GetProcess() == "PHOT") {
      int Predeccessor = HighestOriginID-1;
      while (true) {
        if (Predeccessor == 1) break; // we reached init
        if (m_SiEvent->GetIAAt(Predeccessor-1)->GetOriginID() == m_SiEvent->GetIAAt(HighestOriginID-1)->GetOriginID()) {
          if (m_SiEvent->GetIAAt(Predeccessor-1)->GetProcess() == "COMP") {
            if (find(AllSimIds.begin(), AllSimIds.end(), Predeccessor) != AllSimIds.end()) {
              IsGood = true;
              break;
            } else {
              mdebug<<"ContainsOnlyComptonDependants: Preceeding Compton IA is missing"<<endl;
              return false;
            }
          }
        }
        --Predeccessor;
      }
    }
    if (IsGood == false) {
      mdebug<<"ContainsOnlyComptonDependants: Hits other than dependants for ID="<<ID<<endl;
      return false;
    }
  }

  
  // The second check is downward: Are all particles generated from this process contained?
  for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
    vector<int> Descendents;
    Descendents.push_back(AllSimIds[i]);
    unsigned int Size = 1;
    do {
      Size = Descendents.size();
      for (unsigned int j = 0; j < m_SiEvent->GetNIAs(); ++j) {
        for (unsigned int d = 0; d < Descendents.size(); ++d) {
          if (m_SiEvent->GetIAAt(j)->GetOriginID() == Descendents[d]) {
            if (find(Descendents.begin(), Descendents.end(), j+1) != Descendents.end()) {
              Descendents.push_back(j+1);
            }
          }
        }
      }
    } while (Size < Descendents.size());
    
    // Now check if all are contianed:
    for (unsigned int d = 0; d < Descendents.size(); ++d) {
      if (m_SiEvent->GetIAAt(Descendents[d]-1)->GetSecondaryParticleID() != 0 && m_SiEvent->GetIAAt(Descendents[d]-1)->GetSecondaryParticleID() != 1) {
        bool Found = false;
        for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
          if (d == i) {
            Found = true; 
          }
        }
        if (Found == false) {
          mdebug<<"ContainsOnlyComptonDependants: Originates but not contained: ID="<<d<<endl; 
        }
      }
    }
  }
  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsAbsorbed(const vector<int>& AllSimIds, 
                                          double Energy, double EnergyResolution)
{
  // Rules:
  // If a Compton scattering is included, the energy of the of the recoild electron plus any instant deposits, bremsstrahlug, etc. must be contained
  // If a photo is included, the energy of the INCOMING gamma-ray must be contained
  
  EnergyResolution = 3.0*EnergyResolution + 2.0;
  
  double IdealEnergy = 0.0;
  
  vector<int> SimIDs = AllSimIds;
  
  // (1) First clean the IDs
  vector<int> CleanedSimIDs;
  // (a) If we have secondaries, remove them if the parent is contained
  //     Or inverse, keep all IAs whose parents are not included
  for (unsigned int i = 0; i < SimIDs.size(); ++i) {
    bool Found = false;
    for (unsigned int j = 0; j < SimIDs.size(); ++j) {
      if (i != j) {
        if (m_SiEvent->GetIAAt(SimIDs[i]-1)->GetOriginID() == SimIDs[j]) {
          Found = true;
          break;
        }
      }
    }
    if (Found == false) {
      CleanedSimIDs.push_back(SimIDs[i]);
    }
  }
  SimIDs = CleanedSimIDs;
  CleanedSimIDs.clear();
  
  // (b) Keep only the smallest PHOT IA since we require it to be contained
  for (unsigned int i = 0; i < SimIDs.size(); ++i) {
    if (m_SiEvent->GetIAAt(SimIDs[i]-1)->GetProcess() == "PHOT") {
      if (m_SiEvent->GetIAAt(SimIDs[i]-1)->GetOriginID() == m_SiEvent->GetIAAt(SimIDs[i]-2)->GetOriginID() && m_SiEvent->GetIAAt(SimIDs[i]-2)->GetProcess() != "PHOT") {
        CleanedSimIDs.push_back(SimIDs[i]);
      }
    } else {
      CleanedSimIDs.push_back(SimIDs[i]);
    }
  }
  SimIDs = CleanedSimIDs;
  CleanedSimIDs.clear();
    
  // (c) Exclude fluoresence COMP
  for (unsigned int i = 0; i < SimIDs.size(); ++i) {
    if (m_SiEvent->GetIAAt(SimIDs[i]-1)->GetProcess() == "COMP") {
      if (m_SiEvent->GetIAAt(SimIDs[i]-1)->GetSecondaryParticleID() == 3) {
        CleanedSimIDs.push_back(SimIDs[i]);
      }
    } else {
      CleanedSimIDs.push_back(SimIDs[i]);
    }
  }
  SimIDs = CleanedSimIDs;
  CleanedSimIDs.clear();
  
  mdebug<<"SimIDs: "; for (int i: SimIDs) mdebug<<i<<" "; mdebug<<endl;
  
  // (d) Sanity check - we just should have COMP & PHOT in our list
  for (unsigned int i = 0; i < SimIDs.size(); ++i) {
    if (m_SiEvent->GetIAAt(SimIDs[i]-1)->GetProcess() != "COMP" && m_SiEvent->GetIAAt(SimIDs[i]-1)->GetProcess() != "PHOT") {
      cout<<"Error: We only should have COMP and PHOT IA's at this point and not \""<<m_SiEvent->GetIAAt(SimIDs[i]-1)->GetProcess()<<"\". Did you use hadronic processes for the simulations?"<<endl;
    }
  }
  
  // (2) Now add the energies
  for (unsigned int i = 0; i < SimIDs.size(); ++i) {
    if (m_SiEvent->GetIAAt(SimIDs[i]-1)->GetProcess() == "COMP") {
      if (m_SiEvent->GetIAAt(SimIDs[i]-2)->GetProcess() == "COMP" || m_SiEvent->GetIAAt(SimIDs[i]-2)->GetProcess() == "RAYL") {
        mdebug<<"COMP with COMP/RAYL predeccessor: Adding mother difference energy: "<<m_SiEvent->GetIAAt(SimIDs[i]-2)->GetMotherEnergy() - m_SiEvent->GetIAAt(SimIDs[i]-1)->GetMotherEnergy()<<endl;
        IdealEnergy += m_SiEvent->GetIAAt(SimIDs[i]-2)->GetMotherEnergy() - m_SiEvent->GetIAAt(SimIDs[i]-1)->GetMotherEnergy();
      } else {
        mdebug<<"COMP WITHOUT COMP/RAYL predeccessor: Adding secondary-mother difference energy: "<<m_SiEvent->GetIAAt(SimIDs[i]-2)->GetSecondaryEnergy() - m_SiEvent->GetIAAt(SimIDs[i]-1)->GetMotherEnergy()<<endl;
        IdealEnergy += m_SiEvent->GetIAAt(SimIDs[i]-2)->GetSecondaryEnergy() - m_SiEvent->GetIAAt(SimIDs[i]-1)->GetMotherEnergy();
      }
    } else if (m_SiEvent->GetIAAt(SimIDs[i]-1)->GetProcess() == "PHOT") {
      if (m_SiEvent->GetIAAt(SimIDs[i]-2)->GetProcess() == "COMP" || m_SiEvent->GetIAAt(SimIDs[i]-2)->GetProcess() == "RAYL") {
        mdebug<<"PHOT with COMP/RAYL predeccessor: Adding mother energy: "<<m_SiEvent->GetIAAt(SimIDs[i]-2)->GetMotherEnergy()<<endl;
        IdealEnergy += m_SiEvent->GetIAAt(SimIDs[i]-2)->GetMotherEnergy();
      } else {
        mdebug<<"PHOT withOUT COMP/RAYL predeccessor: Adding secondary energy: "<<m_SiEvent->GetIAAt(SimIDs[i]-2)->GetSecondaryEnergy()<<endl;
        IdealEnergy += m_SiEvent->GetIAAt(SimIDs[i]-2)->GetSecondaryEnergy();
      }      
    } else {
      cout<<"Error: We only should have COMP and PHOT IA's at this point. Did you use hadronic processes for the simulations?"<<endl;      
    }
  }
  
  // (3) Check for absorption
  if (fabs(Energy - IdealEnergy) > EnergyResolution) {
    mdebug<<"IsAbsorbed(): Not completely absorbed: ideal="<<IdealEnergy<<"  observed="<<Energy<<"  3*Eres+2="<<EnergyResolution<<endl;
    return false;
  } else {
    mdebug<<"IsAbsorbed(): Completely absorbed: ideal="<<IdealEnergy<<" observed="<<Energy<<endl;
  }
  return true;  
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsTotalAbsorbed(const vector<int>& AllSimIds, 
                                               double Energy, double EnergyResolution)
{
  massert(AllSimIds.size() > 0);

  EnergyResolution = 3*EnergyResolution+2;

  int MinId = numeric_limits<int>::max();
  for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
    if (MinId > AllSimIds[i] && AllSimIds[i] > 1) MinId = AllSimIds[i];
  }

  if (MinId == numeric_limits<int>::max()) return false;

  double Ideal;
  MSimIA* Top = 0;
  if (m_SiEvent->GetIAAt(MinId-2)->GetOriginID() == m_SiEvent->GetIAAt(MinId-1)->GetOriginID()) {
    Top = m_SiEvent->GetIAAt(MinId-2);
  } else {
    Top = m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(MinId-1)->GetOriginID()-1);
  }

  if (m_SiEvent->GetIAAt(MinId-2)->GetProcess() == "COMP") {
    Ideal = Top->GetMotherEnergy();
  } else {
    Ideal = Top->GetSecondaryEnergy();
  }

  if (MinId-2 != 0) {
    Ideal = m_SiEvent->GetIAAt(MinId-2)->GetMotherEnergy();
  } else {
    Ideal = m_SiEvent->GetIAAt(MinId-2)->GetSecondaryEnergy();
  }

  if (fabs(Ideal - Energy) > EnergyResolution) {
      mdebug<<"Is totally absorbed: Not completely absorbed: Tot abs: i="<<Ideal<<"  r="<<Energy<<"  3*Eres+2="<<EnergyResolution<<endl;
    return false;
  } else {
    mdebug<<"Is totally absorbed: Completely absorbed: Tot abs: i="<<Ideal<<"  r="<<Energy<<"  3*Eres+2="<<EnergyResolution<<endl;
  }
//   if (fabs((Ideal - Energy)/Ideal) > m_MaxEnergyDifferencePercent &&
//       fabs(Ideal - Energy) > m_MaxEnergyDifference ) {
//       mdebug<<"Is totally absorbed: Not completely absorbed: Tot abs: i="<<Ideal<<" r="<<Energy<<endl;
//     return false;
//   }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::GetIdealDepositedEnergy(int MinId)
{
  //  

  double Ideal = 0;

  MSimIA* Top = 0;
  MSimIA* Bottom = 0;
  if (m_SiEvent->GetIAAt(MinId-2)->GetOriginID() == m_SiEvent->GetIAAt(MinId-1)->GetOriginID()) {
    Top = m_SiEvent->GetIAAt(MinId-2);
    Bottom = m_SiEvent->GetIAAt(MinId-1);
  } else {
    Top = m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(MinId-1)->GetOriginID()-1);
    Bottom = m_SiEvent->GetIAAt(MinId-1);
  }

  if (m_SiEvent->GetIAAt(MinId-2)->GetProcess() == "COMP") {
    Ideal = Top->GetMotherEnergy()-Bottom->GetMotherEnergy();
  } else {
    Ideal = Top->GetSecondaryEnergy()-Bottom->GetMotherEnergy();
  }

  return Ideal;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsTrackCompletelyAbsorbed(const vector<int>& Ids, double Energy)
{
  // Return true if the track is completely absorbed
  //
  // Prerequisites: Ids have only one common origin!
  // Realization: Nothing originates from this ID AFTER this hit:

  const int IdOffset = 2;

  // Get origin Id
  int Origin = -1;
  for (unsigned int i = 0; i < Ids.size(); ++i) {
    for (unsigned int oi = 0; oi < m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetNOrigins(); ++oi) {
      if (m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetOriginAt(oi) != 1) { 
        Origin = m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetOriginAt(oi);
        break;
      }
    }
  } 
  if (Origin <= 1) {
    mdebug<<"   IsTrackCompletelyAbsorbed: No origin"<<endl;
    return false;
  }

//   // Check if we do haven an IA originating from this - which is not part of this track:
//   for (unsigned int i = 0; i < m_SiEvent->GetNIAs(); ++i) {
//     if (m_SiEvent->GetIAAt(i)->GetOriginID() == Origin) {
//       if (m_SiEvent->GetIAAt(i)->GetProcess() == "BREM") {
//         // Search the closest hit:
//         int MinHT = -1;
//         double MinDist = numeric_limits<double>::max();
//         for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
//           if (m_SiEvent->GetHTAt(h)->IsOrigin(Origin) == true) {
//             if ((m_SiEvent->GetIAAt(i)->GetPosition() - m_SiEvent->GetHTAt(h)->GetPosition()).Mag() < MinDist) {
//               MinDist = (m_SiEvent->GetIAAt(i)->GetPosition() - m_SiEvent->GetHTAt(h)->GetPosition()).Mag();
//               MinHT = h;
//             }
//           }
//         }
//         if (MinHT > 0) {
//           if (m_SiEvent->GetHTAt(MinHT)->GetTime() >= m_SiEvent->GetHTAt(Ids[0]-IdOffset)->GetTime()) {
//             mdebug<<"   IsTrackCompletelyAbsorbed: Bremsstrahlung emitted after this hit!"<<endl;
//             return false;
//           }
//         }
//       } else {
//         mdebug<<"   IsTrackCompletelyAbsorbed: Originating interaction: "<<m_SiEvent->GetIAAt(i)->GetProcess()<<endl;
//         return false;
//       }
//     }
//   }

  // Check for complete absorption:
  const double EnergyLimitPercent = 0.15; // %
  const double EnergyLimit = 20; //keV

  double RealEnergy = 0;
  for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
    if (m_SiEvent->GetHTAt(h)->IsOrigin(Origin) == true) {
      if (m_SiEvent->GetHTAt(h)->GetTime() >= m_SiEvent->GetHTAt(Ids[0]-IdOffset)->GetTime()) {
        RealEnergy += m_SiEvent->GetHTAt(h)->GetEnergy();
      }
    }
  }
  
  if (fabs(Energy - RealEnergy) > EnergyLimit &&
      fabs(Energy - RealEnergy)/RealEnergy > EnergyLimitPercent) {
    mdebug<<"   IsTrackCompletelyAbsorbed: Missing/Not much energy: i="
          <<RealEnergy<<" r="<<Energy<<endl;
    return false;   
  }


  return  true;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateMinLeverArm(MRESE& Start, MRESE& Central, 
                                               MRESE& Stop)
{
  return MERCSRBayesian::CalculateMinLeverArm(Start.GetPosition(),
                                              Central.GetPosition(),
                                              Stop.GetPosition());
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateDPhiInDegree(MRESE& Start, MRESE& Central, 
                                        MRESE& Stop, double Energy)
{
  return MERCSRBayesian::CalculateDPhiInDegree(&Start, &Central, &Stop, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateDCosPhi(MRESE& Start, MRESE& Central, 
                                                  MRESE& Stop, double Energy)
{
  return MERCSRBayesian::CalculateDCosPhi(&Start, &Central, &Stop, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateDCosAlpha(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateDCosAlpha(&Track, &Central, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateDAlphaInDegree(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateDAlphaInDegree(&Track, &Central, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateCosAlphaE(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateCosAlphaE(&Track, &Central, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateAlphaEInDegree(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateAlphaEInDegree(&Track, &Central, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateCosAlphaG(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateCosAlphaG(&Track, &Central, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateAlphaGInDegree(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateAlphaGInDegree(&Track, &Central, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateAbsorptionProbabilityPhoto(MRESE& Start, 
                                                                     MRESE& Stop, 
                                                                     double Etot)
{
  double Probability = 
    m_ReGeometry->GetPhotoAbsorptionProbability(Start.GetPosition(), 
                                                Stop.GetPosition(), Etot); 

  //cout<<"Probability: "<<Probability<<" E: "<<Etot<<endl;
  //cout<<Start.GetPosition()<<":"<<Stop.GetPosition()<<":"<<(Stop.GetPosition()-Start.GetPosition()).Mag()<<endl;
  return Probability;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateAbsorptionProbabilityTotal(MRESE& Start, 
                                                                     MRESE& Stop, 
                                                                     double Etot)
{
  double Probability = 
    m_ReGeometry->GetAbsorptionProbability(Start.GetPosition(), 
                                           Stop.GetPosition(), Etot); 

  //cout<<"Probability: "<<Probability<<" E: "<<Etot<<endl;
  //cout<<Start.GetPosition()<<":"<<Stop.GetPosition()<<":"<<(Stop.GetPosition()-Start.GetPosition()).Mag()<<endl;
  return Probability;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateAbsorptionProbabilityCompton(MRESE& Start, 
                                                                       MRESE& Stop, 
                                                                       double Etot)
{
  double Distance = 
    m_ReGeometry->GetComptonAbsorptionProbability(Start.GetPosition(), 
                                                  Stop.GetPosition(), Etot); 

  //cout<<"Distance: "<<Distance<<" E: "<<Etot<<endl;
  //cout<<Start.ToString()<<endl;
  //cout<<Stop.ToString()<<endl;
  return Distance;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculatePhiEInDegree(MRESE& Central, double Etot)
{
  return MERCSRBayesian::CalculatePhiEInDegree(&Central, Etot);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculatePhiGInDegree(MRESE& Start, MRESE& Central, 
                                                       MRESE& Stop)
{
  return MERCSRBayesian::CalculatePhiGInDegree(&Start, &Central, &Stop);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateCosPhiG(MRESE& Start, MRESE& Central, 
                                                  MRESE& Stop)
{
  return MERCSRBayesian::CalculateCosPhiG(&Start, &Central, &Stop);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateCosPhiE(MRESE& Central, double Etot)
{
  return MERCSRBayesian::CalculateCosPhiE(&Central, Etot);
}


////////////////////////////////////////////////////////////////////////////////


int MResponseMultipleCompton::GetMaterial(MRESE& Hit)
{
  return MERCSRBayesian::GetMaterial(&Hit);
}


// MResponseMultipleCompton.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
