/*
 * UTDGuardRing.cxx
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
#include "UTDDetectorShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the guard ring detector MDGuardRing
class UTDGuardRing : public UTDDetectorBase
{
public:
  //! Default constructor
  UTDGuardRing() : UTDDetectorBase("UTDGuardRing") {}
  //! Default destructor
  virtual ~UTDGuardRing() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the fixture of a strip detector with a guard ring band
  MString GetStrip() const;
  //! Return the fixture of a voxel detector with a guard ring band
  MString GetVoxel() const;

  //! Test the construction, setters, and getters
  bool TestConstruction();
  //! Test the validation
  bool TestValidate();
  //! Test the guard ring without keywords
  bool TestPassive();
  //! Test the noising of an active guard ring
  bool TestActiveNoise();
  //! Test the grid functions
  bool TestGrid();
  //! Test the copy
  bool TestCopy();
  //! Test the flags of the noising
  bool TestNoiseFlags();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDGuardRing::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestValidate() && Passed;
  Passed = TestPassive() && Passed;
  Passed = TestActiveNoise() && Passed;
  Passed = TestGrid() && Passed;
  Passed = TestCopy() && Passed;
  Passed = TestNoiseFlags() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the fixture of a strip detector with a guard ring band
MString UTDGuardRing::GetStrip() const
{
  return GetBoxVolume("Box", 3.0, 2.0, 1.0) + "Strip2D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.5 0.25\nD.StripNumber 10 7\n";
}


////////////////////////////////////////////////////////////////////////////////


//! Return the fixture of a voxel detector with a guard ring band
MString UTDGuardRing::GetVoxel() const
{
  return GetBoxVolume("Box", 3.0, 2.0, 1.0) + "Voxel3D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.5 0.25 0.1\nD.Voxels 10 7 4\n";
}


////////////////////////////////////////////////////////////////////////////////


//! Test the construction, setters, and getters
bool UTDGuardRing::TestConstruction()
{
  bool Passed = true;

  MDGuardRing Ring("Ring");
  Passed = EvaluateTrue("MDGuardRing()", "name", "The name is stored", Ring.GetName() == "Ring") && Passed;
  Passed = Evaluate("MDGuardRing()", "type", "The type is guard ring", Ring.GetType(), MDDetector::c_GuardRing) && Passed;
  Passed = EvaluateTrue("MDGuardRing()", "type name", "The type name is guard ring", Ring.GetTypeName() == MDDetector::c_GuardRingName) && Passed;
  Passed = EvaluateFalse("MDGuardRing()", "active", "A guard ring is off by default", Ring.IsActive()) && Passed;
  Passed = EvaluateTrue("MDGuardRing()", "position", "The position is not defined", Ring.GetUniquePosition() == g_VectorNotDefined) && Passed;

  Ring.SetActive(true);
  Passed = EvaluateTrue("SetActive()", "true", "The flag is stored", Ring.IsActive()) && Passed;
  Ring.SetActive(false);
  Passed = EvaluateFalse("SetActive()", "false", "The flag is stored", Ring.IsActive()) && Passed;
  Ring.SetUniquePosition(MVector(1.0, 2.0, 3.0));
  Passed = EvaluateVectorNear("SetUniquePosition()", "1, 2, 3", "The position is stored", Ring.GetUniquePosition(), MVector(1.0, 2.0, 3.0), 1e-12) && Passed;

  Passed = EvaluateTrue("GetGeomega()", "any", "A guard ring is written by its mother detector", Ring.GetGeomega() == "") && Passed;

  DisableDefaultStreams();
  bool Named = Ring.CopyDataToNamedDetectors();
  EnableDefaultStreams();
  Passed = EvaluateFalse("CopyDataToNamedDetectors()", "any", "A guard ring cannot have named detectors", Named) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the validation
bool UTDGuardRing::TestValidate()
{
  bool Passed = true;

  auto ValidateQuiet = [this](MDDetector& Detector) {
    DisableDefaultStreams();
    bool Result = Detector.Validate();
    EnableDefaultStreams();
    return Result;
  };

  MDGuardRing NoMother("NoMother");
  Passed = EvaluateFalse("Validate()", "no mother detector", "A guard ring needs a mother detector", ValidateQuiet(NoMother)) && Passed;

  // Mother without sensitive volumes
  // Defect GEO-D-070: The validation accesses the first sensitive volume without a check if the mother detector has any
  MDStrip2D EmptyMother("Mother");
  MDGuardRing EmptyRing("Ring");
  EmptyRing.SetMotherDetector(&EmptyMother);
  EmptyRing.SetUniquePosition(MVector(0.0, 0.0, 0.0));
  Passed = ExpectedToCrash(EvaluateFalse("Validate()", "mother without sensitive volumes", "The validation fails without a crash", EmptyRing.Validate())) && Passed;

  // No position
  MDStrip2D Mother("Mother");
  MDVolume V("V");
  Mother.AddSensitiveVolume(&V);
  MDGuardRing NoPosition("NoPosition");
  NoPosition.SetMotherDetector(&Mother);
  Passed = EvaluateFalse("Validate()", "no unique position", "A guard ring needs a unique position", ValidateQuiet(NoPosition)) && Passed;

  // Passive guard ring: the settings make sure no energy is ever measured
  MDGuardRing Passive("Passive");
  Passive.SetMotherDetector(&Mother);
  Passive.SetUniquePosition(MVector(1.0, 2.0, 3.0));
  Passed = EvaluateTrue("Validate()", "passive guard ring", "The guard ring is valid", ValidateQuiet(Passive)) && Passed;
  Passed = Evaluate("Validate()", "passive guard ring", "The sensitive volumes are copied from the mother", Passive.GetNSensitiveVolumes(), 1u) && Passed;
  Passed = EvaluateTrue("Validate()", "passive guard ring", "The first sensitive volume is the detector volume", Passive.GetDetectorVolume() == &V) && Passed;
  Passed = EvaluateTrue("Validate()", "passive guard ring", "The volume keeps the mother as its detector", V.GetDetector() == &Mother) && Passed;
  Passed = EvaluateNear("Validate()", "passive guard ring", "The energy resolution type is none", Passive.GetEnergyResolutionType(), MDDetector::c_EnergyResolutionTypeNone, 1e-12) && Passed;
  Passed = EvaluateTrue("Validate()", "passive guard ring", "The trigger threshold is huge", Passive.GetTriggerThreshold() > 1E300) && Passed;
  Passed = EvaluateTrue("Validate()", "passive guard ring", "There is only one threshold", Passive.GetNoiseThresholdEqualsTriggerThreshold()) && Passed;

  // A second validation does not add anything
  Passed = EvaluateTrue("Validate()", "second validation", "The guard ring is still valid", ValidateQuiet(Passive)) && Passed;
  Passed = Evaluate("Validate()", "second validation", "The sensitive volumes are not added twice", Passive.GetNSensitiveVolumes(), 1u) && Passed;

  // Active guard ring
  MDGuardRing Active("Active");
  Active.SetMotherDetector(&Mother);
  Active.SetUniquePosition(MVector(1.0, 2.0, 3.0));
  Active.SetActive(true);
  Passed = EvaluateTrue("Validate()", "active guard ring", "The guard ring is valid", ValidateQuiet(Active)) && Passed;
  Passed = EvaluateNear("Validate()", "active guard ring", "The energy resolution type is ideal", Active.GetEnergyResolutionType(), MDDetector::c_EnergyResolutionTypeIdeal, 1e-12) && Passed;
  Passed = EvaluateNear("Validate()", "active guard ring", "The trigger threshold is zero", Active.GetTriggerThreshold(), 0.0, 1e-12) && Passed;

  // Keywords
  Passed = EvaluateFalse("ScanSetupFile()", "one energy resolution point", "One point of the energy resolution is rejected", IsScanOK(GetStrip(), "D.GuardRingEnergyResolution 100 1\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "two energy resolution points", "Two points of the energy resolution are accepted", IsScanOK(GetStrip(), "D.GuardRingEnergyResolution 100 1\nD.GuardRingEnergyResolution 200 1\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "other spelling", "The other spelling is accepted", IsScanOK(GetStrip(), "D.GuardringEnergyResolution 100 1\nD.GuardringEnergyResolutionAt 200 1\nD.GuardringTriggerThreshold 10 1\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "voxel detector", "The keywords work for a voxel detector", IsScanOK(GetVoxel(), "D.GuardRingEnergyResolution 100 1\nD.GuardRingEnergyResolution 200 1\nD.GuardRingTriggerThreshold 10 1\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "GuardRingTriggerThreshold with too many values", "The wrong number of tokens is rejected", IsScanOK(GetStrip(), "D.GuardRingEnergyResolution 100 1\nD.GuardRingEnergyResolution 200 1\nD.GuardRingTriggerThreshold 10 1 1\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "GuardRingEnergyResolution with too many values", "The wrong number of tokens is rejected", IsScanOK(GetStrip(), "D.GuardRingEnergyResolution 100 1 1\nD.GuardRingEnergyResolution 200 1\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "keyword for a calorimeter", "A calorimeter has no guard ring", IsScanOK(GetBoxVolume("Box", 3.0, 2.0, 1.0), "Calorimeter D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.GuardRingTriggerThreshold 10\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the guard ring without keywords
bool UTDGuardRing::TestPassive()
{
  bool Passed = true;
  MString Flags;

  vector<MString> Bodies = { GetStrip(), GetVoxel() };
  for (const MString& Body: Bodies) {
    MDGeometry Geometry;
    Passed = EvaluateTrue("ScanSetupFile()", "no guard ring keywords", "The fixture is accepted", Scan(Geometry, Body)) && Passed;
    if (Geometry.GetDetector("D") == nullptr) {
      continue;
    }
    MDGuardRing* Ring = Geometry.GetDetector("D")->GetGuardRing();
    Passed = EvaluateTrue("GetGuardRing()", "no guard ring keywords", "A guard ring is created", Ring != nullptr) && Passed;
    if (Ring == nullptr) {
      continue;
    }
    Passed = EvaluateFalse("IsActive()", "no guard ring keywords", "The guard ring is passive", Ring->IsActive()) && Passed;

    // Attention: The volume can be a nullptr as in the callers of the guard ring noise
    MVector Position(1.0, 2.0, 3.0);
    double Energy = 1000.0;
    double Time = 5.0;
    Ring->Noise(Position, Energy, Time, Flags, nullptr);
    Passed = EvaluateNear("Noise()", "passive guard ring", "No energy is measured", Energy, 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Noise()", "passive guard ring", "The time stays", Time, 5.0, 1e-12) && Passed;
    Passed = EvaluateVectorNear("Noise()", "passive guard ring", "The position stays", Position, MVector(1.0, 2.0, 3.0), 1e-12) && Passed;

    // Also if noising is deactivated
    Ring->ActivateNoising(false);
    Energy = 1000.0;
    Ring->Noise(Position, Energy, Time, Flags, nullptr);
    Passed = EvaluateNear("Noise()", "passive guard ring, noise deactivated", "No energy is measured", Energy, 0.0, 1e-12) && Passed;

    // A trigger threshold beyond all energies, but the hit does not arrive there since the energy is zero
    Passed = EvaluateTrue("GetNoiseThresholdEqualsTriggerThreshold()", "passive guard ring", "There is only one threshold", Ring->GetNoiseThresholdEqualsTriggerThreshold()) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the noising of an active guard ring
bool UTDGuardRing::TestActiveNoise()
{
  bool Passed = true;
  gRandom->SetSeed(10001);

  const MString Resolution = "D.GuardRingEnergyResolution 100 2\nD.GuardRingEnergyResolution 500 6\n";

  vector<MString> Bodies = { GetStrip(), GetVoxel() };
  for (const MString& Body: Bodies) {
    MDGeometry Geometry;
    Passed = EvaluateTrue("ScanSetupFile()", "active guard ring", "The fixture is accepted", Scan(Geometry, Body, Resolution)) && Passed;
    if (Geometry.GetDetector("D") == nullptr) {
      continue;
    }
    MDGuardRing* Ring = Geometry.GetDetector("D")->GetGuardRing();
    Passed = EvaluateTrue("IsActive()", "active guard ring", "The guard ring is active", Ring->IsActive()) && Passed;
    Passed = EvaluateTrue("GetDetector()", "active guard ring", "The guard ring is in the list of detectors", Geometry.GetDetector("D_GuardRing") == Ring) && Passed;
    Passed = EvaluateNear("GetEnergyResolutionType()", "active guard ring", "The type is Gauss", Ring->GetEnergyResolutionType(), MDDetector::c_EnergyResolutionTypeGauss, 1e-12) && Passed;

    // Peak identical to the energy, width interpolated: 2 + (E - 100)/100
    MVector Null(0.0, 0.0, 0.0);
    Passed = CheckGauss("Noise()", "active at the first table point", SampleEnergy(Ring, nullptr, Null, 100.0), 100.0, 2.0) && Passed;
    Passed = CheckGauss("Noise()", "active between the table points", SampleEnergy(Ring, nullptr, Null, 300.0), 300.0, 4.0) && Passed;
    Passed = CheckGauss("Noise()", "active above the table", SampleEnergy(Ring, nullptr, Null, 900.0), 900.0, 10.0) && Passed;
    Passed = EvaluateNear("GetEnergyResolution()", "active at 300 keV", "The width is interpolated", Ring->GetEnergyResolution(300.0), 4.0, 1e-9) && Passed;

    // Failure rate
    Ring->SetFailureRate(0.25);
    Passed = EvaluateNear("Noise()", "failure rate 0.25", "Three quarters of the hits survive", GetFractionAbove(SampleEnergy(Ring, nullptr, Null, 300.0), 0.0), 0.75, GetFractionTolerance(0.75)) && Passed;
    Ring->SetFailureRate(0.0);

    // Overflow
    Ring->SetOverflow(200.0);
    Ring->SetOverflowSigma(0.0);
    Passed = EvaluateNear("Noise()", "overflow", "The energy is the overflow value", NoiseOnce(Ring, nullptr, Null, 1000.0), 200.0, 1e-9) && Passed;
    Ring->SetOverflow(1E20);

    // Time
    Ring->SetTimeResolution(100.0, 3.0);
    Passed = CheckGauss("Noise()", "time", SampleTime(Ring, nullptr, Null, 300.0, 50.0), 50.0, 3.0) && Passed;

    // Calibration by hand: E' = 2 E, for a width of zero below
    MFunction Calibration;
    Calibration.Add(0.0, 0.0);
    Calibration.Add(1000.0, 2000.0);
    Ring->SetEnergyCalibration(Calibration);
    Passed = EvaluateNear("Noise()", "calibration", "The mean is calibrated", MStatistics::GetMean(SampleEnergy(Ring, nullptr, Null, 300.0)), 600.0, GetMeanTolerance(8.0)) && Passed;

    // No noising
    Ring->ActivateNoising(false);
    Passed = EvaluateNear("Noise()", "active, noise deactivated", "The energy stays", NoiseOnce(Ring, nullptr, Null, 300.0), 300.0, 1e-12) && Passed;
  }

  // Sharp threshold - a width of zero
  MDGeometry Sharp;
  Passed = EvaluateTrue("ScanSetupFile()", "sharp threshold", "The fixture is accepted", Scan(Sharp, GetStrip(), "D.GuardRingEnergyResolution 100 0\nD.GuardRingEnergyResolution 500 0\nD.GuardRingTriggerThreshold 30\n")) && Passed;
  if (CheckDetector(Passed, "Sharp", Sharp, "D", MDDetector::c_Strip2D)) {
    MDGuardRing* Ring = Sharp.GetDetector("D")->GetGuardRing();
    MVector Null(0.0, 0.0, 0.0);
    Passed = EvaluateNear("GetTriggerThreshold()", "sharp threshold", "The value is read", Ring->GetTriggerThreshold(), 30.0, 1e-12) && Passed;
    Passed = EvaluateTrue("GetNoiseThresholdEqualsTriggerThreshold()", "sharp threshold", "There is only one threshold", Ring->GetNoiseThresholdEqualsTriggerThreshold()) && Passed;
    Passed = EvaluateNear("Noise()", "just below the threshold", "The hit is removed", NoiseOnce(Ring, nullptr, Null, 29.999), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Noise()", "just above the threshold", "The hit stays", NoiseOnce(Ring, nullptr, Null, 30.001), 30.001, 1e-9) && Passed;
    Passed = EvaluateTrue("IsAboveTriggerThreshold()", "remaining hit", "Every remaining hit triggers", Ring->IsAboveTriggerThreshold(30.001, Ring->GetGridPoint(Null))) && Passed;
  }

  // Soft threshold: one sigma above survives with Phi(1)
  MDGeometry Soft;
  Passed = EvaluateTrue("ScanSetupFile()", "soft threshold", "The fixture is accepted", Scan(Soft, GetStrip(), "D.GuardRingEnergyResolution 100 0\nD.GuardRingEnergyResolution 500 0\nD.GuardRingTriggerThreshold 30 3\n")) && Passed;
  if (CheckDetector(Passed, "Soft", Soft, "D", MDDetector::c_Strip2D)) {
    MDGuardRing* Ring = Soft.GetDetector("D")->GetGuardRing();
    Passed = EvaluateNear("GetTriggerThresholdSigma()", "soft threshold", "The value is read", Ring->GetTriggerThresholdSigma(), 3.0, 1e-12) && Passed;
    Passed = EvaluateNear("Noise()", "one sigma above the soft threshold", "The surviving fraction is Phi(1)", GetFractionAbove(SampleEnergy(Ring, nullptr, MVector(0.0, 0.0, 0.0), 33.0), 0.0), GetPhi(1.0), GetFractionTolerance(GetPhi(1.0))) && Passed;
  }

  // Only the guard ring is active: the strip detector has its own settings
  MDGeometry Mixed;
  Passed = EvaluateTrue("ScanSetupFile()", "strip and guard ring settings", "The fixture is accepted", Scan(Mixed, GetStrip(), "D.NoiseThreshold 100\nD.GuardRingEnergyResolution 100 0\nD.GuardRingEnergyResolution 500 0\nD.GuardRingTriggerThreshold 30\n")) && Passed;
  if (CheckDetector(Passed, "Mixed", Mixed, "D", MDDetector::c_Strip2D)) {
    MDDetector* D = Mixed.GetDetector("D");
    MVector Null(0.0, 0.0, 0.0);
    Passed = EvaluateNear("Noise()", "50 keV in the strip detector", "The threshold of the strip detector removes the hit", NoiseOnce(D, Mixed.GetVolume("Box"), Null, 50.0), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Noise()", "50 keV in the guard ring", "The hit stays in the guard ring", NoiseOnce(D->GetGuardRing(), nullptr, Null, 50.0), 50.0, 1e-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the grid functions
bool UTDGuardRing::TestGrid()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "strip", "The fixture is accepted", Scan(Geometry, GetStrip(), "D.GuardRingEnergyResolution 100 1\nD.GuardRingEnergyResolution 200 1\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MDGuardRing* Ring = Geometry.GetDetector("D")->GetGuardRing();
  MDVolume* V = Geometry.GetVolume("Box");
  MVector Unique(-2.75, -1.875, 0.0);

  Passed = EvaluateVectorNear("GetUniquePosition()", "strip", "The position is in the middle of the guard ring band", Ring->GetUniquePosition(), Unique, 1e-9) && Passed;

  MDGridPoint Point = Ring->GetGridPoint(MVector(0.0, 0.0, 0.0));
  Passed = Evaluate("GetGridPoint()", "any position", "The type is guard ring", Point.GetType(), MDGridPoint::c_GuardRing) && Passed;
  Passed = Evaluate("GetGridPoint()", "any position", "The x-grid is zero", Point.GetXGrid(), 0u) && Passed;
  Passed = Evaluate("GetGridPoint()", "any position", "The y-grid is zero", Point.GetYGrid(), 0u) && Passed;
  Passed = Evaluate("GetGridPoint()", "any position", "The z-grid is zero", Point.GetZGrid(), 0u) && Passed;

  vector<MDGridPoint> Points = Ring->Grid(MVector(1.0, 1.0, 1.0), 123.0, 45.0, V);
  Passed = EvaluateSize("Grid()", "any position", "There is one grid point", Points.size(), 1) && Passed;
  if (Points.size() == 1) {
    Passed = Evaluate("Grid()", "any position", "The type is guard ring", Points[0].GetType(), MDGridPoint::c_GuardRing) && Passed;
    Passed = EvaluateVectorNear("Grid()", "any position", "The position is the unique one", Points[0].GetPosition(), Unique, 1e-9) && Passed;
    Passed = EvaluateNear("Grid()", "any position", "The energy is stored", Points[0].GetEnergy(), 123.0, 1e-12) && Passed;
    Passed = EvaluateNear("Grid()", "any position", "The time is stored", Points[0].GetTime(), 45.0, 1e-12) && Passed;
  }

  Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "any grid point", "The position is the unique one", Ring->GetPositionInDetectorVolume(3, 3, 3, MVector(1.0, 1.0, 1.0), MDGridPoint::c_GuardRing, V), Unique, 1e-9) && Passed;

  // Text
  Passed = EvaluateTrue("ToString()", "strip", "The name is written", Ring->ToString().Contains("D_GuardRing")) && Passed;
  Passed = EvaluateTrue("ToString()", "strip", "The type is written", Ring->ToString().Contains("GuardRing")) && Passed;

  // Detector volume with several wafers: wafer half size 1.5 x 1 x 0.5, gap 0.2, center of the first wafer at -1.6, offset 0.5 x 0.25
  MString Many = GetBoxVolume("Container", 3.1, 1.0, 0.5);
  // The wafer is the template of the two copies, and is not placed itself
  Many += "Volume Wafer\nWafer.Material Silicon\nWafer.Shape BRIK 1.5 1.0 0.5\n";
  Many += "Wafer.Copy WaferA\nWaferA.Position -1.6 0.0 0.0\nWaferA.Mother Container\n";
  Many += "Wafer.Copy WaferB\nWaferB.Position 1.6 0.0 0.0\nWaferB.Mother Container\n";
  Many += "Strip2D D\nD.SensitiveVolume Wafer\nD.DetectorVolume Container\nD.StructuralPitch 0.2 0.0 0.0\nD.StructuralOffset 0.0 0.0 0.0\nD.StripNumber 3 2\nD.Offset 0.5 0.25\n";
  MDGeometry Wafers;
  Passed = EvaluateTrue("ScanSetupFile()", "several wafers", "The fixture is accepted", Scan(Wafers, Many)) && Passed;
  if (CheckDetector(Passed, "Wafers", Wafers, "D", MDDetector::c_Strip2D)) {
    MDDetector* D = Wafers.GetDetector("D");
    MVector ByMother = D->GetPositionInDetectorVolume(0, 0, 0, MVector(0.0, 0.0, 0.0), MDGridPoint::c_GuardRing, Wafers.GetVolume("Container"));
    // By hand: band of the first wafer -3.1 + 0.25, -1 + 0.125
    Passed = EvaluateVectorNear("MDStrip2D::GetPositionInDetectorVolume()", "several wafers", "The position in the guard ring of the first wafer is returned", ByMother, MVector(-2.85, -0.875, 0.0), 1e-9) && Passed;
    // Defect GEO-D-071: The unique position is relative to the wafer center, but the guard ring returns it as position in the detector volume, different from the mother detector
    Passed = ExpectedToFail(EvaluateVectorNear("GetPositionInDetectorVolume()", "several wafers", "The guard ring returns the same position as its mother", D->GetGuardRing()->GetPositionInDetectorVolume(0, 0, 0, MVector(0.0, 0.0, 0.0), MDGridPoint::c_GuardRing, Wafers.GetVolume("Container")), MVector(-2.85, -0.875, 0.0), 1e-9)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the copy
bool UTDGuardRing::TestCopy()
{
  bool Passed = true;

  MDStrip2D Mother("Mother");
  MDVolume V("V");
  Mother.AddSensitiveVolume(&V);

  MDGuardRing Ring("Ring");
  Ring.SetMotherDetector(&Mother);
  Ring.SetActive(true);
  Ring.SetUniquePosition(MVector(1.0, 2.0, 3.0));
  Ring.SetTriggerThreshold(30.0);

  MDGuardRing Copy(Ring);
  Passed = EvaluateTrue("MDGuardRing(const MDGuardRing&)", "active", "The flag is copied", Copy.IsActive()) && Passed;
  Passed = EvaluateVectorNear("MDGuardRing(const MDGuardRing&)", "position", "The position is copied", Copy.GetUniquePosition(), MVector(1.0, 2.0, 3.0), 1e-12) && Passed;
  Passed = EvaluateNear("MDGuardRing(const MDGuardRing&)", "trigger threshold", "The threshold is copied", Copy.GetTriggerThreshold(), 30.0, 1e-12) && Passed;
  // Documented in the header: two detectors cannot share a guard ring, so the mother is not copied
  DisableDefaultStreams();
  bool Result = Copy.Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("MDGuardRing(const MDGuardRing&)", "mother", "The copy has no mother detector and is thus invalid until a mother is set", Result) && Passed;

  Passed = CheckCloneIndependence("GuardRing", &Ring) && Passed;
  unique_ptr<MDDetector> Cloned(Ring.Clone());
  MDGuardRing* Clone = dynamic_cast<MDGuardRing*>(Cloned.get());
  if (EvaluateTrue("Clone()", "guard ring", "The clone is a guard ring", Clone != nullptr) == false) {
    return false;
  }
  Passed = EvaluateTrue("Clone()", "guard ring", "The flag is cloned", Clone->IsActive()) && Passed;
  Clone->SetMotherDetector(&Mother);
  DisableDefaultStreams();
  Result = Clone->Validate();
  EnableDefaultStreams();
  Passed = EvaluateTrue("SetMotherDetector()", "clone with a new mother", "The clone with a new mother is valid", Result) && Passed;

  // The mother detector has its own guard ring in its copy: covered by UTDStrip2D and UTDVoxel3D
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the flags of the noising
bool UTDGuardRing::TestNoiseFlags()
{
  bool Passed = true;
  gRandom->SetSeed(6006);

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "flags", "The fixture is accepted", Scan(Geometry, GetStrip(), "D.GuardRingEnergyResolution 100 0\nD.GuardRingEnergyResolution 500 0\nD.GuardRingTriggerThreshold 30\n")) && Passed;
  if (CheckDetector(Passed, "Flags", Geometry, "D", MDDetector::c_Strip2D) == false) {
    return false;
  }
  MDDetector* Ring = Geometry.GetDetector("D")->GetGuardRing();
  if (EvaluateTrue("GetGuardRing()", "flags", "The detector has a guard ring", Ring != nullptr) == false) {
    return false;
  }
  MVector Null(0.0, 0.0, 0.0);

  Ring->SetOverflow(200.0);
  MString Normal = NoiseFlagsOnce(Ring, nullptr, Null, 100.0);
  Passed = EvaluateTrue("Noise()", "flags, 100 keV with an overflow of 200 keV", "There are no flags", Normal.IsEmpty()) && Passed;
  // Defect GEO-D-075: The guard ring does not flag failures and overflows as the other detectors do
  Passed = ExpectedToFail(EvaluateTrue("Noise()", "flags, 1000 keV with an overflow of 200 keV", "The overflow is flagged", NoiseFlagsOnce(Ring, nullptr, Null, 1000.0).Contains(" OVERFLOW"))) && Passed;

  Ring->SetFailureRate(1.0);
  Passed = EvaluateNear("Noise()", "failure rate 1", "The hit is removed", NoiseOnce(Ring, nullptr, Null, 100.0), 0.0, 1e-12) && Passed;
  // Defect GEO-D-075: The guard ring does not flag failures and overflows as the other detectors do
  Passed = ExpectedToFail(EvaluateTrue("Noise()", "flags, failure rate 1", "The failure is flagged", NoiseFlagsOnce(Ring, nullptr, Null, 100.0) == " FAILURE")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  gROOT->SetBatch(true);
  gErrorIgnoreLevel = kWarning;
  TApplication ROOT("ROOT", 0, 0);

  UTDGuardRing Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDGuardRing.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
