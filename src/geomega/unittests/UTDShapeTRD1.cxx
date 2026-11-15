/*
 * UTDShapeTRD1.cxx
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
#include "UTDShapeShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the trapezoid shape MDShapeTRD1
class UTDShapeTRD1 : public UTDShapeBase
{
public:
  //! Default constructor
  UTDShapeTRD1() : UTDShapeBase("UTDShapeTRD1") {}
  //! Default destructor
  virtual ~UTDShapeTRD1() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the analytic truth of the trapezoid: half length in x from X1 at -Z to X2 at +Z
  MTruth GetTruth(double X1, double X2, double Y, double Z);

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
bool UTDShapeTRD1::Run()
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


//! Return the analytic truth of the trapezoid: half length in x from X1 at -Z to X2 at +Z
UTDShapeBase::MTruth UTDShapeTRD1::GetTruth(double X1, double X2, double Y, double Z)
{
  return [=](double x, double y, double z) {
    if (fabs(z) > Z || fabs(y) > Y) {
      return false;
    }
    double T = (z + Z)/(2*Z);
    if (fabs(x) > X1 + T*(X2 - X1)) {
      return false;
    }
    return true;
  };
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default state
bool UTDShapeTRD1::TestDefault()
{
  bool Passed = true;

  MDShapeTRD1 Shape("Trd");
  Passed = EvaluateTrue("GetName()", "constructor", "The name of the constructor is stored", Shape.GetName() == "Trd") && Passed;
  Passed = EvaluateTrue("GetType()", "constructor", "The type is TRD1", Shape.GetType() == "TRD1") && Passed;
  Passed = EvaluateNear("GetVolume()", "default", "A new shape has no volume", Shape.GetVolume(), 0.0, 1e-15) && Passed;

  // Defect GEO-C-7: A shape without parameters passes the validation and becomes a zero-size shape
  MDShapeTRD1 NoParameters("NoParameters");
  Passed = ExpectedToFail(EvaluateFalse("Validate()", "no parameters", "A shape without parameters is rejected", Quiet([&]() { return NoParameters.Validate(); }))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the limits of the parameters
bool UTDShapeTRD1::TestLimits()
{
  bool Passed = true;

  const double Just = 1e-9;
  MDShapeTRD1 S("Trd");

  Passed = EvaluateTrue("Set()", "dx1 = 0", "A zero dx1 is accepted", Quiet([&]() { return S.Set(0, 1, 1, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "dx1 just below 0", "A negative dx1 is rejected", Quiet([&]() { return S.Set(-Just, 1, 1, 1); })) && Passed;
  Passed = EvaluateTrue("Set()", "dx2 = 0", "A zero dx2 is accepted", Quiet([&]() { return S.Set(1, 0, 1, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "dx2 just below 0", "A negative dx2 is rejected", Quiet([&]() { return S.Set(1, -Just, 1, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "dx1 = dx2 = 0", "Both x lengths zero are rejected", Quiet([&]() { return S.Set(0, 0, 1, 1); })) && Passed;
  Passed = EvaluateTrue("Set()", "dx1 = 0, dx2 just above 0", "One small x length is sufficient", Quiet([&]() { return S.Set(0, Just, 1, 1); })) && Passed;
  Passed = EvaluateTrue("Set()", "y just above 0", "A small y is accepted", Quiet([&]() { return S.Set(1, 1, Just, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "y = 0", "A zero y is rejected", Quiet([&]() { return S.Set(1, 1, 0, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "y just below 0", "A negative y is rejected", Quiet([&]() { return S.Set(1, 1, -Just, 1); })) && Passed;
  Passed = EvaluateTrue("Set()", "z just above 0", "A small z is accepted", Quiet([&]() { return S.Set(1, 1, 1, Just); })) && Passed;
  Passed = EvaluateFalse("Set()", "z = 0", "A zero z is rejected", Quiet([&]() { return S.Set(1, 1, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "z just below 0", "A negative z is rejected", Quiet([&]() { return S.Set(1, 1, 1, -Just); })) && Passed;

  // A rejected set does not change the shape:
  S.Set(1, 2, 3, 4);
  Quiet([&]() { return S.Set(5, 6, 7, -1); });
  Passed = EvaluateNear("GetDx1()", "after reject", "The old dx1 is kept", S.GetDx1(), 1.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetDx2()", "after reject", "The old dx2 is kept", S.GetDx2(), 2.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetY()", "after reject", "The old y is kept", S.GetY(), 3.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetZ()", "after reject", "The old z is kept", S.GetZ(), 4.0, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the volume and the size
bool UTDShapeTRD1::TestVolumeSize()
{
  bool Passed = true;

  // Trapezoid area (2 x1 + 2 x2)/2 * 2z times 2y
  MDShapeTRD1 Shape("Trd");
  Shape.Set(1, 3, 2, 1.5);
  Passed = EvaluateNear("GetVolume()", "1, 3, 2, 1.5", "The volume is the trapezoid area times the length", Shape.GetVolume(), 0.5*(2 + 6)*3*4, 1e-12) && Passed;
  Passed = EvaluateVectorNear("GetSize()", "x2 larger", "The surrounding box uses the larger x", Shape.GetSize(), MVector(3, 2, 1.5), 1e-15) && Passed;

  MDShapeTRD1 Inverted("Inverted");
  Inverted.Set(3, 1, 2, 1.5);
  Passed = EvaluateNear("GetVolume()", "3, 1, 2, 1.5", "The volume does not depend on the direction", Inverted.GetVolume(), 0.5*(2 + 6)*3*4, 1e-12) && Passed;
  Passed = EvaluateVectorNear("GetSize()", "x1 larger", "The surrounding box uses the larger x", Inverted.GetSize(), MVector(3, 2, 1.5), 1e-15) && Passed;

  // A wedge: one half of the box
  MDShapeTRD1 Wedge("Wedge");
  Wedge.Set(0, 2, 2, 1.5);
  Passed = EvaluateNear("GetVolume()", "wedge", "The volume of a wedge is half of the box", Wedge.GetVolume(), 0.5*4*4*3, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the inside check
bool UTDShapeTRD1::TestIsInside()
{
  bool Passed = true;

  const double Just = 1e-9;

  MDShapeTRD1 Shape("Trd");
  Shape.Set(1, 3, 2, 1.5);
  Shape.Validate();

  // The half length in x is 1 at z = -1.5, 2 at z = 0, and 3 at z = 1.5
  Passed = EvaluateTrue("IsInside()", "center", "The center is inside", Shape.IsInside(MVector(0, 0, 0))) && Passed;
  Passed = EvaluateTrue("IsInside()", "non-symmetric", "A non-symmetric interior point is inside", Shape.IsInside(MVector(-2.5, 1.5, 1))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just inside x at +z", "Just inside the sloped surface at the top is inside", Shape.IsInside(MVector(3 - Just, 0, 1.5 - Just))) && Passed;
  Passed = EvaluateFalse("IsInside()", "wide x at -z", "The width of the top does not exist at the bottom", Shape.IsInside(MVector(2.5, 0, -1.4))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just inside x at z = 0", "Just inside the sloped surface at the center is inside", Shape.IsInside(MVector(2 - Just, 0, 0))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just outside x at z = 0", "Just outside the sloped surface at the center is outside", Shape.IsInside(MVector(2 + Just, 0, 0))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just inside y", "Just inside y is inside", Shape.IsInside(MVector(0, 2 - Just, 0))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just outside y", "Just outside y is outside", Shape.IsInside(MVector(0, 2 + Just, 0))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just inside z", "Just inside z is inside", Shape.IsInside(MVector(0, 0, 1.5 - Just))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just outside z", "Just outside z is outside", Shape.IsInside(MVector(0, 0, 1.5 + Just))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just outside -z", "Just outside -z is outside", Shape.IsInside(MVector(0, 0, -1.5 - Just))) && Passed;

  Passed = CheckToleranceAtSurface("y", Shape, MVector(0, 2, 0), MVector(0, 1, 0)) && Passed;
  Passed = CheckToleranceAtSurface("z", Shape, MVector(0, 0, 1.5), MVector(0, 0, 1)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the distance functions with fixed rays
bool UTDShapeTRD1::TestDistances()
{
  bool Passed = true;

  MDShapeTRD1 Shape("Trd");
  Shape.Set(1, 3, 2, 1.5);
  Shape.Validate();

  Passed = EvaluateNear("DistanceInsideOut()", "+x at z = 0", "The half length at z = 0 is 2", Shape.DistanceInsideOut(MVector(0, 0, 0), MVector(1, 0, 0)), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "+y", "The distance to +y", Shape.DistanceInsideOut(MVector(0, 0.5, 0), MVector(0, 1, 0)), 1.5, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "-z", "The distance to the bottom", Shape.DistanceInsideOut(MVector(0, 0, 0.5), MVector(0, 0, -1)), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceOutsideIn()", "-x at z = 0", "The distance to the sloped surface from outside", Shape.DistanceOutsideIn(MVector(5, 0, 0), MVector(-1, 0, 0)), 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceOutsideIn()", "-z", "The distance to the top from above", Shape.DistanceOutsideIn(MVector(0, 0, 4), MVector(0, 0, -1)), 2.5, 1e-12) && Passed;
  Passed = EvaluateTrue("DistanceOutsideIn()", "miss", "A ray away from the shape is no hit", Shape.DistanceOutsideIn(MVector(0, 0, 4), MVector(0, 0, 1)) > 1e20) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the unique position
bool UTDShapeTRD1::TestUniquePosition()
{
  bool Passed = true;

  MDShapeTRD1 Shape("Trd");
  Shape.Set(1, 3, 2, 1.5);
  Shape.Validate();
  Passed = EvaluateVectorNear("GetUniquePosition()", "trapezoid", "The position is the center", Shape.GetUniquePosition(), MVector(0, 0, 0), 1e-15) && Passed;
  Passed = EvaluateTrue("GetUniquePosition()", "trapezoid", "The position is inside", Shape.IsInside(Shape.GetUniquePosition())) && Passed;

  MDShapeTRD1 Wedge("Wedge");
  Wedge.Set(0, 2, 2, 1.5);
  Wedge.Validate();
  Passed = EvaluateTrue("GetUniquePosition()", "wedge", "The position is inside a wedge", Wedge.IsInside(Wedge.GetUniquePosition())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the geometry against the analytic truth
bool UTDShapeTRD1::TestAgainstTruth()
{
  bool Passed = true;

  const MVector Min(-3.5, -2.5, -2);
  const MVector Max(3.5, 2.5, 2);

  MDShapeTRD1 Shape("Trd");
  Shape.Set(1, 3, 2, 1.5);
  Shape.Validate();
  MTruth Truth = GetTruth(1, 3, 2, 1.5);
  Passed = CheckTruthVolume("trapezoid", Truth, Min, Max, Shape.GetVolume()) && Passed;
  Passed = CheckAgainstTruth("trapezoid", Shape, Truth, Min, Max) && Passed;

  MDShapeTRD1 Inverted("Inverted");
  Inverted.Set(3, 0.5, 2, 1.5);
  Inverted.Validate();
  MTruth TruthInverted = GetTruth(3, 0.5, 2, 1.5);
  Passed = CheckTruthVolume("inverted", TruthInverted, Min, Max, Inverted.GetVolume()) && Passed;
  Passed = CheckAgainstTruth("inverted", Inverted, TruthInverted, Min, Max) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the string representations
bool UTDShapeTRD1::TestStrings()
{
  bool Passed = true;

  MDShapeTRD1 Shape("Trd");
  Shape.Set(1, 3, 2, 1.5);
  Passed = EvaluateTrue("ToString()", "trapezoid", "The string contains type and parameters", Shape.ToString().Contains("TRD1 (1, 3, 2, 1.5)")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "trapezoid", "The text is the keyword with the parameters", Shape.GetGeomega() == "TRD1 1 3 2 1.5") && Passed;

  MDShapeTRD1 Read("Read");
  Passed = EvaluateTrue("Parse()", "round trip", "The text can be parsed", ParseQuiet(Read, MString("V.Shape ") + Shape.GetGeomega())) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "dx1 survives", Read.GetDx1(), 1.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "dx2 survives", Read.GetDx2(), 3.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "y survives", Read.GetY(), 2.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "z survives", Read.GetZ(), 1.5, 1e-4) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the scaling
bool UTDShapeTRD1::TestScale()
{
  bool Passed = true;

  MDShapeTRD1 All("All");
  All.Set(1, 3, 2, 1.5);
  All.Validate();
  double Volume = All.GetVolume();
  Passed = CheckScaleState("XYZ", All, 2.0, "XYZ") && Passed;
  Passed = EvaluateVectorNear("Scale()", "XYZ", "All lengths are scaled", All.GetSize(), MVector(6, 4, 3), 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "The volume is scaled by the third power", All.GetVolume(), 8*Volume, 1e-9) && Passed;
  Passed = CheckInsideAgainstTruth("XYZ", All, GetTruth(2, 6, 4, 3), MVector(-7, -5, -4), MVector(7, 5, 4)) && Passed;

  MDShapeTRD1 X("X");
  X.Set(1, 3, 2, 1.5);
  X.Validate();
  Passed = EvaluateTrue("Scale()", "X", "The axis X is accepted", X.Scale(2.0, "X")) && Passed;
  Passed = EvaluateNear("Scale()", "X", "dx1 is scaled", X.GetDx1(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "X", "dx2 is scaled", X.GetDx2(), 6.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "X", "y is not scaled", X.GetY(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "X", "z is not scaled", X.GetZ(), 1.5, 1e-12) && Passed;

  MDShapeTRD1 Y("Y");
  Y.Set(1, 3, 2, 1.5);
  Y.Validate();
  Passed = EvaluateTrue("Scale()", "Y", "The axis Y is accepted", Y.Scale(2.0, "Y")) && Passed;
  Passed = EvaluateVectorNear("Scale()", "Y", "Only y is scaled", Y.GetSize(), MVector(3, 4, 1.5), 1e-12) && Passed;

  MDShapeTRD1 Z("Z");
  Z.Set(1, 3, 2, 1.5);
  Z.Validate();
  Passed = EvaluateTrue("Scale()", "Z", "The axis Z is accepted", Z.Scale(2.0, "Z")) && Passed;
  Passed = EvaluateVectorNear("Scale()", "Z", "Only z is scaled", Z.GetSize(), MVector(3, 2, 3), 1e-12) && Passed;

  // Defect GEO-C-5: A negative factor is accepted while Set() rejects negative lengths
  MDShapeTRD1 Negative("Negative");
  Negative.Set(1, 3, 2, 1.5);
  Negative.Validate();
  Passed = ExpectedToFail(EvaluateFalse("Scale()", "factor -2", "A negative factor is rejected", Negative.Scale(-2.0, "XYZ"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parsing the keywords
bool UTDShapeTRD1::TestParse()
{
  bool Passed = true;

  MDShapeTRD1 Parameters("Trd");
  Passed = EvaluateTrue("Parse()", "Parameters", "The keyword Parameters is accepted", ParseQuiet(Parameters, "Trd.Parameters 1.5 2.5 3.5 4.5")) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "dx1 is read", Parameters.GetDx1(), 1.5, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "dx2 is read", Parameters.GetDx2(), 2.5, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "y is read", Parameters.GetY(), 3.5, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "z is read", Parameters.GetZ(), 4.5, 1e-15) && Passed;

  MDShapeTRD1 Keyword("Keyword");
  Passed = EvaluateTrue("Parse()", "Shape", "The keyword Shape is accepted", ParseQuiet(Keyword, "V.Shape TRD1 1.5 2.5 3.5 4.5")) && Passed;
  Passed = EvaluateNear("Parse()", "Shape", "The parameters start after the type", Keyword.GetZ(), 4.5, 1e-15) && Passed;

  MDShapeTRD1 Bad("Bad");
  Bad.Set(1, 2, 3, 4);
  Passed = EvaluateFalse("Parse()", "3 parameters", "Three parameters are rejected", ParseQuiet(Bad, "Bad.Parameters 5 6 7")) && Passed;
  Passed = EvaluateFalse("Parse()", "5 parameters", "Five parameters are rejected", ParseQuiet(Bad, "Bad.Parameters 5 6 7 8 9")) && Passed;
  Passed = EvaluateFalse("Parse()", "wrong keyword", "A wrong keyword is rejected", ParseQuiet(Bad, "Bad.Param 5 6 7 8")) && Passed;
  Passed = EvaluateFalse("Parse()", "invalid", "An invalid value is rejected", ParseQuiet(Bad, "Bad.Parameters 5 6 7 0")) && Passed;
  Passed = EvaluateNear("Parse()", "malformed", "The rejected input keeps the values", Bad.GetDx1(), 1.0, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test configuring a validated shape again
bool UTDShapeTRD1::TestReconfiguration()
{
  MDShapeTRD1 Shape("S");
  return CheckReconfiguration("reconfiguration", Shape, MVector(0.0, 0.0, 1.0), [&Shape]() { return Shape.Set(2.0, 2.0, 2.0, 1.0); }, 1.0, [&Shape]() { return Shape.Set(2.0, 2.0, 2.0, 2.0); }, 2.0, "S.Parameters 2 2 2 3", 3.0, [&Shape]() { return Shape.Set(2.0, 2.0, 2.0, -1.0); });
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  gErrorIgnoreLevel = kWarning;

  UTDShapeTRD1 Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDShapeTRD1.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
