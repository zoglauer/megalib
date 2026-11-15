/*
 * UTDShapeTUBS.cxx
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

// ROOT libs:
#include <TApplication.h>
#include <TError.h>
#include <TROOT.h>
using namespace std;

// MEGAlib libs:
#include "MDShapeTUBS.h"
#include "UTDShapeShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the tube shape MDShapeTUBS
class UTDShapeTUBS : public UTDShapeBase
{
public:
  //! Default constructor
  UTDShapeTUBS() : UTDShapeBase("UTDShapeTUBS") {}
  //! Default destructor
  virtual ~UTDShapeTUBS() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the analytic truth of a tube segment, the angles are in degrees and can pass 360
  MTruth GetTruth(double Rmin, double Rmax, double H, double Phi1, double Phi2);

  //! Test the default state
  bool TestDefault();
  //! Test the limits of the parameters
  bool TestLimits();
  //! Test the volume and the size
  bool TestVolumeSize();
  //! Test the inside check
  bool TestIsInside();
  //! Test the distance functions with fixed rays
  bool TestDistances();
  //! Test the unique position
  bool TestUniquePosition();
  //! Test the geometry against the analytic truth
  bool TestAgainstTruth();
  //! Test the string representations
  bool TestStrings();
  //! Test the scaling
  bool TestScale();
  //! Test parsing the keywords
  bool TestParse();
  //! Test configuring a validated shape again
  bool TestReconfiguration();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDShapeTUBS::Run()
{
  bool Passed = true;

  Passed = TestDefault() && Passed;
  Passed = TestLimits() && Passed;
  Passed = TestVolumeSize() && Passed;
  Passed = TestIsInside() && Passed;
  Passed = TestDistances() && Passed;
  Passed = TestUniquePosition() && Passed;
  Passed = TestAgainstTruth() && Passed;
  Passed = TestStrings() && Passed;
  Passed = TestScale() && Passed;
  Passed = TestParse() && Passed;

  Passed = TestReconfiguration() && Passed;
  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the analytic truth of a tube segment, the angles are in degrees and can pass 360
UTDShapeBase::MTruth UTDShapeTUBS::GetTruth(double Rmin, double Rmax, double H, double Phi1, double Phi2)
{
  return [=](double x, double y, double z) {
    double R = sqrt(x*x + y*y);
    if (R < Rmin || R > Rmax || fabs(z) > H) {
      return false;
    }
    double Phi = atan2(y, x)*c_Deg;
    if (Phi < Phi1) {
      Phi += 360;
    }
    if (Phi < Phi1) {
      Phi += 360;
    }
    if (Phi > Phi2) {
      return false;
    }
    return true;
  };
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default state
bool UTDShapeTUBS::TestDefault()
{
  bool Passed = true;

  MDShapeTUBS Shape("Tube");
  Passed = EvaluateTrue("GetName()", "constructor", "The name of the constructor is stored", Shape.GetName() == "Tube") && Passed;
  Passed = EvaluateTrue("GetType()", "constructor", "The type is TUBS", Shape.GetType() == "TUBS") && Passed;
  Passed = EvaluateNear("GetVolume()", "default", "A new tube has no volume", Shape.GetVolume(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetRmin()", "default", "The default is zero", Shape.GetRmin(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetRmax()", "default", "The default is zero", Shape.GetRmax(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetHeight()", "default", "The default is zero", Shape.GetHeight(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetPhi1()", "default", "The default is zero", Shape.GetPhi1(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetPhi2()", "default", "The default is zero", Shape.GetPhi2(), 0.0, 1e-15) && Passed;

  // Defect GEO-C-7: A shape without parameters passes the validation and becomes a zero-size shape
  MDShapeTUBS NoParameters("NoParameters");
  Passed = ExpectedToFail(EvaluateFalse("Validate()", "no parameters", "A tube without parameters is rejected", Quiet([&]() { return NoParameters.Validate(); }))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the limits of the parameters
bool UTDShapeTUBS::TestLimits()
{
  bool Passed = true;

  const double Just = 1e-9;
  MDShapeTUBS S("Tube");

  Passed = EvaluateTrue("Set()", "defaults of the angles", "The angles are optional", Quiet([&]() { return S.Set(0, 1, 1); })) && Passed;
  Passed = EvaluateNear("Set()", "defaults of the angles", "The default of Phi1 is 0", S.GetPhi1(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("Set()", "defaults of the angles", "The default of Phi2 is 360", S.GetPhi2(), 360.0, 1e-15) && Passed;

  Passed = EvaluateFalse("Set()", "Rmin just below 0", "A negative Rmin is rejected", Quiet([&]() { return S.Set(-Just, 1, 1); })) && Passed;
  Passed = EvaluateTrue("Set()", "Rmax just above 0", "A small Rmax is accepted", Quiet([&]() { return S.Set(0, Just, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "Rmax = 0", "Rmax = 0 is rejected", Quiet([&]() { return S.Set(0, 0, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "Rmax just below 0", "A negative Rmax is rejected", Quiet([&]() { return S.Set(0, -Just, 1); })) && Passed;
  Passed = EvaluateTrue("Set()", "Rmin just below Rmax", "Rmin below Rmax is accepted", Quiet([&]() { return S.Set(1 - Just, 1, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "Rmin = Rmax", "Rmin = Rmax is rejected", Quiet([&]() { return S.Set(1, 1, 1); })) && Passed;
  Passed = EvaluateTrue("Set()", "H just above 0", "A small half height is accepted", Quiet([&]() { return S.Set(0, 1, Just); })) && Passed;
  Passed = EvaluateFalse("Set()", "H = 0", "A zero half height is rejected", Quiet([&]() { return S.Set(0, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "H just below 0", "A negative half height is rejected", Quiet([&]() { return S.Set(0, 1, -Just); })) && Passed;

  Passed = EvaluateTrue("Set()", "Phi1 = 0", "Phi1 = 0 is accepted", Quiet([&]() { return S.Set(0, 1, 1, 0, 10); })) && Passed;
  Passed = EvaluateFalse("Set()", "Phi1 just below 0", "Phi1 below 0 is rejected", Quiet([&]() { return S.Set(0, 1, 1, -Just, 10); })) && Passed;
  Passed = EvaluateTrue("Set()", "Phi1 just below 360", "Phi1 just below 360 is accepted", Quiet([&]() { return S.Set(0, 1, 1, 360 - Just, 370); })) && Passed;
  Passed = EvaluateFalse("Set()", "Phi1 = 360", "Phi1 = 360 is rejected", Quiet([&]() { return S.Set(0, 1, 1, 360, 370); })) && Passed;
  Passed = EvaluateTrue("Set()", "difference just above 0", "A small opening is accepted", Quiet([&]() { return S.Set(0, 1, 1, 10, 10 + 1e-6); })) && Passed;
  Passed = EvaluateFalse("Set()", "difference = 0", "No opening is rejected", Quiet([&]() { return S.Set(0, 1, 1, 10, 10); })) && Passed;
  Passed = EvaluateFalse("Set()", "difference below 0", "A negative opening is rejected", Quiet([&]() { return S.Set(0, 1, 1, 20, 10); })) && Passed;
  Passed = EvaluateTrue("Set()", "difference = 360", "The full opening is accepted", Quiet([&]() { return S.Set(0, 1, 1, 10, 370); })) && Passed;
  Passed = EvaluateFalse("Set()", "difference just above 360", "An opening above 360 is rejected", Quiet([&]() { return S.Set(0, 1, 1, 10, 370 + 1e-6); })) && Passed;

  // A rejected set does not change the shape:
  S.Set(1, 2, 3, 4, 5);
  Quiet([&]() { return S.Set(5, 4, 3, 2, 1); });
  Passed = EvaluateNear("GetRmin()", "after reject", "The old Rmin is kept", S.GetRmin(), 1.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetRmax()", "after reject", "The old Rmax is kept", S.GetRmax(), 2.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetHeight()", "after reject", "The old half height is kept", S.GetHeight(), 3.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetPhi1()", "after reject", "The old Phi1 is kept", S.GetPhi1(), 4.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetPhi2()", "after reject", "The old Phi2 is kept", S.GetPhi2(), 5.0, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the volume and the size
bool UTDShapeTUBS::TestVolumeSize()
{
  bool Passed = true;

  // Cylinder: pi R^2 2H
  MDShapeTUBS Full("Full");
  Full.Set(0, 3, 2);
  Passed = EvaluateNear("GetVolume()", "cylinder", "The volume is pi R^2 2H", Full.GetVolume(), c_Pi*9*4, 1e-10) && Passed;

  // Hollow segment over a quarter: pi (Rmax^2 - Rmin^2) 2H / 4
  MDShapeTUBS Segment("Segment");
  Segment.Set(1, 3, 2, 30, 120);
  Passed = EvaluateNear("GetVolume()", "quarter", "The volume is a quarter of the hollow cylinder", Segment.GetVolume(), c_Pi*8*4/4, 1e-10) && Passed;
  Passed = EvaluateVectorNear("GetSize()", "quarter", "The surrounding box is given by Rmax and the half height", Segment.GetSize(), MVector(3, 3, 2), 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the inside check
bool UTDShapeTUBS::TestIsInside()
{
  bool Passed = true;

  const double Just = 1e-9;
  const double Small = 1e-6;

  MDShapeTUBS Shape("Tube");
  Shape.Set(1, 3, 2, 30, 120);
  Shape.Validate();

  MVector P(2*cos(70*c_Rad), 2*sin(70*c_Rad), -1.2);
  Passed = EvaluateTrue("IsInside()", "non-symmetric", "A non-symmetric interior point is inside", Shape.IsInside(P)) && Passed;
  Passed = EvaluateFalse("IsInside()", "center", "The center is outside", Shape.IsInside(MVector(0, 0, 0))) && Passed;

  // Radii at phi = 90:
  Passed = EvaluateTrue("IsInside()", "just below Rmax", "A point just inside Rmax is inside", Shape.IsInside(MVector(0, 3 - Just, 0))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just above Rmax", "A point just outside Rmax is outside", Shape.IsInside(MVector(0, 3 + Just, 0))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just above Rmin", "A point just outside Rmin is inside", Shape.IsInside(MVector(0, 1 + Just, 0))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just below Rmin", "A point just inside Rmin is outside", Shape.IsInside(MVector(0, 1 - Just, 0))) && Passed;
  // Height:
  Passed = EvaluateTrue("IsInside()", "just below H", "A point just inside the top is inside", Shape.IsInside(MVector(0, 2, 2 - Just))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just above H", "A point just outside the top is outside", Shape.IsInside(MVector(0, 2, 2 + Just))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just above -H", "A point just inside the bottom is inside", Shape.IsInside(MVector(0, 2, -2 + Just))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just below -H", "A point just outside the bottom is outside", Shape.IsInside(MVector(0, 2, -2 - Just))) && Passed;
  // Phi:
  Passed = EvaluateTrue("IsInside()", "just above Phi1", "A point just inside Phi1 is inside", Shape.IsInside(MVector(2*cos((30 + Small)*c_Rad), 2*sin((30 + Small)*c_Rad), 0))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just below Phi1", "A point just outside Phi1 is outside", Shape.IsInside(MVector(2*cos((30 - Small)*c_Rad), 2*sin((30 - Small)*c_Rad), 0))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just below Phi2", "A point just inside Phi2 is inside", Shape.IsInside(MVector(2*cos((120 - Small)*c_Rad), 2*sin((120 - Small)*c_Rad), 0))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just above Phi2", "A point just outside Phi2 is outside", Shape.IsInside(MVector(2*cos((120 + Small)*c_Rad), 2*sin((120 + Small)*c_Rad), 0))) && Passed;

  // Tolerance:
  Passed = CheckToleranceAtSurface("Rmax", Shape, MVector(0, 3, 0), MVector(0, 1, 0)) && Passed;
  Passed = CheckToleranceAtSurface("Rmin", Shape, MVector(0, 1, 0), MVector(0, -1, 0)) && Passed;
  Passed = CheckToleranceAtSurface("top", Shape, MVector(0, 2, 2), MVector(0, 0, 1)) && Passed;

  // A segment over phi = 360 = 0:
  MDShapeTUBS Over("Over");
  Over.Set(1, 3, 2, 350, 370);
  Over.Validate();
  Passed = EvaluateTrue("IsInside()", "over 0 deg", "Phi = 0 is inside a segment from 350 to 370 deg", Over.IsInside(MVector(2, 0, 0))) && Passed;
  Passed = EvaluateFalse("IsInside()", "over 0 deg", "Phi = 180 is outside a segment from 350 to 370 deg", Over.IsInside(MVector(-2, 0, 0))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the distance functions with fixed rays
bool UTDShapeTUBS::TestDistances()
{
  bool Passed = true;

  MDShapeTUBS Shape("Tube");
  Shape.Set(1, 3, 2);
  Shape.Validate();

  Passed = EvaluateNear("DistanceInsideOut()", "outward", "The distance to Rmax", Shape.DistanceInsideOut(MVector(2, 0, 0.5), MVector(1, 0, 0)), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "inward", "The distance to Rmin", Shape.DistanceInsideOut(MVector(2, 0, 0.5), MVector(-1, 0, 0)), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "top", "The distance to the top", Shape.DistanceInsideOut(MVector(2, 0, 0.5), MVector(0, 0, 1)), 1.5, 1e-12) && Passed;
  // Chord: from (0, 2, 0) along x the radius 3 is reached after sqrt(9 - 4)
  Passed = EvaluateNear("DistanceInsideOut()", "chord", "The distance along a chord", Shape.DistanceInsideOut(MVector(0, 2, 0), MVector(1, 0, 0)), sqrt(5.0), 1e-12) && Passed;
  Passed = EvaluateNear("DistanceOutsideIn()", "from outside", "The distance to Rmax from outside", Shape.DistanceOutsideIn(MVector(-5, 0, 0), MVector(1, 0, 0)), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceOutsideIn()", "from the hole", "The distance to Rmin from the center", Shape.DistanceOutsideIn(MVector(0, 0, 0), MVector(0, 1, 0)), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceOutsideIn()", "from above", "The distance to the top from above", Shape.DistanceOutsideIn(MVector(2, 0, 7), MVector(0, 0, -1)), 5.0, 1e-12) && Passed;
  Passed = EvaluateTrue("DistanceOutsideIn()", "miss", "A ray away from the tube is no hit", Shape.DistanceOutsideIn(MVector(5, 0, 0), MVector(1, 0, 0)) > 1e20) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the unique position
bool UTDShapeTUBS::TestUniquePosition()
{
  bool Passed = true;

  // Hollow segment: middle of the radii and of the angles, height 0
  MDShapeTUBS Segment("Segment");
  Segment.Set(1, 3, 2, 30, 120);
  Segment.Validate();
  Passed = EvaluateVectorNear("GetUniquePosition()", "hollow", "The position is in the middle of the ranges", Segment.GetUniquePosition(), MVector(2*cos(75*c_Rad), 2*sin(75*c_Rad), 0), 1e-12) && Passed;
  Passed = EvaluateTrue("GetUniquePosition()", "hollow", "The position is inside", Segment.IsInside(Segment.GetUniquePosition())) && Passed;

  // Full cylinder: the center
  MDShapeTUBS Full("Full");
  Full.Set(0, 3, 2);
  Full.Validate();
  Passed = EvaluateVectorNear("GetUniquePosition()", "cylinder", "The position is the center", Full.GetUniquePosition(), MVector(0, 0, 0), 1e-12) && Passed;
  Passed = EvaluateTrue("GetUniquePosition()", "cylinder", "The position is inside", Full.IsInside(Full.GetUniquePosition())) && Passed;

  // Defect GEO-C-12: A solid segment which does not start at phi = 0 returns the axis, which is not part of the shape
  MDShapeTUBS Wedge("Wedge");
  Wedge.Set(0, 3, 2, 10, 50);
  Wedge.Validate();
  Passed = ExpectedToFail(EvaluateTrue("GetUniquePosition()", "solid segment from 10 to 50 deg", "The position is inside", Wedge.IsInside(Wedge.GetUniquePosition()))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the geometry against the analytic truth
bool UTDShapeTUBS::TestAgainstTruth()
{
  bool Passed = true;

  const MVector Min(-3.5, -3.5, -2.5);
  const MVector Max(3.5, 3.5, 2.5);

  MDShapeTUBS Full("Full");
  Full.Set(0, 3, 2);
  Full.Validate();
  MTruth TruthFull = GetTruth(0, 3, 2, 0, 360);
  Passed = CheckTruthVolume("cylinder", TruthFull, Min, Max, Full.GetVolume()) && Passed;
  Passed = CheckAgainstTruth("cylinder", Full, TruthFull, Min, Max) && Passed;

  MDShapeTUBS Segment("Segment");
  Segment.Set(1, 3, 2, 30, 120);
  Segment.Validate();
  MTruth TruthSegment = GetTruth(1, 3, 2, 30, 120);
  Passed = CheckTruthVolume("segment", TruthSegment, Min, Max, Segment.GetVolume()) && Passed;
  Passed = CheckAgainstTruth("segment", Segment, TruthSegment, Min, Max) && Passed;

  MDShapeTUBS Over("Over");
  Over.Set(1, 3, 2, 300, 400);
  Over.Validate();
  MTruth TruthOver = GetTruth(1, 3, 2, 300, 400);
  Passed = CheckTruthVolume("over 0 deg", TruthOver, Min, Max, Over.GetVolume()) && Passed;
  Passed = CheckAgainstTruth("over 0 deg", Over, TruthOver, Min, Max) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the string representations
bool UTDShapeTUBS::TestStrings()
{
  bool Passed = true;

  MDShapeTUBS Shape("Tube");
  Shape.Set(1, 3, 2, 30, 120);
  Passed = EvaluateTrue("ToString()", "segment", "The string contains the type and parameters", Shape.ToString().Contains("TUBS (1,3,2,30,120,")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "segment", "The text is the keyword with the parameters", Shape.GetGeomega() == "TUBS 1 3 2 30 120") && Passed;

  MDShapeTUBS Read("Read");
  Passed = EvaluateTrue("Parse()", "round trip", "The text can be parsed", ParseQuiet(Read, MString("V.Shape ") + Shape.GetGeomega())) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "Rmin survives", Read.GetRmin(), 1.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "Rmax survives", Read.GetRmax(), 3.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "The half height survives", Read.GetHeight(), 2.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "Phi1 survives", Read.GetPhi1(), 30.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "Phi2 survives", Read.GetPhi2(), 120.0, 1e-4) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the scaling
bool UTDShapeTUBS::TestScale()
{
  bool Passed = true;

  MDShapeTUBS All("All");
  All.Set(1, 3, 2, 30, 120);
  All.Validate();
  double Volume = All.GetVolume();
  Passed = CheckScaleState("XYZ", All, 2.0, "XYZ") && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "Rmin is scaled", All.GetRmin(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "Rmax is scaled", All.GetRmax(), 6.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "The half height is scaled", All.GetHeight(), 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "The angles are not scaled", All.GetPhi2(), 120.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "The volume is scaled by the third power", All.GetVolume(), 8*Volume, 1e-9) && Passed;
  Passed = CheckInsideAgainstTruth("XYZ", All, GetTruth(2, 6, 4, 30, 120), MVector(-7, -7, -5), MVector(7, 7, 5)) && Passed;

  MDShapeTUBS XY("XY");
  XY.Set(1, 3, 2);
  XY.Validate();
  Passed = EvaluateTrue("Scale()", "XY", "The axes XY are accepted", XY.Scale(2.0, "XY")) && Passed;
  Passed = EvaluateVectorNear("Scale()", "XY", "Only the radii are scaled", XY.GetSize(), MVector(6, 6, 2), 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XY", "Rmin is scaled", XY.GetRmin(), 2.0, 1e-12) && Passed;

  MDShapeTUBS Z("Z");
  Z.Set(1, 3, 2);
  Z.Validate();
  Passed = EvaluateTrue("Scale()", "Z", "The axis Z is accepted", Z.Scale(2.0, "Z")) && Passed;
  Passed = EvaluateVectorNear("Scale()", "Z", "Only the height is scaled", Z.GetSize(), MVector(3, 3, 4), 1e-12) && Passed;

  MDShapeTUBS X("X");
  X.Set(1, 3, 2);
  X.Validate();
  Passed = EvaluateFalse("Scale()", "X", "The radius cannot be scaled along X alone", Quiet([&]() { return X.Scale(2.0, "X"); })) && Passed;
  Passed = EvaluateVectorNear("Scale()", "X", "A rejected axis keeps the size", X.GetSize(), MVector(3, 3, 2), 1e-12) && Passed;

  // Defect GEO-C-6: A rejected axis combination leaves the shape marked as scaled
  Passed = ExpectedToFail(EvaluateFalse("IsScaled()", "after a rejected scaling", "The shape is not marked as scaled", X.IsScaled())) && Passed;

  // Defect GEO-C-5: A negative factor is accepted while Set() rejects negative radii
  MDShapeTUBS Negative("Negative");
  Negative.Set(1, 3, 2);
  Negative.Validate();
  Passed = ExpectedToFail(EvaluateFalse("Scale()", "factor -2", "A negative factor is rejected", Quiet([&]() { return Negative.Scale(-2.0, "XYZ"); }))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parsing the keywords
bool UTDShapeTUBS::TestParse()
{
  bool Passed = true;

  MDShapeTUBS Short("Short");
  Passed = EvaluateTrue("Parse()", "Parameters, 3", "Three parameters are accepted", ParseQuiet(Short, "Short.Parameters 1.5 2.5 3.5")) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters, 3", "Rmin is read", Short.GetRmin(), 1.5, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters, 3", "Rmax is read", Short.GetRmax(), 2.5, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters, 3", "The half height is read", Short.GetHeight(), 3.5, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters, 3", "Phi2 is the full range", Short.GetPhi2(), 360.0, 1e-15) && Passed;

  MDShapeTUBS Long("Long");
  Passed = EvaluateTrue("Parse()", "Parameters, 5", "Five parameters are accepted", ParseQuiet(Long, "Long.Parameters 1.5 2.5 3.5 10 20")) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters, 5", "Phi1 is read", Long.GetPhi1(), 10.0, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters, 5", "Phi2 is read", Long.GetPhi2(), 20.0, 1e-15) && Passed;

  MDShapeTUBS Tube("Tube");
  Passed = EvaluateTrue("Parse()", "Shape TUBE", "The alternative type is accepted", ParseQuiet(Tube, "V.Shape TUBE 1.5 2.5 3.5")) && Passed;
  Passed = EvaluateNear("Parse()", "Shape TUBE", "The parameters start after the type", Tube.GetRmax(), 2.5, 1e-15) && Passed;

  MDShapeTUBS Bad("Bad");
  Bad.Set(1, 2, 3);
  Passed = EvaluateFalse("Parse()", "2 parameters", "Two parameters are rejected", ParseQuiet(Bad, "Bad.Parameters 4 5")) && Passed;
  Passed = EvaluateFalse("Parse()", "4 parameters", "Four parameters are rejected", ParseQuiet(Bad, "Bad.Parameters 4 5 6 7")) && Passed;
  Passed = EvaluateFalse("Parse()", "6 parameters", "Six parameters are rejected", ParseQuiet(Bad, "Bad.Parameters 4 5 6 7 8 9")) && Passed;
  Passed = EvaluateFalse("Parse()", "wrong keyword", "A wrong keyword is rejected", ParseQuiet(Bad, "Bad.Param 4 5 6")) && Passed;
  Passed = EvaluateFalse("Parse()", "invalid, 3", "Rmin > Rmax is rejected", ParseQuiet(Bad, "Bad.Parameters 5 4 6")) && Passed;
  Passed = EvaluateFalse("Parse()", "invalid, 5", "Invalid angles are rejected", ParseQuiet(Bad, "Bad.Parameters 4 5 6 20 10")) && Passed;
  Passed = EvaluateNear("Parse()", "malformed", "The rejected input keeps Rmax", Bad.GetRmax(), 2.0, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test configuring a validated shape again
bool UTDShapeTUBS::TestReconfiguration()
{
  MDShapeTUBS Shape("S");
  return CheckReconfiguration("reconfiguration", Shape, MVector(0.0, 0.0, 1.0), [&Shape]() { return Shape.Set(0.0, 5.0, 1.0); }, 1.0, [&Shape]() { return Shape.Set(0.0, 5.0, 2.0); }, 2.0, "S.Parameters 0 5 3", 3.0, [&Shape]() { return Shape.Set(-1.0, 5.0, 1.0); });
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  gErrorIgnoreLevel = kWarning;

  UTDShapeTUBS Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDShapeTUBS.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
