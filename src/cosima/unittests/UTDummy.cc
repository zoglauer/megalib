/*
 * UTDummy.cc
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


// Cosima:
#include "MCDummy.hh"

// MEGAlib:
#include "MUnitTest.h"

// Standard lib:
#include <type_traits>
#include <vector>
using namespace std;


//! Unit test class for MCDummy
class UTDummy : public MUnitTest
{
public:
  //! Default constructor
  UTDummy() : MUnitTest("UTDummy") {}
  //! Default destructor
  virtual ~UTDummy() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test construction and destruction
  bool TestLifecycle();
  //! Test the properties of the class as a base class
  bool TestBaseClassProperties();

  //! A class deriving from MCDummy which records its destruction
  class Derived : public MCDummy
  {
  public:
    //! Constructor storing the flag to set on destruction
    Derived(bool* Flag) : m_Flag(Flag) {}
    //! Destructor setting the flag
    virtual ~Derived() { *m_Flag = true; }
  private:
    //! The flag to set on destruction
    bool* m_Flag;
  };
};


////////////////////////////////////////////////////////////////////////////////


bool UTDummy::Run()
{
  bool Passed = true;

  Passed = TestLifecycle() && Passed;
  Passed = TestBaseClassProperties() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDummy::TestLifecycle()
{
  bool Passed = true;

  MCDummy* Dummy = new MCDummy();
  Passed = EvaluateTrue("MCDummy()", "new", "A dummy can be created on the heap", Dummy != nullptr) && Passed;
  delete Dummy;

  // Repeated creation and destruction:
  vector<MCDummy*> Many;
  for (unsigned int i = 0; i < 100; ++i) Many.push_back(new MCDummy());
  Passed = EvaluateSize("MCDummy()", "100 dummies", "Many dummies can be created", Many.size(), 100) && Passed;
  for (MCDummy* D: Many) delete D;

  MCDummy OnStack;
  (void) OnStack;
  Passed = EvaluateTrue("MCDummy()", "stack", "A dummy can be created on the stack", true) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTDummy::TestBaseClassProperties()
{
  bool Passed = true;

  Passed = EvaluateTrue("~MCDummy()", "traits", "The destructor is virtual", has_virtual_destructor<MCDummy>::value) && Passed;

  bool Destroyed = false;
  MCDummy* Base = new Derived(&Destroyed);
  delete Base;
  Passed = EvaluateTrue("~MCDummy()", "delete via base pointer", "Deleting a derived class via the base pointer runs the derived destructor", Destroyed) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTDummy Test;
  return Test.Run() == true ? 0 : 1;
}
