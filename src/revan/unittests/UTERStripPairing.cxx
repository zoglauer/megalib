/*
 * UTERStripPairing.cxx
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
#include <cmath>
#include <set>
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MDDetector.h"
#include "MDVolumeSequence.h"
// Defect REV-E-5: the header of MERStripPairing gives the warning that Analyze hides the one of MERConstruction - it is switched off here to keep the build of the test clean
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include "MERStripPairing.h"
#pragma GCC diagnostic pop
#include "MRawEventIncarnationList.h"
#include "MRawEventIncarnations.h"
#include "MREHit.h"
#include "MRERawEvent.h"
#include "MREStripHit.h"
#include "UTRevanShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Class giving access to the protected functions of MERStripPairing
class UTERStripPairingAccess : public MERStripPairing
{
public:
  using MERStripPairing::FindNewCombinations;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the strip pairing stage MERStripPairing of revan
class UTERStripPairing : public UTRevanBase
{
public:
  //! Default constructor
  UTERStripPairing() : UTRevanBase("UTERStripPairing") {}
  //! Default destructor
  virtual ~UTERStripPairing() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test events without strip hits
  bool TestNoStripHits();
  //! Test one x strip and one y strip
  bool TestOneToOne();
  //! Test the choice of the energy if the two sides disagree
  bool TestEnergyMismatch();
  //! Test two hits in one detector
  bool TestTwoHits();
  //! Test charge sharing between adjacent strips
  bool TestChargeSharing();
  //! Test strip hits in two detectors
  bool TestTwoDetectors();
  //! Test an event with strip hits and hits of other detectors
  bool TestMixedEvent();
  //! Test missing strips on one side
  bool TestMissingStrips();
  //! Test the limit of the number of strips
  bool TestTooManyStrips();
  //! Test an event which cannot be resolved by its energies
  bool TestNotResolvable();
  //! Test strip hits with an ideal energy resolution
  bool TestIdealEnergyResolution();
  //! Test the origins of the paired hit
  bool TestOrigins();
  //! Test the position resolution of the paired hits
  bool TestPositionResolution();
  //! Test the search for combinations of adjacent strips
  bool TestFindNewCombinations();
  //! Test the analysis of an incarnation list
  bool TestIncarnationList();
  //! Test the options text
  bool TestToString();

  //! Return a new strip hit of the depth detector: strip ID 0 to 9, energy and its resolution in keV, depth in the wafer in cm
  MREStripHit* CreateStripHit(MGeometryRevan& Geometry, bool IsXStrip, int StripID, double Energy, double EnergyResolution, double Depth, unsigned int OriginID, double Time = 0.0, long DetectorID = 1);
  //! Return the center of strip 0 to 9 of the depth detector in the wafer in cm
  double GetStripCenter(int StripID) const;
  //! Return the first hit of the event with the given energy within 1e-9 keV, or a nullptr
  MRESE* FindHit(MRERawEvent& Event, double Energy);
  //! Return true if the analysis of an empty incarnation list returns true
  bool IsEmptyListAnalyzed();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERStripPairing::Run()
{
  bool Passed = true;

  Passed = TestNoStripHits() && Passed;
  Passed = TestOneToOne() && Passed;
  Passed = TestEnergyMismatch() && Passed;
  Passed = TestTwoHits() && Passed;
  Passed = TestChargeSharing() && Passed;
  Passed = TestTwoDetectors() && Passed;
  Passed = TestMixedEvent() && Passed;
  Passed = TestMissingStrips() && Passed;
  Passed = TestTooManyStrips() && Passed;
  Passed = TestNotResolvable() && Passed;
  Passed = TestIdealEnergyResolution() && Passed;
  Passed = TestOrigins() && Passed;
  Passed = TestPositionResolution() && Passed;
  Passed = TestFindNewCombinations() && Passed;
  Passed = TestIncarnationList() && Passed;
  Passed = TestToString() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a new strip hit of the depth detector: strip ID 0 to 9, energy and its resolution in keV, depth in the wafer in cm
MREStripHit* UTERStripPairing::CreateStripHit(MGeometryRevan& Geometry, bool IsXStrip, int StripID, double Energy, double EnergyResolution, double Depth, unsigned int OriginID, double Time, long DetectorID)
{
  // The values are set as the noising stage does
  MREStripHit* Hit = new MREStripHit();
  Hit->SetEnergy(Energy);
  Hit->SetEnergyResolution(EnergyResolution);
  Hit->SetTime(Time);
  // The center of the depth wafer
  Hit->SetVolumeSequence(new MDVolumeSequence(Geometry.GetVolumeSequence(MVector(30.0, 0.0, 0.0))));
  Hit->SetDetector(MDDetector::c_Strip3D);
  Hit->SetDetectorID(DetectorID);
  Hit->IsXStrip(IsXStrip);
  Hit->SetStripID(StripID);
  Hit->SetNonStripPosition(GetStripCenter(StripID));
  Hit->SetDepthPosition(Depth);
  Hit->AddOriginID(OriginID);

  return Hit;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the center of strip 0 to 9 of the depth detector in the wafer in cm
double UTERStripPairing::GetStripCenter(int StripID) const
{
  // 10 strips of pitch 1 cm on a wafer from -5 cm to 5 cm
  return -5.0 + StripID + 0.5;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the first hit of the event with the given energy within 1e-9 keV, or a nullptr
MRESE* UTERStripPairing::FindHit(MRERawEvent& Event, double Energy)
{
  for (int r = 0; r < Event.GetNRESEs(); ++r) {
    if (Event.GetRESEAt(r) != nullptr && fabs(Event.GetRESEAt(r)->GetEnergy() - Energy) < 1e-9) {
      return Event.GetRESEAt(r);
    }
  }

  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the analysis of an empty incarnation list returns true
bool UTERStripPairing::IsEmptyListAnalyzed()
{
  MERStripPairing Pairing;
  MRawEventIncarnationList List;
  return Pairing.Analyze(&List);
}


////////////////////////////////////////////////////////////////////////////////


//! Test events without strip hits
bool UTERStripPairing::TestNoStripHits()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry, "DetectorEffectsEngine 2\n")) == false) {
    return false;
  }
  MERStripPairing Pairing;
  Pairing.SetGeometry(&Geometry);

  // An empty event
  MRERawEvent Empty(&Geometry);
  Passed = EvaluateTrue("Analyze()", "empty event", "The analysis succeeds", Pairing.Analyze(&Empty)) && Passed;
  Passed = Evaluate("Analyze()", "empty event", "The event stays empty", Empty.GetNRESEs(), 0) && Passed;
  Passed = Evaluate("Analyze()", "empty event", "The event is not rejected", Empty.GetRejectionReason(), MRERawEvent::c_RejectionNone) && Passed;

  // An event with normal hits only is not touched
  MRERawEvent Event(&Geometry);
  MREHit* Tracker = CreateHit(MVector(1.0, 2.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Calorimeter = CreateHit(MVector(0.0, 0.0, -10.0), 300.0, MDDetector::c_Calorimeter, &Geometry);
  Event.AddRESE(Tracker);
  Event.AddRESE(Calorimeter);
  Passed = EvaluateTrue("Analyze()", "tracker and calorimeter hit", "The analysis succeeds", Pairing.Analyze(&Event)) && Passed;
  Passed = Evaluate("Analyze()", "tracker and calorimeter hit", "Both hits are kept", Event.GetNRESEs(), 2) && Passed;
  if (Event.GetNRESEs() == 2) {
    Passed = EvaluateTrue("Analyze()", "tracker and calorimeter hit", "The first hit is still the tracker hit", Event.GetRESEAt(0) == Tracker) && Passed;
    Passed = EvaluateTrue("Analyze()", "tracker and calorimeter hit", "The second hit is still the calorimeter hit", Event.GetRESEAt(1) == Calorimeter) && Passed;
  }
  Passed = Evaluate("Analyze()", "tracker and calorimeter hit", "The event is not rejected", Event.GetRejectionReason(), MRERawEvent::c_RejectionNone) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test one x strip and one y strip
bool UTERStripPairing::TestOneToOne()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry, "DetectorEffectsEngine 2\n")) == false) {
    return false;
  }
  MERStripPairing Pairing;
  Pairing.SetGeometry(&Geometry);

  // x strip 5: 200 +- 1 keV, depth 0.4 cm, 2 us; y strip 3: 202 +- 1 keV, depth 0.6 cm, 1 us
  MRERawEvent Event(&Geometry);
  Event.AddRESE(CreateStripHit(Geometry, true, 5, 200.0, 1.0, 0.4, 7, 2e-6));
  Event.AddRESE(CreateStripHit(Geometry, false, 3, 202.0, 1.0, 0.6, 7, 1e-6));
  Passed = EvaluateNear("GetEnergy()", "x strip of 200 keV and y strip of 202 keV", "The event energy before the pairing is half the sum of the strips", Event.GetEnergy(), 201.0, 1e-12) && Passed;
  Passed = EvaluateTrue("Analyze()", "x strip 5 and y strip 3", "The analysis succeeds", Pairing.Analyze(&Event)) && Passed;
  Passed = Evaluate("Analyze()", "x strip 5 and y strip 3", "There is one hit", Event.GetNRESEs(), 1) && Passed;
  Passed = Evaluate("Analyze()", "x strip 5 and y strip 3", "The event is not rejected", Event.GetRejectionReason(), MRERawEvent::c_RejectionNone) && Passed;
  Passed = EvaluateTrue("Analyze()", "x strip 5 and y strip 3", "The event is valid", Event.IsValid()) && Passed;
  if (Event.GetNRESEs() == 1) {
    MRESE* Hit = Event.GetRESEAt(0);
    Passed = Evaluate("Analyze()", "x strip 5 and y strip 3", "The result is a hit", Hit->GetType(), MRESE::c_Hit) && Passed;
    Passed = Evaluate("Analyze()", "x strip 5 and y strip 3", "The detector type is Strip3D", Hit->GetDetector(), static_cast<int>(MDDetector::c_Strip3D)) && Passed;
    // The energies agree within 3 sigma: mean weighted with 1/sigma^2 = (200 + 202)/2, resolution sqrt(1/(1 + 1))
    Passed = EvaluateNear("Analyze()", "x strip 5 and y strip 3", "The energy is the weighted mean of both sides", Hit->GetEnergy(), 201.0, 1e-12) && Passed;
    Passed = EvaluateNear("Analyze()", "x strip 5 and y strip 3", "The energy resolution is the one of the weighted mean", Hit->GetEnergyResolution(), 1.0/sqrt(2.0), 1e-12) && Passed;
    // Position: wafer center (30, 0, 0) + strip centers (0.5, -1.5), depth weighted with the energies: (200*0.4 + 202*0.6)/402
    Passed = EvaluateVectorNear("Analyze()", "x strip 5 and y strip 3", "The hit is at the strip crossing and the mean depth", Hit->GetPosition(), MVector(30.5, -1.5, (200.0*0.4 + 202.0*0.6)/402.0), 1e-12) && Passed;
    // Position resolution: pitch 1 cm / sqrt(12), depth resolution 0.1 cm
    Passed = EvaluateVectorNear("Analyze()", "x strip 5 and y strip 3", "The position resolution is the one of the detector", Hit->GetPositionResolution(), MVector(1.0/sqrt(12.0), 1.0/sqrt(12.0), 0.1), 1e-12) && Passed;
    Passed = EvaluateNear("Analyze()", "x strip 5 and y strip 3", "The time is the earlier one", Hit->GetTime(), 1e-6, 1e-18) && Passed;
    // The depth detector has no time resolution, which the detector reports as 1E15 s
    Passed = EvaluateNear("Analyze()", "x strip 5 and y strip 3", "The time resolution is the one of a detector without time measurement", Hit->GetTimeResolution(), 1E15, 1.0) && Passed;
    Passed = Evaluate("Analyze()", "x strip 5 and y strip 3", "The volume sequence points to the depth detector", Hit->GetVolumeSequence()->GetDetector()->GetName(), MString("Depth")) && Passed;
    Passed = EvaluateSize("Analyze()", "x strip 5 and y strip 3", "The hit has one origin", Hit->GetOriginIDs().size(), 1) && Passed;
    Passed = EvaluateSize("Analyze()", "x strip 5 and y strip 3", "The origin is the one of the strips", Hit->GetOriginIDs().count(7), 1) && Passed;
    MREHit* AsHit = dynamic_cast<MREHit*>(Hit);
    Passed = EvaluateTrue("Analyze()", "x strip 5 and y strip 3", "The resolutions of the hit are fixed", AsHit != nullptr && AsHit->HasFixedResolutions() == true) && Passed;
    Passed = EvaluateTrue("Analyze()", "x strip 5 and y strip 3", "The hit is valid", Hit->IsValid()) && Passed;
  }
  Passed = EvaluateNear("GetEnergy()", "x strip 5 and y strip 3", "The event energy is the energy of the hit", Event.GetEnergy(), 201.0, 1e-12) && Passed;

  // The order of the strip hits in the event does not matter
  MRERawEvent Swapped(&Geometry);
  Swapped.AddRESE(CreateStripHit(Geometry, false, 3, 202.0, 1.0, 0.6, 7, 1e-6));
  Swapped.AddRESE(CreateStripHit(Geometry, true, 5, 200.0, 1.0, 0.4, 7, 2e-6));
  Pairing.Analyze(&Swapped);
  Passed = Evaluate("Analyze()", "y strip 3 and x strip 5", "There is one hit", Swapped.GetNRESEs(), 1) && Passed;
  if (Swapped.GetNRESEs() == 1) {
    Passed = EvaluateVectorNear("Analyze()", "y strip 3 and x strip 5", "The hit is at the same position", Swapped.GetRESEAt(0)->GetPosition(), MVector(30.5, -1.5, (200.0*0.4 + 202.0*0.6)/402.0), 1e-12) && Passed;
    Passed = EvaluateNear("Analyze()", "y strip 3 and x strip 5", "The hit has the same energy", Swapped.GetRESEAt(0)->GetEnergy(), 201.0, 1e-12) && Passed;
  }

  // The border strips 0 and 9
  MRERawEvent Border(&Geometry);
  Border.AddRESE(CreateStripHit(Geometry, true, 0, 100.0, 1.0, -0.9, 7));
  Border.AddRESE(CreateStripHit(Geometry, false, 9, 100.0, 1.0, -0.9, 7));
  Pairing.Analyze(&Border);
  Passed = Evaluate("Analyze()", "x strip 0 and y strip 9", "There is one hit", Border.GetNRESEs(), 1) && Passed;
  if (Border.GetNRESEs() == 1) {
    Passed = EvaluateVectorNear("Analyze()", "x strip 0 and y strip 9", "The hit is at the corner strip crossing", Border.GetRESEAt(0)->GetPosition(), MVector(25.5, 4.5, -0.9), 1e-12) && Passed;
    Passed = EvaluateNear("Analyze()", "x strip 0 and y strip 9", "The hit has 100 keV", Border.GetRESEAt(0)->GetEnergy(), 100.0, 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the choice of the energy if the two sides disagree
bool UTERStripPairing::TestEnergyMismatch()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry, "DetectorEffectsEngine 2\n")) == false) {
    return false;
  }
  MERStripPairing Pairing;
  Pairing.SetGeometry(&Geometry);

  // The x side is more than 3 sigma of the y side (2 keV) above the y side: the x side wins
  {
    MRERawEvent Event(&Geometry);
    Event.AddRESE(CreateStripHit(Geometry, true, 5, 300.0, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, false, 3, 200.0, 2.0, 0.0, 7));
    Pairing.Analyze(&Event);
    Passed = Evaluate("Analyze()", "x 300 +- 1 keV, y 200 +- 2 keV", "There is one hit", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      Passed = EvaluateNear("Analyze()", "x 300 +- 1 keV, y 200 +- 2 keV", "The energy is the one of the x side", Event.GetRESEAt(0)->GetEnergy(), 300.0, 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", "x 300 +- 1 keV, y 200 +- 2 keV", "The energy resolution is the one of the x side", Event.GetRESEAt(0)->GetEnergyResolution(), 1.0, 1e-12) && Passed;
    }
    Passed = Evaluate("Analyze()", "x 300 +- 1 keV, y 200 +- 2 keV", "The event is not rejected", Event.GetRejectionReason(), MRERawEvent::c_RejectionNone) && Passed;
  }

  // The y side is more than 3 sigma of the x side above the x side: the y side wins
  {
    MRERawEvent Event(&Geometry);
    Event.AddRESE(CreateStripHit(Geometry, true, 5, 200.0, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, false, 3, 300.0, 2.0, 0.0, 7));
    Pairing.Analyze(&Event);
    Passed = Evaluate("Analyze()", "x 200 +- 1 keV, y 300 +- 2 keV", "There is one hit", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      Passed = EvaluateNear("Analyze()", "x 200 +- 1 keV, y 300 +- 2 keV", "The energy is the one of the y side", Event.GetRESEAt(0)->GetEnergy(), 300.0, 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", "x 200 +- 1 keV, y 300 +- 2 keV", "The energy resolution is the one of the y side", Event.GetRESEAt(0)->GetEnergyResolution(), 2.0, 1e-12) && Passed;
    }
  }

  // Just inside the 3 sigma: x = y + 2.99 sigma of the y side, the weighted mean with weights 1 and 1/4: (205.98 + 200/4)/(1 + 1/4)
  {
    MRERawEvent Event(&Geometry);
    Event.AddRESE(CreateStripHit(Geometry, true, 5, 205.98, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, false, 3, 200.0, 2.0, 0.0, 7));
    Pairing.Analyze(&Event);
    Passed = Evaluate("Analyze()", "x 205.98 +- 1 keV, y 200 +- 2 keV", "There is one hit", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      Passed = EvaluateNear("Analyze()", "x 205.98 +- 1 keV, y 200 +- 2 keV", "The energy is the weighted mean", Event.GetRESEAt(0)->GetEnergy(), (205.98 + 200.0/4.0)/1.25, 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", "x 205.98 +- 1 keV, y 200 +- 2 keV", "The energy resolution is the one of the weighted mean", Event.GetRESEAt(0)->GetEnergyResolution(), sqrt(1.0/1.25), 1e-12) && Passed;
    }
  }

  // Just outside the 3 sigma: x = y + 3.01 sigma of the y side
  {
    MRERawEvent Event(&Geometry);
    Event.AddRESE(CreateStripHit(Geometry, true, 5, 206.02, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, false, 3, 200.0, 2.0, 0.0, 7));
    Pairing.Analyze(&Event);
    Passed = Evaluate("Analyze()", "x 206.02 +- 1 keV, y 200 +- 2 keV", "There is one hit", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      Passed = EvaluateNear("Analyze()", "x 206.02 +- 1 keV, y 200 +- 2 keV", "The energy is the one of the x side", Event.GetRESEAt(0)->GetEnergy(), 206.02, 1e-12) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test two hits in one detector
bool UTERStripPairing::TestTwoHits()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry, "DetectorEffectsEngine 2\n")) == false) {
    return false;
  }
  MERStripPairing Pairing;
  Pairing.SetGeometry(&Geometry);

  // x strips 2 (100 keV) and 7 (300 keV), y strips 8 (99 keV) and 1 (301 keV): the energies pair x2 with y8 and x7 with y1
  {
    MRERawEvent Event(&Geometry);
    Event.AddRESE(CreateStripHit(Geometry, true, 2, 100.0, 1.0, 0.2, 1, 3e-6));
    Event.AddRESE(CreateStripHit(Geometry, true, 7, 300.0, 1.0, -0.4, 2, 5e-6));
    Event.AddRESE(CreateStripHit(Geometry, false, 8, 99.0, 1.0, 0.2, 1, 4e-6));
    Event.AddRESE(CreateStripHit(Geometry, false, 1, 301.0, 1.0, -0.4, 2, 6e-6));
    Passed = EvaluateTrue("Analyze()", "x 100 and 300 keV, y 301 and 99 keV", "The analysis succeeds", Pairing.Analyze(&Event)) && Passed;
    Passed = Evaluate("Analyze()", "x 100 and 300 keV, y 301 and 99 keV", "There are two hits", Event.GetNRESEs(), 2) && Passed;
    Passed = Evaluate("Analyze()", "x 100 and 300 keV, y 301 and 99 keV", "The event is not rejected", Event.GetRejectionReason(), MRERawEvent::c_RejectionNone) && Passed;
    // Weighted means with equal weights: (100 + 99)/2 and (300 + 301)/2
    MRESE* Low = FindHit(Event, 99.5);
    MRESE* High = FindHit(Event, 300.5);
    Passed = EvaluateTrue("Analyze()", "x 100 and 300 keV, y 301 and 99 keV", "There is a hit of 99.5 keV", Low != nullptr) && Passed;
    Passed = EvaluateTrue("Analyze()", "x 100 and 300 keV, y 301 and 99 keV", "There is a hit of 300.5 keV", High != nullptr) && Passed;
    if (Low != nullptr && High != nullptr) {
      // x strip 2 is at -2.5 cm, y strip 8 at 3.5 cm; x strip 7 at 2.5 cm, y strip 1 at -3.5 cm
      Passed = EvaluateVectorNear("Analyze()", "x 100 and 300 keV, y 301 and 99 keV", "The hit of 99.5 keV is at x strip 2 and y strip 8", Low->GetPosition(), MVector(27.5, 3.5, 0.2), 1e-12) && Passed;
      Passed = EvaluateVectorNear("Analyze()", "x 100 and 300 keV, y 301 and 99 keV", "The hit of 300.5 keV is at x strip 7 and y strip 1", High->GetPosition(), MVector(32.5, -3.5, -0.4), 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", "x 100 and 300 keV, y 301 and 99 keV", "The time of the hit of 99.5 keV is its earlier one", Low->GetTime(), 3e-6, 1e-18) && Passed;
      Passed = EvaluateNear("Analyze()", "x 100 and 300 keV, y 301 and 99 keV", "The time of the hit of 300.5 keV is its earlier one", High->GetTime(), 5e-6, 1e-18) && Passed;
      Passed = EvaluateSize("Analyze()", "x 100 and 300 keV, y 301 and 99 keV", "The hit of 99.5 keV has the origin 1", Low->GetOriginIDs().count(1), 1) && Passed;
      Passed = EvaluateSize("Analyze()", "x 100 and 300 keV, y 301 and 99 keV", "The hit of 300.5 keV has the origin 2", High->GetOriginIDs().count(2), 1) && Passed;
      // The hits are created in the order of the strips
      Passed = EvaluateNear("Analyze()", "x 100 and 300 keV, y 301 and 99 keV", "The energy resolution of the first hit is 1/sqrt(2) keV", Low->GetEnergyResolution(), 1.0/sqrt(2.0), 1e-12) && Passed;
      // Defect REV-E-6: the sums of the squared energy resolutions of the two sides are not reset for the next hit of the same detector
      Passed = ExpectedToFail(EvaluateNear("Analyze()", "x 100 and 300 keV, y 301 and 99 keV", "The energy resolution of the second hit is 1/sqrt(2) keV", High->GetEnergyResolution(), 1.0/sqrt(2.0), 1e-12)) && Passed;
    }
    Passed = EvaluateNear("GetEnergy()", "x 100 and 300 keV, y 301 and 99 keV", "The event has 400 keV", Event.GetEnergy(), 400.0, 1e-12) && Passed;
  }

  // The same with a resolution of 1 keV on the x side and 2 keV on the y side: weighted means with weights 1 and 1/4
  {
    MRERawEvent Event(&Geometry);
    Event.AddRESE(CreateStripHit(Geometry, true, 2, 100.0, 1.0, 0.2, 1));
    Event.AddRESE(CreateStripHit(Geometry, true, 7, 300.0, 1.0, -0.4, 2));
    Event.AddRESE(CreateStripHit(Geometry, false, 8, 99.0, 2.0, 0.2, 1));
    Event.AddRESE(CreateStripHit(Geometry, false, 1, 301.0, 2.0, -0.4, 2));
    Pairing.Analyze(&Event);
    Passed = Evaluate("Analyze()", "x +- 1 keV, y +- 2 keV", "There are two hits", Event.GetNRESEs(), 2) && Passed;
    // (100 + 99/4)/1.25 = 99.8
    Passed = EvaluateTrue("Analyze()", "x +- 1 keV, y +- 2 keV", "The first hit has the weighted mean of 99.8 keV", FindHit(Event, (100.0 + 99.0/4.0)/1.25) != nullptr) && Passed;
    // Defect REV-E-6: see above - (300 + 301/4)/1.25 = 300.2
    Passed = ExpectedToFail(EvaluateTrue("Analyze()", "x +- 1 keV, y +- 2 keV", "The second hit has the weighted mean of 300.2 keV", FindHit(Event, (300.0 + 301.0/4.0)/1.25) != nullptr)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test charge sharing between adjacent strips
bool UTERStripPairing::TestChargeSharing()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry, "DetectorEffectsEngine 2\n")) == false) {
    return false;
  }
  MERStripPairing Pairing;
  Pairing.SetGeometry(&Geometry);

  // The x side shares 200 keV between the adjacent strips 4 (120 keV) and 5 (80 keV), the y strip 3 has 200 keV
  {
    MRERawEvent Event(&Geometry);
    Event.AddRESE(CreateStripHit(Geometry, true, 4, 120.0, 1.0, 0.3, 7, 2e-6));
    Event.AddRESE(CreateStripHit(Geometry, true, 5, 80.0, 1.0, 0.3, 7, 1e-6));
    Event.AddRESE(CreateStripHit(Geometry, false, 3, 200.0, 1.0, 0.3, 7, 3e-6));
    Passed = EvaluateTrue("Analyze()", "x strips 4 and 5 shared, y strip 3", "The analysis succeeds", Pairing.Analyze(&Event)) && Passed;
    Passed = Evaluate("Analyze()", "x strips 4 and 5 shared, y strip 3", "There is one hit", Event.GetNRESEs(), 1) && Passed;
    Passed = Evaluate("Analyze()", "x strips 4 and 5 shared, y strip 3", "The event is not rejected", Event.GetRejectionReason(), MRERawEvent::c_RejectionNone) && Passed;
    if (Event.GetNRESEs() == 1) {
      MRESE* Hit = Event.GetRESEAt(0);
      // x side: 200 +- sqrt(2) keV, y side: 200 +- 1 keV
      Passed = EvaluateNear("Analyze()", "x strips 4 and 5 shared, y strip 3", "The energy is 200 keV", Hit->GetEnergy(), 200.0, 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", "x strips 4 and 5 shared, y strip 3", "The energy resolution is sqrt(1/(1/2 + 1)) keV", Hit->GetEnergyResolution(), sqrt(2.0/3.0), 1e-12) && Passed;
      // x: energy weighted mean of the strip centers (120*(-0.5) + 80*0.5)/200 = -0.1 cm
      Passed = EvaluateVectorNear("Analyze()", "x strips 4 and 5 shared, y strip 3", "The x position is the energy weighted mean of the strips", Hit->GetPosition(), MVector(29.9, -1.5, 0.3), 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", "x strips 4 and 5 shared, y strip 3", "The time is the earliest one", Hit->GetTime(), 1e-6, 1e-18) && Passed;
    }
  }

  // Three adjacent strips 4, 5, 6 on the y side with 50, 100, 50 keV
  {
    MRERawEvent Event(&Geometry);
    Event.AddRESE(CreateStripHit(Geometry, true, 2, 200.0, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, false, 4, 50.0, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, false, 5, 100.0, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, false, 6, 50.0, 1.0, 0.0, 7));
    Pairing.Analyze(&Event);
    Passed = Evaluate("Analyze()", "x strip 2, y strips 4, 5, and 6 shared", "There is one hit", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      MRESE* Hit = Event.GetRESEAt(0);
      // x side: 200 +- 1 keV, y side: 200 +- sqrt(3) keV
      Passed = EvaluateNear("Analyze()", "x strip 2, y strips 4, 5, and 6 shared", "The energy is 200 keV", Hit->GetEnergy(), 200.0, 1e-12) && Passed;
      Passed = EvaluateNear("Analyze()", "x strip 2, y strips 4, 5, and 6 shared", "The energy resolution is sqrt(1/(1 + 1/3)) keV", Hit->GetEnergyResolution(), sqrt(0.75), 1e-12) && Passed;
      // y: (50*(-0.5) + 100*0.5 + 50*1.5)/200 = 0.5 cm
      Passed = EvaluateVectorNear("Analyze()", "x strip 2, y strips 4, 5, and 6 shared", "The y position is the energy weighted mean of the strips", Hit->GetPosition(), MVector(27.5, 0.5, 0.0), 1e-12) && Passed;
    }
  }

  // Charge sharing on both sides and a second hit: x 4 + 5 (60 + 40 keV) with y 1 + 2 (30 + 70 keV), and x 8 (300 keV) with y 7 (300 keV)
  {
    MRERawEvent Event(&Geometry);
    Event.AddRESE(CreateStripHit(Geometry, true, 4, 60.0, 1.0, 0.0, 1));
    Event.AddRESE(CreateStripHit(Geometry, true, 5, 40.0, 1.0, 0.0, 1));
    Event.AddRESE(CreateStripHit(Geometry, true, 8, 300.0, 1.0, 0.0, 2));
    Event.AddRESE(CreateStripHit(Geometry, false, 1, 30.0, 1.0, 0.0, 1));
    Event.AddRESE(CreateStripHit(Geometry, false, 2, 70.0, 1.0, 0.0, 1));
    Event.AddRESE(CreateStripHit(Geometry, false, 7, 300.0, 1.0, 0.0, 2));
    Pairing.Analyze(&Event);
    Passed = Evaluate("Analyze()", "two hits, one shared on both sides", "There are two hits", Event.GetNRESEs(), 2) && Passed;
    MRESE* Shared = FindHit(Event, 100.0);
    MRESE* Single = FindHit(Event, 300.0);
    Passed = EvaluateTrue("Analyze()", "two hits, one shared on both sides", "There is a hit of 100 keV", Shared != nullptr) && Passed;
    Passed = EvaluateTrue("Analyze()", "two hits, one shared on both sides", "There is a hit of 300 keV", Single != nullptr) && Passed;
    if (Shared != nullptr && Single != nullptr) {
      // x: (60*(-0.5) + 40*0.5)/100 = -0.1 cm, y: (30*(-3.5) + 70*(-2.5))/100 = -2.8 cm
      Passed = EvaluateVectorNear("Analyze()", "two hits, one shared on both sides", "The shared hit is at the energy weighted means", Shared->GetPosition(), MVector(29.9, -2.8, 0.0), 1e-12) && Passed;
      Passed = EvaluateVectorNear("Analyze()", "two hits, one shared on both sides", "The single hit is at x strip 8 and y strip 7", Single->GetPosition(), MVector(33.5, 2.5, 0.0), 1e-12) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test strip hits in two detectors
bool UTERStripPairing::TestTwoDetectors()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry, "DetectorEffectsEngine 2\n")) == false) {
    return false;
  }
  MERStripPairing Pairing;
  Pairing.SetGeometry(&Geometry);

  // The strips of detector 1 (200 keV) and detector 2 (300 keV) are not mixed, although x of detector 1 fits y of detector 2 as well as its own
  MRERawEvent Event(&Geometry);
  Event.AddRESE(CreateStripHit(Geometry, true, 5, 250.0, 1.0, 0.0, 1, 0.0, 1));
  Event.AddRESE(CreateStripHit(Geometry, false, 3, 200.0, 1.0, 0.0, 1, 0.0, 2));
  Event.AddRESE(CreateStripHit(Geometry, true, 2, 201.0, 1.0, 0.0, 2, 0.0, 2));
  Event.AddRESE(CreateStripHit(Geometry, false, 6, 251.0, 1.0, 0.0, 2, 0.0, 1));
  Passed = EvaluateTrue("Analyze()", "two detectors", "The analysis succeeds", Pairing.Analyze(&Event)) && Passed;
  Passed = Evaluate("Analyze()", "two detectors", "There are two hits", Event.GetNRESEs(), 2) && Passed;
  Passed = Evaluate("Analyze()", "two detectors", "The event is not rejected", Event.GetRejectionReason(), MRERawEvent::c_RejectionNone) && Passed;
  // Detector 1: x 5 (250 keV) and y 6 (251 keV), detector 2: x 2 (201 keV) and y 3 (200 keV)
  MRESE* First = FindHit(Event, 250.5);
  MRESE* Second = FindHit(Event, 200.5);
  Passed = EvaluateTrue("Analyze()", "two detectors", "The hit of detector 1 has 250.5 keV", First != nullptr) && Passed;
  Passed = EvaluateTrue("Analyze()", "two detectors", "The hit of detector 2 has 200.5 keV", Second != nullptr) && Passed;
  if (First != nullptr && Second != nullptr) {
    Passed = EvaluateVectorNear("Analyze()", "two detectors", "The hit of detector 1 is at x strip 5 and y strip 6", First->GetPosition(), MVector(30.5, 1.5, 0.0), 1e-12) && Passed;
    Passed = EvaluateVectorNear("Analyze()", "two detectors", "The hit of detector 2 is at x strip 2 and y strip 3", Second->GetPosition(), MVector(27.5, -1.5, 0.0), 1e-12) && Passed;
    // The sums of the resolutions start again for each detector
    Passed = EvaluateNear("Analyze()", "two detectors", "The energy resolution of the hit of detector 1 is 1/sqrt(2) keV", First->GetEnergyResolution(), 1.0/sqrt(2.0), 1e-12) && Passed;
    Passed = EvaluateNear("Analyze()", "two detectors", "The energy resolution of the hit of detector 2 is 1/sqrt(2) keV", Second->GetEnergyResolution(), 1.0/sqrt(2.0), 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test an event with strip hits and hits of other detectors
bool UTERStripPairing::TestMixedEvent()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry, "DetectorEffectsEngine 2\n")) == false) {
    return false;
  }
  MERStripPairing Pairing;
  Pairing.SetGeometry(&Geometry);

  // A tracker hit of 100 keV, a calorimeter hit of 300 keV, and the two strips of a hit of 200 keV in the depth detector
  MRERawEvent Event(&Geometry);
  Event.AddRESE(CreateHit(MVector(1.0, 2.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry));
  Event.AddRESE(CreateStripHit(Geometry, true, 5, 200.0, 1.0, 0.0, 7));
  Event.AddRESE(CreateHit(MVector(0.0, 0.0, -10.0), 300.0, MDDetector::c_Calorimeter, &Geometry));
  Event.AddRESE(CreateStripHit(Geometry, false, 3, 200.0, 1.0, 0.0, 7));
  Passed = EvaluateNear("GetEnergy()", "tracker hit, calorimeter hit, and two strips", "The event has 600 keV before the pairing", Event.GetEnergy(), 600.0, 1e-12) && Passed;
  Passed = EvaluateTrue("Analyze()", "tracker hit, calorimeter hit, and two strips", "The analysis succeeds", Pairing.Analyze(&Event)) && Passed;
  Passed = EvaluateTrue("Analyze()", "tracker hit, calorimeter hit, and two strips", "The paired hit of 200 keV exists", FindHit(Event, 200.0) != nullptr) && Passed;
  // Defect REV-E-7: all elements of the event are removed before the pairing, not only the strip hits, so the hits of the other detectors are lost
  Passed = ExpectedToFail(Evaluate("Analyze()", "tracker hit, calorimeter hit, and two strips", "There are three hits", Event.GetNRESEs(), 3)) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("Analyze()", "tracker hit, calorimeter hit, and two strips", "The tracker hit of 100 keV is kept", FindHit(Event, 100.0) != nullptr)) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("Analyze()", "tracker hit, calorimeter hit, and two strips", "The calorimeter hit of 300 keV is kept", FindHit(Event, 300.0) != nullptr)) && Passed;
  Passed = ExpectedToFail(EvaluateNear("GetEnergy()", "tracker hit, calorimeter hit, and two strips", "The event has 600 keV after the pairing", Event.GetEnergy(), 600.0, 1e-12)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test missing strips on one side
bool UTERStripPairing::TestMissingStrips()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry, "DetectorEffectsEngine 2\n")) == false) {
    return false;
  }
  MERStripPairing Pairing;
  Pairing.SetGeometry(&Geometry);

  // Only an x strip
  {
    MRERawEvent Event(&Geometry);
    Event.AddRESE(CreateStripHit(Geometry, true, 5, 200.0, 1.0, 0.0, 7));
    Passed = EvaluateTrue("Analyze()", "x strip only", "The analysis succeeds", Pairing.Analyze(&Event)) && Passed;
    Passed = Evaluate("Analyze()", "x strip only", "The event is rejected for missing strips", Event.GetRejectionReason(), MRERawEvent::c_RejectionStripPairingMissingStrips) && Passed;
    Passed = EvaluateFalse("Analyze()", "x strip only", "The event is not valid", Event.IsValid()) && Passed;
  }

  // Only a y strip
  {
    MRERawEvent Event(&Geometry);
    Event.AddRESE(CreateStripHit(Geometry, false, 5, 200.0, 1.0, 0.0, 7));
    Pairing.Analyze(&Event);
    Passed = Evaluate("Analyze()", "y strip only", "The event is rejected for missing strips", Event.GetRejectionReason(), MRERawEvent::c_RejectionStripPairingMissingStrips) && Passed;
    Passed = EvaluateFalse("Analyze()", "y strip only", "The event is not valid", Event.IsValid()) && Passed;
  }

  // One complete detector and one with an x strip only
  {
    MRERawEvent Event(&Geometry);
    Event.AddRESE(CreateStripHit(Geometry, true, 5, 200.0, 1.0, 0.0, 7, 0.0, 1));
    Event.AddRESE(CreateStripHit(Geometry, false, 3, 200.0, 1.0, 0.0, 7, 0.0, 1));
    Event.AddRESE(CreateStripHit(Geometry, true, 2, 300.0, 1.0, 0.0, 7, 0.0, 2));
    Pairing.Analyze(&Event);
    Passed = Evaluate("Analyze()", "second detector with x strip only", "The event is rejected for missing strips", Event.GetRejectionReason(), MRERawEvent::c_RejectionStripPairingMissingStrips) && Passed;
    Passed = EvaluateFalse("Analyze()", "second detector with x strip only", "The event is not valid", Event.IsValid()) && Passed;
  }

  // Three x strips which are not adjacent and one y strip: the numbers of hits differ by more than one
  {
    MRERawEvent Event(&Geometry);
    Event.AddRESE(CreateStripHit(Geometry, true, 1, 100.0, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, true, 4, 100.0, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, true, 7, 100.0, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, false, 3, 300.0, 1.0, 0.0, 7));
    Pairing.Analyze(&Event);
    Passed = Evaluate("Analyze()", "three separate x strips and one y strip", "The event is rejected for missing strips", Event.GetRejectionReason(), MRERawEvent::c_RejectionStripPairingMissingStrips) && Passed;
    Passed = EvaluateFalse("Analyze()", "three separate x strips and one y strip", "The event is not valid", Event.IsValid()) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the limit of the number of strips
bool UTERStripPairing::TestTooManyStrips()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry, "DetectorEffectsEngine 2\n")) == false) {
    return false;
  }
  MERStripPairing Pairing;
  Pairing.SetGeometry(&Geometry);

  // Six strips on one side are the maximum: x strips 0 + 1 (50 + 50 keV), 3, 5, 7, 9 (200, 300, 400, 500 keV), y strips 0, 2, 4, 6, 8 (100 to 500 keV)
  {
    MRERawEvent Event(&Geometry);
    Event.AddRESE(CreateStripHit(Geometry, true, 0, 50.0, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, true, 1, 50.0, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, true, 3, 200.0, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, true, 5, 300.0, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, true, 7, 400.0, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, true, 9, 500.0, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, false, 0, 100.0, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, false, 2, 200.0, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, false, 4, 300.0, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, false, 6, 400.0, 1.0, 0.0, 7));
    Event.AddRESE(CreateStripHit(Geometry, false, 8, 500.0, 1.0, 0.0, 7));
    Pairing.Analyze(&Event);
    Passed = Evaluate("Analyze()", "six x strips and five y strips", "The event is not rejected", Event.GetRejectionReason(), MRERawEvent::c_RejectionNone) && Passed;
    Passed = Evaluate("Analyze()", "six x strips and five y strips", "There are five hits", Event.GetNRESEs(), 5) && Passed;
    Passed = EvaluateNear("GetEnergy()", "six x strips and five y strips", "The event has 1500 keV", Event.GetEnergy(), 1500.0, 1e-9) && Passed;
    // The shared hit: x (50*(-4.5) + 50*(-3.5))/100 = -4 cm, y strip 0 at -4.5 cm
    MRESE* Shared = FindHit(Event, 100.0);
    Passed = EvaluateTrue("Analyze()", "six x strips and five y strips", "There is a hit of 100 keV", Shared != nullptr) && Passed;
    if (Shared != nullptr) {
      Passed = EvaluateVectorNear("Analyze()", "six x strips and five y strips", "The hit of 100 keV is between the x strips 0 and 1", Shared->GetPosition(), MVector(26.0, -4.5, 0.0), 1e-12) && Passed;
    }
  }

  // Seven strips on one side are too many: the event is rejected and keeps its strip hits
  {
    MRERawEvent Event(&Geometry);
    for (int s = 0; s < 7; ++s) {
      Event.AddRESE(CreateStripHit(Geometry, true, s, 100.0, 1.0, 0.0, 7));
    }
    Event.AddRESE(CreateStripHit(Geometry, false, 3, 700.0, 1.0, 0.0, 7));
    Passed = EvaluateTrue("Analyze()", "seven x strips and one y strip", "The analysis succeeds", Pairing.Analyze(&Event)) && Passed;
    Passed = Evaluate("Analyze()", "seven x strips and one y strip", "The event is rejected for too many strips", Event.GetRejectionReason(), MRERawEvent::c_RejectionStripPairinTooManyStrips) && Passed;
    Passed = EvaluateFalse("Analyze()", "seven x strips and one y strip", "The event is not valid", Event.IsValid()) && Passed;
    Passed = Evaluate("Analyze()", "seven x strips and one y strip", "The strip hits stay in the event", Event.GetNRESEs(), 8) && Passed;
  }

  // The same on the y side
  {
    MRERawEvent Event(&Geometry);
    Event.AddRESE(CreateStripHit(Geometry, true, 3, 700.0, 1.0, 0.0, 7));
    for (int s = 0; s < 7; ++s) {
      Event.AddRESE(CreateStripHit(Geometry, false, s, 100.0, 1.0, 0.0, 7));
    }
    Pairing.Analyze(&Event);
    Passed = Evaluate("Analyze()", "one x strip and seven y strips", "The event is rejected for too many strips", Event.GetRejectionReason(), MRERawEvent::c_RejectionStripPairinTooManyStrips) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test an event which cannot be resolved by its energies
bool UTERStripPairing::TestNotResolvable()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry, "DetectorEffectsEngine 2\n")) == false) {
    return false;
  }
  MERStripPairing Pairing;
  Pairing.SetGeometry(&Geometry);

  // x strips with 300 and 100 keV, y strips with 200 and 200 keV: each side has 400 keV, but the hits get the larger energies 300 keV and 200 keV, which is 100 keV too much
  MRERawEvent Event(&Geometry);
  Event.AddRESE(CreateStripHit(Geometry, true, 2, 300.0, 1.0, 0.0, 7));
  Event.AddRESE(CreateStripHit(Geometry, true, 7, 100.0, 1.0, 0.0, 7));
  Event.AddRESE(CreateStripHit(Geometry, false, 1, 200.0, 1.0, 0.0, 7));
  Event.AddRESE(CreateStripHit(Geometry, false, 8, 200.0, 1.0, 0.0, 7));
  Passed = EvaluateTrue("Analyze()", "x 300 and 100 keV, y 200 and 200 keV", "The analysis succeeds", Pairing.Analyze(&Event)) && Passed;
  Passed = Evaluate("Analyze()", "x 300 and 100 keV, y 200 and 200 keV", "The event is rejected as not resolvable", Event.GetRejectionReason(), MRERawEvent::c_RejectionStripPairingNotResolvable) && Passed;
  Passed = EvaluateFalse("Analyze()", "x 300 and 100 keV, y 200 and 200 keV", "The event is not valid", Event.IsValid()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test strip hits with an ideal energy resolution
bool UTERStripPairing::TestIdealEnergyResolution()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry, "DetectorEffectsEngine 2\n")) == false) {
    return false;
  }
  MERStripPairing Pairing;
  Pairing.SetGeometry(&Geometry);

  // An energy resolution of zero means ideal: both sides have exactly 200 keV
  MRERawEvent Event(&Geometry);
  Event.AddRESE(CreateStripHit(Geometry, true, 5, 200.0, 0.0, 0.0, 7));
  Event.AddRESE(CreateStripHit(Geometry, false, 3, 200.0, 0.0, 0.0, 7));
  Pairing.Analyze(&Event);
  // Defect REV-E-8: the chi square divides by the sum of the squared energy resolutions, which is zero for an ideal resolution, and the event is rejected
  Passed = ExpectedToFail(Evaluate("Analyze()", "x and y strip of 200 keV with ideal energy resolution", "The event is not rejected", Event.GetRejectionReason(), MRERawEvent::c_RejectionNone)) && Passed;
  Passed = ExpectedToFail(Evaluate("Analyze()", "x and y strip of 200 keV with ideal energy resolution", "There is one hit", Event.GetNRESEs(), 1)) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("Analyze()", "x and y strip of 200 keV with ideal energy resolution", "The hit has 200 keV", FindHit(Event, 200.0) != nullptr)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the origins of the paired hit
bool UTERStripPairing::TestOrigins()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry, "DetectorEffectsEngine 2\n")) == false) {
    return false;
  }
  MERStripPairing Pairing;
  Pairing.SetGeometry(&Geometry);

  // The x strip has the origins 1 and 2, the y strip 2 and 3: only 2 is common to both
  {
    MRERawEvent Event(&Geometry);
    MREStripHit* XStrip = CreateStripHit(Geometry, true, 5, 200.0, 1.0, 0.0, 1);
    XStrip->AddOriginID(2);
    MREStripHit* YStrip = CreateStripHit(Geometry, false, 3, 200.0, 1.0, 0.0, 2);
    YStrip->AddOriginID(3);
    Event.AddRESE(XStrip);
    Event.AddRESE(YStrip);
    Pairing.Analyze(&Event);
    Passed = Evaluate("Analyze()", "x origins 1 and 2, y origins 2 and 3", "There is one hit", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      set<unsigned int> Origins = Event.GetRESEAt(0)->GetOriginIDs();
      Passed = EvaluateSize("Analyze()", "x origins 1 and 2, y origins 2 and 3", "The hit has the common origin 2", Origins.count(2), 1) && Passed;
      // Defect REV-E-9: the search for the common origins compares the x origins with themselves, so the hit gets all x origins and never a y origin
      Passed = ExpectedToFail(EvaluateSize("Analyze()", "x origins 1 and 2, y origins 2 and 3", "The hit has only the common origin", Origins.size(), 1)) && Passed;
    }
  }

  // Without a common origin the hit gets all origins
  {
    MRERawEvent Event(&Geometry);
    Event.AddRESE(CreateStripHit(Geometry, true, 5, 200.0, 1.0, 0.0, 1));
    Event.AddRESE(CreateStripHit(Geometry, false, 3, 200.0, 1.0, 0.0, 3));
    Pairing.Analyze(&Event);
    Passed = Evaluate("Analyze()", "x origin 1, y origin 3", "There is one hit", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      set<unsigned int> Origins = Event.GetRESEAt(0)->GetOriginIDs();
      Passed = EvaluateSize("Analyze()", "x origin 1, y origin 3", "The hit has the x origin 1", Origins.count(1), 1) && Passed;
      // Defect REV-E-9: see above
      Passed = ExpectedToFail(EvaluateSize("Analyze()", "x origin 1, y origin 3", "The hit has the y origin 3", Origins.count(3), 1)) && Passed;
    }
  }

  // The origins of shared strips are combined: x strips 4 and 5 with the origins 1 and 2, y strip with 1 and 2
  {
    MRERawEvent Event(&Geometry);
    Event.AddRESE(CreateStripHit(Geometry, true, 4, 120.0, 1.0, 0.0, 1));
    Event.AddRESE(CreateStripHit(Geometry, true, 5, 80.0, 1.0, 0.0, 2));
    MREStripHit* YStrip = CreateStripHit(Geometry, false, 3, 200.0, 1.0, 0.0, 1);
    YStrip->AddOriginID(2);
    Event.AddRESE(YStrip);
    Pairing.Analyze(&Event);
    Passed = Evaluate("Analyze()", "shared x strips with origins 1 and 2", "There is one hit", Event.GetNRESEs(), 1) && Passed;
    if (Event.GetNRESEs() == 1) {
      set<unsigned int> Origins = Event.GetRESEAt(0)->GetOriginIDs();
      Passed = EvaluateSize("Analyze()", "shared x strips with origins 1 and 2", "The hit has two origins", Origins.size(), 2) && Passed;
      Passed = EvaluateSize("Analyze()", "shared x strips with origins 1 and 2", "The hit has the origin 1", Origins.count(1), 1) && Passed;
      Passed = EvaluateSize("Analyze()", "shared x strips with origins 1 and 2", "The hit has the origin 2", Origins.count(2), 1) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the position resolution of the paired hits
bool UTERStripPairing::TestPositionResolution()
{
  bool Passed = true;

  // A depth resolution which rises linearly from 0.1 cm at 10 keV to 1.0 cm at 10000 keV
  MString Setup = GetTelescopeGeometry("DetectorEffectsEngine 2\n");
  Setup.ReplaceAllInPlace("Depth.DepthResolution 10000 0.1 0\n", "Depth.DepthResolution 10000 1.0 0\n");
  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope with an energy dependent depth resolution", "The telescope can be read", ScanGeometry(Geometry, Setup)) == false) {
    return false;
  }
  MERStripPairing Pairing;
  Pairing.SetGeometry(&Geometry);

  // Two hits of 100 keV and 300 keV, which are created in this order
  MRERawEvent Event(&Geometry);
  Event.AddRESE(CreateStripHit(Geometry, true, 2, 100.0, 1.0, 0.0, 1));
  Event.AddRESE(CreateStripHit(Geometry, true, 7, 300.0, 1.0, 0.0, 2));
  Event.AddRESE(CreateStripHit(Geometry, false, 8, 100.0, 1.0, 0.0, 1));
  Event.AddRESE(CreateStripHit(Geometry, false, 1, 300.0, 1.0, 0.0, 2));
  Pairing.Analyze(&Event);
  Passed = Evaluate("Analyze()", "hits of 100 keV and 300 keV", "There are two hits", Event.GetNRESEs(), 2) && Passed;
  MRESE* Low = FindHit(Event, 100.0);
  MRESE* High = FindHit(Event, 300.0);
  Passed = EvaluateTrue("Analyze()", "hits of 100 keV and 300 keV", "There is a hit of 100 keV", Low != nullptr) && Passed;
  Passed = EvaluateTrue("Analyze()", "hits of 100 keV and 300 keV", "There is a hit of 300 keV", High != nullptr) && Passed;
  if (Low != nullptr && High != nullptr) {
    // Linear interpolation: 0.1 + 0.9*(E - 10)/9990
    Passed = EvaluateNear("Analyze()", "hits of 100 keV and 300 keV", "The depth resolution of the first hit is the one at 100 keV", Low->GetPositionResolution().Z(), 0.1 + 0.9*90.0/9990.0, 1e-12) && Passed;
    // Defect REV-E-10: the position and time resolutions are taken at the sum of the energies of all hits created so far, not at the energy of the hit
    Passed = ExpectedToFail(EvaluateNear("Analyze()", "hits of 100 keV and 300 keV", "The depth resolution of the second hit is the one at 300 keV", High->GetPositionResolution().Z(), 0.1 + 0.9*290.0/9990.0, 1e-12)) && Passed;
    Passed = EvaluateNear("Analyze()", "hits of 100 keV and 300 keV", "The x resolution is the pitch of 1 cm / sqrt(12)", High->GetPositionResolution().X(), 1.0/sqrt(12.0), 1e-12) && Passed;
    Passed = EvaluateNear("Analyze()", "hits of 100 keV and 300 keV", "The y resolution is the pitch of 1 cm / sqrt(12)", High->GetPositionResolution().Y(), 1.0/sqrt(12.0), 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search for combinations of adjacent strips
bool UTERStripPairing::TestFindNewCombinations()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry, "DetectorEffectsEngine 2\n")) == false) {
    return false;
  }
  UTERStripPairingAccess Pairing;

  // The strips 4, 5, and 7: only 4 and 5 are adjacent
  vector<MREStripHit*> StripHits;
  StripHits.push_back(CreateStripHit(Geometry, true, 4, 100.0, 1.0, 0.0, 7));
  StripHits.push_back(CreateStripHit(Geometry, true, 5, 100.0, 1.0, 0.0, 7));
  StripHits.push_back(CreateStripHit(Geometry, true, 7, 100.0, 1.0, 0.0, 7));
  vector<vector<vector<unsigned int>>> Seed = { { { 0 }, { 1 }, { 2 } } };
  vector<vector<vector<unsigned int>>> New = Pairing.FindNewCombinations(Seed, StripHits);
  Passed = EvaluateSize("FindNewCombinations()", "strips 4, 5, 7", "There is one new combination", New.size(), 1) && Passed;
  if (New.size() == 1) {
    vector<vector<unsigned int>> Expected = { { 0, 1 }, { 2 } };
    Passed = EvaluateTrue("FindNewCombinations()", "strips 4, 5, 7", "The new combination merges the strips 4 and 5", New[0] == Expected) && Passed;
  }
  // Nothing can be merged in the new combination
  Passed = EvaluateSize("FindNewCombinations()", "strips (4, 5), 7", "There is no further combination", Pairing.FindNewCombinations(New, StripHits).size(), 0) && Passed;

  // The strips 4, 5, and 6: 4 with 5 and 5 with 6, then all three
  StripHits[2]->SetStripID(6);
  New = Pairing.FindNewCombinations(Seed, StripHits);
  Passed = EvaluateSize("FindNewCombinations()", "strips 4, 5, 6", "There are two new combinations", New.size(), 2) && Passed;
  if (New.size() == 2) {
    vector<vector<unsigned int>> First = { { 0, 1 }, { 2 } };
    vector<vector<unsigned int>> Second = { { 0 }, { 1, 2 } };
    Passed = EvaluateTrue("FindNewCombinations()", "strips 4, 5, 6", "The first combination merges the strips 4 and 5", New[0] == First) && Passed;
    Passed = EvaluateTrue("FindNewCombinations()", "strips 4, 5, 6", "The second combination merges the strips 5 and 6", New[1] == Second) && Passed;
  }
  vector<vector<vector<unsigned int>>> Next = Pairing.FindNewCombinations(New, StripHits);
  Passed = EvaluateSize("FindNewCombinations()", "strips (4, 5), 6 and 4, (5, 6)", "Both give the combination of all three", Next.size(), 2) && Passed;
  if (Next.size() == 2) {
    vector<vector<unsigned int>> All = { { 0, 1, 2 } };
    Passed = EvaluateTrue("FindNewCombinations()", "strips (4, 5), 6", "The combination merges all three strips", Next[0] == All) && Passed;
    Passed = EvaluateTrue("FindNewCombinations()", "strips 4, (5, 6)", "The combination merges all three strips", Next[1] == All) && Passed;
  }

  // The order of the strip hits does not matter: the strips 6, 4, 5
  StripHits[0]->SetStripID(6);
  StripHits[1]->SetStripID(4);
  StripHits[2]->SetStripID(5);
  New = Pairing.FindNewCombinations(Seed, StripHits);
  Passed = EvaluateSize("FindNewCombinations()", "strips 6, 4, 5", "There are two new combinations", New.size(), 2) && Passed;
  if (New.size() == 2) {
    vector<vector<unsigned int>> First = { { 0, 2 }, { 1 } };
    vector<vector<unsigned int>> Second = { { 0 }, { 1, 2 } };
    Passed = EvaluateTrue("FindNewCombinations()", "strips 6, 4, 5", "The first combination merges the strips 6 and 5", New[0] == First) && Passed;
    Passed = EvaluateTrue("FindNewCombinations()", "strips 6, 4, 5", "The second combination merges the strips 4 and 5", New[1] == Second) && Passed;
  }

  // Nothing to combine
  vector<vector<vector<unsigned int>>> Empty;
  Passed = EvaluateSize("FindNewCombinations()", "no combinations", "No combinations give no new ones", Pairing.FindNewCombinations(Empty, StripHits).size(), 0) && Passed;
  vector<vector<vector<unsigned int>>> Single = { { { 0 } } };
  Passed = EvaluateSize("FindNewCombinations()", "one strip", "One strip gives no new combination", Pairing.FindNewCombinations(Single, StripHits).size(), 0) && Passed;

  for (MREStripHit* StripHit: StripHits) {
    delete StripHit;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the analysis of an incarnation list
bool UTERStripPairing::TestIncarnationList()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry, "DetectorEffectsEngine 2\n")) == false) {
    return false;
  }
  MERStripPairing Pairing;
  Pairing.SetGeometry(&Geometry);

  // Two incarnations with one event each: both are paired
  MRawEventIncarnationList List;
  for (unsigned int i = 0; i < 2; ++i) {
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    Event->AddRESE(CreateStripHit(Geometry, true, 5, 200.0 + 100.0*i, 1.0, 0.0, 7));
    Event->AddRESE(CreateStripHit(Geometry, false, 3, 200.0 + 100.0*i, 1.0, 0.0, 7));
    MRawEventIncarnations* Incarnations = new MRawEventIncarnations();
    Incarnations->AddRawEvent(Event);
    List.Add(Incarnations);
  }
  Passed = EvaluateTrue("Analyze()", "list with two events", "The analysis succeeds", Pairing.Analyze(&List)) && Passed;
  Passed = Evaluate("Analyze()", "list with two events", "The first event has one hit", List.Get(0)->GetRawEventAt(0)->GetNRESEs(), 1) && Passed;
  Passed = Evaluate("Analyze()", "list with two events", "The second event has one hit", List.Get(1)->GetRawEventAt(0)->GetNRESEs(), 1) && Passed;
  if (List.Get(0)->GetRawEventAt(0)->GetNRESEs() == 1 && List.Get(1)->GetRawEventAt(0)->GetNRESEs() == 1) {
    Passed = Evaluate("Analyze()", "list with two events", "The first event has a hit", List.Get(0)->GetRawEventAt(0)->GetRESEAt(0)->GetType(), MRESE::c_Hit) && Passed;
    Passed = EvaluateNear("Analyze()", "list with two events", "The hit of the first event has 200 keV", List.Get(0)->GetRawEventAt(0)->GetRESEAt(0)->GetEnergy(), 200.0, 1e-12) && Passed;
    Passed = EvaluateNear("Analyze()", "list with two events", "The hit of the second event has 300 keV", List.Get(1)->GetRawEventAt(0)->GetRESEAt(0)->GetEnergy(), 300.0, 1e-12) && Passed;
  }
  List.DeleteAll();

  // An incarnation with two events is not paired
  MRawEventIncarnations* Double = new MRawEventIncarnations();
  for (unsigned int i = 0; i < 2; ++i) {
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    Event->AddRESE(CreateStripHit(Geometry, true, 5, 200.0, 1.0, 0.0, 7));
    Event->AddRESE(CreateStripHit(Geometry, false, 3, 200.0, 1.0, 0.0, 7));
    Double->AddRawEvent(Event);
  }
  List.Add(Double);
  DisableDefaultStreams();
  bool Analyzed = Pairing.Analyze(&List);
  EnableDefaultStreams();
  Passed = EvaluateTrue("Analyze()", "incarnation with two events", "The analysis returns true", Analyzed) && Passed;
  Passed = Evaluate("Analyze()", "incarnation with two events", "The strip hits of the first event are not paired", List.Get(0)->GetRawEventAt(0)->GetNRESEs(), 2) && Passed;
  Passed = Evaluate("Analyze()", "incarnation with two events", "The strip hits of the second event are not paired", List.Get(0)->GetRawEventAt(1)->GetNRESEs(), 2) && Passed;
  List.DeleteAll();

  // Defect REV-E-11: the debug output at the start reads the first event of the list without a check, which throws an exception for an empty list
  Passed = ExpectedToCrash(EvaluateTrue("Analyze()", "empty list", "The analysis of an empty list succeeds", IsEmptyListAnalyzed())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the options text
bool UTERStripPairing::TestToString()
{
  bool Passed = true;

  MERStripPairing Pairing;
  MString Text = Pairing.ToString(false);
  Passed = EvaluateTrue("ToString()", "default", "The text starts with the title", Text.BeginsWith("# StripPairing options:")) && Passed;
  Passed = Evaluate("ToString()", "core only", "The core text is the same", Pairing.ToString(true), Text) && Passed;

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

  UTERStripPairing Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERStripPairing.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
