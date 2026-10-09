/*
 * UTRotation.cxx
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


// MEGAlib:
#include "MRotation.h"
#include "MUnitTest.h"

// Standard lib:
#include <sstream>
using namespace std;

//! Unit test class for the MRotation helper
class UTRotation : public MUnitTest
{
public:
  //! Default constructor
  UTRotation() : MUnitTest("UTRotation") {}
  //! Default destructor
  virtual ~UTRotation() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test constructors, setters and element access
  bool TestConstructionAndAccess();
  //! Test vector and matrix multiplication
  bool TestMultiplication();
  //! Test inversion, determinant and rotation validation
  bool TestInversionAndValidation();
  //! Test angle getters and formatting helpers
  bool TestAnglesAndFormatting();
  //! Test additional edge cases and numerical corner cases
  bool TestEdgeCases();
  //! Test the orthonormalization of the axes
  bool TestOrthonormalization();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTRotation::Run()
{
  bool AllPassed = true;

  AllPassed = TestConstructionAndAccess() && AllPassed;
  AllPassed = TestMultiplication() && AllPassed;
  AllPassed = TestInversionAndValidation() && AllPassed;
  AllPassed = TestAnglesAndFormatting() && AllPassed;
  AllPassed = TestEdgeCases() && AllPassed;
  AllPassed = TestOrthonormalization() && AllPassed;

  Summarize();

  return AllPassed;
}


////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////


//! Test constructors, setters and element access
bool UTRotation::TestConstructionAndAccess()
{
  bool Passed = true;

  MRotation Identity;
  Passed = EvaluateNear("GetXX()", "identity", "Default constructor creates the identity matrix", Identity.GetXX(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetYY()", "identity", "Default constructor creates the identity matrix", Identity.GetYY(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetZZ()", "identity", "Default constructor creates the identity matrix", Identity.GetZZ(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetXY()", "identity", "Off-diagonal terms are zero in the identity matrix", Identity.GetXY(), 0.0, 1e-12) && Passed;

  MRotation Explicit(1.0, 2.0, 3.0,
                     4.0, 5.0, 6.0,
                     7.0, 8.0, 9.0);
  Passed = EvaluateNear("GetXX()", "explicit matrix", "Explicit constructor stores XX", Explicit.GetXX(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetYX()", "explicit matrix", "Explicit constructor stores YX", Explicit.GetYX(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetZX()", "explicit matrix", "Explicit constructor stores ZX", Explicit.GetZX(), 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetXY()", "explicit matrix", "Explicit constructor stores XY", Explicit.GetXY(), 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetYY()", "explicit matrix", "Explicit constructor stores YY", Explicit.GetYY(), 5.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetZY()", "explicit matrix", "Explicit constructor stores ZY", Explicit.GetZY(), 6.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetXZ()", "explicit matrix", "Explicit constructor stores XZ", Explicit.GetXZ(), 7.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetYZ()", "explicit matrix", "Explicit constructor stores YZ", Explicit.GetYZ(), 8.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetZZ()", "explicit matrix", "Explicit constructor stores ZZ", Explicit.GetZZ(), 9.0, 1e-12) && Passed;

  MRotation Copy(Explicit);
  Passed = EvaluateTrue("MRotation(const MRotation&)", "copy", "Copy constructor duplicates all elements", Copy == Explicit) && Passed;

  MRotation Assigned;
  Assigned = Explicit;
  Passed = EvaluateTrue("operator=", "assignment", "Assignment operator duplicates all elements", Assigned == Explicit) && Passed;

  // Equality compares all nine elements: changing any single one makes the matrices different
  const double Elements[9] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0 };
  const char* ElementNames[9] = { "XX", "YX", "ZX", "XY", "YY", "ZY", "XZ", "YZ", "ZZ" };
  for (unsigned int e = 0; e < 9; ++e) {
    double Changed[9];
    for (unsigned int i = 0; i < 9; ++i) Changed[i] = Elements[i];
    Changed[e] += 1e-9;
    MRotation Different(Changed[0], Changed[1], Changed[2], Changed[3], Changed[4], Changed[5], Changed[6], Changed[7], Changed[8]);
    Passed = EvaluateFalse("operator==", MString("element ") + ElementNames[e], "Matrices which differ in a single element are not equal", Different == Explicit) && Passed;
  }

  MRotation SetMatrix;
  SetMatrix.Set(1.0, 0.0, 0.0,
                0.0, 1.0, 0.0,
                0.0, 0.0, 1.0);
  Passed = EvaluateTrue("Set(...)", "identity elements", "Set stores all supplied matrix elements", SetMatrix == Identity) && Passed;

  SetMatrix.SetXX(2.0);
  SetMatrix.SetXY(3.0);
  SetMatrix.SetXZ(4.0);
  SetMatrix.SetYX(5.0);
  SetMatrix.SetYY(6.0);
  SetMatrix.SetYZ(7.0);
  SetMatrix.SetZX(8.0);
  SetMatrix.SetZY(9.0);
  SetMatrix.SetZZ(10.0);
  Passed = EvaluateTrue("SetXX...SetZZ()", "component setters", "Individual setters update the requested matrix elements",
                        SetMatrix == MRotation(2.0, 5.0, 8.0, 3.0, 6.0, 9.0, 4.0, 7.0, 10.0)) && Passed;

  SetMatrix.SetIdentity();
  Passed = EvaluateTrue("SetIdentity()", "reset", "SetIdentity restores the identity matrix", SetMatrix == Identity) && Passed;

  Passed = EvaluateTrue("GetX()", "identity", "GetX returns the first basis vector", Identity.GetX().AreEqual(MVector(1.0, 0.0, 0.0), 1e-12)) && Passed;
  Passed = EvaluateTrue("GetY()", "identity", "GetY returns the second basis vector", Identity.GetY().AreEqual(MVector(0.0, 1.0, 0.0), 1e-12)) && Passed;
  Passed = EvaluateTrue("GetZ()", "identity", "GetZ returns the third basis vector", Identity.GetZ().AreEqual(MVector(0.0, 0.0, 1.0), 1e-12)) && Passed;

  MRotation AxisAngle(c_Pi/2.0, MVector(0.0, 0.0, 1.0));
  Passed = EvaluateTrue("MRotation(angle, axis)", "pi/2 around z", "Axis-angle constructor builds the expected rotation", (AxisAngle * MVector(1.0, 0.0, 0.0)).AreEqual(MVector(0.0, 1.0, 0.0), 1e-12)) && Passed;

  MRotation ZeroRotation;
  ZeroRotation.Set(c_Pi/4.0, MVector(0.0, 0.0, 0.0));
  Passed = EvaluateTrue("Set(angle, axis)", "zero axis", "Set(angle, axis) falls back to identity for a zero axis", ZeroRotation == Identity) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test vector and matrix multiplication
bool UTRotation::TestMultiplication()
{
  bool Passed = true;

  MRotation RotateZ(c_Pi/2.0, MVector(0.0, 0.0, 1.0));
  Passed = EvaluateTrue("operator*(rotation, vector)", "pi/2 around z", "Rotation-vector multiplication rotates x to y", (RotateZ * MVector(1.0, 0.0, 0.0)).AreEqual(MVector(0.0, 1.0, 0.0), 1e-12)) && Passed;

  MRotation RotateX(c_Pi/2.0, MVector(1.0, 0.0, 0.0));
  MRotation Combined = RotateZ * RotateX;
  MRotation InPlace = RotateZ;
  InPlace *= RotateX;
  Passed = EvaluateTrue("operator*=(rotation)", "composition", "operator*= matches free matrix multiplication", InPlace == Combined) && Passed;

  MRotation Scaled = RotateZ;
  Scaled *= 2.0;
  Passed = EvaluateNear("operator*=(double)", "scaled rotation", "Scalar multiplication scales matrix coefficients", Scaled.GetXX(), 2.0 * RotateZ.GetXX(), 1e-12) && Passed;
  Passed = EvaluateNear("operator*=(double)", "scaled rotation", "Scalar multiplication scales matrix coefficients", Scaled.GetZY(), 2.0 * RotateZ.GetZY(), 1e-12) && Passed;

  MRotation Additional;
  Additional.Rotate(c_Pi/2.0, MVector(0.0, 0.0, 1.0));
  Passed = EvaluateTrue("Rotate(angle, axis)", "pi/2 around z", "Rotate applies an additional rotation to the matrix", (Additional * MVector(1.0, 0.0, 0.0)).AreEqual(MVector(0.0, 1.0, 0.0), 1e-12)) && Passed;

  MRotation Identity;
  MVector Vector(1.0, -2.0, 3.0);
  Identity.Rotate(Vector);
  Passed = EvaluateTrue("Rotate(MVector&)", "identity", "Rotate(MVector&) leaves a vector unchanged for the identity matrix", Vector.AreEqual(MVector(1.0, -2.0, 3.0), 1e-12)) && Passed;

  // A rotation by 90 deg around z maps (x, y, z) to (-y, x, z), and the in-place rotation equals the matrix product:
  MRotation QuarterZ(c_Pi/2.0, MVector(0.0, 0.0, 1.0));
  MVector RotatedVector(1.0, -2.0, 3.0);
  QuarterZ.Rotate(RotatedVector);
  Passed = EvaluateTrue("Rotate(MVector&)", "pi/2 around z", "Rotate(MVector&) rotates (1, -2, 3) to (2, 1, 3)", RotatedVector.AreEqual(MVector(2.0, 1.0, 3.0), 1e-12)) && Passed;
  Passed = EvaluateTrue("Rotate(MVector&)", "pi/2 around z", "Rotate(MVector&) gives the same result as the matrix product", RotatedVector.AreEqual(QuarterZ * MVector(1.0, -2.0, 3.0), 1e-12)) && Passed;

  MVector InteriorAxis(1.0, 2.0, 3.0);
  InteriorAxis.Unitize();
  MRotation InteriorRotation(0.731, InteriorAxis);
  MVector InteriorInput(0.25, -0.5, 1.75);
  MVector InteriorOutput = InteriorRotation * InteriorInput;
  Passed = EvaluateNear("operator*(rotation, vector)", "interior axis magnitude", "Representative non-axis-aligned rotations preserve vector magnitude", InteriorOutput.Mag(), InteriorInput.Mag(), 1e-12) && Passed;
  Passed = EvaluateTrue("operator*(rotation, vector)", "interior axis changed", "Representative non-axis-aligned rotations change a non-axis-aligned input vector", InteriorOutput.AreEqual(InteriorInput, 1e-12) == false) && Passed;

  MRotation InteriorAdditional;
  InteriorAdditional.Rotate(0.731, InteriorAxis);
  Passed = EvaluateTrue("Rotate(angle, axis)", "interior axis", "Rotate(angle, axis) also handles representative non-axis-aligned rotations", (InteriorAdditional * InteriorInput).AreEqual(InteriorOutput, 1e-12)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test inversion, determinant and rotation validation
bool UTRotation::TestInversionAndValidation()
{
  bool Passed = true;

  MRotation RotateZ(c_Pi/3.0, MVector(0.0, 0.0, 1.0));
  Passed = EvaluateNear("GetDeterminant()", "rotation", "A proper rotation has determinant +1", RotateZ.GetDeterminant(), 1.0, 1e-12) && Passed;
  Passed = EvaluateTrue("IsRotation()", "rotation", "IsRotation accepts a proper rotation", RotateZ.IsRotation()) && Passed;
  Passed = EvaluateTrue("IsRotation()", "identity", "The identity matrix is a rotation", MRotation().IsRotation()) && Passed;

  MRotation Inverse = RotateZ.GetInvers();
  Passed = EvaluateTrue("GetInvers()", "rotation", "Inverse of a rotation undoes the rotation", ((Inverse * RotateZ) * MVector(1.0, 2.0, 3.0)).AreEqual(MVector(1.0, 2.0, 3.0), 1e-10)) && Passed;

  MRotation Inverted = RotateZ;
  Inverted.Invert();
  Passed = EvaluateTrue("Invert()", "rotation", "Invert modifies the matrix to its inverse", ((Inverted * RotateZ) * MVector(1.0, 2.0, 3.0)).AreEqual(MVector(1.0, 2.0, 3.0), 1e-10)) && Passed;

  MRotation Scaled(2.0, 0.0, 0.0,
                   0.0, 3.0, 0.0,
                   0.0, 0.0, 4.0);
  Passed = EvaluateNear("GetDeterminant()", "diagonal scale", "Determinant matches the product of the diagonal entries", Scaled.GetDeterminant(), 24.0, 1e-12) && Passed;
  Passed = EvaluateFalse("IsRotation()", "diagonal scale", "IsRotation rejects non-orthonormal matrices", Scaled.IsRotation()) && Passed;

  MRotation ScaledInverse = Scaled.GetInvers();
  Passed = EvaluateTrue("GetInvers()", "diagonal scale", "GetInvers computes the true matrix inverse", (ScaledInverse * Scaled * MVector(1.0, 2.0, 3.0)).AreEqual(MVector(1.0, 2.0, 3.0), 1e-10)) && Passed;

  MRotation Reflection(-1.0, 0.0, 0.0,
                       0.0, 1.0, 0.0,
                       0.0, 0.0, 1.0);
  Passed = EvaluateNear("GetDeterminant()", "reflection", "A simple reflection has determinant -1", Reflection.GetDeterminant(), -1.0, 1e-12) && Passed;
  Passed = EvaluateFalse("IsRotation()", "reflection", "IsRotation rejects reflections with determinant -1", Reflection.IsRotation()) && Passed;

  MVector XAxis(0.0, 1.0, 0.0);
  MVector ZAxis(0.0, 0.0, 1.0);
  MVector YAxis = ZAxis.Cross(XAxis);
  MRotation BasisRotation(XAxis.X(), YAxis.X(), ZAxis.X(),
                          XAxis.Y(), YAxis.Y(), ZAxis.Y(),
                          XAxis.Z(), YAxis.Z(), ZAxis.Z());
  Passed = EvaluateTrue("IsRotation()", "axis basis", "Axis-basis rotations built from X, Y=ZxX, Z are proper rotations", BasisRotation.IsRotation()) && Passed;
  Passed = EvaluateTrue("GetInvers()", "axis basis", "Axis-basis rotations invert correctly for cached inverse workflows", (BasisRotation.GetInvers() * (BasisRotation * MVector(2.0, 3.0, 4.0))).AreEqual(MVector(2.0, 3.0, 4.0), 1e-12)) && Passed;

  MVector InteriorAxis(1.0, 2.0, 3.0);
  InteriorAxis.Unitize();
  MRotation InteriorRotation(0.731, InteriorAxis);
  Passed = EvaluateNear("GetDeterminant()", "interior rotation", "Representative non-axis-aligned rotations still have determinant +1", InteriorRotation.GetDeterminant(), 1.0, 1e-12) && Passed;
  Passed = EvaluateTrue("IsRotation()", "interior rotation", "IsRotation accepts representative non-axis-aligned rotations", InteriorRotation.IsRotation()) && Passed;
  Passed = EvaluateTrue("GetInvers()", "interior rotation", "Representative non-axis-aligned rotations invert correctly", (InteriorRotation.GetInvers() * (InteriorRotation * MVector(0.25, -0.5, 1.75))).AreEqual(MVector(0.25, -0.5, 1.75), 1e-12)) && Passed;

  // The inverse of a rotation is its transpose: the rows of the inverse are the columns of the rotation
  const MRotation InteriorTranspose(InteriorRotation.GetXX(), InteriorRotation.GetXY(), InteriorRotation.GetXZ(),
                                    InteriorRotation.GetYX(), InteriorRotation.GetYY(), InteriorRotation.GetYZ(),
                                    InteriorRotation.GetZX(), InteriorRotation.GetZY(), InteriorRotation.GetZZ());
  Passed = EvaluateRotationNear("GetInvers()", "interior rotation, transpose", "The inverse of a rotation made from angle and axis is its transpose", InteriorRotation.GetInvers(), InteriorTranspose, 1e-14) && Passed;

  // The inverse of general matrices (neither diagonal nor determinant +-1), expected values from the cofactor formula:
  MRotation General(2.0, 1.0, 0.0,
                    0.0, 3.0, 1.0,
                    1.0, 0.0, 4.0);
  Passed = EvaluateNear("GetDeterminant()", "general matrix", "The determinant of a general matrix (rule of Sarrus)", General.GetDeterminant(), 25.0, 1e-12) && Passed;
  const MRotation GeneralInverse( 12.0/25.0, -4.0/25.0,  1.0/25.0,
                                      1.0/25.0,  8.0/25.0, -2.0/25.0,
                                     -3.0/25.0,  1.0/25.0,  6.0/25.0 );
  Passed = EvaluateRotationNear("GetInvers()", "general matrix", "The inverse of a general matrix with determinant 25", General.GetInvers(), GeneralInverse, 1e-14) && Passed;
  const MRotation Identity9( 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 );
  Passed = EvaluateRotationNear("GetInvers()", "general matrix", "A matrix times its inverse is the identity", General * General.GetInvers(), Identity9, 1e-14) && Passed;
  MRotation GeneralInPlace = General;
  GeneralInPlace.Invert();
  Passed = EvaluateRotationNear("Invert()", "general matrix", "Invert gives the same matrix as GetInvers", GeneralInPlace, GeneralInverse, 1e-14) && Passed;

  MRotation NegativeDeterminant(0.0, 1.0, 0.0,
                                2.0, 0.0, 0.0,
                                0.0, 0.0, 3.0);
  Passed = EvaluateNear("GetDeterminant()", "negative determinant", "The determinant of a matrix with swapped rows", NegativeDeterminant.GetDeterminant(), -6.0, 1e-12) && Passed;
  const MRotation NegativeInverse( 0.0, 0.5, 0.0,
                                      1.0, 0.0, 0.0,
                                      0.0, 0.0, 1.0/3.0 );
  Passed = EvaluateRotationNear("GetInvers()", "negative determinant", "The inverse of a matrix with determinant -6", NegativeDeterminant.GetInvers(), NegativeInverse, 1e-14) && Passed;

  MRotation Integer(1.0, 2.0, 3.0,
                    0.0, 1.0, 4.0,
                    5.0, 6.0, 0.0);
  const MRotation IntegerInverse( -24.0, 18.0,  5.0,
                                      20.0, -15.0, -4.0,
                                      -5.0,  4.0,  1.0 );
  Passed = EvaluateRotationNear("GetInvers()", "integer matrix", "The inverse of an integer matrix with determinant 1", Integer.GetInvers(), IntegerInverse, 1e-12) && Passed;

  // A matrix times its inverse is the identity for every invertible matrix, thus IsRotation has to check the axes themselves:
  Passed = EvaluateFalse("IsRotation()", "uniform scale 0.5", "A uniform scaling by 0.5 is no rotation", MRotation(0.5, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.5).IsRotation()) && Passed;
  Passed = EvaluateFalse("IsRotation()", "uniform scale 2", "A uniform scaling by 2 is no rotation", MRotation(2.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 2.0).IsRotation()) && Passed;
  Passed = EvaluateFalse("IsRotation()", "scale with determinant 1", "A scaling with determinant +1 is no rotation", MRotation(2.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 1.0).IsRotation()) && Passed;
  Passed = EvaluateFalse("IsRotation()", "shear", "A shear with determinant +1 is no rotation", MRotation(1.0, 0.5, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0).IsRotation()) && Passed;
  Passed = EvaluateFalse("IsRotation()", "singular", "A singular matrix is no rotation", MRotation(1.0, 2.0, 3.0, 2.0, 4.0, 6.0, 0.0, 0.0, 1.0).IsRotation()) && Passed;

  MVector MirroredY = ZAxis.Cross(XAxis);
  MirroredY *= -1.0;
  MRotation MirroredBasis(XAxis.X(), MirroredY.X(), ZAxis.X(),
                          XAxis.Y(), MirroredY.Y(), ZAxis.Y(),
                          XAxis.Z(), MirroredY.Z(), ZAxis.Z());
  Passed = EvaluateNear("GetDeterminant()", "mirrored basis", "A basis with y = -(z cross x) has determinant -1", MirroredBasis.GetDeterminant(), -1.0, 1e-12) && Passed;
  Passed = EvaluateFalse("IsRotation()", "mirrored basis", "A mirrored basis is no rotation", MirroredBasis.IsRotation()) && Passed;

  // The tolerance applies to the lengths of the axes (here 1 + 5e-7) and their angles (here a 5e-7 rad deviation from perpendicular), the default is 1e-6:
  MRotation LongAxis(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 + 5.0e-7);
  Passed = EvaluateTrue("IsRotation()", "default tolerance, long axis", "The default tolerance 1e-6 accepts an axis of length 1 + 5e-7", LongAxis.IsRotation()) && Passed;
  Passed = EvaluateFalse("IsRotation()", "default tolerance, too long axis", "The default tolerance 1e-6 rejects an axis of length 1 + 5e-6", MRotation(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 + 5.0e-6).IsRotation()) && Passed;
  MRotation SkewedAxes(1.0, 0.0, sin(5.0e-7), 0.0, 1.0, 0.0, 0.0, 0.0, cos(5.0e-7));
  Passed = EvaluateTrue("IsRotation()", "skewed axes within tolerance", "Axes 5e-7 rad from perpendicular are accepted with the tolerance 1e-6", SkewedAxes.IsRotation(1e-6)) && Passed;
  Passed = EvaluateFalse("IsRotation()", "skewed axes outside tolerance", "The same axes are rejected with the tolerance 1e-8", SkewedAxes.IsRotation(1e-8)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test angle getters and formatting helpers
bool UTRotation::TestAnglesAndFormatting()
{
  bool Passed = true;

  MRotation Identity;
  Passed = EvaluateNear("GetThetaX()", "identity", "GetThetaX returns pi/2 for the x axis of the identity matrix", Identity.GetThetaX(), c_Pi/2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetPhiX()", "identity", "GetPhiX returns zero for the x axis of the identity matrix", Identity.GetPhiX(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetThetaY()", "identity", "GetThetaY returns pi/2 for the y axis of the identity matrix", Identity.GetThetaY(), c_Pi/2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetPhiY()", "identity", "GetPhiY returns pi/2 for the y axis of the identity matrix", Identity.GetPhiY(), c_Pi/2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetThetaZ()", "identity", "GetThetaZ returns zero for the z axis of the identity matrix", Identity.GetThetaZ(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetPhiZ()", "identity", "GetPhiZ returns zero for the z axis of the identity matrix", Identity.GetPhiZ(), 0.0, 1e-12) && Passed;

  ostringstream Out;
  Out << Identity;
  Passed = Evaluate("operator<<", "identity", "Stream output formats the full 3x3 matrix", MString(Out.str()), MString("(1/0/0, 0/1/0, 0/0/1)")) && Passed;

  MVector InteriorAxis(1.0, 2.0, 3.0);
  InteriorAxis.Unitize();
  MRotation InteriorRotation(0.731, InteriorAxis);
  // Expected values from the Rodrigues rotation formula (independent of MRotation) for 0.731 rad around (1, 2, 3)/sqrt(14):
  Passed = EvaluateNear("GetThetaX()", "interior rotation", "GetThetaX returns the polar angle of the rotated x axis for a non-axis-aligned rotation", InteriorRotation.GetThetaX(), 1.8776979432269041, 1e-12) && Passed;
  Passed = EvaluateNear("GetPhiX()", "interior rotation", "GetPhiX returns the azimuth of the rotated x axis for a non-axis-aligned rotation", InteriorRotation.GetPhiX(), 0.6432610203165977, 1e-12) && Passed;
  Passed = EvaluateNear("GetThetaY()", "interior rotation", "GetThetaY returns the polar angle of the rotated y axis for a non-axis-aligned rotation", InteriorRotation.GetThetaY(), 1.2787375736944959, 1e-12) && Passed;
  Passed = EvaluateNear("GetPhiY()", "interior rotation", "GetPhiY returns the azimuth of the rotated y axis for a non-axis-aligned rotation", InteriorRotation.GetPhiY(), 2.1186302327798257, 1e-12) && Passed;
  Passed = EvaluateNear("GetThetaZ()", "interior rotation", "GetThetaZ returns the polar angle of the rotated z axis for a non-axis-aligned rotation", InteriorRotation.GetThetaZ(), 0.4305111265235029, 1e-12) && Passed;
  Passed = EvaluateNear("GetPhiZ()", "interior rotation", "GetPhiZ returns the azimuth of the rotated z axis for a non-axis-aligned rotation", InteriorRotation.GetPhiZ(), -0.1659285875602620, 1e-12) && Passed;

  // The stream output lists the matrix row by row, which the (symmetric) identity cannot show:
  ostringstream OutExplicit;
  OutExplicit << MRotation(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
  Passed = Evaluate("operator<<", "explicit matrix", "Stream output lists the matrix row by row", MString(OutExplicit.str()), MString("(1/2/3, 4/5/6, 7/8/9)")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test additional edge cases and numerical corner cases
bool UTRotation::TestEdgeCases()
{
  bool Passed = true;

  MRotation RotatePiX(c_Pi, MVector(1.0, 0.0, 0.0));
  Passed = EvaluateTrue("MRotation(angle, axis)", "180 deg around x", "A 180 degree x rotation flips y and z",
                        (RotatePiX * MVector(0.0, 1.0, 2.0)).AreEqual(MVector(0.0, -1.0, -2.0), 1e-12)) && Passed;

  MRotation RotatePiY(c_Pi, MVector(0.0, 1.0, 0.0));
  Passed = EvaluateTrue("MRotation(angle, axis)", "180 deg around y", "A 180 degree y rotation flips x and z",
                        (RotatePiY * MVector(1.0, 0.0, 2.0)).AreEqual(MVector(-1.0, 0.0, -2.0), 1e-12)) && Passed;

  MRotation RotatePiZ(c_Pi, MVector(0.0, 0.0, 1.0));
  Passed = EvaluateTrue("MRotation(angle, axis)", "180 deg around z", "A 180 degree z rotation flips x and y",
                        (RotatePiZ * MVector(1.0, 2.0, 0.0)).AreEqual(MVector(-1.0, -2.0, 0.0), 1e-12)) && Passed;

  MRotation RotateTiny(1.0e-9, MVector(0.0, 0.0, 1.0));
  Passed = EvaluateTrue("MRotation(angle, axis)", "tiny angle", "A very small rotation angle still produces a valid rotation matrix", RotateTiny.IsRotation(1e-9)) && Passed;

  MRotation AlmostRotation(cos(1.0e-7), -sin(1.0e-7), 0.0,
                           sin(1.0e-7),  cos(1.0e-7), 0.0,
                           0.0,          0.0,         1.0 + 5.0e-7);
  Passed = EvaluateTrue("IsRotation()", "within tolerance", "IsRotation accepts small numerical deviations within tolerance", AlmostRotation.IsRotation(1e-6)) && Passed;
  Passed = EvaluateFalse("IsRotation()", "outside tolerance", "IsRotation rejects the same matrix once the tolerance is tightened", AlmostRotation.IsRotation(1e-8)) && Passed;

  MRotation Singular(1.0, 2.0, 3.0,
                     2.0, 4.0, 6.0,
                     0.0, 0.0, 1.0);
  Passed = EvaluateNear("GetDeterminant()", "singular matrix", "A matrix with dependent rows has determinant zero", Singular.GetDeterminant(), 0.0, 1e-12) && Passed;

  MString SingularInverseLog = GetTemporaryFileName("singular_inverse.log");
  __merr.Connect(SingularInverseLog, false);
  __merr.DumpToStdOut(false);
  MRotation SingularInverse = Singular.GetInvers();
  __merr.DumpToStdOut(true);
  __merr.Disconnect(SingularInverseLog);
  MString SingularInverseMessage = ReadTextFile(SingularInverseLog);

  Passed = EvaluateTrue("GetInvers()", "singular matrix", "GetInvers returns the identity matrix when inversion fails", SingularInverse == MRotation()) && Passed;
  Passed = EvaluateTrue("GetInvers()", "singular matrix warning", "GetInvers emits a warning for singular matrices",
                        SingularInverseMessage.Contains("determinant is zero")) && Passed;

  MRotation SingularInPlace = Singular;
  MString SingularInvertLog = GetTemporaryFileName("singular_invert.log");
  __merr.Connect(SingularInvertLog, false);
  __merr.DumpToStdOut(false);
  SingularInPlace.Invert();
  __merr.DumpToStdOut(true);
  __merr.Disconnect(SingularInvertLog);
  MString SingularInvertMessage = ReadTextFile(SingularInvertLog);
  Passed = EvaluateTrue("Invert()", "singular matrix", "Invert falls back to the identity matrix for singular matrices", SingularInPlace == MRotation()) && Passed;
  Passed = EvaluateTrue("Invert()", "singular matrix warning", "Invert forwards the singular inversion warning",
                        SingularInvertMessage.Contains("determinant is zero")) && Passed;

  MRotation RotateA(c_Pi / 7.0, MVector(1.0, 0.0, 0.0));
  MRotation RotateB(c_Pi / 5.0, MVector(0.0, 1.0, 0.0));
  MRotation RotateC(c_Pi / 3.0, MVector(0.0, 0.0, 1.0));
  MRotation LeftAssociative = (RotateA * RotateB) * RotateC;
  MRotation RightAssociative = RotateA * (RotateB * RotateC);
  Passed = EvaluateTrue("operator*(rotation, rotation)", "associativity", "Matrix multiplication is associative within numerical precision",
                        (LeftAssociative * MVector(1.0, 2.0, 3.0)).AreEqual(RightAssociative * MVector(1.0, 2.0, 3.0), 1e-12)) && Passed;

  MRotation RotateCopy = RotateA;
  RotateCopy *= MRotation();
  Passed = EvaluateTrue("operator*=(rotation)", "right identity", "Multiplication by the identity on the right leaves the matrix unchanged", RotateCopy == RotateA) && Passed;

  MRotation LeftIdentity = MRotation() * RotateB;
  Passed = EvaluateTrue("operator*(rotation, rotation)", "left identity", "Multiplication by the identity on the left leaves the matrix unchanged", LeftIdentity == RotateB) && Passed;

  MRotation Reflection(-1.0, 0.0, 0.0,
                       0.0, 1.0, 0.0,
                       0.0, 0.0, 1.0);
  Passed = EvaluateNear("GetThetaX()", "reflection", "GetThetaX handles a reflected x axis", Reflection.GetThetaX(), c_Pi / 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetPhiX()", "reflection", "GetPhiX returns pi for a reflected x axis", Reflection.GetPhiX(), c_Pi, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the orthonormalization of the axes
bool UTRotation::TestOrthonormalization()
{
  bool Passed = true;

  const MRotation Identity9( 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 );
  const MRotation Mirrored9( 1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 1.0 );
  const double Root = 1.0/sqrt(2.0);

  // The z-axis keeps its direction, x is made perpendicular to z, y is z cross x (negated for a mirrored matrix).
  // Axes: x = (1, 0, 1), y = (0, 1, 0), z = (0, 0, 2) -> x' = (1, 0, 0), y' = (0, 1, 0), z' = (0, 0, 1)
  MRotation Tilted(1.0, 0.0, 0.0,
                   0.0, 1.0, 0.0,
                   1.0, 0.0, 2.0);
  Passed = EvaluateTrue("Orthonormalize()", "tilted x, long z", "Axes which are not perpendicular and not of unit length can be orthonormalized", Tilted.Orthonormalize()) && Passed;
  Passed = EvaluateRotationNear("Orthonormalize()", "tilted x, long z", "x = (1, 0, 1), z = (0, 0, 2) gives the identity", Tilted, Identity9, 1e-14) && Passed;

  // The same with y = (0, -1, 0): the handedness (determinant -2 < 0) is preserved, y' = -(z' cross x') = (0, -1, 0)
  MRotation TiltedMirrored(1.0,  0.0, 0.0,
                           0.0, -1.0, 0.0,
                           1.0,  0.0, 2.0);
  Passed = EvaluateTrue("Orthonormalize()", "mirrored, tilted x, long z", "A mirrored matrix can be orthonormalized", TiltedMirrored.Orthonormalize()) && Passed;
  Passed = EvaluateRotationNear("Orthonormalize()", "mirrored, tilted x, long z", "The handedness of a mirrored matrix is preserved", TiltedMirrored, Mirrored9, 1e-14) && Passed;

  // Axes 45 deg from perpendicular: z = (1, 0, 1), x = (1, 0, 0) -> z' = (1, 0, 1)/sqrt(2), x' = (1, 0, -1)/sqrt(2), y' = z' cross x' = (0, 1, 0)
  MRotation FortyFive(1.0, 0.0, 1.0,
                      0.0, 1.0, 0.0,
                      0.0, 0.0, 1.0);
  const MRotation FortyFiveExpected(  Root, 0.0, Root,
                                         0.0,  1.0, 0.0,
                                        -Root, 0.0, Root );
  Passed = EvaluateTrue("Orthonormalize()", "45 deg", "Axes 45 deg from perpendicular can be orthonormalized", FortyFive.Orthonormalize()) && Passed;
  Passed = EvaluateRotationNear("Orthonormalize()", "45 deg", "z is kept, x is projected into the plane perpendicular to z", FortyFive, FortyFiveExpected, 1e-14) && Passed;

  // The lengths of the input axes do not matter:
  MRotation LongAxes(0.5, 0.0, 0.0,
                     0.0, 7.0, 0.0,
                     0.0, 0.0, 3.0);
  Passed = EvaluateTrue("Orthonormalize()", "scaled axes", "Axes of different lengths can be orthonormalized", LongAxes.Orthonormalize()) && Passed;
  Passed = EvaluateRotationNear("Orthonormalize()", "scaled axes", "Axes of the lengths 0.5, 7, 3 give the identity", LongAxes, Identity9, 1e-14) && Passed;

  // Generic skewed axes, the same matrix with and without mirroring the y axis (this is also how the function is used: set first, then orthonormalize):
  for (int Mirror = 0; Mirror < 2; ++Mirror) {
    const double Sign = (Mirror == 0) ? 1.0 : -1.0;
    MRotation Skewed;
    Skewed.Set(1.0,  0.2*Sign,  0.3,
               0.1,  1.0*Sign, -0.2,
               0.4,  0.1*Sign,  1.2);
    MVector XBefore = Skewed.GetX();
    MVector ZBefore = Skewed.GetZ();
    double DeterminantBefore = Skewed.GetDeterminant();
    MString Input = (Mirror == 0) ? "generic skew" : "generic skew, mirrored";

    Passed = EvaluateTrue("Orthonormalize()", Input, "Skewed axes can be orthonormalized", Skewed.Orthonormalize()) && Passed;
    MVector X = Skewed.GetX();
    MVector Y = Skewed.GetY();
    MVector Z = Skewed.GetZ();
    Passed = EvaluateNear("Orthonormalize()", Input, "The x axis has unit length", X.Mag(), 1.0, 1e-14) && Passed;
    Passed = EvaluateNear("Orthonormalize()", Input, "The y axis has unit length", Y.Mag(), 1.0, 1e-14) && Passed;
    Passed = EvaluateNear("Orthonormalize()", Input, "The z axis has unit length", Z.Mag(), 1.0, 1e-14) && Passed;
    Passed = EvaluateNear("Orthonormalize()", Input, "x and y are perpendicular", X.Dot(Y), 0.0, 1e-14) && Passed;
    Passed = EvaluateNear("Orthonormalize()", Input, "x and z are perpendicular", X.Dot(Z), 0.0, 1e-14) && Passed;
    Passed = EvaluateNear("Orthonormalize()", Input, "y and z are perpendicular", Y.Dot(Z), 0.0, 1e-14) && Passed;
    Passed = EvaluateTrue("Orthonormalize()", Input, "The z axis keeps its direction", Z.AreEqual(ZBefore.Unit(), 1e-14)) && Passed;
    Passed = EvaluateNear("Orthonormalize()", Input, "The new x axis lies in the plane of the old x axis and the z axis", X.Dot(XBefore.Cross(ZBefore).Unit()), 0.0, 1e-14) && Passed;
    Passed = EvaluateTrue("Orthonormalize()", Input, "The new x axis is the closest one to the old x axis (positive projection)", X.Dot(XBefore) > 0.0) && Passed;
    Passed = EvaluateNear("Orthonormalize()", Input, "The determinant is +-1 with the original sign", Skewed.GetDeterminant(), (DeterminantBefore > 0) ? 1.0 : -1.0, 1e-14) && Passed;
    Passed = EvaluateTrue("Orthonormalize()", Input, "The y axis is +-(z cross x) with the original handedness", Y.AreEqual((DeterminantBefore > 0) ? Z.Cross(X) : MVector(-Z.Cross(X).X(), -Z.Cross(X).Y(), -Z.Cross(X).Z()), 1e-14)) && Passed;

    // The inverse of orthonormal axes is the transpose:
    const MRotation Transposed( Skewed.GetXX(), Skewed.GetXY(), Skewed.GetXZ(),
                                   Skewed.GetYX(), Skewed.GetYY(), Skewed.GetYZ(),
                                   Skewed.GetZX(), Skewed.GetZY(), Skewed.GetZZ() );
    Passed = EvaluateRotationNear("Orthonormalize()", Input, "The inverse of the orthonormalized matrix is its transpose", Skewed.GetInvers(), Transposed, 1e-14) && Passed;
    Passed = EvaluateTrue("IsRotation()", Input, "The orthonormalized matrix is a rotation if the original handedness was right-handed", Skewed.IsRotation(1e-12) == (DeterminantBefore > 0)) && Passed;

    // Orthonormalizing twice changes nothing:
    MRotation Twice = Skewed;
    Passed = EvaluateTrue("Orthonormalize()", Input, "A second orthonormalization works", Twice.Orthonormalize()) && Passed;
    Passed = EvaluateTrue("Orthonormalize()", Input, "A second orthonormalization changes nothing", Twice.GetX().AreEqual(X, 1e-14) && Twice.GetY().AreEqual(Y, 1e-14) && Twice.GetZ().AreEqual(Z, 1e-14)) && Passed;
  }

  // A realistic small skew, as accepted by Cosima for orientations (axes up to 1e-3 rad from perpendicular):
  // x = (1, 0, 0), z = (cos p, sin p, 0) with p = 89.95 deg and 90.05 deg, y = +-(z cross x) = (0, 0, -+sin p).
  // Closed form: x' = x - (x.z) z = sin p * (sin p, -cos p, 0), thus x' = (sin p, -cos p, 0), z' = z, and y' = +-(z' cross x') = (0, 0, -+1).
  for (double Angle : { 89.95, 90.05 }) {
    const double P = Angle*c_Pi/180.0;
    for (int Mirror = 0; Mirror < 2; ++Mirror) {
      const double Sign = (Mirror == 0) ? 1.0 : -1.0;
      const MVector XAxis(1.0, 0.0, 0.0);
      const MVector ZAxis(cos(P), sin(P), 0.0);
      const MVector YAxis = ZAxis.Cross(XAxis);
      MRotation SmallSkew(XAxis.X(), Sign*YAxis.X(), ZAxis.X(),
                          XAxis.Y(), Sign*YAxis.Y(), ZAxis.Y(),
                          XAxis.Z(), Sign*YAxis.Z(), ZAxis.Z());
      const MRotation SmallSkewExpected(sin(P), 0.0, cos(P),
                                        -cos(P), 0.0, sin(P),
                                        0.0, -Sign, 0.0);
      MString Input = MString("axes ") + Angle + " deg apart" + (Mirror == 1 ? ", mirrored" : "");
      Passed = EvaluateTrue("Orthonormalize()", Input, "Axes 0.05 deg from perpendicular can be orthonormalized", SmallSkew.Orthonormalize()) && Passed;
      Passed = EvaluateRotationNear("Orthonormalize()", Input, "x is projected perpendicular to z, z is kept, y has the original handedness", SmallSkew, SmallSkewExpected, 1e-14) && Passed;
    }
  }

  // An exact rotation is not changed:
  MVector InteriorAxis(1.0, 2.0, 3.0);
  InteriorAxis.Unitize();
  MRotation Exact(0.731, InteriorAxis);
  MRotation ExactCopy = Exact;
  Passed = EvaluateTrue("Orthonormalize()", "exact rotation", "An exact rotation can be orthonormalized", Exact.Orthonormalize()) && Passed;
  Passed = EvaluateTrue("Orthonormalize()", "exact rotation", "An exact rotation is not changed", Exact.GetX().AreEqual(ExactCopy.GetX(), 1e-14) && Exact.GetY().AreEqual(ExactCopy.GetY(), 1e-14) && Exact.GetZ().AreEqual(ExactCopy.GetZ(), 1e-14)) && Passed;
  Passed = EvaluateTrue("IsRotation()", "exact rotation", "The result is still a rotation", Exact.IsRotation(1e-12)) && Passed;

  // Axes which cannot be orthonormalized are rejected and the matrix is left unchanged:
  MRotation Zero(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  MRotation ZeroCopy = Zero;
  Passed = EvaluateFalse("Orthonormalize()", "zero matrix", "A zero matrix is rejected", Zero.Orthonormalize()) && Passed;
  Passed = EvaluateTrue("Orthonormalize()", "zero matrix", "A rejected zero matrix is unchanged", Zero == ZeroCopy) && Passed;

  MRotation ZeroZ(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0);
  MRotation ZeroZCopy = ZeroZ;
  Passed = EvaluateFalse("Orthonormalize()", "zero z axis", "A matrix with a zero z axis is rejected", ZeroZ.Orthonormalize()) && Passed;
  Passed = EvaluateTrue("Orthonormalize()", "zero z axis", "A rejected matrix with a zero z axis is unchanged", ZeroZ == ZeroZCopy) && Passed;

  MRotation Singular(1.0, 2.0, 3.0, 2.0, 4.0, 6.0, 0.0, 0.0, 1.0);
  MRotation SingularCopy = Singular;
  Passed = EvaluateFalse("Orthonormalize()", "singular matrix", "A singular matrix is rejected", Singular.Orthonormalize()) && Passed;
  Passed = EvaluateTrue("Orthonormalize()", "singular matrix", "A rejected singular matrix is unchanged", Singular == SingularCopy) && Passed;

  MRotation Parallel(1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0);
  MRotation ParallelCopy = Parallel;
  Passed = EvaluateFalse("Orthonormalize()", "x parallel to z", "Parallel x and z axes are rejected", Parallel.Orthonormalize()) && Passed;
  Passed = EvaluateTrue("Orthonormalize()", "x parallel to z", "A rejected matrix with parallel axes is unchanged", Parallel == ParallelCopy) && Passed;

  // Boundary of "x parallel to z": the part of x perpendicular to z has to be larger than 1e-12 times the length of x.
  // x = (1, e, 0), y = (0, 0, 1), z = (1, 0, 0) has the determinant e and the perpendicular part (0, e, 0).
  MRotation AlmostParallel(1.0, 0.0, 1.0, 1e-14, 0.0, 0.0, 0.0, 1.0, 0.0);
  MRotation AlmostParallelCopy = AlmostParallel;
  Passed = EvaluateFalse("Orthonormalize()", "almost parallel, 1e-14", "Axes with a perpendicular part of 1e-14 are rejected", AlmostParallel.Orthonormalize()) && Passed;
  Passed = EvaluateTrue("Orthonormalize()", "almost parallel, 1e-14", "A rejected almost-parallel matrix is unchanged", AlmostParallel == AlmostParallelCopy) && Passed;

  MRotation BarelyPerpendicular(1.0, 0.0, 1.0, 1e-6, 0.0, 0.0, 0.0, 1.0, 0.0);
  const MRotation BarelyPerpendicularExpected( 0.0, 0.0, 1.0,
                                                  1.0, 0.0, 0.0,
                                                  0.0, 1.0, 0.0 );
  Passed = EvaluateTrue("Orthonormalize()", "almost parallel, 1e-6", "Axes with a perpendicular part of 1e-6 are accepted", BarelyPerpendicular.Orthonormalize()) && Passed;
  Passed = EvaluateRotationNear("Orthonormalize()", "almost parallel, 1e-6", "x' = (0, 1, 0), y' = z' cross x' = (0, 0, 1), z' = (1, 0, 0)", BarelyPerpendicular, BarelyPerpendicularExpected, 1e-14) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Main function
int main()
{
  UTRotation Test;
  return Test.Run() == true ? 0 : 1;
}


////////////////////////////////////////////////////////////////////////////////
