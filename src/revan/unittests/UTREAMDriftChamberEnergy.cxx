/*
 * UTREAMDriftChamberEnergy.cxx
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
using namespace std;

// MEGAlib libs:
#include "MDVolumeSequence.h"
#include "MGlobal.h"
#include "MREAM.h"
#include "MREAMDriftChamberEnergy.h"
#include "MString.h"
#include "MUnitTest.h"
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the total energy deposit in a drift chamber MREAMDriftChamberEnergy
class UTREAMDriftChamberEnergy : public MUnitTest
{
public:
  //! Default constructor
  UTREAMDriftChamberEnergy() : MUnitTest("UTREAMDriftChamberEnergy") {}
  //! Default destructor
  virtual ~UTREAMDriftChamberEnergy() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the state after the default construction
  bool TestDefaults();
  //! Test the energy and its resolution
  bool TestEnergy();
  //! Test copy constructor and clone with a volume sequence
  bool TestCopies();
  //! Test copy constructor and clone without a volume sequence
  bool TestCopiesWithoutVolumeSequence();
  //! Test the functions which are not overwritten
  bool TestBaseClassFunctions();

  //! Return true if a copy of a measurement without volume sequence keeps the energy
  bool IsCopyWithoutVolumeSequenceCorrect();
  //! Return true if a clone of a measurement without volume sequence keeps the energy
  bool IsCloneWithoutVolumeSequenceCorrect();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTREAMDriftChamberEnergy::Run()
{
  bool Passed = true;

  Passed = TestDefaults() && Passed;
  Passed = TestEnergy() && Passed;
  Passed = TestCopies() && Passed;
  Passed = TestCopiesWithoutVolumeSequence() && Passed;
  Passed = TestBaseClassFunctions() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the state after the default construction
bool UTREAMDriftChamberEnergy::TestDefaults()
{
  bool Passed = true;

  MREAMDriftChamberEnergy REAM;
  Passed = Evaluate("MREAMDriftChamberEnergy()", "default", "The type is the drift chamber energy", REAM.GetType(), MREAM::c_DriftChamberEnergy) && Passed;
  Passed = EvaluateTrue("MREAMDriftChamberEnergy()", "default", "There is no volume sequence", REAM.GetVolumeSequence() == nullptr) && Passed;
  Passed = Evaluate("MREAMDriftChamberEnergy()", "default", "The energy is not defined", REAM.GetEnergy(), g_DoubleNotDefined) && Passed;
  Passed = Evaluate("MREAMDriftChamberEnergy()", "default", "The energy resolution is not defined", REAM.GetEnergyResolution(), g_DoubleNotDefined) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the energy and its resolution
bool UTREAMDriftChamberEnergy::TestEnergy()
{
  bool Passed = true;

  MREAMDriftChamberEnergy REAM;
  REAM.SetEnergy(662.5);
  Passed = EvaluateNear("SetEnergy()", "662.5 keV", "The energy is returned", REAM.GetEnergy(), 662.5, 1e-12) && Passed;
  Passed = Evaluate("SetEnergy()", "662.5 keV", "The energy resolution is unchanged", REAM.GetEnergyResolution(), g_DoubleNotDefined) && Passed;
  REAM.SetEnergyResolution(12.25);
  Passed = EvaluateNear("SetEnergyResolution()", "12.25 keV", "The energy resolution is returned", REAM.GetEnergyResolution(), 12.25, 1e-12) && Passed;
  Passed = EvaluateNear("SetEnergyResolution()", "12.25 keV", "The energy is unchanged", REAM.GetEnergy(), 662.5, 1e-12) && Passed;

  // The class stores what it gets:
  REAM.SetEnergy(0.0);
  Passed = EvaluateNear("SetEnergy()", "zero", "A zero energy is stored", REAM.GetEnergy(), 0.0, 1e-12) && Passed;
  REAM.SetEnergy(-1.5);
  Passed = EvaluateNear("SetEnergy()", "negative", "A negative energy is stored unchanged", REAM.GetEnergy(), -1.5, 1e-12) && Passed;
  REAM.SetEnergyResolution(0.0);
  Passed = EvaluateNear("SetEnergyResolution()", "zero", "A zero resolution is stored", REAM.GetEnergyResolution(), 0.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy constructor and clone with a volume sequence
bool UTREAMDriftChamberEnergy::TestCopies()
{
  bool Passed = true;

  MREAMDriftChamberEnergy Original;
  MDVolumeSequence* Sequence = new MDVolumeSequence();
  Sequence->SetPositionInSensitiveVolume(MVector(1.0, 2.0, 3.0));
  Original.SetVolumeSequence(Sequence);
  Original.SetEnergy(662.5);
  Original.SetEnergyResolution(12.25);

  // Copy constructor:
  MREAMDriftChamberEnergy Copy(Original);
  Passed = Evaluate("MREAMDriftChamberEnergy(const MREAMDriftChamberEnergy&)", "with sequence", "The type is copied", Copy.GetType(), MREAM::c_DriftChamberEnergy) && Passed;
  Passed = EvaluateNear("MREAMDriftChamberEnergy(const MREAMDriftChamberEnergy&)", "with sequence", "The energy is copied", Copy.GetEnergy(), 662.5, 1e-12) && Passed;
  Passed = EvaluateNear("MREAMDriftChamberEnergy(const MREAMDriftChamberEnergy&)", "with sequence", "The energy resolution is copied", Copy.GetEnergyResolution(), 12.25, 1e-12) && Passed;
  Passed = EvaluateTrue("MREAMDriftChamberEnergy(const MREAMDriftChamberEnergy&)", "with sequence", "The copy has its own sequence", Copy.GetVolumeSequence() != nullptr && Copy.GetVolumeSequence() != Sequence) && Passed;
  if (Copy.GetVolumeSequence() != nullptr) {
    Passed = EvaluateVectorNear("MREAMDriftChamberEnergy(const MREAMDriftChamberEnergy&)", "with sequence", "The content of the sequence is copied", Copy.GetVolumeSequence()->GetPositionInSensitiveVolume(), MVector(1.0, 2.0, 3.0), 1e-15) && Passed;
  }

  // The copy is independent:
  Original.SetEnergy(100.0);
  Original.SetEnergyResolution(1.0);
  Passed = EvaluateNear("MREAMDriftChamberEnergy(const MREAMDriftChamberEnergy&)", "original changed", "The energy of the copy does not follow the original", Copy.GetEnergy(), 662.5, 1e-12) && Passed;
  Passed = EvaluateNear("MREAMDriftChamberEnergy(const MREAMDriftChamberEnergy&)", "original changed", "The energy resolution of the copy does not follow the original", Copy.GetEnergyResolution(), 12.25, 1e-12) && Passed;

  // Clone through the base class:
  MREAM* Base = &Original;
  MREAM* Clone = Base->Clone();
  MREAMDriftChamberEnergy* TypedClone = dynamic_cast<MREAMDriftChamberEnergy*>(Clone);
  Passed = EvaluateTrue("Clone()", "through the base class", "The clone is a drift chamber energy", TypedClone != nullptr) && Passed;
  if (TypedClone != nullptr) {
    Passed = Evaluate("Clone()", "through the base class", "The type is copied", TypedClone->GetType(), MREAM::c_DriftChamberEnergy) && Passed;
    Passed = EvaluateNear("Clone()", "through the base class", "The energy is copied", TypedClone->GetEnergy(), 100.0, 1e-12) && Passed;
    Passed = EvaluateNear("Clone()", "through the base class", "The energy resolution is copied", TypedClone->GetEnergyResolution(), 1.0, 1e-12) && Passed;
    Passed = EvaluateTrue("Clone()", "through the base class", "The clone has its own sequence", TypedClone->GetVolumeSequence() != nullptr && TypedClone->GetVolumeSequence() != Sequence) && Passed;
  }
  delete Clone;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy constructor and clone without a volume sequence
bool UTREAMDriftChamberEnergy::TestCopiesWithoutVolumeSequence()
{
  bool Passed = true;

  // Defect REV-C-1: The copy constructor dereferences the volume sequence without a check, and a new measurement has none
  Passed = ExpectedToCrash(EvaluateTrue("MREAMDriftChamberEnergy(const MREAMDriftChamberEnergy&)", "without sequence", "The copy of a measurement without sequence keeps the energy and has no sequence", IsCopyWithoutVolumeSequenceCorrect())) && Passed;
  // Defect REV-C-1: Clone uses the copy constructor
  Passed = ExpectedToCrash(EvaluateTrue("Clone()", "without sequence", "The clone of a measurement without sequence keeps the energy and has no sequence", IsCloneWithoutVolumeSequenceCorrect())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the functions which are not overwritten
bool UTREAMDriftChamberEnergy::TestBaseClassFunctions()
{
  bool Passed = true;

  MREAMDriftChamberEnergy REAM;
  REAM.SetEnergy(662.5);
  Passed = Evaluate("ParseLine()", "XE line", "The measurement does not parse a line itself: 2", REAM.ParseLine("XE 1;2;3;662.5", 1), 2) && Passed;
  Passed = Evaluate("ToString()", "level 1", "The text is the one of the base class with the type 1", REAM.ToString(1), MString("   Basic REAM of Type 1\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a copy of a measurement without volume sequence keeps the energy
bool UTREAMDriftChamberEnergy::IsCopyWithoutVolumeSequenceCorrect()
{
  MREAMDriftChamberEnergy Original;
  Original.SetEnergy(662.5);
  MREAMDriftChamberEnergy Copy(Original);
  if (Copy.GetVolumeSequence() != nullptr) {
    return false;
  }
  if (Copy.GetEnergy() != 662.5) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a clone of a measurement without volume sequence keeps the energy
bool UTREAMDriftChamberEnergy::IsCloneWithoutVolumeSequenceCorrect()
{
  MREAMDriftChamberEnergy Original;
  Original.SetEnergy(662.5);
  MREAM* Clone = Original.Clone();
  MREAMDriftChamberEnergy* TypedClone = dynamic_cast<MREAMDriftChamberEnergy*>(Clone);
  bool IsCorrect = false;
  if (TypedClone != nullptr && TypedClone->GetVolumeSequence() == nullptr && TypedClone->GetEnergy() == 662.5) {
    IsCorrect = true;
  }
  delete Clone;

  return IsCorrect;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTREAMDriftChamberEnergy Test;
  return Test.Run() == true ? 0 : 1;
}


// UTREAMDriftChamberEnergy.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
