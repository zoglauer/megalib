/*
 * MERTrackDirectional.cxx
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
// MERTrackDirectional
//
// Base class for event reconstruction tasks, e.g. find clusters, tracks,
// Compton sequence
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERTrackDirectional.h"

// Standard libs:
#include <list>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <functional>
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
#include "MREAM.h"
#include "MREAMDirectional.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MERTrackDirectional)
#endif


////////////////////////////////////////////////////////////////////////////////


MERTrackDirectional::MERTrackDirectional() : MERTrack()
{
  // Construct an instance of MERTrackDirectional
}


////////////////////////////////////////////////////////////////////////////////


MERTrackDirectional::~MERTrackDirectional()
{
  // Delete this instance of MERTrackDirectional
}


////////////////////////////////////////////////////////////////////////////////


MRawEventList* MERTrackDirectional::TrackComptons(MRERawEvent* RE)
{
  // Track Compton events

  //mout<<"TrackD - Track Comptons"<<endl;
  //mout<<RE->ToString()<<endl;

  // Look for all hits with directional info - and 

  // List of raw events - this is what TrackComptons returns
  MRawEventList* List = new MRawEventList();
  MRERawEvent* REdup = RE->Duplicate();
  List->AddRawEvent(REdup);

  for (unsigned int a = 0; a < RE->GetNREAMs(); ++a) {
    //mout<<REdup->ToString()<<endl;
    //mout<<"REAM #: "<<a<<endl;
    if (RE->GetREAMAt(a)->GetType() == MREAM::c_Directional) {
      MREAMDirectional* REAM = dynamic_cast<MREAMDirectional*>(RE->GetREAMAt(a));
      //mout<<"Updating: "<<REAM->ToString()<<flush;
      MRETrack* Track = new MRETrack();
      MRESE* RESE = REdup->GetRESEContainingRESE(REAM->GetRESE()->GetID());
      if (RESE->GetType() == MRESE::c_Track) {
        merr<<"TrackD - Two tracks in one cluster... Ignore the later..."<<show;
        continue;
      }
      MRESE* TrackStart = REdup->RemoveRESEAndCompress(RESE);
      Track->AddRESE(TrackStart);
      Track->SetStartPoint(TrackStart);
      Track->SetFixedDirection(REAM->GetDirection());
      REdup->AddRESE(Track);
    }
  }

  return List;
}


////////////////////////////////////////////////////////////////////////////////


bool MERTrackDirectional::EvaluateTracks(MRERawEvent* RE)
{
  // Evaluate the given tracks...
  
  if (RE->GetVertex() != 0) {
    return EvaluatePairs(RE);
  }

  // Store your results - we always assume it's perfect:
  RE->SetTrackQualityFactor(0.0);
  RE->SetPairQualityFactor(MRERawEvent::c_NoQualityFactor);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MERTrackDirectional::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:

  ostringstream out;

  if (CoreOnly == false) {
    out<<"# Tracking - Directional options:"<<endl;
    out<<"# "<<endl;
  }
  out<<MERTrack::ToString(true);
  if (CoreOnly == false) {
    out<<"# "<<endl;
  }

  return out.str().c_str();
}


// MERTrackDirectional.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
