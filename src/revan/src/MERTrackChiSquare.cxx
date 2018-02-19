/*
 * MERTrackChiSquare.cxx
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
// MERTrackChiSquare
//
// Base class for event reconstruction tasks, e.g. find clusters, tracks,
// Compton sequence
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERTrackChiSquare.h"

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

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MERTrackChiSquare)
#endif


////////////////////////////////////////////////////////////////////////////////


MERTrackChiSquare::MERTrackChiSquare() : MERTrack()
{
  // Construct an instance of MERTrackChiSquare
}


////////////////////////////////////////////////////////////////////////////////


MERTrackChiSquare::~MERTrackChiSquare()
{
  // Delete this instance of MERTrackChiSquare
}


////////////////////////////////////////////////////////////////////////////////


bool MERTrackChiSquare::EvaluateTracks(MRERawEvent* RE)
{
  // Evaluate the given tracks...

  if (RE->GetVertex() != 0) {
    return EvaluatePairs(RE);
  }

  // Mark: Add you evaluations here:
  mout<<"MRawEventIncarnations* MERTrackChiSquare::EvaluateTracks(MRERawEvent* ER): nyi!"<<endl;
  return MERTrack::EvaluateTracks(RE);

  // Store your results:
  RE->SetTrackQualityFactor(MRERawEvent::c_NoQualityFactor);
  RE->SetPairQualityFactor(MRERawEvent::c_NoQualityFactor);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MERTrackChiSquare::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:

  ostringstream out;

  if (CoreOnly == false) {
    out<<"# Tracking - ChiSquare options:"<<endl;
    out<<"# "<<endl;
  }
  out<<MERTrack::ToString(true);
  if (CoreOnly == false) {
    out<<"# "<<endl;
  }

  return out.str().c_str();
}


// MERTrackChiSquare.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
