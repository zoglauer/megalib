/*
 * UTREAMDirectional.cxx
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
#include <cstring>
#include <iostream>
#include <new>
#include <sstream>
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MDVolumeSequence.h"
#include "MGlobal.h"
#include "MREAM.h"
#include "MREAMDirectional.h"
#include "MREHit.h"
#include "MString.h"
#include "MUnitTest.h"
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the direction of a charged particle MREAMDirectional
class UTREAMDirectional : public MUnitTest
{
public:
  //! Default constructor
  UTREAMDirectional() : MUnitTest("UTREAMDirectional") {}
  //! Default destructor
  virtual ~UTREAMDirectional() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the state after the default construction
  bool TestDefaults();
  //! Test that the constructors set the energy
  bool TestEnergyInitialization();
  //! Test direction, energy, and the linked RESE
  bool TestGettersSetters();
  //! Test copy constructor and clone with a volume sequence
  bool TestCopies();
  //! Test copy constructor and clone without a volume sequence
  bool TestCopiesWithoutVolumeSequence();
  //! Test the text output
  bool TestToString();

  //! Return true if a copy of a measurement without volume sequence keeps the direction
  bool IsCopyWithoutVolumeSequenceCorrect();

  //! The byte the memory is filled with before a construction - no valid default of a member
  static const unsigned char c_FillByte = 0xA5;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTREAMDirectional::Run()
{
  bool Passed = true;

  Passed = TestDefaults() && Passed;
  Passed = TestEnergyInitialization() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestCopies() && Passed;
  Passed = TestCopiesWithoutVolumeSequence() && Passed;
  Passed = TestToString() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the state after the default construction
bool UTREAMDirectional::TestDefaults()
{
  bool Passed = true;

  MREAMDirectional REAM;
  Passed = Evaluate("MREAMDirectional()", "default", "The type is the directional measurement", REAM.GetType(), MREAM::c_Directional) && Passed;
  Passed = EvaluateTrue("MREAMDirectional()", "default", "There is no volume sequence", REAM.GetVolumeSequence() == nullptr) && Passed;
  Passed = EvaluateTrue("MREAMDirectional()", "default", "The direction is not defined", REAM.GetDirection() == g_VectorNotDefined) && Passed;
  Passed = EvaluateTrue("MREAMDirectional()", "default", "There is no RESE", REAM.GetRESE() == nullptr) && Passed;
  Passed = Evaluate("ParseLine()", "DR line", "The measurement does not parse a line itself: 2", REAM.ParseLine("DR 1;2;3;0;0;1;80", 1), 2) && Passed;
  Passed = Evaluate("ToEvtaString()", "default", "There is no evta representation", REAM.ToEvtaString(5, 1), MString("")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the constructors set the energy
bool UTREAMDirectional::TestEnergyInitialization()
{
  bool Passed = true;

  // Construct into a block with a known pattern:
  vector<unsigned char> Block(sizeof(MREAMDirectional));
  FillMemory(Block.data(), Block.size(), c_FillByte);
  MREAMDirectional* REAM = new (Block.data()) MREAMDirectional();
  double Energy = REAM->GetEnergy();
  REAM->~MREAMDirectional();
  // Defect REV-C-3: The default constructor does not initialize the energy
  Passed = ExpectedToFail(EvaluateFalse("MREAMDirectional()", "memory filled with 0xA5", "The default constructor sets the energy", IsFillPattern(&Energy, sizeof(double), c_FillByte))) && Passed;

  // The same for the copy constructor:
  MREAMDirectional Original;
  Original.SetVolumeSequence(new MDVolumeSequence());
  Original.SetEnergy(80.5);
  FillMemory(Block.data(), Block.size(), c_FillByte);
  MREAMDirectional* Copy = new (Block.data()) MREAMDirectional(Original);
  double CopiedEnergy = Copy->GetEnergy();
  Copy->~MREAMDirectional();
  // Defect REV-C-3: The copy constructor does not copy the energy
  Passed = ExpectedToFail(EvaluateNear("MREAMDirectional(const MREAMDirectional&)", "energy 80.5 keV", "The copy constructor copies the energy", CopiedEnergy, 80.5, 1e-12)) && Passed;

  // And the clone, which uses the copy constructor: its memory is not under our control, thus it is not checked
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test direction, energy, and the linked RESE
bool UTREAMDirectional::TestGettersSetters()
{
  bool Passed = true;

  MREAMDirectional REAM;
  REAM.SetDirection(MVector(0.6, 0.0, -0.8));
  Passed = EvaluateVectorNear("SetDirection()", "(0.6, 0, -0.8)", "The direction is returned", REAM.GetDirection(), MVector(0.6, 0.0, -0.8), 1e-15) && Passed;
  REAM.SetDirection(MVector(0.0, 3.0, 4.0));
  Passed = EvaluateVectorNear("SetDirection()", "not normalized", "A direction which is not a unit vector is stored unchanged", REAM.GetDirection(), MVector(0.0, 3.0, 4.0), 1e-15) && Passed;

  REAM.SetEnergy(80.5);
  Passed = EvaluateNear("SetEnergy()", "80.5 keV", "The energy is returned", REAM.GetEnergy(), 80.5, 1e-12) && Passed;
  REAM.SetEnergy(0.0);
  Passed = EvaluateNear("SetEnergy()", "zero", "A zero energy is stored", REAM.GetEnergy(), 0.0, 1e-15) && Passed;

  // The RESE is not owned:
  MREHit Hit;
  REAM.SetRESE(&Hit);
  Passed = EvaluateTrue("SetRESE()", "hit", "The RESE is returned", REAM.GetRESE() == &Hit) && Passed;
  Passed = EvaluateVectorNear("SetRESE()", "hit", "The direction is unchanged", REAM.GetDirection(), MVector(0.0, 3.0, 4.0), 1e-15) && Passed;
  REAM.SetRESE(nullptr);
  Passed = EvaluateTrue("SetRESE()", "nullptr", "The RESE can be removed", REAM.GetRESE() == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy constructor and clone with a volume sequence
bool UTREAMDirectional::TestCopies()
{
  bool Passed = true;

  MREHit Hit;
  MREAMDirectional Original;
  MDVolumeSequence* Sequence = new MDVolumeSequence();
  Sequence->SetPositionInSensitiveVolume(MVector(1.0, 2.0, 3.0));
  Original.SetVolumeSequence(Sequence);
  Original.SetDirection(MVector(0.6, 0.0, -0.8));
  Original.SetEnergy(80.5);
  Original.SetRESE(&Hit);

  // Copy constructor - the energy is checked in TestEnergyInitialization:
  MREAMDirectional Copy(Original);
  Passed = Evaluate("MREAMDirectional(const MREAMDirectional&)", "with sequence", "The type is copied", Copy.GetType(), MREAM::c_Directional) && Passed;
  Passed = EvaluateVectorNear("MREAMDirectional(const MREAMDirectional&)", "with sequence", "The direction is copied", Copy.GetDirection(), MVector(0.6, 0.0, -0.8), 1e-15) && Passed;
  Passed = EvaluateTrue("MREAMDirectional(const MREAMDirectional&)", "with sequence", "The link to the RESE is copied", Copy.GetRESE() == &Hit) && Passed;
  Passed = EvaluateTrue("MREAMDirectional(const MREAMDirectional&)", "with sequence", "The copy has its own sequence", Copy.GetVolumeSequence() != nullptr && Copy.GetVolumeSequence() != Sequence) && Passed;
  if (Copy.GetVolumeSequence() != nullptr) {
    Passed = EvaluateVectorNear("MREAMDirectional(const MREAMDirectional&)", "with sequence", "The content of the sequence is copied", Copy.GetVolumeSequence()->GetPositionInSensitiveVolume(), MVector(1.0, 2.0, 3.0), 1e-15) && Passed;
  }

  // The copy is independent:
  Original.SetDirection(MVector(1.0, 0.0, 0.0));
  Passed = EvaluateVectorNear("MREAMDirectional(const MREAMDirectional&)", "original changed", "The direction of the copy does not follow the original", Copy.GetDirection(), MVector(0.6, 0.0, -0.8), 1e-15) && Passed;

  // Clone through the base class:
  MREAM* Base = &Original;
  MREAM* Clone = Base->Clone();
  MREAMDirectional* TypedClone = dynamic_cast<MREAMDirectional*>(Clone);
  Passed = EvaluateTrue("Clone()", "through the base class", "The clone is a directional measurement", TypedClone != nullptr) && Passed;
  if (TypedClone != nullptr) {
    Passed = Evaluate("Clone()", "through the base class", "The type is copied", TypedClone->GetType(), MREAM::c_Directional) && Passed;
    Passed = EvaluateVectorNear("Clone()", "through the base class", "The direction is copied", TypedClone->GetDirection(), MVector(1.0, 0.0, 0.0), 1e-15) && Passed;
    Passed = EvaluateTrue("Clone()", "through the base class", "The link to the RESE is copied", TypedClone->GetRESE() == &Hit) && Passed;
    Passed = EvaluateTrue("Clone()", "through the base class", "The clone has its own sequence", TypedClone->GetVolumeSequence() != nullptr && TypedClone->GetVolumeSequence() != Sequence) && Passed;
  }
  delete Clone;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy constructor and clone without a volume sequence
bool UTREAMDirectional::TestCopiesWithoutVolumeSequence()
{
  bool Passed = true;

  // Defect REV-C-1: The copy constructor dereferences the volume sequence without a check, and a new measurement has none
  Passed = ExpectedToCrash(EvaluateTrue("MREAMDirectional(const MREAMDirectional&)", "without sequence", "The copy of a measurement without sequence keeps the direction and has no sequence", IsCopyWithoutVolumeSequenceCorrect())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output
bool UTREAMDirectional::TestToString()
{
  bool Passed = true;

  MREAMDirectional REAM;
  REAM.SetDirection(MVector(0.0, 0.0, 1.0));
  REAM.SetEnergy(80.5);

  // The vector is written by its stream operator:
  ostringstream Direction;
  Direction<<MVector(0.0, 0.0, 1.0);
  MString Expected = MString("REAMDirectional: Dir:") + Direction.str().c_str() + ", Energy:80.5, RESE-ID:none\n";
  Passed = Evaluate("ToString()", "no RESE", "The text has direction, energy, and no RESE", REAM.ToString(), Expected) && Passed;
  Passed = Evaluate("ToString()", "level 2", "Level 2 is indented by 6 blanks", REAM.ToString(2), MString("      ") + Expected) && Passed;

  MREHit Hit;
  REAM.SetRESE(&Hit);
  ostringstream WithID;
  WithID<<"REAMDirectional: Dir:"<<Direction.str()<<", Energy:80.5, RESE-ID:"<<Hit.GetID()<<endl;
  Passed = Evaluate("ToString()", "with RESE", "The text has the ID of the RESE", REAM.ToString(), MString(WithID.str().c_str())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a copy of a measurement without volume sequence keeps the direction
bool UTREAMDirectional::IsCopyWithoutVolumeSequenceCorrect()
{
  MREAMDirectional Original;
  Original.SetDirection(MVector(0.0, 0.0, 1.0));
  MREAMDirectional Copy(Original);
  if (Copy.GetVolumeSequence() != nullptr) {
    return false;
  }
  if (Copy.GetDirection() != MVector(0.0, 0.0, 1.0)) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTREAMDirectional Test;
  return Test.Run() == true ? 0 : 1;
}


// UTREAMDirectional.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
