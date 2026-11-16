/*
 * UTERTrackShared.h
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


#ifndef __UTERTrackShared__
#define __UTERTrackShared__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <cmath>
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MERTrack.h"
#include "MRERawEvent.h"
#include "MRESE.h"
#include "MRETrack.h"
#include "MRawEventIncarnations.h"
#include "UTRevanShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Base class of the unit tests of the electron trackers (MERTrack and its derived classes):
//! a tracker with twelve layers, events in it, the read out of the reconstructed tracks, and reference values of the quality factors
class UTERTrackBase : public UTRevanBase
{
public:
  //! Standard constructor with the name of the test
  UTERTrackBase(const MString& Name) : UTRevanBase(Name) {}
  //! Default destructor
  virtual ~UTERTrackBase() {}

protected:
  //! Return the test telescope with a tracker of twelve layers at z = -2, -1, 0, ..., 9 cm (the layer number is the z position in cm).
  //! The layers at -2 and -1 cm are above the calorimeter (top at -6 cm), the one at 9 cm is below the shield (bottom at 9.5 cm).
  //! The structural pitch of the tracker of the telescope is the layer distance of 1 cm along z:
  //! MGeometryRevan::GetLayerDistance() derives the layer distance of two hits from it
  MString GetTrackingGeometry(const MString& AdditionalLines = "") const
  {
    MString Lines;
    for (int l = -2; l <= 9; ++l) {
      // Layers 0 to 3 are part of the telescope
      if (l >= 0 && l <= 3) {
        continue;
      }
      MString Name = "Holder";
      if (l < 0) {
        Name += "Minus";
        Name += -l;
      } else {
        Name += l;
      }
      Lines += MString("Holder.Copy ") + Name + "\n";
      Lines += Name + ".Position 0.0 0.0 " + l + ".0\n";
      Lines += Name + ".Mother World\n";
    }
    Lines += AdditionalLines;

    return GetTelescopeGeometry(Lines);
  }

  //! Scan the test telescope with the twelve layer tracker into the geometry
  bool ScanTrackingGeometry(MGeometryRevan& Geometry, const MString& AdditionalLines = "")
  {
    return ScanGeometry(Geometry, GetTrackingGeometry(AdditionalLines), "Tracking.geo.setup");
  }

  //! Return the names of the detectors in which tracks are searched: the tracker of the test telescope
  vector<MString> GetTrackingDetectors() const
  {
    vector<MString> Names;
    Names.push_back("Tracker");
    return Names;
  }

  //! Give the tracker the geometry and the parameters: the searches, the maximum layer jump, and the number of sequences to keep.
  //! The tracking detector is the tracker of the test telescope, and the number of layers for the vertex search is 4
  void SetupTracker(MERTrack& Tracker, MGeometryRevan& Geometry, bool SearchMIPs, bool SearchPairs, bool SearchComptons, unsigned int MaxLayerJump = 2, unsigned int NSequencesToKeep = 1, bool RejectPureAmbiguities = false)
  {
    Tracker.SetGeometry(&Geometry);
    DisableDefaultStreams();
    Tracker.SetParameters(SearchMIPs, SearchPairs, SearchComptons, MaxLayerJump, NSequencesToKeep, RejectPureAmbiguities, 4, GetTrackingDetectors());
    EnableDefaultStreams();
  }

  //! Add a hit at (X, Y) cm in the tracker layer at z = Layer cm (-2 to 9) to the event, which owns it afterwards. Return the hit, or nullptr if it is not in a detector
  MRESE* AddTrackerHit(MRERawEvent* Event, MGeometryRevan& Geometry, double X, double Y, int Layer, double Energy)
  {
    MREHit* Hit = CreateHit(MVector(X, Y, 1.0*Layer), Energy, MDDetector::c_Strip2D, &Geometry);
    if (Hit != nullptr) {
      Event->AddRESE(Hit);
    }
    return Hit;
  }

  //! Add a hit at the given position (cm) in the calorimeter block (centered at (0, 0, -10) cm) to the event, which owns it afterwards. Return the hit, or nullptr if it is not in the calorimeter
  MRESE* AddCalorimeterHit(MRERawEvent* Event, MGeometryRevan& Geometry, const MVector& Position, double Energy)
  {
    MREHit* Hit = CreateHit(Position, Energy, MDDetector::c_Calorimeter, &Geometry);
    if (Hit != nullptr) {
      Event->AddRESE(Hit);
    }
    return Hit;
  }

  //! Return a new event with a typical Compton electron track plus a calorimeter hit. The caller owns the event, until it is handed to an incarnation list.
  //! The electron starts in layer 3 and stops in layer 0, and its energy deposits increase along the track:
  //! (0, 0, 3) cm 50 keV, (1, 0, 2) cm 60 keV, (2, 0, 1) cm 80 keV, (3, 0, 0) cm 200 keV, and (0, 0, -10) cm 500 keV in the calorimeter
  MRERawEvent* CreateStraightTrackEvent(MGeometryRevan& Geometry)
  {
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    AddTrackerHit(Event, Geometry, 0.0, 0.0, 3, 50.0);
    AddTrackerHit(Event, Geometry, 1.0, 0.0, 2, 60.0);
    AddTrackerHit(Event, Geometry, 2.0, 0.0, 1, 80.0);
    AddTrackerHit(Event, Geometry, 3.0, 0.0, 0, 200.0);
    AddCalorimeterHit(Event, Geometry, MVector(0.0, 0.0, -10.0), 500.0);

    return Event;
  }

  //! Return the correlation score of the track of CreateStraightTrackEvent() from layer 3 to layer 0: the correlation of (50, 60, 80, 200) with (1, 2, 3, 4).
  //! By hand: the mean is 97.5, the deviations are (-47.5, -37.5, -17.5, 102.5) and (-1.5, -0.5, 0.5, 1.5),
  //! thus the covariance sum is 71.25 + 18.75 - 8.75 + 153.75 = 235, the variance sums are 14475 and 5, and r = 235/sqrt(72375) = 0.873521...
  double GetStraightTrackScore() const
  {
    return 235.0/sqrt(72375.0);
  }

  //! Return a new event with a pair: a vertex hit of 100 keV at (0, 0, 9) cm, and below it two straight legs with one hit each in the next NLayers layers.
  //! The hit of the first leg in layer l is at x = +0.25*(9 - l) cm with (100 + l) keV, the one of the second leg at x = -0.25*(9 - l) cm with (200 + l) keV.
  //! The hits are added from the top to the bottom, the first leg first. All positions can be represented exactly, thus the directions of all segments of a leg are identical.
  //! The caller owns the event, until it is handed to an incarnation list
  MRERawEvent* CreatePairEvent(MGeometryRevan& Geometry, int NLayers)
  {
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    AddTrackerHit(Event, Geometry, 0.0, 0.0, 9, 100.0);
    for (int l = 8; l > 8 - NLayers; --l) {
      AddTrackerHit(Event, Geometry, 0.25*(9 - l), 0.0, l, 100.0 + l);
      AddTrackerHit(Event, Geometry, -0.25*(9 - l), 0.0, l, 200.0 + l);
    }

    return Event;
  }

  //! Hand the event over to the list as its initial event, and analyze the list with the tracker. The streams are silenced during the analysis.
  //! The list does not delete its events in its destructor: call DeleteAll() when done
  bool AnalyzeEvent(MERTrack& Tracker, MRawEventIncarnations& List, MRERawEvent* Event)
  {
    List.SetInitialRawEvent(Event);
    DisableDefaultStreams();
    bool Result = Tracker.Analyze(&List);
    EnableDefaultStreams();

    return Result;
  }

  //! Return the number of tracks in the event
  unsigned int GetNTracks(MRERawEvent* Event) const
  {
    unsigned int NTracks = 0;
    for (int r = 0; r < Event->GetNRESEs(); ++r) {
      if (Event->GetRESEAt(r)->GetType() == MRESE::c_Track) {
        ++NTracks;
      }
    }
    return NTracks;
  }

  //! Return the track with the given index (in the order of the event), or nullptr if there is none
  MRETrack* GetTrack(MRERawEvent* Event, unsigned int Index) const
  {
    unsigned int NTracks = 0;
    for (int r = 0; r < Event->GetNRESEs(); ++r) {
      if (Event->GetRESEAt(r)->GetType() == MRESE::c_Track) {
        if (NTracks == Index) {
          return dynamic_cast<MRETrack*>(Event->GetRESEAt(r));
        }
        ++NTracks;
      }
    }
    return nullptr;
  }

  //! Return the number of the elements of the event which are no tracks
  unsigned int GetNNonTracks(MRERawEvent* Event) const
  {
    return static_cast<unsigned int>(Event->GetNRESEs()) - GetNTracks(Event);
  }

  //! Return the elements of the track in the order of its links, from its start point to its other end.
  //! The list is empty if the track has no start point, or if the links do not lead exactly once through all elements of the track
  vector<MRESE*> GetSequence(MRETrack* Track) const
  {
    vector<MRESE*> Sequence;
    if (Track == nullptr || Track->GetStartPoint() == nullptr) {
      return Sequence;
    }

    MRESE* Previous = nullptr;
    MRESE* Current = Track->GetStartPoint();
    while (Current != nullptr) {
      Sequence.push_back(Current);
      if (static_cast<int>(Sequence.size()) > Track->GetNRESEs()) {
        Sequence.clear();
        return Sequence;
      }

      // The next element is the link we did not come from:
      MRESE* Next = nullptr;
      for (int l = 0; l < Current->GetNLinks(); ++l) {
        if (Current->GetLinkAt(l) != Previous) {
          Next = Current->GetLinkAt(l);
        }
      }
      Previous = Current;
      Current = Next;
    }
    if (static_cast<int>(Sequence.size()) != Track->GetNRESEs()) {
      Sequence.clear();
    }

    return Sequence;
  }

  //! Return the layers (z positions in cm, rounded) of the elements of the track from its start point to its other end, separated by blanks, e.g. "3 2 1 0".
  //! The text is empty if the track has no start point or broken links, see GetSequence()
  MString GetLayerSequence(MRETrack* Track) const
  {
    MString Layers;
    vector<MRESE*> Sequence = GetSequence(Track);
    for (unsigned int i = 0; i < Sequence.size(); ++i) {
      if (i > 0) {
        Layers += " ";
      }
      Layers += static_cast<int>(lround(Sequence[i]->GetPosition().Z()));
    }

    return Layers;
  }

  //! Return the Pearson correlation coefficient of the values with their index 1, 2, ..., N: the reference of the quality factors.
  //! It is the textbook formula sum((x - <x>)(y - <y>)) / sqrt(sum((x - <x>)^2) sum((y - <y>)^2)), and 0 if one of the variances is zero
  double GetIndexCorrelation(const vector<double>& Values) const
  {
    double N = Values.size();
    double MeanIndex = 0.0;
    double MeanValue = 0.0;
    for (unsigned int i = 0; i < Values.size(); ++i) {
      MeanIndex += (i + 1)/N;
      MeanValue += Values[i]/N;
    }
    double Covariance = 0.0;
    double VarianceIndex = 0.0;
    double VarianceValue = 0.0;
    for (unsigned int i = 0; i < Values.size(); ++i) {
      Covariance += (i + 1 - MeanIndex)*(Values[i] - MeanValue);
      VarianceIndex += (i + 1 - MeanIndex)*(i + 1 - MeanIndex);
      VarianceValue += (Values[i] - MeanValue)*(Values[i] - MeanValue);
    }
    if (VarianceIndex == 0.0 || VarianceValue == 0.0) {
      return 0.0;
    }

    return Covariance/sqrt(VarianceIndex*VarianceValue);
  }

  //! Return the correlation score of a track with the given positions and energies of its hits from its start to its end, the reference of MRETrack::CalculatePearsonCorrelation():
  //! the correlation of the energies with the position along the track (an electron deposits more energy towards its end),
  //! averaged with the correlation of the direction changes at the inner hits with the position along the track (it scatters more towards its end), if there are at least three inner hits.
  //! +1 is a perfect electron track, -1 a track in the wrong direction
  double GetCorrelationScore(const vector<MVector>& Positions, const vector<double>& Energies) const
  {
    double EnergyScore = GetIndexCorrelation(Energies);
    if (Positions.size() < 5) {
      return EnergyScore;
    }

    vector<double> Angles;
    for (unsigned int i = 1; i + 1 < Positions.size(); ++i) {
      Angles.push_back((Positions[i] - Positions[i-1]).Angle(Positions[i+1] - Positions[i]));
    }

    return 0.5*(EnergyScore + GetIndexCorrelation(Angles));
  }

  //! Return the correlation score of the track as it is linked, from its start point to its other end, see GetCorrelationScore() above. It is 0 for a track without a valid sequence
  double GetCorrelationScore(MRETrack* Track) const
  {
    vector<MVector> Positions;
    vector<double> Energies;
    vector<MRESE*> Sequence = GetSequence(Track);
    if (Sequence.size() == 0) {
      return 0.0;
    }
    for (MRESE* RESE: Sequence) {
      Positions.push_back(RESE->GetPosition());
      Energies.push_back(RESE->GetEnergy());
    }

    return GetCorrelationScore(Positions, Energies);
  }
};


#endif


// UTERTrackShared.h: the end...
////////////////////////////////////////////////////////////////////////////////
