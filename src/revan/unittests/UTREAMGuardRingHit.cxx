/*
 * UTREAMGuardRingHit.cxx
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

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MDDetector.h"
#include "MDVolumeSequence.h"
#include "MGeometryRevan.h"
#include "MGlobal.h"
#include "MREAM.h"
#include "MREAMGuardRingHit.h"
#include "MRERawEvent.h"
#include "MString.h"
#include "MVector.h"
#include "UTRevanShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the energy deposit in a guard ring MREAMGuardRingHit
class UTREAMGuardRingHit : public UTRevanBase
{
public:
  //! Default constructor
  UTREAMGuardRingHit() : UTRevanBase("UTREAMGuardRingHit") {}
  //! Default destructor
  virtual ~UTREAMGuardRingHit() {}

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
  //! Test the text for the evta file
  bool TestToEvtaString();

  //! Parse a guard ring line of an evta file and return the measurement, nullptr if it was not accepted - the event owns it
  MREAMGuardRingHit* ParseGuardRingLine(MRERawEvent& Event, const MString& Line);
  //! Return true if a copy of a measurement without volume sequence keeps the energy
  bool IsCopyWithoutVolumeSequenceCorrect();
  //! Return true if the evta text of a measurement without volume sequence is empty
  bool IsEvtaStringWithoutVolumeSequenceEmpty();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTREAMGuardRingHit::Run()
{
  bool Passed = true;

  Passed = TestDefaults() && Passed;
  Passed = TestEnergy() && Passed;
  Passed = TestCopies() && Passed;
  Passed = TestCopiesWithoutVolumeSequence() && Passed;
  Passed = TestToEvtaString() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the state after the default construction
bool UTREAMGuardRingHit::TestDefaults()
{
  bool Passed = true;

  MREAMGuardRingHit REAM;
  Passed = Evaluate("MREAMGuardRingHit()", "default", "The type is the guard ring hit", REAM.GetType(), MREAM::c_GuardRingHit) && Passed;
  Passed = EvaluateTrue("MREAMGuardRingHit()", "default", "There is no volume sequence", REAM.GetVolumeSequence() == nullptr) && Passed;
  Passed = Evaluate("MREAMGuardRingHit()", "default", "The energy is not defined", REAM.GetEnergy(), g_DoubleNotDefined) && Passed;
  Passed = Evaluate("MREAMGuardRingHit()", "default", "The energy resolution is not defined", REAM.GetEnergyResolution(), g_DoubleNotDefined) && Passed;
  Passed = Evaluate("ParseLine()", "GR line", "The measurement does not parse a line itself: 2", REAM.ParseLine("GR 1;2;3;40", 1), 2) && Passed;
  Passed = Evaluate("ToString()", "level 1", "The text is the one of the base class with the type 2", REAM.ToString(1), MString("   Basic REAM of Type 2\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the energy and its resolution
bool UTREAMGuardRingHit::TestEnergy()
{
  bool Passed = true;

  MREAMGuardRingHit REAM;
  REAM.SetEnergy(45.5);
  Passed = EvaluateNear("SetEnergy()", "45.5 keV", "The energy is returned", REAM.GetEnergy(), 45.5, 1e-12) && Passed;
  Passed = Evaluate("SetEnergy()", "45.5 keV", "The energy resolution is unchanged", REAM.GetEnergyResolution(), g_DoubleNotDefined) && Passed;
  REAM.SetEnergyResolution(5.25);
  Passed = EvaluateNear("SetEnergyResolution()", "5.25 keV", "The energy resolution is returned", REAM.GetEnergyResolution(), 5.25, 1e-12) && Passed;
  Passed = EvaluateNear("SetEnergyResolution()", "5.25 keV", "The energy is unchanged", REAM.GetEnergy(), 45.5, 1e-12) && Passed;

  // The class stores what it gets:
  REAM.SetEnergy(0.0);
  Passed = EvaluateNear("SetEnergy()", "zero", "A zero energy is stored", REAM.GetEnergy(), 0.0, 1e-15) && Passed;
  REAM.SetEnergy(-1.5);
  Passed = EvaluateNear("SetEnergy()", "negative", "A negative energy is stored unchanged", REAM.GetEnergy(), -1.5, 1e-15) && Passed;
  REAM.SetEnergyResolution(0.0);
  Passed = EvaluateNear("SetEnergyResolution()", "zero", "A zero resolution is stored", REAM.GetEnergyResolution(), 0.0, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy constructor and clone with a volume sequence
bool UTREAMGuardRingHit::TestCopies()
{
  bool Passed = true;

  MREAMGuardRingHit Original;
  MDVolumeSequence* Sequence = new MDVolumeSequence();
  Sequence->SetPositionInSensitiveVolume(MVector(1.0, 2.0, 3.0));
  Original.SetVolumeSequence(Sequence);
  Original.SetEnergy(45.5);
  Original.SetEnergyResolution(5.25);

  // Copy constructor:
  MREAMGuardRingHit Copy(Original);
  Passed = Evaluate("MREAMGuardRingHit(const MREAMGuardRingHit&)", "with sequence", "The type is copied", Copy.GetType(), MREAM::c_GuardRingHit) && Passed;
  Passed = EvaluateNear("MREAMGuardRingHit(const MREAMGuardRingHit&)", "with sequence", "The energy is copied", Copy.GetEnergy(), 45.5, 1e-12) && Passed;
  Passed = EvaluateNear("MREAMGuardRingHit(const MREAMGuardRingHit&)", "with sequence", "The energy resolution is copied", Copy.GetEnergyResolution(), 5.25, 1e-12) && Passed;
  Passed = EvaluateTrue("MREAMGuardRingHit(const MREAMGuardRingHit&)", "with sequence", "The copy has its own sequence", Copy.GetVolumeSequence() != nullptr && Copy.GetVolumeSequence() != Sequence) && Passed;
  if (Copy.GetVolumeSequence() != nullptr) {
    Passed = EvaluateVectorNear("MREAMGuardRingHit(const MREAMGuardRingHit&)", "with sequence", "The content of the sequence is copied", Copy.GetVolumeSequence()->GetPositionInSensitiveVolume(), MVector(1.0, 2.0, 3.0), 1e-15) && Passed;
  }

  // The copy is independent:
  Original.SetEnergy(100.0);
  Original.SetEnergyResolution(1.0);
  Passed = EvaluateNear("MREAMGuardRingHit(const MREAMGuardRingHit&)", "original changed", "The energy of the copy does not follow the original", Copy.GetEnergy(), 45.5, 1e-12) && Passed;
  Passed = EvaluateNear("MREAMGuardRingHit(const MREAMGuardRingHit&)", "original changed", "The energy resolution of the copy does not follow the original", Copy.GetEnergyResolution(), 5.25, 1e-12) && Passed;

  // Clone through the base class:
  MREAM* Base = &Original;
  MREAM* Clone = Base->Clone();
  MREAMGuardRingHit* TypedClone = dynamic_cast<MREAMGuardRingHit*>(Clone);
  Passed = EvaluateTrue("Clone()", "through the base class", "The clone is a guard ring hit", TypedClone != nullptr) && Passed;
  if (TypedClone != nullptr) {
    Passed = Evaluate("Clone()", "through the base class", "The type is copied", TypedClone->GetType(), MREAM::c_GuardRingHit) && Passed;
    Passed = EvaluateNear("Clone()", "through the base class", "The energy is copied", TypedClone->GetEnergy(), 100.0, 1e-12) && Passed;
    Passed = EvaluateNear("Clone()", "through the base class", "The energy resolution is copied", TypedClone->GetEnergyResolution(), 1.0, 1e-12) && Passed;
    Passed = EvaluateTrue("Clone()", "through the base class", "The clone has its own sequence", TypedClone->GetVolumeSequence() != nullptr && TypedClone->GetVolumeSequence() != Sequence) && Passed;
  }
  delete Clone;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy constructor and clone without a volume sequence
bool UTREAMGuardRingHit::TestCopiesWithoutVolumeSequence()
{
  bool Passed = true;

  // Defect REV-C-1: The copy constructor dereferences the volume sequence without a check, and a new measurement has none
  Passed = ExpectedToCrash(EvaluateTrue("MREAMGuardRingHit(const MREAMGuardRingHit&)", "without sequence", "The copy of a measurement without sequence keeps the energy and has no sequence", IsCopyWithoutVolumeSequenceCorrect())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text for the evta file
bool UTREAMGuardRingHit::TestToEvtaString()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "tracker with guard ring", "The telescope can be scanned", ScanTelescopeGeometry(Geometry, GetGuardRingLines())) && Passed;
  if (Passed == false) {
    return false;
  }

  // A guard ring hit of 40 keV in tracker layer 1 (z = 1 cm), read from an evta line with the world position:
  MRERawEvent Event(&Geometry);
  MREAMGuardRingHit* Hit = ParseGuardRingLine(Event, "GR 4.8;0.2;1.0;40");
  Passed = EvaluateTrue("ParseLine()", "GR line in the tracker", "The guard ring hit is read", Hit != nullptr) && Passed;
  if (Hit == nullptr) {
    return false;
  }
  Passed = EvaluateNear("ParseLine()", "GR line in the tracker", "The energy is the one of the line", Hit->GetEnergy(), 40.0, 1e-12) && Passed;

  // The text is written in fixed (precision 0) and scientific notation
  for (int Precision: { 0, 5 }) {
    MString Input = MString("precision ") + Precision;

    // The function reports with each call on the error stream that it has never been tested (part of defect REV-C-4)
    DisableDefaultStreams();
    MString Text = Hit->ToEvtaString(Precision, 25);
    EnableDefaultStreams();
    Passed = EvaluateTrue("ToEvtaString()", Input, "The line starts with the keyword GR", Text.BeginsWith("GR ")) && Passed;
    Passed = EvaluateTrue("ToEvtaString()", Input, "The line ends with a line break", Text.EndsWith("\n")) && Passed;
    Passed = Evaluate("ToEvtaString()", Input, "The text is one line", Text.Tokenize("\n").size(), static_cast<size_t>(1)) && Passed;

    // Round trip: the text is a line of an evta file, thus the reader has to give the same guard ring hit
    MRERawEvent Reread(&Geometry);
    MREAMGuardRingHit* RereadHit = ParseGuardRingLine(Reread, Text);
    // Defect REV-C-4: The text is "GR <detector type>;x;y;z;E" with the position in the wafer, but the reader expects "GR x;y;z;E" with the position in the world
    Passed = ExpectedToFail(EvaluateTrue("ToEvtaString()", Input, "The written line is accepted by the reader", RereadHit != nullptr)) && Passed;
    if (RereadHit != nullptr) {
      Passed = EvaluateNear("ToEvtaString()", Input, "The energy of the reread hit is the original one", RereadHit->GetEnergy(), 40.0, 1e-3) && Passed;
      Passed = EvaluateTrue("ToEvtaString()", Input, "The reread hit is in the guard ring of the tracker", RereadHit->GetVolumeSequence()->GetDetector()->GetType() == MDDetector::c_GuardRing) && Passed;
    }
  }

  // Defect REV-C-1: The volume sequence is dereferenced without a check, and a new measurement has none
  Passed = ExpectedToCrash(EvaluateTrue("ToEvtaString()", "without sequence", "A measurement without sequence has no evta text", IsEvtaStringWithoutVolumeSequenceEmpty())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse a guard ring line of an evta file and return the measurement, nullptr if it was not accepted - the event owns it
MREAMGuardRingHit* UTREAMGuardRingHit::ParseGuardRingLine(MRERawEvent& Event, const MString& Line)
{
  // The search for the detector writes to cout if there is none
  unsigned int Before = Event.GetNREAMs();
  DisableDefaultStreams();
  BeginCaptureOutput();
  int Result = Event.ParseLine(Line, 25);
  EndCaptureOutput();
  EnableDefaultStreams();
  if (Result != 0 || Event.GetNREAMs() != Before + 1) {
    return nullptr;
  }

  return dynamic_cast<MREAMGuardRingHit*>(Event.GetREAMAt(Before));
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a copy of a measurement without volume sequence keeps the energy
bool UTREAMGuardRingHit::IsCopyWithoutVolumeSequenceCorrect()
{
  MREAMGuardRingHit Original;
  Original.SetEnergy(45.5);
  MREAMGuardRingHit Copy(Original);
  if (Copy.GetVolumeSequence() != nullptr) {
    return false;
  }
  if (Copy.GetEnergy() != 45.5) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the evta text of a measurement without volume sequence is empty
bool UTREAMGuardRingHit::IsEvtaStringWithoutVolumeSequenceEmpty()
{
  MREAMGuardRingHit REAM;
  REAM.SetEnergy(45.5);

  return REAM.ToEvtaString(0, 25).IsEmpty();
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);

  // Hide the info messages of the geometry manager:
  gErrorIgnoreLevel = kWarning;

  UTREAMGuardRingHit Test;
  return Test.Run() == true ? 0 : 1;
}


// UTREAMGuardRingHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
