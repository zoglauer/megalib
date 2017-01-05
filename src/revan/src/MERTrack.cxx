/*
 * MERTrack.cxx
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
// MERTrack
//
// Base class for event reconstruction tasks, e.g. find clusters, tracks,
// Compton sequence
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERTrack.h"

// Standard libs:
#include <list>
#include <algorithm>
#include <functional>
#include <limits>
#include <cfloat>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MRESE.h"
#include "MRETrack.h"
#include "MREHit.h"
#include "MMuonEvent.h"
#include "MTimer.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MERTrack)
#endif



////////////////////////////////////////////////////////////////////////////////


// Some really small helpers:
struct MRESEDist {
  MRESEDist(MRESE* R, int D) { massert(R!=0); RESE=R; Distance=D; }

  MRESE* RESE;
  int Distance;
};

bool MRESEDistCompare(const MRESEDist& a, const MRESEDist& b) {
    return (a.Distance < b.Distance);
}

///

struct MRESETrackCont {
  MRESETrackCont(MRETrack* T, int e1, MRESE* R, int e2, double A) { 
    massert(T!=0); massert(R!=0); m_Track=T; m_EndPoint1 = e1; m_Next=R; m_EndPoint2 = e2; m_Angle=A; }

  MRETrack* m_Track;
  int m_EndPoint1;
  MRESE* m_Next;
  int m_EndPoint2;
  double m_Angle;
};

bool MRESETrackContCompare(const MRESETrackCont& a, const MRESETrackCont& b) {
    return (a.m_Angle < b.m_Angle);
}


////////////////////////////////////////////////////////////////////////////////


MERTrack::MERTrack()
{
  // Construct an instance of MERTrack

  m_SearchPairs = true;
  m_SearchMIPs = true;
  m_SearchComptons = true;

  m_MaxNAmbiguities = 5;
  m_AllowOnlyMinNumberOfRESEsD1 = false;

  m_ComptonMaxLayerJump = 2;
  m_RejectPureAmbiguities = false;

  m_TimePairs = 0;
  m_TimeMips = 0;
  m_TimeComptonSequences = 0;
  m_TimeComptonDirections = 0;
  
  m_NLayersForVertexSearch = 5;
}


////////////////////////////////////////////////////////////////////////////////


MERTrack::~MERTrack()
{
  // Delete this instance of MERTrack
}


////////////////////////////////////////////////////////////////////////////////


bool MERTrack::SetParameters(bool SearchMIPs, 
                             bool SearchPairs, 
                             bool SearchComptons, 
                             unsigned int MaxLayerJump, 
                             unsigned int NSequencesToKeep, 
                             bool RejectPureAmbiguities,
                             unsigned int NLayersForVertexSearch,
                             vector<MString> DetectorList)
{
  m_SearchMIPs = SearchMIPs;
  m_SearchPairs = SearchPairs;
  m_SearchComptons = SearchComptons;
  m_ComptonMaxLayerJump = MaxLayerJump;
  m_NSequencesToKeep = NSequencesToKeep;
  m_RejectPureAmbiguities = RejectPureAmbiguities;
  m_NLayersForVertexSearch = NLayersForVertexSearch;
 
  if (m_NLayersForVertexSearch < 4) {
    merr<<"Error: Revan (tracking): NLayersForVertexSearch (="<<m_NLayersForVertexSearch<<") must be >= 4. Setting it to 4."<<show;
    m_NLayersForVertexSearch = 4;
  }
  
  if (m_ComptonMaxLayerJump < 1) {
    merr<<"Error: Revan (tracking): MaxLayerJump must be >= 1. Setting it to 1."<<show;   
    m_ComptonMaxLayerJump = 1;
  }

  if (m_NSequencesToKeep < 1) {
    merr<<"Error: Revan (tracking): NSequencesToKeep must be >= 1. Setting it to 1."<<show;   
    m_NSequencesToKeep = 1;
  }
  
  // Check if for the tracking detector names real detectors exist
  if (DetectorList.size() == 0) {
    merr<<"Error: Revan (tracking): You did not give *any* detectors for electron tracking."<<endl;
    merr<<"No electron tracking will be performed!"<<show;
  } else {
    for (unsigned int n = 0; n < DetectorList.size(); ++n) {
      bool Found = false;
      for (unsigned int d = 0; d < m_Geometry->GetNDetectors(); ++d) {
        if (m_Geometry->GetDetectorAt(d)->GetName() == DetectorList[n]) {
          m_DetectorList.push_back(m_Geometry->GetDetectorAt(d));
          Found = true;
        }
      }
      if (Found == false) {
        merr<<"Error: Revan (tracking): Did not find detector "<<DetectorList[n]<<" in geometry."<<endl;
        merr<<"No electron tracking will be performed in this detector!"<<show;
      }
    }
    if (m_DetectorList.size() == 0) {
      merr<<"Error: Revan (tracking): I could not find *any* detectors for electron tracking."<<endl;
      merr<<"No electron tracking will be performed!"<<show;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MERTrack::Analyze(MRawEventList* REList)
{
  // Analyze the raw event...

  massert(m_Geometry != 0);

  MERConstruction::Analyze(REList);

  // mdebug<<m_List->ToString()<<endl;

  mdebug<<"Starting tracking"<<endl;


  bool Identified = false;
  MRERawEvent* RE = 0;
  MRawEventList* List = 0;
  MRESE* RESE = 0;
  MRETrack* Track = 0;
  MTimer Timer;

  // Step A: Preparation:

  // Before we do anything we check if we have at least two hits in one of the defines electron trackers
  bool StartTracking = false;
  for (int e = 0; e < m_List->GetNRawEvents(); e++) {
    RE = m_List->GetRawEventAt(e);
    unsigned int NHits = 0;
    for (int h = 0; h < RE->GetNRESEs(); ++h) {
      if (IsInTracker(RE->GetRESEAt(h)) == true) {
        ++NHits;
      }
    }
    //if (NHits > 50) {
    //  mout<<"Too many hits for tracking: "<<NHits<<endl;
    //  return true;
    //}
    if (NHits >= 2) {
      StartTracking = true;
      break;
    }
  }

  if (StartTracking == false) {
    mdebug<<"Not enough hits in tracker!"<<endl;
    return true;
  }

  // Step B: Pairs

  Timer.Start();
  if (m_SearchPairs == true) {
    mdebug<<"* Search for pairs: Vertex"<<endl;
    bool HasVertices = false;
    int e_max = m_List->GetNRawEvents();
    for (int e = 0; e < e_max; e++) {
      RE = m_List->GetRawEventAt(e);
      List = CheckForPair(RE);
      if (List != 0) {
        mdebug<<"List: "<<List->GetNRawEvents()<<endl;
        HasVertices = true;
        m_List->DeleteRawEvent(RE);
        e--;
        e_max--;
        for (int i = 0; i < List->GetNRawEvents(); i++) {
          m_List->AddRawEvent(List->GetRawEventAt(i));
        }
        delete List;
        mdebug<<"Tracking: Event has vertex!"<<endl;
      }
    }
    if (HasVertices == true) {
      // Let's analyze pairs: 
      mdebug<<"* Step: Pair tracking"<<endl;
      mdebug<<m_List->ToString()<<endl;
      int e_max = m_List->GetNRawEvents();
      for (int e = 0; e < e_max; e++) {
        RE = m_List->GetRawEventAt(e);
        if (RE->GetVertex() != 0) {
          TrackPairs(RE);
        }
      }
      mdebug<<"Tracking:  Successful identification: Pair event!"<<endl;
      mdebug<<"        Pair results:"<<endl;
      mdebug<<m_List->ToString()<<endl;
      mdebug<<endl;
      Identified = true;
      m_List->SetOptimumEvent(RE);
      m_List->SetBestTryEvent(RE);
    } // pair...
  }
  m_TimePairs += Timer.ElapsedTime();


  // Step C: Mips

  Timer.Start();
  if (Identified == false && m_SearchMIPs == true) {
    mdebug<<"* Search for MIPs"<<endl;
    for (int e = 0; e < m_List->GetNRawEvents(); e++) {
      RE = m_List->GetRawEventAt(e);
      CheckForMips(RE);
      // If we have found a MIP then we are done!
      if (RE->GetEventType() == MRERawEvent::c_MipEvent) {
        mdebug<<"Tracking: Successful identification: MIP (probably muon)"<<endl;
        Identified = true;
        m_List->SetOptimumEvent(RE);
        m_List->SetBestTryEvent(RE);
      }
      if (RE->GetEventType() == MRERawEvent::c_ShowerEvent) {
        mdebug<<"Tracking: Successful identification: Shower"<<endl;
        Identified = true;
        m_List->SetOptimumEvent(RE);
        m_List->SetBestTryEvent(RE);
      }
    }
    mdebug<<"        MIP results:"<<endl;
    mdebug<<m_List->ToString()<<endl;
    mdebug<<endl;
  }
  m_TimeMips += Timer.ElapsedTime();


  // Step C: Compton tracks:

  if (Identified == false && m_SearchComptons == true) {
    mdebug<<"* Search for Comptons"<<endl;

    // Step C.1: Generate the links

    Timer.Start();
    // Generate a list of possible Compton tracks -
    // we only want to have sequences now and do not yet care aboput directions!
    int e_max = m_List->GetNRawEvents();
    for (int e = 0; e < e_max; e++) {
      RE = m_List->GetRawEventAt(e);
      List = TrackComptons(RE);
      if (List != 0) {
        m_List->DeleteRawEvent(RE);
        e--;
        e_max--;
        for (int i = 0; i < List->GetNRawEvents(); i++) {
          m_List->AddRawEvent(List->GetRawEventAt(i));
        }
        delete List;
      }
    }
    m_TimeComptonSequences += Timer.ElapsedTime();


    // We now accept only events with the maximum possible number of tracks, 
    // if the flag is set
    if (m_AllowOnlyMinNumberOfRESEsD1 == true) {
      mdebug<<"Allowing only events with the maximum number of elements in the tracks. Starting with this amount: "<<m_List->GetNRawEvents()<<endl;
      int MinN = numeric_limits<int>::max();
      for (int e = 0; e < m_List->GetNRawEvents(); e++) {
        int CurrentN = 0;
        RE = m_List->GetRawEventAt(e);
        for (int r = 0; r < RE->GetNRESEs(); r++) {
          RESE = RE->GetRESEAt(r);
          if (IsInTracker(RESE) == true) {
            CurrentN++;
          }
        }
        if (CurrentN < MinN) MinN = CurrentN;
      }

      for (int e = 0; e < m_List->GetNRawEvents(); e++) {
        int CurrentN = 0;
        RE = m_List->GetRawEventAt(e);
        for (int r = 0; r < RE->GetNRESEs(); r++) {
          RESE = RE->GetRESEAt(r);
          if (IsInTracker(RESE) == true) CurrentN++;
        }
        if (CurrentN > MinN) {
          m_List->DeleteRawEvent(RE);
          e--;
        }
      }
      mdebug<<"Keeping "<<m_List->GetNRawEvents()<<" possibilities for further processing."<<endl;
    }


    // Step C.2: Duplicate each track and assign the two possible start positions

    Timer.Start();
    mdebug<<"Checking for track directions..."<<endl;

    int IDRESE = 0;
    int IDEP = 0;
    MRERawEvent* REdup = 0;

    for (int e = 0; e < m_List->GetNRawEvents(); e++) {
      RE = m_List->GetRawEventAt(e);
        
      for (int r = 0; r < RE->GetNRESEs(); r++) {
        RESE = RE->GetRESEAt(r);
        IDRESE = RESE->GetID();
        if (RESE->GetType() == MRESE::c_Track) {
          Track = (MRETrack *) RESE;
          if (Track->GetStartPoint() == 0) {
            massert(Track->GetNEndPoints() == 2);

            IDEP = Track->GetEndPointAt(0)->GetID();
            REdup = RE->Duplicate();
            ((MRETrack *) (REdup->GetRESE(IDRESE)))->SetStartPoint(((MRETrack *) (REdup->GetRESE(IDRESE)))->GetRESE(IDEP));
            m_List->AddRawEvent(REdup);
            
            Track->SetStartPoint(Track->GetEndPointAt(1));
          }
        }
      }
    }

    // Step C.3:  Find the most probable combination

    for (int e = 0; e < m_List->GetNRawEvents(); e++) {
      RE = m_List->GetRawEventAt(e);
      EvaluateTracks(RE);
    }
    SortByTrackQualityFactor(m_List);

    // We always have 1 raw event - the original one!
    massert(m_List->GetNRawEvents() >= 1);

    mdebug<<m_List->ToString()<<endl;

    mdebug<<"Step 5: Best track solution out of "<<m_List->GetNRawEvents()<<": "<<endl
          <<m_List->GetRawEventAt(0)->ToString()<<endl;

    mdebug<<"Tracking: Best Compton track: "<<m_List->GetRawEventAt(0)->GetTrackQualityFactor()<<endl;

    //m_List->SetValidEvent(m_List->GetRawEventAt(m_List->GetNRawEvents()-1));
    m_List->SetBestTryEvent(m_List->GetRawEventAt(0));
       

    // Remove all not-so-good sequences:
    for (int e = m_NSequencesToKeep; e < m_List->GetNRawEvents(); ++e) {
      RE = m_List->GetRawEventAt(e);
      m_List->DeleteRawEvent(RE);
      e--;
    }
    
    m_TimeComptonDirections += Timer.ElapsedTime();
  }


  // Step E: Epilog
  
  mdebug<<"Result of tracking:"<<endl;
  mdebug<<m_List->ToString()<<endl;


  return true;
}



////////////////////////////////////////////////////////////////////////////////


bool MERTrack::IsInTracker(MRESE* R)
{
  //! Return true, if the RESE happened in a detector in our list

  // While building tracks their volume sequence might not be correct
  // But, if we have a track, it is in the tracker... so:
  if (R->GetType() == MRESE::c_Track) return true;
  
  MDVolumeSequence* VS = R->GetVolumeSequence();
  for (unsigned int d = 0; d < m_DetectorList.size(); ++d) {
    if (VS->GetDetector() == m_DetectorList[d]) {
      return true;
    }
  }
  return false;
}
  
  
////////////////////////////////////////////////////////////////////////////////


void MERTrack::SortByTrackQualityFactor(MRawEventList* List)
{
  List->SortByTrackQualityFactor(true);
}


////////////////////////////////////////////////////////////////////////////////


MRawEventList* MERTrack::CheckForPair(MRERawEvent* RE)
{
  // Check if this event could be a pair:
  //
  // The typical pattern of a pair is:
  // (a) There is a vertex
  // (b) In N layers above/below are at least two hits

  // Search the vertex:
  // The vertex is at this point a (non-ambiguous) track or a single (clustered) 
  // hit followed by at least N layers with two hits per layer:

  mdebug<<"Searching for a pair vertex"<<endl;

  MRawEventList *List = 0;
  bool OnlyHitInLayer = false;
  unsigned int MaximumLayerJump = 2;
  //MRESE* RESE = 0;
  unsigned int SearchRange = 30; 
  int MinPairLength = m_NLayersForVertexSearch;
  
  // Create a list of RESEs sorted by depth in tracker
  vector<MRESE*> ReseList;
  for (int h = 0; h < RE->GetNRESEs(); h++) {
    if (IsInTracker(RE->GetRESEAt(h)) == false) continue;
    // Everthing below 90 has to go -- eliminate Bremsstrahlung
    //if (RE->GetRESEAt(h)->GetEnergy() < 90) continue;
    
    ReseList.push_back(RE->GetRESEAt(h));
  }
  sort(ReseList.begin(), ReseList.end(), CompareRESEByZ());

  mdebug<<"RESE's sorted by depth: "<<endl;
  vector<MRESE*>::iterator Iterator1;
  vector<MRESE*>::iterator Iterator2;
  for (Iterator1 = ReseList.begin(); Iterator1 != ReseList.end(); Iterator1++) {
    mdebug<<(*Iterator1)->GetID()<<": "<<(*Iterator1)->GetPosition().Z()<<endl;
  }

  // For each of the RESE's in the list check if it could be the first of the vertex
  for (Iterator1 = ReseList.begin(); Iterator1 != ReseList.end(); Iterator1++) {
 
    // If it is a single hit, and if it is the only one in its layer:
    OnlyHitInLayer = true;
    for (Iterator2 = ReseList.begin(); Iterator2 != ReseList.end(); Iterator2++) {
      if ((*Iterator1) == (*Iterator2)) continue;
      if (m_Geometry->AreInSameLayer((*Iterator1), (*Iterator2)) == true) {
        OnlyHitInLayer = false;
        break;
      }
    }
    if (OnlyHitInLayer == false) continue;
    mdebug<<"Search vertex: Only hit in layer:"<<endl;
    mdebug<<(*Iterator1)->ToString()<<endl;

    // We only have one hit:
    vector<int> NBelow(SearchRange, 0);
    vector<int> NAbove(SearchRange, 0);

    int Distance;
    for (Iterator2 = ReseList.begin(); Iterator2 != ReseList.end(); Iterator2++) {
      if ((*Iterator1) == (*Iterator2)) continue;
      // Ignore small energy deposits equivalent to bremsstrahlung
      //if ((*Iterator2)->GetEnergy() < 90) continue;

      Distance = m_Geometry->GetLayerDistance((*Iterator1), (*Iterator2));
      if (Distance > 0 && Distance < int(SearchRange)) NAbove[Distance]++;
      if (Distance < 0 && abs(Distance) < int(SearchRange)) NBelow[abs(Distance)]++;
      massert(Distance != 0); // In this case the algorithm is broken...
      //mdebug<<"Distance "<<(*Iterator1)->GetID()<<" - "<<(*Iterator2)->GetID()<<": "<<Distance<<endl;
    }


    // Under the following conditions we do have a pair:
    
    // Pair starting from top
    MRESE* Vertex = 0;
    int VertexDirection = 0;

    
    // Check for vertex below
    if (NAbove[1] == 0) {
      int StartIndex = 0; // We start when we have 2 hits for the first time
      int StopIndex = 0; // We stop when we skip 2 layers for the first time
      int LayersWithAtLeastTwoHitsBetweenStartAndStop = 0;

      
      for (unsigned int d = 1; d < SearchRange-1; ++d) {
        // We stop when we skip 2 layers for the first time
        if (NBelow[d] == 0 && NBelow[d+1] == 0) break;
        StopIndex = d;
        // Store the index where we have 2 hits for the first time, twice
        if (StartIndex == 0 && NBelow[d] > 1 && NBelow[d+2] > 1) StartIndex = d;
        
        if (StartIndex != 0) {
          if (NBelow[d] >= 2) ++LayersWithAtLeastTwoHitsBetweenStartAndStop;
        }
      }
      
      // Since we can have just a single track at the end, move upward until we have at least two hits in a row
      for (unsigned int d = StopIndex; d > 2; --d) {
        if (NBelow[d-1] >= 2 && NBelow[d-2] >= 2) break;
        StopIndex = d;
      }
      
      mdebug<<"Search vertex ("<<(*Iterator1)->GetPosition().Z()<<"): Above: ";
      for (int i: NAbove) mdebug<<i<<" ";
      mdebug<<endl;
      mdebug<<"Search vertex ("<<(*Iterator1)->GetPosition().Z()<<"): Below: ";
      for (int i: NBelow) mdebug<<i<<" ";
      mdebug<<endl;
     
      mdebug<<"Vertex statistics (max: "<<SearchRange<<"): layers used: "<<StopIndex<<", start of 2+ hits: "<<StartIndex<<"  layers with 2+ hits between start and stop: "<<LayersWithAtLeastTwoHitsBetweenStartAndStop<<" ("<<((StopIndex-StartIndex > 0) ? 100.0*LayersWithAtLeastTwoHitsBetweenStartAndStop/(StopIndex-StartIndex) : 0)<<"%)"<<endl; 
      
      if (LayersWithAtLeastTwoHitsBetweenStartAndStop > 4 && double (LayersWithAtLeastTwoHitsBetweenStartAndStop)/(StopIndex-StartIndex) > 0.5) {  
        Vertex = (*Iterator1);
        VertexDirection = -1;
      }
    }

    if (Vertex != 0) {
      if (List == 0) List = new MRawEventList();
      RE->SetVertex(Vertex);
      RE->SetVertexDirection(VertexDirection);
      MRERawEvent *New = RE->Duplicate();
      RE->SetVertex(0);
      List->AddRawEvent(New);
      mdebug<<"Search vertex: Found vertex: "<<Vertex->GetID()<<endl;
      break; // Only take first right now
    }
  }

  return List;
}


////////////////////////////////////////////////////////////////////////////////


void MERTrack::TrackPairs(MRERawEvent* RE)
{
  // This assumes that a vertex has been found - then try to track the pair

  mdebug<<"Doing Pair tracking..."<<endl;

  if (RE->GetVertex() == 0) {
    Fatal("void MRERawEvent::TrackPairs()",
          "No vertex found!");
  }

  mdebug<<"Vertex: "<<RE->GetVertex()->GetPosition().Z()<<endl;
  
  MRETrack* Electron = new MRETrack();
  MRETrack* Positron = new MRETrack();

  // Now figure out if we have to go up or down:
  int Direction = RE->GetVertexDirection();
  
  // Now create the vertex:
  Electron->AddRESE(RE->GetVertex());
  Electron->SetStartPoint(RE->GetVertex());

  MRESE* Virtual = 
    new MREHit(RE->GetVertex()->GetPosition(), 0, RE->GetVertex()->GetTime(),
               RE->GetVertex()->GetDetector(), 
               RE->GetVertex()->GetPositionResolution(), 
               RE->GetVertex()->GetEnergyResolution(),
               RE->GetVertex()->GetTimeResolution());
  Positron->AddRESE(Virtual);
  Positron->SetStartPoint(Virtual);
	Positron->SetVolumeSequence(new MDVolumeSequence(*(RE->GetVertex()->GetVolumeSequence())));

  bool HaveElectronDirection = false;
  bool HavePositronDirection = false;
  
  // Now we have the vertex
  // Go up/down the track until we do not find hits any more and assign them 
  // to the tracks
	int NTrials = 0;
  MRESEList List;
  do {
    // Search for hits in next layer, and store them in the list:
    List.RemoveAllRESEs();
    List.Compress();

		NTrials = 0;
		while (List.GetNRESEs() == 0 && NTrials < 10) { 
      mdebug<<"Testing direction: "<<Direction<<endl;
			NTrials++;
			for (int r = 0; r < RE->GetNRESEs(); r++) {
				if (IsInTracker(RE->GetRESEAt(r)) == false) continue;

				if (m_Geometry->IsAbove(RE->GetVertex(), RE->GetRESEAt(r), Direction) == true) {
					List.AddRESE(RE->GetRESEAt(r));
				}      
			}
			mdebug<<"Distance: "<<abs(Direction)<<"  Entries: "<<List.GetNRESEs();
			if (List.GetNRESEs() > 0) {
				mdebug<<" z="<<List.GetRESEAt(0)->GetPosition().Z();
			}
			mdebug<<endl;
			(Direction < 0) ? --Direction : ++Direction;
		}

    // Now search for the best combination:
    int BestEl = -1;
    int BestPos = -1;
    double BestScore = 1E10;
    double NewScore = 0;
    
    // If we have only one hit:
    if (List.GetNRESEs() == 0) {
      break;
    } else if (List.GetNRESEs() == 1) {

      if (HaveElectronDirection == false) {
        BestEl = 0;
      } else if (HavePositronDirection == false) {
        BestPos = 0;
      } else {
        mdebug<<"Pair tracking: One hit: e-angle: "
            <<Electron->GetFinalDirection().Angle(List.GetRESEAt(0)->GetPosition() - 
                                                  Electron->GetStopPoint()->GetPosition())*c_Deg
            <<" p-angle: "<<Positron->GetFinalDirection().Angle(List.GetRESEAt(0)->GetPosition() - 
                                                                Positron->GetStopPoint()->GetPosition())*c_Deg<<endl;
        
				if (Electron->GetFinalDirection().Angle(List.GetRESEAt(0)->GetPosition() - Electron->GetStopPoint()->GetPosition()) <
						Positron->GetFinalDirection().Angle(List.GetRESEAt(0)->GetPosition() - Positron->GetStopPoint()->GetPosition())) {
					BestEl = 0;
					BestPos = -1;
				} else {
					BestEl = -1;
					BestPos = 0;
				}
      }
    } else if (List.GetNRESEs() >= 2) {

      if (HaveElectronDirection == false && HavePositronDirection == false) {
        // Three hits are unlikely , so ignore right now:
        mdebug<<"No track defined...."<<endl;
        BestEl = 0;
        BestPos = 1;

      } else if (HaveElectronDirection == true && HavePositronDirection == false) {
        mdebug<<"No p track"<<endl;

        // Search the straightest continuation for the electron:
        for (int r = 0; r < List.GetNRESEs(); r++) {
          NewScore = Electron->GetFinalDirection().Angle(List.GetRESEAt(r)->GetPosition() - Electron->GetStopPoint()->GetPosition());
          if (NewScore < BestScore) {
            BestScore = NewScore;
            BestEl = r;
          }
        }

        // The other position is for the positron
        for (int r = 0; r < List.GetNRESEs(); r++) {
          if (r != BestEl) {
            BestPos = r;
          }
        }

      } else if (HaveElectronDirection == false && HavePositronDirection == true) {
        mdebug<<"No e track"<<endl;

        // Search the straightest continuation for the positron:
        for (int r = 0; r < List.GetNRESEs(); r++) {
          NewScore = Positron->GetFinalDirection().Angle(List.GetRESEAt(r)->GetPosition() - 
                                                         Positron->GetStopPoint()->GetPosition());
          if (NewScore < BestScore) {
            BestScore = NewScore;
            BestEl = r;
          }
        }

        // The other position is for the positron
        for (int r = 0; r < List.GetNRESEs(); r++) {
          if (r != BestPos) {
            BestEl = r;
          }
        }

      } else {
        mdebug<<"All trackes defined...."<<endl;
        // Loop and search best combination:
        for (int r = 0; r < List.GetNRESEs(); r++) {
          for (int s = r+1; s < List.GetNRESEs(); s++) {
            // Direction One:
            NewScore = 
              Electron->GetFinalDirection().Angle(List.GetRESEAt(r)->GetPosition() - Electron->GetStopPoint()->GetPosition())*
              Electron->GetFinalDirection().Angle(List.GetRESEAt(r)->GetPosition() - Electron->GetStopPoint()->GetPosition()) + 
              Positron->GetFinalDirection().Angle(List.GetRESEAt(s)->GetPosition() - Positron->GetStopPoint()->GetPosition())*
              Positron->GetFinalDirection().Angle(List.GetRESEAt(s)->GetPosition() - Positron->GetStopPoint()->GetPosition());
            if (NewScore < BestScore) {
              BestScore = NewScore;
              BestEl = r;
              BestPos = s;
            }
            
            // Direction Two:
            NewScore = 
              Electron->GetFinalDirection().Angle(List.GetRESEAt(s)->GetPosition() - Electron->GetStopPoint()->GetPosition())*
              Electron->GetFinalDirection().Angle(List.GetRESEAt(s)->GetPosition() - Electron->GetStopPoint()->GetPosition()) + 
              Positron->GetFinalDirection().Angle(List.GetRESEAt(r)->GetPosition() - Positron->GetStopPoint()->GetPosition())*
              Positron->GetFinalDirection().Angle(List.GetRESEAt(r)->GetPosition() - Positron->GetStopPoint()->GetPosition());
            
            if (NewScore < BestScore) {
              BestScore = NewScore;
              BestEl = s;
              BestPos = r;
            }
          } // end loop s (second RESE)
        } // end loop r (first RESE)
      }
    } // end if two hits

    // Now add the new Stop points:
    if (BestEl != -1) {
      mdebug<<"Setting electron stop point: "<<List.GetRESEAt(BestEl)->GetID()<<endl;
      Electron->SetStopPoint(List.GetRESEAt(BestEl));
      RE->RemoveRESE(List.GetRESEAt(BestEl));
      HaveElectronDirection = true;
    }
    if (BestPos != -1) {
      mdebug<<"Seting pos stop point: "<<List.GetRESEAt(BestPos)->GetID()<<endl;
      Positron->SetStopPoint(List.GetRESEAt(BestPos));
      RE->RemoveRESE(List.GetRESEAt(BestPos));
      HavePositronDirection = true;
    }
    RE->CompressRESEs();

  } while (true);

  
  // For the electron and positron track walk down the track and determine the straightness from the 5 straigtest segments
  // Hits with deviate more than X% from this straightness are tossed out of the track
  EliminatePairDeviations(RE, Electron);
  EliminatePairDeviations(RE, Positron);
  
  // Clean up
  RE->RemoveRESE(RE->GetVertex());
  RE->CompressRESEs();

  // We now might have a new start point, thus make sure we fix the vertex
  MRESE* NewStartElectron = Electron->GetStartPoint();
  MRESE* NewStartPositron = Positron->GetStartPoint();
  
  if (NewStartElectron->GetPosition().GetZ() < NewStartPositron->GetPosition().GetZ()) {
    MRESE* NewStart = NewStartPositron->Duplicate();
    NewStartElectron->AddLink(NewStart);
    NewStart->AddLink(NewStartElectron);
    Electron->AddRESE(NewStart);
    Electron->SetStartPoint(NewStart);
    RE->SetVertex(NewStart);
  } else if (NewStartElectron->GetPosition().GetZ() > NewStartPositron->GetPosition().GetZ()) {
    MRESE* NewStart = NewStartElectron->Duplicate();
    NewStartPositron->AddLink(NewStart);
    NewStart->AddLink(NewStartPositron);
    Positron->AddRESE(NewStart);
    Positron->SetStartPoint(NewStart);
    RE->SetVertex(NewStart);    
  } else {
    RE->SetVertex(NewStartElectron);   
  }
  
  
  // Now add (not append) the remaining hits to the nearest track

  for (int r = 0; r < RE->GetNRESEs(); r++) {
    if (Electron->ComputeMinDistance(RE->GetRESEAt(r)) < Positron->ComputeMinDistance(RE->GetRESEAt(r))) {
      Electron->AddRESE(RE->GetRESEAt(r));
    } else {
      Positron->AddRESE(RE->GetRESEAt(r));
    }
    RE->RemoveRESE(RE->GetRESEAt(r));
  }
  
  RE->CompressRESEs();


  // Finally add the new tracks to the event:
  if (Electron->GetNRESEs() > 2 && Positron->GetNRESEs() > 2) {
    RE->AddRESE(Electron);
    RE->AddRESE(Positron);

    RE->SetElectronTrack(Electron);
    RE->SetPositronTrack(Positron);

    mdebug<<"Electron: "<<Electron->ToString()<<endl;
    mdebug<<"Positron: "<<Positron->ToString()<<endl;


    EvaluateTracks(RE);
  } else {
    for (int r = 0; r < Electron->GetNRESEs(); ++r) {
      RE->AddRESE(Electron->GetRESEAt(r));
    }
    for (int r = 0; r < Positron->GetNRESEs(); ++r) {
      RE->AddRESE(Positron->GetRESEAt(r));
    }
    delete Electron;
    delete Positron;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MERTrack::EliminatePairDeviations(MRERawEvent* RE, MRETrack* Track)
{
  const int NEvaluations = 4;
  const double FactorAboveMaxDeviation = 1.3;
  const double MinDeviation = 8.0*c_Rad;
  
  vector<double> Deviations;
  
  MRESE* Start = Track->GetStartPoint();
  if (Start == nullptr) return;
  if (Start->GetNLinks() != 1) return;
  MRESE* Middle = Start->GetLinkAt(0);
  if (Middle->GetNLinks() != 2) return;
  MRESE* End = Middle->GetOtherLink(Start);
  while (true) {
    Deviations.push_back((Middle->GetPosition() - Start->GetPosition()).Angle(End->GetPosition() - Middle->GetPosition()));
    
    Start = Middle;
    Middle = End;
    if (Middle->GetNLinks() != 2) break;
    End = Middle->GetOtherLink(Start);
  }
  
  // If we have less or equal than "NEvaluations" values, we do nothing
  if (Deviations.size() <= NEvaluations) return;
  
  // Sort increasing
  sort(Deviations.begin(), Deviations.end(), std::less<double>());
  
  // Create the average of the first NEvaluations
  double AverageDeviation = 0.0;
  for (unsigned int i = 0; i < NEvaluations; ++i) {
    AverageDeviation += Deviations[i];
  }
  AverageDeviation /= NEvaluations;
  
  // Find the largest deviation
  double MaxDeviation = 0.0;
  for (unsigned int i = 0; i < NEvaluations; ++i) {
    if (Deviations[i] > MaxDeviation) {
      MaxDeviation = Deviations[i];
    }
  }
  MaxDeviation *= FactorAboveMaxDeviation;
  if (MaxDeviation < MinDeviation) MaxDeviation = MinDeviation;
  
  mdebug<<"Maximum deviation: "<<MaxDeviation*c_Deg<<" degrees"<<endl;
  
  // Find the first segment which is below the average deviation:
  Start = Track->GetStartPoint();
  Middle = Start->GetLinkAt(0);
  End = Middle->GetOtherLink(Start);
  while (true) {
    double Deviation = (Middle->GetPosition() - Start->GetPosition()).Angle(End->GetPosition() - Middle->GetPosition());
    
    if (Deviation <= AverageDeviation) break;
    
    Start = Middle;
    Middle = End;
    if (Middle->GetNLinks() != 2) break;
    End = Middle->GetOtherLink(Start);
  }

  // First fix the track upstream:
  MRESE* BestNewStartPoint = Start;
  while (Start->GetNLinks() == 2) { // while this is not the real start
    End = Middle;
    Middle = Start;
    Start = Middle->GetOtherLink(End);
    MRESE* StartOldOtherLink = Middle;
    
    double Deviation = (Middle->GetPosition() - Start->GetPosition()).Angle(End->GetPosition() - Middle->GetPosition());
    mdebug<<"Deviation: "<<Start->GetID()<<"->"<<Middle->GetID()<<"->"<<End->GetID()<<": "<<Deviation*c_Deg<<" deg"<<endl; 

    while (Deviation > MaxDeviation && Start->GetNLinks() == 2) {
      MRESE* ThrowOut = Start;
      Start = Start->GetOtherLink(StartOldOtherLink);
      StartOldOtherLink = ThrowOut;
      
      // Remove the RESE
      Track->RemoveRESE(ThrowOut);
      mdebug<<"Removing RESE: "<<ThrowOut->GetID()<<" from track due to a deviation of "<<Deviation*c_Deg<<" deg (max allowed: "<<MaxDeviation*c_Deg<<" deg)"<<endl;
      RE->AddRESE(ThrowOut);
        
      // Relink the others
      ThrowOut->RemoveAllLinks();
      Start->RemoveLink(ThrowOut);
      Middle->RemoveLink(ThrowOut);
      Start->AddLink(Middle);
      Middle->AddLink(Start);
      
      Deviation = (Middle->GetPosition() - Start->GetPosition()).Angle(End->GetPosition() - Middle->GetPosition());
      mdebug<<"Deviation: "<<Start->GetID()<<"->"<<Middle->GetID()<<"->"<<End->GetID()<<": "<<Deviation*c_Deg<<" deg "<<endl; 
    }

    if (Deviation < MaxDeviation) {
      BestNewStartPoint = Start;
    } else {
      // We are at the old start point, thus clean up:
      Start->GetLinkAt(0)->RemoveLink(Start);
      Start->RemoveAllLinks();
      Track->RemoveRESE(Start);
    }
  }
  
  Track->SetStartPoint(BestNewStartPoint);
  Track->CompressRESEs();
}


////////////////////////////////////////////////////////////////////////////////


void MERTrack::CheckForMips(MRERawEvent* RE)
{
  // Check if the events structure can be created by a massive interacting
  // particle like myons:
  //
  // The typical pattern of a MIP is:
  // We have a straight track with serveral hits which, do not belong to the
  // original MIP, but to its secondaries, bremsstrahlung etc.
  
  mdebug<<"Checking for Mips..."<<endl;

  const int MinElements = 4;
  const double MinStraightness = 0.05;

  // a. Check all hit combinations in D1 for their straightness
  vector<MRETrack*> TrackArray; // Array of Tracks

  vector<MRESE*> D1Hits;
  for (int i = 0; i < RE->GetNRESEs(); ++i) {
    if (IsInTracker(RE->GetRESEAt(i)) == true) {
      D1Hits.push_back(RE->GetRESEAt(i));
    }
  }

  // Sort the array for decreasing z: 
  MRESE* Temp = 0;
  unsigned int HighestPos = 0;
  double HighestValue = -1;
  for (unsigned int i = 0; i < D1Hits.size(); ++i) {
    HighestPos = i;
    HighestValue = D1Hits[i]->GetPosition().Z();
    for (unsigned int j = i; j < D1Hits.size(); ++j) {
      if (D1Hits[j]->GetPosition().Z() > HighestValue) {
        HighestValue = D1Hits[j]->GetPosition().Z();
        HighestPos = j;
      }
    }
    // Change Position
    if (HighestPos != i) {
      Temp = D1Hits[i];
      D1Hits[i] = D1Hits[HighestPos];
      D1Hits[HighestPos] = Temp;
    }
  }


  // We look for straight tripple combinations:
  unsigned int max = D1Hits.size();
  MRESE* iRESE;
  MRESE* jRESE;
  MRESE* kRESE;
  MRETrack* Track = 0;
  MVector Direction1, Direction2;
  for (unsigned int i = 0; i < max; ++i) {
    if (D1Hits[i] == 0) continue;
    iRESE = D1Hits[i];
    //mdebug<<"i:"<<iRESE->ToString()<<endl;

    for (unsigned int j = i+1; j < max; ++j) {
      if (D1Hits[j] == 0) continue;
      jRESE = D1Hits[j];
      //mdebug<<"j:"<<jRESE->ToString()<<endl;
      Direction1 = jRESE->GetPosition() - iRESE->GetPosition();
      
      Track = 0; // No memory leak!

      for (unsigned int k = j+1; k < max; ++k) {
        if (D1Hits[k] == 0) continue;
        kRESE = D1Hits[k];
        //mdebug<<"k:"<<kRESE->ToString()<<endl;
        Direction2 = kRESE->GetPosition() - jRESE->GetPosition();
        
        // Check the straightness:
        //mdebug<<"Angle: "<<Direction1.Angle(Direction2)<<endl;
        if (Direction1.Angle(Direction2) < MinStraightness) {
          //mdebug<<"Found some straight track!!!!"<<endl;
          if (Track == 0) {
            //mdebug<<"Creating new track!!!!"<<endl;
            Track = new MRETrack();
            TrackArray.push_back(Track);
            Track->AddRESE(iRESE);
            Track->AddRESE(jRESE);
            Track->AddRESE(kRESE);
            D1Hits[i] = 0;
            D1Hits[j] = 0;
            D1Hits[k] = 0;
          } else {
            Track->AddRESE(kRESE);
            D1Hits[k] = 0;
          }            
          
          Direction1 = Direction2;
        }
      } // end k

      if (Track != 0) break;

    } // end j
  } // end i

  // We need at least one track with at least MinElements hits:
  // For now: Search for the largest track: 
  HighestPos = 0;
  HighestValue = -1;
  for (unsigned int i = 0; i < TrackArray.size(); ++i) {
    if (TrackArray[i]->GetNRESEs() > MinElements) {
      HighestValue = TrackArray[i]->GetNRESEs();
      HighestPos = i;
    }
  }


  // Now the result:

  // a. Definitely a muon
  if (TrackArray.size() == 1) {
    mdebug<<"==> This was fucking easy! Found a muon!"<<endl;

    // a. Integrate the track into this raw event:
    TrackArray[HighestPos]->LinkSequential();
    for (unsigned int i = 0; i < (unsigned int) TrackArray[HighestPos]->GetNRESEs(); ++i) {
      RE->RemoveRESE(TrackArray[HighestPos]->GetRESEAt(i));
    }
    EvaluateTrack(TrackArray[HighestPos]);
    RE->AddRESE(TrackArray[HighestPos]);
    RE->CompressRESEs();
    
    // b. Store the new event:
    MMuonEvent* Muon = new MMuonEvent(); // is later stored as m_Event!
    Muon->SetEnergy(RE->GetEnergy());
    Muon->SetDirection(TrackArray[HighestPos]->GetAverageDirection());
    Muon->SetCenterOfGravity(TrackArray[HighestPos]->GetCenterOfGravity());

    // c. Remove the track from the array
    for (unsigned int i = 0; i < TrackArray.size(); ++i) {
      if (i != HighestPos) delete TrackArray[i];
    }

    // d. This is a good event...
    RE->SetPhysicalEvent(Muon);
    RE->SetEventType(MRERawEvent::c_MipEvent);
    RE->SetGoodEvent(true);
  } 
  // b. A shower
  else if (TrackArray.size() > 1) {
    mdebug<<"==> Found a shower!"<<endl;
    RE->SetEventType(MRERawEvent::c_ShowerEvent);

    // b. Integrate all tracks into this raw event:
    for (unsigned int i = 0; i < TrackArray.size(); ++i) {
      TrackArray[i]->LinkSequential();
      for (unsigned int j = 0; j < (unsigned int) TrackArray[i]->GetNRESEs(); ++j) {
        RE->RemoveRESE(TrackArray[i]->GetRESEAt(j));
      }
      EvaluateTrack(TrackArray[i]);
      RE->CompressRESEs();
      RE->AddRESE(TrackArray[i]);
      RE->SetGoodEvent(true);
    }
  }

  // return;
}



////////////////////////////////////////////////////////////////////////////////


MRawEventList* MERTrack::TrackComptons(MRERawEvent* RE)
{
  mdebug<<"Track Compton events..."<<endl;

  MRESE* RESE = 0;

  mdebug<<"Create list:"<<endl;

  // Create a list consisting of the RESEs and their distance:
  int Dist = 0;
  bool Added = false;
  vector<MRESE*> MasterRESEs;
  vector<vector<MRESEDist> > DistList;
  vector<MRESEDist>::iterator DistListIter;

  int i_max = RE->GetNRESEs();
  for (int i = 0; i < i_max; i++) {
    RESE = RE->GetRESEAt(i);

    // if it's not in D1 reject it
    if (IsInTracker(RESE) == false) {
      continue; 
    }

    Added = false;
    for (unsigned int m = 0; m < MasterRESEs.size(); ++m) {
      Dist = m_Geometry->GetLayerDistance(MasterRESEs[m], RESE);
      if (Dist == MGeometryRevan::c_DifferentTracker) {
        continue;
      }
      MRESEDist RD(RESE, Dist);
      DistList[m].push_back(RD);
      Added = true;
    }
    if (Added == false) {
      MasterRESEs.push_back(RESE);
      DistList.resize(MasterRESEs.size());
      MRESEDist RD(RESE, 0);
      DistList[DistList.size()-1].push_back(RD);        
    }
  }

  // Sort the list:
  for (unsigned int m = 0; m < MasterRESEs.size(); ++m) {
    sort(DistList[m].begin(), DistList[m].end(), MRESEDistCompare);
  }

  // Create the layer occupation:
  vector<map<int, int> > OccupationList;
  OccupationList.resize(DistList.size());

  for (unsigned int m = 0; m < DistList.size(); ++m) {
    for (DistListIter = DistList[m].begin();
         DistListIter != DistList[m].end(); DistListIter++) {
      OccupationList[m][(*DistListIter).Distance]++;
    }
  }

  // Dump the list:
  for (unsigned int m = 0; m < DistList.size(); ++m) {
    mdebug<<"MasterRESE: "<<MasterRESEs[m]->GetID()<<endl;
    for (DistListIter = DistList[m].begin();
         DistListIter != DistList[m].end(); DistListIter++) {
      mdebug<<"  ID "<<(*DistListIter).RESE->GetID()<<": Distance="
            <<(*DistListIter).Distance<<" ("
            <<OccupationList[m][(*DistListIter).Distance]<<")"<<endl;
    }
  }

  // Augment everything non-ambiguous to linked RESEs:
  unsigned int NAmbiguities = 0;
  bool IsPureAmbiguity = true;
  const int NoDist = numeric_limits<int>::max();
  int LastDist = NoDist;
  MRESE* LastRESE = 0;
  for (unsigned int m = 0; m < DistList.size(); ++m) {
    NAmbiguities += DistList[m].size();
    for (DistListIter = DistList[m].begin();
         DistListIter != DistList[m].end(); DistListIter++) {
      if (OccupationList[m][(*DistListIter).Distance] != 1) {
        LastRESE = 0;
        LastDist = NoDist;
        continue;
      } else {
        RESE = (*DistListIter).RESE;
        if (LastRESE != 0) {
          if ((unsigned int) abs(LastDist - (*DistListIter).Distance) <= m_ComptonMaxLayerJump) {
            RESE->AddLink(LastRESE);
            LastRESE->AddLink(RESE);
            LastDist = (*DistListIter).Distance;
            NAmbiguities--;
            IsPureAmbiguity = false;
          }
        }
        LastRESE = RESE;
        LastDist = (*DistListIter).Distance;     
      }
    }
  }

  MRawEventList* List = new MRawEventList(); 

  if (IsPureAmbiguity == true && m_RejectPureAmbiguities == true) {
    mout<<"Tracking: Rejecting purely ambiguous event!"<<endl;
    List->AddRawEvent(RE->Duplicate());
    return List;
  }

  if (NAmbiguities <= m_MaxNAmbiguities) {
    // Find ambiguous sequences 
    mdebug<<"Testing ambiguous sequences"<<endl;

    List->AddRawEvent(RE->Duplicate());
    
    MRESE* RESEmain = 0;
    MRESE* RESEp1 = 0;
    MRESE* RESEp2 = 0;

    vector<MRESEDist> Partners;
    vector<MRESEDist>::iterator SecDistListIter;
    vector<MRESEDist>::iterator TerDistListIter;
    
    int NNewREs;
    MRERawEvent* NewRE = 0;
    MRERawEvent* NewREdup = 0;
    
    // Find a list of all possible combinations:
    for (unsigned int m = 0; m < DistList.size(); ++m) {
      for (DistListIter = DistList[m].begin();
           DistListIter != DistList[m].end(); DistListIter++) {
        
        RESEmain = (*DistListIter).RESE;
        
        if (RESEmain->GetNLinks() >= 2) {
          continue;
        }
        
        // Find all possible partners:
        int Diff = 0;
        Partners.clear();
        for (SecDistListIter = DistListIter+1;
             SecDistListIter != DistList[m].end(); SecDistListIter++) {
          if ((*SecDistListIter).Distance == (*DistListIter).Distance) {
            continue;
          }
          if (Diff == 0) {
            Diff = (*SecDistListIter).Distance - (*DistListIter).Distance;
          }
          // Only use the nearest neighbor:
          if (Diff != (*SecDistListIter).Distance - (*DistListIter).Distance ||
              (unsigned int) abs(Diff) > m_ComptonMaxLayerJump) {
            break;
          }
          Partners.push_back((*SecDistListIter));
        }
        
        mdebug<<"New partners for "<<RESEmain->GetID()<<": ";
        for (SecDistListIter = Partners.begin();
             SecDistListIter != Partners.end(); SecDistListIter++) {
          mdebug<<(*SecDistListIter).RESE->GetID()<<" ";
        }
        mdebug<<endl;
        

        // Now generate the links:
        NNewREs = List->GetNRawEvents();
        for (int t = 0; t < NNewREs; ++t) {
          NewRE = List->GetRawEventAt(t);
                    
          if (NewRE->GetRESE(RESEmain->GetID())->GetNLinks() == 2) {
            continue; 
          }
          
          // Single partners
          mdebug<<"Searching single partners for "<<RESEmain->GetID()<<endl;
          for (SecDistListIter = Partners.begin();
               SecDistListIter != Partners.end(); SecDistListIter++) {
            RESEp1 = (*SecDistListIter).RESE;
            
            if (NewRE->GetRESE(RESEmain->GetID())->IsLink(NewRE->GetRESE(RESEp1->GetID())) == true) {
              continue;
            }
            
            if (NewRE->GetRESE(RESEp1->GetID())->GetNLinks() <= 1) {
              mdebug<<"Linking: "<<RESEmain->GetID()<<" with "<<RESEp1->GetID()<<endl;
              NewREdup = NewRE->Duplicate();
              NewREdup->GetRESE(RESEmain->GetID())->
                AddLink(NewREdup->GetRESE(RESEp1->GetID()));
              NewREdup->GetRESE(RESEp1->GetID())->
                AddLink(NewREdup->GetRESE(RESEmain->GetID()));
              List->AddRawEvent(NewREdup);
            }
          } // end single partner loop

          if (NewRE->GetRESE(RESEmain->GetID())->GetNLinks() > 0) {
            continue; 
          }

          // Double partners:
          mdebug<<"Searching double partners for "<<RESEmain->GetID()<<endl;
          for (SecDistListIter = Partners.begin();
               SecDistListIter != Partners.end(); SecDistListIter++) {

            RESEp1 = (*SecDistListIter).RESE;
            if (NewRE->GetRESE(RESEp1->GetID())->GetNLinks() > 1) {
              continue;
            }
            if (NewRE->GetRESE(RESEmain->GetID())
                ->IsLink(NewRE->GetRESE(RESEp1->GetID())) == true) {
              continue;
            }
            for (TerDistListIter = SecDistListIter+1;
                 TerDistListIter != Partners.end(); TerDistListIter++) {
              RESEp2 = (*TerDistListIter).RESE;
              if (NewRE->GetRESE(RESEp2->GetID())->GetNLinks() > 1) {
                continue;
              }
              if (NewRE->GetRESE(RESEmain->GetID())->IsLink(NewRE->GetRESE(RESEp2->GetID())) == true) {
                continue;
              }
              mdebug<<"Double link: "<<RESEmain->GetID()<<" with "<<RESEp1->GetID()<<" and "<<RESEp2->GetID()<<endl;
              NewREdup = NewRE->Duplicate();
              NewREdup->GetRESE(RESEmain->GetID())->
                AddLink(NewREdup->GetRESE(RESEp1->GetID()));
              NewREdup->GetRESE(RESEp1->GetID())->
                AddLink(NewREdup->GetRESE(RESEmain->GetID()));
              NewREdup->GetRESE(RESEmain->GetID())->
                AddLink(NewREdup->GetRESE(RESEp2->GetID()));
              NewREdup->GetRESE(RESEp2->GetID())->
                AddLink(NewREdup->GetRESE(RESEmain->GetID()));
              //mdebug<<"NewREdup: "<<endl<<NewREdup->ToString()<<endl;
              List->AddRawEvent(NewREdup);
            }
          } // end double partner loop

        } // end loop over List      
      }
    }
  
    // Create tracks in the list:
    mdebug<<"Newly tracked events: "<<List->GetNRawEvents()<<endl;
    for (int i = 0; i < List->GetNRawEvents(); i++) {
      mdebug<<List->GetRawEventAt(i)->ToString()<<endl;;
    }   
    
    // Create tracks in the list:
    for (int i = 0; i < List->GetNRawEvents(); i++) {
      if (List->GetRawEventAt(i)->CreateTracks() == false) {
        List->DeleteRawEvent(List->GetRawEventAt(i));
        i--;
        continue;
      }
    }

  } else {
    // High energy Compton tracking - very similar to that of pairs:
    // We do NOT generate multiple raw events here!

    mdebug<<"Doing high energy electron tracking..."<<endl;

    // Only continue if we already have a track segment, from which we can start:

    // Upgrade links to tracks... 
    if (RE->CreateTracks() == false) {
      mout<<"No crystallization point! --> Add rejection for this..."<<endl;
      for (int r = 0; r < RE->GetNRESEs(); ++r) {
        RE->GetRESEAt(r)->RemoveAllLinks();
      }
      return 0;
    }

    bool ThereWasAChange = true;
    list<MRESETrackCont> Neighbors; 
    do {
      Neighbors.clear();
      // For each track search all possible continuation (within layer jump)
      for (int r = 0; r < RE->GetNRESEs(); ++r) {
        if (RE->GetRESEAt(r)->GetType() == MRESE::c_Track) {
          MRETrack* Track = dynamic_cast<MRETrack*>(RE->GetRESEAt(r));
          for (int e = 0; e < Track->GetNEndPoints(); ++e) {
            for (int c = 0; c < RE->GetNRESEs(); ++c) {
              if (RE->GetRESEAt(r)->GetID() == RE->GetRESEAt(c)->GetID()) continue;
              if (RE->GetRESEAt(c)->GetType() == MRESE::c_Cluster ||
                  RE->GetRESEAt(c)->GetType() == MRESE::c_Hit) {
                int Dist = m_Geometry->GetLayerDistance(Track->GetEndPointAt(e), RE->GetRESEAt(c));
                if (Dist == MGeometryRevan::c_DifferentTracker) {
                  continue;
                }
                if ((unsigned int) abs(Dist) <= m_ComptonMaxLayerJump && abs(Dist) > 0) {
                  double Angle = Track->GetDirectionOfEndPoint(Track->GetEndPointAt(e)).
                    Angle(RE->GetRESEAt(c)->GetPosition()-Track->GetEndPointAt(e)->GetPosition());
                  MRESETrackCont C(Track, e, RE->GetRESEAt(c), -1, Angle);
                  Neighbors.push_back(C);
                }
              } else if (RE->GetRESEAt(c)->GetType() == MRESE::c_Track) {
                MRETrack* Track2 = dynamic_cast<MRETrack*>(RE->GetRESEAt(c));
                for (int e2 = 0; e2 < Track2->GetNEndPoints(); ++e2) {
                  int Dist = m_Geometry->GetLayerDistance(Track->GetEndPointAt(e), Track2->GetEndPointAt(e2));
                  if (Dist == MGeometryRevan::c_DifferentTracker) {
                    continue;
                  }
                  if ((unsigned int) abs(Dist) <= m_ComptonMaxLayerJump && abs(Dist) > 0) {
                    double Angle = Track->GetDirectionOfEndPoint(Track->GetEndPointAt(e)).
                      Angle(Track2->GetEndPointAt(e2)->GetPosition()-Track->GetEndPointAt(e)->GetPosition());
                    MRESETrackCont C(Track, e, RE->GetRESEAt(c), e2, Angle);
                    Neighbors.push_back(C);
                  }
                }
              }
            }
          }
        }
      }
      
      if (Neighbors.size() == 0) {
        break;
      }

      // Only append the best one!
      Neighbors.sort(MRESETrackContCompare);
      
      if (Neighbors.front().m_EndPoint2 < 0) {
        Neighbors.front().m_Track->AddEndPoint(Neighbors.front().m_Next, 
                                               Neighbors.front().m_Track->GetEndPointAt(Neighbors.front().m_EndPoint1));
        RE->RemoveRESE(Neighbors.front().m_Next);
      } else {
        Neighbors.front().m_Track->AddEndPoint(dynamic_cast<MRETrack*>(Neighbors.front().m_Next), 
                                               dynamic_cast<MRETrack*>(Neighbors.front().m_Next)->GetEndPointAt(Neighbors.front().m_EndPoint2),
                                               Neighbors.front().m_Track->GetEndPointAt(Neighbors.front().m_EndPoint1));        
        RE->RemoveRESE(Neighbors.front().m_Next);
        delete Neighbors.front().m_Next;
      }
      RE->CompressRESEs();

      // Search for new non ambiguous sequences:
      
    } while (ThereWasAChange == true);

    // Add the only raw event...
    List->AddRawEvent(RE->Duplicate());
  }

  for (int r = 0; r < RE->GetNRESEs(); ++r) {
    RE->GetRESEAt(r)->RemoveAllLinks();
  }


  // Let's do some sanity checks:
  
  // Append straight (long) tracks:
  

  return List;
}


////////////////////////////////////////////////////////////////////////////////


bool MERTrack::PostAnalysis()
{
  /*
  mout<<"Timings of tracking:"<<endl;
  mout<<"Time Pairs:                 "<<m_TimePairs<<endl;
  mout<<"Time Mips:                  "<<m_TimeMips<<endl;
  mout<<"Time Comptons - sequence:   "<<m_TimeComptonSequences<<endl;
  mout<<"Time Comptons - directions: "<<m_TimeComptonDirections<<endl;
  mout<<endl;
  */
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MERTrack::EvaluateTracks(MRERawEvent* RE)
{
  if (RE->GetVertex() != 0) {
    return EvaluatePairs(RE);
  }

  double ScoreTracksTotal = MRERawEvent::c_NoQualityFactor;
  double ScoreTracksGeometry = MRERawEvent::c_NoQualityFactor;
  double ScoreTracksHitSequence = MRERawEvent::c_NoQualityFactor;


  // (a) the geometry of the track:
  //     Score = 1 / (# tracks + # remaining hits in D1)

  int NTracks = 0, NRemainingHits = 0;
  
  MRESE* RESE = 0;
  for (int i = 0; i < RE->GetNRESEs(); i++) {
    RESE = RE->GetRESEAt(i);
    if (IsInTracker(RESE) == true) {
      //mdebug<<"Detecor"<<RESE->GetDetector()<<endl;
      if (RESE->GetType() == MRESE::c_Track) {
        NTracks++;
      } else if (RESE->GetType() == MRESE::c_Hit || 
                 RESE->GetType() == MRESE::c_Cluster) {
        NRemainingHits++;
      } else {
        NTracks = 0;
        NRemainingHits = 0;
        break;
      }
    }
  }
  ScoreTracksGeometry = NTracks + NRemainingHits;
  if (ScoreTracksGeometry != 0) {
    ScoreTracksGeometry = 1.0/ScoreTracksGeometry;
  }
  ScoreTracksGeometry *= 2;


  // (b) Evaluate the tracks (and build an average value):
  ScoreTracksHitSequence = 0;
  for (int i = 0; i < RE->GetNRESEs(); i++) {
    RESE = RE->GetRESEAt(i);
    if (RESE->GetType() == MRESE::c_Track) {
      MRETrack* Track = (MRETrack*) RESE;
      EvaluateTrack(Track);
      ScoreTracksHitSequence += Track->GetQualityFactor();
    }
  }
  if (NTracks != 0) {
    ScoreTracksHitSequence /= NTracks;
  }

  ScoreTracksTotal = ScoreTracksGeometry + ScoreTracksHitSequence;


  RE->SetTrackQualityFactor(ScoreTracksTotal);
  RE->SetPairQualityFactor(MRERawEvent::c_NoQualityFactor);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MERTrack::EvaluateTrack(MRETrack* Track)
{
  double QF = 0; 
  QF = Track->CalculatePearsonCorrelation();
  Track->SetQualityFactor(QF);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MERTrack::EvaluatePairs(MRERawEvent* RE)
{
  double ScorePair = MRERawEvent::c_NoQualityFactor;

  //mdebug<<"Evaluating..."<<endl;
  if (RE->GetElectronTrack() != 0 && RE->GetPositronTrack() != 0) {
    double Score = 0;
    Score = ((MRETrack*) RE->GetElectronTrack())->CalculatePearsonCorrelation();
    ((MRETrack*) RE->GetElectronTrack())->SetQualityFactor(Score);
    Score = ((MRETrack*) RE->GetPositronTrack())->CalculatePearsonCorrelation();
    ((MRETrack*) RE->GetPositronTrack())->SetQualityFactor(Score);

    ScorePair = ((MRETrack*) RE->GetElectronTrack())->GetQualityFactor() + 
      ((MRETrack *) RE->GetPositronTrack())->GetQualityFactor();
    mdebug<<"Best score = "<<ScorePair<<endl;
  }

  if (ScorePair < -3.3) {
    mdebug<<"Bad pair found..."<<endl;
    RE->SetGoodEvent(false);
  } else {
    mdebug<<"Pair found..."<<endl;
    RE->SetEventType(MRERawEvent::c_PairEvent);
    RE->SetGoodEvent(true);
  }

  RE->SetPairQualityFactor(ScorePair);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MERTrack::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:

  ostringstream out;

  if (CoreOnly == false) {
    out<<"# Tracking - FoM options:"<<endl;
    out<<"# "<<endl;
  }
  out<<"# Search MIPs:                  "<<m_SearchMIPs<<endl;
  out<<"# Search Pairs:                 "<<m_SearchPairs<<endl;
  out<<"# Search Comptons:              "<<m_SearchComptons<<endl;
  out<<"# Max. number of ambiguities:   "<<m_MaxNAmbiguities<<endl;
  out<<"# Max. layer jump Compton:      "<<m_ComptonMaxLayerJump<<endl;
  out<<"# Number of sequences to keep:  "<<m_NSequencesToKeep<<endl;
  out<<"# Reject pure ambiguities:      "<<m_RejectPureAmbiguities<<endl;
  if (CoreOnly == false) {
    out<<"# "<<endl;
  }

  return out.str().c_str();
}



// MERTrack.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
