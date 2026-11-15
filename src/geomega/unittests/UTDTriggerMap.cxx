/*
 * UTDTriggerMap.cxx
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
#include <vector>

// POSIX libs:
// Process control has no standard C++ equivalent
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// ROOT libs:
#include <TApplication.h>
#include <TError.h>
#include <TROOT.h>

// MEGAlib libs:
#include "MDGeometry.h"
#include "MDGuardRing.h"
#include "MDStrip2D.h"
#include "MDTrigger.h"
#include "MDTriggerMap.h"
#include "MDTriggerUnit.h"
#include "MDVolume.h"
#include "MDVolumeSequence.h"
#include "MFile.h"
#include "MString.h"
#include "UTGeomegaShared.h"


////////////////////////////////////////////////////////////////////////////////


//! A single rule of a trigger map as used by the independent decision in the test
class UTDTriggerMapRule
{
public:
  //! The mode of the rule
  MDTriggerMode m_Mode;
  //! The number of hits per detector
  vector<unsigned int> m_Hits;
  //! The flag per detector indicating that more hits are allowed
  vector<bool> m_AllowMoreHits;

  //! Return true if this rule is fulfilled by the hits per detector
  bool IsFulfilled(const vector<unsigned int>& Counts) const
  {
    for (unsigned int d = 0; d < Counts.size(); ++d) {
      if (Counts[d] < m_Hits[d]) {
        return false;
      }
      if (m_AllowMoreHits[d] == false && Counts[d] > m_Hits[d]) {
        return false;
      }
    }
    return true;
  }
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the trigger map MDTriggerMap
class UTDTriggerMap : public UTGeomegaBase
{
public:
  //! Default constructor
  UTDTriggerMap();
  //! Default destructor
  virtual ~UTDTriggerMap();

  //! Run all tests
  virtual bool Run();

private:
  //! Return a hit in the given detector
  MDVolumeSequence GetHit(MDDetector* Detector);
  //! Add the number of hits to the map
  void AddHits(MDTriggerMap& Map, MDDetector* Detector, unsigned int Hits);
  //! Write a trigger map into a private file and read it, silently
  bool ReadText(MDTriggerMap& Map, const MString& Content);
  //! Return the map file of the GRIPS example with the full path
  MString GetExampleFile() const;

  //! Test the constructor and the detector list
  bool TestConstruction();
  //! Test the long version of setting a trigger
  bool TestSetTriggerLong();
  //! Test the short version of setting a trigger
  bool TestSetTriggerShort();
  //! Test which detectors are triggering and vetoing
  bool TestAppliesTriggeringVetoing();
  //! Test the return values of adding hits
  bool TestAddHit();
  //! Test adding guard ring hits
  bool TestAddGuardRingHit();
  //! Test hits in named detectors
  bool TestNamedDetectors();
  //! Test exact hit numbers against minimum hit numbers, just inside and outside
  bool TestHitThresholds();
  //! Test the decision of all modes against an independent decision for all hit combinations
  bool TestDecisions();
  //! Test the priority of vetoes and non-vetoable triggers and ignoring vetoes
  bool TestVetoPriority();
  //! Test that no state leaks between events
  bool TestReset();
  //! Test reading valid files
  bool TestReadValid();
  //! Test reading malformed files
  bool TestReadMalformed();
  //! Test reusing a map for a second read or a new detector list
  bool TestReuse();
  //! Test writing and reading back
  bool TestWriteRead();
  //! Test the validation
  bool TestValidate();
  //! Test the text output
  bool TestText();
  //! Test the trigger map example in the resources
  bool TestExampleFile();
  //! Test the geometry of the trigger map example
  bool TestExampleGeometry();

  //! The first detector
  MDStrip2D m_DetA;
  //! The second detector
  MDStrip2D m_DetB;
  //! A detector named after the first one
  MDStrip2D m_NamedA;
  //! A volume for all hits
  MDVolume m_Volume;
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
UTDTriggerMap::UTDTriggerMap() :
  UTGeomegaBase("UTDTriggerMap"), m_DetA("DetA"), m_DetB("DetB"), m_NamedA("NamedA"), m_Volume("Volume")
{
  m_DetA.AddNamedDetector(&m_NamedA);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
UTDTriggerMap::~UTDTriggerMap()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDTriggerMap::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestSetTriggerLong() && Passed;
  Passed = TestSetTriggerShort() && Passed;
  Passed = TestAppliesTriggeringVetoing() && Passed;
  Passed = TestAddHit() && Passed;
  Passed = TestAddGuardRingHit() && Passed;
  Passed = TestNamedDetectors() && Passed;
  Passed = TestHitThresholds() && Passed;
  Passed = TestDecisions() && Passed;
  Passed = TestVetoPriority() && Passed;
  Passed = TestReset() && Passed;
  Passed = TestReadValid() && Passed;
  Passed = TestReadMalformed() && Passed;
  Passed = TestReuse() && Passed;
  Passed = TestWriteRead() && Passed;
  Passed = TestValidate() && Passed;
  Passed = TestText() && Passed;
  Passed = TestExampleFile() && Passed;
  Passed = TestExampleGeometry() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a hit in the given detector
MDVolumeSequence UTDTriggerMap::GetHit(MDDetector* Detector)
{
  MDVolumeSequence Hit;
  Hit.AddVolume(&m_Volume);
  Hit.SetDetectorVolume(&m_Volume);
  Hit.SetDetector(Detector);
  return Hit;
}


////////////////////////////////////////////////////////////////////////////////


//! Add the number of hits to the map
void UTDTriggerMap::AddHits(MDTriggerMap& Map, MDDetector* Detector, unsigned int Hits)
{
  MDVolumeSequence Hit = GetHit(Detector);
  for (unsigned int h = 0; h < Hits; ++h) {
    Map.AddHit(Hit);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Write a trigger map into a private file and read it, silently
bool UTDTriggerMap::ReadText(MDTriggerMap& Map, const MString& Content)
{
  const MString FileName = GetTemporaryFileName("Fixture.triggers");
  if (WriteTextFile(FileName, Content) == false) {
    return false;
  }
  DisableDefaultStreams();
  bool Result = Map.ReadTriggerMap(FileName);
  EnableDefaultStreams();
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the map file of the GRIPS example with the full path
MString UTDTriggerMap::GetExampleFile() const
{
  MString FileName = "$(MEGALIB)/resource/examples/geomega/GRIPS/GRIPS_extended.triggers";
  MFile::ExpandFileName(FileName);
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the constructor and the detector list
bool UTDTriggerMap::TestConstruction()
{
  bool Passed = true;

  MDTriggerMap Map("MyMap");
  Passed = EvaluateTrue("GetName()", "MyMap", "The name of the constructor is returned", Map.GetName() == "MyMap") && Passed;
  Passed = EvaluateTrue("GetType()", "default", "The type is the universal trigger", Map.GetType() == MDTriggerType::c_Universal) && Passed;
  Passed = EvaluateSize("GetDetectors()", "default", "There are no detectors", Map.GetDetectors().size(), 0) && Passed;

  // Nothing is raised by an empty map:
  Passed = EvaluateFalse("HasTriggered()", "empty map", "An empty map does not trigger", Map.HasTriggered()) && Passed;
  Passed = EvaluateFalse("HasNonVetoablyTriggered()", "empty map", "An empty map does not trigger", Map.HasNonVetoablyTriggered()) && Passed;
  Passed = EvaluateFalse("HasVetoed()", "empty map", "An empty map does not veto", Map.HasVetoed()) && Passed;
  Map.Reset();
  Passed = EvaluateFalse("Reset()", "empty map", "A reset of an empty map is possible", Map.HasTriggered()) && Passed;

  vector<MString> Detectors = { "DetB", "DetA" };
  Passed = EvaluateTrue("SetDetectors()", "two detectors", "The list is accepted", Map.SetDetectors(Detectors)) && Passed;
  Passed = EvaluateTrue("GetDetectors()", "two detectors", "The list is returned in its order", Map.GetDetectors() == Detectors) && Passed;

  // A map with detectors but without trigger rules:
  AddHits(Map, &m_DetA, 1);
  Passed = EvaluateFalse("HasTriggered()", "no rules, one hit", "A map without rules does not trigger", Map.HasTriggered()) && Passed;
  Passed = EvaluateFalse("HasVetoed()", "no rules, one hit", "A map without rules does not veto", Map.HasVetoed()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the long version of setting a trigger
bool UTDTriggerMap::TestSetTriggerLong()
{
  bool Passed = true;

  MDTriggerMap Map("Map");
  Map.SetDetectors({ "DetA", "DetB" });

  Passed = EvaluateTrue("SetTrigger(long)", "correct sizes", "Vectors of the size of the detector list are accepted", Map.SetTrigger(MDTriggerMode::c_VetoableTrigger, { 1, 0 }, { false, false })) && Passed;

  // The size limits of both vectors, just below and above:
  DisableDefaultStreams();
  bool ShortHits = Map.SetTrigger(MDTriggerMode::c_VetoableTrigger, vector<unsigned int>({ 1 }), vector<bool>({ false, false }));
  bool LongHits = Map.SetTrigger(MDTriggerMode::c_VetoableTrigger, vector<unsigned int>({ 1, 1, 1 }), vector<bool>({ false, false }));
  bool ShortAllow = Map.SetTrigger(MDTriggerMode::c_VetoableTrigger, vector<unsigned int>({ 1, 1 }), vector<bool>({ false }));
  bool LongAllow = Map.SetTrigger(MDTriggerMode::c_VetoableTrigger, vector<unsigned int>({ 1, 1 }), vector<bool>({ false, false, false }));
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetTrigger(long)", "hits n - 1", "Too few hit entries are rejected", ShortHits) && Passed;
  Passed = EvaluateFalse("SetTrigger(long)", "hits n + 1", "Too many hit entries are rejected", LongHits) && Passed;
  Passed = EvaluateFalse("SetTrigger(long)", "flags n - 1", "Too few flag entries are rejected", ShortAllow) && Passed;
  Passed = EvaluateFalse("SetTrigger(long)", "flags n + 1", "Too many flag entries are rejected", LongAllow) && Passed;

  // Only the valid rule exists, it requires exactly 1 hit in A and none in B:
  AddHits(Map, &m_DetA, 1);
  Passed = EvaluateTrue("SetTrigger(long)", "after rejected rules", "The rejected rules have not been stored", Map.HasTriggered()) && Passed;

  // The error message reports the size of the argument and not the size of the stored rules:
  MDTriggerMap Message("Map");
  Message.SetDetectors({ "DetA", "DetB" });
  ostringstream Captured;
  streambuf* Original = cout.rdbuf(Captured.rdbuf());
  Message.SetTrigger(MDTriggerMode::c_VetoableTrigger, vector<unsigned int>({ 1, 1, 1 }), vector<bool>({ false, false }));
  cout.rdbuf(Original);
  // Defect GEO-E-12: The error message prints the number of stored rules instead of the received size
  Passed = ExpectedToFail(EvaluateTrue("SetTrigger(long)", "3 hit entries", "The error message reports the received 3 entries", MString(Captured.str()).Contains("expected: 2, received: 3"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the short version of setting a trigger
bool UTDTriggerMap::TestSetTriggerShort()
{
  bool Passed = true;

  // The not named detector C has zero or more hits:
  MDTriggerMap Map("Map");
  MDStrip2D DetC("DetC");
  Map.SetDetectors({ "DetA", "DetB", "DetC" });
  Passed = EvaluateTrue("SetTrigger(short)", "detector subset, reversed order", "The rule is accepted", Map.SetTrigger(MDTriggerMode::c_VetoableTrigger, { "DetB", "DetA" }, { 2, 1 }, { false, false })) && Passed;

  // Exactly 1 hit in A and exactly 2 in B - the hits belong to the names and not to the positions:
  AddHits(Map, &m_DetA, 1);
  AddHits(Map, &m_DetB, 1);
  Passed = EvaluateFalse("SetTrigger(short)", "A 1, B 1, C 0", "The two hits required in B are missing", Map.HasTriggered()) && Passed;
  AddHits(Map, &m_DetB, 1);
  Passed = EvaluateTrue("SetTrigger(short)", "A 1, B 2, C 0", "The hits belong to the given names", Map.HasTriggered()) && Passed;
  AddHits(Map, &DetC, 5);
  Passed = EvaluateTrue("SetTrigger(short)", "A 1, B 2, C 5", "A detector which is not named allows more hits", Map.HasTriggered()) && Passed;

  // An unknown detector has to be an error - otherwise the rule has no conditions at all and is always raised:
  MDTriggerMap Unknown("Map");
  Unknown.SetDetectors({ "DetA", "DetB" });
  bool UnknownResult = Unknown.SetTrigger(MDTriggerMode::c_VetoableTrigger, { "Typo" }, { 1 }, { false });
  // Defect GEO-E-13: The short SetTrigger ignores an unknown detector and stores a rule without conditions, which is always raised
  Passed = ExpectedToFail(EvaluateFalse("SetTrigger(short)", "unknown detector", "A rule with an unknown detector is rejected", UnknownResult)) && Passed;
  // Defect GEO-E-13: The short SetTrigger ignores an unknown detector and stores a rule without conditions, which is always raised
  Passed = ExpectedToFail(EvaluateFalse("HasTriggered()", "unknown detector, no hits", "A map with a rejected rule does not trigger without hits", Unknown.HasTriggered())) && Passed;

  // Mismatched sizes - the access goes behind the end of the shorter vectors:
  // Defect GEO-E-14: The short SetTrigger does not check the sizes and reads behind the end of the vectors
  MDTriggerMap Mismatched("Map");
  Mismatched.SetDetectors({ "DetA", "DetB" });
  Passed = ExpectedToCrash(EvaluateFalse("SetTrigger(short)", "2 detectors, no hits", "Vectors of different sizes are rejected without a crash", Mismatched.SetTrigger(MDTriggerMode::c_VetoableTrigger, vector<MString>({ "DetA", "DetB" }), vector<unsigned int>(), vector<bool>()))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test which detectors are triggering and vetoing
bool UTDTriggerMap::TestAppliesTriggeringVetoing()
{
  bool Passed = true;

  MDStrip2D DetC("DetC");
  MDStrip2D DetD("DetD");
  MDStrip2D DetOther("Other");

  // A: trigger, B: non-vetoable trigger, C: veto, D: only zero hit conditions
  MDTriggerMap Map("Map");
  Map.SetDetectors({ "DetA", "DetB", "DetC", "DetD" });
  Map.SetTrigger(MDTriggerMode::c_VetoableTrigger, { 1, 0, 0, 0 }, { true, true, true, false });
  Map.SetTrigger(MDTriggerMode::c_NonVetoableTrigger, { 0, 2, 0, 0 }, { true, true, true, false });
  Map.SetTrigger(MDTriggerMode::c_Veto, { 0, 0, 1, 0 }, { true, true, true, false });

  Passed = EvaluateTrue("Applies()", "A", "The map applies to a listed detector", Map.Applies(&m_DetA)) && Passed;
  Passed = EvaluateTrue("Applies()", "D", "The map applies to a listed detector without hit conditions", Map.Applies(&DetD)) && Passed;
  Passed = EvaluateFalse("Applies()", "Other", "The map does not apply to a detector which is not listed", Map.Applies(&DetOther)) && Passed;

  Passed = EvaluateTrue("IsTriggering()", "A", "A required hit in a vetoable trigger is triggering", Map.IsTriggering(&m_DetA)) && Passed;
  Passed = EvaluateTrue("IsTriggering()", "B", "A required hit in a non-vetoable trigger is triggering", Map.IsTriggering(&m_DetB)) && Passed;
  Passed = EvaluateFalse("IsTriggering()", "C", "A required hit in a veto is not triggering", Map.IsTriggering(&DetC)) && Passed;
  Passed = EvaluateFalse("IsTriggering()", "D", "A detector without required hits is not triggering", Map.IsTriggering(&DetD)) && Passed;
  Passed = EvaluateFalse("IsTriggering()", "Other", "A detector which is not listed is not triggering", Map.IsTriggering(&DetOther)) && Passed;

  Passed = EvaluateFalse("IsVetoing()", "A", "A required hit in a trigger is not vetoing", Map.IsVetoing(&m_DetA)) && Passed;
  Passed = EvaluateFalse("IsVetoing()", "B", "A required hit in a non-vetoable trigger is not vetoing", Map.IsVetoing(&m_DetB)) && Passed;
  Passed = EvaluateTrue("IsVetoing()", "C", "A required hit in a veto is vetoing", Map.IsVetoing(&DetC)) && Passed;
  Passed = EvaluateFalse("IsVetoing()", "D", "A detector without required hits is not vetoing", Map.IsVetoing(&DetD)) && Passed;
  Passed = EvaluateFalse("IsVetoing()", "Other", "A detector which is not listed is not vetoing", Map.IsVetoing(&DetOther)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the return values of adding hits
bool UTDTriggerMap::TestAddHit()
{
  bool Passed = true;

  MDStrip2D Other("Other");

  MDTriggerMap Map("Map");
  Map.SetDetectors({ "DetA", "DetB" });
  Map.SetTrigger(MDTriggerMode::c_VetoableTrigger, { 1, 0 }, { false, false });

  MDVolumeSequence HitA = GetHit(&m_DetA);
  MDVolumeSequence HitOther = GetHit(&Other);
  Passed = EvaluateTrue("AddHit()", "listed detector", "A hit in a listed detector is added", Map.AddHit(HitA)) && Passed;
  Passed = EvaluateFalse("AddHit()", "other detector", "A hit in another detector is not added", Map.AddHit(HitOther)) && Passed;
  Passed = EvaluateTrue("AddHit()", "after other detector", "The hit in another detector is not counted", Map.HasTriggered()) && Passed;

  // Without detector:
  MDVolumeSequence NoDetector;
  DisableDefaultStreams();
  bool NoDetectorResult = Map.AddHit(NoDetector);
  bool NoDetectorGuardRingResult = Map.AddGuardRingHit(NoDetector);
  EnableDefaultStreams();
  Passed = EvaluateFalse("AddHit()", "no detector", "A volume sequence without a detector is not added", NoDetectorResult) && Passed;
  Passed = EvaluateFalse("AddGuardRingHit()", "no detector", "A volume sequence without a detector is not added", NoDetectorGuardRingResult) && Passed;

  // A map without detectors:
  MDTriggerMap Empty("Map");
  Passed = EvaluateFalse("AddHit()", "map without detectors", "No hit can be added", Empty.AddHit(HitA)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding guard ring hits
bool UTDTriggerMap::TestAddGuardRingHit()
{
  bool Passed = true;

  MDStrip2D Ringed("Ringed");
  Ringed.HasGuardRing(true);
  MDVolumeSequence HitRinged = GetHit(&Ringed);

  // A veto by the guard ring - it has its own column, separated from the detector:
  MDTriggerMap Map("Map");
  Map.SetDetectors({ "Ringed", "Ringed_GuardRing" });
  Map.SetTrigger(MDTriggerMode::c_VetoableTrigger, { 1, 0 }, { true, true });
  Map.SetTrigger(MDTriggerMode::c_Veto, { 0, 1 }, { true, true });

  Passed = EvaluateTrue("AddHit()", "detector", "The normal hit is added", Map.AddHit(HitRinged)) && Passed;
  Passed = EvaluateTrue("HasTriggered()", "detector hit", "The normal hit raises the trigger", Map.HasTriggered()) && Passed;
  Passed = EvaluateFalse("HasVetoed()", "detector hit", "The normal hit does not raise the guard ring veto", Map.HasVetoed()) && Passed;

  Passed = EvaluateTrue("AddGuardRingHit()", "guard ring", "The guard ring hit is added", Map.AddGuardRingHit(HitRinged)) && Passed;
  Passed = EvaluateTrue("HasVetoed()", "guard ring hit", "The guard ring hit raises the veto", Map.HasVetoed()) && Passed;
  Passed = EvaluateFalse("HasTriggered()", "guard ring hit", "The veto suppresses the trigger", Map.HasTriggered()) && Passed;

  // A guard ring hit alone does not count for the detector:
  MDTriggerMap RingOnly("Map");
  RingOnly.SetDetectors({ "Ringed", "Ringed_GuardRing" });
  RingOnly.SetTrigger(MDTriggerMode::c_VetoableTrigger, { 1, 0 }, { true, true });
  RingOnly.AddGuardRingHit(HitRinged);
  Passed = EvaluateFalse("HasTriggered()", "only guard ring hit", "A guard ring hit does not count for the detector", RingOnly.HasTriggered()) && Passed;

  // Not listed guard ring:
  MDTriggerMap NotListed("Map");
  NotListed.SetDetectors({ "Ringed" });
  Passed = EvaluateFalse("AddGuardRingHit()", "guard ring not listed", "The hit is not added", NotListed.AddGuardRingHit(HitRinged)) && Passed;

  // A detector without a guard ring:
  MDVolumeSequence HitA = GetHit(&m_DetB);
  MDTriggerMap NoRing("Map");
  NoRing.SetDetectors({ "DetB" });
  DisableDefaultStreams();
  bool NoRingResult = NoRing.AddGuardRingHit(HitA);
  EnableDefaultStreams();
  Passed = EvaluateFalse("AddGuardRingHit()", "detector without guard ring", "The hit is not added", NoRingResult) && Passed;

  Ringed.HasGuardRing(false);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test hits in named detectors
bool UTDTriggerMap::TestNamedDetectors()
{
  bool Passed = true;

  MDVolumeSequence HitNamed = GetHit(&m_NamedA);

  // A map which has the name of the named detector counts its hits:
  MDTriggerMap ListedNamed("Map");
  ListedNamed.SetDetectors({ "NamedA" });
  ListedNamed.SetTrigger(MDTriggerMode::c_VetoableTrigger, { 1 }, { true });
  Passed = EvaluateTrue("AddHit()", "map lists the named detector", "The hit is added", ListedNamed.AddHit(HitNamed)) && Passed;
  Passed = EvaluateTrue("HasTriggered()", "map lists the named detector", "The hit raises the trigger", ListedNamed.HasTriggered()) && Passed;

  // The function has a second search with the detector the named detector is named after:
  MDTriggerMap ListedBase("Map");
  ListedBase.SetDetectors({ "DetA" });
  ListedBase.SetTrigger(MDTriggerMode::c_VetoableTrigger, { 1 }, { true });
  // Defect GEO-E-15: The second search compares the old name again, a hit in a named detector does not count for the detector it is named after
  Passed = ExpectedToFail(EvaluateTrue("AddHit()", "map lists the base detector", "A hit in the named detector is added to the base detector", ListedBase.AddHit(HitNamed))) && Passed;
  // Defect GEO-E-15: The second search compares the old name again, a hit in a named detector does not count for the detector it is named after
  Passed = ExpectedToFail(EvaluateTrue("HasTriggered()", "map lists the base detector", "A hit in the named detector raises the trigger", ListedBase.HasTriggered())) && Passed;

  // The basic trigger handles it this way, too - same for guard rings:
  m_NamedA.HasGuardRing(true);
  m_DetA.HasGuardRing(true);
  MDTriggerMap ListedBaseRing("Map");
  ListedBaseRing.SetDetectors({ "DetA_GuardRing" });
  ListedBaseRing.SetTrigger(MDTriggerMode::c_Veto, { 1 }, { true });
  // Defect GEO-E-15: The second search compares the old name again, a hit in a named detector does not count for the detector it is named after
  Passed = ExpectedToFail(EvaluateTrue("AddGuardRingHit()", "map lists the guard ring of the base detector", "A guard ring hit of the named detector is added", ListedBaseRing.AddGuardRingHit(HitNamed))) && Passed;
  m_DetA.HasGuardRing(false);
  m_NamedA.HasGuardRing(false);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test exact hit numbers against minimum hit numbers, just inside and outside
bool UTDTriggerMap::TestHitThresholds()
{
  bool Passed = true;

  // Exactly 2 hits, "2" in a file:
  MDTriggerMap Exact("Map");
  Exact.SetDetectors({ "DetA" });
  Exact.SetTrigger(MDTriggerMode::c_VetoableTrigger, { 2 }, { false });
  AddHits(Exact, &m_DetA, 1);
  Passed = EvaluateFalse("HasTriggered()", "exactly 2, 1 hit", "One hit below is not enough", Exact.HasTriggered()) && Passed;
  AddHits(Exact, &m_DetA, 1);
  Passed = EvaluateTrue("HasTriggered()", "exactly 2, 2 hits", "The exact number triggers", Exact.HasTriggered()) && Passed;
  AddHits(Exact, &m_DetA, 1);
  Passed = EvaluateFalse("HasTriggered()", "exactly 2, 3 hits", "One hit above does not trigger", Exact.HasTriggered()) && Passed;

  // At least 2 hits, "2+" in a file:
  MDTriggerMap Minimum("Map");
  Minimum.SetDetectors({ "DetA" });
  Minimum.SetTrigger(MDTriggerMode::c_VetoableTrigger, { 2 }, { true });
  AddHits(Minimum, &m_DetA, 1);
  Passed = EvaluateFalse("HasTriggered()", "at least 2, 1 hit", "One hit below is not enough", Minimum.HasTriggered()) && Passed;
  AddHits(Minimum, &m_DetA, 1);
  Passed = EvaluateTrue("HasTriggered()", "at least 2, 2 hits", "The minimum number triggers", Minimum.HasTriggered()) && Passed;
  AddHits(Minimum, &m_DetA, 1);
  Passed = EvaluateTrue("HasTriggered()", "at least 2, 3 hits", "More hits are allowed", Minimum.HasTriggered()) && Passed;

  // Exactly 0 hits in the second detector:
  MDTriggerMap Zero("Map");
  Zero.SetDetectors({ "DetA", "DetB" });
  Zero.SetTrigger(MDTriggerMode::c_VetoableTrigger, { 1, 0 }, { true, false });
  AddHits(Zero, &m_DetA, 1);
  Passed = EvaluateTrue("HasTriggered()", "0 in B, no B hit", "No hit in B is required", Zero.HasTriggered()) && Passed;
  AddHits(Zero, &m_DetB, 1);
  Passed = EvaluateFalse("HasTriggered()", "0 in B, 1 B hit", "One hit in B is too many", Zero.HasTriggered()) && Passed;

  // Attention: Zero or more hits everywhere - this rule is always raised, also without any hit:
  MDTriggerMap Always("Map");
  Always.SetDetectors({ "DetA" });
  Always.SetTrigger(MDTriggerMode::c_VetoableTrigger, { 0 }, { true });
  Passed = EvaluateTrue("HasTriggered()", "0+ only", "A rule without any required hit is always raised", Always.HasTriggered()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the decision of all modes against an independent decision for all hit combinations
bool UTDTriggerMap::TestDecisions()
{
  bool Passed = true;

  // All rules on detector A with the hit options 0, 1, 2 and exact/minimum, and on detector B 1+
  const vector<MDTriggerMode> Modes = { MDTriggerMode::c_VetoableTrigger, MDTriggerMode::c_NonVetoableTrigger, MDTriggerMode::c_Veto };
  vector<UTDTriggerMapRule> AllRules;
  for (MDTriggerMode Mode: Modes) {
    for (unsigned int Hits = 0; Hits <= 2; ++Hits) {
      const vector<bool> Options = { false, true };
      for (bool More: Options) {
        UTDTriggerMapRule Rule;
        Rule.m_Mode = Mode;
        Rule.m_Hits = { Hits, 1 };
        Rule.m_AllowMoreHits = { More, true };
        AllRules.push_back(Rule);
      }
    }
  }

  // Maps with up to two rules:
  vector<vector<UTDTriggerMapRule>> Maps;
  for (unsigned int a = 0; a < AllRules.size(); ++a) {
    Maps.push_back({ AllRules[a] });
    for (unsigned int b = a + 1; b < AllRules.size(); ++b) {
      Maps.push_back({ AllRules[a], AllRules[b] });
    }
  }

  unsigned int NDecisions = 0;
  unsigned int NWrongDecisions = 0;
  MString FirstWrongDecision = "";

  for (const vector<UTDTriggerMapRule>& Rules: Maps) {
    const vector<bool> IgnoreOptions = { false, true };
    for (bool Ignore: IgnoreOptions) {
      MDTriggerMap Map("Map");
      Map.SetDetectors({ "DetA", "DetB" });
      for (const UTDTriggerMapRule& Rule: Rules) {
        Map.SetTrigger(Rule.m_Mode, Rule.m_Hits, Rule.m_AllowMoreHits);
      }
      Map.IgnoreVetoes(Ignore);

      for (unsigned int CountA = 0; CountA <= 3; ++CountA) {
        for (unsigned int CountB = 0; CountB <= 2; ++CountB) {
          Map.Reset();
          AddHits(Map, &m_DetA, CountA);
          AddHits(Map, &m_DetB, CountB);

          // The rules of the header: One raised non-vetoable trigger cannot be vetoed and a veto stops the vetoable triggers
          bool Trigger = false;
          bool NonVetoable = false;
          bool Veto = false;
          for (const UTDTriggerMapRule& Rule: Rules) {
            if (Rule.IsFulfilled({ CountA, CountB }) == true) {
              if (Rule.m_Mode == MDTriggerMode::c_VetoableTrigger) {
                Trigger = true;
              } else if (Rule.m_Mode == MDTriggerMode::c_NonVetoableTrigger) {
                NonVetoable = true;
              } else {
                Veto = true;
              }
            }
          }
          bool ExpectedVeto = false;
          if (Veto == true && NonVetoable == false && Ignore == false) {
            ExpectedVeto = true;
          }
          bool ExpectedTrigger = false;
          if (NonVetoable == true || (Trigger == true && (Veto == false || Ignore == true))) {
            ExpectedTrigger = true;
          }

          ++NDecisions;
          if (Map.HasTriggered() != ExpectedTrigger || Map.HasNonVetoablyTriggered() != NonVetoable || Map.HasVetoed() != ExpectedVeto) {
            ++NWrongDecisions;
            if (FirstWrongDecision == "") {
              FirstWrongDecision = MString("A ") + CountA + ", B " + CountB + ", number of rules " + static_cast<unsigned int>(Rules.size());
            }
          }
        }
      }
    }
  }

  // 18 rules give 18 + 18*17/2 = 171 maps, with 2 veto options and 4 x 3 hit combinations = 4104
  Passed = Evaluate("HasTriggered/HasNonVetoablyTriggered/HasVetoed", "all maps with 1 or 2 rules", "All combinations have been tested", NDecisions, 4104U) && Passed;
  Passed = Evaluate("HasTriggered/HasNonVetoablyTriggered/HasVetoed", FirstWrongDecision, "All decisions agree with the independent decision", NWrongDecisions, 0U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the priority of vetoes and non-vetoable triggers and ignoring vetoes
bool UTDTriggerMap::TestVetoPriority()
{
  bool Passed = true;

  // TV A 1+, TN A 2+, VT B 1+
  MDTriggerMap Map("Map");
  Map.SetDetectors({ "DetA", "DetB" });
  Map.SetTrigger(MDTriggerMode::c_VetoableTrigger, { 1, 0 }, { true, true });
  Map.SetTrigger(MDTriggerMode::c_NonVetoableTrigger, { 2, 0 }, { true, true });
  Map.SetTrigger(MDTriggerMode::c_Veto, { 0, 1 }, { true, true });

  // Only a vetoable trigger:
  AddHits(Map, &m_DetA, 1);
  Passed = EvaluateTrue("HasTriggered()", "A 1, B 0", "The vetoable trigger is raised", Map.HasTriggered()) && Passed;
  Passed = EvaluateFalse("HasNonVetoablyTriggered()", "A 1, B 0", "The non-vetoable trigger is not raised", Map.HasNonVetoablyTriggered()) && Passed;
  Passed = EvaluateFalse("HasVetoed()", "A 1, B 0", "There is no veto", Map.HasVetoed()) && Passed;

  // Veto beats vetoable trigger:
  AddHits(Map, &m_DetB, 1);
  Passed = EvaluateFalse("HasTriggered()", "A 1, B 1", "The veto stops the vetoable trigger", Map.HasTriggered()) && Passed;
  Passed = EvaluateTrue("HasVetoed()", "A 1, B 1", "The veto is raised", Map.HasVetoed()) && Passed;

  // Ignoring the veto as during pretriggering:
  Map.IgnoreVetoes(true);
  Passed = EvaluateTrue("IgnoreVetoes()", "true, A 1, B 1", "The vetoable trigger is raised", Map.HasTriggered()) && Passed;
  Passed = EvaluateFalse("IgnoreVetoes()", "true, A 1, B 1", "The veto is not raised", Map.HasVetoed()) && Passed;
  Map.IgnoreVetoes(false);
  Passed = EvaluateFalse("IgnoreVetoes()", "false, A 1, B 1", "The veto is back", Map.HasTriggered()) && Passed;

  // Non-vetoable trigger beats veto:
  AddHits(Map, &m_DetA, 1);
  Passed = EvaluateTrue("HasTriggered()", "A 2, B 1", "The non-vetoable trigger is not stopped", Map.HasTriggered()) && Passed;
  Passed = EvaluateTrue("HasNonVetoablyTriggered()", "A 2, B 1", "The non-vetoable trigger is raised", Map.HasNonVetoablyTriggered()) && Passed;
  Passed = EvaluateFalse("HasVetoed()", "A 2, B 1", "The non-vetoable trigger cancels the veto", Map.HasVetoed()) && Passed;

  // A veto alone without a trigger:
  MDTriggerMap VetoOnly("Map");
  VetoOnly.SetDetectors({ "DetB" });
  VetoOnly.SetTrigger(MDTriggerMode::c_Veto, { 1 }, { true });
  AddHits(VetoOnly, &m_DetB, 1);
  Passed = EvaluateFalse("HasTriggered()", "only veto", "A veto does not trigger", VetoOnly.HasTriggered()) && Passed;
  Passed = EvaluateTrue("HasVetoed()", "only veto", "The veto is raised", VetoOnly.HasVetoed()) && Passed;

  // A rule of an unknown mode raises nothing:
  MDTriggerMap UnknownMode("Map");
  UnknownMode.SetDetectors({ "DetA" });
  UnknownMode.SetTrigger(MDTriggerMode::c_Unknown, { 0 }, { true });
  Passed = EvaluateFalse("HasTriggered()", "unknown mode", "A rule of an unknown mode does not trigger", UnknownMode.HasTriggered()) && Passed;
  Passed = EvaluateFalse("HasVetoed()", "unknown mode", "A rule of an unknown mode does not veto", UnknownMode.HasVetoed()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that no state leaks between events
bool UTDTriggerMap::TestReset()
{
  bool Passed = true;

  MDTriggerMap Map("Map");
  Map.SetDetectors({ "DetA", "DetB" });
  Map.SetTrigger(MDTriggerMode::c_VetoableTrigger, { 2, 0 }, { true, true });
  Map.SetTrigger(MDTriggerMode::c_Veto, { 0, 1 }, { true, true });

  AddHits(Map, &m_DetA, 2);
  Passed = EvaluateTrue("HasTriggered()", "event 1", "Event 1 raises the trigger", Map.HasTriggered()) && Passed;
  Passed = EvaluateTrue("HasTriggered()", "event 1, repeated read", "Reading the decision does not change it", Map.HasTriggered()) && Passed;

  Map.Reset();
  Passed = EvaluateFalse("Reset()", "no hits", "The trigger is gone after the reset", Map.HasTriggered()) && Passed;
  AddHits(Map, &m_DetA, 1);
  Passed = EvaluateFalse("Reset()", "event 2, 1 of 2 hits", "One hit is not enough after the reset", Map.HasTriggered()) && Passed;

  AddHits(Map, &m_DetB, 1);
  Passed = EvaluateTrue("HasVetoed()", "event 2", "Event 2 raises the veto", Map.HasVetoed()) && Passed;
  Map.Reset();
  Passed = EvaluateFalse("Reset()", "event 3, no hits", "The veto is gone after the reset", Map.HasVetoed()) && Passed;

  AddHits(Map, &m_DetA, 2);
  Passed = EvaluateTrue("Reset()", "event 3", "The rules are still intact after the resets", Map.HasTriggered()) && Passed;
  Passed = EvaluateTrue("Reset()", "event 3", "The detector list is still intact", Map.GetDetectors() == vector<MString>({ "DetA", "DetB" })) && Passed;

  Map.Reset();
  Map.Reset();
  Passed = EvaluateFalse("Reset()", "twice", "A repeated reset is possible", Map.HasTriggered()) && Passed;

  // The veto ignore flag survives the reset:
  Map.IgnoreVetoes(true);
  Map.Reset();
  AddHits(Map, &m_DetB, 1);
  Passed = EvaluateFalse("Reset()", "vetoes ignored", "The flag to ignore vetoes survives the reset", Map.HasVetoed()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test reading valid files
bool UTDTriggerMap::TestReadValid()
{
  bool Passed = true;

  // With comments, empty lines, all modes, and exact and minimum numbers:
  MString Content;
  Content += "# Comment line\n";
  Content += "\n";
  Content += "DT DetA DetB\n";
  Content += "// Another comment\n";
  Content += "TV 1+ 0\n";
  Content += "TN 3 1+ // Inline comment\n";
  Content += "VT 0+ 2 // Inline comment\n";

  MDTriggerMap Map("Map");
  Passed = EvaluateTrue("ReadTriggerMap()", "valid file", "The file is read", ReadText(Map, Content)) && Passed;
  Passed = EvaluateTrue("GetDetectors()", "valid file", "The detector list is read in its order", Map.GetDetectors() == vector<MString>({ "DetA", "DetB" })) && Passed;

  // TV 1+ 0
  AddHits(Map, &m_DetA, 1);
  Passed = EvaluateTrue("HasTriggered()", "A 1, B 0", "TV 1+ 0 is raised", Map.HasTriggered()) && Passed;
  Passed = EvaluateFalse("HasNonVetoablyTriggered()", "A 1, B 0", "TN 3 1+ is not raised", Map.HasNonVetoablyTriggered()) && Passed;
  AddHits(Map, &m_DetB, 1);
  Passed = EvaluateFalse("HasTriggered()", "A 1, B 1", "The exact number 0 of TV 1+ 0 is not fulfilled any more", Map.HasTriggered()) && Passed;

  // VT 0+ 2
  AddHits(Map, &m_DetB, 1);
  Passed = EvaluateTrue("HasVetoed()", "A 1, B 2", "VT 0+ 2 is raised", Map.HasVetoed()) && Passed;
  AddHits(Map, &m_DetB, 1);
  Passed = EvaluateFalse("HasVetoed()", "A 1, B 3", "The exact number 2 of VT 0+ 2 is not fulfilled any more", Map.HasVetoed()) && Passed;

  // TN 3 1+
  AddHits(Map, &m_DetA, 2);
  Passed = EvaluateTrue("HasNonVetoablyTriggered()", "A 3, B 3", "TN 3 1+ is raised", Map.HasNonVetoablyTriggered()) && Passed;
  AddHits(Map, &m_DetA, 1);
  Passed = EvaluateFalse("HasNonVetoablyTriggered()", "A 4, B 3", "The exact number 3 of TN 3 1+ is not fulfilled any more", Map.HasNonVetoablyTriggered()) && Passed;

  // The rules may stand before the detector list:
  MDTriggerMap Reversed("Map");
  Passed = EvaluateTrue("ReadTriggerMap()", "rules before the detector list", "The file is read", ReadText(Reversed, "TV 1\nDT DetA\n")) && Passed;
  AddHits(Reversed, &m_DetA, 1);
  Passed = EvaluateTrue("HasTriggered()", "rules before the detector list", "The rule is used", Reversed.HasTriggered()) && Passed;

  // Only a detector list:
  MDTriggerMap OnlyDetectors("Map");
  Passed = EvaluateTrue("ReadTriggerMap()", "only a detector list", "The file is read", ReadText(OnlyDetectors, "DT DetA\n")) && Passed;
  Passed = EvaluateFalse("HasTriggered()", "only a detector list", "There are no rules", OnlyDetectors.HasTriggered()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test reading malformed files
bool UTDTriggerMap::TestReadMalformed()
{
  bool Passed = true;

  // A missing file:
  MDTriggerMap Missing("Map");
  DisableDefaultStreams();
  bool MissingResult = Missing.ReadTriggerMap(GetTemporaryFileName("NotExisting.triggers"));
  EnableDefaultStreams();
  Passed = EvaluateFalse("ReadTriggerMap()", "missing file", "A missing file is rejected", MissingResult) && Passed;
  Passed = EvaluateSize("ReadTriggerMap()", "missing file", "No detectors are stored", Missing.GetDetectors().size(), 0) && Passed;

  // The number of columns just below and above the number of detectors:
  MDTriggerMap FewerColumns("Map");
  Passed = EvaluateFalse("ReadTriggerMap()", "n - 1 columns", "A rule with too few columns is rejected", ReadText(FewerColumns, "DT DetA DetB\nTV 1\n")) && Passed;
  MDTriggerMap MoreColumns("Map");
  Passed = EvaluateFalse("ReadTriggerMap()", "n + 1 columns", "A rule with too many columns is rejected", ReadText(MoreColumns, "DT DetA DetB\nTV 1 1 1\n")) && Passed;
  MDTriggerMap NoColumns("Map");
  Passed = EvaluateFalse("ReadTriggerMap()", "rule without columns", "A rule without columns is rejected", ReadText(NoColumns, "DT DetA DetB\nVT\n")) && Passed;
  MDTriggerMap NoDetectorList("Map");
  Passed = EvaluateFalse("ReadTriggerMap()", "rule without detector list", "A rule without a detector list is rejected", ReadText(NoDetectorList, "TV 1 1\n")) && Passed;
  MDTriggerMap EmptyDetectorList("Map");
  Passed = EvaluateFalse("ReadTriggerMap()", "rule and empty detector list", "A rule with an empty detector list is rejected", ReadText(EmptyDetectorList, "DT\nTV 1\n")) && Passed;

  // Files without any detector cannot describe a trigger, all events would be lost without a message:
  MDTriggerMap EmptyFile("Map");
  // Defect GEO-E-16: A file without detector list and rules is accepted, nothing ever triggers without an error
  Passed = ExpectedToFail(EvaluateFalse("ReadTriggerMap()", "empty file", "An empty file is rejected", ReadText(EmptyFile, ""))) && Passed;
  MDTriggerMap OnlyComments("Map");
  // Defect GEO-E-16: A file without detector list and rules is accepted, nothing ever triggers without an error
  Passed = ExpectedToFail(EvaluateFalse("ReadTriggerMap()", "only comments", "A file with only comments is rejected", ReadText(OnlyComments, "# Nothing here\n"))) && Passed;

  // Hit entries without a number end the program:
  const vector<MString> NoNumbers = { "abc", "+", "++", "x+" };
  const MString FileName = GetTemporaryFileName("NoNumber.triggers");
  for (MString Entry: NoNumbers) {
    Passed = EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(FileName, MString("DT DetA\nTV ") + Entry + "\n")) && Passed;
    // Defect GEO-E-17: A hit entry without a number throws an uncaught exception from the file parser, which ends the program
    MDTriggerMap NoNumber("Map");
    Passed = ExpectedToCrash(EvaluateFalse("ReadTriggerMap()", MString("hit entry ") + Entry, "A hit entry which is not a number is rejected without a crash", NoNumber.ReadTriggerMap(FileName))) && Passed;
  }

  // Hit entries which are no valid hit numbers but read as something else:
  const vector<MString> WrongNumbers = { "-1", "1.5", "1x", "1+1", "99999999999" };
  for (MString Entry: WrongNumbers) {
    MDTriggerMap Map("Map");
    bool Result = ReadText(Map, MString("DT DetA\nTV ") + Entry + "\n");
    // Defect GEO-E-18: A negative, fractional, too large, or partially numeric hit entry is silently read as another number
    Passed = ExpectedToFail(EvaluateFalse("ReadTriggerMap()", MString("hit entry ") + Entry, "A hit entry which is not a valid number of hits is rejected", Result)) && Passed;
  }

  // A failed read must not leave anything behind - the first rule is correct, the second one is not:
  MDTriggerMap Stale("Map");
  bool StaleResult = ReadText(Stale, "DT DetA\nTV 0+\nTV 1 1\n");
  Passed = EvaluateFalse("ReadTriggerMap()", "second rule wrong", "A file with one wrong rule is rejected", StaleResult) && Passed;
  // Defect GEO-E-19: A failed read leaves the detectors and rules before the error behind
  Passed = ExpectedToFail(EvaluateSize("ReadTriggerMap()", "second rule wrong", "No detectors stay behind after the failed read", Stale.GetDetectors().size(), 0)) && Passed;
  // Defect GEO-E-19: A failed read leaves the detectors and rules before the error behind
  Passed = ExpectedToFail(EvaluateFalse("HasTriggered()", "second rule wrong", "No rule stays behind after the failed read", Stale.HasTriggered())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test reusing a map for a second read or a new detector list
bool UTDTriggerMap::TestReuse()
{
  bool Passed = true;

  // The second file replaces the first one:
  MDTriggerMap Reread("Map");
  Passed = EvaluateTrue("ReadTriggerMap()", "first file", "The first file is read", ReadText(Reread, "DT DetA DetB\nTV 1 0\n")) && Passed;
  Passed = EvaluateTrue("ReadTriggerMap()", "second file", "The second file is read", ReadText(Reread, "DT DetA DetB\nTV 0 1\n")) && Passed;
  Passed = EvaluateTrue("GetDetectors()", "second file", "The detector list of the second file is used", Reread.GetDetectors() == vector<MString>({ "DetA", "DetB" })) && Passed;
  AddHits(Reread, &m_DetA, 1);
  // Defect GEO-E-20: A second read adds the new rules to the old ones instead of replacing them
  Passed = ExpectedToFail(EvaluateFalse("HasTriggered()", "A 1 after second file", "The rule of the first file is gone", Reread.HasTriggered())) && Passed;

  // A new detector list does not take over the hits of the old list:
  MDTriggerMap NewList("Map");
  NewList.SetDetectors({ "DetA" });
  AddHits(NewList, &m_DetA, 1);
  NewList.SetDetectors({ "DetB" });
  NewList.SetTrigger(MDTriggerMode::c_VetoableTrigger, { 1 }, { false });
  // Defect GEO-E-21: A new detector list keeps the hit counters of the old list and assigns them to the new detectors
  Passed = ExpectedToFail(EvaluateFalse("SetDetectors()", "hit in old detector", "The hit in the old detector A does not count for the new detector B", NewList.HasTriggered())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test writing and reading back
bool UTDTriggerMap::TestWriteRead()
{
  bool Passed = true;

  MDTriggerMap Map("Map");
  Map.SetDetectors({ "DetA", "DetB" });
  Map.SetTrigger(MDTriggerMode::c_VetoableTrigger, { 1, 0 }, { true, false });
  Map.SetTrigger(MDTriggerMode::c_NonVetoableTrigger, { 3, 1 }, { false, true });
  Map.SetTrigger(MDTriggerMode::c_Veto, { 0, 2 }, { true, false });

  // The file format:
  const MString FileName = GetTemporaryFileName("Written.triggers");
  Passed = EvaluateTrue("WriteTriggerMap()", "all modes", "The file is written", Map.WriteTriggerMap(FileName)) && Passed;
  MString Expected = "# Trigger map\n\n# Detectors\nDT DetA DetB \n\n#Triggers: \nTV 1+ 0 \nTN 3 1+ \nVT 0+ 2 \n\n";
  Passed = EvaluateTrue("WriteTriggerMap()", "all modes", "The file has the documented format", ReadTextFile(FileName) == Expected) && Passed;

  // All decisions of the read map have to be the ones of the original map:
  MDTriggerMap Read("Other");
  DisableDefaultStreams();
  bool ReadResult = Read.ReadTriggerMap(FileName);
  EnableDefaultStreams();
  Passed = EvaluateTrue("ReadTriggerMap()", "written file", "The written file can be read", ReadResult) && Passed;
  Passed = EvaluateTrue("GetDetectors()", "written file", "The detector list is the same", Read.GetDetectors() == Map.GetDetectors()) && Passed;

  unsigned int NDifferent = 0;
  for (unsigned int CountA = 0; CountA <= 4; ++CountA) {
    for (unsigned int CountB = 0; CountB <= 3; ++CountB) {
      Map.Reset();
      Read.Reset();
      AddHits(Map, &m_DetA, CountA);
      AddHits(Map, &m_DetB, CountB);
      AddHits(Read, &m_DetA, CountA);
      AddHits(Read, &m_DetB, CountB);
      if (Map.HasTriggered() != Read.HasTriggered() || Map.HasVetoed() != Read.HasVetoed() || Map.HasNonVetoablyTriggered() != Read.HasNonVetoablyTriggered()) {
        ++NDifferent;
      }
    }
  }
  Passed = Evaluate("ReadTriggerMap()", "round trip, 20 hit combinations", "The read map decides as the original", NDifferent, 0U) && Passed;

  // Writing into a not existing directory:
  DisableDefaultStreams();
  bool NoDirectory = Map.WriteTriggerMap(GetTemporaryDirectoryName("NotExisting") + "/File.triggers");
  EnableDefaultStreams();
  Passed = EvaluateFalse("WriteTriggerMap()", "not existing directory", "The error is reported", NoDirectory) && Passed;

  // A rule of an unknown mode cannot be written:
  MDTriggerMap Unknown("Map");
  Unknown.SetDetectors({ "DetA" });
  Unknown.SetTrigger(MDTriggerMode::c_Unknown, { 1 }, { true });
  DisableDefaultStreams();
  bool UnknownResult = Unknown.WriteTriggerMap(GetTemporaryFileName("Unknown.triggers"));
  EnableDefaultStreams();
  Passed = EvaluateFalse("WriteTriggerMap()", "unknown mode", "The error is reported", UnknownResult) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the validation
bool UTDTriggerMap::TestValidate()
{
  bool Passed = true;

  MDTriggerMap Good("Map");
  Good.SetDetectors({ "DetA", "DetB" });
  Good.SetTrigger(MDTriggerMode::c_VetoableTrigger, { 1, 0 }, { true, true });
  Passed = EvaluateTrue("Validate()", "valid map", "A valid map passes", Good.Validate()) && Passed;

  // The basic trigger rejects a trigger without detectors and detectors which appear twice:
  MDTriggerMap Empty("Map");
  // Defect GEO-E-22: The validation of the map checks nothing, in contrast to the validation of the basic trigger
  Passed = ExpectedToFail(EvaluateFalse("Validate()", "no detectors", "A map without detectors is invalid", Empty.Validate())) && Passed;

  MDTriggerMap Twice("Map");
  Twice.SetDetectors({ "DetA", "DetA" });
  Twice.SetTrigger(MDTriggerMode::c_VetoableTrigger, { 1, 1 }, { true, true });
  // Defect GEO-E-22: The validation of the map checks nothing, in contrast to the validation of the basic trigger
  Passed = ExpectedToFail(EvaluateFalse("Validate()", "detector twice", "A map with a detector which appears twice is invalid - the second column would never get a hit", Twice.Validate())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output
bool UTDTriggerMap::TestText()
{
  bool Passed = true;

  MDTriggerMap Map("MyMap");
  Map.SetDetectors({ "DetA" });
  Map.SetTrigger(MDTriggerMode::c_VetoableTrigger, { 1 }, { true });

  Passed = EvaluateTrue("ToString()", "map", "The text names the trigger class", Map.ToString().Contains("universal")) && Passed;

  // Via the base class the text is available:
  const MDTrigger& Base = Map;
  ostringstream BaseOut;
  BaseOut<<Base;
  Passed = EvaluateTrue("operator<<(MDTrigger)", "map", "The stream output of the base class is the text", MString(BaseOut.str()) == Map.ToString()) && Passed;

  // The basic trigger has the same text in both stream operators:
  ostringstream Out;
  Out<<Map;
  // Defect GEO-E-23: The stream operator of the map writes nothing
  Passed = ExpectedToFail(EvaluateTrue("operator<<(MDTriggerMap)", "map", "The stream output is the text of ToString", MString(Out.str()) == Map.ToString())) && Passed;

  // The description has to be readable by the geometry:
  // Defect GEO-E-24: The geomega description of a map is the text "Doesn't work anyway..." and cannot be read back
  Passed = ExpectedToFail(EvaluateTrue("GetGeomega()", "map", "The description has the keyword of the setup file with the name", Map.GetGeomega().Contains("TriggerMap MyMap"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the trigger map example in the resources
bool UTDTriggerMap::TestExampleFile()
{
  bool Passed = true;

  MDTriggerMap Map("Triggers");
  DisableDefaultStreams();
  bool ReadResult = Map.ReadTriggerMap(GetExampleFile());
  EnableDefaultStreams();
  Passed = EvaluateTrue("ReadTriggerMap()", "GRIPS example", "The example is read", ReadResult) && Passed;

  // The detector names and their order are the first line of the file:
  vector<MString> Names = { "SiStripDetector", "xDCal4L1", "xDCal4L2", "yDCal4L1", "yDCal4L2", "DCal8", "ACDetectorTop", "ACDetectorBottom" };
  Passed = EvaluateTrue("GetDetectors()", "GRIPS example", "All 8 detectors are read in their order", Map.GetDetectors() == Names) && Passed;
  if (Map.GetDetectors() != Names) {
    return false;
  }

  vector<MDStrip2D*> Detectors;
  for (MString Name: Names) {
    Detectors.push_back(new MDStrip2D(Name));
  }

  // The rules according to the comments of the file:
  // Compton: at least 1 tracker hit and at least 1 hit in one calorimeter, vetoable
  // Pair: at least 30 tracker hits and at least 5 hits in one calorimeter, non-vetoable
  // Veto: at least one hit in one of the two anti-coincidence systems
  const vector<unsigned int> TrackerCounts = { 0, 1, 29, 30, 31 };
  const vector<unsigned int> CalorimeterCounts = { 0, 1, 4, 5, 6 };
  unsigned int NCases = 0;
  unsigned int NWrong = 0;
  MString FirstWrong = "";
  for (unsigned int Tracker: TrackerCounts) {
    for (unsigned int Calorimeter = 1; Calorimeter <= 5; ++Calorimeter) {
      for (unsigned int CalorimeterHits: CalorimeterCounts) {
        for (unsigned int Top = 0; Top <= 1; ++Top) {
          for (unsigned int Bottom = 0; Bottom <= 1; ++Bottom) {
            Map.Reset();
            AddHits(Map, Detectors[0], Tracker);
            AddHits(Map, Detectors[Calorimeter], CalorimeterHits);
            AddHits(Map, Detectors[6], Top);
            AddHits(Map, Detectors[7], Bottom);

            bool Compton = false;
            if (Tracker >= 1 && CalorimeterHits >= 1) {
              Compton = true;
            }
            bool Pair = false;
            if (Tracker >= 30 && CalorimeterHits >= 5) {
              Pair = true;
            }
            bool Veto = false;
            if (Top >= 1 || Bottom >= 1) {
              Veto = true;
            }

            bool ExpectedTrigger = false;
            if (Pair == true || (Compton == true && Veto == false)) {
              ExpectedTrigger = true;
            }
            bool ExpectedVeto = false;
            if (Veto == true && Pair == false) {
              ExpectedVeto = true;
            }

            ++NCases;
            if (Map.HasTriggered() != ExpectedTrigger || Map.HasNonVetoablyTriggered() != Pair || Map.HasVetoed() != ExpectedVeto) {
              ++NWrong;
              if (FirstWrong == "") {
                FirstWrong = MString("Tracker ") + Tracker + ", calorimeter " + Calorimeter + " with " + CalorimeterHits + ", top " + Top + ", bottom " + Bottom;
              }
            }
          }
        }
      }
    }
  }
  // 5 tracker counts x 5 calorimeters x 5 calorimeter counts x 2 x 2 = 500
  Passed = Evaluate("HasTriggered/HasNonVetoablyTriggered/HasVetoed", "GRIPS example", "All combinations have been tested", NCases, 500U) && Passed;
  Passed = Evaluate("HasTriggered/HasNonVetoablyTriggered/HasVetoed", FirstWrong, "The decisions are the ones described in the example file", NWrong, 0U) && Passed;

  // The two calorimeter rules are independent of each other: 1 hit in each of two calorimeters is still a Compton trigger
  Map.Reset();
  AddHits(Map, Detectors[0], 1);
  AddHits(Map, Detectors[1], 1);
  AddHits(Map, Detectors[5], 1);
  Passed = EvaluateTrue("HasTriggered()", "GRIPS example, two calorimeters", "More calorimeters than required are allowed", Map.HasTriggered()) && Passed;

  // Which detectors can trigger and veto:
  for (unsigned int d = 0; d < 6; ++d) {
    Passed = EvaluateTrue("IsTriggering()", Names[d], "The tracker and the calorimeters are triggering", Map.IsTriggering(Detectors[d])) && Passed;
    Passed = EvaluateFalse("IsVetoing()", Names[d], "The tracker and the calorimeters are not vetoing", Map.IsVetoing(Detectors[d])) && Passed;
  }
  for (unsigned int d = 6; d < 8; ++d) {
    Passed = EvaluateFalse("IsTriggering()", Names[d], "The anti-coincidence systems are not triggering", Map.IsTriggering(Detectors[d])) && Passed;
    Passed = EvaluateTrue("IsVetoing()", Names[d], "The anti-coincidence systems are vetoing", Map.IsVetoing(Detectors[d])) && Passed;
  }

  for (MDStrip2D* D: Detectors) {
    delete D;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the geometry of the trigger map example
bool UTDTriggerMap::TestExampleGeometry()
{
  bool Passed = true;

  MString FileName = "$(MEGALIB)/resource/examples/geomega/GRIPS/GRIPS_extended.geo.setup";
  MFile::ExpandFileName(FileName);

  MDGeometry Geometry;
  DisableDefaultStreams();
  bool Scanned = Geometry.ScanSetupFile(FileName, false, false, false);
  EnableDefaultStreams();
  Passed = EvaluateTrue("ScanSetupFile()", "GRIPS example", "The geometry with the trigger map is read and validated", Scanned) && Passed;
  if (Scanned == false) {
    return Passed;
  }

  Passed = Evaluate("GetNTriggers()", "GRIPS example", "The geometry has one trigger", Geometry.GetNTriggers(), 1U) && Passed;
  MDTrigger* Trigger = Geometry.GetTrigger("Triggers");
  Passed = EvaluateTrue("GetTrigger()", "GRIPS example", "The trigger is found", Trigger != nullptr) && Passed;
  if (Trigger == nullptr) {
    return Passed;
  }
  Passed = EvaluateTrue("GetType()", "GRIPS example", "The trigger is a map", Trigger->GetType() == MDTriggerType::c_Universal) && Passed;

  // The relative file name of the map in the setup file has been found:
  MDTriggerMap* Map = dynamic_cast<MDTriggerMap*>(Trigger);
  if (Map != nullptr) {
    Passed = EvaluateSize("GetDetectors()", "GRIPS example", "The map has its 8 detectors", Map->GetDetectors().size(), 8) && Passed;
    Passed = EvaluateTrue("Validate()", "GRIPS example", "The trigger unit accepts the map", Geometry.GetTriggerUnit()->Validate()) && Passed;
  }

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

  UTDTriggerMap Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDTriggerMap.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
