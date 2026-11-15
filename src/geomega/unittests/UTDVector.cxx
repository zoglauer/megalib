/*
 * UTDVector.cxx
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
using namespace std;

// MEGAlib libs:
#include "MDVector.h"
#include "MUnitTest.h"
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the named list of positions and values MDVector
//! Attention: the declared copy constructor has no definition and cannot be linked, thus it is not tested
class UTDVector : public MUnitTest
{
public:
  //! Default constructor
  UTDVector() : MUnitTest("UTDVector") {}
  //! Default destructor
  virtual ~UTDVector() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the name handling
  bool TestName();
  //! Test adding and reading back entries
  bool TestAdd();
  //! Test the out of bounds behavior
  bool TestOutOfBounds();
  //! Test the assignment into a populated vector and to itself
  bool TestAssignment();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDVector::Run()
{
  bool Passed = true;

  Passed = TestName() && Passed;
  Passed = TestAdd() && Passed;
  Passed = TestOutOfBounds() && Passed;
  Passed = TestAssignment() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the name handling
bool UTDVector::TestName()
{
  bool Passed = true;

  MDVector Default;
  Passed = EvaluateTrue("MDVector()", "no name", "The default name is empty", Default.GetName() == "") && Passed;

  MDVector Named("Positions");
  Passed = EvaluateTrue("GetName()", "constructor argument", "The name of the constructor is stored", Named.GetName() == "Positions") && Passed;

  Named.SetName("Other");
  Passed = EvaluateTrue("SetName()", "new name", "The new name replaces the old one", Named.GetName() == "Other") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding and reading back entries
bool UTDVector::TestAdd()
{
  bool Passed = true;

  MDVector Vector("V");
  Passed = EvaluateSize("GetSize()", "empty", "A new vector has no entries", Vector.GetSize(), 0) && Passed;

  // The positions do not need any order and duplicates are allowed:
  Vector.Add(MVector(1.5, -2.5, 3.25), 7.5);
  Vector.Add(MVector(-1.0, 0.0, 12.0), -0.25);
  Vector.Add(MVector(1.5, -2.5, 3.25), 7.5);
  Passed = EvaluateSize("GetSize()", "three entries", "All three entries are stored", Vector.GetSize(), 3) && Passed;

  Passed = EvaluateVectorNear("GetPosition()", "first", "The first position is returned", Vector.GetPosition(0), MVector(1.5, -2.5, 3.25), 1e-12) && Passed;
  Passed = EvaluateVectorNear("GetPosition()", "second", "The second position is returned", Vector.GetPosition(1), MVector(-1.0, 0.0, 12.0), 1e-12) && Passed;
  Passed = EvaluateNear("GetValue()", "first", "The first value is returned", Vector.GetValue(0), 7.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetValue()", "second", "The second value belongs to the second position", Vector.GetValue(1), -0.25, 1e-12) && Passed;
  Passed = EvaluateNear("GetValue()", "duplicate", "A duplicate entry is kept", Vector.GetValue(2), 7.5, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the out of bounds behavior
bool UTDVector::TestOutOfBounds()
{
  bool Passed = true;

  MDVector Empty;
  Passed = EvaluateNear("GetValue()", "empty, index 0", "An empty vector returns g_DoubleNotDefined", Empty.GetValue(0), g_DoubleNotDefined, 1e-12) && Passed;
  Passed = EvaluateTrue("GetPosition()", "empty, index 0", "An empty vector returns g_VectorNotDefined", Empty.GetPosition(0) == g_VectorNotDefined) && Passed;

  // The boundary n - 1 is valid, n is not:
  MDVector Vector;
  Vector.Add(MVector(1.0, 2.0, 3.0), 4.0);
  Passed = EvaluateNear("GetValue()", "index n - 1", "The last entry is valid", Vector.GetValue(0), 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetValue()", "index n", "The first index after the end returns g_DoubleNotDefined", Vector.GetValue(1), g_DoubleNotDefined, 1e-12) && Passed;
  Passed = EvaluateTrue("GetPosition()", "index n", "The first index after the end returns g_VectorNotDefined", Vector.GetPosition(1) == g_VectorNotDefined) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the assignment into a populated vector and to itself
bool UTDVector::TestAssignment()
{
  bool Passed = true;

  MDVector Source("Source");
  Source.Add(MVector(1, 2, 3), 4.5);
  MDVector Target("Target");
  Target.Add(MVector(7, 8, 9), 1.5);
  Target.Add(MVector(9, 8, 7), 2.5);

  Target = Source;
  Passed = Evaluate("operator=", "name", "The name is assigned", Target.GetName(), MString("Source")) && Passed;
  Passed = Evaluate("operator=", "size", "The old values are gone", Target.GetSize(), 1U) && Passed;
  Passed = EvaluateNear("operator=", "value", "The value is assigned", Target.GetValue(0), 4.5, 0.0) && Passed;

  Target.Add(MVector(0, 0, 1), 5.5);
  Passed = Evaluate("operator=", "independence", "A change of the assigned vector does not change the source", Source.GetSize(), 1U) && Passed;

  MDVector& Same = Target;
  Target = Same;
  Passed = Evaluate("operator=", "itself", "An assignment to itself keeps the size", Target.GetSize(), 2U) && Passed;
  Passed = EvaluateNear("operator=", "itself", "An assignment to itself keeps the values", Target.GetValue(1), 5.5, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTDVector Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDVector.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
