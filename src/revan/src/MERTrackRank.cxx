/*
 * MERTrackRank.cxx
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


// Include the header:
#include "MERTrackRank.h"

// Standard libs:
#include <list>
#include <algorithm>
#include <functional>
#include <limits>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MRESE.h"
#include "MRETrack.h"
#include "MREHit.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MERTrackRank)
#endif


////////////////////////////////////////////////////////////////////////////////


MERTrackRank::MERTrackRank() : MERTrack()
{
  // Construct an instance of MERTrackRank

  m_AllowOnlyMinNumberOfRESEsD1 = true;
}


////////////////////////////////////////////////////////////////////////////////


MERTrackRank::~MERTrackRank()
{
  // Delete this instance of MERTrackRank
}



////////////////////////////////////////////////////////////////////////////////


bool MERTrackRank::EvaluateTracks(MRERawEvent* RE)
{
  int NTracks = 0;
  double QF = 1.0;
  MRESE* RESE = 0;
  for (int i = 0; i < RE->GetNRESEs(); i++) {
    RESE = RE->GetRESEAt(i);
    if (RESE->GetType() == MRESE::c_Track) {
      MRETrack* Track = (MRETrack*) RESE;
      EvaluateTrack(Track);
      QF *= (1-Track->GetQualityFactor());
      NTracks++;
    }
  }

  QF = 1-QF;

  if (NTracks == 0) {
    RE->SetTrackQualityFactor(MRERawEvent::c_NoQualityFactor);
    RE->SetPairQualityFactor(MRERawEvent::c_NoQualityFactor);
  } else {
    if (RE->GetVertex() != 0) {
      mimp<<"Hacked sanity check..."<<endl; 
      RE->SetPairQualityFactor(1-QF);
      RE->SetEventType(MRERawEvent::c_PairEvent);
      RE->SetGoodEvent(true);
    } else {
      RE->SetTrackQualityFactor(QF);
      RE->SetPairQualityFactor(MRERawEvent::c_NoQualityFactor);
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MERTrackRank::SortByTrackQualityFactor(MRawEventList* List)
{
  // Sort decreasing:

  List->SortByTrackQualityFactor(false);
}


////////////////////////////////////////////////////////////////////////////////


bool MERTrackRank::EvaluateTrack(MRETrack* Track)
{
  Track->SetQualityFactor(Track->CalculateSpearmanRankCorrelation());

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MERTrackRank::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:

  ostringstream out;

  if (CoreOnly == false) {
    out<<"# Tracking - Rank options:"<<endl;
    out<<"# "<<endl;
  }
  out<<MERTrack::ToString(true);
  if (CoreOnly == false) {
    out<<"# "<<endl;
  }
  
  return out.str().c_str();
}



// MERTrackRank.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
