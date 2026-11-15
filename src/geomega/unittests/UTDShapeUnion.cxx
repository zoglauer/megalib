/*
 * UTDShapeUnion.cxx
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
#include <new>

// ROOT libs:
#include <TApplication.h>
#include <TError.h>
#include <TROOT.h>
using namespace std;

// MEGAlib libs:
#include "MDOrientation.h"
#include "MDShapeUnion.h"
#include "MDShapeBRIK.h"
#include "MDShapeSPHE.h"
#include "UTDShapeShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the boolean shape MDShapeUnion against set algebra
class UTDShapeUnion : public UTDShapeBase
{
public:
  //! Default constructor
  UTDShapeUnion() : UTDShapeBase("UTDShapeUnion") {}
  //! Default destructor
  virtual ~UTDShapeUnion() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the set algebra of a box A at the origin and a box B, rotated around z by Angle (deg) and shifted
  MTruth GetTruth(const MVector& A, const MVector& B, const MVector& Position, double Angle = 0);

  //! Test the setting and the getters
  bool TestSet();
  //! Test the error paths of the validation
  bool TestValidate();
  //! Test the volume
  bool TestVolume();
  //! Test the inside check
  bool TestIsInside();
  //! Test the distance functions with fixed rays
  bool TestDistances();
  //! Test the unique position
  bool TestUniquePosition();
  //! Test the geometry against set algebra
  bool TestAgainstTruth();
  //! Test the influence on the random numbers
  bool TestRandomStream();
  //! Test the string representations and the parsing
  bool TestStrings();
  //! Test the scaling
  bool TestScale();
  //! Test the shape read from a geometry file
  bool TestGeometryFile();
  //! Test configuring a validated shape again
  bool TestReconfiguration();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDShapeUnion::Run()
{
  bool Passed = true;

  Passed = TestSet() && Passed;
  Passed = TestValidate() && Passed;
  Passed = TestVolume() && Passed;
  Passed = TestIsInside() && Passed;
  Passed = TestDistances() && Passed;
  Passed = TestUniquePosition() && Passed;
  Passed = TestAgainstTruth() && Passed;
  Passed = TestRandomStream() && Passed;
  Passed = TestStrings() && Passed;
  Passed = TestScale() && Passed;
  Passed = TestGeometryFile() && Passed;

  Passed = TestReconfiguration() && Passed;
  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the set algebra of a box A at the origin and a box B, rotated around z by Angle (deg) and shifted
UTDShapeBase::MTruth UTDShapeUnion::GetTruth(const MVector& A, const MVector& B, const MVector& Position, double Angle)
{
  double C = cos(Angle*c_Rad);
  double Sn = sin(Angle*c_Rad);
  return [=](double x, double y, double z) {
    bool InA = false;
    if (fabs(x) <= A.X() && fabs(y) <= A.Y() && fabs(z) <= A.Z()) {
      InA = true;
    }
    double Dx = x - Position.X();
    double Dy = y - Position.Y();
    double Dz = z - Position.Z();
    bool InB = false;
    if (fabs(C*Dx + Sn*Dy) <= B.X() && fabs(-Sn*Dx + C*Dy) <= B.Y() && fabs(Dz) <= B.Z()) {
      InB = true;
    }
    if (InA == true || InB == true) {
      return true;
    }
    return false;
  };
}


////////////////////////////////////////////////////////////////////////////////


//! Test the setting and the getters
bool UTDShapeUnion::TestSet()
{
  bool Passed = true;

  MDShapeUnion Shape("Shape");
  Passed = EvaluateTrue("GetName()", "constructor", "The name of the constructor is stored", Shape.GetName() == "Shape") && Passed;
  Passed = EvaluateTrue("GetType()", "constructor", "The type is Union", Shape.GetType() == "Union") && Passed;
  Passed = Evaluate("GetNSubShapes()", "constructor", "There are two slots for sub shapes", Shape.GetNSubShapes(), 2U) && Passed;
  Passed = EvaluateTrue("GetSubShape()", "constructor", "The first sub shape is not set", Shape.GetSubShape(0) == nullptr) && Passed;
  Passed = EvaluateTrue("GetAugend()", "constructor", "The first shape is not set", Shape.GetAugend() == nullptr) && Passed;
  Passed = EvaluateTrue("GetAddend()", "constructor", "The second shape is not set", Shape.GetAddend() == nullptr) && Passed;
  Passed = EvaluateTrue("GetOrientationAddend()", "constructor", "The orientation is not set", Shape.GetOrientationAddend() == nullptr) && Passed;

  MDShapeBRIK A("A");
  MDShapeBRIK B("B");
  MDOrientation O("O");
  Passed = EvaluateTrue("Set()", "two shapes", "The shapes are accepted", Shape.Set(&A, &B, &O)) && Passed;
  Passed = EvaluateTrue("GetAugend()", "two shapes", "The first shape is stored", Shape.GetAugend() == &A) && Passed;
  Passed = EvaluateTrue("GetAddend()", "two shapes", "The second shape is stored", Shape.GetAddend() == &B) && Passed;
  Passed = EvaluateTrue("GetOrientationAddend()", "two shapes", "The orientation is stored", Shape.GetOrientationAddend() == &O) && Passed;
  Passed = EvaluateTrue("GetSubShape()", "index 0", "The first sub shape is the first shape", Shape.GetSubShape(0) == &A) && Passed;
  Passed = EvaluateTrue("GetSubShape()", "index n - 1", "The last sub shape is the second shape", Shape.GetSubShape(1) == &B) && Passed;
  Passed = EvaluateTrue("GetSubShape()", "index n", "The first index after the end returns nothing", Shape.GetSubShape(2) == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the error paths of the validation
bool UTDShapeUnion::TestValidate()
{
  bool Passed = true;

  MDShapeBRIK A("A");
  A.Set(2, 2, 2);
  MDShapeBRIK B("B");
  B.Set(1.5, 1.5, 1.5);
  MDOrientation O("O");
  O.SetPosition(MVector(2, 0.5, 0));
  O.Validate();

  MDShapeUnion NoFirst("NoFirst");
  NoFirst.Set(nullptr, &B, &O);
  Passed = EvaluateFalse("Validate()", "no first shape", "A missing first shape is rejected", Quiet([&]() { return NoFirst.Validate(); })) && Passed;

  MDShapeUnion NoSecond("NoSecond");
  NoSecond.Set(&A, nullptr, &O);
  Passed = EvaluateFalse("Validate()", "no second shape", "A missing second shape is rejected", Quiet([&]() { return NoSecond.Validate(); })) && Passed;

  MDShapeUnion Same("Same");
  Same.Set(&A, &A, &O);
  Passed = EvaluateFalse("Validate()", "same shapes", "Identical shapes are rejected", Quiet([&]() { return Same.Validate(); })) && Passed;

  // Defect GEO-C-35: The first shape is validated before the check for itself, the recursion overflows the stack
  MDShapeUnion SelfFirst("SelfFirst");
  SelfFirst.Set(&SelfFirst, &B, &O);
  Passed = ExpectedToCrash(EvaluateFalse("Validate()", "itself first", "The shape itself as first shape is rejected", SelfFirst.Validate())) && Passed;

  MDShapeUnion SelfSecond("SelfSecond");
  SelfSecond.Set(&A, &SelfSecond, &O);
  Passed = EvaluateFalse("Validate()", "itself second", "The shape itself as second shape is rejected", Quiet([&]() { return SelfSecond.Validate(); })) && Passed;

  MDShapeUnion NoOrientation("NoOrientation");
  NoOrientation.Set(&A, &B, nullptr);
  Passed = EvaluateFalse("Validate()", "no orientation", "A missing orientation is rejected", Quiet([&]() { return NoOrientation.Validate(); })) && Passed;

  // An invalid sub shape, which itself has missing shapes:
  MDShapeUnion Inner("Inner");
  MDShapeUnion Outer("Outer");
  Outer.Set(&Inner, &B, &O);
  Passed = EvaluateFalse("Validate()", "invalid sub shape", "An invalid sub shape is rejected", Quiet([&]() { return Outer.Validate(); })) && Passed;

  // Defect GEO-C-36: A cycle over two boolean shapes is not found, the recursion overflows the stack
  MDShapeUnion First("First");
  MDShapeUnion Second("Second");
  First.Set(&Second, &B, &O);
  Second.Set(&First, &B, &O);
  Passed = ExpectedToCrash(EvaluateFalse("Validate()", "cycle", "A cycle of shapes is rejected", First.Validate())) && Passed;

  // A valid one, repeated validation keeps the ROOT shape:
  MDShapeUnion Valid("Valid");
  Valid.Set(&A, &B, &O);
  Passed = EvaluateTrue("Validate()", "valid", "Valid shapes are accepted", Valid.Validate()) && Passed;
  TGeoShape* Root = Valid.GetRootShape();
  Passed = EvaluateTrue("GetRootShape()", "valid", "The ROOT shape exists", Root != nullptr) && Passed;
  Passed = EvaluateTrue("Validate()", "valid, again", "The second validation is accepted", Valid.Validate()) && Passed;
  Passed = EvaluateTrue("GetRootShape()", "valid, again", "An unchanged shape keeps the ROOT shape", Valid.GetRootShape() == Root) && Passed;

  // Defect GEO-C-34: The orientation has to be validated by someone else and has an uninitialized matrix before
  MDShapeBRIK ChildA("ChildA");
  ChildA.Set(2, 2, 2);
  MDShapeBRIK ChildB("ChildB");
  ChildB.Set(1, 1, 1);
  alignas(MDOrientation) unsigned char Block[sizeof(MDOrientation)];
  FillMemory(Block, sizeof(Block), 0xA7);
  MDOrientation* NotValidated = new (Block) MDOrientation("NotValidated");
  MDShapeUnion Shape("Shape");
  Shape.Set(&ChildA, &ChildB, NotValidated);
  Passed = ExpectedToCrash(EvaluateTrue("Validate()", "orientation not validated", "A not validated orientation does not crash the program", (Shape.Validate(), true))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the volume
bool UTDShapeUnion::TestVolume()
{
  bool Passed = true;

  // Box A has the volume 64, box B 27, the overlap x in [0.5, 2], y in [-1, 2], z in [-1.5, 1.5] has 13.5
  MDShapeBRIK A("A");
  A.Set(2, 2, 2);
  MDShapeBRIK B("B");
  B.Set(1.5, 1.5, 1.5);
  MDOrientation O("O");
  O.SetPosition(MVector(2, 0.5, 0));
  O.Validate();
  MDShapeUnion Shape("Shape");
  Shape.Set(&A, &B, &O);
  Shape.Validate();

  gRandom->SetSeed(1234);
  const double Volume = 64.0 + 27.0 - 13.5;
  const double BoxVolume = 5.5*4*4;
  const double Error = GetVolumeError(Volume, BoxVolume, 1);
  Passed = EvaluateNear("GetVolume()", "two boxes", "The sampled volume agrees with set algebra within 5 sigma of the sampling", Shape.GetVolume(), Volume, 5*Error*Volume + 1e-9) && Passed;

  // Defect GEO-C-25: The volume is sampled with 1% scatter, the mass of the same geometry changes per call
  double First = Shape.GetVolume();
  double Second = Shape.GetVolume();
  Passed = ExpectedToFail(EvaluateNear("GetVolume()", "two calls", "The volume of the same shape does not change", First, Second, 1e-9)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the inside check
bool UTDShapeUnion::TestIsInside()
{
  bool Passed = true;

  MDShapeBRIK A("A");
  A.Set(2, 2, 2);
  MDShapeBRIK B("B");
  B.Set(1.5, 1.5, 1.5);
  MDOrientation O("O");
  O.SetPosition(MVector(2, 0.5, 0));
  O.Validate();
  MDShapeUnion Shape("Shape");
  Shape.Set(&A, &B, &O);
  Shape.Validate();

  // Box A only, box B only, both, and none:
  Passed = EvaluateTrue("IsInside()", "only in the first shape", "A point only in the first shape is set by the operation", Shape.IsInside(MVector(-1, -1.5, 0.3))) && Passed;
  Passed = EvaluateTrue("IsInside()", "only in the second shape", "A point only in the second shape is set by the operation", Shape.IsInside(MVector(3, 1, -0.3))) && Passed;
  Passed = EvaluateTrue("IsInside()", "in both shapes", "A point in both shapes is set by the operation", Shape.IsInside(MVector(1, 1, 0.3))) && Passed;
  Passed = EvaluateFalse("IsInside()", "in no shape", "A point in no shape is outside", Shape.IsInside(MVector(-3, 3, 3))) && Passed;

  // The surfaces of the union: the free side of A at x = -2 and the free side of B at x = 3.5
  Passed = CheckToleranceAtSurface("A", Shape, MVector(-2, 0, 0), MVector(-1, 0, 0)) && Passed;
  Passed = CheckToleranceAtSurface("B", Shape, MVector(3.5, 0.5, 0), MVector(1, 0, 0)) && Passed;

  // No inner surface exists where A and B overlap:
  Passed = EvaluateTrue("IsInside()", "A surface in B", "The surface of A inside of B is not a surface", Shape.IsInside(MVector(2.001, 0.5, 0))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the distance functions with fixed rays
bool UTDShapeUnion::TestDistances()
{
  bool Passed = true;

  MDShapeBRIK A("A");
  A.Set(2, 2, 2);
  MDShapeBRIK B("B");
  B.Set(1.5, 1.5, 1.5);
  MDOrientation O("O");
  O.SetPosition(MVector(2, 0.5, 0));
  O.Validate();
  MDShapeUnion Shape("Shape");
  Shape.Set(&A, &B, &O);
  Shape.Validate();

  // From the middle of A to the far side of B along +x, and back to the free side of A:
  Passed = EvaluateNear("DistanceInsideOut()", "+x through both", "The ray ends at the far side of B", Shape.DistanceInsideOut(MVector(0, 0.5, 0), MVector(1, 0, 0)), 3.5, 1e-9) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "-x", "The ray ends at the free side of A", Shape.DistanceInsideOut(MVector(0, 0.5, 0), MVector(-1, 0, 0)), 2.0, 1e-9) && Passed;
  Passed = EvaluateNear("DistanceOutsideIn()", "from +x", "The ray starts at the far side of B", Shape.DistanceOutsideIn(MVector(6, 0.5, 0), MVector(-1, 0, 0)), 2.5, 1e-9) && Passed;
  Passed = EvaluateNear("DistanceOutsideIn()", "from -x", "The ray starts at the free side of A", Shape.DistanceOutsideIn(MVector(-6, 0.5, 0), MVector(1, 0, 0)), 4.0, 1e-9) && Passed;

  Passed = EvaluateTrue("DistanceOutsideIn()", "miss", "A ray away from the shape is no hit", Shape.DistanceOutsideIn(MVector(-5, 0, 0), MVector(-1, 0, 0)) > 1e20) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the unique position
bool UTDShapeUnion::TestUniquePosition()
{
  bool Passed = true;

  MDShapeBRIK A("A");
  A.Set(2, 2, 2);
  MDShapeBRIK B("B");
  B.Set(1.5, 1.5, 1.5);
  MDOrientation O("O");
  O.SetPosition(MVector(2, 0.5, 0));
  O.Validate();
  MDShapeUnion Shape("Shape");
  Shape.Set(&A, &B, &O);
  Shape.Validate();
  Passed = EvaluateTrue("GetUniquePosition()", "two boxes", "The position is inside", Shape.IsInside(Shape.GetUniquePosition())) && Passed;

  // The position does not depend on the random numbers:
  gRandom->SetSeed(777);
  gRandom->Rndm();
  MDShapeUnion Other("Other");
  Other.Set(&A, &B, &O);
  Other.Validate();
  Passed = EvaluateVectorNear("GetUniquePosition()", "other random state", "The position is the same for the same shape", Other.GetUniquePosition(), Shape.GetUniquePosition(), 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the geometry against set algebra
bool UTDShapeUnion::TestAgainstTruth()
{
  bool Passed = true;

  const MVector Min(-4, -4, -3);
  const MVector Max(5, 4, 3);

  // A shift only:
  MDShapeBRIK A("A");
  A.Set(2, 2, 2);
  MDShapeBRIK B("B");
  B.Set(1.5, 1.5, 1.5);
  MDOrientation O("O");
  O.SetPosition(MVector(2, 0.5, 0));
  O.Validate();
  MDShapeUnion Shape("Shape");
  Shape.Set(&A, &B, &O);
  Shape.Validate();
  MTruth Truth = GetTruth(MVector(2, 2, 2), MVector(1.5, 1.5, 1.5), MVector(2, 0.5, 0));
  Passed = CheckTruthVolume("shifted", Truth, Min, Max, 64.0 + 27.0 - 13.5) && Passed;
  Passed = CheckAgainstTruth("shifted", Shape, Truth, Min, Max) && Passed;

  // A rotation around z and a shift:
  MDShapeBRIK C("C");
  C.Set(2, 2, 2);
  MDShapeBRIK D("D");
  D.Set(3, 1, 1.5);
  MDOrientation R("R");
  R.SetPosition(MVector(1, 0.5, 0.5));
  R.SetRotation(0, 0, 30);
  R.Validate();
  MDShapeUnion Rotated("Rotated");
  Rotated.Set(&C, &D, &R);
  Rotated.Validate();
  MTruth TruthRotated = GetTruth(MVector(2, 2, 2), MVector(3, 1, 1.5), MVector(1, 0.5, 0.5), 30);
  Passed = CheckInsideAgainstTruth("rotated", Rotated, TruthRotated, Min, Max) && Passed;
  Passed = CheckRandomPositions("rotated", Rotated, TruthRotated, Min, Max) && Passed;

  // A nested boolean, the sub shape is the same operation again:
  MDShapeBRIK E("E");
  E.Set(2, 2, 2);
  MDShapeBRIK F("F");
  F.Set(1.5, 1.5, 1.5);
  MDShapeBRIK G("G");
  G.Set(1, 1, 1);
  MDOrientation P("P");
  P.SetPosition(MVector(2, 0.5, 0));
  P.Validate();
  MDOrientation Q("Q");
  Q.SetPosition(MVector(0, 0, 1));
  Q.Validate();
  MDShapeUnion Inner("Inner");
  Inner.Set(&E, &F, &P);
  MDShapeUnion Outer("Outer");
  Outer.Set(&Inner, &G, &Q);
  Passed = EvaluateTrue("Validate()", "nested", "A boolean as sub shape is accepted", Outer.Validate()) && Passed;
  MTruth TruthInner = GetTruth(MVector(2, 2, 2), MVector(1.5, 1.5, 1.5), MVector(2, 0.5, 0));
  MTruth TruthNested = [=](double x, double y, double z) {
    bool InA = TruthInner(x, y, z);
    bool InB = false;
    if (fabs(x) <= 1 && fabs(y) <= 1 && fabs(z - 1) <= 1) {
      InB = true;
    }
    if (InA == true || InB == true) {
      return true;
    }
    return false;
  };
  Passed = CheckInsideAgainstTruth("nested", Outer, TruthNested, Min, Max) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the influence on the random numbers
bool UTDShapeUnion::TestRandomStream()
{
  bool Passed = true;

  MDShapeBRIK A("A");
  A.Set(2, 2, 2);
  MDShapeBRIK B("B");
  B.Set(1.5, 1.5, 1.5);
  MDOrientation O("O");
  O.SetPosition(MVector(2, 0.5, 0));
  O.Validate();

  // The reference: the sixth number of a sequence
  gRandom->SetSeed(42);
  for (unsigned int i = 0; i < 5; ++i) {
    gRandom->Rndm();
  }
  double Expected = gRandom->Rndm();

  gRandom->SetSeed(42);
  for (unsigned int i = 0; i < 5; ++i) {
    gRandom->Rndm();
  }
  MDShapeUnion Shape("Shape");
  Shape.Set(&A, &B, &O);
  Shape.Validate();
  double AfterValidate = gRandom->Rndm();

  Passed = EvaluateNear("Validate()", "random stream", "The validation does not change the random numbers", AfterValidate, Expected, 1e-15) && Passed;

  // Defect GEO-C-25: The sampled volume uses the random numbers of the program, thus asking for the volume changes the following simulation
  gRandom->SetSeed(42);
  for (unsigned int i = 0; i < 5; ++i) {
    gRandom->Rndm();
  }
  Shape.GetVolume();
  double AfterVolume = gRandom->Rndm();
  Passed = ExpectedToFail(EvaluateNear("GetVolume()", "random stream", "The volume does not change the random numbers", AfterVolume, Expected, 1e-15)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the string representations and the parsing
bool UTDShapeUnion::TestStrings()
{
  bool Passed = true;

  MDShapeBRIK A("A");
  A.Set(2, 2, 2);
  MDShapeBRIK B("B");
  B.Set(1.5, 1.5, 1.5);
  MDOrientation O("O");
  O.SetPosition(MVector(2, 0.5, 0));
  O.Validate();
  MDShapeUnion Shape("Shape");
  Shape.Set(&A, &B, &O);
  Shape.Validate();

  Passed = EvaluateTrue("ToString()", "two boxes", "The string names the type", Shape.ToString().Contains("Union")) && Passed;

  // The boolean shapes are parsed by the geometry class, not by themselves
  Passed = EvaluateFalse("Parse()", "any line", "The shape cannot parse itself", ParseQuiet(Shape, "Shape.Parameters A B O")) && Passed;

  // Defect GEO-C-24: The text is empty, the volume writes a line with a shape keyword without any shape
  Passed = ExpectedToFail(EvaluateFalse("GetGeomega()", "two boxes", "The text is not empty", Shape.GetGeomega() == "")) && Passed;

  // Defect GEO-C-24: The size is always zero, the box A alone has a half length of 2
  Passed = ExpectedToFail(EvaluateTrue("GetSize()", "two boxes", "The surrounding box contains the first shape", Shape.GetSize().X() >= 2.0)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the scaling
bool UTDShapeUnion::TestScale()
{
  bool Passed = true;

  // Both shapes at the origin:
  {
    MDShapeBRIK A("A");
    A.Set(2, 2, 2);
    MDShapeBRIK B("B");
    B.Set(1, 3, 1);
    MDOrientation O("O");
    O.Validate();
    MDShapeUnion Shape("Shape");
    Shape.Set(&A, &B, &O);
    Shape.Validate();

    // The sampled volume of a second call scatters, 2 calls and 5 sigma, the box is the one of the operation
    const double Volume = 64.0 + 2*6*2 - 16.0;
    const double Box = 4.0*6*4;
    Passed = CheckScaleState("XYZ", Shape, 2.0, "XYZ", 5*sqrt(2.0)*GetVolumeError(Volume, Box, 1) + 1e-9) && Passed;
    Passed = EvaluateVectorNear("Scale()", "XYZ", "The first shape is scaled", A.GetSize(), MVector(4, 4, 4), 1e-12) && Passed;
    Passed = EvaluateVectorNear("Scale()", "XYZ", "The second shape is scaled", B.GetSize(), MVector(2, 6, 2), 1e-12) && Passed;
    Passed = EvaluateNear("GetVolume()", "XYZ", "The volume is scaled by the third power", Shape.GetVolume(), 8*Volume, 5*GetVolumeError(Volume, Box, 1)*8*Volume + 1e-9) && Passed;
    Passed = CheckInsideAgainstTruth("XYZ", Shape, GetTruth(MVector(4, 4, 4), MVector(2, 6, 2), MVector(0, 0, 0)), MVector(-5, -7, -5), MVector(5, 7, 5)) && Passed;
  }

  // Scaling only one axis of shapes at the origin:
  {
    MDShapeBRIK A("A");
    A.Set(2, 2, 2);
    MDShapeBRIK B("B");
    B.Set(1, 3, 1);
    MDOrientation O("O");
    O.Validate();
    MDShapeUnion Shape("Shape");
    Shape.Set(&A, &B, &O);
    Shape.Validate();
    Passed = EvaluateTrue("Scale()", "Z", "The axis Z is accepted", Shape.Scale(3.0, "Z")) && Passed;
    Passed = CheckInsideAgainstTruth("Z", Shape, GetTruth(MVector(2, 2, 6), MVector(1, 3, 3), MVector(0, 0, 0)), MVector(-3, -4, -7), MVector(3, 4, 7)) && Passed;
  }

  // Defect GEO-C-20: The position of the second shape is not scaled, the orientation only stores the factor
  {
    MDShapeBRIK A("A");
    A.Set(2, 2, 2);
    MDShapeBRIK B("B");
    B.Set(1.5, 1.5, 1.5);
    MDOrientation O("O");
    O.SetPosition(MVector(2, 0.5, 0));
    O.Validate();
    MDShapeUnion Shape("Shape");
    Shape.Set(&A, &B, &O);
    Shape.Validate();
    Quiet([&]() { return Shape.Scale(2.0, "XYZ"); });
    MTruth Scaled = GetTruth(MVector(4, 4, 4), MVector(3, 3, 3), MVector(4, 1, 0));
    Passed = ExpectedToFail(Evaluate("Scale()", "shifted second shape", "The shape agrees with the scaled set algebra", GetNumberOfInsideDifferences(Shape, Scaled, MVector(-8, -8, -6), MVector(10, 8, 6)), 0U)) && Passed;
  }

  // Defect GEO-C-19: A sub shape which cannot be scaled along this axis is ignored, the result is true and only one shape is scaled
  {
    MDShapeSPHE A("A");
    A.Set(0, 2);
    MDShapeBRIK B("B");
    B.Set(1, 1, 1);
    MDOrientation O("O");
    O.Validate();
    MDShapeUnion Shape("Shape");
    Shape.Set(&A, &B, &O);
    Shape.Validate();
    Passed = ExpectedToFail(EvaluateFalse("Scale()", "sphere along X", "The scaling fails if a sub shape cannot be scaled", Quiet([&]() { return Shape.Scale(2.0, "X"); }))) && Passed;
  }

  // An already scaled sub shape or orientation is rejected:
  {
    MDShapeBRIK A("A");
    A.Set(2, 2, 2);
    A.Validate();
    A.Scale(2.0, "XYZ");
    MDShapeBRIK B("B");
    B.Set(1, 1, 1);
    MDOrientation O("O");
    O.Validate();
    MDShapeUnion Shape("Shape");
    Shape.Set(&A, &B, &O);
    Shape.Validate();
    Passed = EvaluateFalse("Scale()", "first shape scaled", "An already scaled first shape is rejected", Quiet([&]() { return Shape.Scale(2.0, "XYZ"); })) && Passed;

    // Defect GEO-C-6: A rejected scaling leaves the shape marked as scaled
    Passed = ExpectedToFail(EvaluateFalse("IsScaled()", "after a rejected scaling", "The shape is not marked as scaled", Shape.IsScaled())) && Passed;
  }
  {
    MDShapeBRIK A("A");
    A.Set(2, 2, 2);
    MDShapeBRIK B("B");
    B.Set(0.5, 0.5, 0.5);
    B.Validate();
    B.Scale(2.0, "XYZ");
    MDOrientation O("O");
    O.Validate();
    MDShapeUnion Shape("Shape");
    Shape.Set(&A, &B, &O);
    Shape.Validate();
    Passed = EvaluateFalse("Scale()", "second shape scaled", "An already scaled second shape is rejected", Quiet([&]() { return Shape.Scale(2.0, "XYZ"); })) && Passed;
  }
  {
    MDShapeBRIK A("A");
    A.Set(2, 2, 2);
    MDShapeBRIK B("B");
    B.Set(1, 1, 1);
    MDOrientation O("O");
    O.Validate();
    O.Scale(2.0, "XYZ");
    MDShapeUnion Shape("Shape");
    Shape.Set(&A, &B, &O);
    Shape.Validate();
    Passed = EvaluateFalse("Scale()", "orientation scaled", "An already scaled orientation is rejected", Quiet([&]() { return Shape.Scale(2.0, "XYZ"); })) && Passed;
  }

  // Defect GEO-C-21: Scaling without sub shapes dereferences the empty slots
  MDShapeUnion Empty("Empty");
  Passed = ExpectedToCrash(EvaluateFalse("Scale()", "no sub shapes", "The scaling is rejected", Empty.Scale(2.0, "XYZ"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the shape read from a geometry file
bool UTDShapeUnion::TestGeometryFile()
{
  bool Passed = true;

  MString Content = GetGeometryHead();
  Content += "Shape BOX A\n";
  Content += "A.Parameters 2 2 2\n";
  Content += "Shape BOX B\n";
  Content += "B.Parameters 1.5 1.5 1.5\n";
  Content += "Orientation O\n";
  Content += "O.Position 2 0.5 0\n";
  Content += "Shape Union Combined\n";
  Content += "Combined.Parameters A B O\n";
  Content += "Volume V\n";
  Content += "V.Material Vacuum\n";
  Content += "V.Shape Combined\n";
  Content += "V.Position 0 0 0\n";
  Content += "V.Mother World\n";

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "boolean keyword", "The geometry can be read", ScanGeometry(Geometry, Content)) && Passed;
  MDShape* Shape = Geometry.GetShape("Combined");
  Passed = EvaluateTrue("GetShape()", "boolean keyword", "The shape exists", Shape != nullptr) && Passed;
  if (Shape != nullptr) {
    Passed = EvaluateTrue("GetType()", "boolean keyword", "The type is Union", Shape->GetType() == "Union") && Passed;
    Passed = Evaluate("GetSubShape()", "boolean keyword", "The sub shapes are the ones in the file", Shape->GetNSubShapes(), 2U) && Passed;
    Passed = EvaluateTrue("GetSubShape()", "boolean keyword", "The first sub shape is A", Shape->GetSubShape(0) == Geometry.GetShape("A")) && Passed;
    Passed = EvaluateTrue("GetSubShape()", "boolean keyword", "The second sub shape is B", Shape->GetSubShape(1) == Geometry.GetShape("B")) && Passed;
    Passed = Evaluate("IsInside()", "boolean keyword", "The shape follows set algebra", GetNumberOfInsideDifferences(*Shape, GetTruth(MVector(2, 2, 2), MVector(1.5, 1.5, 1.5), MVector(2, 0.5, 0)), MVector(-4, -4, -3), MVector(5, 4, 3)), 0U) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test configuring a validated shape again
bool UTDShapeUnion::TestReconfiguration()
{
  bool Passed = true;

  // Half lengths of the boxes which determine the distance from the origin along z: 1, then 2
  MDShapeBRIK A("A");
  A.Set(1.0, 1.0, 1.0);
  A.Validate();
  MDShapeBRIK Large("Large");
  Large.Set(2.0, 2.0, 2.0);
  Large.Validate();
  MDShapeBRIK B("B");
  B.Set(0.5, 0.5, 0.5);
  B.Validate();
  MDOrientation O("O");
  O.Validate();
  MDShapeUnion Shape("S");
  Passed = CheckReconfiguration("another shape", Shape, MVector(0.0, 0.0, 1.0), [&]() { return Shape.Set(&A, &B, &O); }, 1.0, [&]() { return Shape.Set(&Large, &B, &O); }, 2.0) && Passed;

  // Defect GEO-C-39: A sub shape which is configured again after the boolean shape has been validated deletes its ROOT shape, which the boolean shape still uses
  Passed = ExpectedToCrash(EvaluateTrue("Validate()", "changed sub shape", "The boolean shape follows a sub shape which was configured again", Large.Set(3.0, 3.0, 3.0) == true && Large.Validate() == true && Shape.Validate() == true && fabs(Shape.DistanceInsideOut(MVector(0.0, 0.0, 0.0), MVector(0.0, 0.0, 1.0)) - 3.0) < 1e-9)) && Passed;

  // Another orientation of the second shape after the validation
  MDOrientation Moved("Moved");
  Moved.SetPosition(MVector(0.0, 0.0, 1.0));
  Moved.Validate();
  Passed = EvaluateTrue("Set()", "changed orientation", "A validated shape can get another orientation", Shape.Set(&A, &B, &Moved)) && Passed;
  Passed = EvaluateTrue("Validate()", "changed orientation", "The new orientation is valid", Shape.Validate()) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "changed orientation", "The ROOT shape follows the new orientation", Shape.DistanceInsideOut(MVector(0.0, 0.0, 0.0), MVector(0.0, 0.0, 1.0)), 1.5, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  gErrorIgnoreLevel = kWarning;

  UTDShapeUnion Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDShapeUnion.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
