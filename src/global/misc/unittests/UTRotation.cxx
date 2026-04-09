/*
 * UTRotation.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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

  Summarize();

  return AllPassed;
}


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

  streambuf* OldBuffer = cerr.rdbuf();
  ostringstream ErrorCapture;
  cerr.rdbuf(ErrorCapture.rdbuf());
  MRotation SingularInverse = Singular.GetInvers();
  cerr.rdbuf(OldBuffer);

  Passed = EvaluateTrue("GetInvers()", "singular matrix", "GetInvers returns the identity matrix when inversion fails", SingularInverse == MRotation()) && Passed;
  Passed = EvaluateTrue("GetInvers()", "singular matrix warning", "GetInvers emits a warning for singular matrices",
                        ErrorCapture.str().find("determinant is zero") != string::npos) && Passed;

  MRotation SingularInPlace = Singular;
  OldBuffer = cerr.rdbuf();
  ostringstream ErrorCaptureInPlace;
  cerr.rdbuf(ErrorCaptureInPlace.rdbuf());
  SingularInPlace.Invert();
  cerr.rdbuf(OldBuffer);
  Passed = EvaluateTrue("Invert()", "singular matrix", "Invert falls back to the identity matrix for singular matrices", SingularInPlace == MRotation()) && Passed;
  Passed = EvaluateTrue("Invert()", "singular matrix warning", "Invert forwards the singular inversion warning",
                        ErrorCaptureInPlace.str().find("determinant is zero") != string::npos) && Passed;

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


//! Main function
int main()
{
  UTRotation Test;
  bool Passed = Test.Run();

  return Passed ? 0 : 1;
}


////////////////////////////////////////////////////////////////////////////////
