/*
 * UTDShapeCONE.cxx
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
#include "MDShapeCONE.h"
#include "UTDShapeShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the cone shape MDShapeCONE
class UTDShapeCONE : public UTDShapeBase
{
public:
  //! Default constructor
  UTDShapeCONE() : UTDShapeBase("UTDShapeCONE") {}
  //! Default destructor
  virtual ~UTDShapeCONE() {}

  //! Run all tests
  virtual bool Run();

private:

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
bool UTDShapeCONE::Run()
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


//! Test the default state
bool UTDShapeCONE::TestDefault()
{
  bool Passed = true;

  MDShapeCONE Shape("Shape");
  Passed = EvaluateTrue("GetName()", "constructor", "The name of the constructor is stored", Shape.GetName() == "Shape") && Passed;
  Passed = EvaluateTrue("GetType()", "constructor", "The type is CONE", Shape.GetType() == "CONE") && Passed;
  Passed = EvaluateNear("GetVolume()", "default", "A new shape has no volume", Shape.GetVolume(), 0.0, 1e-15) && Passed;

  // Defect GEO-C-7: A shape without parameters passes the validation and becomes a zero-size shape
  MDShapeCONE NoParameters("NoParameters");
  Passed = ExpectedToFail(EvaluateFalse("Validate()", "no parameters", "A shape without parameters is rejected", Quiet([&]() { return NoParameters.Validate(); }))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the limits of the parameters
bool UTDShapeCONE::TestLimits()
{
  bool Passed = true;

  const double Just = 1e-9;
  MDShapeCONE S("Shape");

  Passed = EvaluateTrue("Set()", "H just above 0", "A small half height is accepted", Quiet([&]() { return S.Set(Just, 0, 1, 0, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "H = 0", "A zero half height is rejected", Quiet([&]() { return S.Set(0, 0, 1, 0, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "H just below 0", "A negative half height is rejected", Quiet([&]() { return S.Set(-Just, 0, 1, 0, 1); })) && Passed;

  Passed = EvaluateTrue("Set()", "RminBottom = 0", "RminBottom = 0 is accepted", Quiet([&]() { return S.Set(1, 0, 1, 0, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "RminBottom just below 0", "A negative RminBottom is rejected", Quiet([&]() { return S.Set(1, -Just, 1, 0, 1); })) && Passed;
  Passed = EvaluateTrue("Set()", "RmaxBottom just above 0", "A small RmaxBottom is accepted", Quiet([&]() { return S.Set(1, 0, Just, 0, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "RmaxBottom = 0", "RmaxBottom = 0 is rejected", Quiet([&]() { return S.Set(1, 0, 0, 0, 1); })) && Passed;
  Passed = EvaluateTrue("Set()", "RminBottom just below RmaxBottom", "RminBottom below RmaxBottom is accepted", Quiet([&]() { return S.Set(1, 1 - Just, 1, 0, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "RminBottom = RmaxBottom", "RminBottom = RmaxBottom is rejected", Quiet([&]() { return S.Set(1, 1, 1, 0, 1); })) && Passed;

  Passed = EvaluateTrue("Set()", "RminTop = 0", "RminTop = 0 is accepted", Quiet([&]() { return S.Set(1, 0, 1, 0, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "RminTop just below 0", "A negative RminTop is rejected", Quiet([&]() { return S.Set(1, 0, 1, -Just, 1); })) && Passed;
  Passed = EvaluateTrue("Set()", "RmaxTop just above 0", "A small RmaxTop is accepted", Quiet([&]() { return S.Set(1, 0, 1, 0, Just); })) && Passed;
  Passed = EvaluateFalse("Set()", "RmaxTop just below 0", "A negative RmaxTop is rejected", Quiet([&]() { return S.Set(1, 0, 1, 0, -Just); })) && Passed;
  Passed = EvaluateTrue("Set()", "RminTop just below RmaxTop", "RminTop below RmaxTop is accepted", Quiet([&]() { return S.Set(1, 0, 1, 1 - Just, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "RminTop = RmaxTop", "RminTop = RmaxTop is rejected", Quiet([&]() { return S.Set(1, 0, 1, 1, 1); })) && Passed;

  // A rejected set does not change the shape:
  S.Set(1, 2, 3, 4, 5);
  Quiet([&]() { return S.Set(-1, 7, 6, 9, 8); });
  Passed = EvaluateNear("GetHalfHeight()", "after reject", "The old half height is kept", S.GetHalfHeight(), 1.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetRminBottom()", "after reject", "The old RminBottom is kept", S.GetRminBottom(), 2.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetRmaxBottom()", "after reject", "The old RmaxBottom is kept", S.GetRmaxBottom(), 3.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetRminTop()", "after reject", "The old RminTop is kept", S.GetRminTop(), 4.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetRmaxTop()", "after reject", "The old RmaxTop is kept", S.GetRmaxTop(), 5.0, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the volume and the size
bool UTDShapeCONE::TestVolumeSize()
{
  bool Passed = true;

  // Frustum: pi h / 3 (R1^2 + R1 R2 + R2^2) with the full height h = 2H, the hole is subtracted
  MDShapeCONE Shape("Shape");
  Shape.Set(2, 1, 3, 0.5, 2);
  double Expected = c_Pi*4/3*((9 + 6 + 4) - (1 + 0.5 + 0.25));
  Passed = EvaluateNear("GetVolume()", "hollow frustum", "The volume is the difference of two frustums", Shape.GetVolume(), Expected, 1e-10) && Passed;

  // Limit of a cylinder: pi R^2 2H
  MDShapeCONE Cylinder("Cylinder");
  Cylinder.Set(2, 0, 3, 0, 3);
  Passed = EvaluateNear("GetVolume()", "cylinder", "The volume of equal radii is the one of a cylinder", Cylinder.GetVolume(), c_Pi*9*4, 1e-10) && Passed;

  // The radius of the surrounding box is the larger one:
  MDShapeCONE TopLarger("TopLarger");
  TopLarger.Set(2, 0, 1, 0, 3);
  Passed = EvaluateVectorNear("GetSize()", "top larger", "The size is given by the top radius and the half height", TopLarger.GetSize(), MVector(3, 3, 2), 1e-15) && Passed;

  // Defect GEO-C-2: The size uses the top radius even if the bottom radius is larger, thus the surrounding box is too small
  MDShapeCONE BottomLarger("BottomLarger");
  BottomLarger.Set(2, 0, 3, 0, 1);
  Passed = ExpectedToFail(EvaluateVectorNear("GetSize()", "bottom larger", "The size is given by the bottom radius and the half height", BottomLarger.GetSize(), MVector(3, 3, 2), 1e-15)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the inside check
bool UTDShapeCONE::TestIsInside()
{
  bool Passed = true;

  const double Just = 1e-9;

  // Radius at z = 0: Rmin = 1.5 and Rmax = 3, at the bottom: 1 and 2, at the top: 2 and 4
  MDShapeCONE Shape("Shape");
  Shape.Set(2, 1, 2, 2, 4);
  Shape.Validate();

  Passed = EvaluateTrue("IsInside()", "z = 0, middle", "A point between the radii is inside", Shape.IsInside(MVector(2, 0.2, 0))) && Passed;
  Passed = EvaluateFalse("IsInside()", "center", "The hole is outside", Shape.IsInside(MVector(0, 0, 0))) && Passed;
  Passed = EvaluateTrue("IsInside()", "top is the wide side", "A large radius at the top is inside", Shape.IsInside(MVector(0, 3.5, 1.9))) && Passed;
  Passed = EvaluateFalse("IsInside()", "bottom is the narrow side", "The same radius at the bottom is outside", Shape.IsInside(MVector(0, 3.5, -1.9))) && Passed;
  Passed = EvaluateTrue("IsInside()", "bottom small radius", "A small radius at the bottom is inside", Shape.IsInside(MVector(1.2, 0, -1.9))) && Passed;
  Passed = EvaluateFalse("IsInside()", "top small radius", "The same radius at the top is in the hole", Shape.IsInside(MVector(1.2, 0, 1.9))) && Passed;

  Passed = EvaluateTrue("IsInside()", "just inside Rmax at z = 0", "Just inside the outer surface is inside", Shape.IsInside(MVector(3 - Just, 0, 0))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just outside Rmax at z = 0", "Just outside the outer surface is outside", Shape.IsInside(MVector(3 + Just, 0, 0))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just inside Rmin at z = 0", "Just inside the inner surface is inside", Shape.IsInside(MVector(0, 1.5 + Just, 0))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just outside Rmin at z = 0", "Just outside the inner surface is outside", Shape.IsInside(MVector(0, 1.5 - Just, 0))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just below the top", "Just below the top is inside", Shape.IsInside(MVector(3, 0, 2 - Just))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just above the top", "Just above the top is outside", Shape.IsInside(MVector(3, 0, 2 + Just))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just above the bottom", "Just above the bottom is inside", Shape.IsInside(MVector(1.5, 0, -2 + Just))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just below the bottom", "Just below the bottom is outside", Shape.IsInside(MVector(1.5, 0, -2 - Just))) && Passed;

  Passed = CheckToleranceAtSurface("top", Shape, MVector(3, 0, 2), MVector(0, 0, 1)) && Passed;
  Passed = CheckToleranceAtSurface("bottom", Shape, MVector(1.5, 0, -2), MVector(0, 0, -1)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the distance functions with fixed rays
bool UTDShapeCONE::TestDistances()
{
  bool Passed = true;

  MDShapeCONE Shape("Shape");
  Shape.Set(2, 1, 2, 2, 4);
  Shape.Validate();

  // Horizontal rays at z = 0 with the radii 1.5 and 3:
  Passed = EvaluateNear("DistanceInsideOut()", "outward", "The distance to the outer surface", Shape.DistanceInsideOut(MVector(2, 0, 0), MVector(1, 0, 0)), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "inward", "The distance to the inner surface", Shape.DistanceInsideOut(MVector(2, 0, 0), MVector(-1, 0, 0)), 0.5, 1e-12) && Passed;
  // Vertical ray at r = 2.5, inside up to the top:
  Passed = EvaluateNear("DistanceInsideOut()", "up", "The distance to the top", Shape.DistanceInsideOut(MVector(2.5, 0, 0), MVector(0, 0, 1)), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceOutsideIn()", "from outside", "The distance to the outer surface from outside", Shape.DistanceOutsideIn(MVector(5, 0, 0), MVector(-1, 0, 0)), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceOutsideIn()", "from the hole", "The distance to the inner surface from the hole", Shape.DistanceOutsideIn(MVector(0, 0, 0), MVector(0, 1, 0)), 1.5, 1e-12) && Passed;
  Passed = EvaluateTrue("DistanceOutsideIn()", "miss", "A ray away from the shape is no hit", Shape.DistanceOutsideIn(MVector(5, 0, 0), MVector(1, 0, 0)) > 1e20) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the unique position
bool UTDShapeCONE::TestUniquePosition()
{
  bool Passed = true;

  // The mean of the radii at z = 0 is (1.5 + 3)/2
  MDShapeCONE Shape("Shape");
  Shape.Set(2, 1, 2, 2, 4);
  Shape.Validate();
  Passed = EvaluateVectorNear("GetUniquePosition()", "hollow", "The position is between the radii at z = 0", Shape.GetUniquePosition(), MVector(2.25, 0, 0), 1e-12) && Passed;
  Passed = EvaluateTrue("GetUniquePosition()", "hollow", "The position is inside", Shape.IsInside(Shape.GetUniquePosition())) && Passed;

  // A cone with a very small top radius:
  MDShapeCONE Thin("Thin");
  Thin.Set(2, 0, 3, 0, 1e-6);
  Thin.Validate();
  Passed = EvaluateTrue("GetUniquePosition()", "cone with a tip", "The position is inside", Thin.IsInside(Thin.GetUniquePosition())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the geometry against the analytic truth
bool UTDShapeCONE::TestAgainstTruth()
{
  bool Passed = true;

  const MVector Min(-4.5, -4.5, -2.5);
  const MVector Max(4.5, 4.5, 2.5);

  MDShapeCONE Shape("Shape");
  Shape.Set(2, 1, 2, 2, 4);
  Shape.Validate();
  MTruth Truth = GetConeTruth(2, 1, 2, 2, 4);
  Passed = CheckTruthVolume("widening", Truth, Min, Max, Shape.GetVolume()) && Passed;
  Passed = CheckAgainstTruth("widening", Shape, Truth, Min, Max) && Passed;

  MDShapeCONE Narrow("Narrow");
  Narrow.Set(2, 0, 4, 0.5, 1);
  Narrow.Validate();
  MTruth TruthNarrow = GetConeTruth(2, 0, 4, 0.5, 1);
  Passed = CheckTruthVolume("narrowing", TruthNarrow, Min, Max, Narrow.GetVolume()) && Passed;
  Passed = CheckAgainstTruth("narrowing", Narrow, TruthNarrow, Min, Max) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the string representations
bool UTDShapeCONE::TestStrings()
{
  bool Passed = true;

  MDShapeCONE Shape("Shape");
  Shape.Set(2, 1, 3, 0.5, 4);
  Passed = EvaluateTrue("ToString()", "shape", "The string contains type and parameters", Shape.ToString().Contains("CONE (2,1,3,0.5,4)")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "shape", "The text is the keyword with the parameters", Shape.GetGeomega() == "CONE 2 1 3 0.5 4") && Passed;

  MDShapeCONE Read("Read");
  Passed = EvaluateTrue("Parse()", "round trip", "The text can be parsed", ParseQuiet(Read, MString("V.Shape ") + Shape.GetGeomega())) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "The half height survives", Read.GetHalfHeight(), 2.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "RminBottom survives", Read.GetRminBottom(), 1.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "RmaxBottom survives", Read.GetRmaxBottom(), 3.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "RminTop survives", Read.GetRminTop(), 0.5, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "RmaxTop survives", Read.GetRmaxTop(), 4.0, 1e-4) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the scaling
bool UTDShapeCONE::TestScale()
{
  bool Passed = true;

  MDShapeCONE All("All");
  All.Set(2, 1, 3, 0.5, 4);
  All.Validate();
  double Volume = All.GetVolume();
  Passed = CheckScaleState("XYZ", All, 2.0, "XYZ") && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "The half height is scaled", All.GetHalfHeight(), 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "RminBottom is scaled", All.GetRminBottom(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "RmaxBottom is scaled", All.GetRmaxBottom(), 6.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "RminTop is scaled", All.GetRminTop(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "RmaxTop is scaled", All.GetRmaxTop(), 8.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "The volume is scaled by the third power", All.GetVolume(), 8*Volume, 1e-9) && Passed;
  Passed = CheckInsideAgainstTruth("XYZ", All, GetConeTruth(4, 2, 6, 1, 8), MVector(-9, -9, -5), MVector(9, 9, 5)) && Passed;

  MDShapeCONE XY("XY");
  XY.Set(2, 1, 3, 0.5, 4);
  XY.Validate();
  Passed = EvaluateTrue("Scale()", "XY", "The axes XY are accepted", XY.Scale(2.0, "XY")) && Passed;
  Passed = EvaluateNear("Scale()", "XY", "The height is not scaled", XY.GetHalfHeight(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XY", "The radii are scaled", XY.GetRmaxTop(), 8.0, 1e-12) && Passed;

  MDShapeCONE Z("Z");
  Z.Set(2, 1, 3, 0.5, 4);
  Z.Validate();
  Passed = EvaluateTrue("Scale()", "Z", "The axis Z is accepted", Z.Scale(2.0, "Z")) && Passed;
  Passed = EvaluateNear("Scale()", "Z", "The height is scaled", Z.GetHalfHeight(), 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "Z", "The radii are not scaled", Z.GetRmaxTop(), 4.0, 1e-12) && Passed;

  MDShapeCONE Y("Y");
  Y.Set(2, 1, 3, 0.5, 4);
  Y.Validate();
  Passed = EvaluateFalse("Scale()", "Y", "The radii cannot be scaled along Y alone", Quiet([&]() { return Y.Scale(2.0, "Y"); })) && Passed;
  Passed = EvaluateNear("Scale()", "Y", "The rejected axis keeps the radii", Y.GetRmaxTop(), 4.0, 1e-12) && Passed;

  // Defect GEO-C-6: A rejected axis combination leaves the shape marked as scaled
  Passed = ExpectedToFail(EvaluateFalse("IsScaled()", "after a rejected scaling", "The shape is not marked as scaled", Y.IsScaled())) && Passed;

  // Defect GEO-C-5: A negative factor is accepted while Set() rejects negative radii
  MDShapeCONE Negative("Negative");
  Negative.Set(2, 1, 3, 0.5, 4);
  Negative.Validate();
  Passed = ExpectedToFail(EvaluateFalse("Scale()", "factor -2", "A negative factor is rejected", Negative.Scale(-2.0, "XYZ"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parsing the keywords
bool UTDShapeCONE::TestParse()
{
  bool Passed = true;

  MDShapeCONE Parameters("Shape");
  Passed = EvaluateTrue("Parse()", "Parameters", "The keyword Parameters is accepted", ParseQuiet(Parameters, "Shape.Parameters 2.5 1.5 3.5 0.5 4.5")) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "The half height is read", Parameters.GetHalfHeight(), 2.5, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "RminBottom is read", Parameters.GetRminBottom(), 1.5, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "RmaxBottom is read", Parameters.GetRmaxBottom(), 3.5, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "RminTop is read", Parameters.GetRminTop(), 0.5, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "RmaxTop is read", Parameters.GetRmaxTop(), 4.5, 1e-15) && Passed;

  MDShapeCONE Keyword("Keyword");
  Passed = EvaluateTrue("Parse()", "Shape", "The keyword Shape is accepted", ParseQuiet(Keyword, "V.Shape CONE 2.5 1.5 3.5 0.5 4.5")) && Passed;
  Passed = EvaluateNear("Parse()", "Shape", "The parameters start after the type", Keyword.GetRmaxTop(), 4.5, 1e-15) && Passed;

  MDShapeCONE Bad("Bad");
  Bad.Set(1, 2, 3, 4, 5);
  Passed = EvaluateFalse("Parse()", "too few", "Too few parameters are rejected", ParseQuiet(Bad, "Bad.Parameters 5 6 7 8")) && Passed;
  Passed = EvaluateFalse("Parse()", "too many", "Too many parameters are rejected", ParseQuiet(Bad, "Bad.Parameters 5 6 7 8 9 10")) && Passed;
  Passed = EvaluateFalse("Parse()", "wrong keyword", "A wrong keyword is rejected", ParseQuiet(Bad, "Bad.Param 2.5 1.5 3.5 0.5 4.5")) && Passed;
  Passed = EvaluateFalse("Parse()", "invalid", "An invalid value is rejected", ParseQuiet(Bad, "Bad.Parameters 2.5 3.5 1.5 0.5 4.5")) && Passed;
  Passed = EvaluateNear("Parse()", "malformed", "The rejected input keeps the values", Bad.GetRmaxBottom(), 3.0, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test configuring a validated shape again
bool UTDShapeCONE::TestReconfiguration()
{
  MDShapeCONE Shape("S");
  return CheckReconfiguration("reconfiguration", Shape, MVector(0.0, 0.0, 1.0), [&Shape]() { return Shape.Set(1.0, 0.0, 5.0, 0.0, 5.0); }, 1.0, [&Shape]() { return Shape.Set(2.0, 0.0, 5.0, 0.0, 5.0); }, 2.0, "S.Parameters 3 0 5 0 5", 3.0, [&Shape]() { return Shape.Set(-1.0, 0.0, 5.0, 0.0, 5.0); });
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  gErrorIgnoreLevel = kWarning;

  UTDShapeCONE Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDShapeCONE.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
