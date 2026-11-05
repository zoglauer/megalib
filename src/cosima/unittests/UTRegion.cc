/*
 * UTRegion.cc
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
#include "MCRegion.hh"

// MEGAlib:
#include "MGlobal.h"
#include "MString.h"
#include "MUnitTest.h"

// Geant4:
#include "G4SystemOfUnits.hh"

// Standard lib:
#include <vector>
using namespace std;


//! Unit test class for MCRegion
class UTRegion : public MUnitTest
{
public:
  //! Default constructor
  UTRegion() : MUnitTest("UTRegion") {}
  //! Default destructor
  virtual ~UTRegion() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the constructors and the default values
  bool TestDefaultConstruction();
  //! Test the getters and setters
  bool TestGettersSetters();
  //! Test copy construction and assignment
  bool TestCopyAndAssignment();
  //! Test storing regions in containers
  bool TestContainer();
};


////////////////////////////////////////////////////////////////////////////////


bool UTRegion::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestCopyAndAssignment() && Passed;
  Passed = TestContainer() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRegion::TestDefaultConstruction()
{
  bool Passed = true;

  MCRegion Default;
  Passed = Evaluate("MCRegion()", "default", "The default name is empty", Default.GetName(), MString("")) && Passed;
  Passed = Evaluate("MCRegion()", "default", "The default volume name is not defined", Default.GetVolumeName(), g_StringNotDefined) && Passed;
  Passed = EvaluateNear("MCRegion()", "default", "The default range cut is not defined", Default.GetRangeCut(), g_DoubleNotDefined, 1e-12*g_DoubleNotDefined) && Passed;
  Passed = EvaluateFalse("MCRegion()", "default", "By default secondaries are not cut", Default.GetCutAllSecondaries()) && Passed;

  MCRegion Named("Crystal");
  Passed = Evaluate("MCRegion(Name)", "Crystal", "The name is stored", Named.GetName(), MString("Crystal")) && Passed;
  Passed = Evaluate("MCRegion(Name)", "Crystal", "The volume name is not defined", Named.GetVolumeName(), g_StringNotDefined) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRegion::TestGettersSetters()
{
  bool Passed = true;

  MCRegion Region;
  Region.SetName("Shield");
  Passed = Evaluate("SetName/GetName", "Shield", "GetName returns the name set before", Region.GetName(), MString("Shield")) && Passed;
  Region.SetName("");
  Passed = Evaluate("SetName/GetName", "empty", "An empty name can be set", Region.GetName(), MString("")) && Passed;

  Passed = EvaluateTrue("SetVolumeName()", "ShieldVolume", "SetVolumeName returns true", Region.SetVolumeName("ShieldVolume")) && Passed;
  Passed = Evaluate("GetVolumeName()", "ShieldVolume", "GetVolumeName returns the name set before", Region.GetVolumeName(), MString("ShieldVolume")) && Passed;

  Passed = EvaluateTrue("SetRangeCut()", "0.1 mm", "SetRangeCut returns true", Region.SetRangeCut(0.1*mm)) && Passed;
  Passed = EvaluateNear("GetRangeCut()", "0.1 mm", "GetRangeCut returns the cut set before", Region.GetRangeCut()/mm, 0.1, 1e-12) && Passed;
  Passed = EvaluateTrue("SetRangeCut()", "zero", "A zero range cut can be set", Region.SetRangeCut(0.0)) && Passed;
  Passed = EvaluateNear("GetRangeCut()", "zero", "A zero range cut is stored", Region.GetRangeCut(), 0.0, 1e-12) && Passed;

  Passed = EvaluateTrue("SetCutAllSecondaries()", "true", "SetCutAllSecondaries returns true", Region.SetCutAllSecondaries(true)) && Passed;
  Passed = EvaluateTrue("GetCutAllSecondaries()", "true", "The flag is set", Region.GetCutAllSecondaries()) && Passed;
  Passed = EvaluateTrue("SetCutAllSecondaries()", "false", "SetCutAllSecondaries returns true when clearing the flag", Region.SetCutAllSecondaries(false)) && Passed;
  Passed = EvaluateFalse("GetCutAllSecondaries()", "false", "The flag is cleared", Region.GetCutAllSecondaries()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRegion::TestCopyAndAssignment()
{
  bool Passed = true;

  MCRegion A("Original");
  A.SetVolumeName("VolumeA");
  A.SetRangeCut(0.5*mm);
  A.SetCutAllSecondaries(true);

  MCRegion B(A);
  Passed = Evaluate("MCRegion(const MCRegion&)", "copy", "The copy keeps the name", B.GetName(), MString("Original")) && Passed;
  Passed = Evaluate("MCRegion(const MCRegion&)", "copy", "The copy keeps the volume name", B.GetVolumeName(), MString("VolumeA")) && Passed;
  Passed = EvaluateNear("MCRegion(const MCRegion&)", "copy", "The copy keeps the range cut", B.GetRangeCut()/mm, 0.5, 1e-12) && Passed;
  Passed = EvaluateTrue("MCRegion(const MCRegion&)", "copy", "The copy keeps the flag", B.GetCutAllSecondaries()) && Passed;

  B.SetName("Copy");
  B.SetRangeCut(2.0*mm);
  Passed = Evaluate("MCRegion(const MCRegion&)", "independence", "Changing the copy does not change the original", A.GetName(), MString("Original")) && Passed;
  Passed = EvaluateNear("MCRegion(const MCRegion&)", "independence", "Changing the copy does not change the original range cut", A.GetRangeCut()/mm, 0.5, 1e-12) && Passed;

  MCRegion C;
  C = A;
  Passed = Evaluate("operator=", "assign", "The assigned region has the name", C.GetName(), MString("Original")) && Passed;
  Passed = EvaluateNear("operator=", "assign", "The assigned region has the range cut", C.GetRangeCut()/mm, 0.5, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRegion::TestContainer()
{
  bool Passed = true;

  // cosima keeps regions in vectors, thus they are copied:
  vector<MCRegion> Regions;
  for (unsigned int i = 0; i < 20; ++i) {
    MCRegion R(MString("Region") + i);
    R.SetRangeCut(0.1*mm*(i+1));
    R.SetVolumeName(MString("Volume") + i);
    Regions.push_back(R);
  }
  bool AllOK = true;
  for (unsigned int i = 0; i < Regions.size(); ++i) {
    if (Regions[i].GetName() != MString("Region") + i) AllOK = false;
    if (Regions[i].GetVolumeName() != MString("Volume") + i) AllOK = false;
    if (fabs(Regions[i].GetRangeCut()/mm - 0.1*(i+1)) > 1e-9) AllOK = false;
  }
  Passed = EvaluateTrue("vector<MCRegion>", "20 regions", "Regions survive being stored in a growing vector", AllOK) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTRegion Test;
  return Test.Run() == true ? 0 : 1;
}
