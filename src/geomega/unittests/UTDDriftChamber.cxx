/*
 * UTDDriftChamber.cxx
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


//! Unit test of the drift chamber MDDriftChamber
class UTDDriftChamber : public UTDDetectorBase
{
public:
  //! Default constructor
  UTDDriftChamber() : UTDDetectorBase("UTDDriftChamber") {}
  //! Default destructor
  virtual ~UTDDriftChamber() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the standard fixture
  MString GetBody() const;

  //! Test the construction, setters, and getters
  bool TestConstruction();
  //! Test the keyword parsing and validation
  bool TestKeywords();
  //! Test the light travel time
  bool TestLightTravelTime();
  //! Test the discretization without drift
  bool TestDiscretizeWithoutDrift();
  //! Test the discretization with drift
  bool TestDiscretizeWithDrift();
  //! Test the light energy resolution
  bool TestLightEnergyResolution();
  //! Test copy, named detectors, and text output
  bool TestCopyNamedText();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDDriftChamber::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestKeywords() && Passed;
  Passed = TestCommonNoiseChain(GetBody(), MVector(0.25, 0.25, 0.5)) && Passed;
  Passed = TestLightTravelTime() && Passed;
  Passed = TestDiscretizeWithoutDrift() && Passed;
  Passed = TestDiscretizeWithDrift() && Passed;
  Passed = TestLightEnergyResolution() && Passed;
  Passed = TestCopyNamedText() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the standard fixture
MString UTDDriftChamber::GetBody() const
{
  return GetBoxVolume("Box", 3.0, 2.0, 1.0) + "DriftChamber D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.5 0.25\nD.StripNumber 10 7\n";
}


////////////////////////////////////////////////////////////////////////////////


//! Test the construction, setters, and getters
bool UTDDriftChamber::TestConstruction()
{
  bool Passed = true;

  MDDriftChamber Chamber("Chamber");
  Passed = EvaluateTrue("MDDriftChamber()", "name", "The name is stored", Chamber.GetName() == "Chamber") && Passed;
  Passed = Evaluate("MDDriftChamber()", "type", "The type is drift chamber", Chamber.GetType(), MDDetector::c_DriftChamber) && Passed;
  Passed = EvaluateTrue("MDDriftChamber()", "type name", "The type name is drift chamber", Chamber.GetTypeName() == MDDetector::c_DriftChamberName) && Passed;
  Passed = EvaluateNear("MDDriftChamber()", "light speed", "The value is not defined", Chamber.GetLightSpeed(), g_DoubleNotDefined, 1e-9*g_DoubleNotDefined) && Passed;
  Passed = Evaluate("MDDriftChamber()", "light detector", "The value is not defined", Chamber.GetLightDetectorPosition(), g_IntNotDefined) && Passed;
  Passed = EvaluateNear("MDDriftChamber()", "drift constant", "The value is not defined", Chamber.GetDriftConstant(), g_DoubleNotDefined, 1e-9*g_DoubleNotDefined) && Passed;
  Passed = EvaluateNear("MDDriftChamber()", "energy per electron", "The value is not defined", Chamber.GetEnergyPerElectron(), g_DoubleNotDefined, 1e-9*g_DoubleNotDefined) && Passed;

  Chamber.SetLightSpeed(1.5);
  Chamber.SetLightDetectorPosition(-2);
  Chamber.SetDriftConstant(2.5);
  Chamber.SetEnergyPerElectron(3.5);
  Passed = EvaluateNear("SetLightSpeed()", "1.5", "The value is stored", Chamber.GetLightSpeed(), 1.5, 1e-12) && Passed;
  Passed = Evaluate("SetLightDetectorPosition()", "-2", "The value is stored", Chamber.GetLightDetectorPosition(), -2) && Passed;
  Passed = EvaluateNear("SetDriftConstant()", "2.5", "The value is stored", Chamber.GetDriftConstant(), 2.5, 1e-12) && Passed;
  Passed = EvaluateNear("SetEnergyPerElectron()", "3.5", "The value is stored", Chamber.GetEnergyPerElectron(), 3.5, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the keyword parsing and validation
bool UTDDriftChamber::TestKeywords()
{
  bool Passed = true;

  MDGeometry Defaults;
  Passed = EvaluateTrue("ScanSetupFile()", "minimal", "A minimal detector is accepted", Scan(Defaults, GetBody())) && Passed;
  if (CheckDetector(Passed, "Defaults", Defaults, "D", MDDetector::c_DriftChamber)) {
    MDDriftChamber* D = dynamic_cast<MDDriftChamber*>(Defaults.GetDetector("D"));
    Passed = EvaluateNear("Validate()", "default light speed", "The default is the speed of light", D->GetLightSpeed(), c_SpeedOfLight, 1.0) && Passed;
    Passed = Evaluate("Validate()", "default light detector", "The default is no light detector", D->GetLightDetectorPosition(), 0) && Passed;
    Passed = EvaluateNear("Validate()", "default drift constant", "The default is no drift", D->GetDriftConstant(), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Validate()", "default energy per electron", "The default is 22 eV", D->GetEnergyPerElectron(), 0.022, 1e-12) && Passed;
  }

  MDGeometry Values;
  Passed = EvaluateTrue("ScanSetupFile()", "all values", "The fixture is accepted", Scan(Values, GetBody(), "D.LightSpeed 1.5\nD.LightDetectorPosition 3\nD.DriftConstant 0.25\nD.EnergyPerElectron 0.5\n")) && Passed;
  if (CheckDetector(Passed, "Values", Values, "D", MDDetector::c_DriftChamber)) {
    MDDriftChamber* D = dynamic_cast<MDDriftChamber*>(Values.GetDetector("D"));
    Passed = EvaluateNear("ScanSetupFile()", "LightSpeed", "The value is read", D->GetLightSpeed(), 1.5, 1e-12) && Passed;
    Passed = Evaluate("ScanSetupFile()", "LightDetectorPosition", "The value is read", D->GetLightDetectorPosition(), 3) && Passed;
    Passed = EvaluateNear("ScanSetupFile()", "DriftConstant", "The value is read", D->GetDriftConstant(), 0.25, 1e-12) && Passed;
    Passed = EvaluateNear("ScanSetupFile()", "EnergyPerElectron", "The value is read", D->GetEnergyPerElectron(), 0.5, 1e-12) && Passed;
  }

  vector<MString> Wrong = {
    "D.LightSpeed\n", "D.LightSpeed 1 2\n",
    "D.LightDetectorPosition\n", "D.LightDetectorPosition 1 2\n",
    "D.DriftConstant\n", "D.DriftConstant 1 2\n",
    "D.EnergyPerElectron\n", "D.EnergyPerElectron 1 2\n",
    "D.LightEnergyResolution 1\n", "D.LightEnergyResolution 1 2 3\n",
    "D.Voxels 2 2 2\n",
    "D.DirectionalResolution 100 1\n"
  };
  for (const MString& Keyword: Wrong) {
    Passed = EvaluateFalse("ScanSetupFile()", Keyword, "The wrong keyword is rejected", IsScanOK(GetBody(), Keyword)) && Passed;
  }
  Passed = EvaluateFalse("ScanSetupFile()", "no strips", "A missing strip number is rejected", IsScanOK(GetBoxVolume("Box", 3.0, 2.0, 1.0), "DriftChamber D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.5 0.25\n")) && Passed;

  // Defect GEO-D-050: The light detector position is not validated, the values are only checked at each hit
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "light detector position 7", "An unknown light detector position is rejected", IsScanOK(GetBody(), "D.LightDetectorPosition 7\n"))) && Passed;
  // Defect GEO-D-051: The light speed and the energy per electron are not validated
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "light speed 0", "A light speed of zero is rejected", IsScanOK(GetBody(), "D.LightSpeed 0\n"))) && Passed;
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "energy per electron 0", "An energy per electron of zero is rejected", IsScanOK(GetBody(), "D.EnergyPerElectron 0\n"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the light travel time
bool UTDDriftChamber::TestLightTravelTime()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "light detector", "The fixture is accepted", Scan(Geometry, GetBody(), "D.LightSpeed 2.0\nD.LightDetectorPosition 1\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDriftChamber* D = dynamic_cast<MDDriftChamber*>(Geometry.GetDetector("D"));
  MVector Position(1.0, 0.5, 0.25);

  // Distance to the plane of the light detector divided by the speed, half size 3 x 2 x 1
  D->SetLightDetectorPosition(0);
  Passed = EvaluateNear("GetLightTravelTime()", "no light detector", "The time is zero", D->GetLightTravelTime(Position), 0.0, 1e-12) && Passed;
  D->SetLightDetectorPosition(1);
  Passed = EvaluateNear("GetLightTravelTime()", "at +x", "The distance is 3 - 1", D->GetLightTravelTime(Position), 1.0, 1e-12) && Passed;
  D->SetLightDetectorPosition(2);
  Passed = EvaluateNear("GetLightTravelTime()", "at +y", "The distance is 2 - 0.5", D->GetLightTravelTime(Position), 0.75, 1e-12) && Passed;
  D->SetLightDetectorPosition(3);
  Passed = EvaluateNear("GetLightTravelTime()", "at +z", "The distance is 1 - 0.25", D->GetLightTravelTime(Position), 0.375, 1e-12) && Passed;
  // Defect GEO-D-052: The light travel time to a light detector on the negative side uses the wrong sign of the half size and is thus negative
  D->SetLightDetectorPosition(-1);
  Passed = ExpectedToFail(EvaluateNear("GetLightTravelTime()", "at -x", "The distance is 1 + 3", D->GetLightTravelTime(Position), 2.0, 1e-12)) && Passed;
  D->SetLightDetectorPosition(-2);
  Passed = ExpectedToFail(EvaluateNear("GetLightTravelTime()", "at -y", "The distance is 0.5 + 2", D->GetLightTravelTime(Position), 1.25, 1e-12)) && Passed;
  D->SetLightDetectorPosition(-3);
  Passed = ExpectedToFail(EvaluateNear("GetLightTravelTime()", "at -z", "The distance is 0.25 + 1", D->GetLightTravelTime(Position), 0.625, 1e-12)) && Passed;

  D->SetLightDetectorPosition(7);
  DisableDefaultStreams();
  double Wrong = D->GetLightTravelTime(Position);
  EnableDefaultStreams();
  Passed = EvaluateNear("GetLightTravelTime()", "unknown position", "The time is zero", Wrong, 0.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the discretization without drift
bool UTDDriftChamber::TestDiscretizeWithoutDrift()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "standard", "The fixture is accepted", Scan(Geometry, GetBody(), "D.LightSpeed 2.0\nD.LightDetectorPosition 3\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDetector* D = Geometry.GetDetector("D");
  MDVolume* V = Geometry.GetVolume("Box");

  vector<MDGridPoint> Points = D->Discretize(MVector(0.25, 0.25, 0.5), 123.0, 45.0, V);
  Passed = EvaluateSize("Discretize()", "no drift", "Without a drift there is only one electron cloud", Points.size(), 1) && Passed;
  if (Points.size() == 1) {
    Passed = Evaluate("Discretize()", "x-strip", "The x-strip is correct", Points[0].GetXGrid(), 5u) && Passed;
    Passed = Evaluate("Discretize()", "y-strip", "The y-strip is correct", Points[0].GetYGrid(), 4u) && Passed;
    Passed = Evaluate("Discretize()", "type", "The type is a drift voxel", Points[0].GetType(), MDGridPoint::c_VoxelDrift) && Passed;
    Passed = EvaluateNear("Discretize()", "energy", "The energy is stored", Points[0].GetEnergy(), 123.0, 1e-9) && Passed;
    // Time by hand: 45 + (1 - 0.5)/2
    Passed = EvaluateNear("Discretize()", "time", "The light travel time is added", Points[0].GetTime(), 45.25, 1e-9) && Passed;
    Passed = EvaluateNear("Discretize()", "depth", "The depth is stored", Points[0].GetPosition().Z(), 0.5, 1e-12) && Passed;
  }

  // All strips
  for (unsigned int i = 0; i < 10; ++i) {
    for (unsigned int j = 0; j < 7; ++j) {
      MVector Center(-2.5 + (i + 0.5)*0.5, -1.75 + (j + 0.5)*0.5, 0.0);
      Points = D->Discretize(Center, 100.0, 0.0, V);
      Passed = EvaluateSize("Discretize()", "strip center", "There is one point", Points.size(), 1) && Passed;
      if (Points.size() == 1) {
        Passed = Evaluate("Discretize()", "strip center", "The x-strip is found", Points[0].GetXGrid(), i) && Passed;
        Passed = Evaluate("Discretize()", "strip center", "The y-strip is found", Points[0].GetYGrid(), j) && Passed;
      }
      Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "strip center", "The position is the center of the strip", D->GetPositionInDetectorVolume(i, j, 0, MVector(0.0, 0.0, 0.0), MDGridPoint::c_VoxelDrift, V), Center, 1e-9) && Passed;
    }
  }

  // No grid points outside of the active area
  Points = D->Discretize(MVector(2.75, 0.0, 0.0), 100.0, 0.0, V);
  Passed = EvaluateSize("Discretize()", "in the guard ring band", "There is no grid point", Points.size(), 0) && Passed;
  DisableDefaultStreams();
  Points = D->Discretize(MVector(-3.5, 0.0, 0.0), 100.0, 0.0, V);
  EnableDefaultStreams();
  Passed = EvaluateSize("Discretize()", "outside of the volume", "There is no grid point", Points.size(), 0) && Passed;

  // Many wafers: half size 1.5 x 1 x 0.5, gap 0.2 x 0.1, no offset, 3 x 4 strips
  MString Many;
  Many += GetBoxVolume("Container", 3.1, 3.1, 0.5);
  Many += "Volume Wafer\nWafer.Material Silicon\nWafer.Shape BRIK 1.5 1.0 0.5\nWafer.Position -1.6 -2.1 0.0\nWafer.Mother Container\n";
  Many += "DriftChamber D\nD.SensitiveVolume Wafer\nD.DetectorVolume Container\nD.StructuralPitch 0.2 0.1 0.0\nD.StructuralOffset 0.0 0.0 0.0\nD.StripNumber 3 4\nD.Offset 0.0 0.0\n";
  MDGeometry Wafers;
  Passed = EvaluateTrue("ScanSetupFile()", "many wafers", "The fixture is accepted", Scan(Wafers, Many)) && Passed;
  if (CheckDetector(Passed, "Wafers", Wafers, "D", MDDetector::c_DriftChamber)) {
    D = Wafers.GetDetector("D");
    V = Wafers.GetVolume("Container");
    // Wafer (0, 0)
    Points = D->Discretize(MVector(-2.6, -2.85, 0.0), 100.0, 0.0, V);
    Passed = EvaluateSize("Discretize()", "first wafer", "There is one point", Points.size(), 1) && Passed;
    Passed = Evaluate("Discretize()", "first wafer", "The x-strip is correct", Points[0].GetXGrid(), 0u) && Passed;
    Passed = Evaluate("Discretize()", "first wafer", "The y-strip is correct", Points[0].GetYGrid(), 0u) && Passed;
    // Wafer (1, 0): x-center 1.6, first strip -1 relative to it. y is the first strip
    Points = D->Discretize(MVector(0.6, -2.85, 0.0), 100.0, 0.0, V);
    Passed = EvaluateSize("Discretize()", "second wafer in x", "There is one point", Points.size(), 1) && Passed;
    Passed = Evaluate("Discretize()", "second wafer in x", "The x-strip is the first of the second wafer", Points[0].GetXGrid(), 3u) && Passed;
    // Defect GEO-D-053: Discretize calculates the y grid with the x wafer and the number of x-strips, thus all hits in other than the first wafers are in wrong y-strips
    Passed = ExpectedToFail(Evaluate("Discretize()", "second wafer in x", "The y-strip is the first one", Points[0].GetYGrid(), 0u)) && Passed;
    // Wafer (0, 1): y-center 0, first strip -0.75 relative to it
    Points = D->Discretize(MVector(-2.6, -0.75, 0.0), 100.0, 0.0, V);
    Passed = EvaluateSize("Discretize()", "second wafer in y", "There is one point", Points.size(), 1) && Passed;
    Passed = Evaluate("Discretize()", "second wafer in y", "The x-strip is the first one", Points[0].GetXGrid(), 0u) && Passed;
    // Defect GEO-D-053: As above
    Passed = ExpectedToFail(Evaluate("Discretize()", "second wafer in y", "The y-strip is the first of the second wafer", Points[0].GetYGrid(), 4u)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the discretization with drift
bool UTDDriftChamber::TestDiscretizeWithDrift()
{
  bool Passed = true;
  gRandom->SetSeed(7001);

  // Half size 1 x 1 x 1, 1000 x 1000 strips: pitch 0.002 cm, sigma = 0.1 sqrt(drift length)
  MString Fine = GetBoxVolume("Box", 1.0, 1.0, 1.0) + "DriftChamber D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.0 0.0\nD.StripNumber 1000 1000\nD.DriftConstant 0.1\nD.EnergyPerElectron 0.022\n";
  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "drift", "The fixture is accepted", Scan(Geometry, Fine)) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDetector* D = Geometry.GetDetector("D");
  MDVolume* V = Geometry.GetVolume("Box");

  // Top: drift length 2 cm, sigma sqrt(2)*0.1. Number of electrons: 2200 keV/0.022 keV
  vector<MDGridPoint> Points = D->Discretize(MVector(0.0, 0.0, 1.0), 2200.0, 5.0, V);
  Passed = EvaluateNear("Discretize()", "2200 keV", "The number of electrons is the energy divided by the energy per electron", Points.size(), 100000.0, 1.0) && Passed;
  vector<double> X;
  vector<double> Y;
  double Sum = 0.0;
  for (const MDGridPoint& Point: Points) {
    X.push_back(-1.0 + (Point.GetXGrid() + 0.5)*0.002);
    Y.push_back(-1.0 + (Point.GetYGrid() + 0.5)*0.002);
    Sum += Point.GetEnergy();
  }
  double Sigma = 0.1*sqrt(2.0);
  Passed = EvaluateNear("Discretize()", "drift in x", "The cloud is centered", MStatistics::GetMean(X), 0.0, GetMeanTolerance(Sigma, X.size())) && Passed;
  Passed = EvaluateNear("Discretize()", "drift in x", "The width is the drift constant times the square root of the drift length", MStatistics::GetStandardDeviation(X), Sigma, GetRMSTolerance(Sigma, X.size())) && Passed;
  Passed = EvaluateNear("Discretize()", "drift in y", "The cloud is centered", MStatistics::GetMean(Y), 0.0, GetMeanTolerance(Sigma, Y.size())) && Passed;
  Passed = EvaluateNear("Discretize()", "drift in y", "The width is the drift constant times the square root of the drift length", MStatistics::GetStandardDeviation(Y), Sigma, GetRMSTolerance(Sigma, Y.size())) && Passed;
  Passed = EvaluateNear("Discretize()", "energy", "The energy is conserved", Sum, 2200.0, 1e-6) && Passed;

  // Bottom: no drift length
  Points = D->Discretize(MVector(0.0015, 0.0015, -1.0), 22.0, 5.0, V);
  Passed = EvaluateSize("Discretize()", "at the bottom", "1000 electrons are generated", Points.size(), 1000) && Passed;
  bool AllIdentical = true;
  for (const MDGridPoint& Point: Points) {
    if (Point.GetXGrid() != 500 || Point.GetYGrid() != 500) {
      AllIdentical = false;
    }
  }
  Passed = EvaluateTrue("Discretize()", "at the bottom", "All electrons remain in one strip", AllIdentical) && Passed;

  // Less energy than one electron: one electron with all the energy
  Points = D->Discretize(MVector(0.0, 0.0, -1.0), 0.01, 5.0, V);
  Passed = EvaluateSize("Discretize()", "below the energy of one electron", "There is one electron", Points.size(), 1) && Passed;
  if (Points.size() == 1) {
    Passed = EvaluateNear("Discretize()", "below the energy of one electron", "The electron has all the energy", Points[0].GetEnergy(), 0.01, 1e-12) && Passed;
  }

  // Close to the edge: only the part which remains in the active area stays, here Phi(0.01/sigma) in x, y is not cut
  Points = D->Discretize(MVector(0.99, 0.0, 1.0), 2200.0, 5.0, V);
  double Expected = GetPhi(0.01/Sigma);
  Passed = EvaluateNear("Discretize()", "close to the edge", "The electrons drifting out of the volume are lost", Points.size()/100000.0, Expected, GetFractionTolerance(Expected)) && Passed;

  // Energy per electron of zero
  MDGeometry Zero;
  Passed = EvaluateTrue("ScanSetupFile()", "energy per electron 0", "The fixture is accepted", Scan(Zero, Fine, "D.EnergyPerElectron 0.0\n")) && Passed;
  if (CheckDetector(Passed, "Zero", Zero, "D", MDDetector::c_DriftChamber)) {
    MDDetector* ZeroD = Zero.GetDetector("D");
    MDVolume* ZeroV = Zero.GetVolume("Box");
    // Defect GEO-D-051: As an energy per electron of zero is accepted, the number of electrons is undefined and the discretization fails
    Passed = ExpectedToCrash(EvaluateTrue("Discretize()", "energy per electron 0", "The discretization works", (ZeroD->Discretize(MVector(0.0, 0.0, 1.0), 2200.0, 5.0, ZeroV), true))) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the light energy resolution
bool UTDDriftChamber::TestLightEnergyResolution()
{
  bool Passed = true;
  gRandom->SetSeed(7002);

  // Not validated: the type is unknown
  MDDriftChamber Direct("Direct");
  double Energy = 100.0;
  DisableDefaultStreams();
  bool Result = Direct.NoiseLightEnergy(Energy);
  EnableDefaultStreams();
  Passed = EvaluateFalse("NoiseLightEnergy()", "unknown type", "An unknown type is an error", Result) && Passed;

  // Ideal
  MDGeometry Ideal;
  Passed = EvaluateTrue("ScanSetupFile()", "ideal", "The fixture is accepted", Scan(Ideal, GetBody())) && Passed;
  if (CheckDetector(Passed, "Ideal", Ideal, "D", MDDetector::c_DriftChamber)) {
    Energy = 100.0;
    Passed = EvaluateTrue("NoiseLightEnergy()", "ideal", "The ideal type is successful", dynamic_cast<MDDriftChamber*>(Ideal.GetDetector("D"))->NoiseLightEnergy(Energy)) && Passed;
    Passed = EvaluateNear("NoiseLightEnergy()", "ideal", "The energy stays", Energy, 100.0, 1e-12) && Passed;
  }

  // A table
  MDGeometry Table;
  Passed = EvaluateTrue("ScanSetupFile()", "table", "The fixture is accepted", Scan(Table, GetBody(), "D.LightEnergyResolution 100 2\nD.LightEnergyResolution 500 6\n")) && Passed;
  if (CheckDetector(Passed, "Table", Table, "D", MDDetector::c_DriftChamber)) {
    MDDriftChamber* D = dynamic_cast<MDDriftChamber*>(Table.GetDetector("D"));
    Passed = EvaluateNear("GetLightEnergyResolution()", "300 keV", "The resolution is interpolated", D->GetLightEnergyResolution(300.0), 4.0, 1e-9) && Passed;
    vector<double> Values;
    for (unsigned int i = 0; i < c_Samples; ++i) {
      Energy = 300.0;
      D->NoiseLightEnergy(Energy);
      Values.push_back(Energy);
    }
    // Defect GEO-D-054: The type of the light energy resolution is never set to Gauss, thus a configured table is ignored and the validation makes it ideal
    Passed = ExpectedToFail(EvaluateNear("NoiseLightEnergy()", "table at 300 keV", "The width is the interpolated width", MStatistics::GetStandardDeviation(Values), 4.0, GetRMSTolerance(4.0))) && Passed;
    Passed = EvaluateNear("NoiseLightEnergy()", "table at 300 keV", "The mean is the energy", MStatistics::GetMean(Values), 300.0, GetMeanTolerance(4.0)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy, named detectors, and text output
bool UTDDriftChamber::TestCopyNamedText()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "all values", "The fixture is accepted", Scan(Geometry, GetBody(), "D.LightSpeed 1.5\nD.LightDetectorPosition 3\nD.DriftConstant 0.25\nD.EnergyPerElectron 0.5\nD.LightEnergyResolution 100 2\nD.LightEnergyResolution 500 6\nD.DepthResolution 100 0.1\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDriftChamber* D = dynamic_cast<MDDriftChamber*>(Geometry.GetDetector("D"));

  // Clone
  Passed = CheckCloneIndependence("DriftChamber", D) && Passed;
  unique_ptr<MDDetector> Cloned(D->Clone());
  MDDriftChamber* Copy = dynamic_cast<MDDriftChamber*>(Cloned.get());
  if (EvaluateTrue("Clone()", "drift chamber", "The clone is a drift chamber", Copy != nullptr) == false) {
    return false;
  }
  Passed = EvaluateNear("MDDriftChamber(const MDDriftChamber&)", "light speed", "The value is copied", Copy->GetLightSpeed(), 1.5, 1e-12) && Passed;
  Passed = Evaluate("MDDriftChamber(const MDDriftChamber&)", "light detector", "The value is copied", Copy->GetLightDetectorPosition(), 3) && Passed;
  Passed = EvaluateNear("MDDriftChamber(const MDDriftChamber&)", "drift constant", "The value is copied", Copy->GetDriftConstant(), 0.25, 1e-12) && Passed;
  Passed = EvaluateNear("MDDriftChamber(const MDDriftChamber&)", "energy per electron", "The value is copied", Copy->GetEnergyPerElectron(), 0.5, 1e-12) && Passed;
  Passed = EvaluateNear("MDDriftChamber(const MDDriftChamber&)", "light energy resolution", "The table is copied", Copy->GetLightEnergyResolution(300.0), 4.0, 1e-9) && Passed;
  delete Copy->GetGuardRing();

  // Text
  MString Text = D->GetGeomega();
  Passed = EvaluateTrue("GetGeomega()", "light speed", "The value is written", Text.Contains("D.LightSpeed 1.5")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "light detector", "The value is written", Text.Contains("D.LightDetectorPosition 3")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "drift constant", "The value is written", Text.Contains("D.DriftConstant 0.25")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "energy per electron", "The value is written", Text.Contains("D.EnergyPerElectron 0.5")) && Passed;
  // Defect GEO-D-055: GetGeomega writes the type Strip3D for a drift chamber
  Passed = ExpectedToFail(EvaluateTrue("GetGeomega()", "type", "The type is drift chamber", Text.Contains("DriftChamber D"))) && Passed;
  // Defect GEO-D-055: GetGeomega writes the light energy resolution with the keyword of the depth resolution
  Passed = ExpectedToFail(EvaluateTrue("GetGeomega()", "light energy resolution", "The keyword of the light energy resolution is written", Text.Contains("D.LightEnergyResolution 100 2"))) && Passed;

  Passed = EvaluateTrue("ToString()", "standard", "The type is written", D->ToString().Contains("DriftChamber")) && Passed;

  // Named detector
  // Defect GEO-D-056: CopyDataToNamedDetectors calls the function of MDDetector and not of MDStrip3D, the strip and depth data is not inherited and a named drift chamber is always rejected
  Passed = ExpectedToFail(EvaluateTrue("ScanSetupFile()", "named detector", "A named drift chamber is accepted", IsScanOK(GetBody(), "D.Named N\nN.Assign World.Box\n"))) && Passed;
  MDDriftChamber Base("Base");
  MDDriftChamber* Named = new MDDriftChamber("Named");
  Base.AddNamedDetector(Named);
  Base.SetNStrips(3, 4);
  Base.SetLightSpeed(1.5);
  Base.SetLightDetectorPosition(2);
  Base.SetDriftConstant(2.5);
  Base.SetEnergyPerElectron(3.5);
  Base.SetLightEnergyResolution(100.0, 2.0);
  MDDriftChamber* Own = new MDDriftChamber("Own");
  Base.AddNamedDetector(Own);
  Own->SetLightSpeed(9.5);
  Passed = EvaluateTrue("CopyDataToNamedDetectors()", "direct call", "The call is successful", Base.CopyDataToNamedDetectors()) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "light speed", "The value is inherited", Named->GetLightSpeed(), 1.5, 1e-12) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "own light speed", "The own value stays", Own->GetLightSpeed(), 9.5, 1e-12) && Passed;
  Passed = Evaluate("CopyDataToNamedDetectors()", "light detector", "The value is inherited", Named->GetLightDetectorPosition(), 2) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "drift constant", "The value is inherited", Named->GetDriftConstant(), 2.5, 1e-12) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "energy per electron", "The value is inherited", Named->GetEnergyPerElectron(), 3.5, 1e-12) && Passed;
  // Defect GEO-D-056: As above
  Passed = ExpectedToFail(Evaluate("CopyDataToNamedDetectors()", "number of strips", "The number of strips is inherited", Named->GetNStripsX(), 3)) && Passed;
  delete Named;
  delete Own;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  gROOT->SetBatch(true);
  gErrorIgnoreLevel = kWarning;
  TApplication ROOT("ROOT", 0, 0);

  UTDDriftChamber Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDDriftChamber.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
