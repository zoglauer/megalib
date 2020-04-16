/*
 * MRERawEvent.cxx
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
// MRERawEvent.cxx
//
//
// In zeroth order a raw event is a collection of the hits which occurred in 
// the detector. This collection is stored in event tree.
//
// During the analysis, tracks, clusters and single hits are found. So these
// objects replace the hits (but these objects still contain them!)
//
// Moreover this class contains a lot of flags indicating the quality of the
// underlying event, like ...
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDDetector.h"

// Standard libs:
#include <limits>
#include <iostream>
#include <vector>
#include <sstream>
using namespace std;

// ROOT libs:

// MEGALib libs:
#include "MAssert.h"
#include "MRERawEvent.h"
#include "MRETrack.h"
#include "MRECluster.h"
#include "MREHit.h"
#include "MRESE.h"
#include "MRESEIterator.h"
#include "MRawEventIncarnations.h"
#include "MPhysicalEvent.h"
#include "MComptonEvent.h"
#include "MMuonEvent.h"
#include "MPairEvent.h"
#include "MPhotoEvent.h"
#include "MPhysicalEvent.h"
#include "MUnidentifiableEvent.h"
#include "MStreams.h"
#include "MREAM.h"
#include "MREAMDriftChamberEnergy.h"
#include "MREAMGuardRingHit.h"
#include "MREAMDirectional.h"
#include "MREAMStartInformation.h"
#include "MDDetector.h"
#include "MDDriftChamber.h"
#include "MDStrip2D.h"
#include "MDVoxel3D.h"
#include "MDGuardRing.h"


#ifdef ___CLING___
ClassImp(MRERawEvent)
#endif


////////////////////////////////////////////////////////////////////////////////


const int MRERawEvent::c_UnknownEvent = MPhysicalEvent::c_Unknown;
const int MRERawEvent::c_ComptonEvent = MPhysicalEvent::c_Compton;
const int MRERawEvent::c_PairEvent    = MPhysicalEvent::c_Pair;
const int MRERawEvent::c_MipEvent     = MPhysicalEvent::c_Muon;
const int MRERawEvent::c_ShowerEvent  = MPhysicalEvent::c_Shower;
const int MRERawEvent::c_PhotoEvent   = MPhysicalEvent::c_Photo;

const double MRERawEvent::c_NoQualityFactor = numeric_limits<double>::max()/3;
const double MRERawEvent::c_NoScore = numeric_limits<double>::max()/3; // remove later

////////////////////////////////////////////////////////////////////////////////


MRERawEvent::MRERawEvent() : MRESE(), MRotationInterface(), m_CoincidenceWindow(0)
{
  // Construct one MRERawEvent-object

  Init();
}


////////////////////////////////////////////////////////////////////////////////


MRERawEvent::MRERawEvent(MGeometryRevan *Geo) : MRESE(), MRotationInterface(), m_CoincidenceWindow(0)
{
  // Construct one MRERawEvent-object

  Init();

  m_Geo = Geo;
}


////////////////////////////////////////////////////////////////////////////////


MRERawEvent::MRERawEvent(MRERawEvent* RE) : MRESE((MRESE *) RE)
{
  // Some kind of copy constructor

  Init();

  SetGeometry(RE->GetGeometry());
  SetGoodEvent(RE->IsGoodEvent());

  if (RE->GetStartPoint() != 0) {
    int id = RE->GetStartPoint()->GetID();
    for (int i = 0; i < GetNRESEs(); ++i) {
      if (GetRESEAt(i)->GetID() == id) {
        SetStartPoint(GetRESEAt(i));
        break;
      }
    }
  }

  m_EventTime = RE->m_EventTime;
  m_EventClock = RE->m_EventClock;
  m_EventType = RE->GetEventType();
  m_EventID = RE->GetEventID();
  m_RejectionReason = RE->m_RejectionReason;
  m_TimeWalk = RE->m_TimeWalk;

  if (RE->m_HasGalacticPointing == true) { 
    m_GalacticPointingXAxis = RE->m_GalacticPointingXAxis;
    m_GalacticPointingZAxis = RE->m_GalacticPointingZAxis;
    m_HasGalacticPointing = true;
  }

  if (RE->m_HasDetectorRotation == true) { 
    m_DetectorRotationXAxis = RE->m_DetectorRotationXAxis;
    m_DetectorRotationZAxis = RE->m_DetectorRotationZAxis;
    m_HasDetectorRotation = true;  
  }

  if (RE->m_HasHorizonPointing == true) { 
    m_HorizonPointingXAxis = RE->m_HorizonPointingXAxis;
    m_HorizonPointingZAxis = RE->m_HorizonPointingZAxis;
    m_HasHorizonPointing = true; 
  }


  if (RE->GetPhysicalEvent() != 0) {
    if (m_EventType == MRERawEvent::c_ComptonEvent) {
      m_Event = new MComptonEvent();
      m_Event->Assimilate(RE->GetPhysicalEvent());
    } else if (m_EventType == MRERawEvent::c_PairEvent) {
      m_Event = new MPairEvent();
      m_Event->Assimilate(RE->GetPhysicalEvent());
    } else if (m_EventType == MRERawEvent::c_MipEvent) {
      m_Event = new MMuonEvent();
      m_Event->Assimilate(RE->GetPhysicalEvent());
    } else if (m_EventType == MRERawEvent::c_PhotoEvent) {
      m_Event = new MPhotoEvent();
      m_Event->Assimilate(RE->GetPhysicalEvent());
    } else if (m_EventType == MRERawEvent::c_UnknownEvent) {
      m_Event = new MUnidentifiableEvent();
      m_Event->Assimilate(RE->GetPhysicalEvent());
    } else {
      m_Event = 0;
      merr<<"Unknown event type: "<<m_EventType<<show;
    }    
  }

  int id;
  if (RE->GetVertex() != 0) {
    for (int r = 0; r < GetNRESEs(); r++) {
      id = GetRESEAt(r)->GetID();
      if (RE->GetVertex()->GetID() == id) {
        SetVertex(GetRESEAt(r));
        SetVertexDirection(RE->GetVertexDirection());
        break; 
      }
    }
  }

  // This vector is empty at the beginning!
  massert(m_Measurements.size() == 0);
  for (unsigned int m = 0; m < RE->m_Measurements.size(); ++m) {
    m_Measurements.push_back(RE->m_Measurements[m]->Clone()); 
  }
  
  m_Decay = RE->m_Decay;

  m_ClusteringQualityFactor = RE->m_ClusteringQualityFactor;

  m_ComptonQualityFactor1 = RE->m_ComptonQualityFactor1;
  m_ComptonQualityFactor2 = RE->m_ComptonQualityFactor2;

  m_TrackQualityFactor = RE->m_TrackQualityFactor;

  m_PairQualityFactor = RE->m_PairQualityFactor;

  m_GoodEvent = RE->m_GoodEvent;
  
  m_CoincidenceWindow = RE->m_CoincidenceWindow;
  
  m_Comments = RE->m_Comments;
}


////////////////////////////////////////////////////////////////////////////////


MRERawEvent::~MRERawEvent()
{
  // default destructor

  DeleteAll();  

  for (unsigned int m = 0; m < m_Measurements.size(); ++m) {
    delete m_Measurements[m]; 
  }
  m_Measurements.clear();

  delete m_Event;
}


////////////////////////////////////////////////////////////////////////////////


void MRERawEvent::DeleteAll()
{
  // Delete the links and RESEs of this event

  massert(this != 0);

  m_RESEList->DeleteAll();
}


////////////////////////////////////////////////////////////////////////////////


void MRERawEvent::Init()
{
  // Some initialisations equal for all constructors:

  MRotationInterface::Reset();
  
  m_Start = 0;
  m_Geo = 0;
  m_Event = 0;

  m_ClusteringQualityFactor = c_NoQualityFactor;
  m_ComptonQualityFactor1 = c_NoQualityFactor;
  m_ComptonQualityFactor2 = c_NoQualityFactor;
  m_TrackQualityFactor = c_NoQualityFactor;
  m_PairQualityFactor = c_NoQualityFactor;

  m_ElectronTrack = 0;
  m_PositronTrack = 0;

  m_RESEIt_c = 0;
  m_RESEIt_p = 0;
  m_Vertex = 0;
  m_VertexDirection = 0;
  
  m_SubElementType = MRESE::c_Event;
  m_GoodEvent = false;

  m_ExternalBadEventFlag = false;
  m_ExternalBadEventString = "";

  m_EventID = 0;
  m_EventType = c_UnknownEvent;
  m_RejectionReason = c_RejectionNone;
  m_TimeWalk = -1;
  m_EventTime.Set(0);
  m_EventClock.Set(0);
  
  m_GalacticPointingXAxis = MVector(1.0, 0.0, 0.0);
  m_GalacticPointingZAxis = MVector(0.0, 0.0, 1.0);
  m_HasGalacticPointing = false; 

  m_DetectorRotationXAxis = MVector(1.0, 0.0, 0.0);
  m_DetectorRotationZAxis = MVector(0.0, 0.0, 1.0);
  m_HasDetectorRotation = false; 
  
  m_HorizonPointingXAxis = MVector(1.0, 0.0, 0.0);
  m_HorizonPointingZAxis = MVector(0.0, 0.0, 1.0);
  m_HasHorizonPointing = false; 

  m_Decay = false;

  m_Measurements.clear();
  m_Comments.clear();
  
  m_CoincidenceWindow.Set(0);
}


////////////////////////////////////////////////////////////////////////////////


MRERawEvent* MRERawEvent::Duplicate()
{
  // Duplicate this raw event

  return new MRERawEvent(this);
}


////////////////////////////////////////////////////////////////////////////////


void MRERawEvent::Shuffle() 
{
  //! Shuffle the RESEs

  m_RESEList->Shuffle();
}


////////////////////////////////////////////////////////////////////////////////


int MRERawEvent::GetTrackLength()
{
  // Return the length of the *longest* track

  int Length = 0;
  for (int r = 0; r < GetNRESEs(); ++r) {
    if (GetRESEAt(r)->GetType() == MRESE::c_Track) {
      if (GetRESEAt(r)->GetNRESEs() > Length) {
        Length = GetRESEAt(r)->GetNRESEs();
      }
    }
  }

  return Length;
}

////////////////////////////////////////////////////////////////////////////////

int MRERawEvent::GetSequenceLength()
{
  // Return the number of independent RESEs

  return GetNRESEs();
}

 
////////////////////////////////////////////////////////////////////////////////


void MRERawEvent::SetGeometry(MGeometryRevan *Geo)
{
  // Set the geometry description

  m_Geo = Geo;
}


////////////////////////////////////////////////////////////////////////////////


MGeometryRevan* MRERawEvent::GetGeometry()
{
  // get the geometry description

  return m_Geo;
}



////////////////////////////////////////////////////////////////////////////////


void MRERawEvent::SetElectronTrack(MRESE* RESE)
{
  m_ElectronTrack = RESE;
}


////////////////////////////////////////////////////////////////////////////////


void MRERawEvent::SetPositronTrack(MRESE* RESE)
{
  m_PositronTrack = RESE;
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRERawEvent::GetElectronTrack()
{
  return m_ElectronTrack;
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRERawEvent::GetPositronTrack()
{
  return m_PositronTrack;
}


////////////////////////////////////////////////////////////////////////////////


double MRERawEvent::GetEnergy()
{

  double Energy = 0.0;

  for (int i = 0; i < GetNRESEs(); i++) {
    Energy += GetRESEAt(i)->GetEnergy();
  }  
  Energy += m_AdditionalEnergy;

  return Energy;
}


////////////////////////////////////////////////////////////////////////////////


MString MRERawEvent::ToString(bool WithLink, int Level)
{
  //

  char *Text = new char[2000];

  MString String;

  String = MString("");

  // Header:
  sprintf(Text, "Raw Event (%lu) at %lf s with %d hits and %.2f+-%.2f keV, starting with (%d):\n", 
          m_EventID, m_EventTime.GetAsSeconds(), GetNRESEs(), GetEnergy(), m_EnergyResolution, (m_Start != 0) ? m_Start->GetID() : -1);
  for (int i = 0; i < Level; i++) String += MString("   ");
  String += MString(Text);

  // Vetrex  
  if (m_Vertex != 0) {
    for (int i = 0; i < Level; i++) String += MString("   ");
    String += MString("Vertex: ");
    String += m_Vertex->ToString();
  } else {
    for (int i = 0; i < Level; i++) String += MString("   ");
    String += MString("No vertex!\n");
  }

  // RESEs
  for (int i = 0; i < GetNRESEs(); i++) {
    String += GetRESEAt(i)->ToString(WithLink, Level+1);
  }

  // Score
  if (m_Vertex == 0) {
    if (m_TrackQualityFactor != c_NoQualityFactor) {
      sprintf(Text, "RE total score: Compton: %.8f Track: %.8f \n", 
              m_ComptonQualityFactor1, m_TrackQualityFactor);
    } else {
      sprintf(Text, "RE total score: None\n");
    }
    for (int i = 0; i < Level; i++) {
      String += MString("   ");
    }
    String += MString(Text);
    
  } else {
    sprintf(Text, "RE total (pair) score %.3f ", m_PairQualityFactor);
    for (int i = 0; i < Level; i++) String += MString("   ");
    String += MString(Text);
  }

  if (IsGoodEvent() == false) {
    for (int i = 0; i < Level; i++) {
      String += MString("   ");
    }
    String += MString("BAD event:") + GetRejectionReasonAsString() + MString("\n");
  }

  if (m_Decay == true) {
    for (int i = 0; i < Level; i++) {
      String += MString("   ");
    }
    String += MString("Decay flag set");
  }

  delete [] Text;

  return String;
}


////////////////////////////////////////////////////////////////////////////////


bool MRERawEvent::IsValid()
{
  if (m_IsValid == false) return false;
  
  for (int i = 0; i < GetNRESEs(); i++) {
    if (GetRESEAt(i)->IsValid() == false) {
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MRERawEvent::SetGoodEvent(bool IsGoodEvent)
{
  m_GoodEvent = IsGoodEvent;
}


////////////////////////////////////////////////////////////////////////////////


bool MRERawEvent::IsGoodEvent()
{
  return m_GoodEvent;
}


////////////////////////////////////////////////////////////////////////////////


void MRERawEvent::SetStartPoint(MRESE *RESE)
{
  m_Start = RESE;
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRERawEvent::GetStartPoint()
{
  return m_Start;
}


////////////////////////////////////////////////////////////////////////////////


void MRERawEvent::SetVertex(MRESE *RESE)
{
  m_Vertex = RESE;
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRERawEvent::GetVertex()
{
  return m_Vertex;
}


////////////////////////////////////////////////////////////////////////////////


void MRERawEvent::SetVertexDirection(int Direction)
{
  // 1: Up, -1: Down, otherwise: Do not know

  m_VertexDirection = Direction;
}


////////////////////////////////////////////////////////////////////////////////


int MRERawEvent::GetVertexDirection()
{
  return m_VertexDirection;
}


////////////////////////////////////////////////////////////////////////////////


void MRERawEvent::SetEventID(unsigned long Id)
{
  m_EventID = Id;
}


////////////////////////////////////////////////////////////////////////////////


unsigned long MRERawEvent::GetEventID()
{
  return m_EventID;
}


////////////////////////////////////////////////////////////////////////////////


int MRERawEvent::GetEventType()
{
  // Return the type of this event:

  return m_EventType;
}  

 
////////////////////////////////////////////////////////////////////////////////


void MRERawEvent::SetEventType(int Type)
{
  m_EventType = Type;
}

 
////////////////////////////////////////////////////////////////////////////////


MString MRERawEvent::GetEventTypeAsString()
{
  // Return the event type as a string:

  if (m_EventType == c_UnknownEvent) {
    return "Unidentifiable";
  } else if (m_EventType == c_ComptonEvent) {
    return "Compton";
  } else if (m_EventType == c_PairEvent) {
    return "Pair";
  } else if (m_EventType == c_MipEvent) {
    return "MIP";
  } else if (m_EventType == c_ShowerEvent) {
    return "Shower";
  } else if (m_EventType == c_PhotoEvent) {
    return "Photo";
  } else {
    return "Unknown event";
  }
}


////////////////////////////////////////////////////////////////////////////////


void MRERawEvent::SetRejectionReason(int Reason)
{
  m_RejectionReason = Reason;
  
  if (m_RejectionReason != c_RejectionNone) {
    m_IsValid = false;
  }
}


////////////////////////////////////////////////////////////////////////////////


int MRERawEvent::GetRejectionReason()
{
  return m_RejectionReason;
}


////////////////////////////////////////////////////////////////////////////////


MString MRERawEvent::GetRejectionReasonAsString(int r, bool Short)
{
  // Return the rejec<tion reason as string:

  ostringstream out;

  switch (r) {
  case c_RejectionNone:
    if (Short == true) {
      out<<"None";
    } else {
      out<<"None";
    }
    break;
  case c_RejectionTooManyHitsCSR:
    if (Short == true) {
      out<<"TooManyHitsInCSR";
    } else {
      out<<"Too many hits in CSR";
    }
    break;
  case c_RejectionD1Only:
    if (Short == true) {
      out<<"EventRestrictedToStripDetectors";
    } else {
      out<<"Event is restricted to strip detectors";
    }
    break;
  case c_RejectionD2Only:
    if (Short == true) {
      out<<"EventRestrictedToCalorimeters";
    } else {
      out<<"Event is restricted to calorimeters";
    }
    break;
  case c_RejectionD3Only:
    if (Short == true) {
      out<<"EventRestrictedTo3DStripDetectors";
    } else {
      out<<"Event is restricted to 3D strip detectors";
    }
    break;
  case c_RejectionD4Only:
    if (Short == true) {
      out<<"EventRestrictedToScintillators";
    } else {
      out<<"Event is restricted to scintillators";
    }
    break;
  case c_RejectionD5Only:
    if (Short == true) {
      out<<"EventRestrictedToDriftChambers";
    } else {
      out<<"Event is restricted to drift chambers";
    }
    break;
  case c_RejectionD6Only:
    if (Short == true) {
      out<<"EventRestrictedToDirectionalStripDetectors";
    } else {
      out<<"Event is restricted to directional strip detectors";
    }
    break;
  case c_RejectionD7Only:
    if (Short == true) {
      out<<"EventRestrictedToAngerCameras";
    } else {
      out<<"Event is restricted to Anger cameras";
    }
    break;
  case c_RejectionD8Only:
    if (Short == true) {
      out<<"EventRestrictedToVoxelDetector";
    } else {
      out<<"Event is restricted to voxel detectors";
    }
    break;
  case c_RejectionTwoTracksOnly:
    if (Short == true) {
      out<<"EventConsistsOnlyOfTwoTracks";
    } else {
      out<<"Event consists of nothing but two tracks";
    }
    break;
  case c_RejectionOneTrackOnly:
    if (Short == true) {
      out<<"EventConsistsOnlyOfOneTrack";
    } else {
      out<<"Event consists of nothing but one track";
    }
    break;
  case c_RejectionTrackNotValid:
    if (Short == true) {
      out<<"TrackNotValid";
    } else {
      out<<"Track is not valid";
    }
    break;
  case c_RejectionSequenceBad:
    if (Short == true) {
      out<<"ComptonSequenceNotValid";
    } else {
      out<<"Compton sequence is not valid";
    }
    break;
  case c_RejectionTooManyHits:
    if (Short == true) {
      out<<"TooManyOverallHits";
    } else {
      out<<"Too many overall hits";
    }
    break;
  case c_RejectionEventStartNotD1:
    if (Short == true) {
      out<<"EventStartsNotInD1";
    } else {
      out<<"Event starts not in D1";
    }
    break;
  case c_RejectionNoHitsInTracker:  
    if (Short == true) {
      out<<"NoHitsInTracker";
    } else {
      out<<"Start in tracker required, but no hits in tracker";
    }
    break;
  case c_RejectionEventStartUndecided:
    if (Short == true) {
      out<<"EventStartUndecided";
    } else {
      out<<"Event start undecided";
    }
    break;
  case c_RejectionElectronDirectionBad:
    if (Short == true) {
      out<<"ElectronDirectionNotValid";
    } else {
      out<<"Electron direction is not valid";
    }
    break;
  case c_RejectionCSRThreshold:
    if (Short == true) {
      out<<"CSRQualityFactorAboveThreshold";
    } else {
      out<<"CSR quality factor above threshold";
    }
    break;
  case c_RejectionCSRNoGoodCombination:
    if (Short == true) {
      out<<"NoGoodCSRCombination";
    } else {
      out<<"No good CSR combination";
    }
    break;
  case c_RejectionComptelTypeEvent:
    if (Short == true) {
      out<<"EventIsComptelType";
    } else {
      out<<"Event is Comptel type (two hits, no track)";
    }
    break;
  case c_RejectionComptelTypeKinematicsBad:
    if (Short == true) {
      out<<"ComptelTypeWithIncompatibleKinematics";
    } else {
      out<<"Comptel type with incompatible kinematics";
    }
    break;
  case c_RejectionSingleSiteEvent:
    if (Short == true) {
      out<<"EventConsistsOnlyOfOneHitOrCluster";
    } else {
      out<<"Event consists only of one hit or cluster";
    }
    break;
  case c_RejectionNoHits:
    if (Short == true) {
      out<<"EventHasNoHits";
    } else {
      out<<"Event has no hits";
    }
    break;
  case c_RejectionTotalEnergyOutOfLimits:
    if (Short == true) {
      out<<"TotalEnergyOutOfLimits";
    } else {
      out<<"The total energy is out of limits";
    }
    break;
  case c_RejectionLeverArmOutOfLimits:
    if (Short == true) {
      out<<"LeverArmOutOfLimits";
    } else {
      out<<"The lever arm is out of limits";
    }
    break;
  case c_RejectionEventIdOutOfLimits:
    if (Short == true) {
      out<<"EventIDOutOfLimits";
    } else {
      out<<"The event ID is out of limits";
    }
    break;
  case c_RejectionNotFromObject:
    if (Short == true) {
      out<<"EventDoesNotOriginateFromGivenObjects";
    } else {
      out<<"The event does not originate from the given objects";
    }
    break;
  case c_RejectionTooManyUndecidedTrackElements:
    if (Short == true) {
      out<<"TooManyUndecidedTrackElements";
    } else {
      out<<"Too many undecided track elements";
    }
    break;
  case c_RejectionExternalBadEventFlag:
    if (Short == true) {
      out<<"ExternalBadEventFlagRaised";
    } else {
      out<<"External bad event flag raised";
    }
    break;
  case c_RejectionEventClusteringTooManyHits:
    if (Short == true) {
      out<<"EventClusteringTooManyHits";
    } else {
      out<<"Too many hits for the event clustering";
    }
    break;
  case c_RejectionTooManyEventIncarnations:
    if (Short == true) {
      out<<"TooManyEventIncarnations";
    } else {
      out<<"Too many event incarnations";
    }
    break;
  case c_RejectionEventClusteringUnresolvedHits:
    if (Short == true) {
      out<<"EventClusteringUnresolvedHits";
    } else {
      out<<"Unresolved hits in event clustering";
    }
    break;
  case c_RejectionEventClusteringNoOrigins:
    if (Short == true) {
      out<<"EventClusteringNoOrigins";
    } else {
      out<<"No origins found during event clustering";
    }
    break;
  case c_RejectionEventClusteringEnergyOutOfBounds:
    if (Short == true) {
      out<<"EventClusteringEnergyOutOfBounds";
    } else {
      out<<"The energy is outside of what the event clustering was trained for";
    }
    break;
  default:
    if (Short == true) {
      out<<"";
    } else {
      out<<"Unknown rejection type: "<<r;
    }
  }

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MRERawEvent::GetRejectionReasonAsString(bool Short)
{
  return GetRejectionReasonAsString(m_RejectionReason, Short);
}


////////////////////////////////////////////////////////////////////////////////


void MRERawEvent::SetPhysicalEvent(MPhysicalEvent* Event)
{
  m_GoodEvent = true;
  delete m_Event;
  m_Event = Event;
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MRERawEvent::GetPhysicalEvent()
{
  // Return a pointer to the physical event...

  // This is only an experimental flag --- don't activate it unless you know what you are doing...
  bool UseCenterD2 = false;

  if (m_Event != 0 && m_Event->GetType() != m_EventType) {
    // We had an upgrade...
    delete m_Event;
    m_Event = 0;
  }

  if (m_Event == 0) {
    if (m_EventType == c_ComptonEvent) {
      MComptonEvent* CE = new MComptonEvent();
      double ED1 = 0.0;
      double ED2 = 0.0;
      double EResD2 = 0.0;

      ED1 = m_Start->GetEnergy();
      ED2 = GetEnergy() - ED1;
      EResD2 = sqrt(GetEnergyResolution()*
                    GetEnergyResolution() -
                    m_Start->GetEnergyResolution()*
                    m_Start->GetEnergyResolution());
      
      double ToF = m_Start->GetLinkAt(0)->GetTime() - m_Start->GetTime();
      double dToF = sqrt(m_Start->GetLinkAt(0)->GetTimeResolution()*
                         m_Start->GetLinkAt(0)->GetTimeResolution() + 
                         m_Start->GetTimeResolution()*m_Start->GetTimeResolution());
      

      MVector Position2;
      if (UseCenterD2 == true) {
        cout<<"Centering..."<<endl;
        
        MRESEIterator Iter;
        Iter.Start(m_Start);
        Iter.GetNextRESE();
        double Energy = 0;
        while (Iter.GetNextRESE() != 0) {
          Position2 += Iter.GetCurrent()->GetPosition()*Iter.GetCurrent()->GetEnergy();
          Energy += Iter.GetCurrent()->GetEnergy();
        }
        Position2 *= 1/Energy;
        // AdditionalEnergy is not used, because no hits are available!
      } else {
        Position2 = m_Start->GetLinkAt(0)->GetPosition();
      }
      

      if (m_Start->GetType() == MRESE::c_Track) {
        CE->Assimilate(m_Start->GetPosition(),
                       Position2,
                       ((MRETrack *) m_Start)->GetDirection(), ED1, ED2);
        CE->SetTrackLength(m_Start->GetNRESEs());
        CE->SetTrackInitialDeposit(((MRETrack *) m_Start)->GetStartPoint()->GetEnergy());
        CE->SetTrackQualityFactor1(m_TrackQualityFactor);
      } else {
        CE->Assimilate(m_Start->GetPosition(),
                       Position2,
                       MVector(0.0, 0.0, 0.0), ED1, ED2);
        CE->SetTrackInitialDeposit(m_Start->GetEnergy());
        CE->SetTrackLength(1);
      }
      CE->SetdEg(EResD2);
      CE->SetdEe(m_Start->GetEnergyResolution());
      CE->SetdC1(m_Start->GetPositionResolution());
      CE->SetdC2(m_Start->GetLinkAt(0)->GetPositionResolution());
      CE->SetSequenceLength(GetNRESEs());
      // If there was no clustering, don't set a quality factor
      if (m_ClusteringQualityFactor != c_NoQualityFactor) {
        CE->SetClusteringQualityFactor(m_ClusteringQualityFactor);
      }
      CE->SetComptonQualityFactor1(m_ComptonQualityFactor1);
      CE->SetComptonQualityFactor2(m_ComptonQualityFactor2);
      if (ToF != 0) {
        CE->SetToF(ToF);
        CE->SetdToF(dToF);
      }

      double LeverArm = numeric_limits<double>::max();
      ResetIterator();
      MRESE *A, *B;
      if ((A = GetNextRESE()) != 0) {
        while ((B = GetNextRESE()) != 0) {
          if ((B->GetPosition() - A->GetPosition()).Mag() < LeverArm) {
            LeverArm = (B->GetPosition() - A->GetPosition()).Mag();
          }
          A = B;
        }
      }
      CE->SetLeverArm(LeverArm);
      
      CE->SetCoincidenceWindow(m_CoincidenceWindow);

      // Add as hits:
      MRESE* Start = m_Start;
      CE->AddHit(Start->CreatePhysicalEventHit());
      if (m_Start->GetNoiseFlags().Contains("NODEPTH") == true) {
        CE->SetBad(true, "NODEPTH"); 
      }
      MRESE* Middle = Start->GetLinkAt(0);
      if (Middle->GetNoiseFlags().Contains("NODEPTH") == true) {
        CE->SetBad(true, "NODEPTH"); 
      }
      CE->AddHit(Middle->CreatePhysicalEventHit());
      while (Middle->GetNLinks() > 1) {
        MRESE* End = Middle->GetOtherLink(Start);
        CE->AddHit(End->CreatePhysicalEventHit());
        Start = Middle;
        Middle = End;
      }
      
      
      m_Event = (MPhysicalEvent*) CE;
    } else if (m_EventType == c_PairEvent) {
      // We need to have two tracks:
      massert(m_ElectronTrack != 0);
      massert(m_PositronTrack != 0);

      MPairEvent* Pair = new MPairEvent();
      
      Pair->SetPairCreationIA(m_ElectronTrack->GetPosition());
      Pair->SetElectronDirection(((MRETrack *) m_ElectronTrack)->GetDirection());
      Pair->SetPositronDirection(((MRETrack *) m_PositronTrack)->GetDirection());
      Pair->SetEnergyElectron(m_ElectronTrack->GetEnergy());
      Pair->SetEnergyPositron(m_PositronTrack->GetEnergy());
      Pair->SetInitialEnergyDeposit(m_Vertex->GetEnergy());
      Pair->SetTrackQualityFactor(0.5*((MRETrack *) m_ElectronTrack)->GetQualityFactor() + 
                                  0.5*((MRETrack *) m_ElectronTrack)->GetQualityFactor());
      mimp<<"Pair track quality factors are not implemneted uniformly!!"<<endl;

      m_Event = (MPhysicalEvent*) Pair;
    } else if (m_EventType == c_PhotoEvent) {
      // We do have only one hit:
      massert(GetNRESEs() == 1);
      massert(GetRESEAt(0)->GetType() == MRESE::c_Hit || GetRESEAt(0)->GetType() == MRESE::c_Cluster);

      MPhotoEvent* Photo = new MPhotoEvent();
      Photo->SetEnergy(GetRESEAt(0)->GetEnergy());
      Photo->SetPosition(GetRESEAt(0)->GetPosition());

      m_Event = (MPhysicalEvent*) Photo;
      // } else if (m_EventType == c_MipEvent) {
      //mout<<"Do not store muons"<<endl;
      //} else if (m_EventType == c_ShowerEvent) {
      //mout<<"Do not store showers"<<endl;
    } else {
      MUnidentifiableEvent* U = new MUnidentifiableEvent();
      U->SetEnergy(GetEnergy());
      
      m_Event = (MPhysicalEvent*) U;
      m_Event->SetBad(true, GetRejectionReasonAsString(true));
    }
  }

  m_Event->Set(*dynamic_cast<MRotationInterface*>(this));
  
  m_Event->SetAllHitsGood(IsValid());
  m_Event->SetTime(m_EventTime);
  m_Event->SetId(m_EventID);
  m_Event->SetTimeWalk(m_TimeWalk);
  m_Event->SetDecay(m_Decay);
  if (m_ExternalBadEventFlag == true) {
    m_Event->SetBad(m_ExternalBadEventFlag, m_ExternalBadEventString);
  }

  for (unsigned int m = 0; m < m_Measurements.size(); ++m) {
    if (m_Measurements[m]->GetType() == MREAM::c_StartInformation) {
      MREAMStartInformation* Start = dynamic_cast<MREAMStartInformation*>(m_Measurements[m]);
      m_Event->SetOIInformation(Start->GetPosition(), Start->GetDirection(), Start->GetPolarization(), Start->GetEnergy());
    }
  }
  for (unsigned int c = 0; c < m_Comments.size(); ++c) {
    m_Event->AddComment(m_Comments[c]); 
  }

  m_Event->Validate();

  return m_Event;
}


////////////////////////////////////////////////////////////////////////////////


MString MRERawEvent::ToEvtaString(const int Precision, const int Version)
{
  ostringstream out;
  
  out<<"SE"<<endl;
  out<<"ID "<<m_EventID<<endl;
  out<<"TI "<<m_EventTime.GetLongIntsString()<<endl;

  MRotationInterface::Stream(out);
  
  for (int i = 0; i < GetNRESEs(); ++i) {
    out<<GetRESEAt(i)->ToEvtaString(Precision, Version);
  }
  for (unsigned int i = 0; i < GetNREAMs(); ++i) {
    out<<GetREAMAt(i)->ToEvtaString(Precision, Version);
  }
  for (unsigned int i = 0; i < m_Comments.size(); ++i) {
    out<<m_Comments[i]<<endl;
  }

  return out.str().c_str();
}
  
  
////////////////////////////////////////////////////////////////////////////////


MString MRERawEvent::ToCompactString()
{
  //

  if (IsGoodEvent() == false) {
    return MString("");
  }

  char Text[1000];
  double ED1 = 0.0, ED2 = 0.0;

  ED1 = m_Start->GetEnergy();
  ED2 = GetEnergy() - ED1;


  if (m_Start->GetType() == MRESE::c_Track) {
    sprintf(Text, 
            "C;%.3f;%.3f;%.3f;%.3f;%.3f;%.3f;"
            "%.6f;%.6f;%.6f;%.1f;%.1f; 10.0;10.0\n",
            m_Start->GetPosition().X(), 
            m_Start->GetPosition().Y(), 
            m_Start->GetPosition().Z(),
            m_Start->GetLinkAt(0)->GetPosition().X(), 
            m_Start->GetLinkAt(0)->GetPosition().Y(), 
            m_Start->GetLinkAt(0)->GetPosition().Z(),
            ((MRETrack *) m_Start)->GetDirection().X(),
            ((MRETrack *) m_Start)->GetDirection().Y(),
            ((MRETrack *) m_Start)->GetDirection().Z(),
            ED1, ED2);
  } else {
    sprintf(Text, 
            "C;%.3f;%.3f;%.3f;%.3f;%.3f;%.3f;"
            "%.6f;%.6f;%.6f;%.1f;%.1f; 10.0;10.0\n",
            m_Start->GetPosition().X(), 
            m_Start->GetPosition().Y(), 
            m_Start->GetPosition().Z(),
            m_Start->GetLinkAt(0)->GetPosition().X(), 
            m_Start->GetLinkAt(0)->GetPosition().Y(), 
            m_Start->GetLinkAt(0)->GetPosition().Z(),
            0.0, 0.0, 0.0, ED1, ED2);
  }

  return MString(Text);
}


////////////////////////////////////////////////////////////////////////////////


bool MRERawEvent::CreateTracks()
{
  // Create tracks
  // Combine everything linked in D1 to a track

  int i, j, h, h_last;
  bool NewTrack;
  MRESE *RESE;
  MRETrack *Track;
  bool Circular = true;

  //mdebug<<ToString()<<endl;

  // For each RESE in the raw event:
  for (h = 0; h < GetNRESEs(); h++) {
    // ---> Attention: this depends on sequence of calling the "analyzer":
    //      What if we have already some other links?
    //      Solution the call to MRETrackAnalyzer strips all links!
    
    h_last = h-1;

    //mdebug<<"preCheck: "<<h<<" - "<<GetRESEAt(h)->GetID()<<endl;
    // If we found a RESE with links
    if (GetRESEAt(h)->HasLinks() == true && GetRESEAt(h)->GetType() != MRESE::c_Track) {
      // First check if there is no track the RESE might belong to:
      //mdebug<<"Check: "<<GetRESEAt(h)->GetID()<<endl;
      NewTrack = true;
      // Search for a track in the raw event ...
      for (int r = 0; r < GetNRESEs(); r++) {
        RESE = GetRESEAt(r);
        // ... if we found one ...
        if (RESE->GetType() == MRESE::c_Track) {
          // Check if the "h" RESE has a link to a track-RESE
          
          for (int l = 0; l < GetRESEAt(h)->GetNLinks(); l++) {
            if (RESE->ContainsRESE(GetRESEAt(h)->GetLinkAt(l)) == true) {
              RESE->AddRESE(GetRESEAt(h));
              //mdebug<<"Add1: "<<GetRESEAt(h)->GetID()<<endl;
              RemoveRESE(GetRESEAt(h));
              CompressRESEs();
              NewTrack = false;
            }
          }
        }
      }
      // If the RESE belonged already to a track, jump to the next RESE ...
      if (NewTrack == false) {
        h = h_last;
        continue;
      }

    
      // ... otherwise the linked RESE belongs to a track.
      Track = new MRETrack();
      Track->AddRESE(GetRESEAt(h));
      RemoveRESE(GetRESEAt(h));
      CompressRESEs();
      h--;
      // For each element of the track
      for (i = 0; i < Track->GetNRESEs(); i++) {
        //mdebug<<"Track RESEs: "<<Track->GetNRESEs()<<endl;
        //mdebug<<Track->GetRESEAt(i)->GetNLinks()<<endl;
        for (j = 0; j < Track->GetRESEAt(i)->GetNLinks(); j++) {
          //mdebug<<"Links: "<<Track->GetRESEAt(i)->GetNLinks()<<endl;
          //mdebug<<"Test: "<<(int) Track->ContainsRESE(Track->GetRESEAt(i)->GetLinkAt(j))<<endl;
          if (Track->ContainsRESE(Track->GetRESEAt(i)->GetLinkAt(j)) == false) {
            //mdebug<<"Added!"<<endl;
            Track->AddRESE(Track->GetRESEAt(i)->GetLinkAt(j));
            //mdebug<<"Add2: "<<Track->GetRESEAt(i)->GetLinkAt(j)->GetID()<<endl;
            RemoveRESE(Track->GetRESEAt(i)->GetLinkAt(j));
            CompressRESEs();
            h = h_last; //h--;
          }
        }
      }
      // (?) Compress();
      AddRESE(Track);
  
      Circular = true;
      // Check if we are circular: All RESE must have two links:
      for (int i = 0; i < Track->GetNRESEs(); i++) {
        //mdebug<<"Check circularity for "<<Track->GetID()<<endl;
        if (Track->GetRESEAt(i)->GetNLinks() < 2) {
          //mdebug<<"2. not circular:"<<Track->GetRESEAt(i)->GetNLinks()<<endl;
          Circular = false;
          break;
        }
      }  
      if (Circular == true) {
        return false;
      }
    }
  }


  FindEndpoints();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MRERawEvent::FindEndpoints()
{
  // Find the endpoints of the track: This method makes sure, that the tracks 
  // have two non-ambigous linked endpoints, i.e. each endpoint has only one 
  // link.
  //
  // In the moment the endpoints have also two links (adjacents) (if we 
  // have more than two belonging hits - they should have only one). 
  // (a) If the "track" has hits not ordered in a circle its easy:
  //     Simply remove the links which have no corresponding links from the
  //     adjacent.
  // (b) Circular ordered: Remove the adjacents, linking to more distant 
  //     partners

  MRETrack* Track = 0;
  MRESE* RESE = 0;
  MRESE* RESE_sav = 0;
  //MRESE *RESE_last, *RESE_next;


  double Dist = 0;
  //MRESE *DistRESE;
  bool Circular = true;

  for (int h = 0; h < GetNRESEs(); h++) {
    if (GetRESEAt(h)->GetType() == MRESE::c_Track) {
      Track = (MRETrack *) GetRESEAt(h);
      Circular = true;

      // first check what case we have:
      // If we find unanswered links we have case (a) otherwise (b)
      for (int i = 0; i < Track->GetNRESEs(); i++) {
        RESE = Track->GetRESEAt(i);
        for (int a = 0; a < RESE->GetNLinks(); a++) {
          // if the adjancent of this RESE does not link to this RESE
          // then remove the adjacent ...
          if (RESE->GetLinkAt(a)->IsLink(RESE) == false) {
            // if and only if this is NOT the only link
            if (RESE->GetNLinks() > 1) {
              RESE->RemoveLink(RESE->GetLinkAt(a));
              a--; // We have a Compress in RemoveLink()
              Circular = false;
              //mdebug<<"1. not circular:"<<Track->GetRESEAt(i)->GetNLinks()<<endl;
            } else { // answer the link!!!
              // But if its the only link remove the more distant link of the partner!
              if (RESE->GetLinkAt(a)->GetNLinks() > 1) {
                Dist = 0;
                for (int b = 0; b < RESE->GetLinkAt(a)->GetNLinks(); b++) {
                  if (RESE->GetLinkAt(a)->GetLinkAt(b)->ComputeMinDistance(RESE->GetLinkAt(a)) > Dist) {
                    Dist = RESE->GetLinkAt(a)->GetLinkAt(b)->ComputeMinDistance(RESE->GetLinkAt(a));
                    RESE_sav = RESE->GetLinkAt(a)->GetLinkAt(b);
                  }
                }
                RESE->GetLinkAt(a)->RemoveLink(RESE_sav);
                RESE_sav->RemoveLink(RESE->GetLinkAt(a));
              }
              
              RESE->GetLinkAt(a)->AddLink(RESE);
            }
          } 
        }
      }


      // Check if we are circular: All RESE must have two links:
      for (int i = 0; i < Track->GetNRESEs(); i++) {
        //mdebug<<"Check circularity for "<<Track->GetID()<<endl;
        if (Track->GetRESEAt(i)->GetNLinks() < 2) {
          //mdebug<<"2. not circular:"<<Track->GetRESEAt(i)->GetNLinks()<<endl;
          Circular = false;
          break;
        }
      }

      if (Circular == true) {
        Fatal("MRERawEvent::FindEndpoints()",
              "Circular: Can't find endpoints!!!!");
      } // end circular

      // Now store the endpoints in the Track
      for (int i = 0; i < Track->GetNRESEs(); i++) {
        RESE = Track->GetRESEAt(i);
        if (RESE->GetNLinks() == 1) {
          //mdebug<<"AddEP: "<<RESE->ToString()<<endl;
          Track->AddEndPoint(RESE);
        }
      }

    } // end each track
  } // end each RESE
}


////////////////////////////////////////////////////////////////////////////////


void MRERawEvent::SetClusteringQualityFactor(double QF)
{
  m_ClusteringQualityFactor = QF;
}


////////////////////////////////////////////////////////////////////////////////


void MRERawEvent::SetComptonQualityFactors(double ComptonQualityFactor1, 
                                           double ComptonQualityFactor2)
{
  m_ComptonQualityFactor1 = ComptonQualityFactor1;
  m_ComptonQualityFactor2 = ComptonQualityFactor2;
}


////////////////////////////////////////////////////////////////////////////////


void MRERawEvent::SetTrackQualityFactor(double QF)
{
  m_TrackQualityFactor = QF;
}


////////////////////////////////////////////////////////////////////////////////


void MRERawEvent::SetPairQualityFactor(double QF)
{
  m_PairQualityFactor = QF;
}


////////////////////////////////////////////////////////////////////////////////


bool MRERawEvent::TestElectronDirection(double E1, double E2)
{
  // This method rejects the event if
  // (a) The electron angle is not computable
  // (b) the electron angle points backward
  // (c) there is a large discrepancy between alpha and phi + theta
  //
  // So this method does some elementary compton-analysis

  double E0 = 511.004;
  double phi, theta;
  MVector GammaDir, Origin;


  // Determine the photon-scatter-angle:
  phi = 1 - E0*(1/E2 - 1/(E1+E2));
  
  if (phi > -1 && phi < 1) {
    phi = acos(phi);
  } else {
    //mout<<"Electron test: Can't compute photon-scatter-angle!"<<endl;
    return false;
  }

  // Compute the electron-scatter-angle:
  theta = (E1*(E1+E2+E0))/((E1+E2)*sqrt(E1*E1 + 2*E1*(E1+E2)));

  if (theta > -1 && theta < 1) {
    theta = acos(theta);
  } else {
    //mout<<"Electron test: Can't compute electron-scatter-angle!"<<endl;
    return false;
  }

  if (theta > c_Pi/2.0) {
    //mout<<"Electron test: Not allowed back-scattering!"<<endl;
    return false;
  }


  // if we start with a track we can do some additional testing:

//   if (GetStartPoint()->GetType() == MRESE::c_Track) {
//     // Compute the scattered-photon-direction:
//     GammaDir = (GetStartPoint()->GetLinkAt(0)->GetPosition() - 
//                GetStartPoint()->GetPosition()).Unit();
    
//     // Compute the direction of the incoming gamma:
//     Origin = ((-1)*(1.0/(E1+E2))*
//              (((MRETrack *) GetStartPoint())->GetDirection()*sqrt(E1*E1+2*E0*E1) + GammaDir*E2)).Unit();
    
//     if (Origin.Z() < 0) {
//       mout<<"Electron test: Detector event!"<<endl;
//       return false;
//     }
//   }

  //mdebug<<"Result: "<<Origin.X()<<"!"<<Origin.Y()<<"!"<<Origin.Z()<<endl;

  //mdebug<<"phi: "<<phi<<"  theta: "<<theta<<endl;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MRERawEvent::ResetIterator(MRESE *RESE)
{
  // Reset the iterator
  // RESE is the startpoint of the iteration. 
  // It must have one and only one link! 
  // If RESE == 0 then one of the endpoints is used
  // Use GetNextRESE() for the iteration
  //
  // See GetNextRESE for an example

  if (RESE == 0) {
    m_RESEIt_p = 0;
    if (GetStartPoint() != 0) {
      m_RESEIt_c = GetStartPoint();
    } else {
      Fatal("MRERawEvent::ResetIterator(MRESE *RESE)",
            "Can't Reset the iterator, "
            "because I do not have any endpoints of the track!");
      return false;
    }
  } else {
    m_RESEIt_p = 0;
    m_RESEIt_c = RESE;

    if (RESE->GetNLinks() != 1) {
      Fatal("MRERawEvent::ResetIterator(MRESE *RESE)",
            "The iteration startpoint has %d links instead of one!", 
            RESE->GetNLinks());
      m_RESEIt_c = 0;
      return false;
    }
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRERawEvent::GetNextRESE()
{
  // Loop iterative over all RESE in the track
  // If NULL is returned then there is no next RESE
  //
  // Example:
  // Track->ResetIterator();
  // if (Track->IsValid() == true) {
  //   while ((RESE = Track->GetNextRESE()) != 0) {
  //   ...
  //   }
  // }

  MRESE *Temp;

  if (m_RESEIt_c == 0) {
    return 0;
  }

  if (m_RESEIt_c->GetNLinks() == 0) {
    return 0;
  }
  //mdebug<<m_RESEIt_c->GetNLinks()<<endl;
  //mdebug<<m_RESEIt_c->ToString()<<endl;
  Temp = m_RESEIt_c->GetOtherLink(m_RESEIt_p);
  m_RESEIt_p = m_RESEIt_c;
  m_RESEIt_c = Temp;

  return m_RESEIt_p;
}


////////////////////////////////////////////////////////////////////////////////


int MRERawEvent::ParseLine(const char* Line, int Version)
{
  // Return  0, if the line got correctly parsed
  // Return  1, if the line got not correctly parsed
  // Return  2, if the line got not parsed
  // Return -1, if the end of event has been reached

  int Ret = 0;

  if (Line[0] == 'T' && Line[1] == 'I') {
    if (m_EventTime.Set(Line) == false) {
      Ret = 1;
    }
  } else if (Line[0] == 'T' && Line[1] == 'W') {
    // Store the time-walk
    if (sscanf(Line, "TW%i", &m_TimeWalk) != 1) {
      Ret = 1;
    }  
  } else if (Line[0] == 'C' && Line[1] == 'L') {
    if (m_EventClock.Set(Line) == false) {
      Ret = 1;
    }
  } else if (Line[0] == 'I' && Line[1] == 'D') {
    // Store the event ID
    if (sscanf(Line, "ID %lu %*u", &m_EventID) != 1) {
      if (sscanf(Line, "ID %lu", &m_EventID) != 1) {
        Ret = 1;
      }
    }  
  } else if (Line[0] == 'R' && Line[1] == 'X') {
    if (sscanf(Line, "RX %lf %lf %lf", &m_DetectorRotationXAxis[0], &m_DetectorRotationXAxis[1], &m_DetectorRotationXAxis[2]) != 3) {
      Ret = 1;
    }
    m_HasDetectorRotation = true;
  } else if (Line[0] == 'R' && Line[1] == 'Z') {
    if (sscanf(Line, "RZ %lf %lf %lf", &m_DetectorRotationZAxis[0], &m_DetectorRotationZAxis[1], &m_DetectorRotationZAxis[2]) != 3) {
      Ret = 1;
    }
    m_HasDetectorRotation = true;
  } else if (Line[0] == 'G' && Line[1] == 'X') {
    double Longitude, Latitude;
    if (sscanf(Line, "GX %lf %lf", &Longitude, &Latitude) != 2) {
      Ret = 1;
    }
    SetGalacticPointingXAxis(Longitude, Latitude);
  } else if (Line[0] == 'G' && Line[1] == 'Z') {
    double Longitude, Latitude;
    if (sscanf(Line, "GZ %lf %lf", &Longitude, &Latitude) != 2) {
      Ret = 1;
    }
    SetGalacticPointingZAxis(Longitude, Latitude);
  } else if (Line[0] == 'H' && Line[1] == 'X') {
    double Azimuth, Altitude;
    if (sscanf(Line, "HX %lf %lf", &Azimuth, &Altitude) != 2) {
      Ret = 1;
    }
    m_HorizonPointingXAxis.SetMagThetaPhi(1.0, (90-Altitude)*c_Rad, Azimuth*c_Rad);
    m_HasHorizonPointing = true;
  } else if (Line[0] == 'H' && Line[1] == 'Z') {
    double Azimuth, Altitude;
    if (sscanf(Line, "HZ %lf %lf", &Azimuth, &Altitude) != 2) {
      Ret = 1;
    }
    m_HorizonPointingZAxis.SetMagThetaPhi(1.0, (90-Altitude)*c_Rad, Azimuth*c_Rad);
    m_HasHorizonPointing = true;
  } else if (Line[0] == 'P' && Line[1] == 'Q') {
    // Store the clustering quality factor
    if (sscanf(Line, "PQ %lf", &m_ClusteringQualityFactor) != 1) {
      Ret = 1;
    }
  } else if (Line[0] == 'B' && Line[1] == 'D') {
    // Store the clustering quality factor
    m_ExternalBadEventString = Line;
    m_ExternalBadEventString = m_ExternalBadEventString.Remove(0, 3);
    m_ExternalBadEventString = m_ExternalBadEventString.ReplaceAll("\n", "");
    m_ExternalBadEventFlag = true;
  } else if (Line[0] == 'C' && Line[1] == 'C') {
    // A comment
    MString Comment = Line;
    Comment = Comment.Remove(0, 3);
    Comment = Comment.ReplaceAll("\n", "");
    m_Comments.push_back(Comment);
  } else if (Line[0] == 'X' && Line[1] == 'E') {
    // Store the total energy deposit in a drift chamber
    double Energy = 0;
    MVector Pos;
    if (sscanf(Line, "XE %lf;%lf;%lf;%lf", &Pos[0], &Pos[1], &Pos[2], &Energy) != 4) {
      Ret = 1;
    } else {
      massert(m_Geo != 0);
      MDVolumeSequence* V = m_Geo->GetVolumeSequencePointer(Pos, true, true);
      
      if (V->GetDetector() == 0) {
        mout<<"Position of XE does not represent a detector!"<<endl; 
        Ret = 1;
      } else if (V->GetDetector()->GetType() != MDDetector::c_DriftChamber) {
        mout<<"Position of XE does not represent a drift chamber!"<<endl; 
        Ret = 1;
      } else {
        MREAMDriftChamberEnergy* DCE = new MREAMDriftChamberEnergy();
        DCE->SetVolumeSequence(V); // DCE is responsible for the volume sequence!
        // We do NOT do any noising here!!
        // dynamic_cast<MDDriftChamber*>(V->GetDetector())->NoiseLightEnergy(Energy);
        DCE->SetEnergy(Energy);
        DCE->SetEnergyResolution(dynamic_cast<MDDriftChamber*>(V->GetDetector())->GetLightEnergyResolution(Energy));        
        m_Measurements.push_back(DCE);
      }
    }
  } else if (Line[0] == 'G' && Line[1] == 'R') {


    // Store the guard ring hit
    int Detector = 0;
    double Energy = 0;
    MVector Pos;

    if (sscanf(Line, "GR %lf;%lf;%lf;%lf", &Pos[0], &Pos[1], &Pos[2], &Energy) != 4) {
      if (sscanf(Line, "GRsim %d;%lf;%lf;%lf;%lf", &Detector, &Pos[0], &Pos[1], &Pos[2], &Energy) != 5) {
        Ret = 1;
      }
    }

    if (Ret == 0) {
      massert(m_Geo != 0);
      MDVolumeSequence* V = m_Geo->GetVolumeSequencePointer(Pos, true, true);
      
      if (V->GetDetector() == 0) {
        mout<<"Position of GR does not represent a detector!"<<endl; 
        Ret = 1;
      } else if (V->GetDetector()->HasGuardRing() == false && V->GetDetector()->GetType() != MDDetector::c_GuardRing) {
        mout<<"Position of GR does not represent a detector with guard ring!"<<endl; 
        Ret = 1;
      } else {
        MREAMGuardRingHit* GR = new MREAMGuardRingHit();
        if (V->GetDetector()->GetType() != MDDetector::c_GuardRing && V->GetDetector()->HasGuardRing() == true) {
          V->SetDetector(V->GetDetector()->GetGuardRing());
        }
        GR->SetVolumeSequence(V); // GR is responsible for the volume sequence!
        // We do NOT do any noising here!!
        // dynamic_cast<MDStrip2D*>(V->GetDetector())->NoiseGuardRingEnergy(Energy);
        GR->SetEnergy(Energy);
        GR->SetEnergyResolution(V->GetDetector()->GetEnergyResolution(Energy));
        m_Measurements.push_back(GR);
      }
    } else {
      Ret = 1;
    }
  } else if (Line[0] == 'O' && Line[1] == 'I') {
    // Start information
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double dx = 0.0;
    double dy = 0.0;
    double dz = 0.0;
    double px = 0.0;
    double py = 0.0;
    double pz = 0.0;
    double e = 0.0;
    if (sscanf(Line, "OI %lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf", &x, &y, &z, &dx, &dy, &dz, &px, &py, &pz, &e) == 10) {
      MREAMStartInformation* Start = new MREAMStartInformation();
      Start->SetPosition(MVector(x, y, z));
      Start->SetDirection(MVector(dx, dy, dz));
      Start->SetPolarization(MVector(px, py, pz));
      Start->SetEnergy(e);
      m_Measurements.push_back(Start);
    } else {
      Ret = 1;
    }
  } else if (Line[0] == 'D' && Line[1] == 'R') {
    // Store the guard ring hit
    MVector Pos;
    MVector Dir;
    double Energy;
    if (sscanf(Line, "DR %lf;%lf;%lf;%lf;%lf;%lf;%lf", &Pos[0], &Pos[1], &Pos[2], &Dir[0], &Dir[1], &Dir[2], &Energy) != 7) {
      Ret = 1;
    } else {
      massert(m_Geo != 0);
      MDVolumeSequence* V = m_Geo->GetVolumeSequencePointer(Pos, true, true);
      
      if (V->GetDetector() == 0) {
        mout<<"Position of DR does not represent a detector!"<<endl; 
        Ret = 1;
      } else if (V->GetDetector()->GetType() != MDDetector::c_Strip3DDirectional) {
        mout<<"Position of DR does not represent a detector with directional information of charged particles!"<<endl; 
        Ret = 1;
      } else {
        MREAMDirectional* DR = new MREAMDirectional();
        DR->SetVolumeSequence(V); // DR is responsible for the volume sequence!
        // We do NOT do any noising here!!
        DR->SetDirection(Dir);
        DR->SetEnergy(Energy);
        m_Measurements.push_back(DR);
      }
    }
  } else if (Line[0] == 'H' && Line[1] == 'T') {
    // if the hit contains the flag "XO" or "YO" than ignore it right now, but set the m_IsValid flag to bad:
    if (strstr(Line, "XO") != 0 || strstr(Line, "YO") != 0) {
      mdebug<<"int MRERawEvent::ParseLine: Line not parsed, since it contains "
            <<"XO or YO!"<<endl;
      m_IsValid = false;
      return 2;
    }
    // If we have an overflow, than the event is also not valid:
    if (strstr(Line, "OF") != 0) {
      m_IsValid = false;
    }

    if (GetNRESEs() > 10000) {
      mout<<"Hard coded hit limit of max. 10000 hits - no longer adding hits to event "<<m_EventID<<endl;
      m_IsValid = false;
      return 2;
    }

    MREHit* Hit = new MREHit();
    if (Hit->ParseLine(Line, Version) == false) {
      mout<<"Event "<<m_EventID<<": Unable to parse line - removing hit"<<endl;
      mout<<Line<<endl;
      m_IsValid = false;
      delete Hit;
      return 2;      
    }

    // Add position and energy resolution, IF it has NOT been set during reading of the individual line:
    if (Hit->HasFixedResolutions() == false) { 
      // Use the information from the geometry file:
      if (m_Geo != 0) {
        if (Hit->RetrieveResolutions(m_Geo) == false) {
          mout<<"Event "<<m_EventID<<": Unable to determine resolutions - removing hit"<<endl;
          mout<<Line<<endl;
          m_IsValid = false;
          delete Hit;
          return 2;  
        }
      }
    } else {
      if (m_Geo != 0) {
        if (Hit->UpdateVolumeSequence(m_Geo) == false) {
          mout<<"Event "<<m_EventID<<": Unable to update volume sequence - removing hit"<<endl;
          mout<<Line<<endl;
          m_IsValid = false;
          delete Hit;
          return 2; // We need to return not parsed here since we most likely  changed the geometry (e.g. removed detectors) and don't want those hits
        }
      }
    }

    // We need the resolutions (which also set the volume sequence), before we can safely add it!

    // Some sanity checks:
    if (Hit->GetEnergy() <= 0) {
      mout<<"Event "<<m_EventID<<": Hit has no positive energy: "<<Hit->GetEnergy()<<" keV"<<endl;
      m_IsValid = false;
      delete Hit;
    } else {
      AddRESE(Hit);
    }
  } else {
    Ret = 2;
  }

  return Ret;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the number of available additional measurements
unsigned int MRERawEvent::GetNREAMs() const
{
  return m_Measurements.size();
}


////////////////////////////////////////////////////////////////////////////////


//! Return the REAM at position a
MREAM* MRERawEvent::GetREAMAt(unsigned int a)
{
  if (a >= m_Measurements.size()) {
    merr<<"Index out of bounds: max="<<m_Measurements.size()-1<<" you="<<a<<endl;
    return 0;
  }

  return m_Measurements[a];
}


////////////////////////////////////////////////////////////////////////////////


//! Delete a REAM
void MRERawEvent::DeleteREAM(vector<MREAM*>::iterator& Iter)
{    
  MREAM* REAM = (*Iter);
  Iter = m_Measurements.erase(Iter);
  delete REAM;
}


////////////////////////////////////////////////////////////////////////////////


//! Remove and return a REAM
MREAM* MRERawEvent::RemoveREAM(vector<MREAM*>::iterator& Iter)
{    
  MREAM* REAM = (*Iter);
  Iter = m_Measurements.erase(Iter);
  return REAM;
}


// MRERawEvent.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
