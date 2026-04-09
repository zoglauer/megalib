/*
 * UTVector.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MStreams.h"
#include "MVector.h"
#include "MUnitTest.h"

// Standard lib:
#include <sstream>
using namespace std;


//! Unit test class for the MVector helper
class UTVector : public MUnitTest
{
public:
  //! Default constructor
  UTVector() : MUnitTest("UTVector") {}
  //! Default destructor
  virtual ~UTVector() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test constructors, clear, getters, setters and indexing
  bool TestConstructionAndAccess();
  //! Test comparisons and simple arithmetic
  bool TestComparisonAndArithmetic();
  //! Test norms, spherical coordinates and orthogonal vectors
  bool TestGeometry();
  //! Test rotations and high-level geometric relations
  bool TestRotationsAndRelations();
  //! Test string formatting and stream output
  bool TestFormatting();
  //! Test edge cases and degenerate geometry
  bool TestEdgeCases();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTVector::Run()
{
  bool AllPassed = true;

  AllPassed = TestConstructionAndAccess() && AllPassed;
  AllPassed = TestComparisonAndArithmetic() && AllPassed;
  AllPassed = TestGeometry() && AllPassed;
  AllPassed = TestRotationsAndRelations() && AllPassed;
  AllPassed = TestFormatting() && AllPassed;
  AllPassed = TestEdgeCases() && AllPassed;

  Summarize();

  return AllPassed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test constructors, clear, getters, setters and indexing
bool UTVector::TestConstructionAndAccess()
{
  bool Passed = true;

  MVector Default;
  Passed = EvaluateTrue("MVector()", "default", "Default constructor creates the null vector", Default.IsNull()) && Passed;

  MVector Vector(1.0, 2.0, 3.0);
  Passed = EvaluateNear("X()", "(1,2,3)", "X returns the x component", Vector.X(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("Y()", "(1,2,3)", "Y returns the y component", Vector.Y(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("Z()", "(1,2,3)", "Z returns the z component", Vector.Z(), 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetX()", "(1,2,3)", "GetX returns the x component", Vector.GetX(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetY()", "(1,2,3)", "GetY returns the y component", Vector.GetY(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetZ()", "(1,2,3)", "GetZ returns the z component", Vector.GetZ(), 3.0, 1e-12) && Passed;

  MVector Copy(Vector);
  Passed = EvaluateTrue("MVector(const MVector&)", "(1,2,3)", "Copy constructor copies all components", Copy == Vector) && Passed;

  Vector.SetX(4.0);
  Vector.SetY(5.0);
  Vector.SetZ(6.0);
  Passed = EvaluateTrue("SetX/Y/Z()", "(4,5,6)", "SetX, SetY and SetZ modify the requested components", Vector == MVector(4.0, 5.0, 6.0)) && Passed;

  Vector.SetXYZ(7.0, 8.0, 9.0);
  Passed = EvaluateTrue("SetXYZ()", "(7,8,9)", "SetXYZ replaces all components", Vector == MVector(7.0, 8.0, 9.0)) && Passed;

  MVector Assigned;
  Assigned.Set(Vector);
  Passed = EvaluateTrue("Set(const MVector&)", "(7,8,9)", "Set copies another vector", Assigned == Vector) && Passed;

  Assigned[0] = -1.0;
  Assigned[1] = -2.0;
  Assigned[2] = -3.0;
  Passed = EvaluateNear("operator[]", "[-1,-2,-3]", "Index operator returns the x component", Assigned[0], -1.0, 1e-12) && Passed;
  Passed = EvaluateNear("operator[]", "[-1,-2,-3]", "Index operator returns the y component", Assigned[1], -2.0, 1e-12) && Passed;
  Passed = EvaluateNear("operator[]", "[-1,-2,-3]", "Index operator returns the z component", Assigned[2], -3.0, 1e-12) && Passed;

  Assigned.Clear();
  Passed = EvaluateTrue("Clear()", "(-1,-2,-3)", "Clear resets the vector to zero", Assigned.IsNull()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test comparisons and simple arithmetic
bool UTVector::TestComparisonAndArithmetic()
{
  bool Passed = true;

  MVector A(1.0, 2.0, 3.0);
  MVector B(4.0, 5.0, 6.0);

  Passed = EvaluateTrue("operator==", "(1,2,3)", "Exact equality compares all components", A == MVector(1.0, 2.0, 3.0)) && Passed;
  Passed = EvaluateTrue("operator!=", "(1,2,3) vs (4,5,6)", "Inequality detects differing components", A != B) && Passed;
  Passed = EvaluateTrue("operator<", "(1,2,3) vs (4,5,6)", "operator< sorts lexicographically by x, then y, then z", A < B) && Passed;
  Passed = EvaluateTrue("AtLeastOneSmaller()", "(4,5,6) vs (1,2,3)", "AtLeastOneSmaller detects whether at least one component of V is smaller than this vector", B.AtLeastOneSmaller(A)) && Passed;
  Passed = EvaluateTrue("AtLeastOneLarger()", "(1,2,3) vs (4,5,6)", "AtLeastOneLarger detects whether at least one component of V is larger than this vector", A.AtLeastOneLarger(B)) && Passed;
  Passed = EvaluateTrue("AllSmaller()", "(4,5,6) vs (1,2,3)", "AllSmaller requires all components of V to be smaller than this vector", B.AllSmaller(A)) && Passed;
  Passed = EvaluateTrue("AllLarger()", "(1,2,3) vs (4,5,6)", "AllLarger requires all components of V to be larger than this vector", A.AllLarger(B)) && Passed;
  Passed = EvaluateTrue("AreEqual()", "tolerance", "AreEqual accepts component-wise tolerance", A.AreEqual(MVector(1.0 + 1e-7, 2.0 - 1e-7, 3.0), 1e-6)) && Passed;

  MVector Sum = A + B;
  Passed = EvaluateTrue("operator+", "(1,2,3) + (4,5,6)", "Addition returns the component-wise sum", Sum == MVector(5.0, 7.0, 9.0)) && Passed;

  MVector Difference = B - A;
  Passed = EvaluateTrue("operator-", "(4,5,6) - (1,2,3)", "Subtraction returns the component-wise difference", Difference == MVector(3.0, 3.0, 3.0)) && Passed;

  MVector Negative = -A;
  Passed = EvaluateTrue("operator-()", "-(1,2,3)", "Unary minus negates all components", Negative == MVector(-1.0, -2.0, -3.0)) && Passed;

  MVector Added = A;
  Added += B;
  Passed = EvaluateTrue("operator+=", "(1,2,3) += (4,5,6)", "operator+= adds in place", Added == MVector(5.0, 7.0, 9.0)) && Passed;

  MVector Subtracted = B;
  Subtracted -= A;
  Passed = EvaluateTrue("operator-=", "(4,5,6) -= (1,2,3)", "operator-= subtracts in place", Subtracted == MVector(3.0, 3.0, 3.0)) && Passed;

  MVector Multiplied = A * 2.0;
  Passed = EvaluateTrue("operator*(MVector,double)", "(1,2,3) * 2", "Right scalar multiplication scales all components", Multiplied == MVector(2.0, 4.0, 6.0)) && Passed;

  MVector LeftMultiplied = 2.0 * A;
  Passed = EvaluateTrue("operator*(double,MVector)", "2 * (1,2,3)", "Left scalar multiplication scales all components", LeftMultiplied == MVector(2.0, 4.0, 6.0)) && Passed;

  MVector Divided = B / 2.0;
  Passed = EvaluateTrue("operator/", "(4,5,6) / 2", "Division scales all components", Divided.AreEqual(MVector(2.0, 2.5, 3.0), 1e-12)) && Passed;

  __merr.Enable(false); // Spits out error message
  MVector DividedByZero = B / 0.0;
  __merr.Enable(true);
  Passed = EvaluateTrue("operator/", "(4,5,6) / 0", "Division by zero returns the null vector", DividedByZero.IsNull()) && Passed;

  MVector InPlaceScaled = A;
  InPlaceScaled *= 3.0;
  Passed = EvaluateTrue("operator*=", "(1,2,3) *= 3", "operator*= scales in place", InPlaceScaled == MVector(3.0, 6.0, 9.0)) && Passed;

  MVector AddedWithMethod(1.0, 2.0, 3.0);
  AddedWithMethod.Add(B);
  Passed = EvaluateTrue("Add()", "(1,2,3) + (4,5,6)", "Add modifies the vector in place", AddedWithMethod == MVector(5.0, 7.0, 9.0)) && Passed;

  MVector SubtractedWithMethod(4.0, 5.0, 6.0);
  SubtractedWithMethod.Subtract(A);
  Passed = EvaluateTrue("Subtract()", "(4,5,6) - (1,2,3)", "Subtract modifies the vector in place", SubtractedWithMethod == MVector(3.0, 3.0, 3.0)) && Passed;

  MVector MultipliedWithMethod(1.0, 2.0, 3.0);
  MultipliedWithMethod.Multiply(2.0);
  Passed = EvaluateTrue("Multiply()", "(1,2,3) * 2", "Multiply scales the vector in place", MultipliedWithMethod == MVector(2.0, 4.0, 6.0)) && Passed;

  Passed = EvaluateNear("Dot()", "(1,2,3) . (4,5,6)", "Dot returns the scalar product", A.Dot(B), 32.0, 1e-12) && Passed;
  Passed = EvaluateNear("operator*(MVector,MVector)", "(1,2,3) * (4,5,6)", "Vector multiplication operator returns the dot product", A * B, 32.0, 1e-12) && Passed;

  MVector Cross = A.Cross(B);
  Passed = EvaluateTrue("Cross()", "(1,2,3) x (4,5,6)", "Cross returns the vector product", Cross.AreEqual(MVector(-3.0, 6.0, -3.0), 1e-12)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test norms, spherical coordinates and orthogonal vectors
bool UTVector::TestGeometry()
{
  bool Passed = true;

  MVector Vector(3.0, 4.0, 0.0);
  Passed = EvaluateNear("Mag2()", "(3,4,0)", "Mag2 returns the squared magnitude", Vector.Mag2(), 25.0, 1e-12) && Passed;
  Passed = EvaluateNear("Mag()", "(3,4,0)", "Mag returns the magnitude", Vector.Mag(), 5.0, 1e-12) && Passed;
  Passed = EvaluateNear("Phi()", "(3,4,0)", "Phi returns the azimuth angle", Vector.Phi(), atan2(4.0, 3.0), 1e-12) && Passed;
  Passed = EvaluateNear("Theta()", "(3,4,0)", "Theta returns the polar angle", Vector.Theta(), c_Pi/2.0, 1e-12) && Passed;
  Passed = EvaluateNear("PhiApproximateMaths()", "(3,4,0)", "PhiApproximateMaths approximates the azimuth angle", Vector.PhiApproximateMaths(), atan2(4.0, 3.0), 2e-3) && Passed;
  Passed = EvaluateNear("ThetaApproximateMaths()", "(3,4,0)", "ThetaApproximateMaths approximates the polar angle", Vector.ThetaApproximateMaths(), c_Pi/2.0, 2e-3) && Passed;

  MVector Unit = Vector.Unit();
  Passed = EvaluateNear("Unit()", "(3,4,0)", "Unit returns a unit vector", Unit.Mag(), 1.0, 1e-12) && Passed;
  Passed = EvaluateTrue("Unit()", "(3,4,0)", "Unit preserves direction", Unit.AreEqual(MVector(0.6, 0.8, 0.0), 1e-12)) && Passed;

  MVector Unitized = Vector;
  Unitized.Unitize();
  Passed = EvaluateNear("Unitize()", "(3,4,0)", "Unitize normalizes in place", Unitized.Mag(), 1.0, 1e-12) && Passed;

  MVector Zero;
  Passed = EvaluateTrue("Unit()", "(0,0,0)", "Unit leaves the zero vector unchanged", Zero.Unit().IsNull()) && Passed;
  Zero.Unitize();
  Passed = EvaluateTrue("Unitize()", "(0,0,0)", "Unitize leaves the zero vector unchanged", Zero.IsNull()) && Passed;

  MVector Absolute(-1.0, 2.0, -3.0);
  Passed = EvaluateTrue("Abs()", "(-1,2,-3)", "Abs returns positive components", Absolute.Abs() == MVector(1.0, 2.0, 3.0)) && Passed;

  MVector Spherical;
  Spherical.SetMagThetaPhi(2.0, c_Pi/2.0, 0.0);
  Passed = EvaluateTrue("SetMagThetaPhi()", "r=2, theta=pi/2, phi=0", "SetMagThetaPhi converts spherical coordinates", Spherical.AreEqual(MVector(2.0, 0.0, 0.0), 1e-12)) && Passed;

  MVector NegativeMagnitude;
  NegativeMagnitude.SetMagThetaPhi(-2.0, c_Pi/2.0, 0.0);
  Passed = EvaluateTrue("SetMagThetaPhi()", "negative magnitude", "SetMagThetaPhi uses the absolute value of the magnitude", NegativeMagnitude.AreEqual(MVector(2.0, 0.0, 0.0), 1e-12)) && Passed;

  MVector SetMag(1.0, 0.0, 0.0);
  SetMag.SetMag(3.0);
  Passed = EvaluateTrue("SetMag()", "(1,0,0) -> r=3", "SetMag preserves the direction", SetMag.AreEqual(MVector(3.0, 0.0, 0.0), 1e-12)) && Passed;

  MVector SetTheta(0.0, 0.0, 1.0);
  SetTheta.SetTheta(c_Pi/2.0);
  Passed = EvaluateTrue("SetTheta()", "(0,0,1) -> theta=pi/2", "SetTheta preserves magnitude and phi", SetTheta.AreEqual(MVector(1.0, 0.0, 0.0), 1e-12)) && Passed;

  MVector SetPhi(1.0, 0.0, 0.0);
  SetPhi.SetPhi(c_Pi/2.0);
  Passed = EvaluateTrue("SetPhi()", "(1,0,0) -> phi=pi/2", "SetPhi preserves magnitude and theta", SetPhi.AreEqual(MVector(0.0, 1.0, 0.0), 1e-12)) && Passed;

  Passed = EvaluateNear("Angle()", "x vs y", "Angle returns pi/2 for orthogonal vectors", MVector(1.0, 0.0, 0.0).Angle(MVector(0.0, 1.0, 0.0)), c_Pi/2.0, 1e-12) && Passed;
  Passed = EvaluateNear("Angle()", "x vs x", "Angle returns zero for identical directions", MVector(1.0, 0.0, 0.0).Angle(MVector(1.0, 0.0, 0.0)), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Angle()", "zero vector", "Angle returns zero if one vector has zero length", Zero.Angle(MVector(1.0, 0.0, 0.0)), 0.0, 1e-12) && Passed;

  MVector Orthogonal = MVector(1.0, 2.0, 3.0).Orthogonal();
  Passed = EvaluateNear("Orthogonal()", "(1,2,3)", "Orthogonal returns a perpendicular vector", Orthogonal.Dot(MVector(1.0, 2.0, 3.0)), 0.0, 1e-12) && Passed;
  Passed = EvaluateTrue("IsOrthogonal()", "x vs y", "IsOrthogonal detects perpendicular vectors", MVector(1.0, 0.0, 0.0).IsOrthogonal(MVector(0.0, 1.0, 0.0))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test rotations and high-level geometric relations
bool UTVector::TestRotationsAndRelations()
{
  bool Passed = true;

  MVector RotateZ(1.0, 0.0, 0.0);
  RotateZ.RotateZ(c_Pi/2.0);
  Passed = EvaluateTrue("RotateZ()", "(1,0,0) by pi/2", "RotateZ performs the expected planar rotation", RotateZ.AreEqual(MVector(0.0, 1.0, 0.0), 1e-12)) && Passed;

  MVector RotateY(0.0, 0.0, 1.0);
  RotateY.RotateY(c_Pi/2.0);
  Passed = EvaluateTrue("RotateY()", "(0,0,1) by pi/2", "RotateY performs the expected planar rotation", RotateY.AreEqual(MVector(1.0, 0.0, 0.0), 1e-12)) && Passed;

  MVector RotateX(0.0, 1.0, 0.0);
  RotateX.RotateX(c_Pi/2.0);
  Passed = EvaluateTrue("RotateX()", "(0,1,0) by pi/2", "RotateX performs the expected planar rotation", RotateX.AreEqual(MVector(0.0, 0.0, 1.0), 1e-12)) && Passed;

  MVector AroundAxis(1.0, 0.0, 0.0);
  AroundAxis.RotateAroundVector(MVector(0.0, 0.0, 1.0), c_Pi/2.0);
  Passed = EvaluateTrue("RotateAroundVector()", "x around z by pi/2", "RotateAroundVector rotates around the given axis", AroundAxis.AreEqual(MVector(0.0, 1.0, 0.0), 1e-12)) && Passed;

  MVector Reference(0.0, 0.0, 1.0);
  Reference.RotateReferenceFrame(MVector(1.0, 0.0, 0.0));
  Passed = EvaluateTrue("RotateReferenceFrame()", "z into x frame", "RotateReferenceFrame aligns z with the target direction", Reference.AreEqual(MVector(1.0, 0.0, 0.0), 1e-12)) && Passed;

  Passed = EvaluateTrue("Coplanar()", "z=0 plane", "Coplanar accepts vectors in the same plane", MVector(0.25, 0.25, 0.0).Coplanar(MVector(0.0, 0.0, 0.0), MVector(1.0, 0.0, 0.0), MVector(0.0, 1.0, 0.0))) && Passed;
  mout.Enable(false); // Spits out error message
  Passed = EvaluateFalse("Coplanar()", "non-planar point", "Coplanar rejects vectors outside the plane", MVector(0.0, 0.0, 1.0).Coplanar(MVector(0.0, 0.0, 0.0), MVector(1.0, 0.0, 0.0), MVector(0.0, 1.0, 0.0))) && Passed;
  mout.Enable(true);

  Passed = EvaluateNear("DistanceToLine()", "point to x-axis", "DistanceToLine returns the perpendicular distance", MVector(0.0, 1.0, 0.0).DistanceToLine(MVector(0.0, 0.0, 0.0), MVector(1.0, 0.0, 0.0)), 1.0, 1e-12) && Passed;

  MVector ZAxis(0.3, 0.4, sqrt(1.0 - 0.3*0.3 - 0.4*0.4));
  MVector XAxis = ZAxis.Orthogonal();
  XAxis.Unitize();
  MVector YAxis = ZAxis.Cross(XAxis);
  YAxis.Unitize();
  Passed = EvaluateNear("Orthogonal()/Cross()", "orientation basis", "Orthogonal plus Cross can build a right-handed detector basis with perpendicular axes", XAxis.Dot(YAxis), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Orthogonal()/Cross()", "orientation basis", "The derived Y axis is perpendicular to Z", YAxis.Dot(ZAxis), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Orthogonal()/Cross()", "orientation basis", "All basis vectors are normalized for orientation-style use", XAxis.Mag(), 1.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test string formatting and stream output
bool UTVector::TestFormatting()
{
  bool Passed = true;

  MVector Vector(1.0, 2.0, 3.0);
  Passed = Evaluate("ToString()", "(1,2,3)", "ToString formats the vector as a coordinate tuple", Vector.ToString(), MString("(1, 2, 3)")) && Passed;

  ostringstream Out;
  Out << Vector;
  Passed = Evaluate("operator<<", "(1,2,3)", "Stream output formats the vector as a coordinate tuple", MString(Out.str()), MString("(1, 2, 3)")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test edge cases and degenerate geometry
bool UTVector::TestEdgeCases()
{
  bool Passed = true;

  MVector Zero;
  Passed = EvaluateNear("PhiApproximateMaths()", "(0,0,0)", "PhiApproximateMaths returns zero for the null vector", Zero.PhiApproximateMaths(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("ThetaApproximateMaths()", "(0,0,0)", "ThetaApproximateMaths returns zero for the null vector", Zero.ThetaApproximateMaths(), 0.0, 1e-12) && Passed;
  Passed = EvaluateTrue("Orthogonal()", "(0,0,0)", "Orthogonal returns the null vector for the null vector", Zero.Orthogonal().IsNull()) && Passed;

  MVector SetThetaZero;
  SetThetaZero.SetTheta(c_Pi/3.0);
  Passed = EvaluateTrue("SetTheta()", "(0,0,0)", "SetTheta leaves the null vector unchanged", SetThetaZero.IsNull()) && Passed;

  MVector SetPhiZero;
  SetPhiZero.SetPhi(c_Pi/3.0);
  Passed = EvaluateTrue("SetPhi()", "(0,0,0)", "SetPhi leaves the null vector unchanged", SetPhiZero.IsNull()) && Passed;

  MVector SetMagZero;
  SetMagZero.SetMag(5.0);
  Passed = EvaluateTrue("SetMag()", "(0,0,0)", "SetMag on the null vector uses the default zero angles and points along +z", SetMagZero.AreEqual(MVector(0.0, 0.0, 5.0), 1e-12)) && Passed;

  MVector SouthPole(1.0, 2.0, 3.0);
  SouthPole.RotateReferenceFrame(MVector(0.0, 0.0, -1.0));
  Passed = EvaluateTrue("RotateReferenceFrame()", "south pole", "RotateReferenceFrame handles the south-pole special case", SouthPole.AreEqual(MVector(-1.0, 2.0, -3.0), 1e-12)) && Passed;

  MVector ZeroAxis(1.0, 0.0, 0.0);
  ZeroAxis.RotateAroundVector(MVector(0.0, 0.0, 0.0), c_Pi/4.0);
  Passed = EvaluateTrue("RotateAroundVector()", "zero axis", "RotateAroundVector with a zero axis leaves the vector finite", std::isfinite(ZeroAxis.X()) && std::isfinite(ZeroAxis.Y()) && std::isfinite(ZeroAxis.Z())) && Passed;

  Passed = EvaluateNear("DistanceToLine()", "degenerate line", "DistanceToLine with identical line points returns the point-to-point distance", MVector(1.0, 1.0, 0.0).DistanceToLine(MVector(0.0, 0.0, 0.0), MVector(0.0, 0.0, 0.0)), sqrt(2.0), 1e-12) && Passed;

  Passed = EvaluateTrue("Coplanar()", "degenerate plane", "Coplanar accepts degenerate identical points", MVector(0.0, 0.0, 0.0).Coplanar(MVector(0.0, 0.0, 0.0), MVector(0.0, 0.0, 0.0), MVector(0.0, 0.0, 0.0))) && Passed;

  Passed = EvaluateNear("Angle()", "nearly parallel", "Angle remains finite and near zero for nearly parallel vectors", MVector(1.0, 0.0, 0.0).Angle(MVector(1.0, 1e-15, 0.0)), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Angle()", "antiparallel", "Angle returns pi for opposite directions", MVector(1.0, 0.0, 0.0).Angle(MVector(-1.0, 0.0, 0.0)), c_Pi, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Main function
int main()
{
  UTVector Test;
  bool Passed = Test.Run();

  return Passed ? 0 : 1;
}


////////////////////////////////////////////////////////////////////////////////
