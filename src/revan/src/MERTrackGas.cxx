/*
 * MERTrackGas.cxx
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
// MERTrackGas
//
// Base class for event reconstruction tasks, e.g. find clusters, tracks,
// Compton sequence
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERTrackGas.h"

// Standard libs:
#include <list>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <functional>
#include <limits>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MRESE.h"
#include "MRESEIterator.h"
#include "MRETrack.h"
#include "MREHit.h"
#include "MMuonEvent.h"
#include "MTimer.h"
#include "MFile.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MERTrackGas)
#endif


////////////////////////////////////////////////////////////////////////////////


MERTrackGas::MERTrackGas() : MERTrack()
{
  // Construct an instance of MERTrackGas
}


////////////////////////////////////////////////////////////////////////////////


MERTrackGas::~MERTrackGas()
{
  // Delete this instance of MERTrackGas
}


////////////////////////////////////////////////////////////////////////////////


MRawEventList* MERTrackGas::TrackComptons(MRERawEvent* RE)
{
  // Track Compton events
  mdebug<<"Track Compton events in gas detector..."<<endl;

  mout<<"MRawEventList* MERTrackGas::TrackComptons(MRERawEvent* ER): Under construction!"<<endl;

  // List of raw events - this is what TrackComptons returns
  MRawEventList* List = new MRawEventList();

  // define search radius (a few mm)
  double SearchRad = 0.5; // cm - look for next track point within this radius
  //double SearchAng = 45.0 * TMath::Pi()/180.0; // try to find next track point within this

  // duplicate raw event - this is the one we search in
  MRERawEvent* REdup = RE->Duplicate();

  // output raw event - only add finished tracks/RESEs to this
  MRERawEvent* REout = RE->Duplicate();
  // remove all hits, i.e. keep only all general info like ID, etc.
  for (int i = 0; i < REout->GetNRESEs(); ++i) {
    REout->DeleteRESE(REout->GetRESEAt(i));
  }
  REout->CompressRESEs();

  // First thing - take everything that isn't in gas out of REdup
  // and put into REout
  for (int i = 0; i < REdup->GetNRESEs(); ++i) {
    if (IsInTracker(REdup->GetRESEAt(i)) == true) {
      continue;
    }
    REout->AddRESE(REdup->GetRESEAt(i));
    REdup->RemoveRESE(REdup->GetRESEAt(i));
  }
  REdup->CompressRESEs();

  // How many hits/clusters?
  int NumLeft = 0;

  // main loop - look for all the tracks in the gas
  do {
    mout<<"Looking for tracks..."<<endl;
	
    // get center of gravity of non-track hits in gas
    MVector CoG(0.0, 0.0, 0.0);
    double Weight = 0;
    for (int i = 0; i < REdup->GetNRESEs(); ++i) {
      if ((IsInTracker(REdup->GetRESEAt(i)) == false) 
          || (REdup->GetRESEAt(i)->GetType() == MRESE::c_Track)) {
        continue; 
      }
      CoG += REdup->GetRESEAt(i)->GetEnergy()*REdup->GetRESEAt(i)->GetPosition();
      Weight += REdup->GetRESEAt(i)->GetEnergy();
    }

    if (Weight == 0) {
      merr<<" Raw event has no hits!"<<endl;
      List->AddRawEvent(REdup);
      delete REout;
      return List;
    }

    CoG = (1.0 / Weight) *CoG ;

    mout<<" Gas hits center of gravity: "<<CoG<<endl;

    // find closest RESE to CoG = start point
    double MinDist = numeric_limits<double>::max();
    MRESE* Start = 0;
    for (int i = 0; i < REdup->GetNRESEs(); ++i) {
      if ((IsInTracker(REdup->GetRESEAt(i)) == false) 
          || (REdup->GetRESEAt(i)->GetType() == MRESE::c_Track)) {
        continue; 
      }
      if ((REdup->GetRESEAt(i)->GetPosition() - CoG).Mag() < MinDist) {
        MinDist = (REdup->GetRESEAt(i)->GetPosition() - CoG).Mag();
        Start = REdup->GetRESEAt(i);
      }
    }
    if (Start == 0) {
      merr<<" No start point for tracking found!"<<endl;
      List->AddRawEvent(REdup);
      delete REout;
      return List;
    }

    mout<<" Closest RESE to CoG: "<<Start->GetPosition()<<" ID: "<<Start->GetID()<<endl;

    MRETrack* Track = new MRETrack();
    Track->AddRESE(Start);
    Track->CreateEndPoints();
    REdup->RemoveRESEAndCompress(Start);

    int NumWithin = 0;
    do {
      NumWithin = 0;

      // Find the RESE closest to one of the end points, within search radius:
      int EndPoint = 0;
      double MinDistance = numeric_limits<double>::max();
      MRESE* ClosestRESE = 0;
      for (int i = 0; i < REdup->GetNRESEs(); ++i) {
        if ((IsInTracker(REdup->GetRESEAt(i)) == false) 
            || (REdup->GetRESEAt(i)->GetType() == MRESE::c_Track)) {
          continue; 
        }
        //NumWithin++;
        for (int e = 0; e < Track->GetNEndPoints(); ++e) {
          double Distance = (Track->GetEndPointAt(e)->GetPosition() - REdup->GetRESEAt(i)->GetPosition()).Mag();
          if (Distance < MinDistance) {
            MinDistance = Distance;
            EndPoint = e;
            ClosestRESE = REdup->GetRESEAt(i);
          }
        }
      }
      //mout<<"Num: "<<NumWithin<<endl;
      //mout<<"Closest: "<<ClosestRESE->ToString()<<endl;

      // only add to track if within search radius
      if (MinDistance < SearchRad) {
        NumWithin++;
        Track->AddEndPoint(ClosestRESE, Track->GetEndPointAt(EndPoint));
        REdup->RemoveRESEAndCompress(ClosestRESE);
      }
    } while (NumWithin > 0);

    // Now clean up random, isolated hits "near" track
    // Assume probability of another true track so close is negligible
    // I don't know what to do with these except throw them out...

    // get CoG of track
    MVector TrackCoG = Track->GetCenterOfGravity();

    // find max distance of track from CoG
    double TrackRadius = 0.0;
    for (int i = 0; i < Track->GetNRESEs(); ++i) {
      double Distance = (Track->GetRESEAt(i)->GetPosition() - TrackCoG).Mag();
      if (Distance > TrackRadius) {
        TrackRadius = Distance;
      }
    }

    // Any hits or clusters within this radius of TrackCoG that are not 
    // part of the track should be thrown out
    for (int i = 0; i < REdup->GetNRESEs(); ++i) {
      if ((IsInTracker(REdup->GetRESEAt(i)) == false)
          || (REdup->GetRESEAt(i)->GetType() == MRESE::c_Track)) {
        continue; 
      }
      double Distance = (REdup->GetRESEAt(i)->GetPosition() - TrackCoG).Mag();
      if (Distance < TrackRadius) {
        REdup->RemoveRESEAndCompress(REdup->GetRESEAt(i));
        i--;
      }
    }

    // Now see if any other possible tracks in the gas...
    NumLeft = 0;
    for (int i = 0; i < REdup->GetNRESEs(); ++i) {
      if (IsInTracker(REdup->GetRESEAt(i)) == false) {
        continue; 
      }
      if (REdup->GetRESEAt(i)->GetType() != MRESE::c_Track) {
        NumLeft++;
      }
    }
    mout<<" NumLeft: "<<NumLeft<<endl;

    // Now that we're all done cleaning up and seeing what's left,
    // Add the track!
    // ONLY if has more than 1 element
    if (Track->GetNRESEs() > 1) {
      mout<<" Adding track with "<<Track->GetNRESEs()<<" elements"<<endl;
      REout->AddRESE(Track);
    } else {
      // if not, just add the RESE, not the track
      mout<<" Adding single hit, ID: "<<Track->GetRESEAt(0)->GetID()<<endl;
      REout->AddRESE(Track->GetRESEAt(0));
      delete Track;
    }

  } while (NumLeft > 0);

  // Some debugging for me
  /*
  for (int i = 0; i < REout->GetNRESEs(); ++i) {
	if (REout->GetRESEAt(i)->GetType() == MRESE::c_Track) {
		MRETrack* Track2 = (MRETrack*) REout->GetRESEAt(i);
		for (int e = 0; e < Track2->GetNRESEs(); e++) {
			mout<<"T "<<Track2->GetRESEAt(e)->GetDetector()<<" "<<Track2->GetRESEAt(e)->GetPosition().X()<<" "<<Track2->GetRESEAt(e)->GetPosition().Y()<<" "<<Track2->GetRESEAt(e)->GetPosition().Z()<<endl;
		}
	} else {
		mout<<"H "<<REout->GetRESEAt(i)->GetDetector()<<" "<<REout->GetRESEAt(i)->GetPosition().X()<<" "<<REout->GetRESEAt(i)->GetPosition().Y()<<" "<<REout->GetRESEAt(i)->GetPosition().Z()<<endl;
	}
  }
  */

  // Clean up:
  delete REdup;

  mdebug<<"New RE: "<<REout->ToString()<<endl;

  // The list always contains only one raw event...
  List->AddRawEvent(REout);
  return List;
}


////////////////////////////////////////////////////////////////////////////////


bool MERTrackGas::EvaluateTracks(MRERawEvent* RE)
{
  // Evaluate the given tracks...
  
  if (RE->GetVertex() != 0) {
    return EvaluatePairs(RE);
  }

  // Peter: Add you evaluations here:
  //mout<<"MRawEventList* MERTrackGas::EvaluateTracks(MRERawEvent* ER): nyi!"<<endl;
  // Track evaluations should be same as always
  return MERTrack::EvaluateTracks(RE);

  // Store your results:
  RE->SetTrackQualityFactor(MRERawEvent::c_NoQualityFactor);
  RE->SetPairQualityFactor(MRERawEvent::c_NoQualityFactor);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MERTrackGas::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:

  ostringstream out;

  if (CoreOnly == false) {
    out<<"# Tracking - Gas options:"<<endl;
    out<<"# "<<endl;
  }
  out<<MERTrack::ToString(true);
  if (CoreOnly == false) {
    out<<"# "<<endl;
  }

  return out.str().c_str();
}


// MERTrackGas.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
