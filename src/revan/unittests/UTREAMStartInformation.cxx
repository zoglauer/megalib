/*
 * UTREAMStartInformation.cxx
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
#include <iostream>
#include <sstream>
using namespace std;

// MEGAlib libs:
#include "MDVolumeSequence.h"
#include "MGlobal.h"
#include "MREAM.h"
#include "MREAMStartInformation.h"
#include "MString.h"
#include "MUnitTest.h"
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the start information of the original particle MREAMStartInformation
class UTREAMStartInformation : public MUnitTest
{
public:
  //! Default constructor
  UTREAMStartInformation() : MUnitTest("UTREAMStartInformation") {}
  //! Default destructor
  virtual ~UTREAMStartInformation() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the state after the default construction
  bool TestDefaults();
  //! Test position, direction, polarization, energy, and particle ID
  bool TestGettersSetters();
  //! Test copy constructor and clone
  bool TestCopies();
  //! Test the text output
  bool TestToString();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTREAMStartInformation::Run()
{
  bool Passed = true;

  Passed = TestDefaults() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestCopies() && Passed;
  Passed = TestToString() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the state after the default construction
bool UTREAMStartInformation::TestDefaults()
{
  bool Passed = true;

  MREAMStartInformation REAM;
  Passed = Evaluate("MREAMStartInformation()", "default", "The type is the start information", REAM.GetType(), MREAM::c_StartInformation) && Passed;
  Passed = EvaluateTrue("MREAMStartInformation()", "default", "There is no volume sequence", REAM.GetVolumeSequence() == nullptr) && Passed;
  Passed = EvaluateVectorNear("MREAMStartInformation()", "default", "The position is the origin", REAM.GetPosition(), MVector(0.0, 0.0, 0.0), 1e-15) && Passed;
  Passed = EvaluateVectorNear("MREAMStartInformation()", "default", "The direction is zero", REAM.GetDirection(), MVector(0.0, 0.0, 0.0), 1e-15) && Passed;
  Passed = EvaluateVectorNear("MREAMStartInformation()", "default", "The polarization is zero", REAM.GetPolarization(), MVector(0.0, 0.0, 0.0), 1e-15) && Passed;
  Passed = EvaluateNear("MREAMStartInformation()", "default", "The energy is zero", REAM.GetEnergy(), 0.0, 1e-15) && Passed;
  Passed = Evaluate("MREAMStartInformation()", "default", "The particle ID is not defined", REAM.GetParticleID(), g_IntNotDefined) && Passed;
  Passed = Evaluate("ParseLine()", "OI line", "The measurement does not parse a line itself: 2", REAM.ParseLine("OI 1;2;3;0;0;1;1;0;0;511", 1), 2) && Passed;
  Passed = Evaluate("ToEvtaString()", "default", "There is no evta representation", REAM.ToEvtaString(5, 1), MString("")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test position, direction, polarization, energy, and particle ID
bool UTREAMStartInformation::TestGettersSetters()
{
  bool Passed = true;

  // Each value is different to detect swapped members:
  MREAMStartInformation REAM;
  REAM.SetPosition(MVector(1.5, -2.5, 30.0));
  REAM.SetDirection(MVector(0.6, 0.0, -0.8));
  REAM.SetPolarization(MVector(0.0, 1.0, 0.0));
  REAM.SetEnergy(1808.7);
  REAM.SetParticleID(4);
  Passed = EvaluateVectorNear("SetPosition()", "(1.5, -2.5, 30)", "The position is returned", REAM.GetPosition(), MVector(1.5, -2.5, 30.0), 1e-15) && Passed;
  Passed = EvaluateVectorNear("SetDirection()", "(0.6, 0, -0.8)", "The direction is returned", REAM.GetDirection(), MVector(0.6, 0.0, -0.8), 1e-15) && Passed;
  Passed = EvaluateVectorNear("SetPolarization()", "(0, 1, 0)", "The polarization is returned", REAM.GetPolarization(), MVector(0.0, 1.0, 0.0), 1e-15) && Passed;
  Passed = EvaluateNear("SetEnergy()", "1808.7 keV", "The energy is returned", REAM.GetEnergy(), 1808.7, 1e-12) && Passed;
  Passed = Evaluate("SetParticleID()", "4", "The particle ID is returned", REAM.GetParticleID(), 4) && Passed;

  // Overwrite on the same object:
  REAM.SetPosition(MVector(0.0, 0.0, 0.0));
  REAM.SetEnergy(0.0);
  REAM.SetParticleID(g_IntNotDefined);
  Passed = EvaluateVectorNear("SetPosition()", "origin", "The position can be set back to the origin", REAM.GetPosition(), MVector(0.0, 0.0, 0.0), 1e-15) && Passed;
  Passed = EvaluateNear("SetEnergy()", "zero", "The energy can be set back to zero", REAM.GetEnergy(), 0.0, 1e-15) && Passed;
  Passed = Evaluate("SetParticleID()", "not defined", "The particle ID can be set back to not defined", REAM.GetParticleID(), g_IntNotDefined) && Passed;
  Passed = EvaluateVectorNear("SetPosition()", "origin", "The direction is unchanged", REAM.GetDirection(), MVector(0.6, 0.0, -0.8), 1e-15) && Passed;
  Passed = EvaluateVectorNear("SetPosition()", "origin", "The polarization is unchanged", REAM.GetPolarization(), MVector(0.0, 1.0, 0.0), 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy constructor and clone
bool UTREAMStartInformation::TestCopies()
{
  bool Passed = true;

  // The start information has no volume sequence: it is not a measurement in a detector
  MREAMStartInformation Original;
  Original.SetPosition(MVector(1.5, -2.5, 30.0));
  Original.SetDirection(MVector(0.6, 0.0, -0.8));
  Original.SetPolarization(MVector(0.0, 1.0, 0.0));
  Original.SetEnergy(1808.7);
  Original.SetParticleID(4);

  // Copy constructor:
  MREAMStartInformation Copy(Original);
  Passed = Evaluate("MREAMStartInformation(const MREAMStartInformation&)", "all values set", "The type is copied", Copy.GetType(), MREAM::c_StartInformation) && Passed;
  Passed = EvaluateVectorNear("MREAMStartInformation(const MREAMStartInformation&)", "all values set", "The position is copied", Copy.GetPosition(), MVector(1.5, -2.5, 30.0), 1e-15) && Passed;
  Passed = EvaluateVectorNear("MREAMStartInformation(const MREAMStartInformation&)", "all values set", "The direction is copied", Copy.GetDirection(), MVector(0.6, 0.0, -0.8), 1e-15) && Passed;
  Passed = EvaluateVectorNear("MREAMStartInformation(const MREAMStartInformation&)", "all values set", "The polarization is copied", Copy.GetPolarization(), MVector(0.0, 1.0, 0.0), 1e-15) && Passed;
  Passed = EvaluateNear("MREAMStartInformation(const MREAMStartInformation&)", "all values set", "The energy is copied", Copy.GetEnergy(), 1808.7, 1e-12) && Passed;
  Passed = Evaluate("MREAMStartInformation(const MREAMStartInformation&)", "all values set", "The particle ID is copied", Copy.GetParticleID(), 4) && Passed;
  Passed = EvaluateTrue("MREAMStartInformation(const MREAMStartInformation&)", "all values set", "The copy has no volume sequence", Copy.GetVolumeSequence() == nullptr) && Passed;

  // The copy is independent:
  Original.SetEnergy(511.0);
  Original.SetParticleID(1);
  Passed = EvaluateNear("MREAMStartInformation(const MREAMStartInformation&)", "original changed", "The energy of the copy does not follow the original", Copy.GetEnergy(), 1808.7, 1e-12) && Passed;
  Passed = Evaluate("MREAMStartInformation(const MREAMStartInformation&)", "original changed", "The particle ID of the copy does not follow the original", Copy.GetParticleID(), 4) && Passed;

  // Clone through the base class:
  MREAM* Base = &Original;
  MREAM* Clone = Base->Clone();
  MREAMStartInformation* TypedClone = dynamic_cast<MREAMStartInformation*>(Clone);
  Passed = EvaluateTrue("Clone()", "through the base class", "The clone is a start information", TypedClone != nullptr) && Passed;
  if (TypedClone != nullptr) {
    Passed = Evaluate("Clone()", "through the base class", "The type is copied", TypedClone->GetType(), MREAM::c_StartInformation) && Passed;
    Passed = EvaluateVectorNear("Clone()", "through the base class", "The position is copied", TypedClone->GetPosition(), MVector(1.5, -2.5, 30.0), 1e-15) && Passed;
    Passed = EvaluateVectorNear("Clone()", "through the base class", "The direction is copied", TypedClone->GetDirection(), MVector(0.6, 0.0, -0.8), 1e-15) && Passed;
    Passed = EvaluateVectorNear("Clone()", "through the base class", "The polarization is copied", TypedClone->GetPolarization(), MVector(0.0, 1.0, 0.0), 1e-15) && Passed;
    Passed = EvaluateNear("Clone()", "through the base class", "The energy is copied", TypedClone->GetEnergy(), 511.0, 1e-12) && Passed;
    Passed = Evaluate("Clone()", "through the base class", "The particle ID is copied", TypedClone->GetParticleID(), 1) && Passed;
  }
  delete Clone;

  // A default object can be copied:
  MREAMStartInformation Default;
  MREAMStartInformation DefaultCopy(Default);
  Passed = Evaluate("MREAMStartInformation(const MREAMStartInformation&)", "default", "The particle ID of a default object stays not defined", DefaultCopy.GetParticleID(), g_IntNotDefined) && Passed;
  Passed = EvaluateNear("MREAMStartInformation(const MREAMStartInformation&)", "default", "The energy of a default object stays zero", DefaultCopy.GetEnergy(), 0.0, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output
bool UTREAMStartInformation::TestToString()
{
  bool Passed = true;

  MREAMStartInformation REAM;
  REAM.SetPosition(MVector(1.5, -2.5, 30.0));
  REAM.SetDirection(MVector(0.6, 0.0, -0.8));

  // The vectors are written by their stream operator:
  ostringstream Expected;
  Expected<<"REAMStartInformation: original position="<<MVector(1.5, -2.5, 30.0)<<", original direction="<<MVector(0.6, 0.0, -0.8)<<endl;
  Passed = Evaluate("ToString()", "position and direction", "The text has position and direction", REAM.ToString(), MString(Expected.str().c_str())) && Passed;
  Passed = Evaluate("ToString()", "level 1", "Level 1 is indented by 3 blanks", REAM.ToString(1), MString("   ") + Expected.str().c_str()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTREAMStartInformation Test;
  return Test.Run() == true ? 0 : 1;
}


// UTREAMStartInformation.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
