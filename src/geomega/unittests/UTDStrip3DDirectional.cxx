/*
 * UTDStrip3DDirectional.cxx
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


//! Unit test of the directional strip detector MDStrip3DDirectional
class UTDStrip3DDirectional : public UTDDetectorBase
{
public:
  //! Default constructor
  UTDStrip3DDirectional() : UTDDetectorBase("UTDStrip3DDirectional") {}
  //! Default destructor
  virtual ~UTDStrip3DDirectional() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the standard fixture
  MString GetBody() const;
  //! Return the angles of the noised direction relative to the input direction
  vector<double> SampleAngles(MDStrip3DDirectional& Detector, const MVector& Direction, double Energy);

  //! Test the construction and the keywords
  bool TestConstructionAndKeywords();
  //! Test the noising of the direction
  bool TestNoiseDirection();
  //! Test the depth noise and grid inherited from the 3D strip detector
  bool TestInherited();
  //! Test copy, named detectors, and text output
  bool TestCopyNamedText();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDStrip3DDirectional::Run()
{
  bool Passed = true;

  Passed = TestConstructionAndKeywords() && Passed;
  Passed = TestCommonNoiseChain(GetBody(), MVector(0.25, 0.25, 0.5)) && Passed;
  Passed = TestNoiseDirection() && Passed;
  Passed = TestInherited() && Passed;
  Passed = TestCopyNamedText() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the standard fixture
MString UTDStrip3DDirectional::GetBody() const
{
  return GetBoxVolume("Box", 3.0, 2.0, 1.0) + "Strip3DDirectional D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.5 0.25\nD.StripNumber 10 7\n";
}


////////////////////////////////////////////////////////////////////////////////


//! Return the angles of the noised direction relative to the input direction
vector<double> UTDStrip3DDirectional::SampleAngles(MDStrip3DDirectional& Detector, const MVector& Direction, double Energy)
{
  vector<double> Angles;
  Angles.reserve(c_Samples);
  double Reference = atan2(Direction.Y(), Direction.X());
  for (unsigned int i = 0; i < c_Samples; ++i) {
    MVector D = Direction;
    Detector.NoiseDirection(D, Energy);
    double Angle = atan2(D.Y(), D.X()) - Reference;
    // Remove the wrap around:
    if (Angle > c_Pi) {
      Angle -= 2*c_Pi;
    }
    if (Angle < -c_Pi) {
      Angle += 2*c_Pi;
    }
    Angles.push_back(Angle);
  }
  return Angles;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the construction and the keywords
bool UTDStrip3DDirectional::TestConstructionAndKeywords()
{
  bool Passed = true;

  MDStrip3DDirectional Strip("Strip");
  Passed = EvaluateTrue("MDStrip3DDirectional()", "name", "The name is stored", Strip.GetName() == "Strip") && Passed;
  Passed = Evaluate("MDStrip3DDirectional()", "type", "The type is Strip3DDirectional", Strip.GetType(), MDDetector::c_Strip3DDirectional) && Passed;
  Passed = EvaluateTrue("MDStrip3DDirectional()", "type name", "The type name is Strip3DDirectional", Strip.GetTypeName() == MDDetector::c_Strip3DDirectionalName) && Passed;

  const MString Body = GetBody();
  Passed = EvaluateTrue("ScanSetupFile()", "minimal", "A minimal detector is accepted", IsScanOK(Body)) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "depth keywords", "The depth keywords are accepted", IsScanOK(Body, "D.DepthResolution 100 0.1\nD.DepthResolutionThreshold 10\nD.TriggerThresholdDepthCorrection 0 1\nD.NoiseThresholdDepthCorrection 0 1\nD.EnergyResolutionDepthCorrection 0 1\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "guard ring keywords", "The guard ring keywords are accepted", IsScanOK(Body, "D.GuardRingEnergyResolution 100 1\nD.GuardRingEnergyResolution 200 1\nD.GuardRingTriggerThreshold 10 1\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "no offset", "A missing offset is rejected", IsScanOK(GetBoxVolume("Box", 3.0, 2.0, 1.0), "Strip3DDirectional D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.StripNumber 10 7\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "DirectionalResolution with no values", "The wrong number of tokens is rejected", IsScanOK(Body, "D.DirectionalResolution\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "DirectionalResolution with one value", "The wrong number of tokens is rejected", IsScanOK(Body, "D.DirectionalResolution 100\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "DirectionalResolution with four values", "The wrong number of tokens is rejected", IsScanOK(Body, "D.DirectionalResolution 100 1 1 1\n")) && Passed;
  // Defect GEO-D-045: SetDirectionalResolutionAt returns nothing and thus an invalid table line is only printed and otherwise ignored by the scan
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "negative directional resolution", "An invalid value is rejected", IsScanOK(Body, "D.DirectionalResolution 100 -1\n"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the noising of the direction
bool UTDStrip3DDirectional::TestNoiseDirection()
{
  bool Passed = true;
  gRandom->SetSeed(6001);

  // Unknown type: no valid resolution was given and no validation was done
  MDStrip3DDirectional Unknown("Unknown");
  DisableDefaultStreams();
  Unknown.SetDirectionalResolutionAt(-1.0, 1.0, 0.1);
  Unknown.SetDirectionalResolutionAt(1.0, 0.0, 0.1);
  Unknown.SetDirectionalResolutionAt(1.0, -1.0, 0.1);
  Unknown.SetDirectionalResolutionAt(1.0, 1.0, -0.1);
  MVector Direction(1.0, 0.0, 0.0);
  bool Result = Unknown.NoiseDirection(Direction, 100.0);
  EnableDefaultStreams();
  Passed = EvaluateFalse("NoiseDirection()", "unknown type after only invalid inputs", "The call fails", Result) && Passed;
  Passed = EvaluateVectorNear("NoiseDirection()", "unknown type after only invalid inputs", "The direction stays", Direction, MVector(1.0, 0.0, 0.0), 1e-12) && Passed;

  // Ideal after the validation
  MDGeometry Ideal;
  Passed = EvaluateTrue("ScanSetupFile()", "no directional resolution", "The fixture is accepted", Scan(Ideal, GetBody())) && Passed;
  if (CheckDetector(Passed, "Ideal", Ideal, "D", MDDetector::c_Strip3DDirectional)) {
    MVector In(0.6, 0.0, 0.8);
    Passed = EvaluateTrue("NoiseDirection()", "ideal", "The call is successful", dynamic_cast<MDStrip3DDirectional*>(Ideal.GetDetector("D"))->NoiseDirection(In, 100.0)) && Passed;
    Passed = EvaluateVectorNear("NoiseDirection()", "ideal", "The direction stays", In, MVector(0.6, 0.0, 0.8), 1e-12) && Passed;
  }

  // A constant resolution
  MDStrip3DDirectional Constant("Constant");
  Constant.SetDirectionalResolutionAt(100.0, 0.1, 0.0);
  Constant.SetDirectionalResolutionAt(500.0, 0.1, 0.0);
  Passed = CheckGauss("NoiseDirection()", "along x", SampleAngles(Constant, MVector(1.0, 0.0, 0.0), 300.0), 0.0, 0.1) && Passed;
  Passed = CheckGauss("NoiseDirection()", "along y", SampleAngles(Constant, MVector(0.0, 1.0, 0.0), 300.0), 0.0, 0.1) && Passed;
  Passed = CheckGauss("NoiseDirection()", "along negative y", SampleAngles(Constant, MVector(0.0, -1.0, 0.0), 300.0), 0.0, 0.1) && Passed;
  Passed = CheckGauss("NoiseDirection()", "along negative x, around the wrap of the angle", SampleAngles(Constant, MVector(-1.0, 0.0, 0.0), 300.0), 0.0, 0.1) && Passed;
  Passed = CheckGauss("NoiseDirection()", "diagonal", SampleAngles(Constant, MVector(1.0, 2.0, 0.0), 300.0), 0.0, 0.1) && Passed;

  // The result is a unit vector in the x-y plane
  MVector Out(3.0, 4.0, 5.0);
  Passed = EvaluateTrue("NoiseDirection()", "not a unit vector", "The call is successful", Constant.NoiseDirection(Out, 300.0)) && Passed;
  Passed = EvaluateNear("NoiseDirection()", "not a unit vector", "The result is a unit vector", Out.Mag(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("NoiseDirection()", "not a unit vector", "The result is in the x-y plane", Out.Z(), 0.0, 1e-12) && Passed;

  // The sigma of the resolution adds in quadrature for the angle: sqrt(0.1^2 + 0.02^2)
  MDStrip3DDirectional Varying("Varying");
  Varying.SetDirectionalResolutionAt(100.0, 0.1, 0.02);
  Varying.SetDirectionalResolutionAt(500.0, 0.1, 0.02);
  Passed = CheckGauss("NoiseDirection()", "with a sigma of the resolution", SampleAngles(Varying, MVector(1.0, 0.0, 0.0), 300.0), 0.0, sqrt(0.01 + 0.0004)) && Passed;

  // Energy dependence: 0.1 at 100 keV, 0.3 at 500 keV, thus 0.2 at 300 keV
  MDStrip3DDirectional Table("Table");
  Table.SetDirectionalResolutionAt(100.0, 0.1, 0.0);
  Table.SetDirectionalResolutionAt(500.0, 0.3, 0.0);
  // Defect GEO-D-046: NoiseDirection overwrites the energy argument with the length of the direction vector, thus the resolution is always evaluated at the wrong energy
  Passed = ExpectedToFail(EvaluateNear("NoiseDirection()", "energy dependent resolution at 300 keV", "The width is the interpolated value at the energy", MStatistics::GetStandardDeviation(SampleAngles(Table, MVector(1.0, 0.0, 0.0), 300.0)), 0.2, GetRMSTolerance(0.2))) && Passed;

  // Keyword: degrees
  MDGeometry Degrees;
  Passed = EvaluateTrue("ScanSetupFile()", "resolution in degrees", "The fixture is accepted", Scan(Degrees, GetBody(), "D.DirectionalResolution 100 10\nD.DirectionalResolution 500 10\n")) && Passed;
  if (CheckDetector(Passed, "Degrees", Degrees, "D", MDDetector::c_Strip3DDirectional)) {
    double Expected = 10.0*c_Rad;
    vector<double> Angles = SampleAngles(*dynamic_cast<MDStrip3DDirectional*>(Degrees.GetDetector("D")), MVector(1.0, 0.0, 0.0), 300.0);
    Passed = EvaluateNear("ScanSetupFile()", "resolution in degrees", "The resolution of 10 degrees is converted to radians", MStatistics::GetStandardDeviation(Angles), Expected, GetRMSTolerance(Expected)) && Passed;
  }
  MDGeometry DegreesSigma;
  Passed = EvaluateTrue("ScanSetupFile()", "resolution and sigma in degrees", "The fixture is accepted", Scan(DegreesSigma, GetBody(), "D.DirectionalResolution 100 10 1\nD.DirectionalResolution 500 10 1\n")) && Passed;
  if (CheckDetector(Passed, "DegreesSigma", DegreesSigma, "D", MDDetector::c_Strip3DDirectional)) {
    double Expected = sqrt(101.0)*c_Rad;
    vector<double> Angles = SampleAngles(*dynamic_cast<MDStrip3DDirectional*>(DegreesSigma.GetDetector("D")), MVector(1.0, 0.0, 0.0), 300.0);
    // Defect GEO-D-047: The parser converts the directional resolution from degrees to radians but not its sigma
    Passed = ExpectedToFail(EvaluateNear("ScanSetupFile()", "resolution and sigma in degrees", "Both values are converted to radians", MStatistics::GetStandardDeviation(Angles), Expected, GetRMSTolerance(Expected))) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the depth noise and grid inherited from the 3D strip detector
bool UTDStrip3DDirectional::TestInherited()
{
  bool Passed = true;
  gRandom->SetSeed(6002);

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "depth resolution", "The fixture is accepted", Scan(Geometry, GetBody(), "D.DepthResolution 100 0.1\nD.DepthResolution 500 0.3\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDetector* D = Geometry.GetDetector("D");
  MDVolume* V = Geometry.GetVolume("Box");
  Passed = CheckGauss("Noise()", "depth at 300 keV", SamplePosition(D, V, MVector(0.0, 0.0, 0.0), 300.0, 2), 0.0, 0.2) && Passed;

  // x: (0.25 + 2.5)/0.5 = 5, y: (0.25 + 1.75)/0.5 = 4
  MDGridPoint Point = D->GetGridPoint(MVector(0.25, 0.25, 0.5));
  Passed = Evaluate("GetGridPoint()", "x", "The x-strip is correct", Point.GetXGrid(), 5u) && Passed;
  Passed = Evaluate("GetGridPoint()", "y", "The y-strip is correct", Point.GetYGrid(), 4u) && Passed;
  Passed = EvaluateNear("GetGridPoint()", "z", "The depth is stored", Point.GetPosition().Z(), 0.5, 1e-12) && Passed;
  Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "5, 4", "The strip center and the depth are returned", D->GetPositionInDetectorVolume(5, 4, 0, MVector(0.0, 0.0, 0.5), MDGridPoint::c_Voxel, V), MVector(0.25, 0.5, 0.5), 1e-9) && Passed;

  // Attention: The name of the detector type is covered by UTDDetector
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy, named detectors, and text output
bool UTDStrip3DDirectional::TestCopyNamedText()
{
  bool Passed = true;
  gRandom->SetSeed(6003);

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "directional resolution", "The fixture is accepted", Scan(Geometry, GetBody(), "D.DirectionalResolution 100 10\nD.DirectionalResolution 500 10\nD.DepthResolution 100 0.1\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MDStrip3DDirectional* D = dynamic_cast<MDStrip3DDirectional*>(Geometry.GetDetector("D"));

  // Clone
  Passed = CheckCloneIndependence("Strip3DDirectional", D) && Passed;
  unique_ptr<MDDetector> Cloned(D->Clone());
  MDStrip3DDirectional* Copy = dynamic_cast<MDStrip3DDirectional*>(Cloned.get());
  if (EvaluateTrue("Clone()", "directional", "The clone is a directional strip detector", Copy != nullptr) == false) {
    return false;
  }
  double Expected = 10.0*c_Rad;
  Passed = EvaluateNear("MDStrip3DDirectional(const MDStrip3DDirectional&)", "directional resolution", "The table is copied", MStatistics::GetStandardDeviation(SampleAngles(*Copy, MVector(1.0, 0.0, 0.0), 300.0)), Expected, GetRMSTolerance(Expected)) && Passed;
  delete Copy->GetGuardRing();

  // Text
  MString Text = D->GetGeomega();
  Passed = EvaluateTrue("GetGeomega()", "type", "The type is written", Text.Contains("Strip3DDirectional D")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "directional resolution", "The directional resolution is written", Text.Contains("D.DirectionalResolution 100")) && Passed;
  MDGeometry Second;
  Passed = EvaluateTrue("ScanSetupFile()", "round trip", "The text can be scanned again", Scan(Second, GetBoxVolume("Box", 3.0, 2.0, 1.0), Text)) && Passed;
  if (CheckDetector(Passed, "Second", Second, "D", MDDetector::c_Strip3DDirectional)) {
    // Defect GEO-D-048: GetGeomega writes the directional resolution in radians, but the parser reads degrees
    Passed = ExpectedToFail(Evaluate("GetGeomega()", "round trip, all fields", "The text written after the round trip is identical, so that no written field is lost", dynamic_cast<MDStrip3DDirectional*>(Second.GetDetector("D"))->GetGeomega(), Text)) && Passed;
    Passed = Evaluate("GetGeomega()", "round trip", "The type is identical", Second.GetDetector("D")->GetType(), MDDetector::c_Strip3DDirectional) && Passed;
    // Defect GEO-D-048: GetGeomega writes the directional resolution in radians while the parser reads degrees
    Passed = ExpectedToFail(EvaluateNear("GetGeomega()", "round trip", "The directional resolution is identical", MStatistics::GetStandardDeviation(SampleAngles(*dynamic_cast<MDStrip3DDirectional*>(Second.GetDetector("D")), MVector(1.0, 0.0, 0.0), 300.0)), Expected, GetRMSTolerance(Expected))) && Passed;
  }

  MString Dump = D->ToString();
  Passed = EvaluateTrue("ToString()", "name", "The name is written", Dump.Contains("D - 3D-Strip")) && Passed;
  Passed = EvaluateTrue("ToString()", "volume", "The sensitive volume is written", Dump.Contains("sensitive volumes: Box")) && Passed;
  Passed = EvaluateTrue("ToString()", "pitch", "The pitch is written", Dump.Contains("pitch: 0.5, 0.5")) && Passed;

  // Named detector
  // Defect GEO-D-049: CopyDataToNamedDetectors calls the function of MDDetector and not of MDStrip3D, the strip and depth data is not inherited and a named detector of this type is always rejected
  Passed = ExpectedToFail(EvaluateTrue("ScanSetupFile()", "named detector", "A named directional strip detector is accepted", IsScanOK(GetBody(), "D.Named N\nN.Assign World.Box\n"))) && Passed;
  MDStrip3DDirectional Base("Base");
  MDStrip3DDirectional* Named = new MDStrip3DDirectional("Named");
  Base.AddNamedDetector(Named);
  Base.SetNStrips(3, 4);
  Base.SetOffset(0.1, 0.2);
  Base.SetDirectionalResolutionAt(100.0, 0.1, 0.0);
  Passed = EvaluateTrue("CopyDataToNamedDetectors()", "direct call", "The call is successful", Base.CopyDataToNamedDetectors()) && Passed;
  Passed = CheckGauss("CopyDataToNamedDetectors()", "directional resolution", SampleAngles(*Named, MVector(1.0, 0.0, 0.0), 300.0), 0.0, 0.1) && Passed;
  // Defect GEO-D-049: As above
  Passed = ExpectedToFail(Evaluate("CopyDataToNamedDetectors()", "number of strips", "The number of strips is inherited", Named->GetNStripsX(), 3)) && Passed;
  Passed = ExpectedToFail(EvaluateNear("CopyDataToNamedDetectors()", "offset", "The offset is inherited", Named->GetOffsetY(), 0.2, 1e-12)) && Passed;
  delete Named;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  gROOT->SetBatch(true);
  gErrorIgnoreLevel = kWarning;
  TApplication ROOT("ROOT", 0, 0);

  UTDStrip3DDirectional Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDStrip3DDirectional.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
