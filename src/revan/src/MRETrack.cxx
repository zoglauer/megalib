/*
 * MRETrack.cxx
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
// MRETrack.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MRETrack.h"

// Standard Libs:
#include <iostream>
#include <limits>
#include <map>
#include <cfloat>
using namespace std;

// ROOT libs
#include "TMath.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MRECluster.h"
#include "MRESEIterator.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MRETrack)
#endif


////////////////////////////////////////////////////////////////////////////////


  MRETrack::MRETrack() : MRESE()
{
  // Create a track object

  m_SubElementType = MRESE::c_Track;

  m_Start = 0;
  m_Stop = 0;
  m_EndPoints = new MRESEList();
  m_RESEIt_c = 0;      
  m_RESEIt_p = 0;       

  m_QualityFactor = 0;

  m_IsDirectionFixed = false;
  m_FixedDirection = g_VectorNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


MRETrack::MRETrack(MRETrack* Track) : MRESE((MRESE *) Track)
{
  // copy constructor

  m_SubElementType = MRESE::c_Track;

  int id;

  m_Start = 0;
  m_Stop = 0;
  
  m_EndPoints = new MRESEList();
  for (int e = 0; e < Track->GetNEndPoints(); e++) {
    id = Track->GetEndPointAt(e)->GetID();
    for (int i = 0; i < GetNRESEs(); i++) {
      if (GetRESEAt(i)->GetID() == id) {
        AddEndPoint(GetRESEAt(i));
        break;
      }
    }
  }
    
  if (Track->GetStartPoint() != 0) {
    id = Track->GetStartPoint()->GetID();
    for (int i = 0; i < GetNRESEs(); i++) {
      if (GetRESEAt(i)->GetID() == id) {
        m_Start = GetRESEAt(i);
        break;
      }
    }
  }
    
  if (Track->GetStopPoint() != 0) {
    id = Track->GetStopPoint()->GetID();
    for (int i = 0; i < GetNRESEs(); i++) {
      if (GetRESEAt(i)->GetID() == id) {
        m_Stop = GetRESEAt(i);
        break;
      }
    }
  }

  m_RESEIt_c = 0;  
  m_RESEIt_p = 0;

  m_QualityFactor = Track->m_QualityFactor;
  
  m_IsDirectionFixed = Track->m_IsDirectionFixed;
  m_FixedDirection = Track->m_FixedDirection;
}


////////////////////////////////////////////////////////////////////////////////


MRETrack::~MRETrack()
{
  // Delete this object, but do not delete its links and RESEs 

  //cout<<"Track destructor!"<<endl;

  delete m_EndPoints; 
}


////////////////////////////////////////////////////////////////////////////////


int MRETrack::GetDetector()
{
  // return the detector the track lays in

  return m_Detector;
}


////////////////////////////////////////////////////////////////////////////////


double MRETrack::ComputeMinDistance(MRESE *RESE)
{
  // Compute the minimum distance between this track and the sub element:

  return ComputeMinDistanceVector(RESE).Mag();
}


////////////////////////////////////////////////////////////////////////////////


MVector MRETrack::ComputeMinDistanceVector(MRESE *RESE)
{
  // Compute the minimum distance between this track and the sub element:

  int h;
  MVector Dist;
  MVector MinDist(DBL_MAX, DBL_MAX, DBL_MAX);

  if (RESE->GetType() == MRESE::c_Hit) {
    for (h = 0; h < GetNRESEs(); h++) {
      Dist = (-1) * GetRESEAt(h)->ComputeMinDistanceVector(RESE);
      if (Dist.Mag() < MinDist.Mag()) {
        MinDist = Dist;
      }
    }
  } else if (RESE->GetType() == MRESE::c_Cluster) {
    for (h = 0; h < GetNRESEs(); h++) {
      Dist = (-1) * GetRESEAt(h)->ComputeMinDistanceVector(RESE);
      if (Dist.Mag() < MinDist.Mag()) {
        MinDist = Dist;
      }
    }
  } else if (RESE->GetType() == MRESE::c_Track) {
    for (h = 0; h < GetNRESEs(); h++) {
      Dist = (-1) * GetRESEAt(h)->ComputeMinDistanceVector(RESE);
      if (Dist.Mag() < MinDist.Mag()) {
        MinDist = Dist;
      }
    }
  }

  return MinDist;
}


////////////////////////////////////////////////////////////////////////////////


bool MRETrack::AreAdjacent(MRESE* RESE, double Sigma, int Level)
{
  // Compute the minimum distance between this track and the sub element:

  int h, h_max;
  h_max = GetNRESEs();

  if (RESE->GetType() == MRESE::c_Hit) {
    for (h = 0; h < h_max; h++) {
      if (GetRESEAt(h)->AreAdjacent(RESE, Sigma, Level) == true) return true;
    }
  } else if (RESE->GetType() == MRESE::c_Cluster) {
    for (h = 0; h < h_max; h++) {
      if (GetRESEAt(h)->AreAdjacent(RESE, Sigma, Level) == true) return true;
    }
  } else if (RESE->GetType() == MRESE::c_Track) {
    for (h = 0; h < h_max; h++) {
      if (GetRESEAt(h)->AreAdjacent(RESE, Sigma, Level) == true) return true;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MRETrack::AddRESE(MRESE *RESE)
{
  // Add hits and clusters to this track.
  // Adding tracks is not allowed in the moment

  if (RESE->GetType() != MRESE::c_Hit && RESE->GetType() != MRESE::c_Cluster) {
    merr<<"You can only add Hits and clusters to a Track, not this RESE: "<<endl;
    merr<<RESE->ToString().Data()<<show;
    massert(RESE->GetType() == MRESE::c_Hit || RESE->GetType() == MRESE::c_Cluster);
  }

  MRESE::AddRESE(RESE);

  if (m_AllowOverwrite == true) {
    // Set all values:
    m_Energy += RESE->GetEnergy();
    // The additonal energy of the RESEs is already contained!
    
    if (GetNRESEs() == 1) {
      m_Detector = GetRESEAt(0)->GetDetector();
    } else if (RESE->GetDetector() != m_Detector) {
      //merr<<"Different detector types... ok, if we have a pair event... Track: "<<m_Detector<<" new RESE:"<<RESE->GetDetector()<<show;
    }

    m_Time = min(m_Time, RESE->GetTime());
  }

  return;
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRETrack::RemoveRESE(MRESE *RESE)
{
  // Remove a RESE from this cluster
  // RESE must be of type MHit or MRECluster!!

  if (ContainsRESE(RESE) == true) {

    if (m_AllowOverwrite == true) {
      //  Undo the changes in AddHit()
      m_Energy -= RESE->GetEnergy();
    }

    return MRESE::RemoveRESE(RESE);
  } else {
    Fatal("MRETrack::RemoveRESE(MRESE *RESE)",
          "This track does not contain the RESE!");
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MRETrack::RecalculateResolutions()
{
  if (m_AllowOverwrite == false) return;

  m_EnergyResolution = 0;
  for (int r = 0; r < GetNRESEs(); ++r) {
    if (GetRESEAt(r) == 0) continue;
    m_EnergyResolution += 
      GetRESEAt(r)->GetEnergyResolution()*GetRESEAt(r)->GetEnergyResolution();
  }
  m_EnergyResolution = sqrt(m_EnergyResolution);

  if (m_Start != 0) {
    m_PositionResolution = m_Start->GetPositionResolution();
  } else {
    m_PositionResolution = MVector(0.0, 0.0, 0.0);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MRETrack::SetStartPoint(MRESE *RESE) 
{ 
  // Set the startpoint of this track

  m_Start = RESE; 
  m_Position = m_Start->GetPosition();

  if (m_EndPoints->ContainsRESE(m_Start) == true) {
    for (int i = 0; i < GetNEndPoints(); i++) {
      if (GetEndPointAt(i) != m_Start) {
        m_Stop = GetEndPointAt(i);
      }
    }
  }

  RecalculateResolutions();
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRETrack::GetStartPoint() 
{ 
  // Get the startpoint of this track
  // A startpoint must have been set previously.

  return m_Start; 
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRETrack::GetStopPoint() 
{ 
  // Get the startpoint of this track
  // A startpoint must have been set previously.

  return m_Stop; 
}


////////////////////////////////////////////////////////////////////////////////


//! Get the volume sequence: 
//! If it we have a start point, its the VS of the start point, other wise the VS of the first end point, otherwise the VS of the first RESE
MDVolumeSequence* MRETrack::GetVolumeSequence()
{
  if (m_Start != nullptr) {
    return m_Start->GetVolumeSequence(); 
  } else if (GetNEndPoints() > 0) {
    return GetEndPointAt(0)->GetVolumeSequence(); 
  } else if (GetNRESEs() > 0) {
    return GetRESEAt(0)->GetVolumeSequence(); 
  } else {
    return m_VolumeSequence; 
  }
}


////////////////////////////////////////////////////////////////////////////////


MVector MRETrack::GetDirection(int Length)
{
  // Get the direction of this track.
  // A startpoint must have been set previously.

  if (m_IsDirectionFixed == true) {
    return m_FixedDirection;
  }

  massert(GetNRESEs() >= 2);

  if (m_Start == 0) {
    merr<<"This track has no start point!"<<endl;
    return MVector(0.0, 0.0, 0.0);
  }

  if (Length <= 1) {
    return (m_Start->GetLinkAt(0)->GetPosition() - m_Start->GetPosition()).Unit();
  } else {
    MRESEIterator Iter(m_Start);
    if (Iter.GetNRESEs() < Length) {;
      return (Iter.GetRESEAt(Iter.GetNRESEs()-1)->GetPosition() - m_Start->GetPosition()).Unit();    
    } else {
      return (Iter.GetRESEAt(Length)->GetPosition() - m_Start->GetPosition()).Unit();
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


MVector MRETrack::GetDirectionOfEndPoint(MRESE* EndPoint, int Length)
{
  massert(GetNRESEs() >= 2);

  bool EndPointPartOfTrack = false;
  for (int e = 0; e < GetNEndPoints(); ++e) {
    if (GetEndPointAt(e) == EndPoint) {
      EndPointPartOfTrack = true;
      break;
    }
  }
  if (EndPointPartOfTrack == false) {
    merr<<"Endpoint not found in this RESE!"<<endl;
    massert(EndPointPartOfTrack == true);
  }

  if (Length <= 1) {
    return (EndPoint->GetPosition() - EndPoint->GetLinkAt(0)->GetPosition()).Unit();
  } else {
    MRESEIterator Iter(EndPoint);
    if (Iter.GetNRESEs() < Length) {;
      return (EndPoint->GetPosition() - Iter.GetRESEAt(Iter.GetNRESEs()-1)->GetPosition()).Unit();    
    } else {
      return (EndPoint->GetPosition() - Iter.GetRESEAt(Length)->GetPosition()).Unit();
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


MVector MRETrack::GetFinalDirection(int Length)
{
  // Get the direction at the stop point of the track

  if (m_Stop == 0) {
    merr<<"This track has no stop point!"<<endl;
    return MVector(0.0, 0.0, 0.0);
  }
  if (m_Stop->GetNLinks() != 1) {
    merr<<"The stop point is not linked to another hit!"<<endl;
    return MVector(0.0, 0.0, 0.0);
  }

  if (Length <= 1) {
    return (m_Stop->GetPosition() - m_Stop->GetLinkAt(0)->GetPosition()).Unit();
  } else {
    mimp<<"Never tested..."<<endl;
    MRESEIterator Iter(m_Stop);
    if (Iter.GetNRESEs() < Length) {;
      return (Iter.GetRESEAt(Iter.GetNRESEs()-1)->GetPosition() - m_Stop->GetPosition()).Unit();    
    } else {
      return (Iter.GetRESEAt(Length)->GetPosition() - m_Stop->GetPosition()).Unit();
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


MVector MRETrack::GetAverageDirection()
{
  // Calculate linear regression (z is assumed to be ideally known)
  // y: y(z)=a0+a1*z
  // x: x(z)=b0+b1*z
  // D: n*sum(z^2)-sum(z)^2
  // a0 = (sum(z^2)*sum(y)-sum(z)*sum(y*z))/D
  // a1 = (n*sum(z*y)-sum(z)*sum(y))/D

  if (m_Start == 0) {
    merr<<"We do NOT have a start point! --> Ignoring bad track"<<show;
    return MVector(0.0, 0.0, 0.0);
  }
  if (m_Stop == 0) {
    merr<<"We do not have a stop point --> Ignoring bad track"<<show;
    return MVector(0.0, 0.0, 0.0);
  }

  // Calculate all variables:
  double D = 0;      // D
  double sumz = 0;   // sum(z)
  double sumz_2 = 0; // sum(z^2)
  double sum_z2 = 0; // sum(z)^2
  double sumyz = 0;  // sum(y*z)
  double sumy = 0;   // sum(y)
  double sumxz = 0;  // sum(x*z)
  double sumx = 0;   // sum(x)
  for (int i = 0; i < GetNRESEs(); ++i) {
    sumx += GetRESEAt(i)->GetPosition().X();
    sumy += GetRESEAt(i)->GetPosition().Y();
    sumz += GetRESEAt(i)->GetPosition().Z();
    sumxz += GetRESEAt(i)->GetPosition().X()*GetRESEAt(i)->GetPosition().Z();
    sumyz += GetRESEAt(i)->GetPosition().Y()*GetRESEAt(i)->GetPosition().Z();
    sum_z2 += GetRESEAt(i)->GetPosition().Z()*GetRESEAt(i)->GetPosition().Z();
  }
  sumz_2 = sumz*sumz;
  D = GetNRESEs()*sum_z2-sumz_2;
  if (D == 0) {
    // Return a simple:
    return m_Stop - m_Start;
  }
  //double a0 = (sum_z2*sumy-sumz*sumyz)/D;
  double a1 = (GetNRESEs()*sumyz-sumz*sumy)/D;
  //double b0 = (sum_z2*sumx-sumz*sumxz)/D;
  double b1 = (GetNRESEs()*sumxz-sumz*sumx)/D;

  //cout<<"Steigung: x/y: "<<atan(b1)*c_Deg<<"!"<<atan(a1)*c_Deg<<endl;

  // Now calculate the direction
  return MVector(b1, a1, 1);
}


////////////////////////////////////////////////////////////////////////////////


MVector MRETrack::GetCenterOfGravity()
{
  
  MVector CoG(0.0, 0.0, 0.0);
  double Weight = 0;
  for (int i = 0; i < GetNRESEs(); ++i) {
    CoG += GetRESEAt(i)->GetEnergy()*GetRESEAt(i)->GetPosition();
    Weight += GetRESEAt(i)->GetEnergy();
  }

  return (1.0 / Weight) *CoG ;
}


////////////////////////////////////////////////////////////////////////////////


void MRETrack::AddEndPoint(MRESE *RESE)
{
  // Set one if the endpoints

  m_EndPoints->AddRESE(RESE);
}


////////////////////////////////////////////////////////////////////////////////


void MRETrack::AddEndPoint(MRESE* NewEndPoint, MRESE* OldEndPoint)
{
  if (OldEndPoint == 0) {
    if (GetNEndPoints() == 0) {
      AddEndPoint(NewEndPoint);
      AddRESE(NewEndPoint);
    } else {
      merr<<"This track has end points, but you have not given one to link to!"<<endl;
      massert(OldEndPoint == 0 && GetNEndPoints() == 0);
    }
  } else {
    // Check if end point exists:
    bool EndPointPartOfTrack = false;
    for (int e = 0; e < GetNEndPoints(); ++e) {
      if (m_EndPoints->GetRESEAt(e) == OldEndPoint) {
        EndPointPartOfTrack = true;
        break;
      }
    }
    if (EndPointPartOfTrack == false) {
      merr<<"The end point was not found in this track!"<<endl;
      massert(OldEndPoint != 0 && EndPointPartOfTrack == true);
    }
    OldEndPoint->AddLink(NewEndPoint);
    NewEndPoint->AddLink(OldEndPoint);
    if (GetNEndPoints() == 2) {
      RemoveEndPoint(OldEndPoint);
    }
    AddEndPoint(NewEndPoint);
    AddRESE(NewEndPoint);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Append a track to the endpoint AND adds the hit to this Track AND removes them from the track!
void MRETrack::AddEndPoint(MRETrack* Track, MRESE* TrackEndPoint, MRESE* OldEndPoint)
{
  massert(Track != 0);
  massert(TrackEndPoint != 0);
  massert(OldEndPoint != 0);
  
  // Check if end point exists:
  bool EndPointPartOfTrack = false;
  for (int e = 0; e < GetNEndPoints(); ++e) {
    if (m_EndPoints->GetRESEAt(e) == OldEndPoint) {
      EndPointPartOfTrack = true;
      break;
    }
  }
  if (EndPointPartOfTrack == false) {
    merr<<"The end point was not found in this track!"<<endl;
    massert(OldEndPoint != 0 && EndPointPartOfTrack == true);
  }

  OldEndPoint->AddLink(TrackEndPoint);
  TrackEndPoint->AddLink(OldEndPoint);
  if (GetNEndPoints() == 2) {
    RemoveEndPoint(OldEndPoint);
  }
  if (Track->GetNEndPoints() == 1) {
    AddEndPoint(TrackEndPoint);
  } else if (Track->GetOtherEndPoint(TrackEndPoint) != 0) {
    AddEndPoint(Track->GetOtherEndPoint(TrackEndPoint));
  }

  for (int r = 0; r < Track->GetNRESEs(); ++r) {
    AddRESE(Track->GetRESEAt(r));
    Track->RemoveRESE(Track->GetRESEAt(r));
  }
}


////////////////////////////////////////////////////////////////////////////////


void MRETrack::SetStopPoint(MRESE *RESE)
{
  // Set a new stop point, remove the old one, link to the old one

  // Check if the RESE is already contained:
  bool Contained = false;
  for (int r = 0; r < GetNRESEs(); r++) {
    if (RESE == GetRESEAt(r)) {
      Contained = true;
      break;
    }
  }

  // If not add it:
  if (Contained == false) {
    AddRESE(RESE);
  }

  // If we do already have a stop point, 
  // then link between the old and new stop point
  RESE->RemoveAllLinks();

  if (GetStopPoint() != 0) {
    if (GetStopPoint()->GetNLinks() != 1) {
      Fatal("void MRETrack::SetStopPoint(MRESE *RESE)",
            "The stop point has not exactly one link!");
    }
    GetStopPoint()->AddLink(RESE);
    RESE->AddLink(GetStopPoint());
  } else if (GetStartPoint() != 0) {
    if (GetStartPoint()->GetNLinks() != 0) {
      Fatal("void MRETrack::SetStopPoint(MRESE *RESE)",
            "The start point has already links!");
    }
    GetStartPoint()->AddLink(RESE);
    RESE->AddLink(GetStartPoint());
  } else {
    Fatal("void MRETrack::SetStopPoint(MRESE *RESE)",
          "We nor have a start point, neither a stop point!");
  }

  m_Stop = RESE;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MRETrack::RemoveEndPoint(MRESE *RESE)
{
  // Remove an endpoint

  m_EndPoints->RemoveRESE(RESE);
  m_EndPoints->CompressRESEs();
}


////////////////////////////////////////////////////////////////////////////////


void MRETrack::RemoveAllEndPoints()
{
  // Remove all endpoints;

  m_EndPoints->RemoveAllRESEs();
  m_EndPoints->CompressRESEs();
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRETrack::GetEndPointAt(int i)
{
  // Get one of the endpoints - no error checks performed!

  return m_EndPoints->GetRESEAt(i);
}

////////////////////////////////////////////////////////////////////////////////


MRESE* MRETrack::GetOtherEndPoint(MRESE* RESE)
{
  // Get the other endpoints - no error checks performed!

  if (GetNEndPoints() <= 1) return 0;

  if (m_EndPoints->GetRESEAt(0) == RESE) {
    return m_EndPoints->GetRESEAt(1);
  } else if (m_EndPoints->GetRESEAt(1) == RESE) {
    return m_EndPoints->GetRESEAt(0);
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


int MRETrack::GetNEndPoints()
{
  // Get one of the endpoints

  return m_EndPoints->GetNRESEs();
}


////////////////////////////////////////////////////////////////////////////////


bool MRETrack::CreateEndPoints()
{
  // Endpoints can only be created, when the events are correctly linked, i.e.

  RemoveAllEndPoints();

  if (GetNRESEs() == 0) {
    return true;
  }
  if (GetNRESEs() == 1) {
    AddEndPoint(GetRESEAt(0));
    return true;
  }

  // Test if every event is correctly linked:
  // (a) we need two single linked points, 
  //     the rest must be double linked
  int NSingleLinked = 0;
  for (int i = 0; i < GetNRESEs(); i++) { 
    if (GetRESEAt(i)->GetNLinks() == 1) {
      NSingleLinked++;
      AddEndPoint(GetRESEAt(i));
    } else if (GetRESEAt(i)->GetNLinks() == 2) {
      // everything is ok ...
    } else {
      // Something is bad ...
      cout<<GetNRESEs()<<"Number of links: "<<GetRESEAt(i)->GetNLinks()<<GetRESEAt(i)->ToString()<<endl;
      RemoveAllEndPoints();
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MRETrack::ResetIterator(MRESE *RESE)
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
    } else if (GetNEndPoints() > 0) {
      m_RESEIt_c = GetEndPointAt(0);;
    } else {
      Fatal("MRETrack::ResetIteratior(MRESE *RESE)",
            "Can't Reset the iterator, "
            "because I do not have any endpoints of the track!");
      return false;
    }
  } else {
    m_RESEIt_p = 0;
    m_RESEIt_c = RESE;

    if (RESE->GetNLinks() != 1) {
      Fatal("MRETrack::ResetIteratior(MRESE *RESE)",
            "The iteration startpoint has %d links instead of one!", 
            RESE->GetNLinks());
      m_RESEIt_c = 0;
      return false;
    }
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRETrack::GetNextRESE()
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
  //cout<<m_RESEIt_c->GetNLinks()<<endl;
  //cout<<m_RESEIt_c->ToString()<<endl;
  Temp = m_RESEIt_c->GetOtherLink(m_RESEIt_p);
  m_RESEIt_p = m_RESEIt_c;
  m_RESEIt_c = Temp;

  return m_RESEIt_p;
}


////////////////////////////////////////////////////////////////////////////////


bool MRETrack::IsValid()
{
  // Performes some checks on the track.
  // If the track seems to be a good one return true

  if (m_Start == 0) {
    return false;
  }

  for (int i = 0; i < GetNRESEs(); ++i) {
    if (GetRESEAt(i)->IsValid() == false) return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MRETrack::LinkSequential()
{
  // The sequence of the RESEs is already identical to the sequence of the
  // links, so do the actual linking according to this specification.

  if (GetNRESEs() > 1) {
    m_Start = GetRESEAt(0);
    m_Stop = GetRESEAt(GetNRESEs()-1);

    m_Start->AddLink(GetRESEAt(1));
    for (int i = 1; i < GetNRESEs()-1; ++i) {
      GetRESEAt(i)->AddLink(GetRESEAt(i+1));
      GetRESEAt(i)->AddLink(GetRESEAt(i-1));
    }
    m_Stop->AddLink(GetRESEAt(GetNRESEs()-2));


    //cout<<ToString(true)<<endl;
    ValidateLinks();
    CreateEndPoints();
    SetStartPoint(m_Start);
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MRETrack::ValidateLinks()
{
  // Make sure that the first link within the RESE is always from start to stop 
  
  if (m_Start == 0 || m_Stop == 0) {
    Error("void MRETrack::ValidateLinks()",
          "No start and no stop!");
    return false;
  }

  MRESE* Previous = m_Start;
  MRESE* Current = m_Start->GetLinkAt(0);
  MRESE* Next = 0;

  do {
    if (Current->GetNLinks() == 1) break;
    if (Current->GetNLinks() != 2) {
      Error("void MRETrack::ValidateLinks()",
            "The track is not correctly linked!");
      return false;
    }
    // Protect against wrong sequence of links:
    // 1: link to next
    // 2: link to previous
    if (Current->GetLinkAt(0)->GetID() == Previous->GetID()) {
      Next = Current->GetLinkAt(1);
      Current->RemoveAllLinks();
      Current->AddLink(Next);
      Current->AddLink(Previous);
    }

    // Now switch:
    Previous = Current;
    Current = Previous->GetLinkAt(0);

  } while (true);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MRETrack* MRETrack::Duplicate() 
{
  // Duplicate this track

  return new MRETrack(this);
}


////////////////////////////////////////////////////////////////////////////////


double MRETrack::CalculateStraightnessScore()
{
  // This method evaluates the average straightness of the track:

  double ScoreStraightness = 0;

  if (m_Start == 0) {
    return ScoreStraightness;
  }

  if (GetNRESEs() < 3) {
    return ScoreStraightness;
  }


  MRESEIterator Iter;
  Iter.Start(GetStartPoint());
  Iter.GetNextRESE();
  Iter.GetNextRESE();
  
  while (Iter.GetNext() != 0) {
    ScoreStraightness += cos((Iter.GetNext()->GetPosition() - Iter.GetCurrent()->GetPosition()).Angle(MVector(0, 0, 1)) + 
                (Iter.GetCurrent()->GetPosition() - Iter.GetPrevious()->GetPosition()).Angle(MVector(0, 0, 1)));
    Iter.GetNextRESE();
  };

  ScoreStraightness /= (GetNRESEs()-2);

  return ScoreStraightness;
}


////////////////////////////////////////////////////////////////////////////////


double MRETrack::CalculateFirstLastHitScore()
{
  // the method scores the track when the first hit is the smallest
  // and the last one is the largest...

  double ScoreFirstLastHit = 0;

  if (GetNRESEs() < 2) {
    return ScoreFirstLastHit;
  }

  if (GetStartPoint() == 0 || GetStopPoint() == 0) {
    return ScoreFirstLastHit;
  }

  
  if (GetNRESEs() == 2) {
    ScoreFirstLastHit = 
      (GetStopPoint()->GetEnergy() - GetStartPoint()->GetEnergy())/
      (GetStopPoint()->GetEnergy() + GetStartPoint()->GetEnergy());
  } else if (GetNRESEs() == 3) {
    //double AngleIn;
    //double AngleOut;
    MRESEIterator Iter;
    Iter.Start(GetStartPoint());
    Iter.GetNextRESE();
    //AngleIn = (Iter.GetNext()->GetPosition() - Iter.GetCurrent()->GetPosition()).Angle(MVector(0, 0, 1));
    //Iter.GetNextRESE();

    while (Iter.GetNext() != 0) {
      //AngleOut = 0.5*((Iter.GetNext()->GetPosition() - Iter.GetCurrent()->GetPosition()).Angle(MVector(0, 0, 1)) + 
      //                (Iter.GetCurrent()->GetPosition() - Iter.GetPrevious()->GetPosition()).Angle(MVector(0, 0, 1)));
      Iter.GetNextRESE();
    };

    // ScoreFirstLastHit = 
    //   (Iter.GetPrevious()->GetEnergy()*cos(AngleOut) - GetStartPoint()->GetEnergy()*cos(AngleIn))/
    //   (Iter.GetPrevious()->GetEnergy()*cos(AngleOut) + GetStartPoint()->GetEnergy()*cos(AngleIn));

    ScoreFirstLastHit = 
      (Iter.GetPrevious()->GetEnergy() - GetStartPoint()->GetEnergy())/
      (Iter.GetPrevious()->GetEnergy() + GetStartPoint()->GetEnergy());
  }

  return ScoreFirstLastHit;
}


////////////////////////////////////////////////////////////////////////////////


// Structure needed for Spearman-Rank-Correlation:
struct MRanks {
  MRanks(int Id, double Value) { m_Id = Id; m_Value = Value; }
  
  int m_Id;
  double m_Value;
};
  
bool MRanksCompare(const MRanks& a, const MRanks& b) {
  return (a.m_Value < b.m_Value);
};

double MRETrack::CalculateSpearmanRankCorrelation()
{
  // Rank correlation: r = 1 - 6 sum (d*d)/(N(N*N-1))
  // d = index 1,2,3...
  // N = Energy, Angle change...

  // At which track length to start the angle correlation check
  // Has to be larger equal 4!
  const int StartAngleCorrelation = 6;
  massert(StartAngleCorrelation >= 4);

  double Coefficient = 0;

  double CoefficientEnergy = 0;
  double CoefficientAngle = 0;

  // We need two hits for a track...
  if (GetNRESEs() < 2) {
    return Coefficient;
  }

  // And a start and an end point
  if (GetStartPoint() == 0 || GetStopPoint() == 0) {
    merr<<"the track needs a start and an end point!"<<endl;
    return Coefficient;
  }

  // Store all data in two lists:

  double dAngle = 0;

  int EnergyCounter = 0;
  list<MRanks> Energies;
  list<MRanks> dAngles;

  MRESEIterator Iter;
  Iter.Start(GetStartPoint());
  Iter.GetNextRESE();
  // Angle = (Iter.GetNext()->GetPosition() - Iter.GetCurrent()->GetPosition()).Angle(MVector(0, 0, 1));
  // if (Angle > TMath::Pi()/2.0) Angle = TMath::Pi()-Angle;
  Energies.push_back(MRanks(EnergyCounter++, Iter.GetCurrent()->GetEnergy()));

  Iter.GetNextRESE();
  while (Iter.GetNext() != 0) {
    // We do have a central point and can therefore calculate the angular change:

    // Angle = (Iter.GetNext()->GetPosition() - Iter.GetPrevious()->GetPosition()).Angle(MVector(0, 0, 1));
    // if (Angle > TMath::Pi()/2.0) Angle = TMath::Pi()-Angle;
    
    dAngle = (Iter.GetCurrent()->GetPosition() - Iter.GetPrevious()->GetPosition()).
      Angle(Iter.GetNext()->GetPosition() - Iter.GetCurrent()->GetPosition());
    dAngles.push_back(MRanks(EnergyCounter-1, dAngle));

    Energies.push_back(MRanks(EnergyCounter++, Iter.GetCurrent()->GetEnergy()));

    Iter.GetNextRESE();
  }

  // The final energy data point:
  Energies.push_back(MRanks(EnergyCounter++, Iter.GetCurrent()->GetEnergy()));
 

  // Do the evaluation:
  Energies.sort(MRanksCompare);
  dAngles.sort(MRanksCompare);

  list<MRanks>::iterator MapIter;

  // For short tracks the angle criteria is not good enough, so do not use it not before we have 6 hit tracks
  if (EnergyCounter >= StartAngleCorrelation) {
    int Counter = 0;
    for (MapIter = dAngles.begin(); MapIter != dAngles.end(); ++MapIter) {
      CoefficientAngle += ((*MapIter).m_Id-Counter)*((*MapIter).m_Id-Counter);
      Counter++;
    }
    CoefficientAngle = 1 - 6*CoefficientAngle/(Counter*(Counter*Counter-1));
  }

  if (EnergyCounter >= 2) {
    int Counter = 0;
    for (MapIter = Energies.begin(); MapIter != Energies.end(); ++MapIter) {
      CoefficientEnergy += ((*MapIter).m_Id-Counter)*((*MapIter).m_Id-Counter);
      Counter++;
    }
    CoefficientEnergy = 1 - 6*CoefficientEnergy/(Counter*(Counter*Counter-1));
  }

  // Re-Normalize:
  if (EnergyCounter >= StartAngleCorrelation) {
    return (2-CoefficientEnergy-CoefficientAngle)/4;
  } else {
    return (1-CoefficientEnergy)/2;
  }
}


////////////////////////////////////////////////////////////////////////////////


double MRETrack::CalculatePearsonCorrelation(bool OneIsGood)
{
  // Remove the denom to get best performance!

  double ScoreCovariance = 0;
  double ScoreAngle = 0;
  double ScoreEnergy = 0;

  if (GetNRESEs() < 2) {
    return ScoreCovariance;
  }

  if (GetStartPoint() == 0 || GetStopPoint() == 0) {
    return ScoreCovariance;
  }

  double Angle = 0;
  double dAngle = 0;

  vector<double> Energies;
  vector<double> dAngles;

  MRESEIterator Iter;
  Iter.Start(GetStartPoint());
  Iter.GetNextRESE();
  Angle = (Iter.GetNext()->GetPosition() - Iter.GetCurrent()->GetPosition()).Angle(MVector(0, 0, 1));
  if (Angle > TMath::Pi()/2.0) Angle = TMath::Pi()-Angle;
  Energies.push_back(Iter.GetCurrent()->GetEnergy()/*cos(Angle)*/);
  Iter.GetNextRESE();
  
  while (Iter.GetNext() != 0) {
    Angle = (Iter.GetNext()->GetPosition() - Iter.GetPrevious()->GetPosition()).Angle(MVector(0, 0, 1));
    if (Angle > TMath::Pi()/2.0) Angle = TMath::Pi()-Angle;
    
    dAngle = (Iter.GetCurrent()->GetPosition() - Iter.GetPrevious()->GetPosition()).
      Angle(Iter.GetNext()->GetPosition() - Iter.GetCurrent()->GetPosition());
    //cout<<"dAngle: "<<dAngle*c_Deg<<endl;
    
    Energies.push_back(Iter.GetCurrent()->GetEnergy()/*cos(Angle)*/); // *cos(Angle));
    dAngles.push_back(dAngle); // *cos(Angle));

    Iter.GetNextRESE();
  }

  //if (GetNRESEs() == 2) {
  Energies.push_back(Iter.GetCurrent()->GetEnergy()/*cos(Angle)*/);
  //}

  double n;
  double Sum_xy;
  double Sum_x;
  double Sum_y;
  double Sum_x2;
  double Sum_y2;
  double Denom;

  // Calculate best fit straight line:
  if (dAngles.size() >= 2) {
    n = dAngles.size();
    Sum_xy = 0;
    Sum_x = 0;
    Sum_y = 0;
    Sum_x2 = 0;
    Sum_y2 = 0;
    
    for (unsigned int i = 0; i < dAngles.size(); ++i) {
      Sum_xy += (i+1)*dAngles[i];
      Sum_x  += (i+1);
      Sum_y  += dAngles[i];
      Sum_x2 += (i+1)*(i+1);
      Sum_y2 += dAngles[i]*dAngles[i];
    }
    Sum_xy = Sum_xy/n;
    Sum_x  = Sum_x/n;
    Sum_y  = Sum_y/n;
    Sum_x2 = Sum_x2/n;
    Sum_y2 = Sum_y2/n;
    
    Denom = sqrt(Sum_x2 - Sum_x*Sum_x)*sqrt(Sum_y2 - Sum_y*Sum_y);
    if (Denom != 0) {
      ScoreAngle = (Sum_xy - Sum_x*Sum_y)/Denom;
    }
  }

  // Calculate best fit straight line:
  n = Energies.size();
  Sum_xy = 0;
  Sum_x = 0;
  Sum_y = 0;
  Sum_x2 = 0;
  Sum_y2 = 0;
  
  for (unsigned int i = 0; i < Energies.size(); ++i) {
    Sum_xy += (i+1)*Energies[i];
    Sum_x  += (i+1);
    Sum_y  += Energies[i];
    Sum_x2 += (i+1)*(i+1);
    Sum_y2 += Energies[i]*Energies[i];
  }
  Sum_xy = Sum_xy/n;
  Sum_x  = Sum_x/n;
  Sum_y  = Sum_y/n;
  Sum_x2 = Sum_x2/n;
  Sum_y2 = Sum_y2/n;
    
  Denom = sqrt(Sum_x2 - Sum_x*Sum_x)*sqrt(Sum_y2 - Sum_y*Sum_y);
  if (Denom == 0) Denom = 1;
  ScoreEnergy = (Sum_xy - Sum_x*Sum_y)/Denom;

  if (OneIsGood == true) {
    if (dAngles.size() >= 3) {
      return (ScoreEnergy+ScoreAngle)/2;
    } else {
      return ScoreEnergy;
    }
  } else {
    if (dAngles.size() >= 3) {
      return (2-ScoreEnergy-ScoreAngle)/4;
    } else {
      return (1-ScoreEnergy)/2;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


double MRETrack::GetQualityFactor() const
{
  // Return the quality factor of the track

  return m_QualityFactor;
}


////////////////////////////////////////////////////////////////////////////////


void MRETrack::SetQualityFactor(double QualityFactor)
{
  // Set the quality factor of this events

  m_QualityFactor = QualityFactor;
}


////////////////////////////////////////////////////////////////////////////////


MString MRETrack::ToString(bool WithLink, int Level)
{
  // Returns a MString containing the relevant data of this object
  //
  // WithLink: Display the links
  // Level:    A level of N displays 3*N blancs before the text

  char Text[1000];

  MString String("");
  sprintf(Text, "Track (%d) starting with (%d) at (%.3f, %.3f, %.3f)+-(%.3f, %.3f, %.3f) with %.2f+-%.2f keV in %d and the following hits:\n",
          m_ID, (m_Start == 0) ? 0 : m_Start->GetID(), 
          m_Position.X(), m_Position.Y(), m_Position.Z(), 
          m_PositionResolution.X(), m_PositionResolution.Y(), m_PositionResolution.Z(), 
          GetEnergy(), GetEnergyResolution(), GetDetector());
  for (int i = 0; i < Level; i++) {
    String += MString("   ");
  }
  String += MString(Text);

  if (m_Start == 0) {
    for (int h = 0; h < GetNRESEs(); h++) {
      String += GetRESEAt(h)->ToString(WithLink, Level+1);
    }
  } else {
    MRESEIterator Iter(m_Start);
    while (Iter.GetNextRESE() != 0) {
      String += Iter.GetCurrent()->ToString(WithLink, Level+1);     
    } 
  }
 
  if (WithLink == true && GetNLinks() > 0) {
    for (int i = 0; i < Level+1; i++) {
      String += MString("   ");
    }
    String += MString("Linked with: ");   
    for (int a = 0; a < GetNLinks(); a++) {
      sprintf(Text, "  (%d)", GetLinkAt(a)->GetID());
      String += MString(Text);
    }
    String += MString("\n");
  }

  if (GetNEndPoints() != 0) {
    for (int i = 0; i < Level+1; i++) {
      String += MString("   ");
    }
    String += MString("Endpoints: \n");
    for (int h = 0; h < GetNEndPoints(); h++) {
      String += GetEndPointAt(h)->ToString(false, Level+2);
    }
  }

  sprintf(Text, "Quality factor: %.10f\n",  m_QualityFactor);
  for (int i = 0; i < Level+1; i++) {
    String += MString("   ");
  }
  String += MString(Text);

  return String;
}


////////////////////////////////////////////////////////////////////////////////


std::ostream& operator<<(std::ostream& os, /*const*/ MRETrack& Track)
{
  os<<Track.ToString();
  return os;
}



////////////////////////////////////////////////////////////////////////////////



// MRETrack.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
