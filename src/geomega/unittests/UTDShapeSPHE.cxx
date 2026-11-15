/*
 * UTDShapeSPHE.cxx
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
#include "MDShapeSPHE.h"
#include "UTDShapeShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the sphere shape MDShapeSPHE
class UTDShapeSPHE : public UTDShapeBase
{
public:
  //! Default constructor
  UTDShapeSPHE() : UTDShapeBase("UTDShapeSPHE") {}
  //! Default destructor
  virtual ~UTDShapeSPHE() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the analytic truth of a sphere segment, the angles are in degrees
  MTruth GetTruth(double Rmin, double Rmax, double Thetamin, double Thetamax, double Phimin, double Phimax);
  //! Return the volume of a sphere segment from the integral r^2 sin(theta) dr dtheta dphi
  double GetTruthVolume(double Rmin, double Rmax, double Thetamin, double Thetamax, double Phimin, double Phimax);

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
bool UTDShapeSPHE::Run()
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


//! Return the analytic truth of a sphere segment, the angles are in degrees
UTDShapeBase::MTruth UTDShapeSPHE::GetTruth(double Rmin, double Rmax, double Thetamin, double Thetamax, double Phimin, double Phimax)
{
  return [=](double x, double y, double z) {
    double R = sqrt(x*x + y*y + z*z);
    if (R < Rmin || R > Rmax || R == 0) {
      return false;
    }
    double Theta = acos(z/R)*c_Deg;
    if (Theta < Thetamin || Theta > Thetamax) {
      return false;
    }
    double Phi = atan2(y, x)*c_Deg;
    if (Phi < 0) {
      Phi += 360;
    }
    if (Phi < Phimin || Phi > Phimax) {
      return false;
    }
    return true;
  };
}


////////////////////////////////////////////////////////////////////////////////


//! Return the volume of a sphere segment from the integral r^2 sin(theta) dr dtheta dphi
double UTDShapeSPHE::GetTruthVolume(double Rmin, double Rmax, double Thetamin, double Thetamax, double Phimin, double Phimax)
{
  return (pow(Rmax, 3) - pow(Rmin, 3))/3*(cos(Thetamin*c_Rad) - cos(Thetamax*c_Rad))*(Phimax - Phimin)*c_Rad;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default state
bool UTDShapeSPHE::TestDefault()
{
  bool Passed = true;

  MDShapeSPHE Shape("Sphere");
  Passed = EvaluateTrue("GetName()", "constructor", "The name of the constructor is stored", Shape.GetName() == "Sphere") && Passed;
  Passed = EvaluateTrue("GetType()", "constructor", "The type is SPHE", Shape.GetType() == "SPHE") && Passed;
  Passed = EvaluateNear("GetVolume()", "default", "A new sphere has no volume", Shape.GetVolume(), 0.0, 1e-15) && Passed;
  Passed = EvaluateVectorNear("GetSize()", "default", "A new sphere has no size", Shape.GetSize(), MVector(0, 0, 0), 1e-15) && Passed;
  Passed = EvaluateNear("GetRmin()", "default", "The default is zero", Shape.GetRmin(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetRmax()", "default", "The default is zero", Shape.GetRmax(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetThetamin()", "default", "The default is zero", Shape.GetThetamin(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetThetamax()", "default", "The default is zero", Shape.GetThetamax(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetPhimin()", "default", "The default is zero", Shape.GetPhimin(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetPhimax()", "default", "The default is zero", Shape.GetPhimax(), 0.0, 1e-15) && Passed;

  // Defect GEO-C-7: A shape without parameters passes the validation and becomes a zero-size shape
  MDShapeSPHE NoParameters("NoParameters");
  Passed = ExpectedToFail(EvaluateFalse("Validate()", "no parameters", "A sphere without parameters is rejected", Quiet([&]() { return NoParameters.Validate(); }))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the limits of the parameters
bool UTDShapeSPHE::TestLimits()
{
  bool Passed = true;

  const double Just = 1e-9;
  MDShapeSPHE S("Sphere");

  // The radii of the short form:
  Passed = EvaluateTrue("Set(2)", "Rmin = 0", "Rmin = 0 is accepted", Quiet([&]() { return S.Set(0, 1); })) && Passed;
  Passed = EvaluateNear("Set(2)", "defaults", "The maximum theta is 180 deg", S.GetThetamax(), 180.0, 1e-15) && Passed;
  Passed = EvaluateNear("Set(2)", "defaults", "The maximum phi is 360 deg", S.GetPhimax(), 360.0, 1e-15) && Passed;
  Passed = EvaluateFalse("Set(2)", "Rmin just below 0", "A negative Rmin is rejected", Quiet([&]() { return S.Set(-Just, 1); })) && Passed;
  Passed = EvaluateTrue("Set(2)", "Rmax just above 0", "A small Rmax is accepted", Quiet([&]() { return S.Set(0, Just); })) && Passed;
  Passed = EvaluateFalse("Set(2)", "Rmax = 0", "Rmax = 0 is rejected", Quiet([&]() { return S.Set(0, 0); })) && Passed;
  Passed = EvaluateTrue("Set(2)", "Rmin just below Rmax", "Rmin below Rmax is accepted", Quiet([&]() { return S.Set(1 - Just, 1); })) && Passed;
  Passed = EvaluateFalse("Set(2)", "Rmin = Rmax", "Rmin = Rmax is rejected", Quiet([&]() { return S.Set(1, 1); })) && Passed;

  // The radii of the long form:
  Passed = EvaluateTrue("Set(6)", "Rmin = 0", "Rmin = 0 is accepted", Quiet([&]() { return S.Set(0, 1, 0, 180, 0, 360); })) && Passed;
  Passed = EvaluateFalse("Set(6)", "Rmin just below 0", "A negative Rmin is rejected", Quiet([&]() { return S.Set(-Just, 1, 0, 180, 0, 360); })) && Passed;
  Passed = EvaluateFalse("Set(6)", "Rmax = 0", "Rmax = 0 is rejected", Quiet([&]() { return S.Set(0, 0, 0, 180, 0, 360); })) && Passed;
  Passed = EvaluateFalse("Set(6)", "Rmin = Rmax", "Rmin = Rmax is rejected", Quiet([&]() { return S.Set(1, 1, 0, 180, 0, 360); })) && Passed;

  // Theta:
  Passed = EvaluateTrue("Set(6)", "Thetamin = 0", "The lower limit of Thetamin is accepted", Quiet([&]() { return S.Set(0, 1, 0, 180, 0, 360); })) && Passed;
  Passed = EvaluateFalse("Set(6)", "Thetamin just below 0", "Thetamin below 0 is rejected", Quiet([&]() { return S.Set(0, 1, -Just, 180, 0, 360); })) && Passed;
  Passed = EvaluateTrue("Set(6)", "Thetamin just below 180", "Thetamin just below 180 is accepted", Quiet([&]() { return S.Set(0, 1, 180 - Just, 180, 0, 360); })) && Passed;
  Passed = EvaluateFalse("Set(6)", "Thetamin = 180", "Thetamin = 180 is rejected", Quiet([&]() { return S.Set(0, 1, 180, 180, 0, 360); })) && Passed;
  Passed = EvaluateTrue("Set(6)", "Thetamax just above 0", "Thetamax just above 0 is accepted", Quiet([&]() { return S.Set(0, 1, 0, Just, 0, 360); })) && Passed;
  Passed = EvaluateFalse("Set(6)", "Thetamax = 0", "Thetamax = 0 is rejected", Quiet([&]() { return S.Set(0, 1, 0, 0, 0, 360); })) && Passed;
  Passed = EvaluateTrue("Set(6)", "Thetamax = 180", "The upper limit of Thetamax is accepted", Quiet([&]() { return S.Set(0, 1, 0, 180, 0, 360); })) && Passed;
  Passed = EvaluateFalse("Set(6)", "Thetamax just above 180", "Thetamax above 180 is rejected", Quiet([&]() { return S.Set(0, 1, 0, 180 + Just, 0, 360); })) && Passed;
  Passed = EvaluateTrue("Set(6)", "Thetamin just below Thetamax", "Thetamin below Thetamax is accepted", Quiet([&]() { return S.Set(0, 1, 50 - Just, 50, 0, 360); })) && Passed;
  Passed = EvaluateFalse("Set(6)", "Thetamin = Thetamax", "Thetamin = Thetamax is rejected", Quiet([&]() { return S.Set(0, 1, 50, 50, 0, 360); })) && Passed;

  // Phi:
  Passed = EvaluateFalse("Set(6)", "Phimin just below 0", "Phimin below 0 is rejected", Quiet([&]() { return S.Set(0, 1, 0, 180, -Just, 360); })) && Passed;
  Passed = EvaluateTrue("Set(6)", "Phimin just below 360", "Phimin just below 360 is accepted", Quiet([&]() { return S.Set(0, 1, 0, 180, 360 - Just, 360); })) && Passed;
  Passed = EvaluateFalse("Set(6)", "Phimin = 360", "Phimin = 360 is rejected", Quiet([&]() { return S.Set(0, 1, 0, 180, 360, 360); })) && Passed;
  Passed = EvaluateTrue("Set(6)", "Phimax just above 0", "Phimax just above 0 is accepted", Quiet([&]() { return S.Set(0, 1, 0, 180, 0, Just); })) && Passed;
  Passed = EvaluateFalse("Set(6)", "Phimax = 0", "Phimax = 0 is rejected", Quiet([&]() { return S.Set(0, 1, 0, 180, 0, 0); })) && Passed;
  Passed = EvaluateFalse("Set(6)", "Phimax just above 360", "Phimax above 360 is rejected", Quiet([&]() { return S.Set(0, 1, 0, 180, 0, 360 + Just); })) && Passed;
  Passed = EvaluateTrue("Set(6)", "Phimin just below Phimax", "Phimin below Phimax is accepted", Quiet([&]() { return S.Set(0, 1, 0, 180, 100 - Just, 100); })) && Passed;
  Passed = EvaluateFalse("Set(6)", "Phimin = Phimax", "Phimin = Phimax is rejected", Quiet([&]() { return S.Set(0, 1, 0, 180, 100, 100); })) && Passed;

  // A rejected set does not change the shape:
  S.Set(1, 2, 30, 60, 90, 120);
  Quiet([&]() { return S.Set(5, 4, 1, 2, 3, 4); });
  Passed = EvaluateNear("GetRmin()", "after reject", "The old Rmin is kept", S.GetRmin(), 1.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetRmax()", "after reject", "The old Rmax is kept", S.GetRmax(), 2.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetThetamin()", "after reject", "The old Thetamin is kept", S.GetThetamin(), 30.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetThetamax()", "after reject", "The old Thetamax is kept", S.GetThetamax(), 60.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetPhimin()", "after reject", "The old Phimin is kept", S.GetPhimin(), 90.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetPhimax()", "after reject", "The old Phimax is kept", S.GetPhimax(), 120.0, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the volume and the size
bool UTDShapeSPHE::TestVolumeSize()
{
  bool Passed = true;

  // Full sphere: 4/3 pi R^3
  MDShapeSPHE Full("Full");
  Full.Set(0, 3);
  Passed = EvaluateNear("GetVolume()", "full sphere", "The volume is 4/3 pi R^3", Full.GetVolume(), 4.0/3.0*c_Pi*27, 1e-10) && Passed;

  // Hollow sphere: 4/3 pi (Rmax^3 - Rmin^3)
  MDShapeSPHE Hollow("Hollow");
  Hollow.Set(1, 3);
  Passed = EvaluateNear("GetVolume()", "hollow sphere", "The volume is the difference of two spheres", Hollow.GetVolume(), 4.0/3.0*c_Pi*26, 1e-10) && Passed;

  // The upper half: half of a sphere
  MDShapeSPHE Half("Half");
  Half.Set(0, 3, 0, 90, 0, 360);
  Passed = EvaluateNear("GetVolume()", "half sphere", "The volume is half of the sphere", Half.GetVolume(), 2.0/3.0*c_Pi*27, 1e-10) && Passed;

  // A non-symmetric segment:
  MDShapeSPHE Segment("Segment");
  Segment.Set(1, 3, 30, 120, 20, 200);
  Passed = EvaluateNear("GetVolume()", "segment", "The volume agrees with the integral", Segment.GetVolume(), GetTruthVolume(1, 3, 30, 120, 20, 200), 1e-10) && Passed;

  Passed = EvaluateVectorNear("GetSize()", "segment", "The surrounding box is given by Rmax", Segment.GetSize(), MVector(3, 3, 3), 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the inside check
bool UTDShapeSPHE::TestIsInside()
{
  bool Passed = true;

  const double Just = 1e-9;

  MDShapeSPHE Shape("Sphere");
  Shape.Set(1, 3);
  Shape.Validate();

  Passed = EvaluateFalse("IsInside()", "center", "The center of a hollow sphere is outside", Shape.IsInside(MVector(0, 0, 0))) && Passed;
  Passed = EvaluateTrue("IsInside()", "non-symmetric", "A non-symmetric interior point is inside", Shape.IsInside(MVector(-1.1, 0.7, 1.5))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just below Rmax", "A point just inside Rmax is inside", Shape.IsInside(MVector(0, 0, 3 - Just))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just above Rmax", "A point just outside Rmax is outside", Shape.IsInside(MVector(0, 0, 3 + Just))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just above Rmin", "A point just outside Rmin is inside", Shape.IsInside(MVector(0, 1 + Just, 0))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just below Rmin", "A point just inside Rmin is outside", Shape.IsInside(MVector(0, 1 - Just, 0))) && Passed;
  Passed = CheckToleranceAtSurface("Rmax", Shape, MVector(0, 0, 3), MVector(0, 0, 1)) && Passed;
  Passed = CheckToleranceAtSurface("Rmin", Shape, MVector(1, 0, 0), MVector(-1, 0, 0)) && Passed;

  // The limits in theta and phi: theta 30 to 120, phi 20 to 200
  MDShapeSPHE Segment("Segment");
  Segment.Set(1, 3, 30, 120, 20, 200);
  Segment.Validate();
  const double Small = 1e-6;
  MVector P;
  P.SetMagThetaPhi(2, (30 + Small)*c_Rad, 100*c_Rad);
  Passed = EvaluateTrue("IsInside()", "just inside Thetamin", "A point just inside Thetamin is inside", Segment.IsInside(P)) && Passed;
  P.SetMagThetaPhi(2, (30 - Small)*c_Rad, 100*c_Rad);
  Passed = EvaluateFalse("IsInside()", "just outside Thetamin", "A point just outside Thetamin is outside", Segment.IsInside(P)) && Passed;
  P.SetMagThetaPhi(2, (120 - Small)*c_Rad, 100*c_Rad);
  Passed = EvaluateTrue("IsInside()", "just inside Thetamax", "A point just inside Thetamax is inside", Segment.IsInside(P)) && Passed;
  P.SetMagThetaPhi(2, (120 + Small)*c_Rad, 100*c_Rad);
  Passed = EvaluateFalse("IsInside()", "just outside Thetamax", "A point just outside Thetamax is outside", Segment.IsInside(P)) && Passed;
  P.SetMagThetaPhi(2, 90*c_Rad, (20 + Small)*c_Rad);
  Passed = EvaluateTrue("IsInside()", "just inside Phimin", "A point just inside Phimin is inside", Segment.IsInside(P)) && Passed;
  P.SetMagThetaPhi(2, 90*c_Rad, (20 - Small)*c_Rad);
  Passed = EvaluateFalse("IsInside()", "just outside Phimin", "A point just outside Phimin is outside", Segment.IsInside(P)) && Passed;
  P.SetMagThetaPhi(2, 90*c_Rad, (200 - Small)*c_Rad);
  Passed = EvaluateTrue("IsInside()", "just inside Phimax", "A point just inside Phimax is inside", Segment.IsInside(P)) && Passed;
  P.SetMagThetaPhi(2, 90*c_Rad, (200 + Small)*c_Rad);
  Passed = EvaluateFalse("IsInside()", "just outside Phimax", "A point just outside Phimax is outside", Segment.IsInside(P)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the distance functions with fixed rays
bool UTDShapeSPHE::TestDistances()
{
  bool Passed = true;

  MDShapeSPHE Shape("Sphere");
  Shape.Set(1, 3);
  Shape.Validate();

  // Radial rays:
  Passed = EvaluateNear("DistanceInsideOut()", "outward", "The distance to Rmax", Shape.DistanceInsideOut(MVector(2, 0, 0), MVector(1, 0, 0)), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "inward", "The distance to Rmin", Shape.DistanceInsideOut(MVector(2, 0, 0), MVector(-1, 0, 0)), 1.0, 1e-12) && Passed;

  // Tangential: from (0, 2, 0) along x the sphere of radius 3 is reached after sqrt(9 - 4)
  Passed = EvaluateNear("DistanceInsideOut()", "tangential", "The distance along a chord", Shape.DistanceInsideOut(MVector(0, 2, 0), MVector(1, 0, 0)), sqrt(5.0), 1e-12) && Passed;

  // From outside and the re-entry into the hollow part:
  Passed = EvaluateNear("DistanceOutsideIn()", "from outside", "The distance to Rmax from the outside", Shape.DistanceOutsideIn(MVector(-5, 0, 0), MVector(1, 0, 0)), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceOutsideIn()", "from the hollow part", "The distance to Rmin from the center", Shape.DistanceOutsideIn(MVector(0, 0, 0), MVector(0, 0, 1)), 1.0, 1e-12) && Passed;
  Passed = EvaluateTrue("DistanceOutsideIn()", "miss", "A ray away from the sphere is no hit", Shape.DistanceOutsideIn(MVector(5, 0, 0), MVector(1, 0, 0)) > 1e20) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the unique position
bool UTDShapeSPHE::TestUniquePosition()
{
  bool Passed = true;

  // In the middle of all parameter ranges:
  MDShapeSPHE Segment("Segment");
  Segment.Set(1, 3, 30, 120, 20, 200);
  Segment.Validate();
  MVector Expected;
  Expected.SetMagThetaPhi(2, 75*c_Rad, 110*c_Rad);
  Passed = EvaluateVectorNear("GetUniquePosition()", "segment", "The position is in the middle of the ranges", Segment.GetUniquePosition(), Expected, 1e-12) && Passed;
  Passed = EvaluateTrue("GetUniquePosition()", "segment", "The position is inside", Segment.IsInside(Segment.GetUniquePosition())) && Passed;

  MDShapeSPHE Full("Full");
  Full.Set(0, 3);
  Full.Validate();
  Passed = EvaluateTrue("GetUniquePosition()", "full sphere", "The position is inside", Full.IsInside(Full.GetUniquePosition())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the geometry against the analytic truth
bool UTDShapeSPHE::TestAgainstTruth()
{
  bool Passed = true;

  const MVector Min(-3.5, -3.5, -3.5);
  const MVector Max(3.5, 3.5, 3.5);

  // Full sphere:
  MDShapeSPHE Full("Full");
  Full.Set(0, 3);
  Full.Validate();
  MTruth TruthFull = GetTruth(0, 3, 0, 180, 0, 360);
  Passed = CheckTruthVolume("full sphere", TruthFull, Min, Max, Full.GetVolume()) && Passed;
  Passed = CheckAgainstTruth("full sphere", Full, TruthFull, Min, Max) && Passed;

  // A hollow segment:
  MDShapeSPHE Segment("Segment");
  Segment.Set(1, 3, 30, 120, 20, 200);
  Segment.Validate();
  MTruth TruthSegment = GetTruth(1, 3, 30, 120, 20, 200);
  Passed = CheckTruthVolume("segment", TruthSegment, Min, Max, Segment.GetVolume()) && Passed;
  Passed = CheckAgainstTruth("segment", Segment, TruthSegment, Min, Max) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the string representations
bool UTDShapeSPHE::TestStrings()
{
  bool Passed = true;

  MDShapeSPHE Shape("Sphere");
  Shape.Set(1, 3, 30, 120, 20, 200);
  Passed = EvaluateTrue("ToString()", "segment", "The string contains type and parameters", Shape.ToString().Contains("SPHE (1,3,30,120,20,200)")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "segment", "The text is the keyword with the parameters", Shape.GetGeomega() == "SPHE 1 3 30 120 20 200") && Passed;

  // Round trip through the keyword as written by the volume:
  MDShapeSPHE Read("Read");
  Passed = EvaluateTrue("Parse()", "round trip", "The text can be parsed", ParseQuiet(Read, MString("V.Shape ") + Shape.GetGeomega())) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "Rmin survives", Read.GetRmin(), 1.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "Rmax survives", Read.GetRmax(), 3.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "Thetamin survives", Read.GetThetamin(), 30.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "Thetamax survives", Read.GetThetamax(), 120.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "Phimin survives", Read.GetPhimin(), 20.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "Phimax survives", Read.GetPhimax(), 200.0, 1e-4) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the scaling
bool UTDShapeSPHE::TestScale()
{
  bool Passed = true;

  MDShapeSPHE Shape("Sphere");
  Shape.Set(1, 3, 30, 120, 20, 200);
  Shape.Validate();
  double Volume = Shape.GetVolume();
  Passed = CheckScaleState("XYZ", Shape, 2.0, "XYZ") && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "Rmin is scaled", Shape.GetRmin(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "Rmax is scaled", Shape.GetRmax(), 6.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "The angles are not scaled", Shape.GetThetamax(), 120.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "The volume is scaled by the third power", Shape.GetVolume(), 8*Volume, 1e-9) && Passed;
  Passed = CheckInsideAgainstTruth("XYZ", Shape, GetTruth(2, 6, 30, 120, 20, 200), MVector(-7, -7, -7), MVector(7, 7, 7)) && Passed;

  // Only the same factor for all axes is possible:
  for (const MString& Axes : vector<MString>{ "X", "Y", "Z", "XY", "XZ", "YZ" }) {
    MDShapeSPHE Axis("Axis");
    Axis.Set(1, 3);
    Axis.Validate();
    Passed = EvaluateFalse("Scale()", Axes, "Other axes than XYZ are rejected", Quiet([&]() { return Axis.Scale(2.0, Axes); })) && Passed;
    Passed = EvaluateNear("Scale()", Axes, "The rejected axes do not change the radius", Axis.GetRmax(), 3.0, 1e-12) && Passed;
  }

  // Defect GEO-C-6: A rejected axis combination leaves the shape marked as scaled
  MDShapeSPHE Rejected("Rejected");
  Rejected.Set(1, 3);
  Rejected.Validate();
  Quiet([&]() { return Rejected.Scale(2.0, "X"); });
  Passed = ExpectedToFail(EvaluateFalse("IsScaled()", "after a rejected scaling", "The shape is not marked as scaled", Rejected.IsScaled())) && Passed;

  // Defect GEO-C-5: A negative factor is accepted while Set() rejects negative radii
  MDShapeSPHE Negative("Negative");
  Negative.Set(1, 3);
  Negative.Validate();
  Passed = ExpectedToFail(EvaluateFalse("Scale()", "factor -2", "A negative factor is rejected", Quiet([&]() { return Negative.Scale(-2.0, "XYZ"); }))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parsing the keywords
bool UTDShapeSPHE::TestParse()
{
  bool Passed = true;

  MDShapeSPHE Short("Short");
  Passed = EvaluateTrue("Parse()", "Parameters, 2", "Two parameters are accepted", ParseQuiet(Short, "Short.Parameters 1.5 2.5")) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters, 2", "Rmin is read", Short.GetRmin(), 1.5, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters, 2", "Rmax is read", Short.GetRmax(), 2.5, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters, 2", "Thetamax is the full range", Short.GetThetamax(), 180.0, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters, 2", "Phimax is the full range", Short.GetPhimax(), 360.0, 1e-15) && Passed;

  MDShapeSPHE Long("Long");
  Passed = EvaluateTrue("Parse()", "Parameters, 6", "Six parameters are accepted", ParseQuiet(Long, "Long.Parameters 1 2 10 20 30 40")) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters, 6", "Rmin is read", Long.GetRmin(), 1.0, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters, 6", "Rmax is read", Long.GetRmax(), 2.0, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters, 6", "Thetamin is read", Long.GetThetamin(), 10.0, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters, 6", "Thetamax is read", Long.GetThetamax(), 20.0, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters, 6", "Phimin is read", Long.GetPhimin(), 30.0, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters, 6", "Phimax is read", Long.GetPhimax(), 40.0, 1e-15) && Passed;

  MDShapeSPHE ShapeKeyword("Keyword");
  Passed = EvaluateTrue("Parse()", "Shape SPHE", "The keyword Shape is accepted", ParseQuiet(ShapeKeyword, "V.Shape SPHE 1 2 10 20 30 40")) && Passed;
  Passed = EvaluateNear("Parse()", "Shape SPHE", "The parameters start after the type", ShapeKeyword.GetRmax(), 2.0, 1e-15) && Passed;
  MDShapeSPHE ShapeSphere("Sphere");
  Passed = EvaluateTrue("Parse()", "Shape SPHERE", "The alternative type is accepted", ParseQuiet(ShapeSphere, "V.Shape SPHERE 1 2")) && Passed;

  // Malformed input:
  MDShapeSPHE Bad("Bad");
  Bad.Set(1, 2);
  Passed = EvaluateFalse("Parse()", "1 parameter", "One parameter is rejected", ParseQuiet(Bad, "Bad.Parameters 3")) && Passed;
  Passed = EvaluateFalse("Parse()", "3 parameters", "Three parameters are rejected", ParseQuiet(Bad, "Bad.Parameters 3 4 5")) && Passed;
  Passed = EvaluateFalse("Parse()", "7 parameters", "Seven parameters are rejected", ParseQuiet(Bad, "Bad.Parameters 3 4 5 6 7 8 9")) && Passed;
  Passed = EvaluateFalse("Parse()", "wrong keyword", "A wrong keyword is rejected", ParseQuiet(Bad, "Bad.Param 3 4")) && Passed;
  Passed = EvaluateFalse("Parse()", "invalid, 2", "Rmin > Rmax is rejected", ParseQuiet(Bad, "Bad.Parameters 4 3")) && Passed;
  Passed = EvaluateFalse("Parse()", "invalid, 6", "Invalid angles are rejected", ParseQuiet(Bad, "Bad.Parameters 3 4 50 40 0 360")) && Passed;
  Passed = EvaluateNear("Parse()", "malformed", "The rejected input keeps Rmax", Bad.GetRmax(), 2.0, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test configuring a validated shape again
bool UTDShapeSPHE::TestReconfiguration()
{
  MDShapeSPHE Shape("S");
  return CheckReconfiguration("reconfiguration", Shape, MVector(1.0, 0.0, 0.0), [&Shape]() { return Shape.Set(0.0, 1.0); }, 1.0, [&Shape]() { return Shape.Set(0.0, 2.0); }, 2.0, "S.Parameters 0 3", 3.0, [&Shape]() { return Shape.Set(-1.0, 1.0); });
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  gErrorIgnoreLevel = kWarning;

  UTDShapeSPHE Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDShapeSPHE.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
