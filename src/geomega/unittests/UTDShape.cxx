/*
 * UTDShape.cxx
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
#include <cstring>
#include <new>

// ROOT libs:
#include <TApplication.h>
#include <TError.h>
#include <TGeoBBox.h>
#include <TROOT.h>
using namespace std;

// MEGAlib libs:
#include "MDShape.h"
#include "UTDShapeShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Minimal shape which uses all default implementations of the base class - a box around an origin
class UTDShapeMinimal : public MDShape
{
public:
  //! Standard constructor
  UTDShapeMinimal(const MString& Name) : MDShape(Name), m_NeverInsideBefore(0), m_Trials(0) {}

  //! Create the ROOT shape with half lengths and an origin
  void Create(double X, double Y, double Z, double* Origin) { m_Geo = new TGeoBBox(X, Y, Z, Origin); }
  //! Attach a sub shape
  void AddSubShape(MDShape* Shape) { m_SubShapes.push_back(Shape); }
  //! Let the next trials of the inside check fail - and the following ones use the box
  void SetNeverInsideBefore(unsigned long Trials) { m_NeverInsideBefore = Trials; m_Trials = 0; }

  virtual MVector GetSize() { return MVector(); }
  virtual MString ToString() { return ""; }
  virtual MString GetGeomega() const { return ""; }
  virtual double GetVolume() { return 0.0; }
  virtual MVector GetUniquePosition() const { return MVector(); }
  virtual bool IsInside(const MVector& Pos, const double Tolerance = 0, const bool PreferOutside = false)
  {
    ++m_Trials;
    if (m_Trials <= m_NeverInsideBefore) {
      return false;
    }
    return MDShape::IsInside(Pos, Tolerance, PreferOutside);
  }

  using MDShape::m_IsValidated;
  using MDShape::m_Geo;

private:
  //! Number of inside checks which fail
  unsigned long m_NeverInsideBefore;
  //! Number of inside checks done
  unsigned long m_Trials;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the shape base class MDShape
class UTDShape : public UTDShapeBase
{
public:
  //! Default constructor
  UTDShape() : UTDShapeBase("UTDShape") {}
  //! Default destructor
  virtual ~UTDShape() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the default state
  bool TestDefault();
  //! Test the initialization of all members
  bool TestInitialization();
  //! Test the color
  bool TestColor();
  //! Test the sub shapes
  bool TestSubShapes();
  //! Test the default inside check
  bool TestIsInside();
  //! Test the default distance functions
  bool TestDistances();
  //! Test the default random positions
  bool TestRandomPositions();
  //! Test the scaling
  bool TestScale();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDShape::Run()
{
  bool Passed = true;

  Passed = TestDefault() && Passed;
  Passed = TestInitialization() && Passed;
  Passed = TestColor() && Passed;
  Passed = TestSubShapes() && Passed;
  Passed = TestIsInside() && Passed;
  Passed = TestDistances() && Passed;
  Passed = TestRandomPositions() && Passed;
  Passed = TestScale() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default state
bool UTDShape::TestDefault()
{
  bool Passed = true;

  UTDShapeMinimal Shape("Name");
  Passed = EvaluateTrue("GetName()", "constructor", "The name is stored", Shape.GetName() == "Name") && Passed;
  Passed = EvaluateTrue("GetType()", "base class", "The base class has no type", Shape.GetType() == "") && Passed;
  Passed = EvaluateFalse("Validate()", "base class", "The base class never validates", Shape.Validate()) && Passed;
  Passed = EvaluateTrue("GetRootShape()", "new", "There is no ROOT shape before the validation", Shape.GetRootShape() == nullptr) && Passed;
  Passed = EvaluateTrue("CreateSurfacePattern()", "detail 0", "The default surface pattern is empty", Shape.CreateSurfacePattern(0).empty()) && Passed;
  Passed = EvaluateTrue("CreateSurfacePattern()", "detail 5", "The default surface pattern is empty for all details", Shape.CreateSurfacePattern(5).empty()) && Passed;

  // The base class cannot parse:
  Passed = EvaluateFalse("Parse()", "base class", "The base class rejects all input", ParseQuiet(Shape, "Name.Parameters 1 2 3")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the initialization of all members
bool UTDShape::TestInitialization()
{
  bool Passed = true;

  // Special technique 2: the pattern byte shows the members which the constructor does not set
  const unsigned char Pattern = 0xA7;
  alignas(UTDShapeMinimal) unsigned char Block[sizeof(UTDShapeMinimal)];
  FillMemory(Block, sizeof(Block), Pattern);
  UTDShapeMinimal* Shape = new (Block) UTDShapeMinimal("Pattern");

  Passed = EvaluateTrue("MDShape()", "pattern memory", "The ROOT shape is initialized", Shape->GetRootShape() == nullptr) && Passed;
  Passed = EvaluateFalse("MDShape()", "pattern memory", "The validation flag is initialized", Shape->m_IsValidated) && Passed;
  Passed = EvaluateNear("MDShape()", "pattern memory", "The scaler is initialized", Shape->GetScaler(), 1.0, 1e-15) && Passed;
  Passed = EvaluateTrue("MDShape()", "pattern memory", "The scaling axis is initialized", Shape->GetScalingAxis() == "") && Passed;

  int Color = 0;
  memset(&Color, Pattern, sizeof(Color));

  // Defect GEO-C-10: The constructor does not initialize the color, GetColor returns garbage until SetColor is called
  Passed = ExpectedToFail(EvaluateTrue("MDShape()", "pattern memory", "The color is initialized", Shape->GetColor() != Color)) && Passed;

  Shape->~UTDShapeMinimal();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the color
bool UTDShape::TestColor()
{
  bool Passed = true;

  UTDShapeMinimal Shape("Name");
  Shape.SetColor(5);
  Passed = Evaluate("SetColor()", "5", "The color is stored", Shape.GetColor(), 5) && Passed;
  Shape.SetColor(-1);
  Passed = Evaluate("SetColor()", "-1", "The new color replaces the old one", Shape.GetColor(), -1) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the sub shapes
bool UTDShape::TestSubShapes()
{
  bool Passed = true;

  UTDShapeMinimal Shape("Name");
  UTDShapeMinimal First("First");
  UTDShapeMinimal Second("Second");

  Passed = Evaluate("GetNSubShapes()", "none", "A new shape has no sub shapes", Shape.GetNSubShapes(), 0U) && Passed;
  Passed = EvaluateTrue("GetSubShape()", "none, index 0", "The index n = 0 returns nothing", Shape.GetSubShape(0) == nullptr) && Passed;

  Shape.AddSubShape(&First);
  Shape.AddSubShape(&Second);
  Passed = Evaluate("GetNSubShapes()", "two", "Two sub shapes are counted", Shape.GetNSubShapes(), 2U) && Passed;
  Passed = EvaluateTrue("GetSubShape()", "index 0", "The first sub shape is returned", Shape.GetSubShape(0) == &First) && Passed;
  Passed = EvaluateTrue("GetSubShape()", "index n - 1", "The last sub shape is returned", Shape.GetSubShape(1) == &Second) && Passed;
  Passed = EvaluateTrue("GetSubShape()", "index n", "The first index after the end returns nothing", Shape.GetSubShape(2) == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default inside check
bool UTDShape::TestIsInside()
{
  bool Passed = true;

  double Origin[3] = { 1, -2, 3 };
  UTDShapeMinimal Shape("Name");
  Shape.Create(1.5, 2.5, 0.5, Origin);

  // The box covers x in [-0.5, 2.5], y in [-4.5, 0.5], and z in [2.5, 3.5]
  Passed = EvaluateTrue("IsInside()", "origin", "The origin of the box is inside", Shape.IsInside(MVector(1, -2, 3))) && Passed;
  Passed = EvaluateTrue("IsInside()", "non-symmetric", "A non-symmetric point is inside", Shape.IsInside(MVector(2.2, 0.1, 2.7))) && Passed;
  Passed = EvaluateFalse("IsInside()", "center of the frame", "The center of the frame is outside the shifted box", Shape.IsInside(MVector(0, 0, 0))) && Passed;

  // The tolerance moves the surface in the preferred direction:
  Passed = CheckToleranceAtSurface("+x surface", Shape, MVector(2.5, -2, 3), MVector(1, 0, 0)) && Passed;
  Passed = CheckToleranceAtSurface("-z surface", Shape, MVector(1, -2, 2.5), MVector(0, 0, -1)) && Passed;

  // Just inside and outside of the tolerance of 0.01:
  Passed = EvaluateTrue("IsInside()", "0.0099 outside", "A point within the tolerance is inside", Shape.IsInside(MVector(2.5099, -2, 3), 0.01, false)) && Passed;
  Passed = EvaluateFalse("IsInside()", "0.0101 outside", "A point outside the tolerance stays outside", Shape.IsInside(MVector(2.5101, -2, 3), 0.01, false)) && Passed;
  Passed = EvaluateFalse("IsInside()", "0.0099 inside", "A point within the tolerance is outside if outside is preferred", Shape.IsInside(MVector(2.4901, -2, 3), 0.01, true)) && Passed;
  Passed = EvaluateTrue("IsInside()", "0.0101 inside", "A point further than the tolerance stays inside if outside is preferred", Shape.IsInside(MVector(2.4899, -2, 3), 0.01, true)) && Passed;

  // A negative tolerance is the same as none in the base class
  Passed = EvaluateFalse("IsInside()", "negative tolerance", "A negative tolerance does nothing", Shape.IsInside(MVector(2.6, -2, 3), -1.0, false)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default distance functions
bool UTDShape::TestDistances()
{
  bool Passed = true;

  double Origin[3] = { 1, -2, 3 };
  UTDShapeMinimal Shape("Name");
  Shape.Create(1.5, 2.5, 0.5, Origin);

  Passed = EvaluateNear("DistanceInsideOut()", "+x", "The distance to the +x surface at 2.5", Shape.DistanceInsideOut(MVector(1.5, -1, 3), MVector(1, 0, 0)), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "-y", "The distance to the -y surface at -4.5", Shape.DistanceInsideOut(MVector(1.5, -1, 3), MVector(0, -1, 0)), 3.5, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceOutsideIn()", "+z", "The distance to the -z surface at 2.5 from below", Shape.DistanceOutsideIn(MVector(1, -2, 0), MVector(0, 0, 1)), 2.5, 1e-12) && Passed;
  Passed = EvaluateTrue("DistanceOutsideIn()", "miss", "A ray away from the box is no hit", Shape.DistanceOutsideIn(MVector(1, -2, 0), MVector(0, 0, -1)) > 1e20) && Passed;
  Passed = EvaluateTrue("DistanceOutsideIn()", "miss", "A ray passing the box is no hit", Shape.DistanceOutsideIn(MVector(1, -2, 0), MVector(1, 0, 0)) > 1e20) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default random positions
bool UTDShape::TestRandomPositions()
{
  bool Passed = true;

  // Box with origin: uniform means are the origin, variance is the half length squared / 3
  double Origin[3] = { 1, -2, 3 };
  UTDShapeMinimal Shape("Name");
  Shape.Create(1.5, 2.5, 0.5, Origin);

  gRandom->SetSeed(987);
  const unsigned int N = 40000;
  double Sum[3] = { 0, 0, 0 };
  double SumSquare[3] = { 0, 0, 0 };
  unsigned int Outside = 0;
  for (unsigned int i = 0; i < N; ++i) {
    MVector P = Shape.GetRandomPositionInside();
    if (fabs(P.X() - 1) > 1.5 || fabs(P.Y() + 2) > 2.5 || fabs(P.Z() - 3) > 0.5) {
      ++Outside;
    }
    for (unsigned int a = 0; a < 3; ++a) {
      Sum[a] += P[a];
      SumSquare[a] += (P[a] - Origin[a])*(P[a] - Origin[a]);
    }
  }
  Passed = Evaluate("GetRandomPositionInside()", "box with origin", "All positions are inside the box", Outside, 0U) && Passed;

  // Mean: sigma = half length / sqrt(3 N), Variance: sigma = half length^2 * sqrt(4 / (45 N)) of a uniform distribution - 5 sigma each
  const double HalfLength[3] = { 1.5, 2.5, 0.5 };
  for (unsigned int a = 0; a < 3; ++a) {
    Passed = EvaluateNear("GetRandomPositionInside()", "box with origin", "The mean is the origin", Sum[a]/N, Origin[a], 5*HalfLength[a]/sqrt(3.0*N)) && Passed;
    Passed = EvaluateNear("GetRandomPositionInside()", "box with origin", "The variance is the one of a uniform distribution", SumSquare[a]/N, HalfLength[a]*HalfLength[a]/3, 5*HalfLength[a]*HalfLength[a]*sqrt(4.0/(45.0*N))) && Passed;
  }

  // More than 10000 trials give a warning, but still a valid position
  UTDShapeMinimal Slow("Slow");
  Slow.Create(1, 1, 1, Origin);
  Slow.SetNeverInsideBefore(20000);
  DisableDefaultStreams();
  MVector P = Slow.GetRandomPositionInside();
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetRandomPositionInside()", "slow", "After many trials the position is still inside", fabs(P.X() - 1) <= 1 && fabs(P.Y() + 2) <= 1 && fabs(P.Z() - 3) <= 1) && Passed;

  // Defect GEO-C-42: A shape without inside region stops the program with abort after 10 million trials
  UTDShapeMinimal Never("Never");
  double Null[3] = { 0, 0, 0 };
  Never.Create(1, 1, 1, Null);
  Never.SetNeverInsideBefore(100000000UL);
  Passed = ExpectedToCrash(EvaluateTrue("GetRandomPositionInside()", "never inside", "No position is returned if none can be found, and the program goes on", Never.GetRandomPositionInside() == g_VectorNotDefined)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the scaling
bool UTDShape::TestScale()
{
  bool Passed = true;

  UTDShapeMinimal Shape("Name");
  Passed = EvaluateFalse("IsScaled()", "new", "A new shape is not scaled", Shape.IsScaled()) && Passed;

  // All valid axis strings are accepted:
  for (const MString& Axes : vector<MString>{ "X", "Y", "Z", "XY", "YZ", "XZ", "XYZ", "ZYX" }) {
    UTDShapeMinimal Valid("Valid");
    Passed = EvaluateTrue("Scale()", Axes, "The axis string is accepted", Quiet([&]() { return Valid.MDShape::Scale(2.0, Axes); })) && Passed;
    Passed = EvaluateTrue("GetScalingAxis()", Axes, "The axis string is stored", Valid.GetScalingAxis() == Axes) && Passed;
    Passed = EvaluateTrue("IsScaled()", Axes, "The shape is marked as scaled", Valid.IsScaled()) && Passed;
    Passed = EvaluateNear("GetScaler()", Axes, "The scaler is stored", Valid.GetScaler(), 2.0, 1e-15) && Passed;
  }

  // Invalid axes:
  for (const MString& Axes : vector<MString>{ "A", "x", "X Y", "XW" }) {
    UTDShapeMinimal Invalid("Invalid");
    Passed = EvaluateFalse("Scale()", Axes, "The invalid axis string is rejected", Quiet([&]() { return Invalid.MDShape::Scale(2.0, Axes); })) && Passed;
    Passed = EvaluateFalse("IsScaled()", Axes, "The rejected scaling leaves the shape unscaled", Invalid.IsScaled()) && Passed;
    Passed = EvaluateNear("GetScaler()", Axes, "The rejected scaling leaves the scaler", Invalid.GetScaler(), 1.0, 1e-15) && Passed;
  }

  // A factor of one is no scaling:
  UTDShapeMinimal One("One");
  Passed = EvaluateTrue("Scale()", "factor 1", "A factor of 1 is accepted", Quiet([&]() { return One.MDShape::Scale(1.0, "XYZ"); })) && Passed;
  Passed = EvaluateFalse("IsScaled()", "factor 1", "A factor of 1 is not marked as scaled", One.IsScaled()) && Passed;
  Passed = EvaluateTrue("GetScalingAxis()", "factor 1", "A factor of 1 stores no axis", One.GetScalingAxis() == "") && Passed;

  // A second call is rejected:
  UTDShapeMinimal Twice("Twice");
  Passed = EvaluateTrue("Scale()", "first", "The first scaling is accepted", Quiet([&]() { return Twice.MDShape::Scale(2.0, "X"); })) && Passed;
  Passed = EvaluateFalse("Scale()", "second", "The second scaling is rejected", Quiet([&]() { return Twice.MDShape::Scale(3.0, "Y"); })) && Passed;
  Passed = EvaluateNear("GetScaler()", "second", "The second scaling keeps the first scaler", Twice.GetScaler(), 2.0, 1e-15) && Passed;
  Passed = EvaluateTrue("GetScalingAxis()", "second", "The second scaling keeps the first axis", Twice.GetScalingAxis() == "X") && Passed;

  // Defect GEO-C-5: The base class stores negative and zero factors without complaint
  UTDShapeMinimal Negative("Negative");
  Passed = ExpectedToFail(EvaluateFalse("Scale()", "factor -1", "A negative factor is rejected", Quiet([&]() { return Negative.MDShape::Scale(-1.0, "X"); }))) && Passed;

  // Defect GEO-C-11: An empty axis string is accepted and changes the scaler, but the shape is not scaled
  UTDShapeMinimal Empty("Empty");
  Quiet([&]() { return Empty.MDShape::Scale(2.0, ""); });
  Passed = ExpectedToFail(EvaluateNear("GetScaler()", "no axis", "Without an axis nothing is scaled and the scaler stays one", Empty.GetScaler(), 1.0, 1e-15)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  gErrorIgnoreLevel = kWarning;

  UTDShape Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDShape.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
