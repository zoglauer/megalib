/*
 * UTDGridPoint.cxx
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
#include <set>
#include <sstream>
#include <vector>

// POSIX libs:
// Process control has no standard C++ equivalent
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// MEGAlib libs:
#include "MDGridPoint.h"
#include "MString.h"
#include "MUnitTest.h"
#include "UTGeomegaShared.h"
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the grid point MDGridPoint
class UTDGridPoint : public UTGeomegaBase
{
public:
  //! Default constructor
  UTDGridPoint() : UTGeomegaBase("UTDGridPoint") {}
  //! Default destructor
  virtual ~UTDGridPoint() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the type constants
  bool TestConstants();
  //! Test the default constructor
  bool TestDefaultConstruction();
  //! Test the standard constructor
  bool TestStandardConstruction();
  //! Test the getters and setters
  bool TestGettersSetters();
  //! Test Set
  bool TestSet();
  //! Test the copy constructor and the assignment
  bool TestCopyAndAssignment();
  //! Test the equality of grid points
  bool TestEquality();
  //! Test the equality of the types which have no grid point comparison
  bool TestEqualityUnsupportedTypes();
  //! Test adding grid points in the same voxel
  bool TestAddVoxel();
  //! Test adding grid points in different voxels
  bool TestAddDifferentVoxels();
  //! Test adding Anger camera grid points
  bool TestAddAnger();
  //! Test adding guard ring grid points
  bool TestAddGuardRing();
  //! Test adding grid points without energy or without hits
  bool TestAddWithoutEnergyOrHits();
  //! Test adding grid points of other or unsupported types
  bool TestAddOtherTypes();
  //! Test the origin tracking
  bool TestOrigins();
  //! Test the stream output
  bool TestStream();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDGridPoint::Run()
{
  bool Passed = true;

  Passed = TestConstants() && Passed;
  Passed = TestDefaultConstruction() && Passed;
  Passed = TestStandardConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestSet() && Passed;
  Passed = TestCopyAndAssignment() && Passed;
  Passed = TestEquality() && Passed;
  Passed = TestEqualityUnsupportedTypes() && Passed;
  Passed = TestAddVoxel() && Passed;
  Passed = TestAddDifferentVoxels() && Passed;
  Passed = TestAddAnger() && Passed;
  Passed = TestAddGuardRing() && Passed;
  Passed = TestAddWithoutEnergyOrHits() && Passed;
  Passed = TestAddOtherTypes() && Passed;
  Passed = TestOrigins() && Passed;
  Passed = TestStream() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the type constants
bool UTDGridPoint::TestConstants()
{
  bool Passed = true;

  Passed = Evaluate("c_Unknown", "constant", "The value is fixed", MDGridPoint::c_Unknown, 0U) && Passed;
  Passed = Evaluate("c_Voxel", "constant", "The value is fixed", MDGridPoint::c_Voxel, 1U) && Passed;
  Passed = Evaluate("c_VoxelDrift", "constant", "The value is fixed", MDGridPoint::c_VoxelDrift, 2U) && Passed;
  Passed = Evaluate("c_XStrip", "constant", "The value is fixed", MDGridPoint::c_XStrip, 3U) && Passed;
  Passed = Evaluate("c_YStrip", "constant", "The value is fixed", MDGridPoint::c_YStrip, 4U) && Passed;
  Passed = Evaluate("c_XYAnger", "constant", "The value is fixed", MDGridPoint::c_XYAnger, 5U) && Passed;
  Passed = Evaluate("c_XYZAnger", "constant", "The value is fixed", MDGridPoint::c_XYZAnger, 6U) && Passed;
  Passed = Evaluate("c_XYZIndependentAnger", "constant", "The value is fixed", MDGridPoint::c_XYZIndependentAnger, 7U) && Passed;
  Passed = Evaluate("c_GuardRing", "constant", "The value is fixed", MDGridPoint::c_GuardRing, 8U) && Passed;
  Passed = Evaluate("c_MinType", "constant", "The first type is the voxel", MDGridPoint::c_MinType, MDGridPoint::c_Voxel) && Passed;
  Passed = Evaluate("c_MaxType", "constant", "The last type is the guard ring", MDGridPoint::c_MaxType, MDGridPoint::c_GuardRing) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default constructor
bool UTDGridPoint::TestDefaultConstruction()
{
  bool Passed = true;

  MDGridPoint Point;
  Passed = Evaluate("MDGridPoint()", "default", "The x grid is 0", Point.GetXGrid(), 0U) && Passed;
  Passed = Evaluate("MDGridPoint()", "default", "The y grid is 0", Point.GetYGrid(), 0U) && Passed;
  Passed = Evaluate("MDGridPoint()", "default", "The z grid is 0", Point.GetZGrid(), 0U) && Passed;
  Passed = Evaluate("MDGridPoint()", "default", "The type is unknown", Point.GetType(), MDGridPoint::c_Unknown) && Passed;
  Passed = EvaluateVectorNear("MDGridPoint()", "default", "The position is the origin", Point.GetPosition(), MVector(0.0, 0.0, 0.0), 0.0) && Passed;
  Passed = EvaluateNear("MDGridPoint()", "default", "The energy is 0", Point.GetEnergy(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("MDGridPoint()", "default", "The time is 0", Point.GetTime(), 0.0, 0.0) && Passed;
  Passed = Evaluate("MDGridPoint()", "default", "The number of hits is 1", Point.GetHits(), 1U) && Passed;
  Passed = EvaluateNear("MDGridPoint()", "default", "The weight is 1", Point.GetWeight(), 1.0, 0.0) && Passed;
  Passed = EvaluateSize("MDGridPoint()", "default", "There are no origins", Point.GetOriginIDs().size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the standard constructor
bool UTDGridPoint::TestStandardConstruction()
{
  bool Passed = true;

  MDGridPoint Defaults(1, 2, 3);
  Passed = Evaluate("MDGridPoint(x, y, z)", "default arguments", "The x grid is stored", Defaults.GetXGrid(), 1U) && Passed;
  Passed = Evaluate("MDGridPoint(x, y, z)", "default arguments", "The y grid is stored", Defaults.GetYGrid(), 2U) && Passed;
  Passed = Evaluate("MDGridPoint(x, y, z)", "default arguments", "The z grid is stored", Defaults.GetZGrid(), 3U) && Passed;
  Passed = Evaluate("MDGridPoint(x, y, z)", "default arguments", "The default type is the voxel", Defaults.GetType(), MDGridPoint::c_Voxel) && Passed;
  Passed = EvaluateVectorNear("MDGridPoint(x, y, z)", "default arguments", "The default position is the origin", Defaults.GetPosition(), MVector(0.0, 0.0, 0.0), 0.0) && Passed;
  Passed = EvaluateNear("MDGridPoint(x, y, z)", "default arguments", "The default energy is 0", Defaults.GetEnergy(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("MDGridPoint(x, y, z)", "default arguments", "The default time is 0", Defaults.GetTime(), 0.0, 0.0) && Passed;
  Passed = Evaluate("MDGridPoint(x, y, z)", "default arguments", "The default number of hits is 1", Defaults.GetHits(), 1U) && Passed;
  Passed = EvaluateNear("MDGridPoint(x, y, z)", "default arguments", "The default weight is 1", Defaults.GetWeight(), 1.0, 0.0) && Passed;

  MDGridPoint All(4, 5, 6, MDGridPoint::c_XYZAnger, MVector(0.5, -1.5, 2.5), 511.0, 1.5e-9, 7, 0.25);
  Passed = Evaluate("MDGridPoint(all)", "all arguments", "The x grid is stored", All.GetXGrid(), 4U) && Passed;
  Passed = Evaluate("MDGridPoint(all)", "all arguments", "The y grid is stored", All.GetYGrid(), 5U) && Passed;
  Passed = Evaluate("MDGridPoint(all)", "all arguments", "The z grid is stored", All.GetZGrid(), 6U) && Passed;
  Passed = Evaluate("MDGridPoint(all)", "all arguments", "The type is stored", All.GetType(), MDGridPoint::c_XYZAnger) && Passed;
  Passed = EvaluateVectorNear("MDGridPoint(all)", "all arguments", "The position is stored", All.GetPosition(), MVector(0.5, -1.5, 2.5), 0.0) && Passed;
  Passed = EvaluateNear("MDGridPoint(all)", "all arguments", "The energy is stored", All.GetEnergy(), 511.0, 0.0) && Passed;
  Passed = EvaluateNear("MDGridPoint(all)", "all arguments", "The time is stored", All.GetTime(), 1.5e-9, 0.0) && Passed;
  Passed = Evaluate("MDGridPoint(all)", "all arguments", "The number of hits is stored", All.GetHits(), 7U) && Passed;
  Passed = EvaluateNear("MDGridPoint(all)", "all arguments", "The weight is stored", All.GetWeight(), 0.25, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the getters and setters
bool UTDGridPoint::TestGettersSetters()
{
  bool Passed = true;

  MDGridPoint Point(0, 0, 0);

  Point.SetXGrid(11);
  Passed = Evaluate("SetXGrid/GetXGrid", "11", "The x grid is changed", Point.GetXGrid(), 11U) && Passed;
  Passed = Evaluate("SetXGrid/GetYGrid", "11", "The y grid is not changed", Point.GetYGrid(), 0U) && Passed;
  Point.SetYGrid(12);
  Passed = Evaluate("SetYGrid/GetYGrid", "12", "The y grid is changed", Point.GetYGrid(), 12U) && Passed;
  Passed = Evaluate("SetYGrid/GetZGrid", "12", "The z grid is not changed", Point.GetZGrid(), 0U) && Passed;
  Point.SetZGrid(13);
  Passed = Evaluate("SetZGrid/GetZGrid", "13", "The z grid is changed", Point.GetZGrid(), 13U) && Passed;
  Passed = Evaluate("SetZGrid/GetXGrid", "13", "The x grid is not changed", Point.GetXGrid(), 11U) && Passed;

  Point.SetPosition(MVector(-0.25, 0.5, 0.75));
  Passed = EvaluateVectorNear("SetPosition/GetPosition", "interior", "The position is changed", Point.GetPosition(), MVector(-0.25, 0.5, 0.75), 0.0) && Passed;
  Point.SetEnergy(662.0);
  Passed = EvaluateNear("SetEnergy/GetEnergy", "662 keV", "The energy is changed", Point.GetEnergy(), 662.0, 0.0) && Passed;
  Point.SetTime(3.5);
  Passed = EvaluateNear("SetTime/GetTime", "3.5 s", "The time is changed", Point.GetTime(), 3.5, 0.0) && Passed;
  Point.SetHits(9);
  Passed = Evaluate("SetHits/GetHits", "9", "The number of hits is changed", Point.GetHits(), 9U) && Passed;
  Point.SetHits(0);
  Passed = Evaluate("SetHits/GetHits", "0", "Zero hits are stored", Point.GetHits(), 0U) && Passed;
  Point.SetWeight(0.125);
  Passed = EvaluateNear("SetWeight/GetWeight", "0.125", "The weight is changed", Point.GetWeight(), 0.125, 0.0) && Passed;

  set<unsigned int> Origins = { 3, 1, 7 };
  Point.SetOriginIDs(Origins);
  Passed = EvaluateTrue("SetOriginIDs/GetOriginIDs", "three origins", "The origins are stored", Point.GetOriginIDs() == Origins) && Passed;
  Point.SetOriginIDs(set<unsigned int>());
  Passed = EvaluateSize("SetOriginIDs/GetOriginIDs", "empty", "An empty list replaces the origins", Point.GetOriginIDs().size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test Set
bool UTDGridPoint::TestSet()
{
  bool Passed = true;

  MDGridPoint Point(9, 9, 9, MDGridPoint::c_XYAnger, MVector(9.0, 9.0, 9.0), 9.0, 9.0, 9, 9.0);

  Point.Set(1, 2, 3);
  Passed = Evaluate("Set(x, y, z)", "default arguments", "The x grid is set", Point.GetXGrid(), 1U) && Passed;
  Passed = Evaluate("Set(x, y, z)", "default arguments", "The y grid is set", Point.GetYGrid(), 2U) && Passed;
  Passed = Evaluate("Set(x, y, z)", "default arguments", "The z grid is set", Point.GetZGrid(), 3U) && Passed;
  Passed = Evaluate("Set(x, y, z)", "default arguments", "The type goes back to the voxel", Point.GetType(), MDGridPoint::c_Voxel) && Passed;
  Passed = EvaluateVectorNear("Set(x, y, z)", "default arguments", "The position goes back to the origin", Point.GetPosition(), MVector(0.0, 0.0, 0.0), 0.0) && Passed;
  Passed = EvaluateNear("Set(x, y, z)", "default arguments", "The energy goes back to 0", Point.GetEnergy(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("Set(x, y, z)", "default arguments", "The time goes back to 0", Point.GetTime(), 0.0, 0.0) && Passed;
  Passed = Evaluate("Set(x, y, z)", "default arguments", "The number of hits goes back to 1", Point.GetHits(), 1U) && Passed;
  Passed = EvaluateNear("Set(x, y, z)", "default arguments", "The weight goes back to 1", Point.GetWeight(), 1.0, 0.0) && Passed;

  Point.Set(4, 5, 6, MDGridPoint::c_GuardRing, MVector(1.0, 2.0, 3.0), 4.0, 5.0, 6, 7.0);
  Passed = Evaluate("Set(all)", "all arguments", "The x grid is set", Point.GetXGrid(), 4U) && Passed;
  Passed = Evaluate("Set(all)", "all arguments", "The y grid is set", Point.GetYGrid(), 5U) && Passed;
  Passed = Evaluate("Set(all)", "all arguments", "The z grid is set", Point.GetZGrid(), 6U) && Passed;
  Passed = Evaluate("Set(all)", "all arguments", "The type is set", Point.GetType(), MDGridPoint::c_GuardRing) && Passed;
  Passed = EvaluateVectorNear("Set(all)", "all arguments", "The position is set", Point.GetPosition(), MVector(1.0, 2.0, 3.0), 0.0) && Passed;
  Passed = EvaluateNear("Set(all)", "all arguments", "The energy is set", Point.GetEnergy(), 4.0, 0.0) && Passed;
  Passed = EvaluateNear("Set(all)", "all arguments", "The time is set", Point.GetTime(), 5.0, 0.0) && Passed;
  Passed = Evaluate("Set(all)", "all arguments", "The number of hits is set", Point.GetHits(), 6U) && Passed;
  Passed = EvaluateNear("Set(all)", "all arguments", "The weight is set", Point.GetWeight(), 7.0, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the copy constructor and the assignment
bool UTDGridPoint::TestCopyAndAssignment()
{
  bool Passed = true;

  set<unsigned int> Origins = { 2, 4 };
  MDGridPoint Point(1, 2, 3, MDGridPoint::c_VoxelDrift, MVector(0.1, 0.2, 0.3), 10.0, 20.0, 30, 40.0);
  Point.SetOriginIDs(Origins);

  MDGridPoint Copy(Point);
  Passed = Evaluate("MDGridPoint(const MDGridPoint&)", "all members", "The x grid is copied", Copy.GetXGrid(), 1U) && Passed;
  Passed = Evaluate("MDGridPoint(const MDGridPoint&)", "all members", "The y grid is copied", Copy.GetYGrid(), 2U) && Passed;
  Passed = Evaluate("MDGridPoint(const MDGridPoint&)", "all members", "The z grid is copied", Copy.GetZGrid(), 3U) && Passed;
  Passed = Evaluate("MDGridPoint(const MDGridPoint&)", "all members", "The type is copied", Copy.GetType(), MDGridPoint::c_VoxelDrift) && Passed;
  Passed = EvaluateVectorNear("MDGridPoint(const MDGridPoint&)", "all members", "The position is copied", Copy.GetPosition(), MVector(0.1, 0.2, 0.3), 0.0) && Passed;
  Passed = EvaluateNear("MDGridPoint(const MDGridPoint&)", "all members", "The energy is copied", Copy.GetEnergy(), 10.0, 0.0) && Passed;
  Passed = EvaluateNear("MDGridPoint(const MDGridPoint&)", "all members", "The time is copied", Copy.GetTime(), 20.0, 0.0) && Passed;
  Passed = Evaluate("MDGridPoint(const MDGridPoint&)", "all members", "The number of hits is copied", Copy.GetHits(), 30U) && Passed;
  Passed = EvaluateNear("MDGridPoint(const MDGridPoint&)", "all members", "The weight is copied", Copy.GetWeight(), 40.0, 0.0) && Passed;
  Passed = EvaluateTrue("MDGridPoint(const MDGridPoint&)", "all members", "The origins are copied", Copy.GetOriginIDs() == Origins) && Passed;

  // The copy is independent:
  Copy.SetEnergy(11.0);
  Copy.SetOriginIDs(set<unsigned int>({ 5 }));
  Passed = EvaluateNear("MDGridPoint(const MDGridPoint&)", "independence", "A change of the copy does not change the energy of the original", Point.GetEnergy(), 10.0, 0.0) && Passed;
  Passed = EvaluateTrue("MDGridPoint(const MDGridPoint&)", "independence", "A change of the copy does not change the origins of the original", Point.GetOriginIDs() == Origins) && Passed;

  // The assignment replaces everything, also old origins:
  MDGridPoint Assigned(9, 9, 9, MDGridPoint::c_GuardRing, MVector(9.0, 9.0, 9.0), 9.0, 9.0, 9, 9.0);
  Assigned.SetOriginIDs(set<unsigned int>({ 8, 9, 10 }));
  const MDGridPoint& Result = (Assigned = Point);
  Passed = EvaluateTrue("operator=", "all members", "The assignment returns this object", &Result == &Assigned) && Passed;
  Passed = Evaluate("operator=", "all members", "The x grid is assigned", Assigned.GetXGrid(), 1U) && Passed;
  Passed = Evaluate("operator=", "all members", "The y grid is assigned", Assigned.GetYGrid(), 2U) && Passed;
  Passed = Evaluate("operator=", "all members", "The z grid is assigned", Assigned.GetZGrid(), 3U) && Passed;
  Passed = Evaluate("operator=", "all members", "The type is assigned", Assigned.GetType(), MDGridPoint::c_VoxelDrift) && Passed;
  Passed = EvaluateVectorNear("operator=", "all members", "The position is assigned", Assigned.GetPosition(), MVector(0.1, 0.2, 0.3), 0.0) && Passed;
  Passed = EvaluateNear("operator=", "all members", "The energy is assigned", Assigned.GetEnergy(), 10.0, 0.0) && Passed;
  Passed = EvaluateNear("operator=", "all members", "The time is assigned", Assigned.GetTime(), 20.0, 0.0) && Passed;
  Passed = Evaluate("operator=", "all members", "The number of hits is assigned", Assigned.GetHits(), 30U) && Passed;
  Passed = EvaluateNear("operator=", "all members", "The weight is assigned", Assigned.GetWeight(), 40.0, 0.0) && Passed;
  Passed = EvaluateTrue("operator=", "all members", "The origins replace the old ones", Assigned.GetOriginIDs() == Origins) && Passed;

  // An empty origin list also replaces old origins:
  MDGridPoint WithoutOrigins(1, 1, 1);
  Assigned = WithoutOrigins;
  Passed = EvaluateSize("operator=", "no origins", "The assignment of a point without origins removes the old ones", Assigned.GetOriginIDs().size(), 0) && Passed;

  // The alias hides the self assignment from the compiler warning:
  MDGridPoint Self(1, 2, 3, MDGridPoint::c_Voxel, MVector(0.0, 0.0, 0.5), 10.0);
  Self.SetOriginIDs(Origins);
  const MDGridPoint& Alias = Self;
  Self = Alias;
  Passed = EvaluateNear("operator=", "self assignment", "The self assignment keeps the energy", Self.GetEnergy(), 10.0, 0.0) && Passed;
  // Defect GEO-E-6: The self assignment clears the origin list before it copies it
  Passed = ExpectedToFail(EvaluateTrue("operator=", "self assignment", "The self assignment keeps the origins", Self.GetOriginIDs() == Origins)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the equality of grid points
bool UTDGridPoint::TestEquality()
{
  bool Passed = true;

  // Voxels are equal if type and all indices are equal, everything else does not matter:
  const vector<unsigned int> VoxelTypes = { MDGridPoint::c_Voxel, MDGridPoint::c_VoxelDrift };
  for (unsigned int Type: VoxelTypes) {
    MDGridPoint A(1, 2, 3, Type, MVector(0.1, 0.2, 0.3), 10.0, 1.0, 1, 1.0);
    MDGridPoint Same(1, 2, 3, Type, MVector(0.7, 0.8, 0.9), 99.0, 7.0, 5, 3.0);
    MDGridPoint OtherX(0, 2, 3, Type);
    MDGridPoint OtherY(1, 0, 3, Type);
    MDGridPoint OtherZ(1, 2, 0, Type);
    MString Input = MString("Type ") + Type;
    Passed = EvaluateTrue("operator==", Input, "A point is equal to itself", A == A) && Passed;
    Passed = EvaluateTrue("operator==", Input, "Position, energy, time, hits and weight are not compared", A == Same) && Passed;
    Passed = EvaluateFalse("operator==", Input, "A different x index is not equal", A == OtherX) && Passed;
    Passed = EvaluateFalse("operator==", Input, "A different y index is not equal", A == OtherY) && Passed;
    Passed = EvaluateFalse("operator==", Input, "A different z index is not equal", A == OtherZ) && Passed;
  }

  // Different types are never equal, even with the same indices:
  MDGridPoint Voxel(1, 2, 3, MDGridPoint::c_Voxel);
  MDGridPoint Drift(1, 2, 3, MDGridPoint::c_VoxelDrift);
  MDGridPoint GuardRing(1, 2, 3, MDGridPoint::c_GuardRing);
  MDGridPoint Unknown(1, 2, 3, MDGridPoint::c_Unknown);
  Passed = EvaluateFalse("operator==", "voxel and drift", "Different types are not equal", Voxel == Drift) && Passed;
  Passed = EvaluateFalse("operator==", "voxel and guard ring", "Different types are not equal", Voxel == GuardRing) && Passed;
  Passed = EvaluateFalse("operator==", "guard ring and voxel", "The comparison is symmetric", GuardRing == Voxel) && Passed;
  Passed = EvaluateFalse("operator==", "voxel and unknown", "Different types are not equal", Voxel == Unknown) && Passed;

  // Attention: A detector has one guard ring and one Anger grid point - all of these have indices (0, 0, 0) and belong together
  const vector<unsigned int> SinglePointTypes = { MDGridPoint::c_GuardRing, MDGridPoint::c_XYAnger, MDGridPoint::c_XYZAnger };
  for (unsigned int Type: SinglePointTypes) {
    MDGridPoint A(0, 0, 0, Type, MVector(0.1, 0.2, 0.3), 10.0);
    MDGridPoint B(0, 0, 0, Type, MVector(0.7, 0.8, 0.9), 20.0);
    MString Input = MString("Type ") + Type;
    Passed = EvaluateTrue("operator==", Input, "Two points of a type with one point per detector are equal", A == B) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the equality of the types which have no grid point comparison
bool UTDGridPoint::TestEqualityUnsupportedTypes()
{
  bool Passed = true;

  // Different strips are different grid points, independent of massert being active or not
  MDGridPoint XStripOne(1, 0, 0, MDGridPoint::c_XStrip);
  MDGridPoint XStripTwo(2, 0, 0, MDGridPoint::c_XStrip);
  Passed = EvaluateFalse("operator==", "x strips 1 and 2", "Two different x strips are not equal", XStripOne == XStripTwo) && Passed;

  MDGridPoint YStripOne(0, 1, 0, MDGridPoint::c_YStrip);
  MDGridPoint YStripTwo(0, 2, 0, MDGridPoint::c_YStrip);
  Passed = EvaluateFalse("operator==", "y strips 1 and 2", "Two different y strips are not equal", YStripOne == YStripTwo) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding grid points in the same voxel
bool UTDGridPoint::TestAddVoxel()
{
  bool Passed = true;

  const vector<unsigned int> VoxelTypes = { MDGridPoint::c_Voxel, MDGridPoint::c_VoxelDrift };
  for (unsigned int Type: VoxelTypes) {
    MString Input = MString("Type ") + Type;

    MDGridPoint A(1, 2, 3, Type, MVector(0.4, 0.5, 1.0), 10.0, 5.0, 2, 1.0);
    MDGridPoint B(1, 2, 3, Type, MVector(-0.1, 0.2, 3.0), 30.0, 7.0, 3, 2.0);
    const MDGridPoint& Result = (A += B);

    Passed = EvaluateTrue("operator+=", Input, "The addition returns this object", &Result == &A) && Passed;
    Passed = Evaluate("operator+=", Input, "The indices stay", A.GetXGrid(), 1U) && Passed;
    Passed = Evaluate("operator+=", Input, "The type stays", A.GetType(), Type) && Passed;
    Passed = EvaluateNear("operator+=", Input, "The energies add up: 10 + 30", A.GetEnergy(), 40.0, 1e-12) && Passed;
    // Energy weighted depth: (10*1 + 30*3)/40 = 2.5
    Passed = EvaluateNear("operator+=", Input, "The depth is energy weighted", A.GetPosition().Z(), 2.5, 1e-12) && Passed;
    // Attention: A voxel only has a depth inside - there is no lateral information
    Passed = EvaluateNear("operator+=", Input, "The x position is set to the voxel center", A.GetPosition().X(), 0.0, 0.0) && Passed;
    Passed = EvaluateNear("operator+=", Input, "The y position is set to the voxel center", A.GetPosition().Y(), 0.0, 0.0) && Passed;
    Passed = EvaluateNear("operator+=", Input, "The earlier time stays", A.GetTime(), 5.0, 0.0) && Passed;
    Passed = Evaluate("operator+=", Input, "The hits add up: 2 + 3", A.GetHits(), 5U) && Passed;
    // Hit weighted mean: (2*1 + 3*2)/5 = 1.6
    Passed = EvaluateNear("operator+=", Input, "The weight is the hit weighted mean", A.GetWeight(), 1.6, 1e-12) && Passed;

    // The time of the second point is earlier:
    MDGridPoint C(1, 2, 3, Type, MVector(0.0, 0.0, 0.0), 1.0, 9.0);
    MDGridPoint D(1, 2, 3, Type, MVector(0.0, 0.0, 0.0), 1.0, 8.0);
    C += D;
    Passed = EvaluateNear("operator+=", Input, "The earlier time of the added point is taken", C.GetTime(), 8.0, 0.0) && Passed;

    // Three depositions one after the other, (1*1 + 1*2 + 2*5)/4 = 3.25:
    MDGridPoint E1(0, 0, 0, Type, MVector(0.0, 0.0, 1.0), 1.0);
    MDGridPoint E2(0, 0, 0, Type, MVector(0.0, 0.0, 2.0), 1.0);
    MDGridPoint E3(0, 0, 0, Type, MVector(0.0, 0.0, 5.0), 2.0);
    E1 += E2;
    E1 += E3;
    Passed = EvaluateNear("operator+=", Input, "The depth of three depositions is the energy weighted mean", E1.GetPosition().Z(), 3.25, 1e-12) && Passed;
    Passed = EvaluateNear("operator+=", Input, "The energy of three depositions is the sum", E1.GetEnergy(), 4.0, 1e-12) && Passed;
    Passed = Evaluate("operator+=", Input, "The hits of three depositions are the sum", E1.GetHits(), 3U) && Passed;

    // One deposition without energy does not move the depth:
    MDGridPoint F1(0, 0, 0, Type, MVector(0.0, 0.0, 1.5), 7.0);
    MDGridPoint F2(0, 0, 0, Type, MVector(0.0, 0.0, 9.0), 0.0);
    F1 += F2;
    Passed = EvaluateNear("operator+=", Input, "A deposition without energy does not move the depth", F1.GetPosition().Z(), 1.5, 1e-12) && Passed;

    // Adding the point to itself:
    MDGridPoint Self(1, 1, 1, Type, MVector(0.0, 0.0, 1.5), 7.0, 3.0, 2, 0.5);
    const MDGridPoint& Alias = Self;
    Self += Alias;
    Passed = EvaluateNear("operator+=", Input, "Adding a point to itself doubles the energy", Self.GetEnergy(), 14.0, 1e-12) && Passed;
    Passed = EvaluateNear("operator+=", Input, "Adding a point to itself keeps the depth", Self.GetPosition().Z(), 1.5, 1e-12) && Passed;
    Passed = EvaluateNear("operator+=", Input, "Adding a point to itself keeps the time", Self.GetTime(), 3.0, 0.0) && Passed;
    Passed = Evaluate("operator+=", Input, "Adding a point to itself doubles the hits", Self.GetHits(), 4U) && Passed;
    Passed = EvaluateNear("operator+=", Input, "Adding a point to itself keeps the weight", Self.GetWeight(), 0.5, 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding grid points in different voxels
bool UTDGridPoint::TestAddDifferentVoxels()
{
  bool Passed = true;

  // The function only adds if the points are in the same grid:
  MDGridPoint A(1, 2, 3, MDGridPoint::c_Voxel, MVector(0.0, 0.0, 1.0), 10.0, 5.0, 2, 1.0);
  MDGridPoint B(2, 2, 3, MDGridPoint::c_Voxel, MVector(0.0, 0.0, 3.0), 30.0, 3.0, 2, 3.0);
  B.SetOriginIDs(set<unsigned int>({ 4 }));
  A += B;

  Passed = EvaluateNear("operator+=", "different voxel", "The energy stays", A.GetEnergy(), 10.0, 0.0) && Passed;
  Passed = EvaluateNear("operator+=", "different voxel", "The depth stays", A.GetPosition().Z(), 1.0, 0.0) && Passed;
  Passed = EvaluateNear("operator+=", "different voxel", "The time stays", A.GetTime(), 5.0, 0.0) && Passed;
  Passed = EvaluateSize("operator+=", "different voxel", "No origin is added", A.GetOriginIDs().size(), 0) && Passed;

  Passed = Evaluate("operator+=", "different voxel", "The hits stay", A.GetHits(), 2U) && Passed;
  Passed = EvaluateNear("operator+=", "different voxel", "The weight stays", A.GetWeight(), 1.0, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding Anger camera grid points
bool UTDGridPoint::TestAddAnger()
{
  bool Passed = true;

  MDGridPoint XYZ1(0, 0, 0, MDGridPoint::c_XYZAnger, MVector(1.0, 2.0, 3.0), 10.0, 5.0);
  MDGridPoint XYZ2(0, 0, 0, MDGridPoint::c_XYZAnger, MVector(5.0, -2.0, 7.0), 30.0, 4.0);
  XYZ1 += XYZ2;
  // Energy weighted: x: (10*1 + 30*5)/40 = 4, y: (10*2 - 30*2)/40 = -1, z: (10*3 + 30*7)/40 = 6
  Passed = EvaluateVectorNear("operator+=", "XYZ Anger", "The position is energy weighted in all dimensions", XYZ1.GetPosition(), MVector(4.0, -1.0, 6.0), 1e-12) && Passed;
  Passed = EvaluateNear("operator+=", "XYZ Anger", "The energies add up", XYZ1.GetEnergy(), 40.0, 1e-12) && Passed;
  Passed = EvaluateNear("operator+=", "XYZ Anger", "The earlier time is taken", XYZ1.GetTime(), 4.0, 0.0) && Passed;
  Passed = Evaluate("operator+=", "XYZ Anger", "The hits add up", XYZ1.GetHits(), 2U) && Passed;

  MDGridPoint XY1(0, 0, 0, MDGridPoint::c_XYAnger, MVector(1.0, 2.0, 0.0), 10.0);
  MDGridPoint XY2(0, 0, 0, MDGridPoint::c_XYAnger, MVector(5.0, -2.0, 0.0), 30.0);
  XY1 += XY2;
  Passed = EvaluateVectorNear("operator+=", "XY Anger", "The position is energy weighted in x and y, and z is 0", XY1.GetPosition(), MVector(4.0, -1.0, 0.0), 1e-12) && Passed;
  Passed = EvaluateNear("operator+=", "XY Anger", "The energies add up", XY1.GetEnergy(), 40.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding guard ring grid points
bool UTDGridPoint::TestAddGuardRing()
{
  bool Passed = true;

  MDGridPoint A(0, 0, 0, MDGridPoint::c_GuardRing, MVector(1.0, 2.0, 3.0), 10.0, 5.0, 2, 1.0);
  MDGridPoint B(0, 0, 0, MDGridPoint::c_GuardRing, MVector(4.0, 5.0, 6.0), 30.0, 4.0, 3, 2.0);
  A += B;

  Passed = EvaluateNear("operator+=", "guard ring", "The energies add up", A.GetEnergy(), 40.0, 1e-12) && Passed;
  Passed = EvaluateVectorNear("operator+=", "guard ring", "The position is reset - the unique position of the ring is used later", A.GetPosition(), MVector(0.0, 0.0, 0.0), 0.0) && Passed;
  Passed = Evaluate("operator+=", "guard ring", "The hits add up", A.GetHits(), 5U) && Passed;
  // Hit weighted mean: (2*1 + 3*2)/5 = 1.6
  Passed = EvaluateNear("operator+=", "guard ring", "The weight is the hit weighted mean", A.GetWeight(), 1.6, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding grid points without energy or without hits
bool UTDGridPoint::TestAddWithoutEnergyOrHits()
{
  bool Passed = true;

  // AddUndiscretized and hits with zero energy create such points, a mean of two positions has to be a position
  MDGridPoint A(0, 0, 0, MDGridPoint::c_Voxel, MVector(0.0, 0.0, 1.5), 0.0);
  MDGridPoint B(0, 0, 0, MDGridPoint::c_Voxel, MVector(0.0, 0.0, 1.5), 0.0);
  A += B;
  // Defect GEO-E-9: Two points without energy result in a division by 0 and a NaN position
  Passed = ExpectedToFail(EvaluateTrue("operator+=", "voxel, both energies 0", "The depth stays a finite value", std::isfinite(A.GetPosition().Z()))) && Passed;
  Passed = EvaluateNear("operator+=", "voxel, both energies 0", "The energy stays 0", A.GetEnergy(), 0.0, 0.0) && Passed;

  MDGridPoint C(0, 0, 0, MDGridPoint::c_XYZAnger, MVector(1.0, 2.0, 3.0), 0.0);
  MDGridPoint D(0, 0, 0, MDGridPoint::c_XYZAnger, MVector(1.0, 2.0, 3.0), 0.0);
  C += D;
  // Defect GEO-E-9: Two points without energy result in a division by 0 and a NaN position
  Passed = ExpectedToFail(EvaluateTrue("operator+=", "XYZ Anger, both energies 0", "The position stays finite", std::isfinite(C.GetPosition().X()))) && Passed;

  MDGridPoint E(0, 0, 0, MDGridPoint::c_Voxel, MVector(0.0, 0.0, 0.0), 1.0, 0.0, 0, 2.0);
  MDGridPoint F(0, 0, 0, MDGridPoint::c_Voxel, MVector(0.0, 0.0, 0.0), 1.0, 0.0, 0, 2.0);
  E += F;
  // Defect GEO-E-10: Two points without hits result in a division by 0 and a NaN weight
  Passed = ExpectedToFail(EvaluateTrue("operator+=", "voxel, both hits 0", "The weight stays finite", std::isfinite(E.GetWeight()))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding grid points of other or unsupported types
bool UTDGridPoint::TestAddOtherTypes()
{
  bool Passed = true;

  // Different types are not added:
  MDGridPoint Voxel(0, 0, 0, MDGridPoint::c_Voxel, MVector(0.0, 0.0, 1.0), 10.0, 5.0, 2, 1.0);
  MDGridPoint Drift(0, 0, 0, MDGridPoint::c_VoxelDrift, MVector(0.0, 0.0, 3.0), 30.0, 1.0, 3, 9.0);
  Drift.SetOriginIDs(set<unsigned int>({ 1 }));
  Voxel += Drift;
  Passed = EvaluateNear("operator+=", "other type", "The energy stays", Voxel.GetEnergy(), 10.0, 0.0) && Passed;
  Passed = EvaluateNear("operator+=", "other type", "The depth stays", Voxel.GetPosition().Z(), 1.0, 0.0) && Passed;
  Passed = EvaluateNear("operator+=", "other type", "The time stays", Voxel.GetTime(), 5.0, 0.0) && Passed;
  Passed = Evaluate("operator+=", "other type", "The hits stay", Voxel.GetHits(), 2U) && Passed;
  Passed = EvaluateNear("operator+=", "other type", "The weight stays", Voxel.GetWeight(), 1.0, 0.0) && Passed;
  Passed = EvaluateSize("operator+=", "other type", "No origin is added", Voxel.GetOriginIDs().size(), 0) && Passed;

  // Points of the same strip are added:
  const vector<unsigned int> Strips = { MDGridPoint::c_XStrip, MDGridPoint::c_YStrip };
  for (unsigned int Type: Strips) {
    MDGridPoint A(0, 0, 0, Type, MVector(0.0, 0.0, 1.0), 10.0, 5.0, 2, 1.0);
    MDGridPoint B(0, 0, 0, Type, MVector(0.0, 0.0, 3.0), 30.0, 1.0, 3, 9.0);
    A += B;
    MString Input = MString("Type ") + Type;
    Passed = EvaluateNear("operator+=", Input, "The energies add up", A.GetEnergy(), 40.0, 0.0) && Passed;
    Passed = Evaluate("operator+=", Input, "The hits add up", A.GetHits(), 5U) && Passed;
  }

  // The types without an addition branch report an error and do not change the point:
  const vector<unsigned int> Unsupported = { MDGridPoint::c_Unknown };
  for (unsigned int Type: Unsupported) {
    MDGridPoint A(0, 0, 0, Type, MVector(0.0, 0.0, 1.0), 10.0, 5.0, 2, 1.0);
    MDGridPoint B(0, 0, 0, Type, MVector(0.0, 0.0, 3.0), 30.0, 1.0, 3, 9.0);
    DisableDefaultStreams();
    A += B;
    EnableDefaultStreams();
    MString Input = MString("Type ") + Type;
    Passed = EvaluateNear("operator+=", Input, "The energy stays", A.GetEnergy(), 10.0, 0.0) && Passed;
    Passed = Evaluate("operator+=", Input, "The hits stay", A.GetHits(), 2U) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the origin tracking
bool UTDGridPoint::TestOrigins()
{
  bool Passed = true;

  const vector<unsigned int> Types = { MDGridPoint::c_Voxel, MDGridPoint::c_VoxelDrift, MDGridPoint::c_XYAnger, MDGridPoint::c_XYZAnger };
  for (unsigned int Type: Types) {
    MString Input = MString("Type ") + Type;

    MDGridPoint A(0, 0, 0, Type, MVector(0.0, 0.0, 0.0), 1.0);
    A.SetOriginIDs(set<unsigned int>({ 1, 2 }));
    MDGridPoint B(0, 0, 0, Type, MVector(0.0, 0.0, 0.0), 1.0);
    B.SetOriginIDs(set<unsigned int>({ 2, 3, 3 }));
    A += B;
    // Union in order of first appearance without duplicates: 1, 2, 3
    Passed = EvaluateTrue("operator+=", Input, "The origins are merged without duplicates", A.GetOriginIDs() == set<unsigned int>({ 1, 2, 3 })) && Passed;
    Passed = EvaluateTrue("operator+=", Input, "The added point keeps its origins", B.GetOriginIDs() == set<unsigned int>({ 2, 3, 3 })) && Passed;

    // Empty lists on either side:
    MDGridPoint C(0, 0, 0, Type, MVector(0.0, 0.0, 0.0), 1.0);
    C.SetOriginIDs(set<unsigned int>({ 5 }));
    MDGridPoint Empty(0, 0, 0, Type, MVector(0.0, 0.0, 0.0), 1.0);
    C += Empty;
    Passed = EvaluateTrue("operator+=", Input, "Adding a point without origins keeps the origins", C.GetOriginIDs() == set<unsigned int>({ 5 })) && Passed;
    Empty += C;
    Passed = EvaluateTrue("operator+=", Input, "A point without origins receives the origins of the added one", Empty.GetOriginIDs() == set<unsigned int>({ 5 })) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the stream output
bool UTDGridPoint::TestStream()
{
  bool Passed = true;

  const vector<unsigned int> Types = { MDGridPoint::c_Voxel, MDGridPoint::c_VoxelDrift, MDGridPoint::c_XStrip, MDGridPoint::c_YStrip,
                                       MDGridPoint::c_XYAnger, MDGridPoint::c_XYZAnger, MDGridPoint::c_GuardRing };
  const vector<MString> Names = { "(Voxel)", "(VoxelDrift)", "(XStrip)", "(YStrip)", "(XYAnger)", "(XYZAnger)", "(GuardRing)" };
  for (unsigned int t = 0; t < Types.size(); ++t) {
    MDGridPoint Point(1, 2, 3, Types[t], MVector(0.0, 0.0, 0.0), 511.0, 2.5, 4, 0.5);
    ostringstream Out;
    Out<<Point;
    MString Text(Out.str());
    Passed = EvaluateTrue("operator<<", Names[t], "The output has the type name", Text.Contains(Names[t])) && Passed;
    Passed = EvaluateTrue("operator<<", Names[t], "The output has the indices", Text.BeginsWith("Grid point (x=1, y=2, z=3) at ")) && Passed;
    Passed = EvaluateTrue("operator<<", Names[t], "The output has the energy", Text.Contains("511 keV")) && Passed;
    Passed = EvaluateTrue("operator<<", Names[t], "The output has the hits", Text.Contains("4 (original) hits")) && Passed;
    Passed = EvaluateTrue("operator<<", Names[t], "The output has the time", Text.Contains("after 2.5 s")) && Passed;
    Passed = EvaluateTrue("operator<<", Names[t], "The output has the weight", Text.Contains("weight 0.5")) && Passed;
  }

  MDGridPoint Unknown;
  ostringstream Out;
  Out<<Unknown;
  Passed = EvaluateTrue("operator<<", "unknown type", "The output says that the type is not implemented", MString(Out.str()).Contains("not (completely) implemented type")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTDGridPoint Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDGridPoint.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
