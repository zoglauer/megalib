/*
 * UTDShapeTRAP.cxx
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
#include "MDShapeTRAP.h"
#include "UTDShapeShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the general trapezoid shape MDShapeTRAP
class UTDShapeTRAP : public UTDShapeBase
{
public:
  //! Default constructor
  UTDShapeTRAP() : UTDShapeBase("UTDShapeTRAP") {}
  //! Default destructor
  virtual ~UTDShapeTRAP() {}

  //! Run all tests
  virtual bool Run();

private:

  //! Test the default state
  bool TestDefault();
  //! Test the limits of the parameters
  bool TestLimits();
  //! Test the normalization of the angles
  bool TestAngles();
  //! Test the planarity check of the side surfaces
  bool TestPlanarity();
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
bool UTDShapeTRAP::Run()
{
  bool Passed = true;

  Passed = TestDefault() && Passed;
  Passed = TestLimits() && Passed;
  Passed = TestAngles() && Passed;
  Passed = TestPlanarity() && Passed;
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


//! Test the default state
bool UTDShapeTRAP::TestDefault()
{
  bool Passed = true;

  MDShapeTRAP Shape("Trap");
  Passed = EvaluateTrue("GetName()", "constructor", "The name of the constructor is stored", Shape.GetName() == "Trap") && Passed;
  Passed = EvaluateTrue("GetType()", "constructor", "The type is TRAP", Shape.GetType() == "TRAP") && Passed;
  Passed = EvaluateNear("GetVolume()", "default", "A new shape has no volume", Shape.GetVolume(), 0.0, 1e-15) && Passed;

  // Defect GEO-C-7: A shape without parameters passes the validation and becomes a zero-size shape
  MDShapeTRAP NoParameters("NoParameters");
  Passed = ExpectedToFail(EvaluateFalse("Validate()", "no parameters", "A shape without parameters is rejected", Quiet([&]() { return NoParameters.Validate(); }))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the limits of the parameters
bool UTDShapeTRAP::TestLimits()
{
  bool Passed = true;

  const double Just = 1e-9;
  MDShapeTRAP S("Trap");

  // All lengths are positive, the triangular faces can have one zero value:
  Passed = EvaluateTrue("Set()", "Dz just above 0", "A small Dz is accepted", Quiet([&]() { return S.Set(Just, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "Dz = 0", "A zero Dz is rejected", Quiet([&]() { return S.Set(0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "Dz just below 0", "A negative Dz is rejected", Quiet([&]() { return S.Set(-Just, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0); })) && Passed;

  Passed = EvaluateTrue("Set()", "H1 = 0", "A zero H1 gives a wedge", Quiet([&]() { return S.Set(1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "H1 just below 0", "A negative H1 is rejected", Quiet([&]() { return S.Set(1, 0, 0, -Just, 1, 1, 0, 1, 1, 1, 0); })) && Passed;
  Passed = EvaluateTrue("Set()", "H2 = 0", "A zero H2 gives a wedge", Quiet([&]() { return S.Set(1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "H2 just below 0", "A negative H2 is rejected", Quiet([&]() { return S.Set(1, 0, 0, 1, 1, 1, 0, -Just, 1, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "H1 = H2 = 0", "Both H zero are rejected", Quiet([&]() { return S.Set(1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0); })) && Passed;

  Passed = EvaluateFalse("Set()", "Bl1 just below 0", "A negative Bl1 is rejected", Quiet([&]() { return S.Set(1, 0, 0, 1, -Just, 1, 0, 1, 1, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "Tl1 just below 0", "A negative Tl1 is rejected", Quiet([&]() { return S.Set(1, 0, 0, 1, 1, -Just, 0, 1, 1, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "Bl2 just below 0", "A negative Bl2 is rejected", Quiet([&]() { return S.Set(1, 0, 0, 1, 1, 1, 0, 1, -Just, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "Tl2 just below 0", "A negative Tl2 is rejected", Quiet([&]() { return S.Set(1, 0, 0, 1, 1, 1, 0, 1, 1, -Just, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "Bl1 = Bl2 = 0", "Both Bl zero are rejected", Quiet([&]() { return S.Set(1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "Tl1 = Tl2 = 0", "Both Tl zero are rejected", Quiet([&]() { return S.Set(1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0); })) && Passed;

  // A rejected set does not change the shape:
  S.Set(1, 0, 0, 2, 3, 3, 0, 2, 3, 3, 0);
  Quiet([&]() { return S.Set(-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1); });
  Passed = EvaluateNear("GetDz()", "after reject", "The old Dz is kept", S.GetDz(), 1.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetH1()", "after reject", "The old H1 is kept", S.GetH1(), 2.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetBl1()", "after reject", "The old Bl1 is kept", S.GetBl1(), 3.0, 1e-15) && Passed;

  // Defect GEO-C-26: A very large angle never leaves the normalization loop since the subtraction of 180 changes nothing
  MDShapeTRAP Hang("Hang");
  Passed = ExpectedToCrash(EvaluateFalse("Set()", "Theta = 1e300", "A very large angle is rejected and does not hang the program", Hang.Set(1, 1e300, 0, 1, 1, 1, 0, 1, 1, 1, 0))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the normalization of the angles
bool UTDShapeTRAP::TestAngles()
{
  bool Passed = true;

  MDShapeTRAP S("Trap");

  S.Set(1, 190, 390, 1, 1, 1, 370, 1, 1, 1, 370);
  Passed = EvaluateNear("GetTheta()", "190", "Theta is moved below 180", S.GetTheta(), 10.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetPhi()", "390", "Phi is moved below 360", S.GetPhi(), 30.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetAlpha1()", "370", "Alpha1 is moved below 360", S.GetAlpha1(), 10.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetAlpha2()", "370", "Alpha2 is moved below 360", S.GetAlpha2(), 10.0, 1e-12) && Passed;

  S.Set(1, -10, -30, 1, 1, 1, -30, 1, 1, 1, -30);
  Passed = EvaluateNear("GetTheta()", "-10", "Theta is moved above 0", S.GetTheta(), 170.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetPhi()", "-30", "Phi is moved above 0", S.GetPhi(), 330.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetAlpha1()", "-30", "A negative Alpha1 above -360 stays", S.GetAlpha1(), -30.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetAlpha2()", "-30", "A negative Alpha2 above -360 stays", S.GetAlpha2(), -30.0, 1e-12) && Passed;

  // The limits of the moves:
  S.Set(1, 180, 360, 1, 1, 1, 360, 1, 1, 1, 360);
  Passed = EvaluateNear("GetTheta()", "180", "Theta = 180 is the same as 0", S.GetTheta(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetPhi()", "360", "Phi = 360 is the same as 0", S.GetPhi(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetAlpha1()", "360", "Alpha1 = 360 is the same as 0", S.GetAlpha1(), 0.0, 1e-12) && Passed;
  S.Set(1, 0, 0, 1, 1, 1, -370, 1, 1, 1, -370);
  Passed = EvaluateNear("GetAlpha1()", "-370", "Alpha1 is moved above -360", S.GetAlpha1(), -10.0, 1e-12) && Passed;

  // The negative theta keeps the geometry, tan(170) = tan(-10)
  MDShapeTRAP Negative("Negative");
  Negative.Set(2, -10, 0, 1, 1, 1, 0, 1, 1, 1, 0);
  Negative.Validate();
  Passed = Evaluate("IsInside()", "Theta = -10", "The shape follows the analytic truth", GetNumberOfInsideDifferences(Negative, GetTrapTruth(2, -10, 0, 1, 1, 1, 0, 1, 1, 1, 0), MVector(-3, -2, -3), MVector(3, 2, 3)), 0U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the planarity check of the side surfaces
bool UTDShapeTRAP::TestPlanarity()
{
  bool Passed = true;

  MDShapeTRAP S("Trap");

  // A planar frustum: same ratio (Bl - Tl) / H on both faces and the same alpha
  Passed = EvaluateTrue("Set()", "planar", "A shape with planar sides is accepted", Quiet([&]() { return S.Set(3, 20, 30, 2, 1, 1.5, 10, 4, 2, 3, 10); })) && Passed;

  // Non planar: Tl2 is off the planar value 3 by a lot
  Passed = EvaluateFalse("Set()", "not planar", "A shape with twisted sides is rejected", Quiet([&]() { return S.Set(3, 0, 0, 2, 1, 1.5, 0, 4, 2, 2, 0); })) && Passed;

  // The limit of 1e-5 rad: Normals differ by 0.167 rad per unit deviation of Tl2 (independent replication of the angle calculation in a script)
  Passed = EvaluateTrue("Set()", "Tl2 off by 5e-5", "A deviation just inside the limit is accepted", Quiet([&]() { return S.Set(3, 0, 0, 2, 1, 1.5, 0, 4, 2, 3 + 5e-5, 0); })) && Passed;
  Passed = EvaluateFalse("Set()", "Tl2 off by 7e-5", "A deviation just outside the limit is rejected", Quiet([&]() { return S.Set(3, 0, 0, 2, 1, 1.5, 0, 4, 2, 3 + 7e-5, 0); })) && Passed;

  // Different alpha: top and bottom sides are not parallel
  Passed = EvaluateFalse("Set()", "alpha 10 and 20", "A shape with different alpha is rejected", Quiet([&]() { return S.Set(3, 0, 0, 2, 1, 1, 10, 2, 1, 1, 20); })) && Passed;

  // Defect GEO-C-4: The check uses tan(angle*c_Deg) and not tan(angle*c_Rad), thus angles with a difference of pi*c_Rad have the same wrong tangent
  Passed = ExpectedToFail(EvaluateFalse("Set()", "alpha 10 and 10 + 0.0548", "A shape with different alpha is rejected", Quiet([&]() { return S.Set(3, 0, 0, 2, 1, 1, 10, 2, 1, 1, 10 + c_Pi*c_Rad); }))) && Passed;

  // Defect GEO-C-4: Identical sides in the sense of the true tangent, tan(190 deg) = tan(10 deg), are planar, but rejected
  Passed = ExpectedToFail(EvaluateTrue("Set()", "alpha 10 and 190", "A shape with planar sides is accepted", Quiet([&]() { return S.Set(3, 0, 0, 2, 1, 1, 10, 2, 1, 1, 190); }))) && Passed;

  // Defect GEO-C-27: A theta of 90 deg has an infinite tangent and thus no valid shape, but it is accepted
  Passed = ExpectedToFail(EvaluateFalse("Set()", "Theta = 90", "A shape with an infinite shear is rejected", Quiet([&]() { return S.Set(3, 90, 0, 2, 1, 1, 0, 2, 1, 1, 0); }))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the volume and the size
bool UTDShapeTRAP::TestVolumeSize()
{
  bool Passed = true;

  // A box: 2 Bl 2 H 2 Dz
  MDShapeTRAP Box("Box");
  Box.Set(3, 0, 0, 2, 1, 1, 0, 2, 1, 1, 0);
  Passed = EvaluateNear("GetVolume()", "box", "The volume of equal lengths is the one of a box", Box.GetVolume(), 2*4*6.0, 1e-12) && Passed;

  // Shear does not change the volume:
  MDShapeTRAP Shear("Shear");
  Shear.Set(3, 20, 30, 2, 1, 1, 10, 2, 1, 1, 10);
  Passed = EvaluateNear("GetVolume()", "sheared box", "A shear does not change the volume", Shear.GetVolume(), 2*4*6.0, 1e-12) && Passed;

  // A trapezoidal prism: (2 Bl + 2 Tl) / 2 * 2H * 2Dz
  MDShapeTRAP Prism("Prism");
  Prism.Set(3, 0, 0, 2, 1, 2, 0, 2, 1, 2, 0);
  Passed = EvaluateNear("GetVolume()", "prism", "The volume is the area of the trapezoid times the length", Prism.GetVolume(), 3.0*4*6, 1e-12) && Passed;

  // A truncated pyramid with areas A1, A2: h/3 (A1 + A2 + sqrt(A1 A2))
  MDShapeTRAP Frustum("Frustum");
  Frustum.Set(3, 0, 0, 2, 2, 2, 0, 1, 1, 1, 0);
  Passed = EvaluateNear("GetVolume()", "frustum", "The volume of a truncated pyramid", Frustum.GetVolume(), 6.0/3*(16 + 4 + 8), 1e-12) && Passed;

  // Defect GEO-C-15: The size is not implemented and returns zero with an error message
  Passed = ExpectedToFail(EvaluateTrue("GetSize()", "box", "The size is the one of the surrounding box", Quiet([&]() { return (Box.GetSize() - MVector(1, 2, 3)).Mag() < 1e-12; }))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the inside check
bool UTDShapeTRAP::TestIsInside()
{
  bool Passed = true;

  const double Just = 1e-9;

  // A straight prism with the half length 1 at -H and 2 at +H, i.e. 1.5 at y = 0
  MDShapeTRAP Shape("Trap");
  Shape.Set(3, 0, 0, 2, 1, 2, 0, 2, 1, 2, 0);
  Shape.Validate();

  Passed = EvaluateTrue("IsInside()", "center", "The center is inside", Shape.IsInside(MVector(0, 0, 0))) && Passed;
  Passed = EvaluateTrue("IsInside()", "non-symmetric", "A non-symmetric interior point is inside", Shape.IsInside(MVector(-1.7, 1.5, 2.5))) && Passed;
  Passed = EvaluateFalse("IsInside()", "wide side at -H", "The long side does not exist at -H", Shape.IsInside(MVector(1.7, -1.5, 2.5))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just inside x", "Just inside in x is inside", Shape.IsInside(MVector(1.5 - Just, 0, 0))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just outside x", "Just outside in x is outside", Shape.IsInside(MVector(1.5 + Just, 0, 0))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just inside y", "Just inside in y is inside", Shape.IsInside(MVector(0, 2 - Just, 0))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just outside y", "Just outside in y is outside", Shape.IsInside(MVector(0, 2 + Just, 0))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just inside z", "Just inside in z is inside", Shape.IsInside(MVector(0, 0, 3 - Just))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just outside z", "Just outside in z is outside", Shape.IsInside(MVector(0, 0, 3 + Just))) && Passed;

  Passed = CheckToleranceAtSurface("top", Shape, MVector(0, 0, 3), MVector(0, 0, 1)) && Passed;
  Passed = CheckToleranceAtSurface("+y", Shape, MVector(0, 2, 0), MVector(0, 1, 0)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the distance functions with fixed rays
bool UTDShapeTRAP::TestDistances()
{
  bool Passed = true;

  MDShapeTRAP Shape("Trap");
  Shape.Set(3, 0, 0, 2, 1, 2, 0, 2, 1, 2, 0);
  Shape.Validate();

  Passed = EvaluateNear("DistanceInsideOut()", "+x at y = 0", "The half length at y = 0 is 1.5", Shape.DistanceInsideOut(MVector(0, 0, 0), MVector(1, 0, 0)), 1.5, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "+y", "The distance to +y", Shape.DistanceInsideOut(MVector(0, 0.5, 0), MVector(0, 1, 0)), 1.5, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "-z", "The distance to the bottom", Shape.DistanceInsideOut(MVector(0, 0, 1), MVector(0, 0, -1)), 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("DistanceOutsideIn()", "-x", "The distance from outside", Shape.DistanceOutsideIn(MVector(5, 0, 0), MVector(-1, 0, 0)), 3.5, 1e-12) && Passed;
  Passed = EvaluateTrue("DistanceOutsideIn()", "miss", "A ray away from the shape is no hit", Shape.DistanceOutsideIn(MVector(5, 0, 0), MVector(1, 0, 0)) > 1e20) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the unique position
bool UTDShapeTRAP::TestUniquePosition()
{
  bool Passed = true;

  // The middle of a sheared shape at z = 0 is the origin by the definition of the tilt
  MDShapeTRAP Shape("Trap");
  Shape.Set(3, 20, 30, 2, 1, 1.5, 10, 4, 2, 3, 10);
  Shape.Validate();
  Passed = EvaluateVectorNear("GetUniquePosition()", "sheared", "The position is the center", Shape.GetUniquePosition(), MVector(0, 0, 0), 1e-15) && Passed;
  Passed = EvaluateTrue("GetUniquePosition()", "sheared", "The position is inside", Shape.IsInside(Shape.GetUniquePosition())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the geometry against the analytic truth
bool UTDShapeTRAP::TestAgainstTruth()
{
  bool Passed = true;

  // A sheared one with faces of different sizes:
  MDShapeTRAP Shape("Trap");
  Shape.Set(3, 20, 30, 2, 1, 1.5, 10, 4, 2, 3, 10);
  Shape.Validate();
  MTruth Truth = GetTrapTruth(3, 20, 30, 2, 1, 1.5, 10, 4, 2, 3, 10);
  const MVector Min(-7, -6, -3.5);
  const MVector Max(7, 6, 3.5);
  Passed = CheckTruthVolume("sheared", Truth, Min, Max, Shape.GetVolume()) && Passed;
  Passed = CheckAgainstTruth("sheared", Shape, Truth, Min, Max) && Passed;

  // A wedge with H1 = 0:
  MDShapeTRAP Wedge("Wedge");
  Wedge.Set(3, 0, 0, 0, 2, 2, 0, 2, 2, 2, 0);
  Wedge.Validate();
  MTruth TruthWedge = GetTrapTruth(3, 0, 0, 0, 2, 2, 0, 2, 2, 2, 0);
  const MVector MinWedge(-2.5, -2.5, -3.5);
  const MVector MaxWedge(2.5, 2.5, 3.5);
  Passed = CheckTruthVolume("wedge", TruthWedge, MinWedge, MaxWedge, Wedge.GetVolume()) && Passed;
  Passed = CheckInsideAgainstTruth("wedge", Wedge, TruthWedge, MinWedge, MaxWedge) && Passed;
  Passed = CheckRandomPositions("wedge", Wedge, TruthWedge, MinWedge, MaxWedge) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the string representations
bool UTDShapeTRAP::TestStrings()
{
  bool Passed = true;

  MDShapeTRAP Shape("Trap");
  Shape.Set(3, 20, 30, 2, 1, 1.5, 10, 4, 2, 3, 10);
  Passed = EvaluateTrue("ToString()", "shape", "The string contains type and parameters", Shape.ToString().Contains("TRAP (3, 20, 30, 2, 1, 1.5, 10, 4, 2, 3, 10)")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "shape", "The text is the keyword with the parameters", Shape.GetGeomega() == "TRAP 3 20 30 2 1 1.5 10 4 2 3 10") && Passed;

  MDShapeTRAP Read("Read");
  Passed = EvaluateTrue("Parse()", "round trip", "The text can be parsed", ParseQuiet(Read, MString("V.Shape ") + Shape.GetGeomega())) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "Dz survives", Read.GetDz(), 3.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "Theta survives", Read.GetTheta(), 20.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "Phi survives", Read.GetPhi(), 30.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "H1 survives", Read.GetH1(), 2.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "Bl1 survives", Read.GetBl1(), 1.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "Tl1 survives", Read.GetTl1(), 1.5, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "Alpha1 survives", Read.GetAlpha1(), 10.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "H2 survives", Read.GetH2(), 4.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "Bl2 survives", Read.GetBl2(), 2.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "Tl2 survives", Read.GetTl2(), 3.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "Alpha2 survives", Read.GetAlpha2(), 10.0, 1e-4) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the scaling
bool UTDShapeTRAP::TestScale()
{
  bool Passed = true;

  // All axes keep the angles:
  MDShapeTRAP All("All");
  All.Set(3, 20, 30, 2, 1, 1.5, 10, 4, 2, 3, 10);
  All.Validate();
  double Volume = All.GetVolume();
  Passed = CheckScaleState("XYZ", All, 2.0, "XYZ") && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "Dz is scaled", All.GetDz(), 6.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "H1 is scaled", All.GetH1(), 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "Bl2 is scaled", All.GetBl2(), 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "Theta is not scaled", All.GetTheta(), 20.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "The volume is scaled by the third power", All.GetVolume(), 8*Volume, 1e-9) && Passed;
  Passed = CheckInsideAgainstTruth("XYZ", All, GetTrapTruth(6, 20, 30, 4, 2, 3, 10, 8, 4, 6, 10), MVector(-14, -12, -7), MVector(14, 12, 7)) && Passed;

  // A straight shape has no angles to care about:
  MDShapeTRAP X("X");
  X.Set(3, 0, 0, 2, 1, 1.5, 0, 4, 2, 3, 0);
  X.Validate();
  Passed = EvaluateTrue("Scale()", "X", "The axis X is accepted", X.Scale(2.0, "X")) && Passed;
  Passed = EvaluateNear("Scale()", "X", "Bl1 is scaled", X.GetBl1(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "X", "Tl2 is scaled", X.GetTl2(), 6.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "X", "H1 is not scaled", X.GetH1(), 2.0, 1e-12) && Passed;
  Passed = Evaluate("IsInside()", "X, straight", "The shape is the stretched analytic truth", GetNumberOfInsideDifferences(X, GetTrapTruth(3, 0, 0, 2, 2, 3, 0, 4, 4, 6, 0), MVector(-7, -5, -4), MVector(7, 5, 4)), 0U) && Passed;

  MDShapeTRAP Y("Y");
  Y.Set(3, 0, 0, 2, 1, 1.5, 0, 4, 2, 3, 0);
  Y.Validate();
  Passed = EvaluateTrue("Scale()", "Y", "The axis Y is accepted", Y.Scale(2.0, "Y")) && Passed;
  Passed = EvaluateNear("Scale()", "Y", "H1 is scaled", Y.GetH1(), 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "Y", "H2 is scaled", Y.GetH2(), 8.0, 1e-12) && Passed;

  MDShapeTRAP Z("Z");
  Z.Set(3, 0, 0, 2, 1, 1.5, 0, 4, 2, 3, 0);
  Z.Validate();
  Passed = EvaluateTrue("Scale()", "Z", "The axis Z is accepted", Z.Scale(2.0, "Z")) && Passed;
  Passed = EvaluateNear("Scale()", "Z", "Dz is scaled", Z.GetDz(), 6.0, 1e-12) && Passed;

  // Defect GEO-C-14: Scaling only some axes keeps the angles, the shape is not the stretched one, the shift of the faces has to follow x only
  MDShapeTRAP ShearX("ShearX");
  ShearX.Set(3, 20, 30, 2, 1, 1, 10, 2, 1, 1, 10);
  ShearX.Validate();
  ShearX.Scale(2.0, "X");
  double Tilt = tan(20*c_Rad);
  MTruth StretchedX = [=](double x, double y, double z) {
    // Inverse of the stretch of x by 2 of the original shape with the shift along x
    double Xo = x/2 - z*Tilt*cos(30*c_Rad);
    double Yo = y - z*Tilt*sin(30*c_Rad);
    Xo -= Yo*tan(10*c_Rad);
    if (fabs(z) > 3 || fabs(Yo) > 2 || fabs(Xo) > 1) {
      return false;
    }
    return true;
  };
  Passed = ExpectedToFail(Evaluate("Scale()", "X, sheared", "The shape is the stretched analytic truth", GetNumberOfInsideDifferences(ShearX, StretchedX, MVector(-12, -6, -3.5), MVector(12, 6, 3.5)), 0U)) && Passed;

  // Defect GEO-C-14: Scaling z only must keep the shift of the faces in x and y
  MDShapeTRAP ShearZ("ShearZ");
  ShearZ.Set(3, 20, 0, 2, 1, 1, 0, 2, 1, 1, 0);
  ShearZ.Validate();
  ShearZ.Scale(2.0, "Z");
  MTruth StretchedZ = [=](double x, double y, double z) {
    // Inverse of the stretch of z by 2
    double Xo = x - (z/2)*Tilt;
    if (fabs(z) > 6 || fabs(y) > 2 || fabs(Xo) > 1) {
      return false;
    }
    return true;
  };
  Passed = ExpectedToFail(Evaluate("Scale()", "Z, sheared", "The shape is the stretched analytic truth", GetNumberOfInsideDifferences(ShearZ, StretchedZ, MVector(-4, -3, -6.5), MVector(4, 3, 6.5)), 0U)) && Passed;

  // Defect GEO-C-5: A negative factor is accepted while Set() rejects negative lengths
  MDShapeTRAP Negative("Negative");
  Negative.Set(3, 0, 0, 2, 1, 1, 0, 2, 1, 1, 0);
  Negative.Validate();
  Passed = ExpectedToFail(EvaluateFalse("Scale()", "factor -2", "A negative factor is rejected", Negative.Scale(-2.0, "XYZ"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parsing the keywords
bool UTDShapeTRAP::TestParse()
{
  bool Passed = true;

  MDShapeTRAP Parameters("Trap");
  Passed = EvaluateTrue("Parse()", "Parameters", "The keyword Parameters is accepted", ParseQuiet(Parameters, "Trap.Parameters 3 20 30 2 1 1.5 10 4 2 3 10")) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "Dz is read", Parameters.GetDz(), 3.0, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "Theta is read", Parameters.GetTheta(), 20.0, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "Phi is read", Parameters.GetPhi(), 30.0, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "H1 is read", Parameters.GetH1(), 2.0, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "Bl1 is read", Parameters.GetBl1(), 1.0, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "Tl1 is read", Parameters.GetTl1(), 1.5, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "H2 is read", Parameters.GetH2(), 4.0, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "Bl2 is read", Parameters.GetBl2(), 2.0, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "Tl2 is read", Parameters.GetTl2(), 3.0, 1e-15) && Passed;

  MDShapeTRAP Keyword("Keyword");
  Passed = EvaluateTrue("Parse()", "Shape", "The keyword Shape is accepted", ParseQuiet(Keyword, "V.Shape TRAP 3 20 30 2 1 1.5 10 4 2 3 10")) && Passed;
  Passed = EvaluateNear("Parse()", "Shape", "The parameters start after the type", Keyword.GetTl2(), 3.0, 1e-15) && Passed;

  MDShapeTRAP Bad("Bad");
  Bad.Set(1, 0, 0, 2, 3, 3, 0, 2, 3, 3, 0);
  Passed = EvaluateFalse("Parse()", "10 parameters", "Ten parameters are rejected", ParseQuiet(Bad, "Bad.Parameters 3 20 30 2 1 1.5 10 4 2 3")) && Passed;
  Passed = EvaluateFalse("Parse()", "12 parameters", "Twelve parameters are rejected", ParseQuiet(Bad, "Bad.Parameters 3 20 30 2 1 1.5 10 4 2 3 10 11")) && Passed;
  Passed = EvaluateFalse("Parse()", "wrong keyword", "A wrong keyword is rejected", ParseQuiet(Bad, "Bad.Param 3 20 30 2 1 1.5 10 4 2 3 10")) && Passed;
  Passed = EvaluateFalse("Parse()", "invalid", "An invalid value is rejected", ParseQuiet(Bad, "Bad.Parameters 0 20 30 2 1 1.5 10 4 2 3 10")) && Passed;
  Passed = EvaluateFalse("Parse()", "not planar", "A shape which is not planar is rejected", ParseQuiet(Bad, "Bad.Parameters 3 0 0 2 1 1.5 0 4 2 2 0")) && Passed;
  Passed = EvaluateNear("Parse()", "malformed", "The rejected input keeps the values", Bad.GetDz(), 1.0, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test configuring a validated shape again
bool UTDShapeTRAP::TestReconfiguration()
{
  MDShapeTRAP Shape("S");
  return CheckReconfiguration("reconfiguration", Shape, MVector(0.0, 0.0, 1.0), [&Shape]() { return Shape.Set(1.0, 0.0, 0.0, 2.0, 2.0, 2.0, 0.0, 2.0, 2.0, 2.0, 0.0); }, 1.0, [&Shape]() { return Shape.Set(2.0, 0.0, 0.0, 2.0, 2.0, 2.0, 0.0, 2.0, 2.0, 2.0, 0.0); }, 2.0, "S.Parameters 3 0 0 2 2 2 0 2 2 2 0", 3.0, [&Shape]() { return Shape.Set(-1.0, 0.0, 0.0, 2.0, 2.0, 2.0, 0.0, 2.0, 2.0, 2.0, 0.0); });
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  gErrorIgnoreLevel = kWarning;

  UTDShapeTRAP Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDShapeTRAP.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
