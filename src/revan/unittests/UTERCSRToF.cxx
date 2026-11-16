/*
 * UTERCSRToF.cxx
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
#include <iostream>
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MComptonEvent.h"
#include "MERCSRToF.h"
#include "MREAMDriftChamberEnergy.h"
#include "UTERCSRShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Access to the protected functions of MERCSRToF
class UTERCSRToFAccess : public MERCSRToF
{
public:
  //! Default constructor
  UTERCSRToFAccess() : MERCSRToF() {}
  //! Default destructor
  virtual ~UTERCSRToFAccess() {}

  using MERCSRToF::ComputeQualityFactor;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the Compton sequence reconstruction with time of flight MERCSRToF
class UTERCSRToF : public UTERCSRBase
{
public:
  //! Default constructor
  UTERCSRToF() : UTERCSRBase("UTERCSRToF") {}
  //! Default destructor
  virtual ~UTERCSRToF() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the chi-square of the Compton scatter at the central hit: (cos phi by energies - cos phi by geometry)^2 divided by the squared errors
  double GetComptonChiSquare(MRESE* First, MRESE* Central, MRESE* Last) const;

  //! Test the constructor and the text of the options
  bool TestConstructionAndToString();
  //! Test the quality factor of two-site sequences
  bool TestTwoSites();
  //! Test the quality factor of three-site sequences
  bool TestThreeSites();
  //! Test the search for the sequence
  bool TestAnalyze();
  //! Test the correction of the energies with the light of a drift chamber
  bool TestDriftChamberEnergy();

  //! The test telescope
  MGeometryRevan m_Geometry;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERCSRToF::Run()
{
  bool Passed = true;

  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The test telescope can be scanned", ScanTelescopeGeometry(m_Geometry)) && Passed;

  Passed = TestConstructionAndToString() && Passed;
  if (m_Geometry.IsScanned() == true) {
    Passed = TestTwoSites() && Passed;
    Passed = TestThreeSites() && Passed;
    Passed = TestAnalyze() && Passed;
  }
  Passed = TestDriftChamberEnergy() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the chi-square of the Compton scatter at the central hit: (cos phi by energies - cos phi by geometry)^2 divided by the squared errors
double UTERCSRToF::GetComptonChiSquare(MRESE* First, MRESE* Central, MRESE* Last) const
{
  double Ee = Central->GetEnergy();
  double dEe = Central->GetEnergyResolution();
  double Eg = Last->GetEnergy();
  double dEg = Last->GetEnergyResolution();

  // Gaussian error propagation of cos phi = 1 - E0/Eg + E0/(Ee + Eg): d/dEe = -E0/(Ee + Eg)^2, d/dEg = E0/Eg^2 - E0/(Ee + Eg)^2
  double CosPhiE = MComptonEvent::ComputeCosPhiViaEeEg(Ee, Eg);
  double dCosPhiE2 = pow(c_E0/((Ee + Eg)*(Ee + Eg)), 2)*dEe*dEe + pow(c_E0/(Eg*Eg) - c_E0/((Ee + Eg)*(Ee + Eg)), 2)*dEg*dEg;
  double CosPhiA = GetCosScatterAngle(First->GetPosition(), Central->GetPosition(), Last->GetPosition());
  double dCosPhiA = GetNumericalPositionError(First, Central, Last);

  return (CosPhiE - CosPhiA)*(CosPhiE - CosPhiA)/(dCosPhiE2 + dCosPhiA*dCosPhiA);
}


////////////////////////////////////////////////////////////////////////////////


//! Test the constructor and the text of the options
bool UTERCSRToF::TestConstructionAndToString()
{
  bool Passed = true;

  MERCSRToF CSR;
  Passed = EvaluateTrue("SetParameters()", "all parameters", "The parameters are accepted", CSR.SetParameters(&m_Geometry, 0.25, 7.5, 4)) && Passed;
  MString Text = CSR.ToString();
  Passed = EvaluateTrue("ToString()", "all parameters", "The minimum quality factor is listed", Text.Contains("# QualityFactorMin:                   0.25\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The maximum quality factor is listed", Text.Contains("# QualityFactorMax:                    7.5\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The maximum number of interactions is listed", Text.Contains("# MaxNInteractions:                    4\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the quality factor of two-site sequences
bool UTERCSRToF::TestTwoSites()
{
  bool Passed = true;

  UTERCSRToFAccess CSR;
  CSR.SetParameters(&m_Geometry);

  // Tracker, then calorimeter, with the times of the speed of light
  vector<MVector> Positions = { MVector(0.0, 0.0, 3.0), MVector(1.0, 1.0, -8.0) };
  MRERawEvent* Event = CreateTimedEvent(m_Geometry, Positions, { 100.0, 400.0 });
  vector<MRESE*> Sequence = { Event->GetRESEAt(0), Event->GetRESEAt(1) };
  vector<MRESE*> Reversed = { Event->GetRESEAt(1), Event->GetRESEAt(0) };
  Passed = EvaluateNear("ComputeQualityFactor()", "two sites, true order", "The quality factor is zero", CSR.ComputeQualityFactor(Sequence), 0.0, 1E-12) && Passed;
  double ReversedChiSquare = GetTimeOfFlightChiSquare(Reversed[0], Reversed[1]);
  Passed = EvaluateNear("ComputeQualityFactor()", "two sites, reversed order", "The quality factor is 1 - chi-square probability of the time of flight", CSR.ComputeQualityFactor(Reversed), GetChiSquareCumulative(ReversedChiSquare, 1), 1E-12) && Passed;
  Passed = EvaluateTrue("ComputeQualityFactor()", "two sites, reversed order", "The reversed order is worse", CSR.ComputeQualityFactor(Reversed) > 0.99) && Passed;

  // The second hit is 150 ps late
  Event->GetRESEAt(1)->SetTime(Event->GetRESEAt(1)->GetTime() + 1.5E-10);
  double LateChiSquare = GetTimeOfFlightChiSquare(Sequence[0], Sequence[1]);
  Passed = EvaluateTrue("GetTimeOfFlightChiSquare()", "second hit 150 ps late", "The chi-square is of the order of 1", LateChiSquare > 0.1 && LateChiSquare < 2.0) && Passed;
  Passed = EvaluateNear("ComputeQualityFactor()", "second hit 150 ps late", "The quality factor is 1 - chi-square probability of the time of flight", CSR.ComputeQualityFactor(Sequence), GetChiSquareCumulative(LateChiSquare, 1), 1E-12) && Passed;

  // The energies have no influence on two-site sequences
  double Before = CSR.ComputeQualityFactor(Sequence);
  Event->GetRESEAt(0)->SetEnergy(50.0);
  Event->GetRESEAt(1)->SetEnergy(50.0);
  Passed = EvaluateNear("ComputeQualityFactor()", "two sites, other energies", "The energies have no influence", CSR.ComputeQualityFactor(Sequence), Before, 0.0) && Passed;

  // Without a time resolution of one of the hits the time of flight is not used: the quality factor is 1
  Event->GetRESEAt(1)->SetTimeResolution(0.0);
  DisableDefaultStreams();
  double NoSecondResolution = CSR.ComputeQualityFactor(Sequence);
  double NoFirstResolution = CSR.ComputeQualityFactor(Reversed);
  EnableDefaultStreams();
  Passed = EvaluateNear("ComputeQualityFactor()", "second hit without time resolution", "The quality factor is 1", NoSecondResolution, 1.0, 0.0) && Passed;
  Passed = EvaluateNear("ComputeQualityFactor()", "first hit without time resolution", "The quality factor is 1", NoFirstResolution, 1.0, 0.0) && Passed;
  delete Event;

  // Two hits in the same detector volume: the time of flight is not used
  Event = CreateTimedEvent(m_Geometry, { MVector(1.0, 1.0, -8.0), MVector(-2.0, 0.0, -12.0) }, { 100.0, 400.0 });
  Sequence = { Event->GetRESEAt(0), Event->GetRESEAt(1) };
  DisableDefaultStreams();
  double SameVolume = CSR.ComputeQualityFactor(Sequence);
  EnableDefaultStreams();
  Passed = EvaluateNear("ComputeQualityFactor()", "two hits in the calorimeter", "The quality factor is 1", SameVolume, 1.0, 0.0) && Passed;
  delete Event;

  // Two hits in different wafers of the tracker are in different detector volumes
  Event = CreateTimedEvent(m_Geometry, { MVector(0.0, 0.0, 3.0), MVector(2.0, 0.0, 1.0) }, { 100.0, 400.0 });
  Sequence = { Event->GetRESEAt(0), Event->GetRESEAt(1) };
  Passed = EvaluateNear("ComputeQualityFactor()", "two layers of the tracker, true order", "The quality factor is zero", CSR.ComputeQualityFactor(Sequence), 0.0, 1E-12) && Passed;
  delete Event;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the quality factor of three-site sequences
bool UTERCSRToF::TestThreeSites()
{
  bool Passed = true;

  UTERCSRToFAccess CSR;
  CSR.SetParameters(&m_Geometry);

  // Exact kinematics and exact times
  vector<MVector> Positions = GetThreeSitePositions();
  vector<double> Energies = GetComptonEnergies(Positions, 150.0, 300.0);
  Passed = EvaluateSize("GetComptonEnergies()", "three sites", "The energies of the sequence exist", Energies.size(), 3) && Passed;
  if (Energies.size() != 3) {
    return Passed;
  }
  MRERawEvent* Event = CreateTimedEvent(m_Geometry, Positions, Energies);
  vector<MRESE*> Sequence = { Event->GetRESEAt(0), Event->GetRESEAt(1), Event->GetRESEAt(2) };
  Passed = EvaluateNear("ComputeQualityFactor()", "three sites, exact", "The quality factor is zero", CSR.ComputeQualityFactor(Sequence), 0.0, 1E-12) && Passed;

  // Other energies and times: two times of flight and one Compton scatter, i.e. three degrees of freedom
  Event->GetRESEAt(1)->SetEnergy(Energies[1] + 4.0);
  Event->GetRESEAt(1)->SetTime(Event->GetRESEAt(1)->GetTime() + 1.0E-10);
  Event->GetRESEAt(2)->SetTime(Event->GetRESEAt(2)->GetTime() - 0.5E-10);
  double ChiSquare = GetTimeOfFlightChiSquare(Sequence[0], Sequence[1]) + GetTimeOfFlightChiSquare(Sequence[1], Sequence[2]) + GetComptonChiSquare(Sequence[0], Sequence[1], Sequence[2]);
  Passed = EvaluateTrue("GetComptonChiSquare()", "three sites, shifted", "The chi-square is of the order of 1", ChiSquare > 0.1 && ChiSquare < 10.0) && Passed;
  // The position errors of the expectation are numerical derivatives with a relative error of 1E-8
  // Defect REV-G-6: The sum of the squared energy resolutions of the hits behind the scatter is used as if it was the resolution, without the square root (here 100 keV instead of 10 keV)
  Passed = ExpectedToFail(EvaluateNear("ComputeQualityFactor()", "three sites, shifted", "The quality factor is 1 - chi-square probability for three degrees of freedom", CSR.ComputeQualityFactor(Sequence), GetChiSquareCumulative(ChiSquare, 3), 1E-7)) && Passed;
  // With a resolution of 1 keV of the last hit the missing square root has no effect
  Event->GetRESEAt(2)->SetEnergyResolution(1.0);
  ChiSquare = GetTimeOfFlightChiSquare(Sequence[0], Sequence[1]) + GetTimeOfFlightChiSquare(Sequence[1], Sequence[2]) + GetComptonChiSquare(Sequence[0], Sequence[1], Sequence[2]);
  Passed = EvaluateNear("ComputeQualityFactor()", "three sites, shifted, 1 keV resolution of the last hit", "The quality factor is 1 - chi-square probability for three degrees of freedom", CSR.ComputeQualityFactor(Sequence), GetChiSquareCumulative(ChiSquare, 3), 1E-7) && Passed;

  // The energy of the first hit has no influence
  double Before = CSR.ComputeQualityFactor(Sequence);
  Event->GetRESEAt(0)->SetEnergy(77.0);
  Passed = EvaluateNear("ComputeQualityFactor()", "three sites, other first energy", "The energy of the first hit has no influence", CSR.ComputeQualityFactor(Sequence), Before, 0.0) && Passed;

  // Without energy resolutions the Compton scatter is not used: two degrees of freedom
  Event->GetRESEAt(1)->SetEnergyResolution(0.0);
  Event->GetRESEAt(2)->SetEnergyResolution(0.0);
  DisableDefaultStreams();
  double NoEnergyResolution = CSR.ComputeQualityFactor(Sequence);
  EnableDefaultStreams();
  double TimeChiSquare = GetTimeOfFlightChiSquare(Sequence[0], Sequence[1]) + GetTimeOfFlightChiSquare(Sequence[1], Sequence[2]);
  Passed = EvaluateNear("ComputeQualityFactor()", "three sites, no energy resolutions", "The quality factor is 1 - chi-square probability of the times of flight for two degrees of freedom", NoEnergyResolution, GetChiSquareCumulative(TimeChiSquare, 2), 1E-12) && Passed;

  // Without energy in the last hit the Compton scatter is not used either
  Event->GetRESEAt(2)->SetEnergy(0.0);
  DisableDefaultStreams();
  double NoEnergy = CSR.ComputeQualityFactor(Sequence);
  EnableDefaultStreams();
  Passed = EvaluateNear("ComputeQualityFactor()", "three sites, last hit without energy", "The quality factor is 1 - chi-square probability of the times of flight for two degrees of freedom", NoEnergy, GetChiSquareCumulative(TimeChiSquare, 2), 1E-12) && Passed;

  delete Event;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search for the sequence
bool UTERCSRToF::TestAnalyze()
{
  bool Passed = true;

  MRawEventIncarnations List;
  MERCSRToF CSR;
  CSR.SetParameters(&m_Geometry);

  // Three sites with exact kinematics and times, stored in the order 2, 0, 1
  vector<MVector> Positions = GetThreeSitePositions();
  vector<double> Energies = GetComptonEnergies(Positions, 150.0, 300.0);
  if (Energies.size() != 3) {
    return Passed;
  }
  MRERawEvent* Timed = CreateTimedEvent(m_Geometry, Positions, Energies);
  MRERawEvent* Event = new MRERawEvent(&m_Geometry);
  Event->AddRESE(Timed->RemoveRESEAt(2));
  Event->AddRESE(Timed->RemoveRESEAt(0));
  Event->AddRESE(Timed->RemoveRESEAt(1));
  delete Timed;
  vector<int> Stored = GetStoredIDs(Event);
  List.SetInitialRawEvent(Event);
  Passed = EvaluateTrue("Analyze()", "three sites, exact", "The analysis returns true", CSR.Analyze(&List)) && Passed;
  vector<int> Sequence = GetSequenceIDs(Event);
  Passed = EvaluateTrue("Analyze()", "three sites, exact", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "three sites, exact", "The event is a Compton event", Event->GetEventType(), MRERawEvent::c_ComptonEvent) && Passed;
  Passed = EvaluateSize("Analyze()", "three sites, exact", "The sequence has three hits", Sequence.size(), 3) && Passed;
  if (Sequence.size() == 3) {
    Passed = EvaluateTrue("Analyze()", "three sites, exact", "The sequence is the true one", Sequence[0] == Stored[1] && Sequence[1] == Stored[2] && Sequence[2] == Stored[0]) && Passed;
  }
  Passed = EvaluateNear("Analyze()", "three sites, exact", "The first quality factor is zero", Event->GetComptonQualityFactor1(), 0.0, 1E-12) && Passed;
  Passed = EvaluateTrue("Analyze()", "three sites, exact", "The second quality factor is worse", Event->GetComptonQualityFactor2() > 0.5) && Passed;

  // From now on the energies and resolutions of the hits are fixed
  bool AllFixed = true;
  for (int r = 0; r < Event->GetNRESEs(); ++r) {
    if (Event->GetRESEAt(r)->AllowOverwrite() == true) {
      AllFixed = false;
    }
  }
  Passed = EvaluateTrue("Analyze()", "three sites, exact", "The energies and resolutions of the hits cannot be overwritten any more", AllFixed) && Passed;

  // Two sites: the earlier hit is the start, whatever the energies are
  Timed = CreateTimedEvent(m_Geometry, { MVector(1.0, 1.0, -8.0), MVector(0.0, 0.0, 3.0) }, { 100.0, 400.0 });
  List.SetInitialRawEvent(Timed);
  CSR.Analyze(&List);
  Passed = EvaluateTrue("Analyze()", "two sites, calorimeter first", "The event is good", Timed->IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "two sites, calorimeter first", "The start point is the earlier hit", Timed->GetStartPoint() == Timed->GetRESEAt(0)) && Passed;

  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the correction of the energies with the light of a drift chamber
bool UTERCSRToF::TestDriftChamberEnergy()
{
  bool Passed = true;

  // A drift chamber of 10 x 10 x 10 cm at (-30, 0, 0) cm with the light detector at its +z face and a light speed of 20 cm/ns
  MString Chamber;
  Chamber += "Volume ChamberVolume\n";
  Chamber += "ChamberVolume.Material Germanium\n";
  Chamber += "ChamberVolume.Shape BRIK 5.0 5.0 5.0\n";
  Chamber += "ChamberVolume.Position -30.0 0.0 0.0\n";
  Chamber += "ChamberVolume.Mother World\n";
  Chamber += "DriftChamber Chamber\n";
  Chamber += "Chamber.DetectorVolume ChamberVolume\n";
  Chamber += "Chamber.SensitiveVolume ChamberVolume\n";
  Chamber += "Chamber.Offset 0.0 0.0\n";
  Chamber += "Chamber.StripNumber 10 10\n";
  Chamber += "Chamber.NoiseThreshold 10\n";
  Chamber += "Chamber.TriggerThreshold 30\n";
  Chamber += "Chamber.EnergyResolution Gauss 10 10 1\n";
  Chamber += "Chamber.EnergyResolution Gauss 10000 10000 1\n";
  Chamber += "Chamber.FailureRate 0.0\n";
  Chamber += "Chamber.LightSpeed 20.0E+9\n";
  Chamber += "Chamber.LightDetectorPosition 3\n";
  Chamber += "Chamber.DriftConstant 0.25\n";
  Chamber += "Chamber.EnergyPerElectron 0.5\n";
  Chamber += "Trigger ChamberTrigger\n";
  Chamber += "ChamberTrigger.Veto false\n";
  Chamber += "ChamberTrigger.TriggerByChannel true\n";
  Chamber += "ChamberTrigger.Detector Chamber 1\n";

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope with drift chamber", "The telescope with a drift chamber can be scanned", ScanTelescopeGeometry(Geometry, Chamber)) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  // Two hits in the chamber with (100 +- 3) keV and (200 +- 4) keV, one in the calorimeter, and a light measurement of (310 +- 5) keV
  MRERawEvent* Event = new MRERawEvent(&Geometry);
  MREHit* First = CreateHit(MVector(-31.0, 1.0, 1.0), 100.0, MDDetector::c_DriftChamber, &Geometry, 5E-9);
  MREHit* Second = CreateHit(MVector(-28.0, -2.0, -3.0), 200.0, MDDetector::c_DriftChamber, &Geometry, 6E-9);
  MREHit* Outside = CreateHit(MVector(1.0, 1.0, -8.0), 300.0, MDDetector::c_Calorimeter, &Geometry, 7E-9);
  Passed = EvaluateTrue("CreateHit()", "drift chamber", "The hits are created", First != nullptr && Second != nullptr && Outside != nullptr) && Passed;
  if (First == nullptr || Second == nullptr || Outside == nullptr) {
    delete Event;
    return Passed;
  }
  First->SetEnergyResolution(3.0);
  Second->SetEnergyResolution(4.0);
  Event->AddRESE(First);
  Event->AddRESE(Second);
  Event->AddRESE(Outside);

  MREAMDriftChamberEnergy* Light = new MREAMDriftChamberEnergy();
  Light->SetEnergy(310.0);
  Light->SetEnergyResolution(5.0);
  Light->SetVolumeSequence(new MDVolumeSequence(*First->GetVolumeSequence()));
  Event->AddREAM(Light);

  MRawEventIncarnations List;
  List.SetInitialRawEvent(Event);
  MERCSRToF CSR;
  CSR.SetParameters(&Geometry);
  DisableDefaultStreams();
  CSR.Analyze(&List);
  EnableDefaultStreams();

  // Each energy is combined with the second measurement "light minus the other hits", weighted with the inverse variances:
  // First hit: 100 +- 3 and 310 - 200 = 110 +- sqrt(5^2 + 4^2): E = (41*100 + 9*110)/50 = 101.8, sigma = sqrt(9*41/50)
  // Second hit: 200 +- 4 and 310 - 100 = 210 +- sqrt(5^2 + 3^2): E = (34*200 + 16*210)/50 = 203.2, sigma = sqrt(16*34/50)
  Passed = EvaluateNear("Analyze()", "drift chamber, first hit", "The energy is the weighted average of both measurements", First->GetEnergy(), 101.8, 1E-12) && Passed;
  Passed = EvaluateNear("Analyze()", "drift chamber, second hit", "The energy is the weighted average of both measurements", Second->GetEnergy(), 203.2, 1E-12) && Passed;
  // Defect REV-G-5: The error of the weighted average multiplies the squared weight of the second measurement with the wrong variance
  Passed = ExpectedToFail(EvaluateNear("Analyze()", "drift chamber, first hit", "The energy resolution is the error of the weighted average", First->GetEnergyResolution(), sqrt(9.0*41.0/50.0), 1E-12)) && Passed;
  Passed = ExpectedToFail(EvaluateNear("Analyze()", "drift chamber, second hit", "The energy resolution is the error of the weighted average", Second->GetEnergyResolution(), sqrt(16.0*34.0/50.0), 1E-12)) && Passed;
  Passed = EvaluateNear("Analyze()", "drift chamber, hit in the calorimeter", "The energy of the hit in another detector is not changed", Outside->GetEnergy(), 300.0, 0.0) && Passed;
  Passed = EvaluateNear("Analyze()", "drift chamber, hit in the calorimeter", "The energy resolution of the hit in another detector is not changed", Outside->GetEnergyResolution(), 10.0, 1E-12) && Passed;

  // The travel time of the light to the detector at z = +5 cm of the chamber is subtracted: (5 - 1) cm/(20 cm/ns) and (5 + 3) cm/(20 cm/ns)
  Passed = EvaluateNear("Analyze()", "drift chamber, first hit", "The travel time of the light is subtracted", First->GetTime(), 5E-9 - 4.0/20.0E+9, 1E-18) && Passed;
  Passed = EvaluateNear("Analyze()", "drift chamber, second hit", "The travel time of the light is subtracted", Second->GetTime(), 6E-9 - 8.0/20.0E+9, 1E-18) && Passed;
  Passed = EvaluateNear("Analyze()", "drift chamber, hit in the calorimeter", "The time of the hit in another detector is not changed", Outside->GetTime(), 7E-9, 0.0) && Passed;

  List.DeleteAll();

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

  UTERCSRToF Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERCSRToF.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
