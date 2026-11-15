/*
 * UTDShapeTRD2.cxx
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
#include "MDShapeTRD1.h"
#include "MDShapeTRD2.h"
#include "UTDShapeShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the trapezoid shape MDShapeTRD2
class UTDShapeTRD2 : public UTDShapeBase
{
public:
  //! Default constructor
  UTDShapeTRD2() : UTDShapeBase("UTDShapeTRD2") {}
  //! Default destructor
  virtual ~UTDShapeTRD2() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the analytic truth: half lengths in x and y are linear from -Z to +Z
  MTruth GetTruth(double X1, double X2, double Y1, double Y2, double Z);
  //! Return the volume from the integral of 2x(z) 2y(z) dz over the height
  double GetTruthVolume(double X1, double X2, double Y1, double Y2, double Z);

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
bool UTDShapeTRD2::Run()
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


//! Return the analytic truth: half lengths in x and y are linear from -Z to +Z
UTDShapeBase::MTruth UTDShapeTRD2::GetTruth(double X1, double X2, double Y1, double Y2, double Z)
{
  return [=](double x, double y, double z) {
    if (fabs(z) > Z) {
      return false;
    }
    double T = (z + Z)/(2*Z);
    if (fabs(x) > X1 + T*(X2 - X1) || fabs(y) > Y1 + T*(Y2 - Y1)) {
      return false;
    }
    return true;
  };
}


////////////////////////////////////////////////////////////////////////////////


//! Return the volume from the integral of 2x(z) 2y(z) dz over the height
double UTDShapeTRD2::GetTruthVolume(double X1, double X2, double Y1, double Y2, double Z)
{
  return 8*Z*((X1*Y1 + X2*Y2)/3 + (X1*Y2 + X2*Y1)/6);
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default state
bool UTDShapeTRD2::TestDefault()
{
  bool Passed = true;

  MDShapeTRD2 Shape("Trd");
  Passed = EvaluateTrue("GetName()", "constructor", "The name of the constructor is stored", Shape.GetName() == "Trd") && Passed;
  Passed = EvaluateTrue("GetType()", "constructor", "The type is TRD2", Shape.GetType() == "TRD2") && Passed;
  Passed = EvaluateNear("GetVolume()", "default", "A new shape has no volume", Shape.GetVolume(), 0.0, 1e-15) && Passed;

  // Defect GEO-C-7: A shape without parameters passes the validation and becomes a zero-size shape
  MDShapeTRD2 NoParameters("NoParameters");
  Passed = ExpectedToFail(EvaluateFalse("Validate()", "no parameters", "A shape without parameters is rejected", Quiet([&]() { return NoParameters.Validate(); }))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the limits of the parameters
bool UTDShapeTRD2::TestLimits()
{
  bool Passed = true;

  const double Just = 1e-9;
  MDShapeTRD2 S("Trd");

  Passed = EvaluateTrue("Set()", "dx1 = 0", "A zero dx1 is accepted", Quiet([&]() { return S.Set(0, 1, 1, 1, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "dx1 just below 0", "A negative dx1 is rejected", Quiet([&]() { return S.Set(-Just, 1, 1, 1, 1); })) && Passed;
  Passed = EvaluateTrue("Set()", "dx2 = 0", "A zero dx2 is accepted", Quiet([&]() { return S.Set(1, 0, 1, 1, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "dx2 just below 0", "A negative dx2 is rejected", Quiet([&]() { return S.Set(1, -Just, 1, 1, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "dx1 = dx2 = 0", "Both x lengths zero are rejected", Quiet([&]() { return S.Set(0, 0, 1, 1, 1); })) && Passed;
  Passed = EvaluateTrue("Set()", "dy1 = 0", "A zero dy1 is accepted", Quiet([&]() { return S.Set(1, 1, 0, 1, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "dy1 just below 0", "A negative dy1 is rejected", Quiet([&]() { return S.Set(1, 1, -Just, 1, 1); })) && Passed;
  Passed = EvaluateTrue("Set()", "dy2 = 0", "A zero dy2 is accepted", Quiet([&]() { return S.Set(1, 1, 1, 0, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "dy2 just below 0", "A negative dy2 is rejected", Quiet([&]() { return S.Set(1, 1, 1, -Just, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "dy1 = dy2 = 0", "Both y lengths zero are rejected", Quiet([&]() { return S.Set(1, 1, 0, 0, 1); })) && Passed;
  Passed = EvaluateTrue("Set()", "z just above 0", "A small z is accepted", Quiet([&]() { return S.Set(1, 1, 1, 1, Just); })) && Passed;
  Passed = EvaluateFalse("Set()", "z = 0", "A zero z is rejected", Quiet([&]() { return S.Set(1, 1, 1, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "z just below 0", "A negative z is rejected", Quiet([&]() { return S.Set(1, 1, 1, 1, -Just); })) && Passed;

  // A rejected set does not change the shape:
  S.Set(1, 2, 3, 4, 5);
  Quiet([&]() { return S.Set(6, 7, 8, 9, -1); });
  Passed = EvaluateNear("GetDx1()", "after reject", "The old dx1 is kept", S.GetDx1(), 1.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetDx2()", "after reject", "The old dx2 is kept", S.GetDx2(), 2.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetDy1()", "after reject", "The old dy1 is kept", S.GetDy1(), 3.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetDy2()", "after reject", "The old dy2 is kept", S.GetDy2(), 4.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetZ()", "after reject", "The old z is kept", S.GetZ(), 5.0, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the volume and the size
bool UTDShapeTRD2::TestVolumeSize()
{
  bool Passed = true;

  // A box: 2x 2y 2z
  MDShapeTRD2 Box("Box");
  Box.Set(2, 2, 3, 3, 1.5);
  Passed = EvaluateNear("GetVolume()", "box", "The volume of equal lengths is the one of a box", Box.GetVolume(), 4*6*3.0, 1e-12) && Passed;

  // Only x changes: same as the sibling TRD1
  MDShapeTRD2 OnlyX("OnlyX");
  OnlyX.Set(1, 3, 2, 2, 1.5);
  MDShapeTRD1 Sibling("Sibling");
  Sibling.Set(1, 3, 2, 1.5);
  Passed = EvaluateNear("GetVolume()", "only x changes", "The volume agrees with the integral", OnlyX.GetVolume(), GetTruthVolume(1, 3, 2, 2, 1.5), 1e-12) && Passed;
  Passed = EvaluateNear("GetVolume()", "only x changes", "The volume agrees with the sibling TRD1", OnlyX.GetVolume(), Sibling.GetVolume(), 1e-12) && Passed;

  // Only y changes:
  MDShapeTRD2 OnlyY("OnlyY");
  OnlyY.Set(2, 2, 1, 3, 1.5);
  Passed = EvaluateNear("GetVolume()", "only y changes", "The volume agrees with the integral", OnlyY.GetVolume(), GetTruthVolume(2, 2, 1, 3, 1.5), 1e-12) && Passed;

  // Defect GEO-C-1: The volume is the area at half height times the height, which is wrong if both x and y change - a pyramid has base area * height / 3
  MDShapeTRD2 Pyramid("Pyramid");
  Pyramid.Set(2, 0, 2, 0, 1.5);
  Passed = ExpectedToFail(EvaluateNear("GetVolume()", "pyramid", "The volume of a pyramid is base area times height over 3", Pyramid.GetVolume(), 16.0*3.0/3.0, 1e-12)) && Passed;

  // Defect GEO-C-1: Non-symmetric frustum with the integral as truth
  MDShapeTRD2 Frustum("Frustum");
  Frustum.Set(1, 3, 2, 0.5, 1.5);
  Passed = ExpectedToFail(EvaluateNear("GetVolume()", "frustum", "The volume agrees with the integral", Frustum.GetVolume(), GetTruthVolume(1, 3, 2, 0.5, 1.5), 1e-12)) && Passed;

  // The size:
  Passed = EvaluateVectorNear("GetSize()", "x2 and y1 larger", "The surrounding box uses the larger lengths", Frustum.GetSize(), MVector(3, 2, 1.5), 1e-15) && Passed;
  MDShapeTRD2 Other("Other");
  Other.Set(3, 1, 0.5, 2, 1.5);
  Passed = EvaluateVectorNear("GetSize()", "x1 and y2 larger", "The surrounding box uses the larger lengths", Other.GetSize(), MVector(3, 2, 1.5), 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the inside check
bool UTDShapeTRD2::TestIsInside()
{
  bool Passed = true;

  const double Just = 1e-9;

  // Half lengths at z = 0: x = 2 and y = 1.25
  MDShapeTRD2 Shape("Trd");
  Shape.Set(1, 3, 2, 0.5, 1.5);
  Shape.Validate();

  Passed = EvaluateTrue("IsInside()", "center", "The center is inside", Shape.IsInside(MVector(0, 0, 0))) && Passed;
  Passed = EvaluateTrue("IsInside()", "non-symmetric", "A non-symmetric interior point is inside", Shape.IsInside(MVector(-2.5, 0.4, 1))) && Passed;
  Passed = EvaluateFalse("IsInside()", "wide x at the bottom", "The width of the top does not exist at the bottom", Shape.IsInside(MVector(2.5, 0, -1.4))) && Passed;
  Passed = EvaluateTrue("IsInside()", "wide y at the bottom", "The large y of the bottom is inside", Shape.IsInside(MVector(0, 1.8, -1.4))) && Passed;
  Passed = EvaluateFalse("IsInside()", "wide y at the top", "The large y of the bottom does not exist at the top", Shape.IsInside(MVector(0, 1.8, 1.4))) && Passed;

  Passed = EvaluateTrue("IsInside()", "just inside x", "Just inside the surface in x is inside", Shape.IsInside(MVector(2 - Just, 0, 0))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just outside x", "Just outside the surface in x is outside", Shape.IsInside(MVector(2 + Just, 0, 0))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just inside y", "Just inside the surface in y is inside", Shape.IsInside(MVector(0, 1.25 - Just, 0))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just outside y", "Just outside the surface in y is outside", Shape.IsInside(MVector(0, 1.25 + Just, 0))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just inside z", "Just inside z is inside", Shape.IsInside(MVector(0, 0, 1.5 - Just))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just outside z", "Just outside z is outside", Shape.IsInside(MVector(0, 0, 1.5 + Just))) && Passed;

  Passed = CheckToleranceAtSurface("top", Shape, MVector(0, 0, 1.5), MVector(0, 0, 1)) && Passed;
  Passed = CheckToleranceAtSurface("bottom", Shape, MVector(0, 0, -1.5), MVector(0, 0, -1)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the distance functions with fixed rays
bool UTDShapeTRD2::TestDistances()
{
  bool Passed = true;

  MDShapeTRD2 Shape("Trd");
  Shape.Set(1, 3, 2, 0.5, 1.5);
  Shape.Validate();

  Passed = EvaluateNear("DistanceInsideOut()", "+x at z = 0", "The half length at z = 0 is 2", Shape.DistanceInsideOut(MVector(0, 0, 0), MVector(1, 0, 0)), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "+y at z = 0", "The half length at z = 0 is 1.25", Shape.DistanceInsideOut(MVector(0, 0, 0), MVector(0, 1, 0)), 1.25, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "-z", "The distance to the bottom", Shape.DistanceInsideOut(MVector(0, 0, 0.5), MVector(0, 0, -1)), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceOutsideIn()", "-x at z = 0", "The distance to the surface from outside", Shape.DistanceOutsideIn(MVector(5, 0, 0), MVector(-1, 0, 0)), 3.0, 1e-12) && Passed;
  Passed = EvaluateTrue("DistanceOutsideIn()", "miss", "A ray away from the shape is no hit", Shape.DistanceOutsideIn(MVector(5, 0, 0), MVector(1, 0, 0)) > 1e20) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the unique position
bool UTDShapeTRD2::TestUniquePosition()
{
  bool Passed = true;

  MDShapeTRD2 Shape("Trd");
  Shape.Set(1, 3, 2, 0.5, 1.5);
  Shape.Validate();
  Passed = EvaluateVectorNear("GetUniquePosition()", "frustum", "The position is the center", Shape.GetUniquePosition(), MVector(0, 0, 0), 1e-15) && Passed;
  Passed = EvaluateTrue("GetUniquePosition()", "frustum", "The position is inside", Shape.IsInside(Shape.GetUniquePosition())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the geometry against the analytic truth
bool UTDShapeTRD2::TestAgainstTruth()
{
  bool Passed = true;

  const MVector Min(-3.5, -2.5, -2);
  const MVector Max(3.5, 2.5, 2);

  // Attention: The analytic volume is used - see the defect of the volume
  MDShapeTRD2 Shape("Trd");
  Shape.Set(1, 3, 2, 0.5, 1.5);
  Shape.Validate();
  MTruth Truth = GetTruth(1, 3, 2, 0.5, 1.5);
  Passed = CheckTruthVolume("frustum", Truth, Min, Max, GetTruthVolume(1, 3, 2, 0.5, 1.5)) && Passed;
  Passed = CheckAgainstTruth("frustum", Shape, Truth, Min, Max) && Passed;

  MDShapeTRD2 Pyramid("Pyramid");
  Pyramid.Set(3, 0, 2, 0, 1.5);
  Pyramid.Validate();
  MTruth TruthPyramid = GetTruth(3, 0, 2, 0, 1.5);
  Passed = CheckTruthVolume("pyramid", TruthPyramid, Min, Max, GetTruthVolume(3, 0, 2, 0, 1.5)) && Passed;
  Passed = CheckAgainstTruth("pyramid", Pyramid, TruthPyramid, Min, Max) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the string representations
bool UTDShapeTRD2::TestStrings()
{
  bool Passed = true;

  MDShapeTRD2 Shape("Trd");
  Shape.Set(1, 3, 2, 0.5, 1.5);
  Passed = EvaluateTrue("ToString()", "frustum", "The string contains type and parameters", Shape.ToString().Contains("TRD2 (1, 3, 2, 0.5, 1.5)")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "frustum", "The text is the keyword with the parameters", Shape.GetGeomega() == "TRD2 1 3 2 0.5 1.5") && Passed;

  MDShapeTRD2 Read("Read");
  Passed = EvaluateTrue("Parse()", "round trip", "The text can be parsed", ParseQuiet(Read, MString("V.Shape ") + Shape.GetGeomega())) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "dx1 survives", Read.GetDx1(), 1.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "dx2 survives", Read.GetDx2(), 3.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "dy1 survives", Read.GetDy1(), 2.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "dy2 survives", Read.GetDy2(), 0.5, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "z survives", Read.GetZ(), 1.5, 1e-4) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the scaling
bool UTDShapeTRD2::TestScale()
{
  bool Passed = true;

  MDShapeTRD2 All("All");
  All.Set(1, 3, 2, 0.5, 1.5);
  All.Validate();
  double Volume = All.GetVolume();
  Passed = CheckScaleState("XYZ", All, 2.0, "XYZ") && Passed;
  Passed = EvaluateVectorNear("Scale()", "XYZ", "All lengths are scaled", All.GetSize(), MVector(6, 4, 3), 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "The volume is scaled by the third power", All.GetVolume(), 8*Volume, 1e-9) && Passed;
  Passed = CheckInsideAgainstTruth("XYZ", All, GetTruth(2, 6, 4, 1, 3), MVector(-7, -5, -4), MVector(7, 5, 4)) && Passed;

  MDShapeTRD2 X("X");
  X.Set(1, 3, 2, 0.5, 1.5);
  X.Validate();
  Passed = EvaluateTrue("Scale()", "X", "The axis X is accepted", X.Scale(2.0, "X")) && Passed;
  Passed = EvaluateNear("Scale()", "X", "dx1 is scaled", X.GetDx1(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "X", "dx2 is scaled", X.GetDx2(), 6.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "X", "dy1 is not scaled", X.GetDy1(), 2.0, 1e-12) && Passed;

  MDShapeTRD2 Y("Y");
  Y.Set(1, 3, 2, 0.5, 1.5);
  Y.Validate();
  Passed = EvaluateTrue("Scale()", "Y", "The axis Y is accepted", Y.Scale(2.0, "Y")) && Passed;
  Passed = EvaluateNear("Scale()", "Y", "dy1 is scaled", Y.GetDy1(), 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "Y", "dy2 is scaled", Y.GetDy2(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "Y", "dx1 is not scaled", Y.GetDx1(), 1.0, 1e-12) && Passed;

  MDShapeTRD2 Z("Z");
  Z.Set(1, 3, 2, 0.5, 1.5);
  Z.Validate();
  Passed = EvaluateTrue("Scale()", "Z", "The axis Z is accepted", Z.Scale(2.0, "Z")) && Passed;
  Passed = EvaluateNear("Scale()", "Z", "z is scaled", Z.GetZ(), 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "Z", "dx1 is not scaled", Z.GetDx1(), 1.0, 1e-12) && Passed;

  // Defect GEO-C-5: A negative factor is accepted while Set() rejects negative lengths
  MDShapeTRD2 Negative("Negative");
  Negative.Set(1, 3, 2, 0.5, 1.5);
  Negative.Validate();
  Passed = ExpectedToFail(EvaluateFalse("Scale()", "factor -2", "A negative factor is rejected", Negative.Scale(-2.0, "XYZ"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parsing the keywords
bool UTDShapeTRD2::TestParse()
{
  bool Passed = true;

  MDShapeTRD2 Parameters("Trd");
  Passed = EvaluateTrue("Parse()", "Parameters", "The keyword Parameters is accepted", ParseQuiet(Parameters, "Trd.Parameters 1.5 2.5 3.5 4.5 5.5")) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "dx1 is read", Parameters.GetDx1(), 1.5, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "dx2 is read", Parameters.GetDx2(), 2.5, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "dy1 is read", Parameters.GetDy1(), 3.5, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "dy2 is read", Parameters.GetDy2(), 4.5, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "z is read", Parameters.GetZ(), 5.5, 1e-15) && Passed;

  MDShapeTRD2 Keyword("Keyword");
  Passed = EvaluateTrue("Parse()", "Shape", "The keyword Shape is accepted", ParseQuiet(Keyword, "V.Shape TRD2 1.5 2.5 3.5 4.5 5.5")) && Passed;
  Passed = EvaluateNear("Parse()", "Shape", "The parameters start after the type", Keyword.GetZ(), 5.5, 1e-15) && Passed;

  MDShapeTRD2 Bad("Bad");
  Bad.Set(1, 2, 3, 4, 5);
  Passed = EvaluateFalse("Parse()", "4 parameters", "Four parameters are rejected", ParseQuiet(Bad, "Bad.Parameters 5 6 7 8")) && Passed;
  Passed = EvaluateFalse("Parse()", "6 parameters", "Six parameters are rejected", ParseQuiet(Bad, "Bad.Parameters 5 6 7 8 9 10")) && Passed;
  Passed = EvaluateFalse("Parse()", "wrong keyword", "A wrong keyword is rejected", ParseQuiet(Bad, "Bad.Param 5 6 7 8 9")) && Passed;
  Passed = EvaluateFalse("Parse()", "invalid", "An invalid value is rejected", ParseQuiet(Bad, "Bad.Parameters 5 6 7 8 0")) && Passed;
  Passed = EvaluateNear("Parse()", "malformed", "The rejected input keeps the values", Bad.GetDx1(), 1.0, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test configuring a validated shape again
bool UTDShapeTRD2::TestReconfiguration()
{
  MDShapeTRD2 Shape("S");
  return CheckReconfiguration("reconfiguration", Shape, MVector(0.0, 0.0, 1.0), [&Shape]() { return Shape.Set(2.0, 2.0, 2.0, 2.0, 1.0); }, 1.0, [&Shape]() { return Shape.Set(2.0, 2.0, 2.0, 2.0, 2.0); }, 2.0, "S.Parameters 2 2 2 2 3", 3.0, [&Shape]() { return Shape.Set(2.0, 2.0, 2.0, 2.0, -1.0); });
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  gErrorIgnoreLevel = kWarning;

  UTDShapeTRD2 Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDShapeTRD2.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
