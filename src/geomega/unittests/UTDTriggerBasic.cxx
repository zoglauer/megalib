/*
 * UTDTriggerBasic.cxx
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
#include <vector>
using namespace std;

// ROOT libs:
#include <TApplication.h>
#include <TError.h>
#include <TROOT.h>

// MEGAlib libs:
#include "MDCalorimeter.h"
#include "MDDetector.h"
#include "MDGeometry.h"
#include "MDStrip2D.h"
#include "MDTrigger.h"
#include "MDTriggerBasic.h"
#include "MDVolume.h"
#include "MDVolumeSequence.h"
#include "MString.h"
#include "UTGeomegaShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the basic trigger MDTriggerBasic
class UTDTriggerBasic : public UTGeomegaBase
{
public:
  //! Default constructor
  UTDTriggerBasic();
  //! Default destructor
  virtual ~UTDTriggerBasic() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return a hit in the given detector and detector volume
  MDVolumeSequence GetHit(MDDetector* Detector, MDVolume* Volume);
  //! Return the setup of a geometry with one strip detector and one scintillator
  MString GetGeometry() const;

  //! Test the constructor, the name, and the flags
  bool TestDefaultsAndFlags();
  //! Test the stored setup of the trigger
  bool TestSetup();
  //! Test the validation of the setup
  bool TestValidate();
  //! Test which detectors a trigger applies to
  bool TestApplies();
  //! Test the return values of adding hits
  bool TestAddHitReturnValues();
  //! Test the trigger decision by channel and by detector for all hit counts around the threshold
  bool TestChannelAgainstDetector();
  //! Test the trigger decision for several conditions which all have to be fulfilled
  bool TestCombinedConditions();
  //! Test hits in named detectors
  bool TestNamedDetectors();
  //! Test guard ring conditions
  bool TestGuardRings();
  //! Test the veto decision and that vetoes are ignored
  bool TestVeto();
  //! Test the decision of a trigger without criteria
  bool TestNoCriteria();
  //! Test that no state leaks between events
  bool TestReset();
  //! Test the text output
  bool TestText();
  //! Test that the description of a trigger can be read back
  bool TestGeomegaRoundTrip();
  //! Test the handling of the hit number in the setup file
  bool TestSetupFileHits();

  //! A strip detector
  MDStrip2D m_StripA;
  //! A second strip detector
  MDStrip2D m_StripB;
  //! A detector named after the first strip detector
  MDStrip2D m_NamedA;
  //! A calorimeter
  MDCalorimeter m_Calorimeter;
  //! The first volume
  MDVolume m_VolumeA;
  //! The second volume
  MDVolume m_VolumeB;
  //! The third volume
  MDVolume m_VolumeC;
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
UTDTriggerBasic::UTDTriggerBasic() :
  UTGeomegaBase("UTDTriggerBasic"), m_StripA("StripA"), m_StripB("StripB"), m_NamedA("NamedA"), m_Calorimeter("Calorimeter"),
  m_VolumeA("VolumeA"), m_VolumeB("VolumeB"), m_VolumeC("VolumeC")
{
  m_StripA.AddNamedDetector(&m_NamedA);
}


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDTriggerBasic::Run()
{
  bool Passed = true;

  Passed = TestDefaultsAndFlags() && Passed;
  Passed = TestSetup() && Passed;
  Passed = TestValidate() && Passed;
  Passed = TestApplies() && Passed;
  Passed = TestAddHitReturnValues() && Passed;
  Passed = TestChannelAgainstDetector() && Passed;
  Passed = TestCombinedConditions() && Passed;
  Passed = TestNamedDetectors() && Passed;
  Passed = TestGuardRings() && Passed;
  Passed = TestVeto() && Passed;
  Passed = TestNoCriteria() && Passed;
  Passed = TestReset() && Passed;
  Passed = TestText() && Passed;
  Passed = TestGeomegaRoundTrip() && Passed;
  Passed = TestSetupFileHits() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a hit in the given detector and detector volume
MDVolumeSequence UTDTriggerBasic::GetHit(MDDetector* Detector, MDVolume* Volume)
{
  MDVolumeSequence Hit;
  Hit.AddVolume(Volume);
  Hit.SetDetectorVolume(Volume);
  Hit.SetDetector(Detector);
  return Hit;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the setup of a geometry with one strip detector and one scintillator
MString UTDTriggerBasic::GetGeometry() const
{
  MString Setup = GetGeometryHead();
  Setup += "Volume StripVolume\n";
  Setup += "StripVolume.Material Germanium\n";
  Setup += "StripVolume.Shape BRIK 5.0 5.0 0.5\n";
  Setup += "StripVolume.Position 0.0 0.0 0.0\n";
  Setup += "StripVolume.Mother World\n";
  Setup += "Volume ScintillatorVolume\n";
  Setup += "ScintillatorVolume.Material Germanium\n";
  Setup += "ScintillatorVolume.Shape BRIK 5.0 5.0 0.5\n";
  Setup += "ScintillatorVolume.Position 0.0 0.0 10.0\n";
  Setup += "ScintillatorVolume.Mother World\n";
  Setup += "Strip2D Strips\n";
  Setup += "Strips.DetectorVolume StripVolume\n";
  Setup += "Strips.SensitiveVolume StripVolume\n";
  Setup += "Strips.StructuralPitch 0.0 0.0 0.0\n";
  Setup += "Strips.StructuralOffset 0.0 0.0 0.0\n";
  Setup += "Strips.Offset 0.0 0.0\n";
  Setup += "Strips.StripNumber 10 10\n";
  Setup += "Strips.NoiseThreshold 10\n";
  Setup += "Strips.TriggerThreshold 30\n";
  Setup += "Strips.EnergyResolution Ideal\n";
  Setup += "Strips.FailureRate 0.0\n";
  Setup += "Scintillator Scinti\n";
  Setup += "Scinti.DetectorVolume ScintillatorVolume\n";
  Setup += "Scinti.SensitiveVolume ScintillatorVolume\n";
  Setup += "Scinti.NoiseThreshold 10\n";
  Setup += "Scinti.TriggerThreshold 30\n";
  Setup += "Scinti.EnergyResolution Ideal\n";
  Setup += "Scinti.FailureRate 0.0\n";
  return Setup;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the constructor, the name, and the flags
bool UTDTriggerBasic::TestDefaultsAndFlags()
{
  bool Passed = true;

  MDTriggerBasic Trigger("MyTrigger");
  Passed = EvaluateTrue("GetName()", "MyTrigger", "The name of the constructor is returned", Trigger.GetName() == "MyTrigger") && Passed;
  Passed = EvaluateTrue("GetType()", "default", "The type is the basic trigger", Trigger.GetType() == MDTriggerType::c_Basic) && Passed;
  Passed = EvaluateFalse("IsVeto()", "default", "A new trigger is no veto", Trigger.IsVeto()) && Passed;
  Passed = EvaluateTrue("IsTriggerByDetector()", "default", "A new trigger counts by detector", Trigger.IsTriggerByDetector()) && Passed;
  Passed = EvaluateFalse("IsTriggerByChannel()", "default", "A new trigger does not count by channel", Trigger.IsTriggerByChannel()) && Passed;

  Trigger.SetVeto(true);
  Passed = EvaluateTrue("SetVeto/IsVeto", "true", "The flag is set", Trigger.IsVeto()) && Passed;
  Trigger.SetVeto(false);
  Passed = EvaluateFalse("SetVeto/IsVeto", "false", "The flag is cleared", Trigger.IsVeto()) && Passed;

  // The two count modes exclude each other:
  Trigger.SetTriggerByChannel(true);
  Passed = EvaluateTrue("SetTriggerByChannel/IsTriggerByChannel", "true", "The channel mode is active", Trigger.IsTriggerByChannel()) && Passed;
  Passed = EvaluateFalse("SetTriggerByChannel/IsTriggerByDetector", "true", "The detector mode is not active", Trigger.IsTriggerByDetector()) && Passed;
  Trigger.SetTriggerByChannel(false);
  Passed = EvaluateFalse("SetTriggerByChannel/IsTriggerByChannel", "false", "The channel mode is not active", Trigger.IsTriggerByChannel()) && Passed;
  Passed = EvaluateTrue("SetTriggerByChannel/IsTriggerByDetector", "false", "The detector mode is active", Trigger.IsTriggerByDetector()) && Passed;
  Trigger.SetTriggerByDetector(false);
  Passed = EvaluateFalse("SetTriggerByDetector/IsTriggerByDetector", "false", "The detector mode is not active", Trigger.IsTriggerByDetector()) && Passed;
  Passed = EvaluateTrue("SetTriggerByDetector/IsTriggerByChannel", "false", "The channel mode is active", Trigger.IsTriggerByChannel()) && Passed;
  Trigger.SetTriggerByDetector(true);
  Passed = EvaluateTrue("SetTriggerByDetector/IsTriggerByDetector", "true", "The detector mode is active", Trigger.IsTriggerByDetector()) && Passed;
  Passed = EvaluateFalse("SetTriggerByDetector/IsTriggerByChannel", "true", "The channel mode is not active", Trigger.IsTriggerByChannel()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the stored setup of the trigger
bool UTDTriggerBasic::TestSetup()
{
  bool Passed = true;

  MDTriggerBasic Empty("Empty");
  Passed = EvaluateSize("GetDetectorTypes()", "new trigger", "There are no detector types", Empty.GetDetectorTypes().size(), 0) && Passed;
  Passed = EvaluateSize("GetDetectors()", "new trigger", "There are no detectors", Empty.GetDetectors().size(), 0) && Passed;
  Passed = EvaluateSize("GetTriggerTypes()", "new trigger", "There are no trigger types", Empty.GetTriggerTypes().size(), 0) && Passed;

  MDTriggerBasic Types("Types");
  Types.SetDetectorType(MDDetector::c_Strip2D, 2);
  Types.SetGuardRingDetectorType(MDDetector::c_Calorimeter, 1);
  Passed = EvaluateTrue("SetDetectorType/GetDetectorTypes", "two types", "The types are stored in order", Types.GetDetectorTypes() == vector<int>({ MDDetector::c_Strip2D, MDDetector::c_Calorimeter })) && Passed;
  Passed = EvaluateTrue("SetGuardRingDetectorType/GetTriggerTypes", "two types", "The normal and the guard ring entry are distinguished", Types.GetTriggerTypes() == vector<int>({ MDTrigger::c_Detector, MDTrigger::c_GuardRing })) && Passed;
  Passed = EvaluateSize("GetDetectors()", "two types", "No detector is stored for detector types", Types.GetDetectors().size(), 0) && Passed;

  MDTriggerBasic Detectors("Detectors");
  Detectors.SetDetector(&m_StripA, 1);
  Detectors.SetGuardRingDetector(&m_StripB, 1);
  Passed = EvaluateTrue("SetDetector/GetDetectors", "two detectors", "The detectors are stored in order", Detectors.GetDetectors() == vector<MDDetector*>({ &m_StripA, &m_StripB })) && Passed;
  Passed = EvaluateTrue("SetGuardRingDetector/GetTriggerTypes", "two detectors", "The normal and the guard ring entry are distinguished", Detectors.GetTriggerTypes() == vector<int>({ MDTrigger::c_Detector, MDTrigger::c_GuardRing })) && Passed;
  Passed = EvaluateSize("GetDetectorTypes()", "two detectors", "No type is stored for detectors", Detectors.GetDetectorTypes().size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the validation of the setup
bool UTDTriggerBasic::TestValidate()
{
  bool Passed = true;

  MDTriggerBasic Empty("T");
  DisableDefaultStreams();
  bool EmptyResult = Empty.Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "no detectors", "A trigger without detectors is invalid", EmptyResult) && Passed;

  MDTriggerBasic Mixed("T");
  Mixed.SetDetectorType(MDDetector::c_Strip2D, 1);
  Mixed.SetDetector(&m_Calorimeter, 1);
  DisableDefaultStreams();
  bool MixedResult = Mixed.Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "detector type and detector", "Detector types and detectors cannot be mixed", MixedResult) && Passed;

  // The range limits of the detector types, just inside and just outside:
  MDTriggerBasic Below("T");
  Below.SetDetectorType(MDDetector::c_MinDetector - 1, 1);
  DisableDefaultStreams();
  bool BelowResult = Below.Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "type below minimum", "A type below the minimum is invalid", BelowResult) && Passed;

  MDTriggerBasic AtMinimum("T");
  AtMinimum.SetDetectorType(MDDetector::c_MinDetector, 1);
  Passed = EvaluateTrue("Validate()", "type at minimum", "The minimum type is valid", AtMinimum.Validate()) && Passed;

  MDTriggerBasic AtMaximum("T");
  AtMaximum.SetDetectorType(MDDetector::c_MaxDetector, 1);
  Passed = EvaluateTrue("Validate()", "type at maximum", "The maximum type is valid", AtMaximum.Validate()) && Passed;

  MDTriggerBasic Above("T");
  Above.SetDetectorType(MDDetector::c_MaxDetector + 1, 1);
  DisableDefaultStreams();
  bool AboveResult = Above.Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "type above maximum", "A type above the maximum is invalid", AboveResult) && Passed;

  // Duplicates:
  MDTriggerBasic TwoTypes("T");
  TwoTypes.SetDetectorType(MDDetector::c_Strip2D, 1);
  TwoTypes.SetDetectorType(MDDetector::c_Calorimeter, 1);
  Passed = EvaluateTrue("Validate()", "two different types", "Different types are valid", TwoTypes.Validate()) && Passed;

  MDTriggerBasic SameTypes("T");
  SameTypes.SetDetectorType(MDDetector::c_Strip2D, 1);
  SameTypes.SetDetectorType(MDDetector::c_Strip2D, 2);
  DisableDefaultStreams();
  bool SameTypesResult = SameTypes.Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "same type twice", "A type must not appear twice", SameTypesResult) && Passed;

  MDTriggerBasic TwoDetectors("T");
  TwoDetectors.SetDetector(&m_StripA, 1);
  TwoDetectors.SetDetector(&m_StripB, 1);
  Passed = EvaluateTrue("Validate()", "two different detectors", "Different detectors are valid", TwoDetectors.Validate()) && Passed;

  MDTriggerBasic SameDetectors("T");
  SameDetectors.SetDetector(&m_StripA, 1);
  SameDetectors.SetDetector(&m_StripA, 2);
  DisableDefaultStreams();
  bool SameDetectorsResult = SameDetectors.Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "same detector twice", "A detector must not appear twice", SameDetectorsResult) && Passed;

  // Validation replaces the named detector by the one it is named after:
  MDTriggerBasic Named("T");
  Named.SetDetector(&m_NamedA, 1);
  Passed = EvaluateTrue("Validate()", "named detector", "A named detector is valid", Named.Validate()) && Passed;
  Passed = EvaluateTrue("Validate()", "named detector", "The stored detector is the one it is named after", Named.GetDetectors() == vector<MDDetector*>({ &m_StripA })) && Passed;

  MDTriggerBasic NamedAndBase("T");
  NamedAndBase.SetDetector(&m_StripA, 1);
  NamedAndBase.SetDetector(&m_NamedA, 1);
  DisableDefaultStreams();
  bool NamedAndBaseResult = NamedAndBase.Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "detector and its named detector", "Both end up as the same detector, which must not appear twice", NamedAndBaseResult) && Passed;

  // Validation can be repeated:
  Passed = EvaluateTrue("Validate()", "second call", "A repeated validation has the same result", Named.Validate()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test which detectors a trigger applies to
bool UTDTriggerBasic::TestApplies()
{
  bool Passed = true;

  MDTriggerBasic ByType("T");
  ByType.SetDetectorType(MDDetector::c_Strip2D, 1);
  Passed = EvaluateTrue("Applies()", "matching type", "The trigger applies to all detectors of the type", ByType.Applies(&m_StripA)) && Passed;
  Passed = EvaluateTrue("Applies()", "matching type", "The trigger applies to all detectors of the type", ByType.Applies(&m_StripB)) && Passed;
  Passed = EvaluateFalse("Applies()", "other type", "The trigger does not apply to another type", ByType.Applies(&m_Calorimeter)) && Passed;
  Passed = EvaluateTrue("IsTriggering()", "matching type, no veto", "A positive trigger is triggering", ByType.IsTriggering(&m_StripA)) && Passed;
  Passed = EvaluateFalse("IsTriggering()", "other type, no veto", "A positive trigger is not triggering on another type", ByType.IsTriggering(&m_Calorimeter)) && Passed;
  Passed = EvaluateFalse("IsVetoing()", "matching type, no veto", "A positive trigger is not vetoing", ByType.IsVetoing(&m_StripA)) && Passed;
  Passed = EvaluateTrue("IncludesDetectorAsPositiveTrigger()", "matching type", "The detector is a positive part", ByType.IncludesDetectorAsPositiveTrigger(&m_StripA)) && Passed;
  Passed = EvaluateFalse("IncludesDetectorAsPositiveTrigger()", "other type", "The detector is not a part", ByType.IncludesDetectorAsPositiveTrigger(&m_Calorimeter)) && Passed;
  Passed = EvaluateTrue("IncludesDetectorAsPositiveTrigger()", "named detector of matching type", "A named detector is a positive part", ByType.IncludesDetectorAsPositiveTrigger(&m_NamedA)) && Passed;

  MDTriggerBasic ByName("T");
  ByName.SetDetector(&m_StripA, 1);
  Passed = EvaluateTrue("Applies()", "matching name", "The trigger applies to the detector", ByName.Applies(&m_StripA)) && Passed;
  Passed = EvaluateFalse("Applies()", "same type, other name", "The trigger does not apply to another detector of the same type", ByName.Applies(&m_StripB)) && Passed;
  Passed = EvaluateTrue("IsTriggering()", "matching name, no veto", "A positive trigger is triggering", ByName.IsTriggering(&m_StripA)) && Passed;
  Passed = EvaluateFalse("IsVetoing()", "matching name, no veto", "A positive trigger is not vetoing", ByName.IsVetoing(&m_StripA)) && Passed;
  Passed = EvaluateTrue("IncludesDetectorAsPositiveTrigger()", "matching name", "The detector is a positive part", ByName.IncludesDetectorAsPositiveTrigger(&m_StripA)) && Passed;
  Passed = EvaluateFalse("IncludesDetectorAsPositiveTrigger()", "other name", "The detector is not a part", ByName.IncludesDetectorAsPositiveTrigger(&m_StripB)) && Passed;
  Passed = EvaluateTrue("IncludesDetectorAsPositiveTrigger()", "detector named after a matching name", "A named detector is a positive part", ByName.IncludesDetectorAsPositiveTrigger(&m_NamedA)) && Passed;

  MDTriggerBasic Veto("T");
  Veto.SetDetector(&m_StripA, 1);
  Veto.SetVeto(true);
  Passed = EvaluateTrue("Applies()", "veto", "A veto applies to its detector", Veto.Applies(&m_StripA)) && Passed;
  Passed = EvaluateFalse("IsTriggering()", "veto", "A veto is not triggering", Veto.IsTriggering(&m_StripA)) && Passed;
  Passed = EvaluateTrue("IsVetoing()", "veto", "A veto is vetoing", Veto.IsVetoing(&m_StripA)) && Passed;
  Passed = EvaluateFalse("IsVetoing()", "veto, other detector", "A veto is not vetoing another detector", Veto.IsVetoing(&m_StripB)) && Passed;
  Passed = EvaluateFalse("IncludesDetectorAsPositiveTrigger()", "veto", "A veto has no positive part", Veto.IncludesDetectorAsPositiveTrigger(&m_StripA)) && Passed;

  // Guard rings are not counted as the detector itself:
  MDTriggerBasic GuardRingType("T");
  GuardRingType.SetGuardRingDetectorType(MDDetector::c_Strip2D, 1);
  Passed = EvaluateFalse("Applies()", "guard ring type", "A guard ring condition does not apply to the detector", GuardRingType.Applies(&m_StripA)) && Passed;
  Passed = EvaluateFalse("IncludesDetectorAsPositiveTrigger()", "guard ring type", "A guard ring is no positive part", GuardRingType.IncludesDetectorAsPositiveTrigger(&m_StripA)) && Passed;
  MDTriggerBasic GuardRingName("T");
  GuardRingName.SetGuardRingDetector(&m_StripA, 1);
  Passed = EvaluateFalse("Applies()", "guard ring detector", "A guard ring condition does not apply to the detector", GuardRingName.Applies(&m_StripA)) && Passed;
  Passed = EvaluateFalse("IncludesDetectorAsPositiveTrigger()", "guard ring detector", "A guard ring is no positive part", GuardRingName.IncludesDetectorAsPositiveTrigger(&m_StripA)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the return values of adding hits
bool UTDTriggerBasic::TestAddHitReturnValues()
{
  bool Passed = true;

  MDVolumeSequence HitA = GetHit(&m_StripA, &m_VolumeA);
  MDVolumeSequence HitB = GetHit(&m_StripB, &m_VolumeB);
  MDVolumeSequence HitCalorimeter = GetHit(&m_Calorimeter, &m_VolumeC);

  const vector<bool> Modes = { true, false };
  for (bool ByChannel: Modes) {
    MString Mode = ByChannel == true ? "by channel" : "by detector";

    MDTriggerBasic ByType("T");
    ByType.SetTriggerByChannel(ByChannel);
    ByType.SetDetectorType(MDDetector::c_Strip2D, 1);
    Passed = EvaluateTrue("AddHit()", Mode + ", matching type", "A hit of the type is added", ByType.AddHit(HitA)) && Passed;
    Passed = EvaluateTrue("AddHit()", Mode + ", matching type, second", "A second hit is added", ByType.AddHit(HitA)) && Passed;
    Passed = EvaluateFalse("AddHit()", Mode + ", other type", "A hit of another type is not added", ByType.AddHit(HitCalorimeter)) && Passed;
    Passed = EvaluateFalse("AddGuardRingHit()", Mode + ", no guard ring condition", "A guard ring hit is not added to a normal condition", ByType.AddGuardRingHit(HitA)) && Passed;

    MDTriggerBasic ByName("T");
    ByName.SetTriggerByChannel(ByChannel);
    ByName.SetDetector(&m_StripA, 1);
    Passed = EvaluateTrue("AddHit()", Mode + ", matching name", "A hit in the detector is added", ByName.AddHit(HitA)) && Passed;
    Passed = EvaluateFalse("AddHit()", Mode + ", same type, other name", "A hit in another detector of the same type is not added", ByName.AddHit(HitB)) && Passed;
    Passed = EvaluateFalse("AddGuardRingHit()", Mode + ", no guard ring condition", "A guard ring hit is not added to a normal condition", ByName.AddGuardRingHit(HitA)) && Passed;

    MDTriggerBasic GuardType("T");
    GuardType.SetTriggerByChannel(ByChannel);
    GuardType.SetGuardRingDetectorType(MDDetector::c_Strip2D, 1);
    Passed = EvaluateTrue("AddGuardRingHit()", Mode + ", matching type", "A guard ring hit of the type is added", GuardType.AddGuardRingHit(HitA)) && Passed;
    Passed = EvaluateFalse("AddGuardRingHit()", Mode + ", other type", "A guard ring hit of another type is not added", GuardType.AddGuardRingHit(HitCalorimeter)) && Passed;
    Passed = EvaluateFalse("AddHit()", Mode + ", guard ring condition", "A normal hit is not added to a guard ring condition", GuardType.AddHit(HitA)) && Passed;

    MDTriggerBasic GuardName("T");
    GuardName.SetTriggerByChannel(ByChannel);
    GuardName.SetGuardRingDetector(&m_StripA, 1);
    Passed = EvaluateTrue("AddGuardRingHit()", Mode + ", matching name", "A guard ring hit of the detector is added", GuardName.AddGuardRingHit(HitA)) && Passed;
    Passed = EvaluateFalse("AddGuardRingHit()", Mode + ", other name", "A guard ring hit of another detector is not added", GuardName.AddGuardRingHit(HitB)) && Passed;
    Passed = EvaluateFalse("AddHit()", Mode + ", guard ring condition", "A normal hit is not added to a guard ring condition", GuardName.AddHit(HitA)) && Passed;

    MDTriggerBasic NoCondition("T");
    NoCondition.SetTriggerByChannel(ByChannel);
    Passed = EvaluateFalse("AddHit()", Mode + ", no condition", "A hit is not added to a trigger without conditions", NoCondition.AddHit(HitA)) && Passed;
    Passed = EvaluateFalse("AddGuardRingHit()", Mode + ", no condition", "A guard ring hit is not added to a trigger without conditions", NoCondition.AddGuardRingHit(HitA)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the trigger decision by channel and by detector for all hit counts around the threshold
bool UTDTriggerBasic::TestChannelAgainstDetector()
{
  bool Passed = true;

  vector<MDVolume*> Volumes = { &m_VolumeA, &m_VolumeB, &m_VolumeC };

  // The numbers of hits in the three detector volumes:
  for (unsigned int Required = 1; Required <= 3; ++Required) {
    for (unsigned int HitsA = 0; HitsA <= 4; ++HitsA) {
      for (unsigned int HitsB = 0; HitsB <= 2; ++HitsB) {
        for (unsigned int HitsC = 0; HitsC <= 1; ++HitsC) {
          vector<unsigned int> Hits = { HitsA, HitsB, HitsC };

          // Channels count all hits, detectors count the volumes with at least one hit:
          unsigned int Channels = HitsA + HitsB + HitsC;
          unsigned int Detectors = 0;
          for (unsigned int H: Hits) {
            if (H > 0) {
              ++Detectors;
            }
          }

          const vector<bool> Modes = { true, false };
          for (bool ByChannel: Modes) {
            const vector<bool> UseType = { true, false };
            for (bool ByType: UseType) {
              MDTriggerBasic Trigger("T");
              Trigger.SetTriggerByChannel(ByChannel);
              if (ByType == true) {
                Trigger.SetDetectorType(MDDetector::c_Strip2D, Required);
              } else {
                Trigger.SetDetector(&m_StripA, Required);
              }

              for (unsigned int v = 0; v < Volumes.size(); ++v) {
                MDVolumeSequence Hit = GetHit(&m_StripA, Volumes[v]);
                for (unsigned int h = 0; h < Hits[v]; ++h) {
                  Trigger.AddHit(Hit);
                }
              }

              bool Expected = false;
              if (ByChannel == true && Channels >= Required) {
                Expected = true;
              }
              if (ByChannel == false && Detectors >= Required) {
                Expected = true;
              }

              MString Input = MString(ByChannel == true ? "by channel" : "by detector") + (ByType == true ? ", by type" : ", by name") +
                ", required " + Required + ", hits " + HitsA + " " + HitsB + " " + HitsC;
              Passed = Evaluate("HasTriggered()", Input, "The decision follows the count mode", Trigger.HasTriggered(), Expected) && Passed;
              Passed = EvaluateFalse("HasVetoed()", Input, "A positive trigger never vetoes", Trigger.HasVetoed()) && Passed;
              Passed = EvaluateFalse("HasNonVetoablyTriggered()", Input, "A basic trigger is never non-vetoable", Trigger.HasNonVetoablyTriggered()) && Passed;
            }
          }
        }
      }
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the trigger decision for several conditions which all have to be fulfilled
bool UTDTriggerBasic::TestCombinedConditions()
{
  bool Passed = true;

  MDVolumeSequence HitStrip = GetHit(&m_StripA, &m_VolumeA);
  MDVolumeSequence HitCalorimeter = GetHit(&m_Calorimeter, &m_VolumeB);

  // Two strip hits and one calorimeter hit in all combinations, by type and by name:
  const vector<bool> UseType = { true, false };
  for (bool ByType: UseType) {
    for (unsigned int NStrip = 0; NStrip <= 3; ++NStrip) {
      for (unsigned int NCalorimeter = 0; NCalorimeter <= 2; ++NCalorimeter) {
        MDTriggerBasic Trigger("T");
        Trigger.SetTriggerByChannel(true);
        if (ByType == true) {
          Trigger.SetDetectorType(MDDetector::c_Strip2D, 2);
          Trigger.SetDetectorType(MDDetector::c_Calorimeter, 1);
        } else {
          Trigger.SetDetector(&m_StripA, 2);
          Trigger.SetDetector(&m_Calorimeter, 1);
        }
        for (unsigned int h = 0; h < NStrip; ++h) {
          Trigger.AddHit(HitStrip);
        }
        for (unsigned int h = 0; h < NCalorimeter; ++h) {
          Trigger.AddHit(HitCalorimeter);
        }

        bool Expected = false;
        if (NStrip >= 2 && NCalorimeter >= 1) {
          Expected = true;
        }
        MString Input = MString(ByType == true ? "by type" : "by name") + ", strip hits " + NStrip + ", calorimeter hits " + NCalorimeter;
        Passed = Evaluate("HasTriggered()", Input, "All conditions have to be fulfilled", Trigger.HasTriggered(), Expected) && Passed;
      }
    }
  }

  // Hits are only counted for their own condition, independent of the order:
  MDTriggerBasic Order("T");
  Order.SetTriggerByChannel(true);
  Order.SetDetectorType(MDDetector::c_Calorimeter, 1);
  Order.SetDetectorType(MDDetector::c_Strip2D, 2);
  Order.AddHit(HitStrip);
  Order.AddHit(HitCalorimeter);
  Passed = EvaluateFalse("HasTriggered()", "condition order reversed, 1 strip hit", "One strip hit is not enough", Order.HasTriggered()) && Passed;
  Order.AddHit(HitStrip);
  Passed = EvaluateTrue("HasTriggered()", "condition order reversed, 2 strip hits", "Now all conditions are fulfilled", Order.HasTriggered()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test hits in named detectors
bool UTDTriggerBasic::TestNamedDetectors()
{
  bool Passed = true;

  MDVolumeSequence HitNamed = GetHit(&m_NamedA, &m_VolumeA);
  Passed = EvaluateTrue("GetHit()", "named detector", "The volume sequence returns the named detector", HitNamed.GetDetector() == &m_NamedA) && Passed;

  const vector<bool> Modes = { true, false };
  for (bool ByChannel: Modes) {
    MString Mode = ByChannel == true ? "by channel" : "by detector";

    // A hit in a named detector counts as hit in the detector it is named after:
    MDTriggerBasic ByName("T");
    ByName.SetTriggerByChannel(ByChannel);
    ByName.SetDetector(&m_StripA, 1);
    Passed = EvaluateTrue("AddHit()", Mode + ", condition on base detector", "A hit in the named detector is added", ByName.AddHit(HitNamed)) && Passed;
    Passed = EvaluateTrue("HasTriggered()", Mode + ", condition on base detector", "A hit in the named detector raises the trigger", ByName.HasTriggered()) && Passed;

    MDTriggerBasic ByType("T");
    ByType.SetTriggerByChannel(ByChannel);
    ByType.SetDetectorType(MDDetector::c_Strip2D, 1);
    Passed = EvaluateTrue("AddHit()", Mode + ", condition on type", "A hit in the named detector is added", ByType.AddHit(HitNamed)) && Passed;
    Passed = EvaluateTrue("HasTriggered()", Mode + ", condition on type", "A hit in the named detector raises the trigger", ByType.HasTriggered()) && Passed;

    // After the validation a condition on the named detector is a condition on the detector it is named after:
    MDTriggerBasic OnNamed("T");
    OnNamed.SetTriggerByChannel(ByChannel);
    OnNamed.SetDetector(&m_NamedA, 1);
    OnNamed.Validate();
    Passed = EvaluateTrue("AddHit()", Mode + ", condition on named detector", "A hit in the named detector is added", OnNamed.AddHit(HitNamed)) && Passed;
    Passed = EvaluateTrue("HasTriggered()", Mode + ", condition on named detector", "A hit in the named detector raises the trigger", OnNamed.HasTriggered()) && Passed;

    // Guard rings:
    MDTriggerBasic GuardRing("T");
    GuardRing.SetTriggerByChannel(ByChannel);
    GuardRing.SetGuardRingDetector(&m_StripA, 1);
    Passed = EvaluateTrue("AddGuardRingHit()", Mode + ", guard ring condition on base detector", "A guard ring hit in the named detector is added", GuardRing.AddGuardRingHit(HitNamed)) && Passed;
    Passed = EvaluateTrue("HasTriggered()", Mode + ", guard ring condition on base detector", "The guard ring hit raises the trigger", GuardRing.HasTriggered()) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test guard ring conditions
bool UTDTriggerBasic::TestGuardRings()
{
  bool Passed = true;

  MDVolumeSequence HitStrip = GetHit(&m_StripA, &m_VolumeA);
  MDVolumeSequence HitCalorimeter = GetHit(&m_Calorimeter, &m_VolumeB);

  // A positive condition on a guard ring: one strip hit and two hits in the calorimeter guard ring in all combinations:
  const vector<bool> Modes = { true, false };
  for (bool ByChannel: Modes) {
    for (unsigned int NStrip = 0; NStrip <= 2; ++NStrip) {
      for (unsigned int NGuardRing = 0; NGuardRing <= 3; ++NGuardRing) {
        MDTriggerBasic Trigger("T");
        Trigger.SetTriggerByChannel(ByChannel);
        Trigger.SetDetectorType(MDDetector::c_Strip2D, 1);
        Trigger.SetGuardRingDetectorType(MDDetector::c_Calorimeter, 2);
        for (unsigned int h = 0; h < NStrip; ++h) {
          Trigger.AddHit(HitStrip);
        }
        for (unsigned int h = 0; h < NGuardRing; ++h) {
          Trigger.AddGuardRingHit(HitCalorimeter);
        }

        // By detector all guard ring hits are in one volume, thus they count as one and the condition cannot be fulfilled:
        bool Expected = false;
        if (ByChannel == true && NStrip >= 1 && NGuardRing >= 2) {
          Expected = true;
        }
        MString Input = MString(ByChannel == true ? "by channel" : "by detector") + ", strip hits " + NStrip + ", guard ring hits " + NGuardRing;
        Passed = Evaluate("HasTriggered()", Input, "Normal and guard ring conditions are counted separately", Trigger.HasTriggered(), Expected) && Passed;
      }
    }
  }

  // Normal hits do not count for a guard ring condition, and the reverse:
  MDTriggerBasic NormalOnly("T");
  NormalOnly.SetTriggerByChannel(true);
  NormalOnly.SetGuardRingDetectorType(MDDetector::c_Strip2D, 1);
  NormalOnly.AddHit(HitStrip);
  Passed = EvaluateFalse("HasTriggered()", "normal hit, guard ring condition", "A normal hit does not fulfill a guard ring condition", NormalOnly.HasTriggered()) && Passed;

  MDTriggerBasic GuardRingOnly("T");
  GuardRingOnly.SetTriggerByChannel(true);
  GuardRingOnly.SetDetectorType(MDDetector::c_Strip2D, 1);
  GuardRingOnly.AddGuardRingHit(HitStrip);
  Passed = EvaluateFalse("HasTriggered()", "guard ring hit, normal condition", "A guard ring hit does not fulfill a normal condition", GuardRingOnly.HasTriggered()) && Passed;

  // A guard ring veto:
  MDTriggerBasic Veto("T");
  Veto.SetVeto(true);
  Veto.SetTriggerByChannel(true);
  Veto.SetGuardRingDetector(&m_StripA, 1);
  Passed = EvaluateFalse("HasVetoed()", "guard ring veto, no hit", "Without a hit there is no veto", Veto.HasVetoed()) && Passed;
  Veto.AddHit(HitStrip);
  Passed = EvaluateFalse("HasVetoed()", "guard ring veto, normal hit", "A normal hit does not raise a guard ring veto", Veto.HasVetoed()) && Passed;
  Veto.AddGuardRingHit(HitStrip);
  Passed = EvaluateTrue("HasVetoed()", "guard ring veto, guard ring hit", "A guard ring hit raises the guard ring veto", Veto.HasVetoed()) && Passed;
  Passed = EvaluateFalse("HasTriggered()", "guard ring veto, guard ring hit", "A veto does not trigger", Veto.HasTriggered()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the veto decision and that vetoes are ignored
bool UTDTriggerBasic::TestVeto()
{
  bool Passed = true;

  MDVolumeSequence HitA = GetHit(&m_StripA, &m_VolumeA);
  MDVolumeSequence HitB = GetHit(&m_StripA, &m_VolumeB);

  // The veto threshold in both count modes, just below and at the required hits:
  MDTriggerBasic ByChannel("T");
  ByChannel.SetVeto(true);
  ByChannel.SetTriggerByChannel(true);
  ByChannel.SetDetector(&m_StripA, 2);
  Passed = EvaluateFalse("HasVetoed()", "by channel, 0 of 2 hits", "No veto without hits", ByChannel.HasVetoed()) && Passed;
  ByChannel.AddHit(HitA);
  Passed = EvaluateFalse("HasVetoed()", "by channel, 1 of 2 hits", "No veto below the required hits", ByChannel.HasVetoed()) && Passed;
  ByChannel.AddHit(HitA);
  Passed = EvaluateTrue("HasVetoed()", "by channel, 2 of 2 hits", "The veto is raised at the required hits", ByChannel.HasVetoed()) && Passed;
  ByChannel.AddHit(HitA);
  Passed = EvaluateTrue("HasVetoed()", "by channel, 3 of 2 hits", "The veto stays raised above the required hits", ByChannel.HasVetoed()) && Passed;
  Passed = EvaluateFalse("HasTriggered()", "by channel, veto raised", "A veto trigger never raises a trigger", ByChannel.HasTriggered()) && Passed;

  MDTriggerBasic ByDetector("T");
  ByDetector.SetVeto(true);
  ByDetector.SetTriggerByDetector(true);
  ByDetector.SetDetectorType(MDDetector::c_Strip2D, 2);
  ByDetector.AddHit(HitA);
  ByDetector.AddHit(HitA);
  Passed = EvaluateFalse("HasVetoed()", "by detector, 2 hits in 1 detector", "Hits in the same detector count once", ByDetector.HasVetoed()) && Passed;
  ByDetector.AddHit(HitB);
  Passed = EvaluateTrue("HasVetoed()", "by detector, hits in 2 detectors", "The veto is raised by the second detector", ByDetector.HasVetoed()) && Passed;

  // Ignoring the veto does not change the hits, only the answer:
  ByChannel.IgnoreVetoes(true);
  Passed = EvaluateFalse("IgnoreVetoes()", "true, veto raised", "An ignored veto is not raised", ByChannel.HasVetoed()) && Passed;
  Passed = EvaluateFalse("IgnoreVetoes()", "true, veto raised", "An ignored veto does not become a trigger", ByChannel.HasTriggered()) && Passed;
  ByChannel.IgnoreVetoes(false);
  Passed = EvaluateTrue("IgnoreVetoes()", "false again", "The veto is back after the flag has been cleared", ByChannel.HasVetoed()) && Passed;

  // Ignoring vetoes does not change a positive trigger:
  MDTriggerBasic Positive("T");
  Positive.SetTriggerByChannel(true);
  Positive.SetDetector(&m_StripA, 1);
  Positive.IgnoreVetoes(true);
  Positive.AddHit(HitA);
  Passed = EvaluateTrue("IgnoreVetoes()", "positive trigger", "A positive trigger is not changed", Positive.HasTriggered()) && Passed;

  // Switching the role of a trigger with hits:
  Positive.IgnoreVetoes(false);
  Positive.SetVeto(true);
  Passed = EvaluateFalse("SetVeto()", "hits exist, now veto", "It does not trigger any more", Positive.HasTriggered()) && Passed;
  Passed = EvaluateTrue("SetVeto()", "hits exist, now veto", "The existing hits raise the veto", Positive.HasVetoed()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the decision of a trigger without criteria
bool UTDTriggerBasic::TestNoCriteria()
{
  bool Passed = true;

  MDTriggerBasic Positive("T");
  Passed = EvaluateFalse("HasTriggered()", "no criteria", "A trigger without criteria does not trigger", Positive.HasTriggered()) && Passed;
  Passed = EvaluateFalse("HasVetoed()", "no criteria", "A trigger without criteria does not veto", Positive.HasVetoed()) && Passed;

  MDTriggerBasic Veto("T");
  Veto.SetVeto(true);
  Passed = EvaluateFalse("HasTriggered()", "no criteria, veto", "A veto without criteria does not trigger", Veto.HasTriggered()) && Passed;
  Passed = EvaluateFalse("HasVetoed()", "no criteria, veto", "A veto without criteria does not veto", Veto.HasVetoed()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that no state leaks between events
bool UTDTriggerBasic::TestReset()
{
  bool Passed = true;

  MDVolumeSequence HitA = GetHit(&m_StripA, &m_VolumeA);
  MDVolumeSequence HitB = GetHit(&m_StripA, &m_VolumeB);

  // Reset on a new trigger is possible:
  MDTriggerBasic New("T");
  New.Reset();
  Passed = EvaluateFalse("Reset()", "new trigger without conditions", "A reset of a new trigger changes nothing", New.HasTriggered()) && Passed;

  const vector<bool> Modes = { true, false };
  for (bool ByChannel: Modes) {
    MString Mode = ByChannel == true ? "by channel" : "by detector";

    // Hits in two different detector volumes:
    MDTriggerBasic Trigger("T");
    Trigger.SetTriggerByChannel(ByChannel);
    Trigger.SetDetectorType(MDDetector::c_Strip2D, 2);

    // Event 1 raises the trigger:
    Trigger.AddHit(HitA);
    Trigger.AddHit(HitB);
    Passed = EvaluateTrue("HasTriggered()", Mode + ", event 1", "Event 1 raises the trigger", Trigger.HasTriggered()) && Passed;
    Passed = EvaluateTrue("HasTriggered()", Mode + ", event 1, repeated read", "Reading the decision does not change it", Trigger.HasTriggered()) && Passed;

    // Event 2 has too few hits and does not see those of event 1:
    Trigger.Reset();
    Passed = EvaluateFalse("Reset()", Mode + ", no hits", "The trigger is gone after the reset", Trigger.HasTriggered()) && Passed;
    Trigger.AddHit(HitA);
    Passed = EvaluateFalse("Reset()", Mode + ", event 2 with 1 of 2 hits", "One hit is not enough after the reset", Trigger.HasTriggered()) && Passed;

    // Event 3 raises the trigger again - by detector the stored detectors must be gone, too:
    Trigger.Reset();
    Trigger.AddHit(HitA);
    Trigger.AddHit(HitB);
    Passed = EvaluateTrue("Reset()", Mode + ", event 3", "The setup is still intact after two resets", Trigger.HasTriggered()) && Passed;

    // A repeated reset:
    Trigger.Reset();
    Trigger.Reset();
    Passed = EvaluateFalse("Reset()", Mode + ", twice", "A repeated reset is possible", Trigger.HasTriggered()) && Passed;

    // The setup is not changed:
    Passed = EvaluateTrue("Reset()", Mode, "The detector types stay", Trigger.GetDetectorTypes() == vector<int>({ MDDetector::c_Strip2D })) && Passed;
    Passed = Evaluate("Reset()", Mode, "The count mode stays", Trigger.IsTriggerByChannel(), ByChannel) && Passed;
    Passed = EvaluateFalse("Reset()", Mode, "The veto flag stays", Trigger.IsVeto()) && Passed;

    // The same for a veto:
    MDTriggerBasic Veto("T");
    Veto.SetVeto(true);
    Veto.SetTriggerByChannel(ByChannel);
    Veto.SetDetector(&m_StripA, 1);
    Veto.AddHit(HitA);
    Passed = EvaluateTrue("HasVetoed()", Mode + ", event 1", "Event 1 raises the veto", Veto.HasVetoed()) && Passed;
    Veto.Reset();
    Passed = EvaluateFalse("Reset()", Mode + ", event 2", "The veto is gone after the reset", Veto.HasVetoed()) && Passed;
    Passed = EvaluateTrue("Reset()", Mode + ", event 2", "The veto flag stays", Veto.IsVeto()) && Passed;

    // The same for a guard ring:
    MDTriggerBasic GuardRing("T");
    GuardRing.SetTriggerByChannel(ByChannel);
    GuardRing.SetGuardRingDetector(&m_StripA, 1);
    GuardRing.AddGuardRingHit(HitA);
    Passed = EvaluateTrue("HasTriggered()", Mode + ", guard ring, event 1", "Event 1 raises the trigger", GuardRing.HasTriggered()) && Passed;
    GuardRing.Reset();
    Passed = EvaluateFalse("Reset()", Mode + ", guard ring, event 2", "The guard ring hit is gone after the reset", GuardRing.HasTriggered()) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output
bool UTDTriggerBasic::TestText()
{
  bool Passed = true;

  // Type names and hits are written as in the setup file - by default the count mode is by detector:
  MDTriggerBasic Types("T");
  Types.SetDetectorType(MDDetector::c_Strip2D, 2);
  Types.SetGuardRingDetectorType(MDDetector::c_Calorimeter, 1);
  MString ExpectedTypes = "Trigger T\nT.Veto false\nT.TriggerByDetector 1\nT.DetectorType Strip2D 2\nT.GuardRingDetectorType Calorimeter 1\n";
  Passed = EvaluateTrue("GetGeomega()", "types", "The text has all setup lines", Types.GetGeomega() == ExpectedTypes) && Passed;

  MDTriggerBasic Detectors("T");
  Detectors.SetVeto(true);
  Detectors.SetTriggerByChannel(true);
  Detectors.SetDetector(&m_StripA, 3);
  Detectors.SetGuardRingDetector(&m_StripB, 4);
  MString ExpectedDetectors = "Trigger T\nT.Veto true\nT.TriggerByChannel 1\nT.Detector StripA 3\nT.GuardRingDetector StripB 4\n";
  Passed = EvaluateTrue("GetGeomega()", "detectors", "The text has all setup lines", Detectors.GetGeomega() == ExpectedDetectors) && Passed;

  MString ExpectedString = "Trigger (Basic): T\n  Veto: yes\n  Detector names: StripA 3\n  Detector names: StripB 4 (guard ring!)\n";
  Passed = EvaluateTrue("ToString()", "detectors", "The text has the name, veto flag, and detectors", Detectors.ToString() == ExpectedString) && Passed;

  MString ExpectedTypeString = MString("Trigger (Basic): T\n  Veto: no\n  Detector types: ") + MDDetector::c_Strip2D + " 2\n  Detector types: " + MDDetector::c_Calorimeter + " 1 (guard ring!)\n";
  Passed = EvaluateTrue("ToString()", "types", "The text has the name, veto flag, and types", Types.ToString() == ExpectedTypeString) && Passed;

  ostringstream Out;
  Out<<Detectors;
  Passed = EvaluateTrue("operator<<", "detectors", "The stream output is the text of ToString", MString(Out.str()) == ExpectedString) && Passed;

  // Via the base class:
  const MDTrigger& Base = Detectors;
  ostringstream BaseOut;
  BaseOut<<Base;
  Passed = EvaluateTrue("operator<<", "via base class", "The stream output of the base class uses the same text", MString(BaseOut.str()) == ExpectedString) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the description of a trigger can be read back
bool UTDTriggerBasic::TestGeomegaRoundTrip()
{
  bool Passed = true;

  // Each of the setups is written, read back, and has to return the same description:
  vector<MDTriggerBasic*> Triggers;

  MDStrip2D Strips("Strips");
  MDStrip2D Scinti("Scinti");

  MDTriggerBasic ByType("RoundTrip");
  ByType.SetTriggerByChannel(true);
  ByType.SetDetectorType(MDDetector::c_Strip2D, 2);
  ByType.SetDetectorType(MDDetector::c_Scintillator, 1);
  Triggers.push_back(&ByType);

  MDTriggerBasic ByName("RoundTrip");
  ByName.SetDetector(&Strips, 3);
  ByName.SetDetector(&Scinti, 1);
  Triggers.push_back(&ByName);

  MDTriggerBasic Veto("RoundTrip");
  Veto.SetVeto(true);
  Veto.SetTriggerByChannel(true);
  Veto.SetDetector(&Scinti, 1);
  Veto.SetGuardRingDetector(&Strips, 1);
  Triggers.push_back(&Veto);

  for (unsigned int t = 0; t < Triggers.size(); ++t) {
    MString Input = MString("Setup ") + t;

    MString Setup = GetGeometry();
    Setup += Triggers[t]->GetGeomega();
    Setup += "Scinti.NoiseThresholdEqualsTriggerThreshold true\n";

    MDGeometry Geometry;
    bool Scanned = ScanGeometry(Geometry, Setup);
    Passed = EvaluateTrue("GetGeomega()", Input, "The description can be read back", Scanned) && Passed;
    if (Scanned == true) {
      MDTrigger* Read = Geometry.GetTrigger("RoundTrip");
      Passed = EvaluateTrue("GetGeomega()", Input, "The trigger exists after reading", Read != nullptr) && Passed;
      if (Read != nullptr) {
        Passed = EvaluateTrue("GetGeomega()", Input, "The read trigger has the same description", Read->GetGeomega() == Triggers[t]->GetGeomega()) && Passed;
      }
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the handling of the hit number in the setup file
bool UTDTriggerBasic::TestSetupFileHits()
{
  bool Passed = true;

  // A valid file as reference:
  MString Valid = GetGeometry() + "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Strips 1\n";
  MDGeometry ValidGeometry;
  Passed = EvaluateTrue("ScanSetupFile()", "1 hit", "A positive number of hits is accepted", ScanGeometry(ValidGeometry, Valid)) && Passed;

  // A negative number of hits turns into about 4 billion hits, thus into a trigger which can never fire:
  MString Negative = GetGeometry() + "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Strips -1\n";
  MDGeometry NegativeGeometry;
  // Defect GEO-E-11: A negative number of hits is accepted and silently becomes a trigger which never fires
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "-1 hits", "A negative number of hits is rejected", ScanGeometry(NegativeGeometry, Negative))) && Passed;

  MString NegativeType = GetGeometry() + "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.DetectorType Strip2D -1\n";
  MDGeometry NegativeTypeGeometry;
  // Defect GEO-E-11: A negative number of hits is accepted and silently becomes a trigger which never fires
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "-1 hits for a type", "A negative number of hits is rejected", ScanGeometry(NegativeTypeGeometry, NegativeType))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);

  // Hide the info messages of the geometry manager:
  gErrorIgnoreLevel = kWarning;

  UTDTriggerBasic Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDTriggerBasic.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
