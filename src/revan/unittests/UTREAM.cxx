/*
 * UTREAM.cxx
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
#include "MREAM.h"
#include "MString.h"
#include "MUnitTest.h"
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the base class of the additional measurements MREAM
class UTREAM : public MUnitTest
{
public:
  //! Default constructor
  UTREAM() : MUnitTest("UTREAM") {}
  //! Default destructor
  virtual ~UTREAM() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the type constants
  bool TestConstants();
  //! Test the state after the default construction
  bool TestDefaults();
  //! Test setting and getting the volume sequence
  bool TestVolumeSequence();
  //! Test copy constructor, clone, and assignment with a volume sequence
  bool TestCopies();
  //! Test copy constructor, clone, and assignment without a volume sequence
  bool TestCopiesWithoutVolumeSequence();
  //! Test the text output
  bool TestToString();

  //! Return true if a copy of a measurement without volume sequence has no volume sequence
  bool IsCopyWithoutVolumeSequenceEmpty();
  //! Return true if a clone of a measurement without volume sequence has no volume sequence
  bool IsCloneWithoutVolumeSequenceEmpty();
  //! Return true if a measurement has no volume sequence after the assignment of one without
  bool IsAssignedWithoutVolumeSequenceEmpty();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTREAM::Run()
{
  bool Passed = true;

  Passed = TestConstants() && Passed;
  Passed = TestDefaults() && Passed;
  Passed = TestVolumeSequence() && Passed;
  Passed = TestCopies() && Passed;
  Passed = TestCopiesWithoutVolumeSequence() && Passed;
  Passed = TestToString() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the type constants
bool UTREAM::TestConstants()
{
  bool Passed = true;

  Passed = Evaluate("c_Unknown", "constant", "The unknown type is 0", MREAM::c_Unknown, 0U) && Passed;
  Passed = Evaluate("c_DriftChamberEnergy", "constant", "The drift chamber energy is type 1", MREAM::c_DriftChamberEnergy, 1U) && Passed;
  Passed = Evaluate("c_GuardRingHit", "constant", "The guard ring hit is type 2", MREAM::c_GuardRingHit, 2U) && Passed;
  Passed = Evaluate("c_Directional", "constant", "The directional measurement is type 3", MREAM::c_Directional, 3U) && Passed;
  Passed = Evaluate("c_StartInformation", "constant", "The start information is type 4", MREAM::c_StartInformation, 4U) && Passed;
  Passed = Evaluate("c_Min", "constant", "The lowest type is the drift chamber energy", MREAM::c_Min, MREAM::c_DriftChamberEnergy) && Passed;
  Passed = Evaluate("c_Max", "constant", "The highest type is the start information", MREAM::c_Max, MREAM::c_StartInformation) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the state after the default construction
bool UTREAM::TestDefaults()
{
  bool Passed = true;

  MREAM REAM;
  Passed = Evaluate("MREAM()", "default", "The type is unknown", REAM.GetType(), MREAM::c_Unknown) && Passed;
  Passed = EvaluateTrue("MREAM()", "default", "There is no volume sequence", REAM.GetVolumeSequence() == nullptr) && Passed;
  Passed = Evaluate("ParseLine()", "any line", "The base class does not parse a line: 2", REAM.ParseLine("GR 1;2;3;4", 1), 2) && Passed;
  Passed = Evaluate("ParseLine()", "empty line", "The base class does not parse an empty line: 2", REAM.ParseLine("", 1), 2) && Passed;
  Passed = Evaluate("ToEvtaString()", "default", "The base class has no evta representation", REAM.ToEvtaString(5, 1), MString("")) && Passed;
  Passed = Evaluate("ToEvtaString()", "default version", "The base class has no evta representation with the default version", REAM.ToEvtaString(0), MString("")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test setting and getting the volume sequence
bool UTREAM::TestVolumeSequence()
{
  bool Passed = true;

  // The measurement owns the sequence and deletes it
  MREAM REAM;
  MDVolumeSequence* Sequence = new MDVolumeSequence();
  Sequence->SetPositionInSensitiveVolume(MVector(1.0, 2.0, 3.0));
  REAM.SetVolumeSequence(Sequence);
  Passed = EvaluateTrue("SetVolumeSequence()", "sequence", "The sequence which was set is returned", REAM.GetVolumeSequence() == Sequence) && Passed;
  Passed = EvaluateVectorNear("GetVolumeSequence()", "sequence", "The sequence still has its position", REAM.GetVolumeSequence()->GetPositionInSensitiveVolume(), MVector(1.0, 2.0, 3.0), 1e-15) && Passed;
  Passed = EvaluateTrue("GetVolumeSequence()", "second call", "A second call returns the same sequence", REAM.GetVolumeSequence() == Sequence) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy constructor, clone, and assignment with a volume sequence
bool UTREAM::TestCopies()
{
  bool Passed = true;

  MREAM Original;
  MDVolumeSequence* Sequence = new MDVolumeSequence();
  Sequence->SetPositionInSensitiveVolume(MVector(1.0, 2.0, 3.0));
  Original.SetVolumeSequence(Sequence);

  // Copy constructor:
  MREAM Copy(Original);
  Passed = Evaluate("MREAM(const MREAM&)", "with sequence", "The type is copied", Copy.GetType(), MREAM::c_Unknown) && Passed;
  Passed = EvaluateTrue("MREAM(const MREAM&)", "with sequence", "The copy has a sequence", Copy.GetVolumeSequence() != nullptr) && Passed;
  Passed = EvaluateTrue("MREAM(const MREAM&)", "with sequence", "The copy has its own sequence", Copy.GetVolumeSequence() != Sequence) && Passed;
  if (Copy.GetVolumeSequence() != nullptr) {
    Passed = EvaluateVectorNear("MREAM(const MREAM&)", "with sequence", "The content of the sequence is copied", Copy.GetVolumeSequence()->GetPositionInSensitiveVolume(), MVector(1.0, 2.0, 3.0), 1e-15) && Passed;
    Sequence->SetPositionInSensitiveVolume(MVector(4.0, 5.0, 6.0));
    Passed = EvaluateVectorNear("MREAM(const MREAM&)", "original changed", "The copy does not follow a change of the original", Copy.GetVolumeSequence()->GetPositionInSensitiveVolume(), MVector(1.0, 2.0, 3.0), 1e-15) && Passed;
  }

  // Clone:
  MREAM* Clone = Original.Clone();
  Passed = EvaluateTrue("Clone()", "with sequence", "A clone is created", Clone != nullptr) && Passed;
  if (Clone != nullptr) {
    Passed = Evaluate("Clone()", "with sequence", "The type is copied", Clone->GetType(), MREAM::c_Unknown) && Passed;
    Passed = EvaluateTrue("Clone()", "with sequence", "The clone has its own sequence", Clone->GetVolumeSequence() != nullptr && Clone->GetVolumeSequence() != Sequence) && Passed;
    if (Clone->GetVolumeSequence() != nullptr) {
      Passed = EvaluateVectorNear("Clone()", "with sequence", "The content of the sequence is copied", Clone->GetVolumeSequence()->GetPositionInSensitiveVolume(), MVector(4.0, 5.0, 6.0), 1e-15) && Passed;
    }
    delete Clone;
  }

  // Assignment:
  MREAM Assigned;
  Assigned = Original;
  Passed = EvaluateTrue("operator=", "with sequence", "The assigned measurement has its own sequence", Assigned.GetVolumeSequence() != nullptr && Assigned.GetVolumeSequence() != Sequence) && Passed;
  if (Assigned.GetVolumeSequence() != nullptr) {
    Passed = EvaluateVectorNear("operator=", "with sequence", "The content of the sequence is copied", Assigned.GetVolumeSequence()->GetPositionInSensitiveVolume(), MVector(4.0, 5.0, 6.0), 1e-15) && Passed;
  }

  // Self assignment:
  MDVolumeSequence* Before = Assigned.GetVolumeSequence();
  MREAM& Self = Assigned;
  Assigned = Self;
  Passed = EvaluateTrue("operator=", "self assignment", "A self assignment keeps the sequence", Assigned.GetVolumeSequence() == Before) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy constructor, clone, and assignment without a volume sequence
bool UTREAM::TestCopiesWithoutVolumeSequence()
{
  bool Passed = true;

  // Defect REV-C-1: The copy constructor dereferences the volume sequence without a check, and a new measurement has none
  Passed = ExpectedToCrash(EvaluateTrue("MREAM(const MREAM&)", "without sequence", "The copy of a measurement without sequence has no sequence", IsCopyWithoutVolumeSequenceEmpty())) && Passed;
  // Defect REV-C-1: Clone uses the copy constructor
  Passed = ExpectedToCrash(EvaluateTrue("Clone()", "without sequence", "The clone of a measurement without sequence has no sequence", IsCloneWithoutVolumeSequenceEmpty())) && Passed;
  // Defect REV-C-1: The assignment dereferences the volume sequence without a check
  Passed = ExpectedToCrash(EvaluateTrue("operator=", "without sequence", "The assignment of a measurement without sequence gives no sequence", IsAssignedWithoutVolumeSequenceEmpty())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output
bool UTREAM::TestToString()
{
  bool Passed = true;

  MREAM REAM;
  Passed = Evaluate("ToString()", "default", "The text names the type", REAM.ToString(), MString("Basic REAM of Type 0\n")) && Passed;
  Passed = Evaluate("ToString()", "level 0", "Level 0 has no indentation", REAM.ToString(0), MString("Basic REAM of Type 0\n")) && Passed;
  Passed = Evaluate("ToString()", "level 2", "Level 2 is indented by 6 blanks", REAM.ToString(2), MString("      Basic REAM of Type 0\n")) && Passed;
  Passed = Evaluate("ToString()", "negative level", "A negative level has no indentation", REAM.ToString(-1), MString("Basic REAM of Type 0\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a copy of a measurement without volume sequence has no volume sequence
bool UTREAM::IsCopyWithoutVolumeSequenceEmpty()
{
  MREAM Original;
  MREAM Copy(Original);
  if (Copy.GetVolumeSequence() != nullptr) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a clone of a measurement without volume sequence has no volume sequence
bool UTREAM::IsCloneWithoutVolumeSequenceEmpty()
{
  MREAM Original;
  MREAM* Clone = Original.Clone();
  bool IsEmpty = false;
  if (Clone->GetVolumeSequence() == nullptr) {
    IsEmpty = true;
  }
  delete Clone;

  return IsEmpty;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a measurement has no volume sequence after the assignment of one without
bool UTREAM::IsAssignedWithoutVolumeSequenceEmpty()
{
  MREAM Original;
  MREAM Assigned;
  Assigned.SetVolumeSequence(new MDVolumeSequence());
  Assigned = Original;
  if (Assigned.GetVolumeSequence() != nullptr) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTREAM Test;
  return Test.Run() == true ? 0 : 1;
}


// UTREAM.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
