/*
 * MERTrackRank.cxx
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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


#ifdef ___CLING___
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


void MERTrackRank::SortByTrackQualityFactor(MRawEventIncarnations* List)
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
