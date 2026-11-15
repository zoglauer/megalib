/*
 * UTDOrientation.cxx
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
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TGeoMatrix.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MDDebugInfo.h"
#include "MDGeometry.h"
#include "MDOrientation.h"
#include "MDVolume.h"
#include "MRotation.h"
#include "MTokenizer.h"
#include "UTGeomegaShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the named position and rotation MDOrientation
//! Attention: The declared copy constructor has no definition and cannot be linked, thus it is not tested
class UTDOrientation : public UTGeomegaBase
{
public:
  //! Default constructor
  UTDOrientation() : UTGeomegaBase("UTDOrientation") {}
  //! Default destructor
  virtual ~UTDOrientation() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Transform from the local to the master system with the ROOT matrix
  MVector LocalToMaster(MDOrientation& Orientation, const MVector& Local) const;
  //! Parse one line of text
  bool ParseLine(MDOrientation& Orientation, const MString& Line);

  //! Test the constructor and the name
  bool TestDefaults();
  //! Test that the constructor initializes all members
  bool TestInitialization();
  //! Test the position
  bool TestPosition();
  //! Test the rotation by three angles
  bool TestRotationAngles();
  //! Test the rotation in Geant3 notation
  bool TestRotationGeant3();
  //! Test the rotation by nine elements
  bool TestRotationElements();
  //! Test the rotation by matrix
  bool TestRotationMatrix();
  //! Test the ROOT matrix
  bool TestValidate();
  //! Test the parser
  bool TestParse();
  //! Test the scaling
  bool TestScale();
  //! Test the scaling with a boolean shape in a geometry
  bool TestScaleInGeometry();
  //! Test the use in volumes in a geometry
  bool TestUseInGeometry();
  //! Test the text output
  bool TestToString();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDOrientation::Run()
{
  bool Passed = true;

  Passed = TestDefaults() && Passed;
  Passed = TestInitialization() && Passed;
  Passed = TestPosition() && Passed;
  Passed = TestRotationAngles() && Passed;
  Passed = TestRotationGeant3() && Passed;
  Passed = TestRotationElements() && Passed;
  Passed = TestRotationMatrix() && Passed;
  Passed = TestValidate() && Passed;
  Passed = TestParse() && Passed;
  Passed = TestScale() && Passed;
  Passed = TestScaleInGeometry() && Passed;
  Passed = TestUseInGeometry() && Passed;
  Passed = TestToString() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Transform from the local to the master system with the ROOT matrix
MVector UTDOrientation::LocalToMaster(MDOrientation& Orientation, const MVector& Local) const
{
  double In[3] = { Local.X(), Local.Y(), Local.Z() };
  double Out[3] = { 0.0, 0.0, 0.0 };
  Orientation.GetRootMatrix()->LocalToMaster(In, Out);
  return MVector(Out[0], Out[1], Out[2]);
}


////////////////////////////////////////////////////////////////////////////////


//! Parse one line of text
bool UTDOrientation::ParseLine(MDOrientation& Orientation, const MString& Line)
{
  MTokenizer Tokenizer(Line, ' ', true);
  MDDebugInfo Info(Line, "Test", 1);
  DisableDefaultStreams();
  bool Result = Orientation.Parse(Tokenizer, Info);
  EnableDefaultStreams();
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the constructor and the name
bool UTDOrientation::TestDefaults()
{
  bool Passed = true;

  MDOrientation O("Name");
  Passed = EvaluateTrue("GetName()", "constructor argument", "The name of the constructor is stored", O.GetName() == "Name") && Passed;
  O.SetName("Other");
  Passed = EvaluateTrue("SetName()", "new name", "The new name replaces the old one", O.GetName() == "Other") && Passed;
  Passed = EvaluateVectorNear("GetPosition()", "new", "The position starts at the origin", O.GetPosition(), MVector(0, 0, 0), 1e-15) && Passed;
  Passed = EvaluateFalse("IsRotated()", "new", "A new orientation is not rotated", O.IsRotated()) && Passed;
  Passed = EvaluateRotationNear("GetRotationMatrix()", "new", "The rotation is the identity", O.GetRotationMatrix(), MRotation(), 1e-15) && Passed;
  Passed = EvaluateRotationNear("GetInvRotationMatrix()", "new", "The inverse rotation is the identity", O.GetInvRotationMatrix(), MRotation(), 1e-15) && Passed;
  Passed = EvaluateFalse("IsScaled()", "new", "A new orientation is not scaled", O.IsScaled()) && Passed;
  Passed = EvaluateTrue("GetScalingAxis()", "new", "A new orientation has no scaling axes", O.GetScalingAxis() == "") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the constructor initializes all members
bool UTDOrientation::TestInitialization()
{
  bool Passed = true;

  // Fill the memory with a pattern which cannot be a valid default
  alignas(MDOrientation) unsigned char Block[sizeof(MDOrientation)];
  for (unsigned int b = 0; b < sizeof(MDOrientation); ++b) {
    Block[b] = 0x5A;
  }
  MDOrientation* O = new (Block) MDOrientation("Placed");

  // Defect GEO-B-23: The constructor does not initialize the scaler
  Passed = ExpectedToFail(EvaluateNear("MDOrientation()", "memory filled with a pattern", "The scaler starts at 1 like in the shapes", O->GetScaler(), 1.0, 1e-15)) && Passed;
  // Defect GEO-B-24: The constructor does not initialize the ROOT matrix
  Passed = ExpectedToFail(EvaluateTrue("MDOrientation()", "memory filled with a pattern", "The ROOT matrix is zero before the validation", O->GetRootMatrix() == nullptr)) && Passed;

  O->~MDOrientation();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the position
bool UTDOrientation::TestPosition()
{
  bool Passed = true;

  MDOrientation O("O");
  O.SetPosition(MVector(1.5, -2.5, 3.5));
  Passed = EvaluateVectorNear("SetPosition()", "interior value", "The position is stored", O.GetPosition(), MVector(1.5, -2.5, 3.5), 1e-15) && Passed;
  O.SetPosition(MVector(0, 0, 0));
  Passed = EvaluateVectorNear("SetPosition()", "zero", "The position is overwritten", O.GetPosition(), MVector(0, 0, 0), 1e-15) && Passed;
  Passed = EvaluateFalse("SetPosition()", "zero", "The position does not rotate", O.IsRotated()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the rotation by three angles
bool UTDOrientation::TestRotationAngles()
{
  bool Passed = true;

  const MVector X(1, 0, 0);
  const MVector Y(0, 1, 0);
  const MVector Z(0, 0, 1);

  // Active rotations: The local axes are rotated counterclockwise in the mother system
  MDOrientation Rz("Rz");
  Rz.SetRotation(0.0, 0.0, 90.0);
  Passed = EvaluateTrue("SetRotation()", "x, y, z", "The orientation is rotated", Rz.IsRotated()) && Passed;
  Passed = EvaluateVectorNear("SetRotation()", "90 deg around z", "The local x axis is y in the mother system", Rz.GetInvRotationMatrix()*X, Y, 1e-14) && Passed;
  Passed = EvaluateVectorNear("SetRotation()", "90 deg around z", "The y axis of the mother is the local x axis", Rz.GetRotationMatrix()*Y, X, 1e-14) && Passed;

  MDOrientation Rx("Rx");
  Rx.SetRotation(90.0, 0.0, 0.0);
  Passed = EvaluateVectorNear("SetRotation()", "90 deg around x", "The local y axis is z in the mother system", Rx.GetInvRotationMatrix()*Y, Z, 1e-14) && Passed;

  MDOrientation Ry("Ry");
  Ry.SetRotation(0.0, 90.0, 0.0);
  Passed = EvaluateVectorNear("SetRotation()", "90 deg around y", "The local z axis is x in the mother system", Ry.GetInvRotationMatrix()*Z, X, 1e-14) && Passed;

  // Interior angles and invariants:
  MDOrientation Mixed("Mixed");
  Mixed.SetRotation(30.0, 40.0, 50.0);
  const MVector Test(0.3, -1.7, 2.9);
  Passed = EvaluateVectorNear("SetRotation()", "30, 40, 50 deg", "The rotation is Rz*Ry*Rx, derived by hand", Mixed.GetInvRotationMatrix()*Test, RotateActive(Test, 30.0, 40.0, 50.0), 1e-13) && Passed;
  Passed = EvaluateRotationNear("GetInvRotationMatrix()", "30, 40, 50 deg", "The product with the inverse is the identity", Mixed.GetRotationMatrix()*Mixed.GetInvRotationMatrix(), MRotation(), 1e-14) && Passed;
  Passed = EvaluateNear("GetRotationMatrix()", "30, 40, 50 deg", "The determinant is +1", Mixed.GetRotationMatrix().GetDeterminant(), 1.0, 1e-14) && Passed;
  Passed = EvaluateNear("GetRotationMatrix()", "30, 40, 50 deg", "The length is preserved", (Mixed.GetRotationMatrix()*Test).Mag(), Test.Mag(), 1e-14) && Passed;

  // Zero angles still flag the rotation:
  MDOrientation Zero("Zero");
  Zero.SetRotation(0.0, 0.0, 0.0);
  Passed = EvaluateRotationNear("SetRotation()", "zero angles", "The rotation is the identity", Zero.GetRotationMatrix(), MRotation(), 1e-15) && Passed;

  // Reuse:
  Mixed.SetRotation(0.0, 0.0, 90.0);
  Passed = EvaluateRotationNear("SetRotation()", "second rotation", "A second rotation replaces the first", Mixed.GetRotationMatrix(), Rz.GetRotationMatrix(), 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the rotation in Geant3 notation
bool UTDOrientation::TestRotationGeant3()
{
  bool Passed = true;

  const MVector X(1, 0, 0);
  const MVector Y(0, 1, 0);

  MDOrientation Rz("Rz");
  Rz.SetRotation(0.0, 0.0, 90.0);

  // The new x axis points to +y, the new y axis to -x, the new z axis to +z:
  MDOrientation Euler("Euler");
  Euler.SetRotation(90.0, 90.0, 90.0, 180.0, 0.0, 0.0);
  Passed = EvaluateTrue("SetRotation()", "Geant3 angles", "The orientation is rotated", Euler.IsRotated()) && Passed;
  Passed = EvaluateVectorNear("SetRotation()", "Geant3 angles", "The local x axis is y in the mother system", Euler.GetInvRotationMatrix()*X, Y, 1e-14) && Passed;
  Passed = EvaluateRotationNear("SetRotation()", "Geant3 angles", "The result is the same as 90 deg around z", Euler.GetRotationMatrix(), Rz.GetRotationMatrix(), 1e-14) && Passed;

  // Refused: Reflection
  MDOrientation Reflected("Reflected");
  DisableDefaultStreams();
  Reflected.SetRotation(90.0, 0.0, 90.0, 90.0, 180.0, 0.0);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetRotation()", "reflection", "A reflection is ignored", Reflected.IsRotated()) && Passed;
  Passed = EvaluateRotationNear("SetRotation()", "reflection", "The matrix stays the identity", Reflected.GetRotationMatrix(), MRotation(), 1e-15) && Passed;

  // Refused: Axes which are not orthogonal
  MDOrientation NotOrthogonal("NotOrthogonal");
  DisableDefaultStreams();
  NotOrthogonal.SetRotation(90.0, 0.0, 90.0, 80.0, 0.0, 0.0);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetRotation()", "10 deg from orthogonal", "Axes which are not orthogonal are ignored", NotOrthogonal.IsRotated()) && Passed;

  // The threshold of the check: cos < 1e-6 corresponds to 5.73e-5 deg from orthogonal
  MDOrientation Inside("Inside");
  DisableDefaultStreams();
  Inside.SetRotation(90.0, 0.0, 90.0, 90.0 + 5.0E-5, 0.0, 0.0);
  EnableDefaultStreams();
  Passed = EvaluateTrue("SetRotation()", "5.0e-5 deg from orthogonal", "Just inside the threshold of cos = 1e-6 the rotation is accepted", Inside.IsRotated()) && Passed;
  MDOrientation Outside("Outside");
  DisableDefaultStreams();
  Outside.SetRotation(90.0, 0.0, 90.0, 90.0 + 7.0E-5, 0.0, 0.0);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetRotation()", "7.0e-5 deg from orthogonal", "Just outside the threshold of cos = 1e-6 the rotation is ignored", Outside.IsRotated()) && Passed;

  // A refused rotation keeps an existing one:
  MDOrientation Keep("Keep");
  Keep.SetRotation(0.0, 0.0, 90.0);
  DisableDefaultStreams();
  Keep.SetRotation(90.0, 0.0, 90.0, 80.0, 0.0, 0.0);
  EnableDefaultStreams();
  Passed = EvaluateRotationNear("SetRotation()", "refused second rotation", "The first rotation stays", Keep.GetRotationMatrix(), Rz.GetRotationMatrix(), 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the rotation by nine elements
bool UTDOrientation::TestRotationElements()
{
  bool Passed = true;

  // The elements are x{row number}{column number} in the order of the arguments: x11, x21, x31, x12, ...
  // Matrix with the rows (0, 1, 0), (-1, 0, 0), (0, 0, 1):
  MDOrientation O("O");
  O.SetRotation(0.0, -1.0, 0.0,
                1.0,  0.0, 0.0,
                0.0,  0.0, 1.0);
  Passed = EvaluateTrue("SetRotation()", "nine elements", "The orientation is rotated", O.IsRotated()) && Passed;
  Passed = EvaluateVectorNear("SetRotation()", "nine elements", "The first row is (0, 1, 0)", O.GetRotationMatrix()*MVector(0, 1, 0), MVector(1, 0, 0), 1e-15) && Passed;
  Passed = EvaluateVectorNear("SetRotation()", "nine elements", "The second row is (-1, 0, 0)", O.GetRotationMatrix()*MVector(1, 0, 0), MVector(0, -1, 0), 1e-15) && Passed;
  Passed = EvaluateRotationNear("SetRotation()", "nine elements", "The inverse is stored", O.GetRotationMatrix()*O.GetInvRotationMatrix(), MRotation(), 1e-15) && Passed;

  // The zero matrix cannot be inverted, the identity is used:
  MDOrientation Singular("Singular");
  DisableDefaultStreams();
  Singular.SetRotation(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  EnableDefaultStreams();
  Passed = EvaluateRotationNear("SetRotation()", "singular matrix", "The matrix falls back to the identity", Singular.GetRotationMatrix(), MRotation(), 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the rotation by matrix
bool UTDOrientation::TestRotationMatrix()
{
  bool Passed = true;

  MDOrientation Reference("Reference");
  Reference.SetRotation(30.0, 40.0, 50.0);

  MDOrientation O("O");
  O.SetRotation(Reference.GetRotationMatrix());
  Passed = EvaluateTrue("SetRotation()", "matrix", "The orientation is rotated", O.IsRotated()) && Passed;
  Passed = EvaluateRotationNear("SetRotation()", "matrix", "The matrix is the given one", O.GetRotationMatrix(), Reference.GetRotationMatrix(), 1e-14) && Passed;
  Passed = EvaluateRotationNear("SetRotation()", "matrix", "The inverse is the inverse of the given one", O.GetInvRotationMatrix(), Reference.GetInvRotationMatrix(), 1e-14) && Passed;

  MDOrientation WithID("WithID");
  WithID.SetRotation(Reference.GetRotationMatrix(), 7);
  Passed = EvaluateTrue("SetRotation()", "matrix and ID", "The orientation is rotated", WithID.IsRotated()) && Passed;
  Passed = EvaluateRotationNear("SetRotation()", "matrix and ID", "The matrix is the given one", WithID.GetRotationMatrix(), Reference.GetRotationMatrix(), 1e-14) && Passed;
  Passed = EvaluateRotationNear("SetRotation()", "matrix and ID", "The inverse is the inverse of the given one", WithID.GetInvRotationMatrix(), Reference.GetInvRotationMatrix(), 1e-14) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the ROOT matrix
bool UTDOrientation::TestValidate()
{
  bool Passed = true;

  const MVector Local(0.3, -1.7, 2.9);

  // Identity:
  MDOrientation Plain("Plain");
  Passed = EvaluateTrue("Validate()", "new", "The validation succeeds", Plain.Validate()) && Passed;
  Passed = EvaluateTrue("GetRootMatrix()", "new", "The matrix exists after the validation", Plain.GetRootMatrix() != nullptr) && Passed;
  Passed = EvaluateVectorNear("GetRootMatrix()", "new", "The matrix is the identity", LocalToMaster(Plain, Local), Local, 1e-14) && Passed;

  // Translation:
  MDOrientation Shifted("Shifted");
  Shifted.SetPosition(MVector(1, 2, 3));
  Shifted.Validate();
  Passed = EvaluateVectorNear("GetRootMatrix()", "translation", "The matrix translates", LocalToMaster(Shifted, Local), Local + MVector(1, 2, 3), 1e-14) && Passed;

  // Each kind of rotation with translation: Master = position + rotated local position
  const MVector Expected = MVector(1, 2, 3) + RotateActive(Local, 30.0, 40.0, 50.0);

  MDOrientation Angles("Angles");
  Angles.SetPosition(MVector(1, 2, 3));
  Angles.SetRotation(30.0, 40.0, 50.0);
  Angles.Validate();
  Passed = EvaluateVectorNear("GetRootMatrix()", "three angles", "ROOT uses the same convention as geomega", LocalToMaster(Angles, Local), Expected, 1e-12) && Passed;

  MDOrientation Matrix("Matrix");
  Matrix.SetPosition(MVector(1, 2, 3));
  Matrix.SetRotation(Angles.GetRotationMatrix());
  Matrix.Validate();
  Passed = EvaluateVectorNear("GetRootMatrix()", "matrix", "ROOT uses the same convention as geomega", LocalToMaster(Matrix, Local), Expected, 1e-12) && Passed;

  MDOrientation WithID("WithID");
  WithID.SetPosition(MVector(1, 2, 3));
  WithID.SetRotation(Angles.GetRotationMatrix(), 5);
  WithID.Validate();
  Passed = EvaluateVectorNear("GetRootMatrix()", "matrix with ID", "ROOT uses the same convention as geomega", LocalToMaster(WithID, Local), Expected, 1e-12) && Passed;

  MDOrientation Geant3("Geant3");
  Geant3.SetPosition(MVector(1, 2, 3));
  Geant3.SetRotation(90.0, 90.0, 90.0, 180.0, 0.0, 0.0);
  Geant3.Validate();
  Passed = EvaluateVectorNear("GetRootMatrix()", "Geant3 angles", "ROOT uses the same convention as geomega", LocalToMaster(Geant3, MVector(1, 0, 0)), MVector(1, 3, 3), 1e-12) && Passed;

  // A second validation sees the changes:
  Shifted.SetPosition(MVector(-1, -2, -3));
  Shifted.Validate();
  Passed = EvaluateVectorNear("GetRootMatrix()", "second validation", "The matrix is updated", LocalToMaster(Shifted, Local), Local - MVector(1, 2, 3), 1e-14) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the parser
bool UTDOrientation::TestParse()
{
  bool Passed = true;

  MDOrientation O("O");
  Passed = EvaluateTrue("Parse()", "Position", "A position with three values is accepted", ParseLine(O, "O.Position 1.0 2.0 3.0")) && Passed;
  Passed = EvaluateVectorNear("Parse()", "Position", "The position is stored", O.GetPosition(), MVector(1, 2, 3), 1e-15) && Passed;
  Passed = EvaluateTrue("Parse()", "Translation", "The second keyword for a position is accepted", ParseLine(O, "O.Translation -1.0 -2.0 -3.0")) && Passed;
  Passed = EvaluateVectorNear("Parse()", "Translation", "The position is stored", O.GetPosition(), MVector(-1, -2, -3), 1e-15) && Passed;
  Passed = EvaluateFalse("Parse()", "Position, n - 1 values", "Two values are refused", ParseLine(O, "O.Position 4.0 5.0")) && Passed;
  Passed = EvaluateFalse("Parse()", "Position, n + 1 values", "Four values are refused", ParseLine(O, "O.Position 4.0 5.0 6.0 7.0")) && Passed;
  Passed = EvaluateVectorNear("Parse()", "wrong positions", "A wrong line does not change the position", O.GetPosition(), MVector(-1, -2, -3), 1e-15) && Passed;

  MDOrientation Rz("Rz");
  Rz.SetRotation(0.0, 0.0, 90.0);

  MDOrientation R1("R1");
  Passed = EvaluateTrue("Parse()", "Rotation, three", "A rotation with three values is accepted", ParseLine(R1, "R1.Rotation 0.0 0.0 90.0")) && Passed;
  Passed = EvaluateRotationNear("Parse()", "Rotation, three", "The rotation is stored", R1.GetRotationMatrix(), Rz.GetRotationMatrix(), 1e-14) && Passed;

  MDOrientation R2("R2");
  Passed = EvaluateTrue("Parse()", "Rotate, three", "The second keyword for a rotation is accepted", ParseLine(R2, "R2.Rotate 0.0 0.0 90.0")) && Passed;
  Passed = EvaluateRotationNear("Parse()", "Rotate, three", "The rotation is stored", R2.GetRotationMatrix(), Rz.GetRotationMatrix(), 1e-14) && Passed;

  MDOrientation R3("R3");
  Passed = EvaluateTrue("Parse()", "Rotation, six", "A rotation with six values is accepted", ParseLine(R3, "R3.Rotation 90.0 90.0 90.0 180.0 0.0 0.0")) && Passed;
  Passed = EvaluateRotationNear("Parse()", "Rotation, six", "The rotation is stored", R3.GetRotationMatrix(), Rz.GetRotationMatrix(), 1e-14) && Passed;

  MDOrientation R4("R4");
  Passed = EvaluateFalse("Parse()", "Rotation, four", "Four values are refused", ParseLine(R4, "R4.Rotation 1.0 2.0 3.0 4.0")) && Passed;
  Passed = EvaluateFalse("Parse()", "Rotation, two", "Two values are refused", ParseLine(R4, "R4.Rotation 1.0 2.0")) && Passed;
  Passed = EvaluateFalse("Parse()", "wrong rotations", "A wrong line does not rotate", R4.IsRotated()) && Passed;

  // The volume parser refuses this text and stops:
  MDOrientation R5("R5");
  bool Reflection = ParseLine(R5, "R5.Rotation 90.0 0.0 90.0 90.0 180.0 0.0");
  Passed = EvaluateFalse("Parse()", "Rotation, reflection", "The rotation is not applied", R5.IsRotated()) && Passed;
  // Defect GEO-B-25: Parse reports success for Geant3 angles which SetRotation had to ignore
  Passed = ExpectedToFail(EvaluateFalse("Parse()", "Rotation, reflection", "A refused rotation is reported as an error", Reflection)) && Passed;

  Passed = EvaluateFalse("Parse()", "unknown keyword", "An unknown keyword is refused", ParseLine(O, "O.Unknown 1.0 2.0 3.0")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the scaling
bool UTDOrientation::TestScale()
{
  bool Passed = true;

  MDOrientation O("O");
  O.SetPosition(MVector(1, 2, 3));

  // Invalid axes:
  DisableDefaultStreams();
  bool BadAxis = O.Scale(2.0, "Q");
  bool LowerCase = O.Scale(2.0, "x");
  EnableDefaultStreams();
  Passed = EvaluateFalse("Scale()", "unknown axis", "An unknown axis is refused", BadAxis) && Passed;
  Passed = EvaluateFalse("Scale()", "lower case axis", "A lower case axis is refused", LowerCase) && Passed;
  Passed = EvaluateFalse("Scale()", "refused", "A refused scaling does not scale", O.IsScaled()) && Passed;

  // Factor 1:
  Passed = EvaluateTrue("Scale()", "factor 1", "Factor 1 is accepted", O.Scale(1.0, "XYZ")) && Passed;
  Passed = EvaluateFalse("Scale()", "factor 1", "Factor 1 does not scale", O.IsScaled()) && Passed;

  // Good one:
  Passed = EvaluateTrue("Scale()", "factor 2, XY", "The scaling is accepted", O.Scale(2.0, "XY")) && Passed;
  Passed = EvaluateTrue("IsScaled()", "factor 2, XY", "The orientation is scaled", O.IsScaled()) && Passed;
  Passed = EvaluateNear("GetScaler()", "factor 2, XY", "The factor is stored", O.GetScaler(), 2.0, 1e-15) && Passed;
  Passed = EvaluateTrue("GetScalingAxis()", "factor 2, XY", "The axes are stored", O.GetScalingAxis() == "XY") && Passed;

  // Twice:
  DisableDefaultStreams();
  bool Twice = O.Scale(3.0, "XYZ");
  EnableDefaultStreams();
  Passed = EvaluateFalse("Scale()", "second scaling", "A second scaling is refused", Twice) && Passed;
  Passed = EvaluateNear("GetScaler()", "second scaling", "The old factor stays", O.GetScaler(), 2.0, 1e-15) && Passed;

  // Rotated volume:
  MDOrientation Rotated("Rotated");
  Rotated.SetRotation(0.0, 0.0, 45.0);
  DisableDefaultStreams();
  bool OneAxis = Rotated.Scale(2.0, "X");
  EnableDefaultStreams();
  Passed = EvaluateFalse("Scale()", "rotated, X", "A rotated orientation cannot be scaled along one axis", OneAxis) && Passed;
  Passed = EvaluateFalse("Scale()", "rotated, X", "The orientation stays unscaled", Rotated.IsScaled()) && Passed;
  Passed = EvaluateTrue("Scale()", "rotated, XYZ", "A rotated orientation can be scaled along all axes", Rotated.Scale(2.0, "XYZ")) && Passed;

  // The position has to follow the scaling or the sub-shapes of a boolean shape do not fit anymore
  MDOrientation Shifted("Shifted");
  Shifted.SetPosition(MVector(1, 2, 3));
  Shifted.Scale(2.0, "XYZ");
  Shifted.Validate();
  // Defect GEO-B-26: Scale only stores the factor, position and ROOT matrix are not scaled
  Passed = ExpectedToFail(EvaluateVectorNear("Scale()", "factor 2, XYZ", "The translation of the ROOT matrix is scaled", LocalToMaster(Shifted, MVector(0, 0, 0)), MVector(2, 4, 6), 1e-14)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the scaling with a boolean shape in a geometry
bool UTDOrientation::TestScaleInGeometry()
{
  bool Passed = true;

  MString Text = GetGeometryHead();
  Text += "Shape BRIK First\n";
  Text += "First.Parameters 1.0 1.0 1.0\n";
  Text += "Shape BRIK Second\n";
  Text += "Second.Parameters 1.0 1.0 1.0\n";
  Text += "Orientation Offset\n";
  Text += "Offset.Position 3.0 0.0 0.0\n";
  Text += "Shape Union Both\n";
  Text += "Both.Parameters First Second Offset\n";
  Text += "Volume Scaled\n";
  Text += "Scaled.Material Aluminium\n";
  Text += "Scaled.Shape Both\n";
  Text += "Scaled.Scale 2.0\n";
  Text += "Scaled.Mother World\n";

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "scaled union", "The geometry can be read", ScanGeometry(Geometry, Text)) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  // After scaling by 2 the first box spans x = -2 ... 2 and the second box x = 4 ... 8
  MDVolume* Scaled = Geometry.GetVolume("Scaled");
  Passed = EvaluateTrue("IsInternalPositionInside()", "first box", "The first box is scaled", Scaled->IsInternalPositionInside(MVector(1.5, 1.5, 1.5))) && Passed;
  // Defect GEO-B-26: Scale only stores the factor, position and ROOT matrix are not scaled
  Passed = ExpectedToFail(EvaluateTrue("IsInternalPositionInside()", "second box", "The second box moved with the scaling to x = 4 ... 8", Scaled->IsInternalPositionInside(MVector(7.5, 0, 0)))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the use in volumes in a geometry
bool UTDOrientation::TestUseInGeometry()
{
  bool Passed = true;

  MString Text = GetGeometryHead();
  Text += "Orientation Place\n";
  Text += "Place.Position 1.0 2.0 3.0\n";
  Text += "Place.Rotation 0.0 0.0 90.0\n";
  Text += "Volume Box\n";
  Text += "Box.Material Aluminium\n";
  Text += "Box.Shape BRIK 2.0 3.0 4.0\n";
  Text += "Box.Orientation Place\n";
  Text += "Box.Mother World\n";

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "orientation of a volume", "The geometry can be read", ScanGeometry(Geometry, Text)) && Passed;
  if (Geometry.IsScanned() == true) {
    MDVolume* Box = Geometry.GetVolume("Box");
    MDOrientation* Place = Geometry.GetOrientation("Place");
    Passed = EvaluateTrue("GetOrientation()", "Place", "The orientation is found", Place != nullptr) && Passed;
    Passed = EvaluateVectorNear("GetPosition()", "orientation of a volume", "The volume has the position of the orientation", Box->GetPosition(), MVector(1, 2, 3), 1e-14) && Passed;
    Passed = EvaluateRotationNear("GetRotationMatrix()", "orientation of a volume", "The volume has the rotation of the orientation", Box->GetRotationMatrix(), Place->GetRotationMatrix(), 1e-14) && Passed;
    Passed = EvaluateVectorNear("GetPositionInMotherVolume()", "orientation of a volume", "The local x axis points to y", Box->GetPositionInMotherVolume(MVector(1, 0, 0)), MVector(1, 3, 3), 1e-14) && Passed;
  }

  // The orientation is defined after its use in the file:
  MString Late = GetGeometryHead();
  Late += "Volume Box\n";
  Late += "Box.Material Aluminium\n";
  Late += "Box.Shape BRIK 2.0 3.0 4.0\n";
  Late += "Orientation Place\n";
  Late += "Box.Orientation Place\n";
  Late += "Box.Mother World\n";
  Late += "Place.Position 1.0 2.0 3.0\n";

  MDGeometry LateGeometry;
  Passed = EvaluateTrue("ScanSetupFile()", "orientation data after the use", "The geometry can be read", ScanGeometry(LateGeometry, Late, "Late.geo.setup")) && Passed;
  if (LateGeometry.IsScanned() == true) {
    // Defect GEO-B-27: A volume copies the position of its orientation when the line is read, later lines of the orientation are lost
    Passed = ExpectedToFail(EvaluateVectorNear("GetPosition()", "orientation data after the use", "The volume has the position of the orientation", LateGeometry.GetVolume("Box")->GetPosition(), MVector(1, 2, 3), 1e-14)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output
bool UTDOrientation::TestToString()
{
  bool Passed = true;

  MDOrientation O("O");
  O.SetPosition(MVector(1.5, -2.5, 3.5));
  MString Text = O.ToString();
  Passed = EvaluateTrue("ToString()", "position", "The text starts with the keyword", Text.Contains("Pos:")) && Passed;
  Passed = EvaluateTrue("ToString()", "position", "The x position is in the text", Text.Contains("1.5")) && Passed;
  Passed = EvaluateTrue("ToString()", "position", "The y position is in the text", Text.Contains("-2.5")) && Passed;
  Passed = EvaluateTrue("ToString()", "position", "The z position is in the text", Text.Contains("3.5")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  // Hide the information messages of ROOT
  gErrorIgnoreLevel = kWarning;

  UTDOrientation Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDOrientation.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
