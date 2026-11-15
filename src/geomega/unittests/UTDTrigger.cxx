/*
 * UTDTrigger.cxx
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
#include <sstream>
using namespace std;

// MEGAlib libs:
#include "MDStrip2D.h"
#include "MDTrigger.h"
#include "MDVolume.h"
#include "MDVolumeSequence.h"
#include "MString.h"
#include "MUnitTest.h"


////////////////////////////////////////////////////////////////////////////////


//! Test subclass to reach the protected members of the base class of the triggers
class UTDTriggerAccess : public MDTrigger
{
public:
  //! Standard constructor with a name
  UTDTriggerAccess(const MString& Name) : MDTrigger(Name) {}
  //! Return the flag indicating that vetoes are ignored
  bool GetIgnoreVetoes() const { return m_IgnoreVetoes; }
  //! Return a text which differs from the base class
  virtual MString ToString() const { return "Derived text"; }
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the trigger base class MDTrigger
class UTDTrigger : public MUnitTest
{
public:
  //! Default constructor
  UTDTrigger() : MUnitTest("UTDTrigger") {}
  //! Default destructor
  virtual ~UTDTrigger() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the constants and the enumerations
  bool TestConstants();
  //! Test the name and the type
  bool TestNameAndType();
  //! Test the veto ignore flag
  bool TestIgnoreVetoes();
  //! Test the default answers of the interface
  bool TestInterfaceDefaults();
  //! Test the text output and that it is polymorphic
  bool TestText();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDTrigger::Run()
{
  bool Passed = true;

  Passed = TestConstants() && Passed;
  Passed = TestNameAndType() && Passed;
  Passed = TestIgnoreVetoes() && Passed;
  Passed = TestInterfaceDefaults() && Passed;
  Passed = TestText() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the constants and the enumerations
bool UTDTrigger::TestConstants()
{
  bool Passed = true;

  Passed = Evaluate("c_Detector", "constant", "A normal detector has the type 0", MDTrigger::c_Detector, 0) && Passed;
  Passed = Evaluate("c_GuardRing", "constant", "A guard ring has the type 1", MDTrigger::c_GuardRing, 1) && Passed;

  Passed = Evaluate("MDTriggerType", "c_Unknown", "The value is fixed", static_cast<int>(MDTriggerType::c_Unknown), 0) && Passed;
  Passed = Evaluate("MDTriggerType", "c_Basic", "The value is fixed", static_cast<int>(MDTriggerType::c_Basic), 1) && Passed;
  Passed = Evaluate("MDTriggerType", "c_Universal", "The value is fixed", static_cast<int>(MDTriggerType::c_Universal), 2) && Passed;

  Passed = Evaluate("MDTriggerMode", "c_Unknown", "The value is fixed", static_cast<int>(MDTriggerMode::c_Unknown), 0) && Passed;
  Passed = Evaluate("MDTriggerMode", "c_VetoableTrigger", "The value is fixed", static_cast<int>(MDTriggerMode::c_VetoableTrigger), 1) && Passed;
  Passed = Evaluate("MDTriggerMode", "c_NonVetoableTrigger", "The value is fixed", static_cast<int>(MDTriggerMode::c_NonVetoableTrigger), 2) && Passed;
  Passed = Evaluate("MDTriggerMode", "c_Veto", "The value is fixed", static_cast<int>(MDTriggerMode::c_Veto), 3) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the name and the type
bool UTDTrigger::TestNameAndType()
{
  bool Passed = true;

  MDTrigger Trigger("MyTrigger");
  Passed = EvaluateTrue("GetName()", "MyTrigger", "The name of the constructor is returned", Trigger.GetName() == "MyTrigger") && Passed;
  Passed = EvaluateTrue("GetType()", "base class", "The type of the base class is unknown", Trigger.GetType() == MDTriggerType::c_Unknown) && Passed;

  MDTrigger Empty("");
  Passed = EvaluateTrue("GetName()", "empty name", "An empty name is stored", Empty.GetName() == "") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the veto ignore flag
bool UTDTrigger::TestIgnoreVetoes()
{
  bool Passed = true;

  UTDTriggerAccess Trigger("T");
  Passed = EvaluateFalse("MDTrigger()", "default", "Vetoes are not ignored by default", Trigger.GetIgnoreVetoes()) && Passed;

  Trigger.IgnoreVetoes(true);
  Passed = EvaluateTrue("IgnoreVetoes()", "true", "The flag is set", Trigger.GetIgnoreVetoes()) && Passed;
  Trigger.IgnoreVetoes(true);
  Passed = EvaluateTrue("IgnoreVetoes()", "true twice", "A repeated call changes nothing", Trigger.GetIgnoreVetoes()) && Passed;
  Trigger.IgnoreVetoes(false);
  Passed = EvaluateFalse("IgnoreVetoes()", "false", "The flag is cleared again", Trigger.GetIgnoreVetoes()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default answers of the interface
bool UTDTrigger::TestInterfaceDefaults()
{
  bool Passed = true;

  MDStrip2D Detector("D");
  MDVolume Volume("V");
  MDVolumeSequence Hit;
  Hit.AddVolume(&Volume);
  Hit.SetDetectorVolume(&Volume);
  Hit.SetDetector(&Detector);

  MDTrigger Trigger("T");

  Passed = EvaluateFalse("Validate()", "base class", "The base class is not a valid trigger", Trigger.Validate()) && Passed;
  Passed = EvaluateFalse("Applies()", "base class", "The base class applies to no detector", Trigger.Applies(&Detector)) && Passed;
  Passed = EvaluateFalse("IsTriggering()", "base class", "The base class triggers on no detector", Trigger.IsTriggering(&Detector)) && Passed;
  Passed = EvaluateFalse("IsVetoing()", "base class", "The base class vetoes no detector", Trigger.IsVetoing(&Detector)) && Passed;
  Passed = EvaluateFalse("AddHit()", "base class", "The base class accepts no hit", Trigger.AddHit(Hit)) && Passed;
  Passed = EvaluateFalse("AddGuardRingHit()", "base class", "The base class accepts no guard ring hit", Trigger.AddGuardRingHit(Hit)) && Passed;

  // Nothing is ever raised, also not after a hit and a reset:
  Passed = EvaluateFalse("HasTriggered()", "after a hit", "The base class never triggers", Trigger.HasTriggered()) && Passed;
  Passed = EvaluateFalse("HasNonVetoablyTriggered()", "after a hit", "The base class never triggers non-vetoably", Trigger.HasNonVetoablyTriggered()) && Passed;
  Passed = EvaluateFalse("HasVetoed()", "after a hit", "The base class never vetoes", Trigger.HasVetoed()) && Passed;
  Trigger.Reset();
  Passed = EvaluateFalse("Reset()", "base class", "The base class has not triggered after the reset", Trigger.HasTriggered()) && Passed;
  Passed = EvaluateTrue("Reset()", "base class", "The reset does not change the name", Trigger.GetName() == "T") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output and that it is polymorphic
bool UTDTrigger::TestText()
{
  bool Passed = true;

  MDTrigger Trigger("T");
  Passed = EvaluateTrue("GetGeomega()", "base class", "The base class names the derived classes as responsible", Trigger.GetGeomega() == "GetGeomega must be defined in derived class") && Passed;
  Passed = EvaluateTrue("ToString()", "base class", "The base class names the derived classes as responsible", Trigger.ToString() == "ToString must be defined in derived class") && Passed;

  ostringstream Out;
  Out<<Trigger;
  Passed = EvaluateTrue("operator<<", "base class", "The stream output is the text of ToString", MString(Out.str()) == "ToString must be defined in derived class") && Passed;

  // The stream operator of the base class has to use the text of the derived class:
  UTDTriggerAccess Derived("D");
  const MDTrigger& Base = Derived;
  ostringstream DerivedOut;
  DerivedOut<<Base;
  Passed = EvaluateTrue("operator<<", "derived class", "The stream output uses the text of the derived class", MString(DerivedOut.str()) == "Derived text") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTDTrigger Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDTrigger.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
