/*
 * UTDShapeGTRA.cxx
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
#include "MDShapeGTRA.h"
#include "UTDShapeShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the twisted trapezoid shape MDShapeGTRA
//! Attention: The class has no getters for its parameters, thus they are read via the text output
class UTDShapeGTRA : public UTDShapeBase
{
public:
  //! Default constructor
  UTDShapeGTRA() : UTDShapeBase("UTDShapeGTRA") {}
  //! Default destructor
  virtual ~UTDShapeGTRA() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the analytic truth of a twisted square bar with half side A along z, the twist is in degrees
  //! The faces are rotated by -Twist/2 and +Twist/2 as defined by the ROOT shape and the corners are connected linearly
  MTruth GetTwistedTruth(double Dz, double Twist, double A);

  //! Test the default state
  bool TestDefault();
  //! Test the limits of the parameters
  bool TestLimits();
  //! Test the normalization of the angles
  bool TestAngles();
  //! Test the planarity check of the side surfaces
  bool TestPlanarity();
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
bool UTDShapeGTRA::Run()
{
  bool Passed = true;

  Passed = TestDefault() && Passed;
  Passed = TestLimits() && Passed;
  Passed = TestAngles() && Passed;
  Passed = TestPlanarity() && Passed;
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


//! Return the analytic truth of a twisted square bar with half side A along z, the twist is in degrees
UTDShapeBase::MTruth UTDShapeGTRA::GetTwistedTruth(double Dz, double Twist, double A)
{
  vector<MVector> Bottom = GetTrapFace(0, 0, A, A, A, 0);
  vector<MVector> Top = Bottom;
  double Half = 0.5*Twist*c_Rad;
  for (unsigned int c = 0; c < 4; ++c) {
    double X = Bottom[c].X();
    double Y = Bottom[c].Y();
    Bottom[c] = MVector(X*cos(-Half) + Y*sin(-Half), -X*sin(-Half) + Y*cos(-Half), 0);
    Top[c] = MVector(X*cos(Half) + Y*sin(Half), -X*sin(Half) + Y*cos(Half), 0);
  }
  return GetInterpolatedQuadTruth(Dz, Bottom, Top);
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default state
bool UTDShapeGTRA::TestDefault()
{
  bool Passed = true;

  MDShapeGTRA Shape("Gtra");
  Passed = EvaluateTrue("GetName()", "constructor", "The name of the constructor is stored", Shape.GetName() == "Gtra") && Passed;
  Passed = EvaluateTrue("GetType()", "constructor", "The type is GTRA", Shape.GetType() == "GTRA") && Passed;
  Passed = EvaluateNear("GetVolume()", "default", "A new shape has no volume", Shape.GetVolume(), 0.0, 1e-15) && Passed;

  // Defect GEO-C-7: A shape without parameters passes the validation and becomes a zero-size shape
  MDShapeGTRA NoParameters("NoParameters");
  Passed = ExpectedToFail(EvaluateFalse("Validate()", "no parameters", "A shape without parameters is rejected", Quiet([&]() { return NoParameters.Validate(); }))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the limits of the parameters
bool UTDShapeGTRA::TestLimits()
{
  bool Passed = true;

  const double Just = 1e-9;
  MDShapeGTRA S("Gtra");

  Passed = EvaluateTrue("Set()", "Dz just above 0", "A small Dz is accepted", Quiet([&]() { return S.Set(Just, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "Dz = 0", "A zero Dz is rejected", Quiet([&]() { return S.Set(0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0); })) && Passed;

  Passed = EvaluateTrue("Set()", "Twist = 0", "No twist is accepted", Quiet([&]() { return S.Set(1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "Twist just below 0", "A negative twist is rejected", Quiet([&]() { return S.Set(1, 0, 0, -Just, 1, 1, 1, 0, 1, 1, 1, 0); })) && Passed;
  Passed = EvaluateTrue("Set()", "Twist just below 360", "A twist just below 360 is accepted", Quiet([&]() { return S.Set(1, 0, 0, 360 - 1e-6, 1, 1, 1, 0, 1, 1, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "Twist = 360", "A twist of 360 is rejected", Quiet([&]() { return S.Set(1, 0, 0, 360, 1, 1, 1, 0, 1, 1, 1, 0); })) && Passed;

  // The length limits, zero is not allowed here:
  Passed = EvaluateTrue("Set()", "H1 just above 0", "A small H1 is accepted", Quiet([&]() { return S.Set(1, 0, 0, 0, 1e-3, 1, 1, 0, 1, 1, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "H1 = 0", "H1 = 0 is rejected", Quiet([&]() { return S.Set(1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "Bl1 = 0", "Bl1 = 0 is rejected", Quiet([&]() { return S.Set(1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "Tl1 = 0", "Tl1 = 0 is rejected", Quiet([&]() { return S.Set(1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "H2 = 0", "H2 = 0 is rejected", Quiet([&]() { return S.Set(1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "Bl2 = 0", "Bl2 = 0 is rejected", Quiet([&]() { return S.Set(1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "Tl2 = 0", "Tl2 = 0 is rejected", Quiet([&]() { return S.Set(1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0); })) && Passed;

  // A rejected set does not change the shape:
  S.Set(1, 2, 3, 4, 5, 5, 5, 0, 5, 5, 5, 0);
  Quiet([&]() { return S.Set(-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1); });
  Passed = EvaluateTrue("Set()", "after reject", "The old parameters are kept", S.GetGeomega() == "GTRA 1 2 3 4 5 5 5 0 5 5 5 0") && Passed;

  // Defect GEO-C-26: A very large angle never leaves the normalization loop since the subtraction of 180 changes nothing
  MDShapeGTRA Hang("Hang");
  Passed = ExpectedToCrash(EvaluateFalse("Set()", "Theta = 1e300", "A very large angle is rejected and does not hang the program", Hang.Set(1, 1e300, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the normalization of the angles
bool UTDShapeGTRA::TestAngles()
{
  bool Passed = true;

  MDShapeGTRA S("Gtra");

  S.Set(1, 190, 390, 0, 1, 1, 1, 370, 1, 1, 1, 370);
  Passed = EvaluateTrue("Set()", "above the ranges", "The angles are moved into the ranges", S.GetGeomega() == "GTRA 1 10 30 0 1 1 1 10 1 1 1 10") && Passed;

  S.Set(1, -10, -30, 0, 1, 1, 1, -370, 1, 1, 1, -370);
  Passed = EvaluateTrue("Set()", "below the ranges", "The angles are moved into the ranges", S.GetGeomega() == "GTRA 1 170 330 0 1 1 1 -10 1 1 1 -10") && Passed;

  S.Set(1, 180, 360, 0, 1, 1, 1, 360, 1, 1, 1, 360);
  Passed = EvaluateTrue("Set()", "the limits", "The upper limits are the same as 0", S.GetGeomega() == "GTRA 1 0 0 0 1 1 1 0 1 1 1 0") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the planarity check of the side surfaces
bool UTDShapeGTRA::TestPlanarity()
{
  bool Passed = true;

  MDShapeGTRA S("Gtra");

  Passed = EvaluateTrue("Set()", "planar", "A shape with planar sides is accepted", Quiet([&]() { return S.Set(3, 20, 30, 0, 2, 1, 1.5, 10, 4, 2, 3, 10); })) && Passed;
  Passed = EvaluateFalse("Set()", "not planar", "A shape with twisted sides is rejected", Quiet([&]() { return S.Set(3, 0, 0, 0, 2, 1, 1.5, 0, 4, 2, 2, 0); })) && Passed;

  // The limit of 1e-6 rad: Normals differ by 0.167 rad per unit deviation of Tl2 (independent replication of the angle calculation in a script)
  Passed = EvaluateTrue("Set()", "Tl2 off by 5e-6", "A deviation just inside the limit is accepted", Quiet([&]() { return S.Set(3, 0, 0, 0, 2, 1, 1.5, 0, 4, 2, 3 + 5e-6, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "Tl2 off by 7e-6", "A deviation just outside the limit is rejected", Quiet([&]() { return S.Set(3, 0, 0, 0, 2, 1, 1.5, 0, 4, 2, 3 + 7e-6, 0); })) && Passed;

  Passed = EvaluateFalse("Set()", "alpha 10 and 20", "A shape with different alpha is rejected", Quiet([&]() { return S.Set(3, 0, 0, 0, 2, 1, 1, 10, 2, 1, 1, 20); })) && Passed;

  // Defect GEO-C-4: The check uses tan(angle*c_Deg) and not tan(angle*c_Rad), thus angles with a difference of pi*c_Rad have the same wrong tangent
  Passed = ExpectedToFail(EvaluateFalse("Set()", "alpha 10 and 10 + 0.0548", "A shape with different alpha is rejected", Quiet([&]() { return S.Set(3, 0, 0, 0, 2, 1, 1, 10, 2, 1, 1, 10 + c_Pi*c_Rad); }))) && Passed;

  // Defect GEO-C-4: Identical sides in the sense of the true tangent, tan(190 deg) = tan(10 deg), are planar, but rejected
  Passed = ExpectedToFail(EvaluateTrue("Set()", "alpha 10 and 190", "A shape with planar sides is accepted", Quiet([&]() { return S.Set(3, 0, 0, 0, 2, 1, 1, 10, 2, 1, 1, 190); }))) && Passed;

  // Defect GEO-C-27: A theta of 90 deg has an infinite tangent and thus no valid shape, but it is accepted
  Passed = ExpectedToFail(EvaluateFalse("Set()", "Theta = 90", "A shape with an infinite shear is rejected", Quiet([&]() { return S.Set(3, 90, 0, 0, 2, 1, 1, 0, 2, 1, 1, 0); }))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the volume and the size
bool UTDShapeGTRA::TestVolumeSize()
{
  bool Passed = true;

  // Without twist the same formulas as for TRAP
  MDShapeGTRA Box("Box");
  Box.Set(3, 0, 0, 0, 2, 1, 1, 0, 2, 1, 1, 0);
  Passed = EvaluateNear("GetVolume()", "box", "The volume of equal lengths is the one of a box", Box.GetVolume(), 2*4*6.0, 1e-12) && Passed;

  MDShapeGTRA Shear("Shear");
  Shear.Set(3, 20, 30, 0, 2, 1, 1, 10, 2, 1, 1, 10);
  Passed = EvaluateNear("GetVolume()", "sheared box", "A shear does not change the volume", Shear.GetVolume(), 2*4*6.0, 1e-12) && Passed;

  MDShapeGTRA Frustum("Frustum");
  Frustum.Set(3, 0, 0, 0, 2, 2, 2, 0, 1, 1, 1, 0);
  Passed = EvaluateNear("GetVolume()", "frustum", "The volume of a truncated pyramid", Frustum.GetVolume(), 6.0/3*(16 + 4 + 8), 1e-12) && Passed;

  // Defect GEO-C-13: The twist is ignored in the volume - for a square of half side a: V = 2 Dz 4 a^2 (2 + cos(twist))/3 from the integral of the area with linearly connected corners
  MDShapeGTRA Twisted("Twisted");
  Twisted.Set(3, 0, 0, 90, 2, 2, 2, 0, 2, 2, 2, 0);
  Passed = ExpectedToFail(EvaluateNear("GetVolume()", "twist 90", "The volume agrees with the integral", Twisted.GetVolume(), 6.0*16*2.0/3, 1e-10)) && Passed;

  // Defect GEO-C-15: The size is not implemented and returns zero with an error message
  Passed = ExpectedToFail(EvaluateTrue("GetSize()", "box", "The size is the one of the surrounding box", Quiet([&]() { return (Box.GetSize() - MVector(1, 2, 3)).Mag() < 1e-12; }))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the inside check
bool UTDShapeGTRA::TestIsInside()
{
  bool Passed = true;

  const double Just = 1e-9;

  // Twist 90, half side 2: the corners are on the axes at the faces, at z = 0 the corners are at (sqrt(2), sqrt(2))
  MDShapeGTRA Shape("Gtra");
  Shape.Set(3, 0, 0, 90, 2, 2, 2, 0, 2, 2, 2, 0);
  Shape.Validate();

  Passed = EvaluateTrue("IsInside()", "center", "The center is inside", Shape.IsInside(MVector(0, 0, 0))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just inside z", "Just inside the top is inside", Shape.IsInside(MVector(0, 0, 3 - Just))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just outside z", "Just outside the top is outside", Shape.IsInside(MVector(0, 0, 3 + Just))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just inside the middle square", "Just inside the side at z = 0 is inside", Shape.IsInside(MVector(sqrt(2.0) - 1e-6, 0, 0))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just outside the middle square", "Just outside the side at z = 0 is outside", Shape.IsInside(MVector(sqrt(2.0) + 1e-6, 0, 0))) && Passed;
  Passed = EvaluateTrue("IsInside()", "corner at the top", "A point at the corner of the top face is inside", Shape.IsInside(MVector(0, 2.7, 2.9))) && Passed;
  Passed = EvaluateFalse("IsInside()", "corner of the original square", "The original corner at the top face is outside", Shape.IsInside(MVector(1.9, 1.9, 2.9))) && Passed;

  Passed = CheckToleranceAtSurface("top", Shape, MVector(0, 0, 3), MVector(0, 0, 1)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the distance functions with fixed rays
bool UTDShapeGTRA::TestDistances()
{
  bool Passed = true;

  MDShapeGTRA Shape("Gtra");
  Shape.Set(3, 0, 0, 90, 2, 2, 2, 0, 2, 2, 2, 0);
  Shape.Validate();

  Passed = EvaluateNear("DistanceInsideOut()", "-z", "The distance to the bottom", Shape.DistanceInsideOut(MVector(0, 0, 1), MVector(0, 0, -1)), 4.0, 1e-9) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "+z", "The distance to the top", Shape.DistanceInsideOut(MVector(0, 0, 1), MVector(0, 0, 1)), 2.0, 1e-9) && Passed;
  Passed = EvaluateNear("DistanceOutsideIn()", "-z", "The distance to the top from above", Shape.DistanceOutsideIn(MVector(0, 0, 7), MVector(0, 0, -1)), 4.0, 1e-9) && Passed;
  Passed = EvaluateTrue("DistanceOutsideIn()", "miss", "A ray away from the shape is no hit", Shape.DistanceOutsideIn(MVector(0, 0, 7), MVector(0, 0, 1)) > 1e20) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the unique position
bool UTDShapeGTRA::TestUniquePosition()
{
  bool Passed = true;

  MDShapeGTRA Shape("Gtra");
  Shape.Set(3, 20, 30, 0, 2, 1, 1.5, 10, 4, 2, 3, 10);
  Shape.Validate();
  Passed = EvaluateVectorNear("GetUniquePosition()", "sheared", "The position is the center", Shape.GetUniquePosition(), MVector(0, 0, 0), 1e-15) && Passed;
  Passed = EvaluateTrue("GetUniquePosition()", "sheared", "The position is inside", Shape.IsInside(Shape.GetUniquePosition())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the geometry against the analytic truth
bool UTDShapeGTRA::TestAgainstTruth()
{
  bool Passed = true;

  // Without twist:
  MDShapeGTRA Shape("Gtra");
  Shape.Set(3, 20, 30, 0, 2, 1, 1.5, 10, 4, 2, 3, 10);
  Shape.Validate();
  MTruth Truth = GetTrapTruth(3, 20, 30, 2, 1, 1.5, 10, 4, 2, 3, 10);
  const MVector Min(-7, -6, -3.5);
  const MVector Max(7, 6, 3.5);
  Passed = CheckTruthVolume("sheared", Truth, Min, Max, Shape.GetVolume()) && Passed;
  Passed = CheckAgainstTruth("sheared", Shape, Truth, Min, Max) && Passed;

  // With twist: The analytic volume is used - see the defect of the volume
  MDShapeGTRA Twisted("Twisted");
  Twisted.Set(3, 0, 0, 60, 2, 2, 2, 0, 2, 2, 2, 0);
  Twisted.Validate();
  MTruth TruthTwisted = GetTwistedTruth(3, 60, 2);
  const MVector MinTwisted(-3, -3, -3.5);
  const MVector MaxTwisted(3, 3, 3.5);
  Passed = CheckTruthVolume("twist 60", TruthTwisted, MinTwisted, MaxTwisted, 6.0*16*(2 + cos(60*c_Rad))/3) && Passed;
  Passed = CheckInsideAgainstTruth("twist 60", Twisted, TruthTwisted, MinTwisted, MaxTwisted) && Passed;
  Passed = CheckRandomPositions("twist 60", Twisted, TruthTwisted, MinTwisted, MaxTwisted) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the string representations
bool UTDShapeGTRA::TestStrings()
{
  bool Passed = true;

  MDShapeGTRA Shape("Gtra");
  Shape.Set(3, 20, 30, 40, 2, 1, 1.5, 10, 4, 2, 3, 10);
  Passed = EvaluateTrue("ToString()", "shape", "The string contains type and parameters", Shape.ToString().Contains("GTRA (3, 20, 30, 40, 2, 1, 1.5, 10, 4, 2, 3, 10)")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "shape", "The text is the keyword with the parameters", Shape.GetGeomega() == "GTRA 3 20 30 40 2 1 1.5 10 4 2 3 10") && Passed;

  MDShapeGTRA Read("Read");
  Passed = EvaluateTrue("Parse()", "round trip", "The text can be parsed", ParseQuiet(Read, MString("V.Shape ") + Shape.GetGeomega())) && Passed;
  Passed = EvaluateTrue("Parse()", "round trip", "All parameters survive", Read.GetGeomega() == Shape.GetGeomega()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the scaling
bool UTDShapeGTRA::TestScale()
{
  bool Passed = true;

  MDShapeGTRA All("All");
  All.Set(3, 0, 0, 60, 2, 2, 2, 0, 2, 2, 2, 0);
  All.Validate();
  double Volume = All.GetVolume();
  Passed = CheckScaleState("XYZ", All, 2.0, "XYZ") && Passed;
  Passed = EvaluateTrue("Scale()", "XYZ", "All lengths are scaled and the angles are not", All.GetGeomega() == "GTRA 6 0 0 60 4 4 4 0 4 4 4 0") && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "The volume is scaled by the third power", All.GetVolume(), 8*Volume, 1e-9) && Passed;
  Passed = CheckInsideAgainstTruth("XYZ", All, GetTwistedTruth(6, 60, 4), MVector(-6, -6, -7), MVector(6, 6, 7)) && Passed;

  MDShapeGTRA X("X");
  X.Set(3, 0, 0, 0, 2, 1, 1.5, 0, 4, 2, 3, 0);
  X.Validate();
  Passed = EvaluateTrue("Scale()", "X", "The axis X is accepted", X.Scale(2.0, "X")) && Passed;
  Passed = EvaluateTrue("Scale()", "X", "The lengths in x are scaled", X.GetGeomega() == "GTRA 3 0 0 0 2 2 3 0 4 4 6 0") && Passed;

  MDShapeGTRA Y("Y");
  Y.Set(3, 0, 0, 0, 2, 1, 1.5, 0, 4, 2, 3, 0);
  Y.Validate();
  Passed = EvaluateTrue("Scale()", "Y", "The axis Y is accepted", Y.Scale(2.0, "Y")) && Passed;
  Passed = EvaluateTrue("Scale()", "Y", "The lengths in y are scaled", Y.GetGeomega() == "GTRA 3 0 0 0 4 1 1.5 0 8 2 3 0") && Passed;

  MDShapeGTRA Z("Z");
  Z.Set(3, 0, 0, 60, 2, 2, 2, 0, 2, 2, 2, 0);
  Z.Validate();
  Passed = EvaluateTrue("Scale()", "Z", "The axis Z is accepted", Z.Scale(2.0, "Z")) && Passed;
  Passed = Evaluate("IsInside()", "Z, twisted", "A stretch along the axis of the twist is the stretched analytic truth", GetNumberOfInsideDifferences(Z, GetTwistedTruth(6, 60, 2), MVector(-3, -3, -7), MVector(3, 3, 7)), 0U) && Passed;

  // Defect GEO-C-14: Scaling only x of a twisted shape scales the lengths before the rotation, the shape is not the stretched one
  MDShapeGTRA TwistX("TwistX");
  TwistX.Set(3, 0, 0, 60, 2, 2, 2, 0, 2, 2, 2, 0);
  TwistX.Validate();
  TwistX.Scale(2.0, "X");
  MTruth Original = GetTwistedTruth(3, 60, 2);
  MTruth Stretched = [=](double x, double y, double z) { return Original(x/2, y, z); };
  Passed = ExpectedToFail(Evaluate("Scale()", "X, twisted", "The shape is the stretched analytic truth", GetNumberOfInsideDifferences(TwistX, Stretched, MVector(-6, -3, -3.5), MVector(6, 3, 3.5)), 0U)) && Passed;

  // Defect GEO-C-5: A negative factor is accepted while Set() rejects negative lengths
  MDShapeGTRA Negative("Negative");
  Negative.Set(3, 0, 0, 0, 2, 1, 1, 0, 2, 1, 1, 0);
  Negative.Validate();
  Passed = ExpectedToFail(EvaluateFalse("Scale()", "factor -2", "A negative factor is rejected", Negative.Scale(-2.0, "XYZ"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parsing the keywords
bool UTDShapeGTRA::TestParse()
{
  bool Passed = true;

  MDShapeGTRA Parameters("Gtra");
  Passed = EvaluateTrue("Parse()", "Parameters", "The keyword Parameters is accepted", ParseQuiet(Parameters, "Gtra.Parameters 3 20 30 40 2 1 1.5 10 4 2 3 10")) && Passed;
  Passed = EvaluateTrue("Parse()", "Parameters", "The parameters are read in the right order", Parameters.GetGeomega() == "GTRA 3 20 30 40 2 1 1.5 10 4 2 3 10") && Passed;

  MDShapeGTRA Keyword("Keyword");
  Passed = EvaluateTrue("Parse()", "Shape", "The keyword Shape is accepted", ParseQuiet(Keyword, "V.Shape GTRA 3 20 30 40 2 1 1.5 10 4 2 3 10")) && Passed;
  Passed = EvaluateTrue("Parse()", "Shape", "The parameters start after the type", Keyword.GetGeomega() == "GTRA 3 20 30 40 2 1 1.5 10 4 2 3 10") && Passed;

  MDShapeGTRA Bad("Bad");
  Bad.Set(1, 0, 0, 0, 2, 3, 3, 0, 2, 3, 3, 0);
  Passed = EvaluateFalse("Parse()", "11 parameters", "Eleven parameters are rejected", ParseQuiet(Bad, "Bad.Parameters 3 20 30 40 2 1 1.5 10 4 2 3")) && Passed;
  Passed = EvaluateFalse("Parse()", "13 parameters", "Thirteen parameters are rejected", ParseQuiet(Bad, "Bad.Parameters 3 20 30 40 2 1 1.5 10 4 2 3 10 11")) && Passed;
  Passed = EvaluateFalse("Parse()", "wrong keyword", "A wrong keyword is rejected", ParseQuiet(Bad, "Bad.Param 3 20 30 40 2 1 1.5 10 4 2 3 10")) && Passed;
  Passed = EvaluateFalse("Parse()", "invalid", "An invalid value is rejected", ParseQuiet(Bad, "Bad.Parameters 0 20 30 40 2 1 1.5 10 4 2 3 10")) && Passed;
  Passed = EvaluateTrue("Parse()", "malformed", "The rejected input keeps the values", Bad.GetGeomega() == "GTRA 1 0 0 0 2 3 3 0 2 3 3 0") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test configuring a validated shape again
bool UTDShapeGTRA::TestReconfiguration()
{
  MDShapeGTRA Shape("S");
  return CheckReconfiguration("reconfiguration", Shape, MVector(0.0, 0.0, 1.0), [&Shape]() { return Shape.Set(1.0, 0.0, 0.0, 10.0, 2.0, 2.0, 2.0, 0.0, 2.0, 2.0, 2.0, 0.0); }, 1.0, [&Shape]() { return Shape.Set(2.0, 0.0, 0.0, 10.0, 2.0, 2.0, 2.0, 0.0, 2.0, 2.0, 2.0, 0.0); }, 2.0, "S.Parameters 3 0 0 10 2 2 2 0 2 2 2 0", 3.0, [&Shape]() { return Shape.Set(-1.0, 0.0, 0.0, 10.0, 2.0, 2.0, 2.0, 0.0, 2.0, 2.0, 2.0, 0.0); });
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  gErrorIgnoreLevel = kWarning;

  UTDShapeGTRA Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDShapeGTRA.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
