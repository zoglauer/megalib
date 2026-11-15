/*
 * UTDACS.cxx
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
using namespace std;

// MEGAlib libs:
#include "MDShapeSPHE.h"
#include "UTDDetectorShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the scintillator detector MDACS
class UTDACS : public UTDDetectorBase
{
public:
  //! Default constructor
  UTDACS() : UTDDetectorBase("UTDACS") {}
  //! Default destructor
  virtual ~UTDACS() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the standard fixture
  MString GetBody() const;
  //! Test the construction and the copy
  bool TestConstruction();
  //! Test the keyword parsing and validation
  bool TestKeywords();
  //! Test the discretization
  bool TestDiscretization();
  //! Test the veto decision
  bool TestVeto();
  //! Test the position resolution
  bool TestPositionResolution();
  //! Test the text output
  bool TestText();
  //! Test named detectors
  bool TestNamedDetectors();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDACS::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestKeywords() && Passed;
  Passed = TestCommonNoiseChain(GetBody(), MVector(0.5, 0.25, -0.5)) && Passed;
  Passed = TestDiscretization() && Passed;
  Passed = TestVeto() && Passed;
  Passed = TestPositionResolution() && Passed;
  Passed = TestText() && Passed;
  Passed = TestNamedDetectors() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the standard fixture
MString UTDACS::GetBody() const
{
  return GetBoxVolume("Box", 3.0, 2.0, 1.0) + "Scintillator D\nD.SensitiveVolume Box\nD.DetectorVolume Box\n";
}


////////////////////////////////////////////////////////////////////////////////


//! Test the construction and the copy
bool UTDACS::TestConstruction()
{
  bool Passed = true;

  MDACS ACS("ACS");
  Passed = EvaluateTrue("MDACS()", "name", "The name is stored", ACS.GetName() == "ACS") && Passed;
  Passed = Evaluate("MDACS()", "type", "The type is scintillator", ACS.GetType(), MDDetector::c_Scintillator) && Passed;
  Passed = EvaluateTrue("MDACS()", "description", "The type name is scintillator", ACS.GetTypeName() == MDDetector::c_ScintillatorName) && Passed;

  ACS.SetNoiseThreshold(12.0);
  MDACS Copy(ACS);
  Passed = Evaluate("MDACS(const MDACS&)", "type", "The type is copied", Copy.GetType(), MDDetector::c_Scintillator) && Passed;
  Passed = EvaluateNear("MDACS(const MDACS&)", "noise threshold", "The threshold is copied", Copy.GetNoiseThreshold(), 12.0, 1e-12) && Passed;

  Passed = CheckCloneIndependence("ACS", &ACS) && Passed;
  unique_ptr<MDDetector> Cloned(ACS.Clone());
  Passed = EvaluateTrue("Clone()", "type", "The clone is a scintillator", dynamic_cast<MDACS*>(Cloned.get()) != nullptr) && Passed;
  Passed = EvaluateNear("Clone()", "noise threshold", "The threshold is cloned", Cloned->GetNoiseThreshold(), 12.0, 1e-12) && Passed;

  MDVolume V("V");
  MDACS Direct("Direct");
  Direct.SetDetectorVolume(&V);
  Passed = EvaluateTrue("SetDetectorVolume()", "volume", "The volume is stored", Direct.GetDetectorVolume() == &V) && Passed;

  // An ACS does not need a structure
  MDVolume S("S");
  MDACS Validated("Validated");
  Validated.AddSensitiveVolume(&S);
  DisableDefaultStreams();
  bool Result = Validated.Validate();
  EnableDefaultStreams();
  Passed = EvaluateTrue("Validate()", "only one sensitive volume", "One sensitive volume is sufficient", Result) && Passed;
  MDACS Empty("Empty");
  DisableDefaultStreams();
  Result = Empty.Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "no volumes", "A detector without volumes is invalid", Result) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the keyword parsing and validation
bool UTDACS::TestKeywords()
{
  bool Passed = true;

  // The aliases of the type
  const MString Volume = GetBoxVolume("Box", 3.0, 2.0, 1.0);
  vector<MString> Aliases = { "Scintillator", "MDACS", "ACS", "Simple" };
  for (const MString& Alias: Aliases) {
    MDGeometry Geometry;
    Passed = EvaluateTrue("ScanSetupFile()", Alias, "The alias of the type is accepted", Scan(Geometry, Volume, Alias + " D\nD.SensitiveVolume Box\nD.DetectorVolume Box\n")) && Passed;
    if (CheckDetector(Passed, "Geometry", Geometry, "D", MDDetector::c_ACS)) {
      Passed = Evaluate("ScanSetupFile()", Alias, "The detector type is scintillator", Geometry.GetDetector("D")->GetType(), MDDetector::c_Scintillator) && Passed;
    }
  }

  // A scintillator does not need to be a box
  MDGeometry Sphere;
  Passed = EvaluateTrue("ScanSetupFile()", "sphere", "A sphere is accepted", Scan(Sphere, "Volume Ball\nBall.Material Silicon\nBall.Shape SPHE 0 2 0 180 0 360\nBall.Position 0 0 0\nBall.Mother World\nScintillator D\nD.SensitiveVolume Ball\nD.DetectorVolume Ball\n")) && Passed;

  // Several sensitive volumes in one detector volume
  MString Multi;
  Multi += GetBoxVolume("Container", 5.0, 5.0, 5.0);
  Multi += GetBoxVolume("Part1", 1.0, 1.0, 1.0, 2.0, "Container");
  Multi += GetBoxVolume("Part2", 1.0, 1.0, 1.0, -2.0, "Container");
  Multi += "Scintillator D\nD.SensitiveVolume Part1\nD.SensitiveVolume Part2\nD.DetectorVolume Container\n";
  MDGeometry Several;
  Passed = EvaluateTrue("ScanSetupFile()", "two sensitive volumes", "Two sensitive volumes are accepted", Scan(Several, Multi)) && Passed;
  if (CheckDetector(Passed, "Several", Several, "D", MDDetector::c_ACS)) {
    Passed = Evaluate("GetNSensitiveVolumes()", "two sensitive volumes", "Two sensitive volumes are stored", Several.GetDetector("D")->GetNSensitiveVolumes(), 2u) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the discretization
bool UTDACS::TestDiscretization()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "standard", "The fixture is accepted", Scan(Geometry, GetBody())) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDetector* D = Geometry.GetDetector("D");
  MDVolume* V = Geometry.GetVolume("Box");

  vector<MVector> Positions = { MVector(0.0, 0.0, 0.0), MVector(2.9, 1.9, 0.9), MVector(-2.9, -1.9, -0.9) };
  for (const MVector& Position: Positions) {
    MDGridPoint Point = D->GetGridPoint(Position);
    Passed = Evaluate("GetGridPoint()", "any position", "There is one voxel in x", Point.GetXGrid(), 0u) && Passed;
    Passed = Evaluate("GetGridPoint()", "any position", "There is one voxel in y", Point.GetYGrid(), 0u) && Passed;
    Passed = Evaluate("GetGridPoint()", "any position", "There is one voxel in z", Point.GetZGrid(), 0u) && Passed;
    Passed = Evaluate("GetGridPoint()", "any position", "The type is voxel", Point.GetType(), MDGridPoint::c_Voxel) && Passed;

    vector<MDGridPoint> Points = D->Discretize(Position, 123.0, 45.0, V);
    Passed = EvaluateSize("Discretize()", "any position", "There is exactly one grid point", Points.size(), 1) && Passed;
    Passed = EvaluateNear("Discretize()", "energy", "The energy is stored", Points[0].GetEnergy(), 123.0, 1e-12) && Passed;
    Passed = EvaluateNear("Discretize()", "time", "The time is stored", Points[0].GetTime(), 45.0, 1e-12) && Passed;
    Passed = EvaluateVectorNear("Discretize()", "position", "The position is the unique position of the box", Points[0].GetPosition(), V->GetShape()->GetUniquePosition(), 1e-12) && Passed;
  }

  Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "any grid point", "The position is the unique position of the shape", D->GetPositionInDetectorVolume(0, 0, 0, MVector(1.0, 1.0, 1.0), MDGridPoint::c_Voxel, V), V->GetShape()->GetUniquePosition(), 1e-12) && Passed;

  // Blocked channel
  Passed = EvaluateTrue("BlockTriggerChannel()", "channel 0, 0", "The only channel can be blocked", D->BlockTriggerChannel(0, 0)) && Passed;
  Passed = EvaluateFalse("IsAboveTriggerThreshold()", "blocked", "A blocked channel does not trigger", D->IsAboveTriggerThreshold(1000.0, D->GetGridPoint(Positions[0]))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the veto decision
bool UTDACS::TestVeto()
{
  bool Passed = true;
  gRandom->SetSeed(3001);

  MDGeometry Sharp;
  Passed = EvaluateTrue("ScanSetupFile()", "veto threshold", "The fixture is accepted", Scan(Sharp, GetBody(), "D.TriggerThreshold 50\n")) && Passed;
  MDGeometry Soft;
  Passed = EvaluateTrue("ScanSetupFile()", "soft veto threshold", "The fixture is accepted", Scan(Soft, GetBody(), "D.TriggerThreshold 50 5\n")) && Passed;
  MDGeometry Failing;
  Passed = EvaluateTrue("ScanSetupFile()", "failing veto", "The fixture is accepted", Scan(Failing, GetBody(), "D.FailureRate 0.25\n")) && Passed;
  MDGeometry AlwaysFailing;
  Passed = EvaluateTrue("ScanSetupFile()", "always failing veto", "The fixture is accepted", Scan(AlwaysFailing, GetBody(), "D.FailureRate 1.0\n")) && Passed;
  if (Passed == false) {
    return false;
  }

  MVector Position(0.0, 0.0, 0.0);
  Passed = EvaluateFalse("IsVeto()", "just below", "A hit just below the threshold does not veto", Sharp.GetDetector("D")->IsVeto(Position, 49.999)) && Passed;
  Passed = EvaluateTrue("IsVeto()", "just above", "A hit just above the threshold vetoes", Sharp.GetDetector("D")->IsVeto(Position, 50.001)) && Passed;

  // One sigma above the threshold: Phi(1)
  double Vetoes = 0.0;
  for (unsigned int i = 0; i < c_Samples; ++i) {
    if (Soft.GetDetector("D")->IsVeto(Position, 55.0) == true) {
      Vetoes += 1.0;
    }
  }
  Passed = EvaluateNear("IsVeto()", "one sigma above", "The veto fraction is Phi(1)", Vetoes/c_Samples, GetPhi(1.0), GetFractionTolerance(GetPhi(1.0))) && Passed;

  // A failing detector does not veto
  Vetoes = 0.0;
  for (unsigned int i = 0; i < c_Samples; ++i) {
    if (Failing.GetDetector("D")->IsVeto(Position, 1000.0) == true) {
      Vetoes += 1.0;
    }
  }
  Passed = EvaluateNear("IsVeto()", "failure rate 0.25", "Three quarters of the hits veto", Vetoes/c_Samples, 0.75, GetFractionTolerance(0.75)) && Passed;
  Passed = EvaluateFalse("IsVeto()", "failure rate 1", "A completely failing detector never vetoes", AlwaysFailing.GetDetector("D")->IsVeto(Position, 1000.0)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the position resolution
bool UTDACS::TestPositionResolution()
{
  bool Passed = true;

  MDGeometry Box;
  Passed = EvaluateTrue("ScanSetupFile()", "standard", "The fixture is accepted", Scan(Box, GetBody())) && Passed;
  if (Passed == false) {
    return false;
  }

  // Attention: The value for a box is not checked: half size or standard deviation is a question to the user

  // Other shapes have a zero resolution
  MDGeometry Sphere;
  Passed = EvaluateTrue("ScanSetupFile()", "sphere", "A sphere is accepted", Scan(Sphere, "Volume Ball\nBall.Material Silicon\nBall.Shape SPHE 0 2 0 180 0 360\nBall.Position 0 0 0\nBall.Mother World\nScintillator D\nD.SensitiveVolume Ball\nD.DetectorVolume Ball\n")) && Passed;
  if (CheckDetector(Passed, "Sphere", Sphere, "D", MDDetector::c_ACS)) {
    Passed = EvaluateVectorNear("GetPositionResolution()", "sphere", "The position resolution of other shapes is not supported and zero", Sphere.GetDetector("D")->GetPositionResolution(MVector(0.0, 0.0, 0.0), 100.0), MVector(0.0, 0.0, 0.0), 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output
bool UTDACS::TestText()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "standard", "The fixture is accepted", Scan(Geometry, GetBody(), "D.NoiseThreshold 10 1\nD.TriggerThreshold 20 2\nD.EnergyResolution Gauss 100 100 1\nD.EnergyResolution Gauss 200 200 2\nD.TimeResolution 100 3\nD.FailureRate 0.5\nD.Overflow 1000 5\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MDACS* D = dynamic_cast<MDACS*>(Geometry.GetDetector("D"));

  MString Text = D->GetGeomega();
  Passed = EvaluateTrue("GetGeomega()", "type", "The type is written", Text.Contains("Scintillator D")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "volumes", "The sensitive volume is written", Text.Contains("D.SensitiveVolume Box")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "volumes", "The detector volume is written", Text.Contains("D.DetectorVolume Box")) && Passed;

  // Round trip
  MDGeometry Second;
  Passed = EvaluateTrue("ScanSetupFile()", "round trip", "The text can be scanned again", Scan(Second, GetBoxVolume("Box", 3.0, 2.0, 1.0), Text)) && Passed;
  if (CheckDetector(Passed, "Second", Second, "D", MDDetector::c_ACS)) {
    Passed = Evaluate("GetGeomega()", "round trip, all fields", "The text written after the round trip is identical, so that no written field is lost", dynamic_cast<MDACS*>(Second.GetDetector("D"))->GetGeomega(), Text) && Passed;
    MDDetector* R = Second.GetDetector("D");
    Passed = EvaluateNear("GetGeomega()", "round trip", "The noise threshold is identical", R->GetNoiseThreshold(), 10.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The noise threshold sigma is identical", R->GetNoiseThresholdSigma(), 1.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The trigger threshold is identical", R->GetTriggerThreshold(), 20.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The trigger threshold sigma is identical", R->GetTriggerThresholdSigma(), 2.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The width is identical", R->GetEnergyResolutionWidth1(150.0), 1.5, 1e-9) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The time resolution is identical", R->GetTimeResolution(100.0), 3.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The failure rate is identical", R->GetFailureRate(), 0.5, 1e-12) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The overflow is identical", R->GetOverflow(), 1000.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The overflow sigma is identical", R->GetOverflowSigma(), 5.0, 1e-12) && Passed;
  }

  Passed = EvaluateTrue("ToString()", "standard", "The name is written", D->ToString().Contains("\"D\"")) && Passed;
  Passed = EvaluateTrue("ToString()", "standard", "The type is written", D->ToString().Contains("Scintillator")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test named detectors
bool UTDACS::TestNamedDetectors()
{
  bool Passed = true;

  Passed = CheckCopyDataToNamedDetectors<MDACS>("ACS") && Passed;
  gRandom->SetSeed(3002);

  MString Body = GetBody();
  Body += "D.EnergyResolution Gauss 100 100 1\nD.EnergyResolution Gauss 200 200 1\nD.TriggerThreshold 10\n";
  Body += "D.Named N\nN.Assign World.Box\nN.EnergyResolution Gauss 100 100 3\nN.EnergyResolution Gauss 200 200 3\n";

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "named detector", "The fixture is accepted", Scan(Geometry, Body)) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDetector* N = Geometry.GetDetector("N");
  MDDetector* D = Geometry.GetDetector("D");

  Passed = EvaluateTrue("CopyDataToNamedDetectors()", "named detector", "The call on the named detector is successful", N->CopyDataToNamedDetectors()) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "inherited trigger threshold", "The value is inherited", N->GetTriggerThreshold(), 10.0, 1e-12) && Passed;
  Passed = EvaluateTrue("Clone()", "named detector", "The named detector is a scintillator", dynamic_cast<MDACS*>(N) != nullptr) && Passed;

  MDVolume* V = Geometry.GetVolume("Box");
  Passed = CheckGauss("Noise()", "base detector", SampleEnergy(D, V, MVector(0.0, 0.0, 0.0), 150.0), 150.0, 1.0) && Passed;
  Passed = CheckGauss("Noise()", "named detector", SampleEnergy(N, V, MVector(0.0, 0.0, 0.0), 150.0), 150.0, 3.0) && Passed;

  // The volume needs to know the original detector and not a named one
  MString NoDetectorVolume = GetBoxVolume("Box", 3.0, 2.0, 1.0);
  NoDetectorVolume += "Scintillator D\nD.SensitiveVolume Box\nD.Named N\nN.Assign 0.0 0.0 0.0\n";
  MDGeometry Implicit;
  bool ScanResult = Scan(Implicit, NoDetectorVolume);
  Passed = EvaluateTrue("ScanSetupFile()", "named detector, implicit detector volume", "A named detector with an implicit detector volume is accepted", ScanResult) && Passed;
  if (ScanResult == true) {
    // Defect GEO-D-024: MDACS::SetDetectorVolume does not check for named detectors like the base class, thus the volume can point to the named detector during validation
    Passed = ExpectedToFail(EvaluateTrue("SetDetectorVolume()", "named detector, implicit detector volume", "The volume keeps the base detector as its detector", Implicit.GetVolume("Box")->GetDetector() == Implicit.GetDetector("D"))) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  gROOT->SetBatch(true);
  gErrorIgnoreLevel = kWarning;
  TApplication ROOT("ROOT", 0, 0);

  UTDACS Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDACS.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
