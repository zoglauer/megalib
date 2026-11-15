/*
 * UTDShapePCON.cxx
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
#include <iostream>
#include <sstream>

// ROOT libs:
#include <TApplication.h>
#include <TError.h>
#include <TROOT.h>
using namespace std;

// MEGAlib libs:
#include "MDShapePCON.h"
#include "UTDShapeShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the polycone shape MDShapePCON
class UTDShapePCON : public UTDShapeBase
{
public:
  //! Default constructor
  UTDShapePCON() : UTDShapeBase("UTDShapePCON") {}
  //! Default destructor
  virtual ~UTDShapePCON() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the analytic truth, Sections has the entries z, rmin, rmax with increasing z, the angles are in degrees
  MTruth GetTruth(double Phi, double DPhi, const vector<vector<double>>& Sections);
  //! Return the volume from the sum of the frustums of the sections
  double GetTruthVolume(double DPhi, const vector<vector<double>>& Sections);
  //! Set up a shape with the sections
  bool SetUp(MDShapePCON& Shape, double Phi, double DPhi, const vector<vector<double>>& Sections);

  //! Test the default state
  bool TestDefault();
  //! Test the limits of the parameters
  bool TestLimits();
  //! Test the sections
  bool TestSections();
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
bool UTDShapePCON::Run()
{
  bool Passed = true;

  Passed = TestDefault() && Passed;
  Passed = TestLimits() && Passed;
  Passed = TestSections() && Passed;
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


//! Return the analytic truth, Sections has the entries z, rmin, rmax with increasing z, the angles are in degrees
UTDShapeBase::MTruth UTDShapePCON::GetTruth(double Phi, double DPhi, const vector<vector<double>>& Sections)
{
  return [=](double x, double y, double z) {
    if (z < Sections.front()[0] || z > Sections.back()[0]) {
      return false;
    }
    double Angle = atan2(y, x)*c_Deg;
    while (Angle < Phi) {
      Angle += 360;
    }
    if (Angle > Phi + DPhi) {
      return false;
    }
    double R = sqrt(x*x + y*y);
    for (unsigned int s = 1; s < Sections.size(); ++s) {
      if (z >= Sections[s - 1][0] && z <= Sections[s][0] && Sections[s][0] > Sections[s - 1][0]) {
        double T = (z - Sections[s - 1][0])/(Sections[s][0] - Sections[s - 1][0]);
        double Rmin = Sections[s - 1][1] + T*(Sections[s][1] - Sections[s - 1][1]);
        double Rmax = Sections[s - 1][2] + T*(Sections[s][2] - Sections[s - 1][2]);
        if (R >= Rmin && R <= Rmax) {
          return true;
        }
        return false;
      }
    }
    return false;
  };
}


////////////////////////////////////////////////////////////////////////////////


//! Return the volume from the sum of the frustums of the sections
double UTDShapePCON::GetTruthVolume(double DPhi, const vector<vector<double>>& Sections)
{
  // Area of the circle or polygon per squared radius:
  double AreaFactor = c_Pi*DPhi/360;
  double Volume = 0;
  for (unsigned int s = 1; s < Sections.size(); ++s) {
    double Dz = Sections[s][0] - Sections[s - 1][0];
    double R1 = Sections[s - 1][2];
    double R2 = Sections[s][2];
    double r1 = Sections[s - 1][1];
    double r2 = Sections[s][1];
    Volume += AreaFactor*Dz/3*((R1*R1 + R1*R2 + R2*R2) - (r1*r1 + r1*r2 + r2*r2));
  }
  return Volume;
}


////////////////////////////////////////////////////////////////////////////////


//! Set up a shape with the sections
bool UTDShapePCON::SetUp(MDShapePCON& Shape, double Phi, double DPhi, const vector<vector<double>>& Sections)
{
  if (Shape.Set(Phi, DPhi, Sections.size()) == false) {
    return false;
  }
  for (unsigned int s = 0; s < Sections.size(); ++s) {
    if (Shape.AddSection(s, Sections[s][0], Sections[s][1], Sections[s][2]) == false) {
      return false;
    }
  }
  return Shape.Validate();
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default state
bool UTDShapePCON::TestDefault()
{
  bool Passed = true;

  MDShapePCON Shape("Shape");
  Passed = EvaluateTrue("GetName()", "constructor", "The name of the constructor is stored", Shape.GetName() == "Shape") && Passed;
  Passed = EvaluateTrue("GetType()", "constructor", "The type is PCON", Shape.GetType() == "PCON") && Passed;
  Passed = EvaluateNear("GetVolume()", "default", "A new shape has no volume", Shape.GetVolume(), 0.0, 1e-15) && Passed;
  Passed = Evaluate("GetNSections()", "default", "A new shape has no sections", Shape.GetNSections(), 0U) && Passed;
  Passed = EvaluateNear("GetPhi()", "default", "The default is zero", Shape.GetPhi(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetDPhi()", "default", "The default is zero", Shape.GetDPhi(), 0.0, 1e-15) && Passed;

  // Defect GEO-C-7: A shape without parameters passes the validation
  MDShapePCON NoParameters("NoParameters");
  Passed = ExpectedToFail(EvaluateFalse("Validate()", "no parameters", "A shape without parameters is rejected", NoParameters.Validate())) && Passed;

  // Defect GEO-C-17: A shape with sections which were never defined passes the validation
  MDShapePCON NoSections("NoSections");
  NoSections.Set(0, 360, 2);
  Passed = ExpectedToCrash(EvaluateFalse("Validate()", "no sections", "A shape without defined sections is rejected", NoSections.Validate())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the limits of the parameters
bool UTDShapePCON::TestLimits()
{
  bool Passed = true;

  const double Just = 1e-9;
  MDShapePCON S("Shape");

  Passed = EvaluateTrue("Set()", "Phi = 0", "Phi = 0 is accepted", Quiet([&]() { return S.Set(0, 360, 2); })) && Passed;
  Passed = EvaluateFalse("Set()", "Phi just below 0", "Phi below 0 is rejected", Quiet([&]() { return S.Set(-Just, 360, 2); })) && Passed;
  Passed = EvaluateTrue("Set()", "Phi just below 360", "Phi just below 360 is accepted", Quiet([&]() { return S.Set(360 - Just, 360, 2); })) && Passed;
  Passed = EvaluateFalse("Set()", "Phi = 360", "Phi = 360 is rejected", Quiet([&]() { return S.Set(360, 360, 2); })) && Passed;
  Passed = EvaluateTrue("Set()", "DPhi just above 0", "A small opening is accepted", Quiet([&]() { return S.Set(0, Just, 2); })) && Passed;
  Passed = EvaluateFalse("Set()", "DPhi = 0", "No opening is rejected", Quiet([&]() { return S.Set(0, 0, 2); })) && Passed;
  Passed = EvaluateTrue("Set()", "DPhi = 360", "The full opening is accepted", Quiet([&]() { return S.Set(0, 360, 2); })) && Passed;
  Passed = EvaluateFalse("Set()", "DPhi just above 360", "An opening above 360 is rejected", Quiet([&]() { return S.Set(0, 360 + 1e-6, 2); })) && Passed;
  Passed = EvaluateTrue("Set()", "2 sections", "Two sections are accepted", Quiet([&]() { return S.Set(0, 360, 2); })) && Passed;
  Passed = EvaluateFalse("Set()", "1 section", "One section is rejected", Quiet([&]() { return S.Set(0, 360, 1); })) && Passed;
  Passed = EvaluateFalse("Set()", "0 sections", "No section is rejected", Quiet([&]() { return S.Set(0, 360, 0); })) && Passed;

  // A rejected set does not change the shape:
  S.Set(10, 20, 3);
  Quiet([&]() { return S.Set(30, 40, 1); });
  Passed = EvaluateNear("GetPhi()", "after reject", "The old Phi is kept", S.GetPhi(), 10.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetDPhi()", "after reject", "The old DPhi is kept", S.GetDPhi(), 20.0, 1e-15) && Passed;
  Passed = Evaluate("GetNSections()", "after reject", "The old number of sections is kept", S.GetNSections(), 3U) && Passed;

  // Defect GEO-C-30: The error message of too few sections names the wrong shape
  streambuf* Original = cout.rdbuf();
  ostringstream Capture;
  cout.rdbuf(Capture.rdbuf());
  S.Set(0, 360, 1);
  cout.rdbuf(Original);
  Passed = ExpectedToFail(EvaluateFalse("Set()", "1 section, error message", "The error message does not name another shape", MString(Capture.str().c_str()).Contains("BRIK"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the sections
bool UTDShapePCON::TestSections()
{
  bool Passed = true;

  const double Just = 1e-9;
  MDShapePCON S("Shape");
  S.Set(0, 360, 2);

  Passed = EvaluateTrue("AddSection()", "Rmin = 0", "Rmin = 0 is accepted", Quiet([&]() { return S.AddSection(0, 1, 0, 1); })) && Passed;
  Passed = EvaluateFalse("AddSection()", "Rmin just below 0", "A negative Rmin is rejected", Quiet([&]() { return S.AddSection(0, 1, -Just, 1); })) && Passed;
  Passed = EvaluateFalse("AddSection()", "Rmax just below 0", "A negative Rmax is rejected", Quiet([&]() { return S.AddSection(0, 1, 0, -Just); })) && Passed;
  Passed = EvaluateTrue("AddSection()", "Rmin just below Rmax", "Rmin below Rmax is accepted", Quiet([&]() { return S.AddSection(0, 1, 1 - Just, 1); })) && Passed;
  Passed = EvaluateFalse("AddSection()", "Rmin = Rmax", "Rmin = Rmax is rejected", Quiet([&]() { return S.AddSection(0, 1, 1, 1); })) && Passed;
  Passed = EvaluateFalse("AddSection()", "Rmin = Rmax = 0", "A section without extent is rejected", Quiet([&]() { return S.AddSection(0, 1, 0, 0); })) && Passed;

  // Read back and non-symmetric values:
  S.AddSection(0, -1.5, 0.5, 2.5);
  S.AddSection(1, 3.5, 1.5, 4.5);
  Passed = EvaluateNear("GetZ()", "section 0", "The z of the section is stored", S.GetZ(0), -1.5, 1e-15) && Passed;
  Passed = EvaluateNear("GetRmin()", "section 0", "The Rmin of the section is stored", S.GetRmin(0), 0.5, 1e-15) && Passed;
  Passed = EvaluateNear("GetRmax()", "section 0", "The Rmax of the section is stored", S.GetRmax(0), 2.5, 1e-15) && Passed;
  Passed = EvaluateNear("GetZ()", "section n - 1", "The z of the last section is stored", S.GetZ(1), 3.5, 1e-15) && Passed;
  Passed = EvaluateNear("GetRmin()", "section n - 1", "The Rmin of the last section is stored", S.GetRmin(1), 1.5, 1e-15) && Passed;
  Passed = EvaluateNear("GetRmax()", "section n - 1", "The Rmax of the last section is stored", S.GetRmax(1), 4.5, 1e-15) && Passed;

  // A rejected section keeps the old values:
  Quiet([&]() { return S.AddSection(1, 9, 9, 8); });
  Passed = EvaluateNear("AddSection()", "after reject", "The old z is kept", S.GetZ(1), 3.5, 1e-15) && Passed;

  // Defect GEO-C-16: The section index is not checked, the values are written behind the end of the array and true is returned
  MDShapePCON Bounds("Bounds");
  Bounds.Set(0, 360, 2);
  Passed = ExpectedToCrash(EvaluateFalse("AddSection()", "section n", "An index behind the last section is rejected", Bounds.AddSection(2, 1, 0, 1))) && Passed;

  // Defect GEO-C-16: A section without Set writes into an empty array
  MDShapePCON NoSet("NoSet");
  Passed = ExpectedToCrash(EvaluateFalse("AddSection()", "no Set", "A section before the number of sections is known is rejected", NoSet.AddSection(0, 1, 0, 1))) && Passed;

  // Defect GEO-C-16: The getters only have an assertion, which is removed in the default build with NDEBUG, a far index reads outside the process memory
  MDShapePCON FarIndex("FarIndex");
  FarIndex.Set(0, 360, 2);
  Passed = ExpectedToCrash(EvaluateTrue("GetZ()", "far index", "A far index does not crash the program", (FarIndex.GetZ(1000000000U), true))) && Passed;

  // Defect GEO-C-17: A decreasing z in the sections is accepted without Parse, which only knows the reordering, and the volume becomes negative
  MDShapePCON Decreasing("Decreasing");
  Decreasing.Set(0, 360, 2);
  Decreasing.AddSection(0, 1, 0, 1);
  Decreasing.AddSection(1, -1, 0, 1);
  Passed = ExpectedToFail(EvaluateTrue("GetVolume()", "decreasing z", "The volume is not negative", Decreasing.GetVolume() >= 0)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the volume and the size
bool UTDShapePCON::TestVolumeSize()
{
  bool Passed = true;

  // A cylinder: pi R^2 h
  MDShapePCON Cylinder("Cylinder");
  SetUp(Cylinder, 0, 360, { {-2, 0, 3}, {2, 0, 3} });
  Passed = EvaluateNear("GetVolume()", "cylinder", "The volume is pi R^2 h", Cylinder.GetVolume(), c_Pi*9*4, 1e-10) && Passed;

  // A cone: one third of the cylinder
  MDShapePCON Cone("Cone");
  SetUp(Cone, 0, 360, { {-2, 0, 3}, {2, 0, 1e-12} });
  Passed = EvaluateNear("GetVolume()", "cone", "The volume is one third of the cylinder", Cone.GetVolume(), c_Pi*9*4/3, 1e-9) && Passed;

  MDShapePCON Hollow("Hollow");
  SetUp(Hollow, 0, 360, { {-2, 1, 3}, {2, 1, 3} });
  Passed = EvaluateNear("GetVolume()", "hollow cylinder", "The volume is the difference of two cylinders", Hollow.GetVolume(), c_Pi*8*4, 1e-10) && Passed;

  MDShapePCON Quarter("Quarter");
  SetUp(Quarter, 30, 90, { {-2, 0, 3}, {2, 0, 3} });
  Passed = EvaluateNear("GetVolume()", "quarter", "The opening scales the volume", Quarter.GetVolume(), c_Pi*9*4/4, 1e-10) && Passed;

  // Non-symmetric with 3 sections and a hole:
  const vector<vector<double>> Sections = { {-2, 1, 3}, {1, 0.5, 2}, {4, 1, 5} };
  MDShapePCON Stack("Stack");
  Passed = EvaluateTrue("Validate()", "3 sections", "The shape is valid", SetUp(Stack, 20, 90, Sections)) && Passed;
  Passed = EvaluateNear("GetVolume()", "3 sections", "The volume agrees with the sum of the frustums", Stack.GetVolume(), GetTruthVolume(90, Sections), 1e-10) && Passed;

  // Defect GEO-C-3: The z size is the sum of all z values, which is zero here
  MDShapePCON Symmetric("Symmetric");
  SetUp(Symmetric, 0, 360, { {-5, 0, 3}, {5, 0, 1} });
  Passed = EvaluateNear("GetSize()", "z from -5 to 5", "The size in x is the largest radius", Symmetric.GetSize().X(), 3.0, 1e-12) && Passed;
  Passed = ExpectedToFail(EvaluateNear("GetSize()", "z from -5 to 5", "The half size in z is 5", Symmetric.GetSize().Z(), 5.0, 1e-12)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the inside check
bool UTDShapePCON::TestIsInside()
{
  bool Passed = true;

  const double Just = 1e-9;
  const double Small = 1e-6;

  // Between z = -2 and z = 4 the radii go linear: at z = 1: Rmin = 1.5 and Rmax = 3
  MDShapePCON Shape("Shape");
  SetUp(Shape, 0, 360, { {-2, 1, 2}, {4, 2, 4} });

  // The direction of the middle of the first side (the polycone has no sides):
  const double C = 30.0*c_Rad;
  MVector Out(cos(C), sin(C), 0);

  Passed = EvaluateTrue("IsInside()", "interior", "A point between the radii is inside", Shape.IsInside(2.2*Out + MVector(0, 0, 1))) && Passed;
  Passed = EvaluateFalse("IsInside()", "hole", "The hole is outside", Shape.IsInside(MVector(0, 0, 1))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just inside Rmax", "Just inside the outer surface is inside", Shape.IsInside((3 - Just)*Out + MVector(0, 0, 1))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just outside Rmax", "Just outside the outer surface is outside", Shape.IsInside((3 + Just)*Out + MVector(0, 0, 1))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just outside Rmin", "Just outside the inner surface is inside", Shape.IsInside((1.5 + Just)*Out + MVector(0, 0, 1))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just inside Rmin", "Just inside the inner surface is outside", Shape.IsInside((1.5 - Just)*Out + MVector(0, 0, 1))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just below the top", "Just below the top is inside", Shape.IsInside(3*Out + MVector(0, 0, 4 - Just))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just above the top", "Just above the top is outside", Shape.IsInside(3*Out + MVector(0, 0, 4 + Just))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just above the bottom", "Just above the bottom is inside", Shape.IsInside(1.5*Out + MVector(0, 0, -2 + Just))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just below the bottom", "Just below the bottom is outside", Shape.IsInside(1.5*Out + MVector(0, 0, -2 - Just))) && Passed;

  Passed = CheckToleranceAtSurface("top", Shape, 3*Out + MVector(0, 0, 4), MVector(0, 0, 1)) && Passed;
  Passed = CheckToleranceAtSurface("bottom", Shape, 1.5*Out + MVector(0, 0, -2), MVector(0, 0, -1)) && Passed;

  // The limits in phi from 30 to 120 deg:
  MDShapePCON Segment("Segment");
  SetUp(Segment, 30, 90, { {-2, 1, 3}, {2, 1, 3} });
  Passed = EvaluateTrue("IsInside()", "just above Phi", "Just inside the start angle is inside", Segment.IsInside(MVector(2*cos((30 + Small)*c_Rad), 2*sin((30 + Small)*c_Rad), 0))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just below Phi", "Just outside the start angle is outside", Segment.IsInside(MVector(2*cos((30 - Small)*c_Rad), 2*sin((30 - Small)*c_Rad), 0))) && Passed;
  Passed = EvaluateTrue("IsInside()", "just below Phi + DPhi", "Just inside the end angle is inside", Segment.IsInside(MVector(2*cos((120 - Small)*c_Rad), 2*sin((120 - Small)*c_Rad), 0))) && Passed;
  Passed = EvaluateFalse("IsInside()", "just above Phi + DPhi", "Just outside the end angle is outside", Segment.IsInside(MVector(2*cos((120 + Small)*c_Rad), 2*sin((120 + Small)*c_Rad), 0))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the distance functions with fixed rays
bool UTDShapePCON::TestDistances()
{
  bool Passed = true;

  MDShapePCON Shape("Shape");
  SetUp(Shape, 0, 360, { {-2, 1, 3}, {2, 1, 3} });

  const double C = 30.0*c_Rad;
  MVector Out(cos(C), sin(C), 0);

  Passed = EvaluateNear("DistanceInsideOut()", "outward", "The distance to the outer surface", Shape.DistanceInsideOut(2*Out, Out), 1.0, 1e-9) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "inward", "The distance to the inner surface", Shape.DistanceInsideOut(2*Out, -1.0*Out), 1.0, 1e-9) && Passed;
  Passed = EvaluateNear("DistanceInsideOut()", "up", "The distance to the top", Shape.DistanceInsideOut(2*Out + MVector(0, 0, 0.5), MVector(0, 0, 1)), 1.5, 1e-9) && Passed;
  Passed = EvaluateNear("DistanceOutsideIn()", "from outside", "The distance to the outer surface from outside", Shape.DistanceOutsideIn(5*Out, -1.0*Out), 2.0, 1e-9) && Passed;
  Passed = EvaluateNear("DistanceOutsideIn()", "from above", "The distance to the top from above", Shape.DistanceOutsideIn(2*Out + MVector(0, 0, 7), MVector(0, 0, -1)), 5.0, 1e-9) && Passed;
  Passed = EvaluateTrue("DistanceOutsideIn()", "miss", "A ray away from the shape is no hit", Shape.DistanceOutsideIn(5*Out, Out) > 1e20) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the unique position
bool UTDShapePCON::TestUniquePosition()
{
  bool Passed = true;

  // A filled one: the center
  MDShapePCON Filled("Filled");
  SetUp(Filled, 0, 360, { {-2, 0, 3}, {4, 0, 1} });
  Passed = EvaluateVectorNear("GetUniquePosition()", "filled", "The position is the center of the z range", Filled.GetUniquePosition(), MVector(0, 0, 1), 1e-12) && Passed;
  Passed = EvaluateTrue("GetUniquePosition()", "filled", "The position is inside", Filled.IsInside(Filled.GetUniquePosition())) && Passed;

  // A hollow one: middle of the radii of the first section
  MDShapePCON Hollow("Hollow");
  SetUp(Hollow, 30, 90, { {-2, 1, 3}, {2, 2, 4} });
  Passed = EvaluateVectorNear("GetUniquePosition()", "hollow", "The position is in the middle of the first section", Hollow.GetUniquePosition(), MVector(2.5*cos(75*c_Rad), 2.5*sin(75*c_Rad), 0), 1e-12) && Passed;
  Passed = EvaluateTrue("GetUniquePosition()", "hollow", "The position is inside", Hollow.IsInside(Hollow.GetUniquePosition())) && Passed;

  // Defect GEO-C-29: Two sections at the same z at the start are accepted, but ROOT stops the program in the validation
  MDShapePCON StepAtStart("StepAtStart");
  Passed = ExpectedToCrash(EvaluateFalse("Validate()", "step at the first sections", "The shape is rejected without stopping the program", SetUp(StepAtStart, 0, 360, { {0, 0.5, 1}, {0, 2, 3}, {5, 2, 3} }))) && Passed;

  // Defect GEO-C-29: Two sections at the same z at the end are accepted, but ROOT stops the program in the validation
  MDShapePCON StepAtEnd("StepAtEnd");
  Passed = ExpectedToCrash(EvaluateFalse("Validate()", "step at the last sections", "The shape is rejected without stopping the program", SetUp(StepAtEnd, 0, 360, { {0, 2, 3}, {5, 2, 3}, {5, 0.5, 1} }))) && Passed;

  // A step in the middle is allowed - the position is in the first part:
  MDShapePCON Middle("Middle");
  SetUp(Middle, 0, 360, { {0, 0.5, 1}, {2, 0.5, 1}, {2, 2, 3}, {5, 2, 3} });
  Passed = EvaluateTrue("GetUniquePosition()", "step in the middle", "The position is inside", Middle.IsInside(Middle.GetUniquePosition())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the geometry against the analytic truth
bool UTDShapePCON::TestAgainstTruth()
{
  bool Passed = true;

  const MVector Min(-6.5, -6.5, -3);
  const MVector Max(6.5, 6.5, 5);

  // Filled, full circle:
  const vector<vector<double>> Filled = { {-2, 0, 3}, {1, 0, 2}, {4, 0, 5} };
  MDShapePCON ShapeFilled("Filled");
  SetUp(ShapeFilled, 0, 360, Filled);
  MTruth TruthFilled = GetTruth(0, 360, Filled);
  Passed = CheckTruthVolume("filled", TruthFilled, Min, Max, ShapeFilled.GetVolume()) && Passed;
  Passed = CheckAgainstTruth("filled", ShapeFilled, TruthFilled, Min, Max) && Passed;

  // Hollow segment:
  const vector<vector<double>> Hollow = { {-2, 1, 3}, {1, 0.5, 2}, {4, 1, 5} };
  MDShapePCON ShapeHollow("Hollow");
  SetUp(ShapeHollow, 20, 200, Hollow);
  MTruth TruthHollow = GetTruth(20, 200, Hollow);
  Passed = CheckTruthVolume("hollow segment", TruthHollow, Min, Max, ShapeHollow.GetVolume()) && Passed;
  Passed = CheckAgainstTruth("hollow segment", ShapeHollow, TruthHollow, Min, Max) && Passed;

  // Segment over phi = 0:
  MDShapePCON ShapeOver("Over");
  SetUp(ShapeOver, 300, 100, Hollow);
  MTruth TruthOver = GetTruth(300, 100, Hollow);
  Passed = CheckTruthVolume("over 0 deg", TruthOver, Min, Max, ShapeOver.GetVolume()) && Passed;
  Passed = CheckAgainstTruth("over 0 deg", ShapeOver, TruthOver, Min, Max) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the string representations
bool UTDShapePCON::TestStrings()
{
  bool Passed = true;

  MDShapePCON Shape("Shape");
  SetUp(Shape, 20, 90, { {-2, 1, 3}, {4, 0.5, 2} });
  Passed = EvaluateTrue("ToString()", "shape", "The string contains type and parameters", Shape.ToString().Contains("PCON (20,90,2,-2,1,3,4,0.5,2)")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "shape", "The text is the keyword with the parameters", Shape.GetGeomega() == "PCON 20 90 2 -2 1 3 4 0.5 2") && Passed;

  MDShapePCON Read("Read");
  Passed = EvaluateTrue("Parse()", "round trip", "The text can be parsed", ParseQuiet(Read, MString("V.Shape ") + Shape.GetGeomega())) && Passed;
  Passed = EvaluateTrue("Parse()", "round trip", "All parameters survive", Read.GetGeomega() == Shape.GetGeomega()) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "The volume survives", Read.GetVolume(), Shape.GetVolume(), 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the scaling
bool UTDShapePCON::TestScale()
{
  bool Passed = true;

  const vector<vector<double>> Sections = { {-2, 1, 3}, {1, 0.5, 2}, {4, 1, 5} };
  const vector<vector<double>> Scaled = { {-4, 2, 6}, {2, 1, 4}, {8, 2, 10} };

  MDShapePCON All("All");
  SetUp(All, 20, 200, Sections);
  double Volume = All.GetVolume();
  Passed = CheckScaleState("XYZ", All, 2.0, "XYZ") && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "z is scaled", All.GetZ(2), 8.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "Rmin is scaled", All.GetRmin(1), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "Rmax is scaled", All.GetRmax(0), 6.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "The angles are not scaled", All.GetDPhi(), 200.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XYZ", "The volume is scaled by the third power", All.GetVolume(), 8*Volume, 1e-9) && Passed;
  Passed = CheckInsideAgainstTruth("XYZ", All, GetTruth(20, 200, Scaled), MVector(-13, -13, -5), MVector(13, 13, 9)) && Passed;

  MDShapePCON XY("XY");
  SetUp(XY, 20, 200, Sections);
  Passed = EvaluateTrue("Scale()", "XY", "The axes XY are accepted", XY.Scale(2.0, "XY")) && Passed;
  Passed = EvaluateNear("Scale()", "XY", "The radii are scaled", XY.GetRmax(0), 6.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "XY", "z is not scaled", XY.GetZ(2), 4.0, 1e-12) && Passed;

  MDShapePCON Z("Z");
  SetUp(Z, 20, 200, Sections);
  Passed = EvaluateTrue("Scale()", "Z", "The axis Z is accepted", Z.Scale(2.0, "Z")) && Passed;
  Passed = EvaluateNear("Scale()", "Z", "The radii are not scaled", Z.GetRmax(0), 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "Z", "z is scaled", Z.GetZ(2), 8.0, 1e-12) && Passed;

  MDShapePCON X("X");
  SetUp(X, 20, 200, Sections);
  Passed = EvaluateFalse("Scale()", "X", "The radii cannot be scaled along X alone", Quiet([&]() { return X.Scale(2.0, "X"); })) && Passed;
  Passed = EvaluateNear("Scale()", "X", "The rejected axis keeps the radii", X.GetRmax(0), 3.0, 1e-12) && Passed;

  // Defect GEO-C-6: A rejected axis combination leaves the shape marked as scaled
  Passed = ExpectedToFail(EvaluateFalse("IsScaled()", "after a rejected scaling", "The shape is not marked as scaled", X.IsScaled())) && Passed;

  // Defect GEO-C-5: A negative factor is accepted while AddSection() rejects negative radii
  MDShapePCON Negative("Negative");
  SetUp(Negative, 20, 200, Sections);
  // Hide the warnings of ROOT about the negative radii
  Int_t ErrorLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;
  const bool IsNegativeScaled = Negative.Scale(-2.0, "XYZ");
  gErrorIgnoreLevel = ErrorLevel;
  Passed = ExpectedToFail(EvaluateFalse("Scale()", "factor -2", "A negative factor is rejected", IsNegativeScaled)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parsing the keywords
bool UTDShapePCON::TestParse()
{
  bool Passed = true;

  MDShapePCON Increasing("Increasing");
  Passed = EvaluateTrue("Parse()", "Parameters, increasing", "The keyword Parameters is accepted", ParseQuiet(Increasing, "S.Parameters 20 90 3 -2 1 3 1 0.5 2 4 1 5")) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "Phi is read", Increasing.GetPhi(), 20.0, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "DPhi is read", Increasing.GetDPhi(), 90.0, 1e-15) && Passed;

  Passed = Evaluate("Parse()", "Parameters", "The number of sections is read", Increasing.GetNSections(), 3U) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "The z of the first section is read", Increasing.GetZ(0), -2.0, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "The Rmin of the second section is read", Increasing.GetRmin(1), 0.5, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "Parameters", "The Rmax of the last section is read", Increasing.GetRmax(2), 5.0, 1e-15) && Passed;

  MDShapePCON Keyword("Keyword");
  Passed = EvaluateTrue("Parse()", "Shape", "The keyword Shape is accepted", ParseQuiet(Keyword, "V.Shape PCON 20 90 2 -2 1 3 1 0.5 2")) && Passed;
  Passed = EvaluateNear("Parse()", "Shape", "The parameters start after the type", Keyword.GetRmax(1), 2.0, 1e-15) && Passed;

  // Decreasing z is reordered - the radii stay with their z:
  MDShapePCON Decreasing("Decreasing");
  Passed = EvaluateTrue("Parse()", "decreasing", "A decreasing z is accepted", ParseQuiet(Decreasing, "S.Parameters 20 90 3 4 1 5 1 0.5 2 -2 1 3")) && Passed;
  Passed = EvaluateNear("Parse()", "decreasing", "The sections are reordered", Decreasing.GetZ(0), -2.0, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "decreasing", "The Rmax follows its z", Decreasing.GetRmax(0), 3.0, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "decreasing", "The last z is the largest", Decreasing.GetZ(2), 4.0, 1e-15) && Passed;
  Passed = EvaluateNear("Parse()", "decreasing", "The volume is the same as the one with increasing z", Decreasing.GetVolume(), Increasing.GetVolume(), 1e-12) && Passed;

  // A step in the z is allowed:
  MDShapePCON Step("Step");
  Passed = EvaluateTrue("Parse()", "step", "Two equal z values are accepted", ParseQuiet(Step, "S.Parameters 0 360 4 0 0 1 1 0 1 1 0 2 2 0 2")) && Passed;
  Passed = EvaluateNear("Parse()", "step", "A step section is kept", Step.GetRmax(2), 2.0, 1e-15) && Passed;

  // Malformed input:
  MDShapePCON Bad("Bad");
  Passed = EvaluateFalse("Parse()", "too few", "Too few parameters are rejected", ParseQuiet(Bad, "Bad.Parameters 0 360")) && Passed;
  Passed = EvaluateFalse("Parse()", "section missing", "A missing section is rejected", ParseQuiet(Bad, "Bad.Parameters 0 360 3 0 0 1 1 0 1")) && Passed;
  Passed = EvaluateFalse("Parse()", "section value missing", "A missing value is rejected", ParseQuiet(Bad, "Bad.Parameters 0 360 2 0 0 1 1 0")) && Passed;
  Passed = EvaluateFalse("Parse()", "section too much", "One section too much is rejected", ParseQuiet(Bad, "Bad.Parameters 0 360 2 0 0 1 1 0 1 2 0 1")) && Passed;
  Passed = EvaluateFalse("Parse()", "one section", "A single section is rejected", ParseQuiet(Bad, "Bad.Parameters 0 360 1 0 0 1")) && Passed;
  Passed = EvaluateFalse("Parse()", "wrong keyword", "A wrong keyword is rejected", ParseQuiet(Bad, "Bad.Param 0 360 2 0 0 1 1 0 1")) && Passed;
  Passed = EvaluateFalse("Parse()", "invalid angle", "An invalid angle is rejected", ParseQuiet(Bad, "Bad.Parameters 400 360 2 0 0 1 1 0 1")) && Passed;
  Passed = EvaluateFalse("Parse()", "invalid section", "An invalid section is rejected", ParseQuiet(Bad, "Bad.Parameters 0 360 2 0 1 0 1 0 1")) && Passed;
  Passed = EvaluateFalse("Parse()", "unordered", "An unordered z is rejected", ParseQuiet(Bad, "Bad.Parameters 0 360 3 0 0 1 2 0 1 1 0 1")) && Passed;
  Passed = EvaluateFalse("Parse()", "unordered, other direction", "An unordered z is rejected", ParseQuiet(Bad, "Bad.Parameters 0 360 3 2 0 1 0 0 1 1 0 1")) && Passed;

  // Defect GEO-C-33: A rejected line leaves a partly changed shape, the number of sections of the line is already stored
  MDShapePCON Stale("Stale");
  ParseQuiet(Stale, "S.Parameters 0 360 2 0 0 1 1 0 1");
  ParseQuiet(Stale, "S.Parameters 0 360 3 0 0 1 2 0 1 1 0 1");
  Passed = ExpectedToFail(Evaluate("Parse()", "rejected after a valid line", "The rejected line keeps the number of sections", Stale.GetNSections(), 2U)) && Passed;

  // Defect GEO-C-17: Sections with all the same z give a shape without height
  MDShapePCON Flat("Flat");
  Passed = ExpectedToFail(EvaluateFalse("Parse()", "all z equal", "A shape without height is rejected", ParseQuiet(Flat, "Bad.Parameters 0 360 2 1 0 1 1 0 2"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test configuring a validated shape again
bool UTDShapePCON::TestReconfiguration()
{
  MDShapePCON Shape("S");
  return CheckReconfiguration("reconfiguration", Shape, MVector(0.0, 0.0, 1.0), [&Shape]() { return Shape.Set(0.0, 360.0, 2) && Shape.AddSection(0, -1.0, 0.0, 5.0) && Shape.AddSection(1, 1.0, 0.0, 5.0); }, 1.0, [&Shape]() { return Shape.Set(0.0, 360.0, 2) && Shape.AddSection(0, -2.0, 0.0, 5.0) && Shape.AddSection(1, 2.0, 0.0, 5.0); }, 2.0, "S.Parameters 0 360 2 -3 0 5 3 0 5", 3.0, [&Shape]() { return Shape.Set(0.0, 360.0, 1); });
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  gErrorIgnoreLevel = kWarning;

  UTDShapePCON Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDShapePCON.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
