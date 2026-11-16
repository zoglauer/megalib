/*
 * UTRETrack.cxx
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


// Standard libs:
#include <cfloat>
#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MDDetector.h"
#include "MDVolumeSequence.h"
#include "MGeometryRevan.h"
#include "MRECluster.h"
#include "MREHit.h"
#include "MRESE.h"
#include "MRETrack.h"
#include "UTRevanShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the track class MRETrack
class UTRETrack : public UTRevanBase
{
public:
  //! Default constructor
  UTRETrack() : UTRevanBase("UTRETrack") {}
  //! Default destructor
  virtual ~UTRETrack() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Create hits at the positions with the energies, add them to the track, and link them in this sequence - the caller deletes the hits
  void FillTrack(MRETrack& Track, vector<MREHit*>& Hits, const vector<MVector>& Positions, const vector<double>& Energies);
  //! Create the reference track of four hits: (0, 0, 0), (3, 0, 4), (3, 4, 8), (3, 4, 12) with 100, 200, 300, 400 keV
  void FillReferenceTrack(MRETrack& Track, vector<MREHit*>& Hits);
  //! Create a straight track along z with the given energies, one hit per cm
  void FillVerticalTrack(MRETrack& Track, vector<MREHit*>& Hits, const vector<double>& Energies);
  //! Delete the hits
  void DeleteHits(vector<MREHit*>& Hits);

  //! Return true if the direction of a track with a single hit is a null vector
  bool DirectionOfSingleHit();
  //! Return true if the direction over as many steps as the track has hits is the direction to the last hit
  bool DirectionWithFullLength();
  //! Return true if removing a hit which is not in the track is survived and returns a nullptr
  bool RemoveUnknownHit();
  //! Return true if the iterator refuses a start in the middle of the track and the program goes on
  bool ResetIteratorInTheMiddle();

  //! Test the state of a new track
  bool TestDefaultConstruction();
  //! Test adding and removing hits and clusters
  bool TestAddAndRemoveRESE();
  //! Test linking the hits in their sequence
  bool TestLinkSequential();
  //! Test the start and the stop point
  bool TestStartAndStopPoint();
  //! Test the end points
  bool TestEndPoints();
  //! Test appending a track
  bool TestAppendTrack();
  //! Test the creation of the end points from the links
  bool TestCreateEndPoints();
  //! Test the direction at the start of the track
  bool TestGetDirection();
  //! Test the fixed direction
  bool TestFixedDirection();
  //! Test the direction at the end of the track and at its end points
  bool TestFinalAndEndPointDirections();
  //! Test the average direction and the center of gravity
  bool TestAverageDirectionAndCenterOfGravity();
  //! Test the iterator
  bool TestIterator();
  //! Test the validation of the links
  bool TestValidateLinks();
  //! Test the validity
  bool TestIsValid();
  //! Test the copy constructor and the duplication
  bool TestCopyAndDuplicate();
  //! Test the straightness and the first-last-hit score
  bool TestScores();
  //! Test the Pearson correlation
  bool TestPearsonCorrelation();
  //! Test the Spearman rank correlation
  bool TestSpearmanRankCorrelation();
  //! Test the quality factor
  bool TestQualityFactor();
  //! Test the distances and the adjacency check
  bool TestDistancesAndAreAdjacent();
  //! Test the volume sequence
  bool TestGetVolumeSequence();
  //! Test the text output
  bool TestToString();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTRETrack::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestAddAndRemoveRESE() && Passed;
  Passed = TestLinkSequential() && Passed;
  Passed = TestStartAndStopPoint() && Passed;
  Passed = TestEndPoints() && Passed;
  Passed = TestAppendTrack() && Passed;
  Passed = TestCreateEndPoints() && Passed;
  Passed = TestGetDirection() && Passed;
  Passed = TestFixedDirection() && Passed;
  Passed = TestFinalAndEndPointDirections() && Passed;
  Passed = TestAverageDirectionAndCenterOfGravity() && Passed;
  Passed = TestIterator() && Passed;
  Passed = TestValidateLinks() && Passed;
  Passed = TestIsValid() && Passed;
  Passed = TestCopyAndDuplicate() && Passed;
  Passed = TestScores() && Passed;
  Passed = TestPearsonCorrelation() && Passed;
  Passed = TestSpearmanRankCorrelation() && Passed;
  Passed = TestQualityFactor() && Passed;
  Passed = TestDistancesAndAreAdjacent() && Passed;
  Passed = TestGetVolumeSequence() && Passed;
  Passed = TestToString() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Create hits at the positions with the energies, add them to the track, and link them in this sequence - the caller deletes the hits
void UTRETrack::FillTrack(MRETrack& Track, vector<MREHit*>& Hits, const vector<MVector>& Positions, const vector<double>& Energies)
{
  for (unsigned int h = 0; h < Positions.size(); ++h) {
    MREHit* Hit = CreateHit(Positions[h], Energies[h], MDDetector::c_Strip2D);
    Hits.push_back(Hit);
    Track.AddRESE(Hit);
  }
  Track.LinkSequential();
}


////////////////////////////////////////////////////////////////////////////////


//! Create the reference track of four hits: (0, 0, 0), (3, 0, 4), (3, 4, 8), (3, 4, 12) with 100, 200, 300, 400 keV
void UTRETrack::FillReferenceTrack(MRETrack& Track, vector<MREHit*>& Hits)
{
  vector<MVector> Positions = { MVector(0.0, 0.0, 0.0), MVector(3.0, 0.0, 4.0), MVector(3.0, 4.0, 8.0), MVector(3.0, 4.0, 12.0) };
  vector<double> Energies = { 100.0, 200.0, 300.0, 400.0 };
  FillTrack(Track, Hits, Positions, Energies);
}


////////////////////////////////////////////////////////////////////////////////


//! Create a straight track along z with the given energies, one hit per cm
void UTRETrack::FillVerticalTrack(MRETrack& Track, vector<MREHit*>& Hits, const vector<double>& Energies)
{
  vector<MVector> Positions;
  for (unsigned int h = 0; h < Energies.size(); ++h) {
    Positions.push_back(MVector(0.0, 0.0, 1.0*h));
  }
  FillTrack(Track, Hits, Positions, Energies);
}


////////////////////////////////////////////////////////////////////////////////


//! Delete the hits
void UTRETrack::DeleteHits(vector<MREHit*>& Hits)
{
  for (MREHit* Hit: Hits) {
    delete Hit;
  }
  Hits.clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the direction of a track with a single hit is a null vector
bool UTRETrack::DirectionOfSingleHit()
{
  MREHit* Hit = CreateHit(MVector(1.0, 2.0, 3.0), 100.0, MDDetector::c_Strip2D);
  MRETrack Track;
  Track.AddRESE(Hit);
  Track.SetStartPoint(Hit);
  DisableDefaultStreams();
  MVector Direction = Track.GetDirection();
  EnableDefaultStreams();
  delete Hit;

  if (Direction.Mag() != 0) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the direction over as many steps as the track has hits is the direction to the last hit
bool UTRETrack::DirectionWithFullLength()
{
  MRETrack Track;
  vector<MREHit*> Hits;
  FillReferenceTrack(Track, Hits);
  DisableDefaultStreams();
  MVector Direction = Track.GetDirection(4);
  EnableDefaultStreams();
  DeleteHits(Hits);

  // (3, 4, 12)/13
  if ((Direction - MVector(3.0/13.0, 4.0/13.0, 12.0/13.0)).Mag() > 1e-12) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if removing a hit which is not in the track is survived and returns a nullptr
bool UTRETrack::RemoveUnknownHit()
{
  MRETrack Track;
  MREHit Hit;
  DisableDefaultStreams();
  MRESE* Removed = Track.RemoveRESE(&Hit);
  EnableDefaultStreams();

  if (Removed != nullptr) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the iterator refuses a start in the middle of the track and the program goes on
bool UTRETrack::ResetIteratorInTheMiddle()
{
  MRETrack Track;
  vector<MREHit*> Hits;
  FillReferenceTrack(Track, Hits);
  DisableDefaultStreams();
  bool IsReset = Track.ResetIterator(Hits[1]);
  EnableDefaultStreams();
  DeleteHits(Hits);

  if (IsReset == true) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the state of a new track
bool UTRETrack::TestDefaultConstruction()
{
  bool Passed = true;

  MRETrack Track;
  Passed = Evaluate("MRETrack()", "default", "The type is a track", Track.GetType(), MRESE::c_Track) && Passed;
  Passed = EvaluateTrue("MRETrack()", "default", "There is no start point", Track.GetStartPoint() == nullptr) && Passed;
  Passed = EvaluateTrue("MRETrack()", "default", "There is no stop point", Track.GetStopPoint() == nullptr) && Passed;
  Passed = Evaluate("MRETrack()", "default", "There are no end points", Track.GetNEndPoints(), 0) && Passed;
  Passed = Evaluate("MRETrack()", "default", "There are no hits", Track.GetNRESEs(), 0) && Passed;
  Passed = EvaluateNear("MRETrack()", "default", "The quality factor is zero", Track.GetQualityFactor(), 0.0, 0.0) && Passed;
  Passed = EvaluateFalse("MRETrack()", "default", "The direction is not fixed", Track.IsDirectionFixed()) && Passed;
  Passed = EvaluateNear("MRETrack()", "default", "The energy is zero", Track.GetEnergy(), 0.0, 0.0) && Passed;
  Passed = Evaluate("MRETrack()", "default", "There is no detector type", Track.GetDetector(), MDDetector::c_NoDetectorType) && Passed;
  Passed = EvaluateFalse("MRETrack()", "default", "A track without a start point is not valid", Track.IsValid()) && Passed;
  Passed = EvaluateTrue("MRETrack()", "default", "The track has its own volume sequence", Track.GetVolumeSequence() != nullptr) && Passed;
  Passed = EvaluateTrue("GetOtherEndPoint()", "no end points", "There is no other end point", Track.GetOtherEndPoint(nullptr) == nullptr) && Passed;

  // The scores of an empty track are zero:
  Passed = EvaluateNear("CalculateStraightnessScore()", "empty track", "The score is zero", Track.CalculateStraightnessScore(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("CalculateFirstLastHitScore()", "empty track", "The score is zero", Track.CalculateFirstLastHitScore(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("CalculatePearsonCorrelation()", "empty track", "The score is zero", Track.CalculatePearsonCorrelation(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("CalculateSpearmanRankCorrelation()", "empty track", "The score is zero", Track.CalculateSpearmanRankCorrelation(), 0.0, 0.0) && Passed;
  Passed = EvaluateTrue("CreateEndPoints()", "empty track", "The end points of an empty track can be created", Track.CreateEndPoints()) && Passed;
  Passed = Evaluate("CreateEndPoints()", "empty track", "There are still no end points", Track.GetNEndPoints(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding and removing hits and clusters
bool UTRETrack::TestAddAndRemoveRESE()
{
  bool Passed = true;

  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  A->SetEnergyResolution(3.0);
  MREHit* B = CreateHit(MVector(3.0, 0.0, 4.0), 200.0, MDDetector::c_Strip2D);
  B->SetEnergyResolution(4.0);
  MREHit* C = CreateHit(MVector(3.0, 4.0, 8.0), 50.0, MDDetector::c_Calorimeter);
  C->SetEnergyResolution(12.0);
  MRECluster* Cluster = new MRECluster();
  BeginCaptureOutput();
  Cluster->AddRESE(C);
  EndCaptureOutput();

  MRETrack Track;
  Track.AddRESE(A);
  Passed = Evaluate("AddRESE()", "first hit", "There is one hit", Track.GetNRESEs(), 1) && Passed;
  Passed = EvaluateNear("AddRESE()", "first hit", "The energy is the one of the hit", Track.GetEnergy(), 100.0, 1e-12) && Passed;
  Passed = Evaluate("AddRESE()", "first hit", "The detector type is the one of the hit", Track.GetDetector(), MDDetector::c_Strip2D) && Passed;
  Passed = EvaluateNear("AddRESE()", "first hit", "The energy resolution is the one of the hit", Track.GetEnergyResolution(), 3.0, 1e-12) && Passed;
  // The position is the one of the start point, which is not set yet:
  Passed = EvaluateVectorNear("AddRESE()", "first hit", "The position is not set without a start point", Track.GetPosition(), MVector(0, 0, 0), 0.0) && Passed;
  Passed = Evaluate("AddRESE()", "first hit", "Adding a hit creates no end point", Track.GetNEndPoints(), 0) && Passed;

  Track.AddRESE(B);
  Passed = Evaluate("AddRESE()", "second hit", "There are two hits", Track.GetNRESEs(), 2) && Passed;
  Passed = EvaluateNear("AddRESE()", "second hit", "The energy is the sum, 300 keV", Track.GetEnergy(), 300.0, 1e-12) && Passed;
  // sqrt(3^2 + 4^2) = 5
  Passed = EvaluateNear("AddRESE()", "second hit", "The energy resolution is the quadratic sum, 5 keV", Track.GetEnergyResolution(), 5.0, 1e-12) && Passed;

  // A cluster in another detector type - the detector type of a track is the one of its first hit
  Track.AddRESE(Cluster);
  Passed = Evaluate("AddRESE()", "cluster", "There are three elements", Track.GetNRESEs(), 3) && Passed;
  Passed = EvaluateNear("AddRESE()", "cluster", "The energy is the sum, 350 keV", Track.GetEnergy(), 350.0, 1e-12) && Passed;
  Passed = Evaluate("AddRESE()", "cluster", "The detector type is still the one of the first hit", Track.GetDetector(), MDDetector::c_Strip2D) && Passed;
  // sqrt(3^2 + 4^2 + 12^2) = 13
  Passed = EvaluateNear("AddRESE()", "cluster", "The energy resolution is the quadratic sum, 13 keV", Track.GetEnergyResolution(), 13.0, 1e-12) && Passed;

  // Defect REV-B-14: Tracks are rejected only by a massert, which is compiled out in the default build (-DNDEBUG), so after the error message the track is added
  MRETrack Other;
  DisableDefaultStreams();
  Track.AddRESE(&Other);
  EnableDefaultStreams();
  Passed = ExpectedToFail(Evaluate("AddRESE()", "track", "A track is not added to a track", Track.GetNRESEs(), 3)) && Passed;
  if (Track.ContainsRESE(&Other) == true) {
    Track.RemoveRESE(&Other);
    Track.CompressRESEs();
  }

  // Removing:
  MRESE* Removed = Track.RemoveRESE(B);
  Passed = EvaluateTrue("RemoveRESE()", "second hit", "The removed hit is returned", Removed == B) && Passed;
  Passed = EvaluateFalse("RemoveRESE()", "second hit", "The track no longer contains the hit", Track.ContainsRESE(B)) && Passed;
  Passed = EvaluateNear("RemoveRESE()", "second hit", "The energy is reduced to 150 keV", Track.GetEnergy(), 150.0, 1e-12) && Passed;
  // sqrt(3^2 + 12^2) = sqrt(153)
  Passed = EvaluateNear("RemoveRESE()", "second hit", "The energy resolution is the quadratic sum of the remaining elements", Track.GetEnergyResolution(), sqrt(153.0), 1e-12) && Passed;
  Track.CompressRESEs();
  Passed = Evaluate("CompressRESEs()", "after the removal", "There are two elements", Track.GetNRESEs(), 2) && Passed;

  // Defect REV-B-9: Removing a hit which is not in the track ends the program with Fatal() instead of returning an error
  Passed = ExpectedToCrash(EvaluateTrue("RemoveRESE()", "hit of another track", "A nullptr is returned and the program goes on", RemoveUnknownHit())) && Passed;

  // A track which may not be overwritten keeps its data:
  MRETrack Fixed;
  Fixed.SetEnergy(555.0);
  Fixed.SetEnergyResolution(11.0);
  Fixed.SetDetector(MDDetector::c_Strip3D);
  Fixed.AllowOverwrite(false);
  Fixed.AddRESE(A);
  Fixed.AddRESE(B);
  Passed = Evaluate("AddRESE()", "no overwrite", "The hits are added", Fixed.GetNRESEs(), 2) && Passed;
  Passed = EvaluateNear("AddRESE()", "no overwrite", "The energy is kept", Fixed.GetEnergy(), 555.0, 0.0) && Passed;
  Passed = EvaluateNear("AddRESE()", "no overwrite", "The energy resolution is kept", Fixed.GetEnergyResolution(), 11.0, 0.0) && Passed;
  Passed = Evaluate("AddRESE()", "no overwrite", "The detector type is kept", Fixed.GetDetector(), MDDetector::c_Strip3D) && Passed;
  Fixed.RemoveRESE(B);
  Passed = EvaluateNear("RemoveRESE()", "no overwrite", "The energy is kept", Fixed.GetEnergy(), 555.0, 0.0) && Passed;

  delete A;
  delete B;
  delete C;
  delete Cluster;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test linking the hits in their sequence
bool UTRETrack::TestLinkSequential()
{
  bool Passed = true;

  MRETrack Track;
  vector<MREHit*> Hits;
  FillReferenceTrack(Track, Hits);
  Hits[0]->SetPositionResolution(MVector(0.1, 0.2, 0.3));

  Passed = EvaluateTrue("LinkSequential()", "four hits", "The start point is the first hit", Track.GetStartPoint() == Hits[0]) && Passed;
  Passed = EvaluateTrue("LinkSequential()", "four hits", "The stop point is the last hit", Track.GetStopPoint() == Hits[3]) && Passed;
  Passed = Evaluate("LinkSequential()", "four hits", "The first hit has one link", Hits[0]->GetNLinks(), 1) && Passed;
  Passed = Evaluate("LinkSequential()", "four hits", "The second hit has two links", Hits[1]->GetNLinks(), 2) && Passed;
  Passed = Evaluate("LinkSequential()", "four hits", "The third hit has two links", Hits[2]->GetNLinks(), 2) && Passed;
  Passed = Evaluate("LinkSequential()", "four hits", "The last hit has one link", Hits[3]->GetNLinks(), 1) && Passed;
  Passed = EvaluateTrue("LinkSequential()", "four hits", "The first hit is linked to the second", Hits[0]->GetLinkAt(0) == Hits[1]) && Passed;
  Passed = EvaluateTrue("LinkSequential()", "four hits", "The first link of the second hit is the third hit", Hits[1]->GetLinkAt(0) == Hits[2]) && Passed;
  Passed = EvaluateTrue("LinkSequential()", "four hits", "The second link of the second hit is the first hit", Hits[1]->GetLinkAt(1) == Hits[0]) && Passed;
  Passed = EvaluateTrue("LinkSequential()", "four hits", "The last hit is linked to the third", Hits[3]->GetLinkAt(0) == Hits[2]) && Passed;
  Passed = Evaluate("LinkSequential()", "four hits", "There are two end points", Track.GetNEndPoints(), 2) && Passed;
  if (Track.GetNEndPoints() == 2) {
    Passed = EvaluateTrue("LinkSequential()", "four hits", "The end points are the first and the last hit", Track.GetEndPointAt(0) == Hits[0] && Track.GetEndPointAt(1) == Hits[3]) && Passed;
  }
  Passed = EvaluateVectorNear("LinkSequential()", "four hits", "The position of the track is the one of the start point", Track.GetPosition(), MVector(0.0, 0.0, 0.0), 0.0) && Passed;
  Passed = EvaluateTrue("LinkSequential()", "four hits", "The track is valid", Track.IsValid()) && Passed;
  Passed = EvaluateNear("LinkSequential()", "four hits", "The energy is the sum, 1000 keV", Track.GetEnergy(), 1000.0, 1e-12) && Passed;
  DeleteHits(Hits);

  // Two hits:
  MRETrack Two;
  FillTrack(Two, Hits, { MVector(1.0, 2.0, 3.0), MVector(1.0, 2.0, 4.0) }, { 10.0, 20.0 });
  Passed = EvaluateTrue("LinkSequential()", "two hits", "Start and stop point are the two hits", Two.GetStartPoint() == Hits[0] && Two.GetStopPoint() == Hits[1]) && Passed;
  Passed = Evaluate("LinkSequential()", "two hits", "Both hits have one link", Hits[0]->GetNLinks() + Hits[1]->GetNLinks(), 2) && Passed;
  Passed = Evaluate("LinkSequential()", "two hits", "There are two end points", Two.GetNEndPoints(), 2) && Passed;
  Passed = EvaluateVectorNear("LinkSequential()", "two hits", "The position of the track is the one of the start point", Two.GetPosition(), MVector(1.0, 2.0, 3.0), 0.0) && Passed;
  DeleteHits(Hits);

  // A single hit cannot be linked: nothing happens
  MRETrack One;
  FillTrack(One, Hits, { MVector(1.0, 2.0, 3.0) }, { 10.0 });
  Passed = EvaluateTrue("LinkSequential()", "one hit", "There is no start point", One.GetStartPoint() == nullptr) && Passed;
  Passed = Evaluate("LinkSequential()", "one hit", "The hit has no links", Hits[0]->GetNLinks(), 0) && Passed;
  Passed = Evaluate("LinkSequential()", "one hit", "There are no end points", One.GetNEndPoints(), 0) && Passed;
  DeleteHits(Hits);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the start and the stop point
bool UTRETrack::TestStartAndStopPoint()
{
  bool Passed = true;

  MREHit* A = CreateHit(MVector(1.0, 2.0, 3.0), 100.0, MDDetector::c_Strip2D);
  A->SetPositionResolution(MVector(0.1, 0.2, 0.3));
  MREHit* B = CreateHit(MVector(1.0, 2.0, 4.0), 200.0, MDDetector::c_Strip2D);
  B->SetPositionResolution(MVector(0.4, 0.5, 0.6));
  MREHit* C = CreateHit(MVector(1.0, 5.0, 8.0), 300.0, MDDetector::c_Strip2D);

  // A start point without end points: the stop point stays open
  MRETrack Track;
  Track.AddRESE(A);
  Track.SetStartPoint(A);
  Passed = EvaluateTrue("SetStartPoint()", "single hit", "The start point is the hit", Track.GetStartPoint() == A) && Passed;
  Passed = EvaluateTrue("SetStartPoint()", "single hit", "There is no stop point", Track.GetStopPoint() == nullptr) && Passed;
  Passed = EvaluateVectorNear("SetStartPoint()", "single hit", "The position of the track is the one of the start point", Track.GetPosition(), MVector(1.0, 2.0, 3.0), 0.0) && Passed;
  Passed = EvaluateVectorNear("SetStartPoint()", "single hit", "The position resolution of the track is the one of the start point", Track.GetPositionResolution(), MVector(0.1, 0.2, 0.3), 0.0) && Passed;
  Passed = EvaluateTrue("SetStartPoint()", "single hit", "The track is valid", Track.IsValid()) && Passed;

  // A stop point for a track with only a start point: the new hit is added and linked to the start point
  Track.SetStopPoint(B);
  Passed = EvaluateTrue("SetStopPoint()", "track with a start point", "The stop point is the new hit", Track.GetStopPoint() == B) && Passed;
  Passed = Evaluate("SetStopPoint()", "track with a start point", "The new hit is added to the track", Track.GetNRESEs(), 2) && Passed;
  Passed = EvaluateTrue("SetStopPoint()", "track with a start point", "The start point is linked to the stop point", A->GetNLinks() == 1 && A->GetLinkAt(0) == B) && Passed;
  Passed = EvaluateTrue("SetStopPoint()", "track with a start point", "The stop point is linked to the start point", B->GetNLinks() == 1 && B->GetLinkAt(0) == A) && Passed;
  Passed = EvaluateNear("SetStopPoint()", "track with a start point", "The energy is the sum, 300 keV", Track.GetEnergy(), 300.0, 1e-12) && Passed;

  // A new stop point for a track with a stop point: it is linked to the old stop point
  MREHit Stale;
  C->AddLink(&Stale);
  Track.SetStopPoint(C);
  Passed = EvaluateTrue("SetStopPoint()", "track with a stop point", "The stop point is the new hit", Track.GetStopPoint() == C) && Passed;
  Passed = Evaluate("SetStopPoint()", "track with a stop point", "The new hit is added to the track", Track.GetNRESEs(), 3) && Passed;
  Passed = EvaluateTrue("SetStopPoint()", "track with a stop point", "The old links of the new stop point are removed", C->GetNLinks() == 1 && C->GetLinkAt(0) == B) && Passed;
  Passed = EvaluateTrue("SetStopPoint()", "track with a stop point", "The old stop point is linked to the start point and to the new stop point", B->GetNLinks() == 2 && B->GetLinkAt(0) == A && B->GetLinkAt(1) == C) && Passed;
  Passed = EvaluateTrue("SetStopPoint()", "track with a stop point", "The start point is unchanged", Track.GetStartPoint() == A) && Passed;
  Passed = EvaluateNear("SetStopPoint()", "track with a stop point", "The energy is the sum, 600 keV", Track.GetEnergy(), 600.0, 1e-12) && Passed;
  // (1, 5, 8) - (1, 2, 4) = (0, 3, 4), length 5
  Passed = EvaluateVectorNear("GetFinalDirection()", "after SetStopPoint()", "The final direction points from the old to the new stop point", Track.GetFinalDirection(), MVector(0.0, 0.6, 0.8), 1e-12) && Passed;

  // A stop point which is already in the track is not added again:
  MRETrack Second;
  MREHit* D = CreateHit(MVector(0.0, 0.0, 0.0), 10.0, MDDetector::c_Strip2D);
  MREHit* E = CreateHit(MVector(0.0, 0.0, 1.0), 20.0, MDDetector::c_Strip2D);
  Second.AddRESE(D);
  Second.AddRESE(E);
  Second.SetStartPoint(D);
  Second.SetStopPoint(E);
  Passed = Evaluate("SetStopPoint()", "hit of the track", "The hit is not added a second time", Second.GetNRESEs(), 2) && Passed;
  Passed = EvaluateNear("SetStopPoint()", "hit of the track", "The energy is not counted twice", Second.GetEnergy(), 30.0, 1e-12) && Passed;
  Passed = EvaluateTrue("SetStopPoint()", "hit of the track", "The stop point is the hit", Second.GetStopPoint() == E) && Passed;

  // A start point which is an end point makes the other end point the stop point:
  MRETrack Third;
  vector<MREHit*> Hits;
  FillReferenceTrack(Third, Hits);
  Third.SetStartPoint(Hits[3]);
  Passed = EvaluateTrue("SetStartPoint()", "last hit of a linked track", "The start point is the last hit", Third.GetStartPoint() == Hits[3]) && Passed;
  Passed = EvaluateTrue("SetStartPoint()", "last hit of a linked track", "The stop point is the other end point, the first hit", Third.GetStopPoint() == Hits[0]) && Passed;
  Passed = EvaluateVectorNear("SetStartPoint()", "last hit of a linked track", "The position of the track is the one of the new start point", Third.GetPosition(), MVector(3.0, 4.0, 12.0), 0.0) && Passed;
  // (3, 4, 8) - (3, 4, 12) = (0, 0, -4)
  Passed = EvaluateVectorNear("GetDirection()", "reversed track", "The direction follows the new start point", Third.GetDirection(), MVector(0.0, 0.0, -1.0), 1e-12) && Passed;
  DeleteHits(Hits);

  delete A;
  delete B;
  delete C;
  delete D;
  delete E;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the end points
bool UTRETrack::TestEndPoints()
{
  bool Passed = true;

  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  MREHit* B = CreateHit(MVector(0.0, 0.0, 1.0), 200.0, MDDetector::c_Strip2D);
  MREHit* C = CreateHit(MVector(0.0, 0.0, 2.0), 300.0, MDDetector::c_Strip2D);
  MREHit* D = CreateHit(MVector(0.0, 0.0, -1.0), 400.0, MDDetector::c_Strip2D);

  // Only the list of end points:
  MRETrack List;
  List.AddEndPoint(A);
  Passed = Evaluate("AddEndPoint(MRESE*)", "one hit", "There is one end point", List.GetNEndPoints(), 1) && Passed;
  Passed = EvaluateTrue("AddEndPoint(MRESE*)", "one hit", "The end point is the hit", List.GetEndPointAt(0) == A) && Passed;
  Passed = Evaluate("AddEndPoint(MRESE*)", "one hit", "The hit is not added to the track", List.GetNRESEs(), 0) && Passed;
  Passed = Evaluate("AddEndPoint(MRESE*)", "one hit", "The hit is not linked", A->GetNLinks(), 0) && Passed;
  Passed = EvaluateTrue("GetOtherEndPoint()", "one end point", "There is no other end point", List.GetOtherEndPoint(A) == nullptr) && Passed;
  List.AddEndPoint(B);
  Passed = EvaluateTrue("GetOtherEndPoint()", "two end points", "The other end point of the first is the second", List.GetOtherEndPoint(A) == B) && Passed;
  Passed = EvaluateTrue("GetOtherEndPoint()", "two end points", "The other end point of the second is the first", List.GetOtherEndPoint(B) == A) && Passed;
  Passed = EvaluateTrue("GetOtherEndPoint()", "two end points", "A hit which is no end point has no other end point", List.GetOtherEndPoint(C) == nullptr) && Passed;
  List.RemoveEndPoint(A);
  Passed = Evaluate("RemoveEndPoint()", "first of two", "There is one end point", List.GetNEndPoints(), 1) && Passed;
  Passed = EvaluateTrue("RemoveEndPoint()", "first of two", "The remaining end point is the second one", List.GetEndPointAt(0) == B) && Passed;
  List.AddEndPoint(C);
  List.RemoveAllEndPoints();
  Passed = Evaluate("RemoveAllEndPoints()", "two end points", "There are no end points", List.GetNEndPoints(), 0) && Passed;

  // Growing a track at its end points:
  MRETrack Track;
  Track.AddEndPoint(A, nullptr);
  Passed = Evaluate("AddEndPoint(MRESE*, MRESE*)", "first hit", "There is one end point", Track.GetNEndPoints(), 1) && Passed;
  Passed = Evaluate("AddEndPoint(MRESE*, MRESE*)", "first hit", "The hit is added to the track", Track.GetNRESEs(), 1) && Passed;
  Passed = EvaluateNear("AddEndPoint(MRESE*, MRESE*)", "first hit", "The energy is the one of the hit", Track.GetEnergy(), 100.0, 1e-12) && Passed;

  Track.AddEndPoint(B, A);
  Passed = Evaluate("AddEndPoint(MRESE*, MRESE*)", "second hit", "There are two end points", Track.GetNEndPoints(), 2) && Passed;
  Passed = Evaluate("AddEndPoint(MRESE*, MRESE*)", "second hit", "The hit is added to the track", Track.GetNRESEs(), 2) && Passed;
  Passed = EvaluateTrue("AddEndPoint(MRESE*, MRESE*)", "second hit", "The hits are linked to each other", A->GetNLinks() == 1 && A->GetLinkAt(0) == B && B->GetNLinks() == 1 && B->GetLinkAt(0) == A) && Passed;

  Track.AddEndPoint(C, B);
  Passed = Evaluate("AddEndPoint(MRESE*, MRESE*)", "third hit at the second", "There are still two end points", Track.GetNEndPoints(), 2) && Passed;
  Passed = EvaluateTrue("AddEndPoint(MRESE*, MRESE*)", "third hit at the second", "The end points are the first and the third hit", Track.GetOtherEndPoint(A) == C && Track.GetOtherEndPoint(C) == A) && Passed;
  Passed = Evaluate("AddEndPoint(MRESE*, MRESE*)", "third hit at the second", "The second hit has two links", B->GetNLinks(), 2) && Passed;
  Passed = Evaluate("AddEndPoint(MRESE*, MRESE*)", "third hit at the second", "There are three hits", Track.GetNRESEs(), 3) && Passed;

  // At the other end:
  Track.AddEndPoint(D, A);
  Passed = EvaluateTrue("AddEndPoint(MRESE*, MRESE*)", "fourth hit at the first", "The end points are the third and the fourth hit", Track.GetOtherEndPoint(D) == C && Track.GetOtherEndPoint(C) == D) && Passed;
  Passed = EvaluateTrue("AddEndPoint(MRESE*, MRESE*)", "fourth hit at the first", "The first hit is linked to the second and the fourth", A->GetNLinks() == 2 && A->GetLinkAt(0) == B && A->GetLinkAt(1) == D) && Passed;
  Passed = EvaluateNear("AddEndPoint(MRESE*, MRESE*)", "fourth hit at the first", "The energy is the sum, 1000 keV", Track.GetEnergy(), 1000.0, 1e-12) && Passed;
  Passed = EvaluateTrue("AddEndPoint(MRESE*, MRESE*)", "fourth hit at the first", "Adding end points sets no start point", Track.GetStartPoint() == nullptr) && Passed;

  // The direction of the end points points out of the track: D at z = -1 is linked to A at z = 0, C at z = 2 to B at z = 1
  Passed = EvaluateVectorNear("GetDirectionOfEndPoint()", "lower end point", "The direction points down, away from the track", Track.GetDirectionOfEndPoint(D), MVector(0.0, 0.0, -1.0), 1e-12) && Passed;
  Passed = EvaluateVectorNear("GetDirectionOfEndPoint()", "upper end point", "The direction points up, away from the track", Track.GetDirectionOfEndPoint(C), MVector(0.0, 0.0, 1.0), 1e-12) && Passed;

  delete A;
  delete B;
  delete C;
  delete D;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test appending a track
bool UTRETrack::TestAppendTrack()
{
  bool Passed = true;

  // The first track from z = 0 to z = 1, the second from z = 2 to z = 3
  MRETrack First;
  vector<MREHit*> FirstHits;
  FillTrack(First, FirstHits, { MVector(0.0, 0.0, 0.0), MVector(0.0, 0.0, 1.0) }, { 100.0, 200.0 });
  MRETrack Second;
  vector<MREHit*> SecondHits;
  FillTrack(Second, SecondHits, { MVector(0.0, 0.0, 2.0), MVector(0.0, 0.0, 3.0) }, { 300.0, 400.0 });

  First.AddEndPoint(&Second, SecondHits[0], FirstHits[1]);
  Passed = Evaluate("AddEndPoint(MRETrack*, ...)", "two tracks of two hits", "The first track has four hits", First.GetNRESEs(), 4) && Passed;
  Passed = EvaluateNear("AddEndPoint(MRETrack*, ...)", "two tracks of two hits", "The energy of the first track is the sum, 1000 keV", First.GetEnergy(), 1000.0, 1e-12) && Passed;
  Passed = EvaluateTrue("AddEndPoint(MRETrack*, ...)", "two tracks of two hits", "The connected end points are linked", FirstHits[1]->GetNLinks() == 2 && FirstHits[1]->GetLinkAt(1) == SecondHits[0] && SecondHits[0]->GetNLinks() == 2 && SecondHits[0]->GetLinkAt(1) == FirstHits[1]) && Passed;
  Passed = Evaluate("AddEndPoint(MRETrack*, ...)", "two tracks of two hits", "The first track has two end points", First.GetNEndPoints(), 2) && Passed;
  Passed = EvaluateTrue("AddEndPoint(MRETrack*, ...)", "two tracks of two hits", "The end points are the outer hits of both tracks", First.GetOtherEndPoint(FirstHits[0]) == SecondHits[1]) && Passed;
  Passed = EvaluateTrue("AddEndPoint(MRETrack*, ...)", "two tracks of two hits", "The hits are removed from the second track", Second.ContainsRESE(SecondHits[0]) == false && Second.ContainsRESE(SecondHits[1]) == false) && Passed;
  Passed = EvaluateNear("AddEndPoint(MRETrack*, ...)", "two tracks of two hits", "The second track has no energy left", Second.GetEnergy(), 0.0, 1e-12) && Passed;

  // The whole track can be walked through from its start point:
  First.ResetIterator();
  vector<MRESE*> Sequence;
  MRESE* RESE = nullptr;
  while ((RESE = First.GetNextRESE()) != nullptr && Sequence.size() < 10) {
    Sequence.push_back(RESE);
  }
  Passed = EvaluateSize("GetNextRESE()", "appended track", "The iteration finds four hits", Sequence.size(), 4) && Passed;
  if (Sequence.size() == 4) {
    Passed = EvaluateTrue("GetNextRESE()", "appended track", "The hits come in the sequence of the links", Sequence[0] == FirstHits[0] && Sequence[1] == FirstHits[1] && Sequence[2] == SecondHits[0] && Sequence[3] == SecondHits[1]) && Passed;
  }

  // A track with a single hit and one end point:
  MRETrack Single;
  MREHit* Last = CreateHit(MVector(0.0, 0.0, 4.0), 500.0, MDDetector::c_Strip2D);
  Single.AddEndPoint(Last, nullptr);
  First.AddEndPoint(&Single, Last, SecondHits[1]);
  Passed = Evaluate("AddEndPoint(MRETrack*, ...)", "track of one hit", "The first track has five hits", First.GetNRESEs(), 5) && Passed;
  Passed = EvaluateTrue("AddEndPoint(MRETrack*, ...)", "track of one hit", "The end points are the first hit and the new hit", First.GetNEndPoints() == 2 && First.GetOtherEndPoint(FirstHits[0]) == Last) && Passed;
  Passed = EvaluateNear("AddEndPoint(MRETrack*, ...)", "track of one hit", "The energy is the sum, 1500 keV", First.GetEnergy(), 1500.0, 1e-12) && Passed;

  DeleteHits(FirstHits);
  DeleteHits(SecondHits);
  delete Last;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the creation of the end points from the links
bool UTRETrack::TestCreateEndPoints()
{
  bool Passed = true;

  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  MREHit* B = CreateHit(MVector(0.0, 0.0, 1.0), 200.0, MDDetector::c_Strip2D);
  MREHit* C = CreateHit(MVector(0.0, 0.0, 2.0), 300.0, MDDetector::c_Strip2D);

  // A single hit is its own end point:
  MRETrack Single;
  Single.AddRESE(A);
  Passed = EvaluateTrue("CreateEndPoints()", "one hit", "The end point is created", Single.CreateEndPoints()) && Passed;
  Passed = EvaluateTrue("CreateEndPoints()", "one hit", "The hit is the only end point", Single.GetNEndPoints() == 1 && Single.GetEndPointAt(0) == A) && Passed;
  // A second call does not double it:
  Single.CreateEndPoints();
  Passed = Evaluate("CreateEndPoints()", "one hit, second call", "There is still one end point", Single.GetNEndPoints(), 1) && Passed;

  // A chain: the hits with one link are the end points
  MRETrack Chain;
  Chain.AddRESE(B);
  Chain.AddRESE(A);
  Chain.AddRESE(C);
  A->AddLink(B);
  B->AddLink(A);
  B->AddLink(C);
  C->AddLink(B);
  Chain.AddEndPoint(B);
  Passed = EvaluateTrue("CreateEndPoints()", "chain of three hits", "The end points are created", Chain.CreateEndPoints()) && Passed;
  Passed = Evaluate("CreateEndPoints()", "chain of three hits", "There are two end points, the old ones are removed", Chain.GetNEndPoints(), 2) && Passed;
  if (Chain.GetNEndPoints() == 2) {
    Passed = EvaluateTrue("CreateEndPoints()", "chain of three hits", "The end points are the hits with one link", Chain.GetEndPointAt(0) == A && Chain.GetEndPointAt(1) == C) && Passed;
  }

  // A hit without links in a track of several hits: no end points
  B->RemoveLink(C);
  C->RemoveAllLinks();
  BeginCaptureOutput();
  bool Created = Chain.CreateEndPoints();
  MString Output = EndCaptureOutput();
  Passed = EvaluateFalse("CreateEndPoints()", "hit without links", "The end points cannot be created", Created) && Passed;
  Passed = Evaluate("CreateEndPoints()", "hit without links", "There are no end points", Chain.GetNEndPoints(), 0) && Passed;
  // Defect REV-B-15: CreateEndPoints reports a wrongly linked hit on the standard output and not on the MEGAlib streams
  Passed = ExpectedToFail(EvaluateTrue("CreateEndPoints()", "hit without links", "Nothing is printed to the standard output", Output.IsEmpty())) && Passed;

  delete A;
  delete B;
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the direction at the start of the track
bool UTRETrack::TestGetDirection()
{
  bool Passed = true;

  MRETrack Track;
  vector<MREHit*> Hits;
  FillReferenceTrack(Track, Hits);

  // First step: (3, 0, 4) - (0, 0, 0), length 5
  Passed = EvaluateVectorNear("GetDirection()", "default length", "The direction points from the start point to the second hit", Track.GetDirection(), MVector(0.6, 0.0, 0.8), 1e-12) && Passed;
  Passed = EvaluateVectorNear("GetDirection()", "length 1", "Length 1 is the default", Track.GetDirection(1), MVector(0.6, 0.0, 0.8), 1e-12) && Passed;
  Passed = EvaluateVectorNear("GetDirection()", "length 0", "A length below 1 is taken as 1", Track.GetDirection(0), MVector(0.6, 0.0, 0.8), 1e-12) && Passed;
  Passed = EvaluateNear("GetDirection()", "default length", "The direction is a unit vector", Track.GetDirection().Mag(), 1.0, 1e-12) && Passed;
  // Two steps: (3, 4, 8) - (0, 0, 0), length sqrt(89)
  Passed = EvaluateVectorNear("GetDirection()", "length 2", "The direction points from the start point to the third hit", Track.GetDirection(2), MVector(3.0/sqrt(89.0), 4.0/sqrt(89.0), 8.0/sqrt(89.0)), 1e-12) && Passed;
  // Three steps: (3, 4, 12) - (0, 0, 0), length 13
  Passed = EvaluateVectorNear("GetDirection()", "length 3", "The direction points from the start point to the last hit", Track.GetDirection(3), MVector(3.0/13.0, 4.0/13.0, 12.0/13.0), 1e-12) && Passed;
  // More steps than the track has: the last hit
  Passed = EvaluateVectorNear("GetDirection()", "length 5", "A length beyond the track uses the last hit", Track.GetDirection(5), MVector(3.0/13.0, 4.0/13.0, 12.0/13.0), 1e-12) && Passed;
  // Defect REV-B-16: A length equal to the number of hits is not caught by the check for the end of the track, and the hit beyond the last one is used
  Passed = ExpectedToCrash(EvaluateTrue("GetDirection()", "length 4, four hits", "A length of the number of hits uses the last hit", DirectionWithFullLength())) && Passed;

  // Without a start point there is no direction:
  MRETrack NoStart;
  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  MREHit* B = CreateHit(MVector(0.0, 0.0, 1.0), 100.0, MDDetector::c_Strip2D);
  NoStart.AddRESE(A);
  NoStart.AddRESE(B);
  DisableDefaultStreams();
  MVector Direction = NoStart.GetDirection();
  EnableDefaultStreams();
  Passed = EvaluateVectorNear("GetDirection()", "no start point", "The direction is the null vector", Direction, MVector(0, 0, 0), 0.0) && Passed;
  delete A;
  delete B;

  // Defect REV-B-14: A track with fewer than two hits is caught only by a massert, which is compiled out in the default build
  Passed = ExpectedToCrash(EvaluateTrue("GetDirection()", "track with one hit", "The direction is the null vector", DirectionOfSingleHit())) && Passed;

  DeleteHits(Hits);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the fixed direction
bool UTRETrack::TestFixedDirection()
{
  bool Passed = true;

  MRETrack Track;
  vector<MREHit*> Hits;
  FillReferenceTrack(Track, Hits);

  Track.SetFixedDirection(MVector(0.0, 1.0, 0.0));
  Passed = EvaluateTrue("SetFixedDirection()", "(0, 1, 0)", "The direction is fixed", Track.IsDirectionFixed()) && Passed;
  Passed = EvaluateVectorNear("GetDirection()", "fixed direction", "The fixed direction is returned", Track.GetDirection(), MVector(0.0, 1.0, 0.0), 0.0) && Passed;
  Passed = EvaluateVectorNear("GetDirection()", "fixed direction, length 3", "The length is ignored", Track.GetDirection(3), MVector(0.0, 1.0, 0.0), 0.0) && Passed;
  // The other directions do not use the fixed direction, see the header: (3, 4, 12) - (3, 4, 8)
  Passed = EvaluateVectorNear("GetFinalDirection()", "fixed direction", "The final direction is not changed", Track.GetFinalDirection(), MVector(0.0, 0.0, 1.0), 1e-12) && Passed;
  Passed = EvaluateVectorNear("GetDirectionOfEndPoint()", "fixed direction", "The direction of the end point is not changed", Track.GetDirectionOfEndPoint(Hits[0]), MVector(-0.6, 0.0, -0.8), 1e-12) && Passed;

  // A track without hits returns its fixed direction too:
  MRETrack Empty;
  Empty.SetFixedDirection(MVector(1.0, 0.0, 0.0));
  Passed = EvaluateVectorNear("GetDirection()", "fixed direction, no hits", "The fixed direction is returned", Empty.GetDirection(), MVector(1.0, 0.0, 0.0), 0.0) && Passed;

  // The copy keeps it:
  MRETrack* Duplicate = Track.Duplicate();
  Passed = EvaluateTrue("Duplicate()", "fixed direction", "The direction of the duplicate is fixed", Duplicate->IsDirectionFixed()) && Passed;
  Passed = EvaluateVectorNear("Duplicate()", "fixed direction", "The fixed direction is copied", Duplicate->GetDirection(), MVector(0.0, 1.0, 0.0), 0.0) && Passed;
  Duplicate->DeleteAll();
  delete Duplicate;

  Track.RemoveFixedDirection();
  Passed = EvaluateFalse("RemoveFixedDirection()", "fixed direction", "The direction is no longer fixed", Track.IsDirectionFixed()) && Passed;
  Passed = EvaluateVectorNear("GetDirection()", "fixed direction removed", "The direction is calculated from the hits again", Track.GetDirection(), MVector(0.6, 0.0, 0.8), 1e-12) && Passed;

  DeleteHits(Hits);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the direction at the end of the track and at its end points
bool UTRETrack::TestFinalAndEndPointDirections()
{
  bool Passed = true;

  MRETrack Track;
  vector<MREHit*> Hits;
  FillReferenceTrack(Track, Hits);

  // Last step: (3, 4, 12) - (3, 4, 8)
  Passed = EvaluateVectorNear("GetFinalDirection()", "default length", "The direction points from the third hit to the stop point", Track.GetFinalDirection(), MVector(0.0, 0.0, 1.0), 1e-12) && Passed;
  // Two steps back: (3, 4, 12) - (3, 0, 4) = (0, 4, 8), length sqrt(80)
  // Defect REV-B-17: For a length above 1 the final direction points backwards, from the stop point into the track - the code says itself that it was never tested
  DisableDefaultStreams();
  MVector FinalTwo = Track.GetFinalDirection(2);
  EnableDefaultStreams();
  Passed = ExpectedToFail(EvaluateVectorNear("GetFinalDirection()", "length 2", "The direction points from the second hit to the stop point", FinalTwo, MVector(0.0, 4.0/sqrt(80.0), 8.0/sqrt(80.0)), 1e-12)) && Passed;

  // The end points: the direction points out of the track
  // (0, 0, 0) - (3, 0, 4), length 5
  Passed = EvaluateVectorNear("GetDirectionOfEndPoint()", "start point", "The direction points from the second hit to the start point", Track.GetDirectionOfEndPoint(Hits[0]), MVector(-0.6, 0.0, -0.8), 1e-12) && Passed;
  Passed = EvaluateVectorNear("GetDirectionOfEndPoint()", "stop point", "The direction points from the third hit to the stop point", Track.GetDirectionOfEndPoint(Hits[3]), MVector(0.0, 0.0, 1.0), 1e-12) && Passed;
  // (0, 0, 0) - (3, 4, 8), length sqrt(89)
  Passed = EvaluateVectorNear("GetDirectionOfEndPoint()", "start point, length 2", "The direction points from the third hit to the start point", Track.GetDirectionOfEndPoint(Hits[0], 2), MVector(-3.0/sqrt(89.0), -4.0/sqrt(89.0), -8.0/sqrt(89.0)), 1e-12) && Passed;
  // (3, 4, 12) - (3, 0, 4) = (0, 4, 8), length sqrt(80)
  Passed = EvaluateVectorNear("GetDirectionOfEndPoint()", "stop point, length 2", "The direction points from the second hit to the stop point", Track.GetDirectionOfEndPoint(Hits[3], 2), MVector(0.0, 4.0/sqrt(80.0), 8.0/sqrt(80.0)), 1e-12) && Passed;
  // (3, 4, 12) - (0, 0, 0), length 13
  Passed = EvaluateVectorNear("GetDirectionOfEndPoint()", "stop point, length 7", "A length beyond the track uses the other end", Track.GetDirectionOfEndPoint(Hits[3], 7), MVector(3.0/13.0, 4.0/13.0, 12.0/13.0), 1e-12) && Passed;

  // Without a stop point there is no final direction:
  MRETrack NoStop;
  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  NoStop.AddRESE(A);
  NoStop.SetStartPoint(A);
  DisableDefaultStreams();
  MVector Direction = NoStop.GetFinalDirection();
  EnableDefaultStreams();
  Passed = EvaluateVectorNear("GetFinalDirection()", "no stop point", "The direction is the null vector", Direction, MVector(0, 0, 0), 0.0) && Passed;
  delete A;

  // A stop point which is not at the end of the links has no final direction:
  Track.SetStartPoint(Hits[0]);
  MRETrack Middle;
  vector<MREHit*> MiddleHits;
  FillReferenceTrack(Middle, MiddleHits);
  Middle.RemoveAllEndPoints();
  Middle.AddEndPoint(MiddleHits[0]);
  Middle.AddEndPoint(MiddleHits[1]);
  Middle.SetStartPoint(MiddleHits[0]);
  Passed = EvaluateTrue("SetStartPoint()", "end points first and second hit", "The stop point is the second hit", Middle.GetStopPoint() == MiddleHits[1]) && Passed;
  DisableDefaultStreams();
  Direction = Middle.GetFinalDirection();
  EnableDefaultStreams();
  Passed = EvaluateVectorNear("GetFinalDirection()", "stop point with two links", "The direction is the null vector", Direction, MVector(0, 0, 0), 0.0) && Passed;

  DeleteHits(Hits);
  DeleteHits(MiddleHits);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the average direction and the center of gravity
bool UTRETrack::TestAverageDirectionAndCenterOfGravity()
{
  bool Passed = true;

  // Hits on a straight line along (1, 2, 4): the fit gives the slopes dx/dz = 0.25 and dy/dz = 0.5
  {
    MRETrack Track;
    vector<MREHit*> Hits;
    FillTrack(Track, Hits, { MVector(0.0, 0.0, 0.0), MVector(1.0, 2.0, 4.0), MVector(2.0, 4.0, 8.0), MVector(4.0, 8.0, 16.0) }, { 100.0, 100.0, 100.0, 100.0 });
    // Intended: the vector is not normalized, its z component is 1
    Passed = EvaluateVectorNear("GetAverageDirection()", "straight line along (1, 2, 4)", "The direction is (0.25, 0.5, 1)", Track.GetAverageDirection(), MVector(0.25, 0.5, 1.0), 1e-12) && Passed;
    DeleteHits(Hits);
  }

  // The reference track: n = 4, sum z = 24, sum z^2 = 224, D = 4*224 - 24^2 = 320
  // x: sum x = 9, sum xz = 72, slope (4*72 - 24*9)/320 = 0.225; y: sum y = 8, sum yz = 80, slope (4*80 - 24*8)/320 = 0.4
  {
    MRETrack Track;
    vector<MREHit*> Hits;
    FillReferenceTrack(Track, Hits);
    Passed = EvaluateVectorNear("GetAverageDirection()", "reference track", "The direction is (0.225, 0.4, 1)", Track.GetAverageDirection(), MVector(0.225, 0.4, 1.0), 1e-12) && Passed;

    // Center of gravity: (100*(0, 0, 0) + 200*(3, 0, 4) + 300*(3, 4, 8) + 400*(3, 4, 12))/1000 = (2.7, 2.8, 8)
    Passed = EvaluateVectorNear("GetCenterOfGravity()", "reference track", "The center of gravity is the energy weighted mean (2.7, 2.8, 8)", Track.GetCenterOfGravity(), MVector(2.7, 2.8, 8.0), 1e-12) && Passed;
    DeleteHits(Hits);
  }

  // All hits in one plane of constant z: there is no slope, the direction is the one from the start to the stop point
  {
    MRETrack Track;
    vector<MREHit*> Hits;
    FillTrack(Track, Hits, { MVector(0.0, 0.0, 5.0), MVector(1.0, 2.0, 5.0), MVector(3.0, 4.0, 5.0) }, { 100.0, 100.0, 100.0 });
    // Defect REV-B-18: For hits at the same z the function returns the difference of the pointers to the stop and the start point as a vector, not the difference of their positions
    Passed = ExpectedToFail(EvaluateVectorNear("GetAverageDirection()", "all hits at z = 5", "The direction points from the start to the stop point", Track.GetAverageDirection(), MVector(3.0, 4.0, 0.0), 1e-12)) && Passed;
    DeleteHits(Hits);
  }

  // Without a start or a stop point there is no direction:
  {
    MRETrack Track;
    MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
    MREHit* B = CreateHit(MVector(0.0, 0.0, 1.0), 300.0, MDDetector::c_Strip2D);
    Track.AddRESE(A);
    Track.AddRESE(B);
    DisableDefaultStreams();
    MVector Direction = Track.GetAverageDirection();
    EnableDefaultStreams();
    Passed = EvaluateVectorNear("GetAverageDirection()", "no start point", "The direction is the null vector", Direction, MVector(0, 0, 0), 0.0) && Passed;
    Track.SetStartPoint(A);
    DisableDefaultStreams();
    Direction = Track.GetAverageDirection();
    EnableDefaultStreams();
    Passed = EvaluateVectorNear("GetAverageDirection()", "no stop point", "The direction is the null vector", Direction, MVector(0, 0, 0), 0.0) && Passed;

    // The center of gravity needs neither: (100*0 + 300*1)/400 = 0.75
    Passed = EvaluateVectorNear("GetCenterOfGravity()", "two hits, no links", "The center of gravity is (0, 0, 0.75)", Track.GetCenterOfGravity(), MVector(0.0, 0.0, 0.75), 1e-12) && Passed;
    delete A;
    delete B;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the iterator
bool UTRETrack::TestIterator()
{
  bool Passed = true;

  MRETrack Track;
  vector<MREHit*> Hits;
  FillReferenceTrack(Track, Hits);

  // From the start point:
  Passed = EvaluateTrue("ResetIterator()", "start point", "The iterator is reset", Track.ResetIterator()) && Passed;
  bool InSequence = true;
  for (unsigned int h = 0; h < 4; ++h) {
    if (Track.GetNextRESE() != Hits[h]) {
      InSequence = false;
    }
  }
  Passed = EvaluateTrue("GetNextRESE()", "from the start point", "The hits come from the start to the stop point", InSequence) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "after the last hit", "A nullptr ends the iteration", Track.GetNextRESE() == nullptr) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "after the end", "It stays a nullptr", Track.GetNextRESE() == nullptr) && Passed;

  // From the stop point:
  Passed = EvaluateTrue("ResetIterator()", "stop point", "The iterator is reset", Track.ResetIterator(Hits[3])) && Passed;
  InSequence = true;
  for (unsigned int h = 0; h < 4; ++h) {
    if (Track.GetNextRESE() != Hits[3 - h]) {
      InSequence = false;
    }
  }
  Passed = EvaluateTrue("GetNextRESE()", "from the stop point", "The hits come from the stop to the start point", InSequence) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "after the last hit", "A nullptr ends the iteration", Track.GetNextRESE() == nullptr) && Passed;

  // A reset in the middle of an iteration starts over:
  Track.ResetIterator();
  Track.GetNextRESE();
  Track.GetNextRESE();
  Track.ResetIterator();
  Passed = EvaluateTrue("ResetIterator()", "in the middle of an iteration", "The iteration starts at the start point again", Track.GetNextRESE() == Hits[0]) && Passed;

  // Without a start point the first end point is used:
  MRETrack EndPoints;
  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  MREHit* B = CreateHit(MVector(0.0, 0.0, 1.0), 100.0, MDDetector::c_Strip2D);
  EndPoints.AddEndPoint(B, nullptr);
  EndPoints.AddEndPoint(A, B);
  Passed = EvaluateTrue("ResetIterator()", "end points, no start point", "The iterator is reset", EndPoints.ResetIterator()) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "end points, no start point", "The iteration starts at the first end point", EndPoints.GetNextRESE() == B) && Passed;
  Passed = EvaluateTrue("GetNextRESE()", "end points, no start point", "The second hit follows", EndPoints.GetNextRESE() == A) && Passed;
  delete A;
  delete B;

  // Before the first reset there is nothing to iterate:
  MRETrack Fresh;
  Passed = EvaluateTrue("GetNextRESE()", "no reset", "A nullptr is returned", Fresh.GetNextRESE() == nullptr) && Passed;

  // Defect REV-B-9: A start in the middle of the track (and a track without start and end points) ends the program with Fatal() instead of returning false
  Passed = ExpectedToCrash(EvaluateTrue("ResetIterator()", "hit with two links", "The start is refused and the program goes on", ResetIteratorInTheMiddle())) && Passed;

  DeleteHits(Hits);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the validation of the links
bool UTRETrack::TestValidateLinks()
{
  bool Passed = true;

  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  MREHit* B = CreateHit(MVector(0.0, 0.0, 1.0), 100.0, MDDetector::c_Strip2D);
  MREHit* C = CreateHit(MVector(0.0, 0.0, 2.0), 100.0, MDDetector::c_Strip2D);
  MREHit* D = CreateHit(MVector(0.0, 0.0, 3.0), 100.0, MDDetector::c_Strip2D);

  // The links of the inner hits are in the wrong sequence, the previous hit first:
  MRETrack Track;
  Track.AddRESE(A);
  Track.AddRESE(B);
  Track.AddRESE(C);
  Track.AddRESE(D);
  A->AddLink(B);
  B->AddLink(A);
  B->AddLink(C);
  C->AddLink(B);
  C->AddLink(D);
  D->AddLink(C);
  Track.CreateEndPoints();
  Track.SetStartPoint(A);
  Passed = EvaluateTrue("SetStartPoint()", "chain of four hits", "The stop point is the other end point", Track.GetStopPoint() == D) && Passed;

  Passed = EvaluateTrue("ValidateLinks()", "previous hit as first link", "The links are valid", Track.ValidateLinks()) && Passed;
  Passed = EvaluateTrue("ValidateLinks()", "previous hit as first link", "The first link of the second hit is now the next hit", B->GetLinkAt(0) == C && B->GetLinkAt(1) == A) && Passed;
  Passed = EvaluateTrue("ValidateLinks()", "previous hit as first link", "The first link of the third hit is now the next hit", C->GetLinkAt(0) == D && C->GetLinkAt(1) == B) && Passed;
  Passed = EvaluateTrue("ValidateLinks()", "previous hit as first link", "The links of the start and the stop point are unchanged", A->GetLinkAt(0) == B && D->GetLinkAt(0) == C) && Passed;

  // A second call changes nothing:
  Passed = EvaluateTrue("ValidateLinks()", "second call", "The links are still valid", Track.ValidateLinks()) && Passed;
  Passed = EvaluateTrue("ValidateLinks()", "second call", "The sequence of the links is unchanged", B->GetLinkAt(0) == C && C->GetLinkAt(0) == D) && Passed;

  // Without start and stop point the links cannot be validated - the message of ROOT is hidden
  MRETrack NoStart;
  int ErrorIgnoreLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kBreak;
  bool Validated = NoStart.ValidateLinks();
  gErrorIgnoreLevel = ErrorIgnoreLevel;
  Passed = EvaluateFalse("ValidateLinks()", "no start and stop point", "The links are not valid", Validated) && Passed;

  delete A;
  delete B;
  delete C;
  delete D;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the validity
bool UTRETrack::TestIsValid()
{
  bool Passed = true;

  MRETrack Track;
  vector<MREHit*> Hits;
  FillReferenceTrack(Track, Hits);
  Passed = EvaluateTrue("IsValid()", "linked track", "A track with a start point and valid hits is valid", Track.IsValid()) && Passed;

  Hits[2]->SetValid(false);
  Passed = EvaluateFalse("IsValid()", "third hit not valid", "The track is not valid", Track.IsValid()) && Passed;
  Hits[2]->SetValid(true);
  Passed = EvaluateTrue("IsValid()", "third hit valid again", "The track is valid again", Track.IsValid()) && Passed;
  DeleteHits(Hits);

  // Without a start point:
  MRETrack NoStart;
  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  NoStart.AddRESE(A);
  Passed = EvaluateFalse("IsValid()", "no start point", "A track without a start point is not valid", NoStart.IsValid()) && Passed;
  delete A;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the copy constructor and the duplication
bool UTRETrack::TestCopyAndDuplicate()
{
  bool Passed = true;

  MRETrack Track;
  vector<MREHit*> Hits;
  FillReferenceTrack(Track, Hits);
  Track.SetQualityFactor(0.75);

  MRETrack* Duplicate = Track.Duplicate();
  Passed = EvaluateTrue("Duplicate()", "reference track", "A new track is returned", Duplicate != nullptr && Duplicate != &Track) && Passed;
  if (Duplicate != nullptr) {
    Passed = Evaluate("Duplicate()", "reference track", "The type is a track", Duplicate->GetType(), MRESE::c_Track) && Passed;
    Passed = Evaluate("Duplicate()", "reference track", "The ID is kept", Duplicate->GetID(), Track.GetID()) && Passed;
    Passed = Evaluate("Duplicate()", "reference track", "There are four hits", Duplicate->GetNRESEs(), 4) && Passed;
    Passed = EvaluateNear("Duplicate()", "reference track", "The energy is copied", Duplicate->GetEnergy(), 1000.0, 1e-12) && Passed;
    Passed = EvaluateNear("Duplicate()", "reference track", "The quality factor is copied", Duplicate->GetQualityFactor(), 0.75, 0.0) && Passed;
    Passed = EvaluateFalse("Duplicate()", "reference track", "The direction is not fixed", Duplicate->IsDirectionFixed()) && Passed;
    Passed = EvaluateTrue("Duplicate()", "reference track", "The duplicate is valid", Duplicate->IsValid()) && Passed;

    // Start, stop, and end points are the duplicated hits with the IDs of the original ones:
    Passed = EvaluateTrue("Duplicate()", "reference track", "The start point is an own hit", Duplicate->GetStartPoint() != nullptr && Duplicate->GetStartPoint() != Hits[0]) && Passed;
    if (Duplicate->GetStartPoint() != nullptr) {
      Passed = Evaluate("Duplicate()", "reference track", "The start point has the ID of the original start point", Duplicate->GetStartPoint()->GetID(), Hits[0]->GetID()) && Passed;
    }
    Passed = EvaluateTrue("Duplicate()", "reference track", "The stop point is an own hit", Duplicate->GetStopPoint() != nullptr && Duplicate->GetStopPoint() != Hits[3]) && Passed;
    if (Duplicate->GetStopPoint() != nullptr) {
      Passed = Evaluate("Duplicate()", "reference track", "The stop point has the ID of the original stop point", Duplicate->GetStopPoint()->GetID(), Hits[3]->GetID()) && Passed;
    }
    Passed = Evaluate("Duplicate()", "reference track", "There are two end points", Duplicate->GetNEndPoints(), 2) && Passed;
    if (Duplicate->GetNEndPoints() == 2) {
      Passed = EvaluateTrue("Duplicate()", "reference track", "The end points are the start and the stop point of the duplicate", Duplicate->GetEndPointAt(0) == Duplicate->GetStartPoint() && Duplicate->GetEndPointAt(1) == Duplicate->GetStopPoint()) && Passed;
    }

    // The links are restored between the duplicated hits:
    if (Duplicate->GetNRESEs() == 4) {
      bool LinksRestored = false;
      if (Duplicate->GetRESEAt(0)->GetNLinks() == 1 && Duplicate->GetRESEAt(0)->GetLinkAt(0) == Duplicate->GetRESEAt(1) &&
          Duplicate->GetRESEAt(1)->GetNLinks() == 2 && Duplicate->GetRESEAt(1)->GetLinkAt(0) == Duplicate->GetRESEAt(2) && Duplicate->GetRESEAt(1)->GetLinkAt(1) == Duplicate->GetRESEAt(0) &&
          Duplicate->GetRESEAt(3)->GetNLinks() == 1 && Duplicate->GetRESEAt(3)->GetLinkAt(0) == Duplicate->GetRESEAt(2)) {
        LinksRestored = true;
      }
      Passed = EvaluateTrue("Duplicate()", "reference track", "The duplicated hits are linked to each other in the same sequence", LinksRestored) && Passed;
    }
    Passed = EvaluateVectorNear("Duplicate()", "reference track", "The direction is the one of the original", Duplicate->GetDirection(), MVector(0.6, 0.0, 0.8), 1e-12) && Passed;
    Passed = EvaluateVectorNear("Duplicate()", "reference track", "The final direction is the one of the original", Duplicate->GetFinalDirection(), MVector(0.0, 0.0, 1.0), 1e-12) && Passed;

    // The duplicate is independent:
    Duplicate->GetStartPoint()->SetPosition(MVector(9.0, 9.0, 9.0));
    Duplicate->SetQualityFactor(0.1);
    Passed = EvaluateVectorNear("Duplicate()", "duplicate changed", "The start point of the original is unchanged", Hits[0]->GetPosition(), MVector(0.0, 0.0, 0.0), 0.0) && Passed;
    Passed = EvaluateNear("Duplicate()", "duplicate changed", "The quality factor of the original is unchanged", Track.GetQualityFactor(), 0.75, 0.0) && Passed;
    Passed = Evaluate("Duplicate()", "duplicate changed", "The links of the original are unchanged", Hits[1]->GetNLinks(), 2) && Passed;

    Duplicate->DeleteAll();
    delete Duplicate;
  }

  // Through the base class pointer the track is duplicated as a track:
  MRESE* Base = &Track;
  MRESE* BaseDuplicate = Base->Duplicate();
  Passed = EvaluateTrue("Duplicate()", "base class pointer", "The duplicate is a track", dynamic_cast<MRETrack*>(BaseDuplicate) != nullptr) && Passed;
  BaseDuplicate->DeleteAll();
  delete BaseDuplicate;

  // A track without start point and end points:
  MRETrack Plain;
  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  Plain.AddRESE(A);
  MRETrack Copy(&Plain);
  Passed = EvaluateTrue("MRETrack(MRETrack*)", "no start point", "The copy has no start point", Copy.GetStartPoint() == nullptr) && Passed;
  Passed = EvaluateTrue("MRETrack(MRETrack*)", "no start point", "The copy has no stop point", Copy.GetStopPoint() == nullptr) && Passed;
  Passed = Evaluate("MRETrack(MRETrack*)", "no start point", "The copy has no end points", Copy.GetNEndPoints(), 0) && Passed;
  Passed = Evaluate("MRETrack(MRETrack*)", "no start point", "The copy has one hit", Copy.GetNRESEs(), 1) && Passed;
  Copy.DeleteAll();
  delete A;

  DeleteHits(Hits);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the straightness and the first-last-hit score
bool UTRETrack::TestScores()
{
  bool Passed = true;

  // A straight track along z: each pair of steps has the angles 0 and 0 to the z axis, cos(0 + 0) = 1
  {
    MRETrack Track;
    vector<MREHit*> Hits;
    FillVerticalTrack(Track, Hits, { 100.0, 200.0, 300.0, 400.0 });
    Passed = EvaluateNear("CalculateStraightnessScore()", "four hits along z", "The score of a straight track along z is 1", Track.CalculateStraightnessScore(), 1.0, 1e-12) && Passed;
    DeleteHits(Hits);
  }

  // Fewer than three hits have no straightness:
  {
    MRETrack Track;
    vector<MREHit*> Hits;
    FillVerticalTrack(Track, Hits, { 100.0, 200.0 });
    Passed = EvaluateNear("CalculateStraightnessScore()", "two hits", "The score is zero", Track.CalculateStraightnessScore(), 0.0, 0.0) && Passed;

    // First-last-hit score of two hits: (200 - 100)/(200 + 100) = 1/3
    Passed = EvaluateNear("CalculateFirstLastHitScore()", "100 and 200 keV", "The score is 1/3", Track.CalculateFirstLastHitScore(), 1.0/3.0, 1e-12) && Passed;
    DeleteHits(Hits);
  }
  {
    MRETrack Track;
    vector<MREHit*> Hits;
    FillVerticalTrack(Track, Hits, { 300.0, 100.0 });
    // (100 - 300)/(100 + 300) = -0.5
    Passed = EvaluateNear("CalculateFirstLastHitScore()", "300 and 100 keV", "The score is -0.5", Track.CalculateFirstLastHitScore(), -0.5, 1e-12) && Passed;
    DeleteHits(Hits);
  }
  {
    MRETrack Track;
    vector<MREHit*> Hits;
    FillVerticalTrack(Track, Hits, { 100.0, 5000.0, 400.0 });
    // Three hits: only the first and the last one count, (400 - 100)/(400 + 100) = 0.6
    // Defect REV-B-19: For three hits the score uses the hit before the last one, here (5000 - 100)/(5000 + 100)
    Passed = ExpectedToFail(EvaluateNear("CalculateFirstLastHitScore()", "100, 5000, and 400 keV", "The score is 0.6", Track.CalculateFirstLastHitScore(), 0.6, 1e-12)) && Passed;
    DeleteHits(Hits);
  }

  // Without a start point there are no scores:
  {
    MRETrack Track;
    MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
    MREHit* B = CreateHit(MVector(0.0, 0.0, 1.0), 200.0, MDDetector::c_Strip2D);
    MREHit* C = CreateHit(MVector(0.0, 0.0, 2.0), 300.0, MDDetector::c_Strip2D);
    Track.AddRESE(A);
    Track.AddRESE(B);
    Track.AddRESE(C);
    Passed = EvaluateNear("CalculateStraightnessScore()", "no start point", "The score is zero", Track.CalculateStraightnessScore(), 0.0, 0.0) && Passed;
    Passed = EvaluateNear("CalculateFirstLastHitScore()", "no start point", "The score is zero", Track.CalculateFirstLastHitScore(), 0.0, 0.0) && Passed;
    Passed = EvaluateNear("CalculatePearsonCorrelation()", "no start point", "The score is zero", Track.CalculatePearsonCorrelation(), 0.0, 0.0) && Passed;
    DisableDefaultStreams();
    double Spearman = Track.CalculateSpearmanRankCorrelation();
    EnableDefaultStreams();
    Passed = EvaluateNear("CalculateSpearmanRankCorrelation()", "no start point", "The score is zero", Spearman, 0.0, 0.0) && Passed;
    delete A;
    delete B;
    delete C;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the Pearson correlation
bool UTRETrack::TestPearsonCorrelation()
{
  bool Passed = true;

  // The correlation of the energies with their position in the track, 1, 2, 3, ...

  // Energies which rise linearly are fully correlated:
  {
    MRETrack Track;
    vector<MREHit*> Hits;
    FillVerticalTrack(Track, Hits, { 10.0, 20.0, 30.0, 40.0 });
    Passed = EvaluateNear("CalculatePearsonCorrelation()", "10, 20, 30, 40 keV", "The correlation is 1", Track.CalculatePearsonCorrelation(), 1.0, 1e-12) && Passed;
    Passed = EvaluateNear("CalculatePearsonCorrelation()", "10, 20, 30, 40 keV, default", "One is good by default", Track.CalculatePearsonCorrelation(true), 1.0, 1e-12) && Passed;
    // (1 - r)/2 = 0
    Passed = EvaluateNear("CalculatePearsonCorrelation()", "10, 20, 30, 40 keV, zero is good", "The score is 0", Track.CalculatePearsonCorrelation(false), 0.0, 1e-12) && Passed;
    DeleteHits(Hits);
  }

  // Falling energies are anti-correlated:
  {
    MRETrack Track;
    vector<MREHit*> Hits;
    FillVerticalTrack(Track, Hits, { 40.0, 10.0 });
    Passed = EvaluateNear("CalculatePearsonCorrelation()", "40, 10 keV", "The correlation is -1", Track.CalculatePearsonCorrelation(), -1.0, 1e-12) && Passed;
    // (1 - r)/2 = 1
    Passed = EvaluateNear("CalculatePearsonCorrelation()", "40, 10 keV, zero is good", "The score is 1", Track.CalculatePearsonCorrelation(false), 1.0, 1e-12) && Passed;
    DeleteHits(Hits);
  }

  // x = 1, 2, 3 and y = 10, 30, 20: mean x = 2, mean y = 20, covariance (10 + 60 + 60)/3 - 40 = 10/3,
  // variance x = 14/3 - 4 = 2/3, variance y = 1400/3 - 400 = 200/3, r = (10/3)/sqrt(2/3 * 200/3) = 0.5
  {
    MRETrack Track;
    vector<MREHit*> Hits;
    FillVerticalTrack(Track, Hits, { 10.0, 30.0, 20.0 });
    Passed = EvaluateNear("CalculatePearsonCorrelation()", "10, 30, 20 keV", "The correlation is 0.5", Track.CalculatePearsonCorrelation(), 0.5, 1e-12) && Passed;
    // (1 - r)/2 = 0.25
    Passed = EvaluateNear("CalculatePearsonCorrelation()", "10, 30, 20 keV, zero is good", "The score is 0.25", Track.CalculatePearsonCorrelation(false), 0.25, 1e-12) && Passed;
    DeleteHits(Hits);
  }

  // Equal energies have no correlation:
  {
    MRETrack Track;
    vector<MREHit*> Hits;
    FillVerticalTrack(Track, Hits, { 25.0, 25.0 });
    Passed = EvaluateNear("CalculatePearsonCorrelation()", "25, 25 keV", "The correlation is 0", Track.CalculatePearsonCorrelation(), 0.0, 1e-12) && Passed;
    DeleteHits(Hits);
  }

  // From five hits on (three angles) the correlation of the change of the direction counts half: a straight track has no change, thus no correlation
  {
    MRETrack Track;
    vector<MREHit*> Hits;
    FillVerticalTrack(Track, Hits, { 10.0, 20.0, 30.0, 40.0, 50.0 });
    // (1 + 0)/2
    Passed = EvaluateNear("CalculatePearsonCorrelation()", "five hits, straight, rising energies", "The score is 0.5", Track.CalculatePearsonCorrelation(), 0.5, 1e-12) && Passed;
    // (2 - 1 - 0)/4
    Passed = EvaluateNear("CalculatePearsonCorrelation()", "five hits, straight, rising energies, zero is good", "The score is 0.25", Track.CalculatePearsonCorrelation(false), 0.25, 1e-12) && Passed;
    DeleteHits(Hits);
  }

  // Five hits in the x-z plane with the directions 0, 10, 30, 60 deg to the z axis: the changes 10, 20, 30 deg rise linearly, the energies too
  {
    MRETrack Track;
    vector<MREHit*> Hits;
    vector<MVector> Positions;
    MVector Position(0.0, 0.0, 0.0);
    Positions.push_back(Position);
    double Angles[4] = { 0.0, 10.0, 30.0, 60.0 };
    for (unsigned int s = 0; s < 4; ++s) {
      Position += MVector(sin(Angles[s]*c_Rad), 0.0, cos(Angles[s]*c_Rad));
      Positions.push_back(Position);
    }
    FillTrack(Track, Hits, Positions, { 10.0, 20.0, 30.0, 40.0, 50.0 });
    // (1 + 1)/2 - the angles are calculated from the positions, thus a tolerance of 1e-9
    Passed = EvaluateNear("CalculatePearsonCorrelation()", "five hits, rising angle changes and energies", "The score is 1", Track.CalculatePearsonCorrelation(), 1.0, 1e-9) && Passed;
    // (2 - 1 - 1)/4
    Passed = EvaluateNear("CalculatePearsonCorrelation()", "five hits, rising angle changes and energies, zero is good", "The score is 0", Track.CalculatePearsonCorrelation(false), 0.0, 1e-9) && Passed;
    DeleteHits(Hits);
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the Spearman rank correlation
bool UTRETrack::TestSpearmanRankCorrelation()
{
  bool Passed = true;

  // The rank correlation r = 1 - 6 sum(d^2)/(n (n^2 - 1)) of the energies with their position in the track, returned as (1 - r)/2

  // Rising energies: all ranks agree, r = 1
  {
    MRETrack Track;
    vector<MREHit*> Hits;
    FillVerticalTrack(Track, Hits, { 10.0, 20.0, 35.0, 90.0 });
    Passed = EvaluateNear("CalculateSpearmanRankCorrelation()", "10, 20, 35, 90 keV", "The score is 0", Track.CalculateSpearmanRankCorrelation(), 0.0, 1e-12) && Passed;
    DeleteHits(Hits);
  }

  // Two falling energies: d = 1, -1, r = 1 - 6*2/(2*3) = -1
  {
    MRETrack Track;
    vector<MREHit*> Hits;
    FillVerticalTrack(Track, Hits, { 40.0, 10.0 });
    Passed = EvaluateNear("CalculateSpearmanRankCorrelation()", "40, 10 keV", "The score is 1", Track.CalculateSpearmanRankCorrelation(), 1.0, 1e-12) && Passed;
    DeleteHits(Hits);
  }

  // 10, 30, 20 keV: the ranks are 0, 2, 1, d^2 = 0 + 1 + 1, r = 1 - 6*2/(3*8) = 0.5
  {
    MRETrack Track;
    vector<MREHit*> Hits;
    FillVerticalTrack(Track, Hits, { 10.0, 30.0, 20.0 });
    Passed = EvaluateNear("CalculateSpearmanRankCorrelation()", "10, 30, 20 keV", "The score is 0.25", Track.CalculateSpearmanRankCorrelation(), 0.25, 1e-12) && Passed;
    DeleteHits(Hits);
  }

  // From six hits on the rank correlation of the changes of the direction counts half: all changes of a straight track are equal and keep their sequence, r = 1
  {
    MRETrack Track;
    vector<MREHit*> Hits;
    FillVerticalTrack(Track, Hits, { 10.0, 20.0, 30.0, 40.0, 50.0, 60.0 });
    // (2 - 1 - 1)/4
    Passed = EvaluateNear("CalculateSpearmanRankCorrelation()", "six hits, straight, rising energies", "The score is 0", Track.CalculateSpearmanRankCorrelation(), 0.0, 1e-12) && Passed;
    DeleteHits(Hits);
  }
  {
    MRETrack Track;
    vector<MREHit*> Hits;
    FillVerticalTrack(Track, Hits, { 60.0, 50.0, 40.0, 30.0, 20.0, 10.0 });
    // Energies: d = 5, 3, 1, -1, -3, -5, r = 1 - 6*70/(6*35) = -1, (2 + 1 - 1)/4
    Passed = EvaluateNear("CalculateSpearmanRankCorrelation()", "six hits, straight, falling energies", "The score is 0.5", Track.CalculateSpearmanRankCorrelation(), 0.5, 1e-12) && Passed;
    DeleteHits(Hits);
  }

  // Five hits are below the limit for the angles:
  {
    MRETrack Track;
    vector<MREHit*> Hits;
    FillVerticalTrack(Track, Hits, { 50.0, 40.0, 30.0, 20.0, 10.0 });
    // d = 4, 2, 0, -2, -4, r = 1 - 6*40/(5*24) = -1, (1 + 1)/2
    Passed = EvaluateNear("CalculateSpearmanRankCorrelation()", "five hits, falling energies", "The score is 1", Track.CalculateSpearmanRankCorrelation(), 1.0, 1e-12) && Passed;
    DeleteHits(Hits);
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the quality factor
bool UTRETrack::TestQualityFactor()
{
  bool Passed = true;

  MRETrack Track;
  Track.SetQualityFactor(0.625);
  Passed = EvaluateNear("SetQualityFactor()/GetQualityFactor()", "0.625", "The quality factor is 0.625", Track.GetQualityFactor(), 0.625, 0.0) && Passed;
  Track.SetQualityFactor(-1.5);
  Passed = EvaluateNear("SetQualityFactor()/GetQualityFactor()", "-1.5", "A negative quality factor is kept", Track.GetQualityFactor(), -1.5, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the distances and the adjacency check
bool UTRETrack::TestDistancesAndAreAdjacent()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope is scanned", ScanTelescopeGeometry(Geometry)) && Passed;

  // A track through the centers of the tracker layers 0 to 2
  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* B = CreateHit(MVector(0.0, 0.0, 1.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* C = CreateHit(MVector(0.0, 0.0, 2.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Beside = CreateHit(MVector(1.0, 0.0, 1.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Away = CreateHit(MVector(3.0, 4.0, 2.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  bool AllCreated = false;
  if (A != nullptr && B != nullptr && C != nullptr && Beside != nullptr && Away != nullptr) {
    AllCreated = true;
  }
  Passed = EvaluateTrue("CreateHit()", "tracker hits", "All hits are created", AllCreated) && Passed;
  if (AllCreated == false) {
    return Passed;
  }

  MRETrack Track;
  Track.AddRESE(A);
  Track.AddRESE(B);
  Track.AddRESE(C);

  // The closest hit of the track is C: (3, 4, 2) - (0, 0, 2) = (3, 4, 0), length 5
  Passed = EvaluateNear("ComputeMinDistance()", "track to hit", "The distance from the closest hit of the track is 5 cm", Track.ComputeMinDistance(Away), 5.0, 1e-12) && Passed;
  Passed = EvaluateNear("ComputeMinDistance()", "track to its own hit", "The distance to a hit of the track is zero", Track.ComputeMinDistance(B), 0.0, 0.0) && Passed;
  // Defect REV-B-12: The vector of a track (and of a cluster) points from the other element to the track, the hits have it point to the other element
  Passed = ExpectedToFail(EvaluateVectorNear("ComputeMinDistanceVector()", "track to hit", "The vector points from the closest hit of the track to the hit", Track.ComputeMinDistanceVector(Away), MVector(3.0, 4.0, 0.0), 1e-12)) && Passed;

  MRECluster Cluster;
  BeginCaptureOutput();
  Cluster.AddRESE(Away);
  EndCaptureOutput();
  Passed = EvaluateNear("ComputeMinDistance()", "track to cluster", "The distance of the closest pair of hits is 5 cm", Track.ComputeMinDistance(&Cluster), 5.0, 1e-12) && Passed;
  MRETrack Other;
  Other.AddRESE(Away);
  Passed = EvaluateNear("ComputeMinDistance()", "track to track", "The distance of the closest pair of hits is 5 cm", Track.ComputeMinDistance(&Other), 5.0, 1e-12) && Passed;

  MRETrack Empty;
  Passed = EvaluateVectorNear("ComputeMinDistanceVector()", "empty track to hit", "All components are DBL_MAX", Empty.ComputeMinDistanceVector(Away), MVector(DBL_MAX, DBL_MAX, DBL_MAX), 0.0) && Passed;

  // Adjacent: one strip beside the hit of the track in layer 1
  Passed = EvaluateTrue("AreAdjacent()", "hit beside the track", "A hit one strip from a hit of the track is adjacent", Track.AreAdjacent(Beside, 3, 1)) && Passed;
  Passed = EvaluateFalse("AreAdjacent()", "hit away from the track", "A hit five strips from the track is not adjacent", Track.AreAdjacent(Away, 3, 1)) && Passed;
  Passed = EvaluateFalse("AreAdjacent()", "cluster away from the track", "A cluster five strips from the track is not adjacent", Track.AreAdjacent(&Cluster, 3, 1)) && Passed;
  MRETrack BesideTrack;
  BesideTrack.AddRESE(Beside);
  Passed = EvaluateTrue("AreAdjacent()", "track beside the track", "A track with an adjacent hit is adjacent", Track.AreAdjacent(&BesideTrack, 3, 1)) && Passed;
  Passed = EvaluateFalse("AreAdjacent()", "empty track", "An empty track is not adjacent to a hit", Empty.AreAdjacent(A, 3, 1)) && Passed;

  delete A;
  delete B;
  delete C;
  delete Beside;
  delete Away;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the volume sequence
bool UTRETrack::TestGetVolumeSequence()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope is scanned", ScanTelescopeGeometry(Geometry)) && Passed;

  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* B = CreateHit(MVector(0.0, 0.0, 1.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* C = CreateHit(MVector(0.0, 0.0, 2.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  bool AllCreated = false;
  if (A != nullptr && B != nullptr && C != nullptr) {
    AllCreated = true;
  }
  Passed = EvaluateTrue("CreateHit()", "tracker hits", "All hits are created", AllCreated) && Passed;
  if (AllCreated == false) {
    return Passed;
  }

  MRETrack Track;
  Track.AddRESE(A);
  Track.AddRESE(B);
  Track.AddRESE(C);
  Passed = EvaluateTrue("GetVolumeSequence()", "no start point, no end points", "The volume sequence is the one of the first hit", Track.GetVolumeSequence() == A->GetVolumeSequence()) && Passed;

  Track.AddEndPoint(C);
  Track.AddEndPoint(A);
  Passed = EvaluateTrue("GetVolumeSequence()", "end points, no start point", "The volume sequence is the one of the first end point", Track.GetVolumeSequence() == C->GetVolumeSequence()) && Passed;

  Track.SetStartPoint(B);
  Passed = EvaluateTrue("GetVolumeSequence()", "start point", "The volume sequence is the one of the start point", Track.GetVolumeSequence() == B->GetVolumeSequence()) && Passed;
  Passed = EvaluateTrue("GetVolumeSequence()", "start point", "The detector is the tracker", Track.GetVolumeSequence()->GetDetector() != nullptr && Track.GetVolumeSequence()->GetDetector()->GetName() == "Tracker") && Passed;

  // Through the base class pointer the same volume sequence is returned:
  MRESE* Base = &Track;
  Passed = EvaluateTrue("GetVolumeSequence()", "base class pointer", "The volume sequence is the one of the start point", Base->GetVolumeSequence() == B->GetVolumeSequence()) && Passed;

  delete A;
  delete B;
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output
bool UTRETrack::TestToString()
{
  bool Passed = true;

  MRESE::ResetIDCounter();
  MRETrack Track;
  MREHit* A = CreateHit(MVector(1.0, 2.0, 3.0), 100.0, MDDetector::c_Strip2D);
  A->SetEnergyResolution(3.0);
  MREHit* B = CreateHit(MVector(1.0, 2.0, 4.0), 200.0, MDDetector::c_Strip2D);
  B->SetEnergyResolution(4.0);

  // The track has ID 1, the hits 2 and 3
  MString HitLineA = "Hit (2) at (1.000, 2.000, 3.000)+-(0.000, 0.000, 0.000) with 100.00+-3.00 keV in detector 1\n";
  MString HitLineB = "Hit (3) at (1.000, 2.000, 4.000)+-(0.000, 0.000, 0.000) with 200.00+-4.00 keV in detector 1\n";

  // Without a start point the hits are listed in the sequence in which they were added:
  Track.AddRESE(B);
  Track.AddRESE(A);
  MString Unlinked = "Track (1) starting with (0) at (0.000, 0.000, 0.000)+-(0.000, 0.000, 0.000) with 300.00+-5.00 keV in 1 and the following hits:\n";
  Unlinked += "   " + HitLineB;
  Unlinked += "   " + HitLineA;
  Unlinked += "   Quality factor: 0.0000000000\n";
  Passed = Evaluate("ToString()", "two hits, no start point", "The text has the track, its hits, and the quality factor", Track.ToString(), Unlinked) && Passed;

  // With a start point the hits follow the links, and the end points are listed:
  A->AddLink(B);
  B->AddLink(A);
  Track.CreateEndPoints();
  Track.SetStartPoint(A);
  Track.SetQualityFactor(0.5);
  MString Linked = "Track (1) starting with (2) at (1.000, 2.000, 3.000)+-(0.000, 0.000, 0.000) with 300.00+-5.00 keV in 1 and the following hits:\n";
  Linked += "   " + HitLineA;
  Linked += "      Linked with:   (3)\n";
  Linked += "   " + HitLineB;
  Linked += "      Linked with:   (2)\n";
  Linked += "   Endpoints: \n";
  Linked += "      " + HitLineB;
  Linked += "      " + HitLineA;
  Linked += "   Quality factor: 0.5000000000\n";
  Passed = Evaluate("ToString()", "two linked hits", "The hits follow the links from the start point, then the end points and the quality factor", Track.ToString(), Linked) && Passed;

  // Without links:
  MString NoLinks = "Track (1) starting with (2) at (1.000, 2.000, 3.000)+-(0.000, 0.000, 0.000) with 300.00+-5.00 keV in 1 and the following hits:\n";
  NoLinks += "   " + HitLineA;
  NoLinks += "   " + HitLineB;
  NoLinks += "   Endpoints: \n";
  NoLinks += "      " + HitLineB;
  NoLinks += "      " + HitLineA;
  NoLinks += "   Quality factor: 0.5000000000\n";
  Passed = Evaluate("ToString()", "two linked hits, without links", "The links of the hits are not listed", Track.ToString(false), NoLinks) && Passed;

  // The stream operator writes the same text:
  ostringstream Out;
  Out<<Track;
  Passed = Evaluate("operator<<", "two linked hits", "The stream gets the text of ToString()", MString(Out.str().c_str()), Linked) && Passed;

  delete A;
  delete B;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  // Hide the information messages of ROOT
  gErrorIgnoreLevel = kWarning;

  UTRETrack Test;
  return Test.Run() == true ? 0 : 1;
}


// UTRETrack.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
