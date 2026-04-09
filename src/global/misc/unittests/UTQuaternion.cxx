/*
 * UTQuaternion.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MQuaternion.h"
#include "MUnitTest.h"
#include "MStreams.h"

// Standard lib:
#include <sstream>
using namespace std;


//! Unit test class for the MQuaternion helper
class UTQuaternion : public MUnitTest
{
public:
  //! Default constructor
  UTQuaternion() : MUnitTest("UTQuaternion") {}
  //! Default destructor
  virtual ~UTQuaternion() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test constructors, getters and identity-style helpers
  bool TestConstructionAndAccess();
  //! Test quaternion arithmetic operators
  bool TestArithmetic();
  //! Test norm-based helpers and edge cases
  bool TestNormAndInverse();
  //! Test conversion to and from rotations
  bool TestRotationConversion();
  //! Test interpolation and stream formatting
  bool TestInterpolationAndFormatting();

  //! Return true if two quaternions match within a tolerance
  bool AreEqual(const MQuaternion& A, const MQuaternion& B, double Tolerance);
  //! Return true if two quaternions represent the same rotation within a tolerance
  bool AreEquivalentRotations(const MQuaternion& A, const MQuaternion& B, double Tolerance);
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTQuaternion::Run()
{
  bool AllPassed = true;

  AllPassed = TestConstructionAndAccess() && AllPassed;
  AllPassed = TestArithmetic() && AllPassed;
  AllPassed = TestNormAndInverse() && AllPassed;
  AllPassed = TestRotationConversion() && AllPassed;
  AllPassed = TestInterpolationAndFormatting() && AllPassed;

  Summarize();

  return AllPassed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if two quaternions match within a tolerance
bool UTQuaternion::AreEqual(const MQuaternion& A, const MQuaternion& B, double Tolerance)
{
  if (fabs(A.GetW() - B.GetW()) > Tolerance) return false;
  if (fabs(A.GetX() - B.GetX()) > Tolerance) return false;
  if (fabs(A.GetY() - B.GetY()) > Tolerance) return false;
  if (fabs(A.GetZ() - B.GetZ()) > Tolerance) return false;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if two quaternions represent the same rotation within a tolerance
bool UTQuaternion::AreEquivalentRotations(const MQuaternion& A, const MQuaternion& B, double Tolerance)
{
  return AreEqual(A, B, Tolerance) || AreEqual(A, -B, Tolerance);
}


////////////////////////////////////////////////////////////////////////////////


//! Test constructors, getters and identity-style helpers
bool UTQuaternion::TestConstructionAndAccess()
{
  bool Passed = true;

  MQuaternion Zero;
  Passed = EvaluateNear("MQuaternion()", "default W", "Default constructor initializes W to zero", Zero.GetW(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MQuaternion()", "default X", "Default constructor initializes X to zero", Zero.GetX(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MQuaternion()", "default Y", "Default constructor initializes Y to zero", Zero.GetY(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MQuaternion()", "default Z", "Default constructor initializes Z to zero", Zero.GetZ(), 0.0, 1e-12) && Passed;

  MQuaternion Explicit(1.0, 2.0, 3.0, 4.0);
  Passed = EvaluateNear("GetW()", "explicit", "Explicit constructor stores W", Explicit.GetW(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetX()", "explicit", "Explicit constructor stores X", Explicit.GetX(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetY()", "explicit", "Explicit constructor stores Y", Explicit.GetY(), 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetZ()", "explicit", "Explicit constructor stores Z", Explicit.GetZ(), 4.0, 1e-12) && Passed;

  MQuaternion Copy(Explicit);
  Passed = EvaluateTrue("MQuaternion(const MQuaternion&)", "copy", "Copy constructor duplicates all components", Copy == Explicit) && Passed;

  MQuaternion Assigned;
  Assigned = Explicit;
  Passed = EvaluateTrue("operator=", "assignment", "Assignment operator duplicates all components", Assigned == Explicit) && Passed;

  Passed = EvaluateTrue("operator==", "equal", "operator== recognizes identical quaternions", Explicit == MQuaternion(1.0, 2.0, 3.0, 4.0)) && Passed;
  Passed = EvaluateTrue("operator!=", "different", "operator!= recognizes different quaternions", Explicit != MQuaternion(1.0, 2.0, 3.0, 5.0)) && Passed;

  MQuaternion Negative = -Explicit;
  Passed = EvaluateTrue("operator-()", "unary minus", "Unary minus negates all quaternion components", Negative == MQuaternion(-1.0, -2.0, -3.0, -4.0)) && Passed;

  Passed = EvaluateNear("GetDotProduct()", "basis quaternions", "GetDotProduct returns the standard Euclidean dot product",
                        Explicit.GetDotProduct(Explicit, MQuaternion(0.5, -1.0, 2.0, 1.5)), 10.5, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test quaternion arithmetic operators
bool UTQuaternion::TestArithmetic()
{
  bool Passed = true;

  MQuaternion A(1.0, 2.0, 3.0, 4.0);
  MQuaternion B(0.5, -1.0, 2.0, 1.5);

  Passed = EvaluateTrue("operator+", "addition", "operator+ adds all components", A + B == MQuaternion(1.5, 1.0, 5.0, 5.5)) && Passed;
  Passed = EvaluateTrue("operator-", "subtraction", "operator- subtracts all components", A - B == MQuaternion(0.5, 3.0, 1.0, 2.5)) && Passed;
  Passed = EvaluateTrue("operator*", "multiplication", "operator* performs quaternion multiplication", A * B == MQuaternion(-9.5, -3.5, -3.5, 10.5)) && Passed;

  MQuaternion Sum = A;
  Sum += B;
  Passed = EvaluateTrue("operator+=", "addition", "operator+= updates this quaternion in place", Sum == MQuaternion(1.5, 1.0, 5.0, 5.5)) && Passed;

  MQuaternion Difference = A;
  Difference -= B;
  Passed = EvaluateTrue("operator-=", "subtraction", "operator-= updates this quaternion in place", Difference == MQuaternion(0.5, 3.0, 1.0, 2.5)) && Passed;

  MQuaternion Product = A;
  Product *= B;
  Passed = EvaluateTrue("operator*=", "multiplication", "operator*= matches quaternion multiplication", Product == MQuaternion(-9.5, -3.5, -3.5, 10.5)) && Passed;

  MQuaternion Quotient = A / B;
  MQuaternion InPlaceQuotient = A;
  InPlaceQuotient /= B;
  Passed = EvaluateTrue("operator/=", "division", "operator/= matches operator/", AreEqual(Quotient, InPlaceQuotient, 1e-12)) && Passed;
  Passed = EvaluateTrue("operator/", "division", "operator/ multiplied by the divisor reconstructs the original quaternion",
                        AreEqual(Quotient * B, A, 1e-10)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test norm-based helpers and edge cases
bool UTQuaternion::TestNormAndInverse()
{
  bool Passed = true;

  MQuaternion Q(1.0, 2.0, 3.0, 4.0);

  Passed = EvaluateNear("GetNorm()", "1,2,3,4", "GetNorm returns the sum of the squared components", Q.GetNorm(), 30.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMagnitude()", "1,2,3,4", "GetMagnitude returns the square root of the norm", Q.GetMagnitude(), sqrt(30.0), 1e-12) && Passed;
  Passed = EvaluateTrue("GetScale()", "scale by 2", "GetScale multiplies all components by the requested scalar", Q.GetScale(2.0) == MQuaternion(2.0, 4.0, 6.0, 8.0)) && Passed;
  Passed = EvaluateTrue("GetConjugate()", "1,2,3,4", "GetConjugate negates the vector part only", Q.GetConjugate() == MQuaternion(1.0, -2.0, -3.0, -4.0)) && Passed;

  MQuaternion Inverse = Q.GetInverse();
  Passed = EvaluateTrue("GetInverse()", "1,2,3,4", "The inverse multiplied by the quaternion yields the identity quaternion",
                        AreEqual(Inverse * Q, MQuaternion(1.0, 0.0, 0.0, 0.0), 1e-10)) && Passed;

  MQuaternion Unit = Q.GetUnitQuaternion();
  Passed = EvaluateNear("GetUnitQuaternion()", "norm", "GetUnitQuaternion normalizes the quaternion to magnitude one", Unit.GetMagnitude(), 1.0, 1e-12) && Passed;
  Passed = EvaluateTrue("GetUnitQuaternion()", "direction", "GetUnitQuaternion preserves the quaternion direction",
                        AreEqual(Unit, Q.GetScale(1.0 / sqrt(30.0)), 1e-12)) && Passed;

  MQuaternion Zero;
  __merr.Enable(false);
  MQuaternion ZeroInverse = Zero.GetInverse();
  MQuaternion ZeroUnit = Zero.GetUnitQuaternion();
  MRotation ZeroRotation = Zero.GetRotation();
  __merr.Enable(true);

  Passed = EvaluateTrue("GetInverse()", "zero quaternion", "GetInverse returns the default quaternion for zero norm", ZeroInverse == MQuaternion()) && Passed;
  Passed = EvaluateTrue("GetUnitQuaternion()", "zero quaternion", "GetUnitQuaternion returns the default quaternion for zero magnitude", ZeroUnit == MQuaternion()) && Passed;
  Passed = EvaluateTrue("GetRotation()", "zero quaternion", "GetRotation falls back to the identity rotation for zero magnitude", ZeroRotation == MRotation()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test conversion to and from rotations
bool UTQuaternion::TestRotationConversion()
{
  bool Passed = true;

  MQuaternion Identity(1.0, 0.0, 0.0, 0.0);
  MRotation IdentityRotation = Identity.GetRotation();
  Passed = EvaluateTrue("GetRotation()", "identity quaternion", "The identity quaternion converts to the identity rotation", IdentityRotation == MRotation()) && Passed;

  MQuaternion RotateZQuarter(cos(c_Pi / 4.0), 0.0, 0.0, sin(c_Pi / 4.0));
  MRotation Rotation = RotateZQuarter.GetRotation();
  Passed = EvaluateTrue("GetRotation()", "90 deg around z", "Quaternion-to-rotation conversion rotates x to y",
                        (Rotation * MVector(1.0, 0.0, 0.0)).AreEqual(MVector(0.0, 1.0, 0.0), 1e-12)) && Passed;

  MQuaternion FromRotation(Rotation);
  Passed = EvaluateTrue("MQuaternion(const MRotation&)", "90 deg around z", "Rotation-to-quaternion conversion reproduces the same rotation",
                        AreEquivalentRotations(FromRotation.GetUnitQuaternion(), RotateZQuarter.GetUnitQuaternion(), 1e-12)) && Passed;

  MRotation RotateX(c_Pi / 3.0, MVector(1.0, 0.0, 0.0));
  MQuaternion FromRotateX(RotateX);
  Passed = EvaluateTrue("MQuaternion(const MRotation&)", "60 deg around x", "Constructing from a rotation matrix preserves the represented rotation",
                        (FromRotateX.GetRotation() * MVector(0.0, 1.0, 0.0)).AreEqual(RotateX * MVector(0.0, 1.0, 0.0), 1e-12)) && Passed;

  MRotation RotateY(c_Pi, MVector(0.0, 1.0, 0.0));
  MQuaternion FromRotateY(RotateY);
  Passed = EvaluateTrue("MQuaternion(const MRotation&)", "180 deg around y", "The alternative conversion branch also preserves the represented rotation",
                        (FromRotateY.GetRotation() * MVector(1.0, 0.0, 0.0)).AreEqual(RotateY * MVector(1.0, 0.0, 0.0), 1e-12)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test interpolation and stream formatting
bool UTQuaternion::TestInterpolationAndFormatting()
{
  bool Passed = true;

  MQuaternion Identity(1.0, 0.0, 0.0, 0.0);
  MQuaternion RotateZHalfTurn(0.0, 0.0, 0.0, 1.0);

  Passed = EvaluateTrue("GetLerp()", "start point", "GetLerp returns the first endpoint for T = 0",
                        AreEquivalentRotations(Identity.GetLerp(Identity, RotateZHalfTurn, 0.0), Identity, 1e-12)) && Passed;
  Passed = EvaluateTrue("GetLerp()", "end point", "GetLerp returns the second endpoint for T = 1",
                        AreEquivalentRotations(Identity.GetLerp(Identity, RotateZHalfTurn, 1.0), RotateZHalfTurn, 1e-12)) && Passed;

  MQuaternion MidLerp = Identity.GetLerp(Identity, RotateZHalfTurn, 0.5);
  Passed = EvaluateNear("GetLerp()", "midpoint magnitude", "GetLerp returns a unit quaternion", MidLerp.GetMagnitude(), 1.0, 1e-12) && Passed;
  Passed = EvaluateTrue("GetLerp()", "midpoint rotation", "GetLerp halfway between identity and 180 degrees around z yields a 90 degree z rotation",
                        (MidLerp.GetRotation() * MVector(1.0, 0.0, 0.0)).AreEqual(MVector(0.0, 1.0, 0.0), 1e-12)) && Passed;

  Passed = EvaluateTrue("GetSlerp()", "start point", "GetSlerp returns the first endpoint for T = 0",
                        AreEquivalentRotations(Identity.GetSlerp(Identity, RotateZHalfTurn, 0.0), Identity, 1e-12)) && Passed;
  Passed = EvaluateTrue("GetSlerp()", "end point", "GetSlerp returns the second endpoint for T = 1",
                        AreEquivalentRotations(Identity.GetSlerp(Identity, RotateZHalfTurn, 1.0), RotateZHalfTurn, 1e-12)) && Passed;

  MQuaternion MidSlerp = Identity.GetSlerp(Identity, RotateZHalfTurn, 0.5);
  Passed = EvaluateNear("GetSlerp()", "midpoint magnitude", "GetSlerp returns a unit quaternion", MidSlerp.GetMagnitude(), 1.0, 1e-12) && Passed;
  Passed = EvaluateTrue("GetSlerp()", "midpoint rotation", "GetSlerp halfway between identity and 180 degrees around z yields a 90 degree z rotation",
                        (MidSlerp.GetRotation() * MVector(1.0, 0.0, 0.0)).AreEqual(MVector(0.0, 1.0, 0.0), 1e-12)) && Passed;

  MQuaternion RotateZQuarter(cos(c_Pi / 4.0), 0.0, 0.0, sin(c_Pi / 4.0));
  MQuaternion SameRotationDifferentSign = -RotateZQuarter;
  MQuaternion ShortPath = Identity.GetSlerp(RotateZQuarter, SameRotationDifferentSign, 0.5);
  Passed = EvaluateTrue("GetSlerp()", "sign ambiguity", "GetSlerp handles the quaternion sign ambiguity when both endpoints represent the same rotation",
                        AreEquivalentRotations(ShortPath, RotateZQuarter, 1e-12)) && Passed;

  ostringstream Out;
  Out << MQuaternion(1.0, 2.0, 3.0, 4.0);
  Passed = EvaluateTrue("operator<<", "stream formatting", "The stream operator prints the documented quaternion layout", Out.str() == "[1, (2, 3, 4)]") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTQuaternion Test;
  return Test.Run() == true ? 0 : 1;
}
