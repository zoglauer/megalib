/*
 * UTDShapeBRIK.cxx
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
#include <TGeoBBox.h>
#include <TROOT.h>
using namespace std;

// MEGAlib libs:
#include "MDShapeBRIK.h"
#include "UTDShapeShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the box shape MDShapeBRIK
class UTDShapeBRIK : public UTDShapeBase
{
public:
  //! Default constructor
  UTDShapeBRIK() : UTDShapeBase("UTDShapeBRIK") {}
  //! Default destructor
  virtual ~UTDShapeBRIK() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the analytic truth of a box with the half lengths X, Y, Z
  MTruth GetTruth(double X, double Y, double Z);

  //! Test the default state
  bool TestDefault();
  //! Test the limits of the parameters
  bool TestLimits();
  //! Test the setting from a ROOT box
  bool TestSetFromRoot();
  //! Test the volume and the size
  bool TestVolumeSize();
  //! Test the inside check
  bool TestIsInside();
  //! Test the distance functions
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
  //! Test reusing the same object
  bool TestReuse();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDShapeBRIK::Run()
{
  bool Passed = true;

  Passed = TestDefault() && Passed;
  Passed = TestLimits() && Passed;
  Passed = TestSetFromRoot() && Passed;
  Passed = TestVolumeSize() && Passed;
  Passed = TestIsInside() && Passed;
  Passed = TestDistances() && Passed;
  Passed = TestUniquePosition() && Passed;
  Passed = TestAgainstTruth() && Passed;
  Passed = TestStrings() && Passed;
  Passed = TestScale() && Passed;
  Passed = TestParse() && Passed;
  Passed = TestReuse() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the analytic truth of a box with the half lengths X, Y, Z
UTDShapeBase::MTruth UTDShapeBRIK::GetTruth(double X, double Y, double Z)
{
  return [=](double x, double y, double z) {
    if (fabs(x) > X || fabs(y) > Y || fabs(z) > Z) {
      return false;
    }
    return true;
  };
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default state
bool UTDShapeBRIK::TestDefault()
{
  bool Passed = true;

  MDShapeBRIK Shape("Box");
  Passed = EvaluateTrue("GetName()", "constructor", "The name of the constructor is stored", Shape.GetName() == "Box") && Passed;
  Passed = EvaluateTrue("GetType()", "constructor", "The type is BRIK", Shape.GetType() == "BRIK") && Passed;
  Passed = EvaluateNear("GetVolume()", "default", "A new box has no volume", Shape.GetVolume(), 0.0, 1e-15) && Passed;
  Passed = EvaluateVectorNear("GetSize()", "default", "A new box has no size", Shape.GetSize(), MVector(0, 0, 0), 1e-15) && Passed;
  Passed = Evaluate("GetNSubShapes()", "default", "A box has no sub shapes", Shape.GetNSubShapes(), 0U) && Passed;
  Passed = EvaluateTrue("GetSubShape()", "default", "A box returns no sub shape", Shape.GetSubShape(0) == nullptr) && Passed;
  Passed = EvaluateTrue("CreateSurfacePattern()", "default", "The surface pattern of a box is empty", Shape.CreateSurfacePattern().empty()) && Passed;

  // Defect GEO-C-7: A shape without parameters passes the validation and becomes a zero-size shape
  MDShapeBRIK NoParameters("NoParameters");
  Passed = ExpectedToFail(EvaluateFalse("Validate()", "no parameters", "A box without parameters is rejected", Quiet([&]() { return NoParameters.Validate(); }))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the limits of the parameters
bool UTDShapeBRIK::TestLimits()
{
  bool Passed = true;

  const double Just = 1e-9;

  MDShapeBRIK Shape("Box");
  Passed = EvaluateTrue("Set()", "x just above 0", "A small positive x is accepted", Quiet([&]() { return Shape.Set(Just, 1, 1); })) && Passed;
  Passed = EvaluateTrue("Set()", "y just above 0", "A small positive y is accepted", Quiet([&]() { return Shape.Set(1, Just, 1); })) && Passed;
  Passed = EvaluateTrue("Set()", "z just above 0", "A small positive z is accepted", Quiet([&]() { return Shape.Set(1, 1, Just); })) && Passed;

  Passed = EvaluateFalse("Set()", "x = 0", "A zero x is rejected", Quiet([&]() { return Shape.Set(0, 1, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "y = 0", "A zero y is rejected", Quiet([&]() { return Shape.Set(1, 0, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "z = 0", "A zero z is rejected", Quiet([&]() { return Shape.Set(1, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "x just below 0", "A negative x is rejected", Quiet([&]() { return Shape.Set(-Just, 1, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "y just below 0", "A negative y is rejected", Quiet([&]() { return Shape.Set(1, -Just, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "z just below 0", "A negative z is rejected", Quiet([&]() { return Shape.Set(1, 1, -Just); })) && Passed;

  // The rejected values leave the last accepted ones:
  Passed = EvaluateNear("GetSizeX()", "after rejects", "The last accepted x is kept", Shape.GetSizeX(), 1.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetSizeY()", "after rejects", "The last accepted y is kept", Shape.GetSizeY(), 1.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetSizeZ()", "after rejects", "The last accepted z is kept", Shape.GetSizeZ(), Just, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the setting from a ROOT box
bool UTDShapeBRIK::TestSetFromRoot()
{
  bool Passed = true;

  TGeoBBox Box(1.5, 2.5, 3.5);
  MDShapeBRIK Shape("Box");
  Passed = EvaluateTrue("Set(TGeoBBox*)", "valid box", "A valid ROOT box is accepted", Shape.Set(&Box)) && Passed;
  Passed = EvaluateVectorNear("GetSize()", "from ROOT box", "The half lengths are copied", Shape.GetSize(), MVector(1.5, 2.5, 3.5), 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the volume and the size
bool UTDShapeBRIK::TestVolumeSize()
{
  bool Passed = true;

  MDShapeBRIK Shape("Box");
  Shape.Set(1.5, 2.5, 0.5);

  // Volume = 2x * 2y * 2z
  Passed = EvaluateNear("GetVolume()", "1.5, 2.5, 0.5", "The volume is the product of the full lengths", Shape.GetVolume(), 3.0*5.0*1.0, 1e-12) && Passed;
  Passed = EvaluateVectorNear("GetSize()", "1.5, 2.5, 0.5", "The size is given by the half lengths", Shape.GetSize(), MVector(1.5, 2.5, 0.5), 1e-15) && Passed;
  Passed = EvaluateNear("GetSizeX()", "1.5, 2.5, 0.5", "The size in x is the half length", Shape.GetSizeX(), 1.5, 1e-15) && Passed;
  Passed = EvaluateNear("GetSizeY()", "1.5, 2.5, 0.5", "The size in y is the half length", Shape.GetSizeY(), 2.5, 1e-15) && Passed;
  Passed = EvaluateNear("GetSizeZ()", "1.5, 2.5, 0.5", "The size in z is the half length", Shape.GetSizeZ(), 0.5, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the inside check
bool UTDShapeBRIK::TestIsInside()
{
  bool Passed = true;

  const double Just = 1e-9;

  MDShapeBRIK Shape("Box");
  Shape.Set(1.5, 2.5, 0.5);
  Shape.Validate();

  // Interior including a non-symmetric point:
  Passed = EvaluateTrue("IsInside()", "center", "The center is inside", Shape.IsInside(MVector(0, 0, 0))) && Passed;
  Passed = EvaluateTrue("IsInside()", "non-symmetric", "A non-symmetric interior point is inside", Shape.IsInside(MVector(-1.2, 2.1, 0.3))) && Passed;

  // Each surface: on it, just inside, and just outside
  const vector<MVector> Axes = { MVector(1.5, 0, 0), MVector(-1.5, 0, 0), MVector(0, 2.5, 0), MVector(0, -2.5, 0), MVector(0, 0, 0.5), MVector(0, 0, -0.5) };
  for (const MVector& S : Axes) {
    MVector Out = S.Unit();
    Passed = EvaluateTrue("IsInside()", "on the surface", "A point on the surface is inside", Shape.IsInside(S)) && Passed;
    Passed = EvaluateTrue("IsInside()", "just below the surface", "A point just inside the surface is inside", Shape.IsInside(S - Just*Out)) && Passed;
    Passed = EvaluateFalse("IsInside()", "just above the surface", "A point just outside the surface is outside", Shape.IsInside(S + Just*Out)) && Passed;
    Passed = CheckToleranceAtSurface("surface of a box", Shape, S, Out) && Passed;
  }

  // Exterior, only one coordinate is outside:
  Passed = EvaluateFalse("IsInside()", "x outside", "A point with only x outside is outside", Shape.IsInside(MVector(1.6, 2.4, 0.4))) && Passed;
  Passed = EvaluateFalse("IsInside()", "y outside", "A point with only y outside is outside", Shape.IsInside(MVector(1.4, -2.6, 0.4))) && Passed;
  Passed = EvaluateFalse("IsInside()", "z outside", "A point with only z outside is outside", Shape.IsInside(MVector(1.4, 2.4, -0.6))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the distance functions
bool UTDShapeBRIK::TestDistances()
{
  bool Passed = true;

  MDShapeBRIK Shape("Box");
  Shape.Set(1.5, 2.5, 0.5);
  Shape.Validate();

  // Straight lines, distance is half length minus coordinate
  Passed = EvaluateNear("DistanceInsideOut()", "+x", "The distance to the +x surface", Shape.DistanceInsideOut(MVector(0.5, 1, 0.1), MVector(1, 0, 0)), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "-y", "The distance to the -y surface", Shape.DistanceInsideOut(MVector(0.5, 1, 0.1), MVector(0, -1, 0)), 3.5, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "+z", "The distance to the +z surface", Shape.DistanceInsideOut(MVector(0.5, 1, 0.1), MVector(0, 0, 1)), 0.4, 1e-12) && Passed;

  // Diagonal: (0.6, 0, 0.8) reaches z = 0.5 after 0.5/0.8, x = 0.375 is still inside
  Passed = EvaluateNear("DistanceInsideOut()", "diagonal", "The distance along a non-symmetric direction", Shape.DistanceInsideOut(MVector(0, 0, 0), MVector(0.6, 0, 0.8)), 0.625, 1e-12) && Passed;

  // From outside towards the surface:
  Passed = EvaluateNear("DistanceOutsideIn()", "+x", "The distance from outside along +x", Shape.DistanceOutsideIn(MVector(-5, 0.3, 0.1), MVector(1, 0, 0)), 3.5, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceOutsideIn()", "-z", "The distance from outside along -z", Shape.DistanceOutsideIn(MVector(0.2, 0.3, 4), MVector(0, 0, -1)), 3.5, 1e-12) && Passed;
  // (0.6, 0, 0.8) from (-3, 0, -1): z = -0.5 after 0.625 at x = -2.625 is not yet in, x = -1.5 after 2.5 at z = 1 has passed - no hit
  Passed = EvaluateTrue("DistanceOutsideIn()", "miss", "A ray which passes the box is no hit", Shape.DistanceOutsideIn(MVector(-3, 0, -1), MVector(0.6, 0, 0.8)) > 1e20) && Passed;

  // A ray away from the box in the direction of a surface normal, and its sibling with a different direction
  Passed = EvaluateTrue("DistanceOutsideIn()", "miss, parallel", "A ray parallel to the box above it is no hit", Shape.DistanceOutsideIn(MVector(0, 0, 4), MVector(1, 0, 0)) > 1e20) && Passed;

  // Defect GEO-C-8: A ray moving away from the box returns the distance 0 - identical to a hit at the start point - and not the no-hit value of all other rays
  Passed = ExpectedToFail(EvaluateTrue("DistanceOutsideIn()", "miss, moving away", "A ray moving away from the box is no hit", Shape.DistanceOutsideIn(MVector(0, 0, 4), MVector(0, 0, 1)) > 1e20)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the unique position
bool UTDShapeBRIK::TestUniquePosition()
{
  bool Passed = true;

  MDShapeBRIK Shape("Box");
  Shape.Set(1.5, 2.5, 0.5);
  Shape.Validate();
  Passed = EvaluateVectorNear("GetUniquePosition()", "box", "The unique position is the center", Shape.GetUniquePosition(), MVector(0, 0, 0), 1e-15) && Passed;
  Passed = EvaluateTrue("GetUniquePosition()", "box", "The unique position is inside", Shape.IsInside(Shape.GetUniquePosition())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the geometry against the analytic truth
bool UTDShapeBRIK::TestAgainstTruth()
{
  bool Passed = true;

  MDShapeBRIK Shape("Box");
  Shape.Set(1.5, 2.5, 0.5);
  Shape.Validate();

  MTruth Truth = GetTruth(1.5, 2.5, 0.5);
  MVector Min(-2, -3, -1);
  MVector Max(2, 3, 1);
  Passed = CheckTruthVolume("box", Truth, Min, Max, 15.0) && Passed;
  Passed = CheckInsideAgainstTruth("box", Shape, Truth, Min, Max) && Passed;
  Passed = CheckRandomPositions("box", Shape, Truth, Min, Max) && Passed;

  // Rays from the inside:
  unsigned int WrongInsideOut = 0;
  for (unsigned int r = 0; r < 2000; ++r) {
    MVector P = GetTruthPoint(Truth, Min, Max);
    MVector D = GetRandomDirection();
    double L = Shape.DistanceInsideOut(P, D);
    MVector Before = P + (L - 1e-6)*D;
    MVector After = P + (L + 1e-6)*D;
    if (Truth(Before.X(), Before.Y(), Before.Z()) == false || Truth(After.X(), After.Y(), After.Z()) == true) {
      ++WrongInsideOut;
    }
  }
  Passed = Evaluate("DistanceInsideOut()", "box", "The exit of all random rays agrees with the analytic truth", WrongInsideOut, 0U) && Passed;

  // Rays to an inside point can never be a moving away ray
  unsigned int WrongOutsideIn = 0;
  for (unsigned int r = 0; r < 2000; ++r) {
    MVector P;
    do {
      P = GetBoxPoint(Min, Max);
    } while (Truth(P.X(), P.Y(), P.Z()) == true);
    MVector Target = GetTruthPoint(Truth, Min, Max);
    MVector D = (Target - P).Unit();
    double L = Shape.DistanceOutsideIn(P, D);
    MVector Before = P + (L - 1e-6)*D;
    MVector After = P + (L + 1e-6)*D;
    if (Truth(Before.X(), Before.Y(), Before.Z()) == true || Truth(After.X(), After.Y(), After.Z()) == false) {
      ++WrongOutsideIn;
    }
  }
  Passed = Evaluate("DistanceOutsideIn()", "box", "The entry of all random rays agrees with the analytic truth", WrongOutsideIn, 0U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the string representations
bool UTDShapeBRIK::TestStrings()
{
  bool Passed = true;

  MDShapeBRIK Shape("Box");
  Shape.Set(1.5, 2.5, 0.5);

  MString String = Shape.ToString();
  Passed = EvaluateTrue("ToString()", "1.5, 2.5, 0.5", "The string contains the type and the parameters", String.Contains("BRIK (1.5, 2.5, 0.5)")) && Passed;

  // The volume writes "<Name>.Shape " followed by this text
  MString Geomega = Shape.GetGeomega();
  Passed = EvaluateTrue("GetGeomega()", "1.5, 2.5, 0.5", "The text contains the parameters", Geomega.Contains("1.5 2.5 0.5")) && Passed;

  // Defect GEO-C-9: The text is a two line shape definition, which gives a broken line if appended to "<Name>.Shape " as the volume does
  MDShapeBRIK Read("Read");
  Passed = ExpectedToFail(EvaluateTrue("GetGeomega()", "round trip", "The text can be parsed as the shape keyword of a volume", ParseQuiet(Read, MString("V.Shape ") + Geomega))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the scaling
bool UTDShapeBRIK::TestScale()
{
  bool Passed = true;

  // All axes:
  MDShapeBRIK All("All");
  All.Set(1.5, 2.5, 0.5);
  All.Validate();
  Passed = CheckScaleState("XYZ", All, 2.0, "XYZ") && Passed;
  Passed = EvaluateVectorNear("Scale()", "XYZ", "All half lengths are scaled", All.GetSize(), MVector(3, 5, 1), 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "The volume is scaled by the third power", All.GetVolume(), 8*15.0, 1e-12) && Passed;
  Passed = CheckInsideAgainstTruth("XYZ", All, GetTruth(3, 5, 1), MVector(-4, -6, -2), MVector(4, 6, 2)) && Passed;

  // Single axes and a pair:
  MDShapeBRIK X("X");
  X.Set(1.5, 2.5, 0.5);
  X.Validate();
  Passed = EvaluateTrue("Scale()", "X", "The axis X is accepted", X.Scale(3.0, "X")) && Passed;
  Passed = EvaluateVectorNear("Scale()", "X", "Only x is scaled", X.GetSize(), MVector(4.5, 2.5, 0.5), 1e-12) && Passed;

  MDShapeBRIK Y("Y");
  Y.Set(1.5, 2.5, 0.5);
  Y.Validate();
  Passed = EvaluateTrue("Scale()", "Y", "The axis Y is accepted", Y.Scale(3.0, "Y")) && Passed;
  Passed = EvaluateVectorNear("Scale()", "Y", "Only y is scaled", Y.GetSize(), MVector(1.5, 7.5, 0.5), 1e-12) && Passed;

  MDShapeBRIK Z("Z");
  Z.Set(1.5, 2.5, 0.5);
  Z.Validate();
  Passed = EvaluateTrue("Scale()", "Z", "The axis Z is accepted", Z.Scale(0.5, "Z")) && Passed;
  Passed = EvaluateVectorNear("Scale()", "Z", "Only z is scaled", Z.GetSize(), MVector(1.5, 2.5, 0.25), 1e-12) && Passed;
  Passed = EvaluateTrue("Scale()", "Z", "The ROOT shape follows the scaling", Z.DistanceInsideOut(MVector(0, 0, 0), MVector(0, 0, 1)) < 0.25 + 1e-12) && Passed;

  // Defect GEO-C-5: A negative factor is accepted while Set() rejects negative half lengths
  MDShapeBRIK Negative("Negative");
  Negative.Set(1.5, 2.5, 0.5);
  Negative.Validate();
  Passed = ExpectedToFail(EvaluateFalse("Scale()", "factor -2", "A negative factor is rejected", Quiet([&]() { return Negative.Scale(-2.0, "XYZ"); }))) && Passed;

  // Defect GEO-C-5: A factor of zero is accepted while Set() rejects zero half lengths
  MDShapeBRIK Zero("Zero");
  Zero.Set(1.5, 2.5, 0.5);
  Zero.Validate();
  Passed = ExpectedToFail(EvaluateFalse("Scale()", "factor 0", "A factor of zero is rejected", Quiet([&]() { return Zero.Scale(0.0, "XYZ"); }))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parsing the keywords
bool UTDShapeBRIK::TestParse()
{
  bool Passed = true;

  MDShapeBRIK Parameters("Box");
  Passed = EvaluateTrue("Parse()", "Parameters", "The keyword Parameters is accepted", ParseQuiet(Parameters, "Box.Parameters 1.5 2.5 0.5")) && Passed;
  Passed = EvaluateVectorNear("Parse()", "Parameters", "The parameters are read in the order x, y, z", Parameters.GetSize(), MVector(1.5, 2.5, 0.5), 1e-15) && Passed;

  MDShapeBRIK Shape("Box");
  Passed = EvaluateTrue("Parse()", "Shape BRIK", "The keyword Shape is accepted", ParseQuiet(Shape, "Volume.Shape BRIK 1.5 2.5 0.5")) && Passed;
  Passed = EvaluateVectorNear("Parse()", "Shape BRIK", "The parameters are read after the type", Shape.GetSize(), MVector(1.5, 2.5, 0.5), 1e-15) && Passed;

  MDShapeBRIK Box("Box");
  Passed = EvaluateTrue("Parse()", "Shape BOX", "The alternative type BOX is accepted", ParseQuiet(Box, "Volume.Shape BOX 1.5 2.5 0.5")) && Passed;

  // Malformed input does not change the shape:
  MDShapeBRIK Bad("Bad");
  Bad.Set(1, 2, 3);
  Passed = EvaluateFalse("Parse()", "too few", "Too few parameters are rejected", ParseQuiet(Bad, "Bad.Parameters 1.5 2.5")) && Passed;
  Passed = EvaluateFalse("Parse()", "too many", "Too many parameters are rejected", ParseQuiet(Bad, "Bad.Parameters 1.5 2.5 0.5 0.1")) && Passed;
  Passed = EvaluateFalse("Parse()", "wrong keyword", "A wrong keyword is rejected", ParseQuiet(Bad, "Bad.Param 1.5 2.5 0.5")) && Passed;
  Passed = EvaluateFalse("Parse()", "zero", "An invalid parameter is rejected", ParseQuiet(Bad, "Bad.Parameters 1.5 0 0.5")) && Passed;
  Passed = EvaluateVectorNear("Parse()", "malformed", "The rejected input leaves the old values", Bad.GetSize(), MVector(1, 2, 3), 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test reusing the same object
bool UTDShapeBRIK::TestReuse()
{
  bool Passed = true;

  MDShapeBRIK Shape("Box");
  Shape.Set(1, 1, 1);
  Passed = EvaluateTrue("Validate()", "first", "The first validation creates the shape", Shape.Validate()) && Passed;
  TGeoShape* First = Shape.GetRootShape();
  Passed = EvaluateTrue("GetRootShape()", "first", "The root shape exists", First != nullptr) && Passed;
  Passed = EvaluateTrue("Validate()", "second", "The second validation is accepted", Shape.Validate()) && Passed;
  Passed = EvaluateTrue("GetRootShape()", "second", "An unchanged shape keeps its root shape", Shape.GetRootShape() == First) && Passed;

  Passed = EvaluateTrue("Set()", "new values", "New values are accepted", Shape.Set(2, 2, 2)) && Passed;
  Passed = EvaluateTrue("Validate()", "after Set", "The validation after a change is accepted", Shape.Validate()) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "after Set", "The root shape follows the change", Shape.DistanceInsideOut(MVector(0, 0, 0), MVector(1, 0, 0)), 2.0, 1e-12) && Passed;

  Passed = EvaluateTrue("Parse()", "new values", "A new parse is accepted", ParseQuiet(Shape, "Box.Parameters 3 3 3")) && Passed;
  Passed = EvaluateTrue("Validate()", "after Parse", "The validation after a parse is accepted", Shape.Validate()) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "after Parse", "The root shape follows the parse", Shape.DistanceInsideOut(MVector(0, 0, 0), MVector(1, 0, 0)), 3.0, 1e-12) && Passed;

  DisableDefaultStreams();
  const bool Accepted = Shape.Set(-1, 1, 1);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Set()", "invalid values", "An invalid configuration of a valid shape is rejected", Accepted) && Passed;
  Passed = EvaluateTrue("Validate()", "after invalid values", "The shape is still valid after the rejected configuration", Shape.Validate()) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "after invalid values", "The shape keeps its last valid configuration", Shape.DistanceInsideOut(MVector(0, 0, 0), MVector(1, 0, 0)), 3.0, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  gErrorIgnoreLevel = kWarning;

  UTDShapeBRIK Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDShapeBRIK.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
