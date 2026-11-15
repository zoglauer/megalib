/*
 * UTDDetectorEffectsEngine.cxx
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
#include <cmath>
#include <set>
#include <vector>
using namespace std;

// ROOT libs:
#include <TApplication.h>
#include <TError.h>
#include <TRandom.h>
#include <TROOT.h>

// MEGAlib libs:
#include "MDDetector.h"
#include "MDDetectorEffectsEngine.h"
#include "MDGeometry.h"
#include "MDGridPoint.h"
#include "MDGridPointCollection.h"
#include "MDGuardRing.h"
#include "MDTriggerUnit.h"
#include "MDVolumeSequence.h"
#include "MString.h"
#include "MTime.h"
#include "MVector.h"
#include "UTGeomegaShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the MEGAlib 5 detector effects engine MDDetectorEffectsEngine
class UTDDetectorEffectsEngine : public UTGeomegaBase
{
public:
  //! Default constructor
  UTDDetectorEffectsEngine();
  //! Default destructor
  virtual ~UTDDetectorEffectsEngine() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return a geometry: Strips (three wafers with guard rings, noise threshold 10, trigger threshold 30, guard ring 40), Scinti (10, 50) and Shield (noise equals trigger threshold 50)
  MString GetGeometry(const MString& StripResolution = "Strips.EnergyResolution Ideal\n") const;
  //! Scan the geometry with the additional setup lines, and report if it fails
  bool Setup(MDGeometry& Geometry, const MString& Lines, const MString& Input);
  //! Add a hit at the given world position to the engine of the geometry
  void AddHit(MDGeometry& Geometry, const MVector& Position, double Energy, double Time = 0.0, const set<unsigned int>& OriginIDs = set<unsigned int>());
  //! Add a guard ring hit to the engine of the geometry the way revan does: the volume sequence carries the guard ring as detector
  bool AddGuardRingHit(MDGeometry& Geometry, const MVector& Position, double Energy);
  //! Run the engine of the geometry with silenced streams
  bool RunEngine(MDGeometry& Geometry);
  //! Return the first collection whose volume sequence contains a volume with the given name, nullptr if there is none
  const MDGridPointCollection* FindCollection(MDGeometry& Geometry, const MString& VolumeName);
  //! Return true if the collection has a grid point with these IDs
  bool HasGridPoint(const MDGridPointCollection* Collection, unsigned int xGrid, unsigned int yGrid) const;

  //! Test the keyword which selects the engine
  bool TestKeyword();
  //! Test the validation
  bool TestValidate();
  //! Test the event time and the reset
  bool TestEventTimeAndReset();
  //! Test a single hit above and below the trigger threshold
  bool TestSingleHit();
  //! Test that hits in the same channel are merged before the thresholds are applied
  bool TestMergedHits();
  //! Test that the hits are sorted into one collection per detector volume
  bool TestCollections();
  //! Test the transformation of the world position into the detector
  bool TestPositions();
  //! Test which hits are read out after an event triggered
  bool TestReadOut();
  //! Test the read out of the neighbors of a triggered channel
  bool TestNeighbors();
  //! Test the simple veto which stops the processing early
  bool TestSimpleVeto();
  //! Test the list of the simple veto detectors
  bool TestSimpleVetoDetectors();
  //! Test guard ring hits
  bool TestGuardRing();
  //! Test that the engine can run twice on the same event
  bool TestRunTwice();
  //! Test that the noise threshold is no longer applied in the noising of the detectors
  bool TestDetectorNoise();
  //! Test a hit without detector
  bool TestHitWithoutDetector();
  //! Test channels which are blocked from triggering
  bool TestBlockedChannels();

  //! A position in the first wafer, strip (4, 4) (cm)
  MVector m_WaferA;
  //! A position in the strip (5, 4) of the first wafer (cm)
  MVector m_WaferANeighbor;
  //! A position in the strip (5, 5) of the first wafer (cm)
  MVector m_WaferADiagonal;
  //! A position in the strip (7, 4) of the first wafer (cm)
  MVector m_WaferAFar;
  //! A position in the second wafer, strip (4, 4) (cm)
  MVector m_WaferB;
  //! A position in the guard ring of the first wafer (cm)
  MVector m_GuardRingA;
  //! A position in the scintillator (cm)
  MVector m_Scinti;
  //! A position in the shield (cm)
  MVector m_Shield;
  //! A position in no detector (cm)
  MVector m_Nowhere;
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
UTDDetectorEffectsEngine::UTDDetectorEffectsEngine() :
  UTGeomegaBase("UTDDetectorEffectsEngine"),
  m_WaferA(0.1, 0.1, -20.0), m_WaferANeighbor(1.1, 0.1, -20.0), m_WaferADiagonal(1.1, 1.1, -20.0), m_WaferAFar(3.1, 0.1, -20.0),
  m_WaferB(0.1, 0.1, -10.0), m_GuardRingA(4.8, 0.1, -20.0),
  m_Scinti(0.0, 0.0, 10.0), m_Shield(0.0, 0.0, 20.0), m_Nowhere(50.0, 50.0, 50.0)
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDDetectorEffectsEngine::Run()
{
  bool Passed = true;

  // The neighbor test uses a Gaussian energy resolution
  gRandom->SetSeed(1);

  Passed = TestKeyword() && Passed;
  Passed = TestValidate() && Passed;
  Passed = TestEventTimeAndReset() && Passed;
  Passed = TestSingleHit() && Passed;
  Passed = TestMergedHits() && Passed;
  Passed = TestCollections() && Passed;
  Passed = TestPositions() && Passed;
  Passed = TestReadOut() && Passed;
  Passed = TestNeighbors() && Passed;
  Passed = TestSimpleVeto() && Passed;
  Passed = TestSimpleVetoDetectors() && Passed;
  Passed = TestGuardRing() && Passed;
  Passed = TestRunTwice() && Passed;
  Passed = TestDetectorNoise() && Passed;
  Passed = TestHitWithoutDetector() && Passed;
  Passed = TestBlockedChannels() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a geometry: Strips (three wafers with guard rings, noise threshold 10, trigger threshold 30, guard ring 40), Scinti (10, 50) and Shield (noise equals trigger threshold 50)
MString UTDDetectorEffectsEngine::GetGeometry(const MString& StripResolution) const
{
  MString Setup = GetGeometryHead();

  Setup += "DetectorEffectsEngine 2\n";

  // The wafer is 10 x 10 cm in a holder, which is placed three times, the last one rotated:
  Setup += "Volume Holder\n";
  Setup += "Holder.Material Vacuum\n";
  Setup += "Holder.Shape BRIK 6.0 6.0 1.0\n";
  Setup += "Volume Wafer\n";
  Setup += "Wafer.Material Germanium\n";
  Setup += "Wafer.Shape BRIK 5.0 5.0 0.5\n";
  Setup += "Wafer.Position 0.0 0.0 0.0\n";
  Setup += "Wafer.Mother Holder\n";
  Setup += "Holder.Copy HolderA\n";
  Setup += "HolderA.Position 0.0 0.0 -20.0\n";
  Setup += "HolderA.Mother World\n";
  Setup += "Holder.Copy HolderB\n";
  Setup += "HolderB.Position 0.0 0.0 -10.0\n";
  Setup += "HolderB.Mother World\n";
  Setup += "Holder.Copy HolderC\n";
  Setup += "HolderC.Position 0.0 0.0 -30.0\n";
  Setup += "HolderC.Rotation 0.0 0.0 90.0\n";
  Setup += "HolderC.Mother World\n";

  Setup += "Volume ScintiVolume\n";
  Setup += "ScintiVolume.Material Germanium\n";
  Setup += "ScintiVolume.Shape BRIK 5.0 5.0 0.5\n";
  Setup += "ScintiVolume.Position 0.0 0.0 10.0\n";
  Setup += "ScintiVolume.Mother World\n";

  Setup += "Volume ShieldVolume\n";
  Setup += "ShieldVolume.Material Germanium\n";
  Setup += "ShieldVolume.Shape BRIK 5.0 5.0 0.5\n";
  Setup += "ShieldVolume.Position 0.0 0.0 20.0\n";
  Setup += "ShieldVolume.Mother World\n";

  // 9 strips of pitch 1 cm and a guard ring of 0.5 cm:
  Setup += "Strip2D Strips\n";
  Setup += "Strips.DetectorVolume Wafer\n";
  Setup += "Strips.SensitiveVolume Wafer\n";
  Setup += "Strips.StructuralPitch 0.0 0.0 0.0\n";
  Setup += "Strips.StructuralOffset 0.0 0.0 0.0\n";
  Setup += "Strips.Offset 0.5 0.5\n";
  Setup += "Strips.StripNumber 9 9\n";
  Setup += "Strips.NoiseThreshold 10\n";
  Setup += "Strips.TriggerThreshold 30\n";
  Setup += StripResolution;
  Setup += "Strips.FailureRate 0.0\n";
  Setup += "Strips.GuardRingTriggerThreshold 40\n";
  Setup += "Strips.GuardRingEnergyResolution 100 0.001\n";
  Setup += "Strips.GuardRingEnergyResolution 1000 0.001\n";

  Setup += "Scintillator Scinti\n";
  Setup += "Scinti.DetectorVolume ScintiVolume\n";
  Setup += "Scinti.SensitiveVolume ScintiVolume\n";
  Setup += "Scinti.NoiseThreshold 10\n";
  Setup += "Scinti.TriggerThreshold 50\n";
  Setup += "Scinti.EnergyResolution Ideal\n";
  Setup += "Scinti.FailureRate 0.0\n";

  Setup += "Scintillator Shield\n";
  Setup += "Shield.DetectorVolume ShieldVolume\n";
  Setup += "Shield.SensitiveVolume ShieldVolume\n";
  Setup += "Shield.NoiseThresholdEqualsTriggerThreshold true\n";
  Setup += "Shield.TriggerThreshold 50\n";
  Setup += "Shield.EnergyResolution Ideal\n";
  Setup += "Shield.FailureRate 0.0\n";

  return Setup;
}


////////////////////////////////////////////////////////////////////////////////


//! Scan the geometry with the additional setup lines, and report if it fails
bool UTDDetectorEffectsEngine::Setup(MDGeometry& Geometry, const MString& Lines, const MString& Input)
{
  return EvaluateTrue("ScanSetupFile()", Input, "The geometry of the test can be read", ScanGeometry(Geometry, GetGeometry() + Lines));
}


////////////////////////////////////////////////////////////////////////////////


//! Add a hit at the given world position to the engine of the geometry
void UTDDetectorEffectsEngine::AddHit(MDGeometry& Geometry, const MVector& Position, double Energy, double Time, const set<unsigned int>& OriginIDs)
{
  MDVolumeSequence Sequence = Geometry.GetVolumeSequence(Position, false, true);
  Geometry.GetDetectorEffectsEngine()->AddHit(Position, Energy, Time, OriginIDs, Sequence);
}


////////////////////////////////////////////////////////////////////////////////


//! Add a guard ring hit to the engine of the geometry the way revan does: the volume sequence carries the guard ring as detector
bool UTDDetectorEffectsEngine::AddGuardRingHit(MDGeometry& Geometry, const MVector& Position, double Energy)
{
  MDVolumeSequence Sequence = Geometry.GetVolumeSequence(Position, false, true);
  if (Sequence.GetDetector() == nullptr || Sequence.GetDetector()->HasGuardRing() == false) {
    return false;
  }
  Sequence.SetDetector(Sequence.GetDetector()->GetGuardRing());
  Geometry.GetDetectorEffectsEngine()->AddHit(Position, Energy, 0.0, set<unsigned int>(), Sequence);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Run the engine of the geometry with silenced streams
bool UTDDetectorEffectsEngine::RunEngine(MDGeometry& Geometry)
{
  DisableDefaultStreams();
  bool Result = Geometry.GetDetectorEffectsEngine()->Run();
  EnableDefaultStreams();

  return Result;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the first collection whose volume sequence contains a volume with the given name, nullptr if there is none
const MDGridPointCollection* UTDDetectorEffectsEngine::FindCollection(MDGeometry& Geometry, const MString& VolumeName)
{
  const vector<MDGridPointCollection>& Collections = Geometry.GetDetectorEffectsEngine()->GetGridPointCollections();
  for (unsigned int c = 0; c < Collections.size(); ++c) {
    const MDVolumeSequence& Sequence = Collections[c].GetVolumeSequence();
    for (unsigned int v = 0; v < Sequence.GetNVolumes(); ++v) {
      if (Sequence.GetVolumeAt(v)->GetName() == VolumeName) {
        return &Collections[c];
      }
    }
  }

  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the collection has a grid point with these IDs
bool UTDDetectorEffectsEngine::HasGridPoint(const MDGridPointCollection* Collection, unsigned int xGrid, unsigned int yGrid) const
{
  if (Collection == nullptr) {
    return false;
  }
  for (unsigned int p = 0; p < Collection->GetNGridPoints(); ++p) {
    MDGridPoint Point = Collection->GetGridPointAt(p);
    if (Point.GetXGrid() == xGrid && Point.GetYGrid() == yGrid) {
      return true;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the keyword which selects the engine
bool UTDDetectorEffectsEngine::TestKeyword()
{
  bool Passed = true;

  // The default is the old engine
  MString Default = GetGeometry();
  Default.ReplaceAllInPlace("DetectorEffectsEngine 2\n", "");
  {
    MDGeometry Geometry;
    Passed = EvaluateTrue("ScanSetupFile()", "no keyword", "The geometry is accepted", ScanGeometry(Geometry, Default)) && Passed;
    Passed = Evaluate("GetDetectorEffectsEngineVersion()", "no keyword", "The geometry uses the engine 1 by default", Geometry.GetDetectorEffectsEngineVersion(), 1U) && Passed;
    if (EvaluateTrue("GetDetector()", "no keyword", "The strip detector exists", Geometry.GetDetector("Strips") != nullptr) == true) {
      Passed = Evaluate("GetDetectorEffectsEngineVersion()", "no keyword", "The detector uses the engine 1 by default", Geometry.GetDetector("Strips")->GetDetectorEffectsEngineVersion(), 1U) && Passed;
    } else {
      Passed = false;
    }
  }

  {
    MDGeometry Geometry;
    Passed = EvaluateTrue("ScanSetupFile()", "DetectorEffectsEngine 1", "The geometry is accepted", ScanGeometry(Geometry, Default + "DetectorEffectsEngine 1\n")) && Passed;
    Passed = Evaluate("GetDetectorEffectsEngineVersion()", "DetectorEffectsEngine 1", "The geometry uses the engine 1", Geometry.GetDetectorEffectsEngineVersion(), 1U) && Passed;
  }

  {
    MDGeometry Geometry;
    if (Setup(Geometry, "", "DetectorEffectsEngine 2") == false) {
      return false;
    }
    Passed = Evaluate("GetDetectorEffectsEngineVersion()", "DetectorEffectsEngine 2", "The geometry uses the engine 2", Geometry.GetDetectorEffectsEngineVersion(), 2U) && Passed;
    Passed = EvaluateTrue("GetDetectorEffectsEngine()", "DetectorEffectsEngine 2", "The geometry has an engine", Geometry.GetDetectorEffectsEngine() != nullptr) && Passed;
    for (unsigned int d = 0; d < Geometry.GetNDetectors(); ++d) {
      if (Geometry.GetDetectorAt(d)->GetType() == MDDetector::c_GuardRing) {
        continue;
      }
      Passed = Evaluate("GetDetectorEffectsEngineVersion()", Geometry.GetDetectorAt(d)->GetName(), "Each detector uses the engine 2", Geometry.GetDetectorAt(d)->GetDetectorEffectsEngineVersion(), 2U) && Passed;
    }
    if (EvaluateTrue("GetGuardRing()", "DetectorEffectsEngine 2", "The strip detector has a guard ring", Geometry.GetDetector("Strips") != nullptr && Geometry.GetDetector("Strips")->GetGuardRing() != nullptr) == true) {
      // GEO-E-35: The guard ring does not get the version of the geometry
      Passed = ExpectedToFail(Evaluate("GetDetectorEffectsEngineVersion()", "guard ring", "The guard ring uses the engine 2", Geometry.GetDetector("Strips")->GetGuardRing()->GetDetectorEffectsEngineVersion(), 2U)) && Passed;
    } else {
      Passed = false;
    }

    // A second scan without the keyword goes back to the default
    Passed = EvaluateTrue("ScanSetupFile()", "second scan without keyword", "The geometry is accepted", ScanGeometry(Geometry, Default)) && Passed;
    Passed = Evaluate("GetDetectorEffectsEngineVersion()", "second scan without keyword", "The geometry is back to the engine 1", Geometry.GetDetectorEffectsEngineVersion(), 1U) && Passed;
  }

  // Invalid values
  vector<MString> Invalid = { "DetectorEffectsEngine\n", "DetectorEffectsEngine 0\n", "DetectorEffectsEngine 3\n", "DetectorEffectsEngine 1 2\n", "DetectorEffectsEngine -1\n", "DetectorEffectsEngine two\n" };
  for (unsigned int i = 0; i < Invalid.size(); ++i) {
    MDGeometry Geometry;
    Passed = EvaluateFalse("ScanSetupFile()", Invalid[i], "The geometry is rejected", ScanGeometry(Geometry, Default + Invalid[i])) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the validation
bool UTDDetectorEffectsEngine::TestValidate()
{
  bool Passed = true;

  MDDetectorEffectsEngine Engine;
  DisableDefaultStreams();
  bool Valid = Engine.Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "no geometry", "An engine without geometry is not valid", Valid) && Passed;

  MDGeometry Geometry;
  if (Setup(Geometry, "", "validate") == false) {
    return false;
  }
  Engine.SetGeometry(&Geometry);
  Passed = EvaluateTrue("Validate()", "geometry", "An engine with geometry is valid", Engine.Validate()) && Passed;
  Passed = EvaluateTrue("Validate()", "engine of the geometry", "The engine of a geometry is valid", Geometry.GetDetectorEffectsEngine()->Validate()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the event time and the reset
bool UTDDetectorEffectsEngine::TestEventTimeAndReset()
{
  bool Passed = true;

  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "event time") == false) {
    return false;
  }
  MDDetectorEffectsEngine* Engine = Geometry.GetDetectorEffectsEngine();

  MTime Time(12.5);
  Engine->Reset();
  Engine->AddEventTime(Time);
  Passed = EvaluateNear("GetEventTime()", "12.5 s", "The event time is stored", Engine->GetEventTime().GetAsDouble(), 12.5, 1e-9) && Passed;
  AddHit(Geometry, m_Scinti, 100.0);
  Passed = EvaluateTrue("Run()", "100 keV in Scinti", "The event triggers", RunEngine(Geometry)) && Passed;
  Passed = EvaluateNear("GetEventTime()", "after the run, ideal time resolution of the system", "The event time is not changed", Engine->GetEventTime().GetAsDouble(), 12.5, 1e-9) && Passed;
  Passed = EvaluateSize("GetGridPointCollections()", "after the run", "There is one collection", Engine->GetGridPointCollections().size(), 1) && Passed;

  Engine->Reset();
  Passed = EvaluateNear("GetEventTime()", "after the reset", "The event time is zero", Engine->GetEventTime().GetAsDouble(), 0.0, 1e-15) && Passed;
  Passed = EvaluateSize("GetGridPointCollections()", "after the reset", "There is no collection", Engine->GetGridPointCollections().size(), 0) && Passed;
  Passed = EvaluateFalse("Run()", "after the reset", "An event without hits does not trigger", RunEngine(Geometry)) && Passed;
  Passed = EvaluateSize("GetGridPointCollections()", "event without hits", "There is no collection", Engine->GetGridPointCollections().size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a single hit above and below the trigger threshold
bool UTDDetectorEffectsEngine::TestSingleHit()
{
  bool Passed = true;

  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "single hit") == false) {
    return false;
  }
  MDDetectorEffectsEngine* Engine = Geometry.GetDetectorEffectsEngine();

  Engine->Reset();
  AddHit(Geometry, m_Scinti, 50.001, 3.5, { 2, 7 });
  Passed = EvaluateTrue("Run()", "50.001 keV, threshold 50 keV", "A hit just above the trigger threshold triggers", RunEngine(Geometry)) && Passed;
  if (EvaluateSize("GetGridPointCollections()", "50.001 keV", "There is one collection", Engine->GetGridPointCollections().size(), 1) == false) {
    return false;
  }
  const MDGridPointCollection& Collection = Engine->GetGridPointCollections()[0];
  Passed = EvaluateTrue("GetVolumeSequence()", "50.001 keV", "The collection has the detector of the hit", Collection.GetVolumeSequence().GetDetector() == Geometry.GetDetector("Scinti")) && Passed;
  if (EvaluateSize("GetNGridPoints()", "50.001 keV", "The collection has one grid point", Collection.GetNGridPoints(), 1) == false) {
    return false;
  }
  MDGridPoint Point = Collection.GetGridPointAt(0);
  Passed = EvaluateNear("GetEnergy()", "50.001 keV", "The energy is kept with an ideal energy resolution", Point.GetEnergy(), 50.001, 1e-12) && Passed;
  // Attention: A detector without time resolution sets the time to zero during the noising
  Passed = EvaluateNear("GetTime()", "3.5 s, detector without time resolution", "The time is set to zero", Point.GetTime(), 0.0, 1e-15) && Passed;
  Passed = EvaluateTrue("GetOriginIDs()", "origins 2 and 7", "The origins are kept", Point.GetOriginIDs() == set<unsigned int>({ 2, 7 })) && Passed;
  Passed = Evaluate("GetType()", "scintillator", "The grid point is a voxel", Point.GetType(), static_cast<unsigned int>(MDGridPoint::c_Voxel)) && Passed;
  Passed = EvaluateTrue("IsAboveTriggerThreshold()", "50.001 keV", "The grid point is above the trigger threshold", Point.IsAboveTriggerThreshold()) && Passed;
  Passed = EvaluateTrue("IsReadOut()", "50.001 keV", "The grid point is read out", Point.IsReadOut()) && Passed;
  Passed = EvaluateVectorNear("GetWorldPositionGridPointAt()", "scintillator", "The world position is the center of the scintillator", Collection.GetWorldPositionGridPointAt(0), MVector(0.0, 0.0, 10.0), 1e-12) && Passed;

  Engine->Reset();
  AddHit(Geometry, m_Scinti, 49.999, 3.5, { 2, 7 });
  Passed = EvaluateFalse("Run()", "49.999 keV, threshold 50 keV", "A hit just below the trigger threshold does not trigger", RunEngine(Geometry)) && Passed;
  Passed = EvaluateSize("GetGridPointCollections()", "49.999 keV", "An event which did not trigger has no collections", Engine->GetGridPointCollections().size(), 0) && Passed;

  // A hit in a detector which is in no trigger
  Engine->Reset();
  AddHit(Geometry, m_WaferA, 1000.0);
  Passed = EvaluateFalse("Run()", "1000 keV in a detector without trigger", "The event does not trigger", RunEngine(Geometry)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that hits in the same channel are merged before the thresholds are applied
bool UTDDetectorEffectsEngine::TestMergedHits()
{
  bool Passed = true;

  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
  Lines += "Trigger S\nS.Veto false\nS.TriggerByChannel true\nS.Detector Strips 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "merged hits") == false) {
    return false;
  }
  MDDetectorEffectsEngine* Engine = Geometry.GetDetectorEffectsEngine();

  // Two hits below the threshold of 50 keV in the one channel of the scintillator
  Engine->Reset();
  AddHit(Geometry, m_Scinti, 30.0, 5.0, { 1 });
  AddHit(Geometry, MVector(2.0, -3.0, 10.2), 25.0, 4.0, { 2, 3 });
  Passed = EvaluateTrue("Run()", "30 keV and 25 keV in one channel, threshold 50 keV", "The sum of the hits triggers", RunEngine(Geometry)) && Passed;
  if (EvaluateSize("GetGridPointCollections()", "two hits in one channel", "There is one collection", Engine->GetGridPointCollections().size(), 1) == false) {
    return false;
  }
  if (EvaluateSize("GetNGridPoints()", "two hits in one channel", "The collection has one grid point", Engine->GetGridPointCollections()[0].GetNGridPoints(), 1) == false) {
    return false;
  }
  MDGridPoint Point = Engine->GetGridPointCollections()[0].GetGridPointAt(0);
  Passed = EvaluateNear("GetEnergy()", "30 keV and 25 keV", "The energies are added", Point.GetEnergy(), 55.0, 1e-12) && Passed;
  Passed = EvaluateTrue("GetOriginIDs()", "origins 1 and 2, 3", "The origins are merged", Point.GetOriginIDs() == set<unsigned int>({ 1, 2, 3 })) && Passed;
  Passed = Evaluate("GetHits()", "two hits in one channel", "The grid point counts two hits", Point.GetHits(), 2U) && Passed;

  // Two hits in the same strip crossing, each below the threshold of 30 keV
  Engine->Reset();
  AddHit(Geometry, m_WaferA, 20.0);
  AddHit(Geometry, m_WaferA + MVector(0.3, -0.2, 0.1), 20.0);
  Passed = EvaluateTrue("Run()", "20 keV twice in one strip crossing, threshold 30 keV", "The sum of the hits triggers", RunEngine(Geometry)) && Passed;
  if (EvaluateSize("GetGridPointCollections()", "two hits in one strip crossing", "There is one collection", Engine->GetGridPointCollections().size(), 1) == true) {
    Passed = EvaluateSize("GetNGridPoints()", "two hits in one strip crossing", "The collection has one grid point", Engine->GetGridPointCollections()[0].GetNGridPoints(), 1) && Passed;
    if (Engine->GetGridPointCollections()[0].GetNGridPoints() == 1) {
      Passed = EvaluateNear("GetEnergy()", "two hits in one strip crossing", "The energies are added", Engine->GetGridPointCollections()[0].GetGridPointAt(0).GetEnergy(), 40.0, 1e-12) && Passed;
    }
  } else {
    Passed = false;
  }

  // The same in two different strip crossings does not trigger
  Engine->Reset();
  AddHit(Geometry, m_WaferA, 20.0);
  AddHit(Geometry, m_WaferANeighbor, 20.0);
  Passed = EvaluateFalse("Run()", "20 keV in two neighboring strip crossings, threshold 30 keV", "The hits are not merged and do not trigger", RunEngine(Geometry)) && Passed;

  // The same in the same strip crossing of two wafers does not trigger
  Engine->Reset();
  AddHit(Geometry, m_WaferA, 20.0);
  AddHit(Geometry, m_WaferB, 20.0);
  Passed = EvaluateFalse("Run()", "20 keV in the same strip crossing of two wafers, threshold 30 keV", "The hits are not merged and do not trigger", RunEngine(Geometry)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the hits are sorted into one collection per detector volume
bool UTDDetectorEffectsEngine::TestCollections()
{
  bool Passed = true;

  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
  Lines += "Trigger S\nS.Veto false\nS.TriggerByChannel true\nS.Detector Strips 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "collections") == false) {
    return false;
  }
  MDDetectorEffectsEngine* Engine = Geometry.GetDetectorEffectsEngine();

  Engine->Reset();
  AddHit(Geometry, m_WaferA, 100.0);
  AddHit(Geometry, m_WaferB, 110.0);
  AddHit(Geometry, m_Scinti, 120.0);
  AddHit(Geometry, m_WaferAFar, 130.0);
  AddHit(Geometry, m_WaferB, 140.0);
  Passed = EvaluateTrue("Run()", "five hits in three detector volumes", "The event triggers", RunEngine(Geometry)) && Passed;
  Passed = EvaluateSize("GetGridPointCollections()", "five hits in three detector volumes", "There are three collections", Engine->GetGridPointCollections().size(), 3) && Passed;

  const MDGridPointCollection* A = FindCollection(Geometry, "HolderA");
  const MDGridPointCollection* B = FindCollection(Geometry, "HolderB");
  const MDGridPointCollection* S = FindCollection(Geometry, "ScintiVolume");
  if (EvaluateTrue("GetGridPointCollections()", "five hits in three detector volumes", "Each detector volume has a collection", A != nullptr && B != nullptr && S != nullptr) == false) {
    return false;
  }
  Passed = EvaluateSize("GetNGridPoints()", "first wafer", "The collection has the two strip crossings", A->GetNGridPoints(), 2) && Passed;
  Passed = EvaluateTrue("GetGridPointAt()", "first wafer", "The collection has the strip crossing (4, 4)", HasGridPoint(A, 4, 4)) && Passed;
  Passed = EvaluateTrue("GetGridPointAt()", "first wafer", "The collection has the strip crossing (7, 4)", HasGridPoint(A, 7, 4)) && Passed;
  Passed = EvaluateSize("GetNGridPoints()", "second wafer", "The collection has the one strip crossing", B->GetNGridPoints(), 1) && Passed;
  Passed = EvaluateSize("GetNGridPoints()", "scintillator", "The collection has one grid point", S->GetNGridPoints(), 1) && Passed;

  // GetEnergy() is not const:
  MDGridPointCollection CopyA = *A;
  MDGridPointCollection CopyB = *B;
  MDGridPointCollection CopyS = *S;
  Passed = EvaluateNear("GetEnergy()", "first wafer", "The collection has the energy of its hits", CopyA.GetEnergy(), 230.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetEnergy()", "second wafer", "The collection has the energy of its hits", CopyB.GetEnergy(), 250.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetEnergy()", "scintillator", "The collection has the energy of its hits", CopyS.GetEnergy(), 120.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the transformation of the world position into the detector
bool UTDDetectorEffectsEngine::TestPositions()
{
  bool Passed = true;

  MString Lines = "Trigger S\nS.Veto false\nS.TriggerByChannel true\nS.Detector Strips 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "positions") == false) {
    return false;
  }
  MDDetectorEffectsEngine* Engine = Geometry.GetDetectorEffectsEngine();

  // The strips have a pitch of 1 cm and start at -4.5 cm: x = 2.1 cm is in strip 6 with its center at 2.0 cm, y = -3.2 cm in strip 1 with its center at -3.0 cm
  Engine->Reset();
  AddHit(Geometry, MVector(2.1, -3.2, -10.2), 100.0);
  Passed = EvaluateTrue("Run()", "translated wafer", "The event triggers", RunEngine(Geometry)) && Passed;
  if (EvaluateSize("GetGridPointCollections()", "translated wafer", "There is one collection", Engine->GetGridPointCollections().size(), 1) == true &&
      EvaluateSize("GetNGridPoints()", "translated wafer", "The collection has one grid point", Engine->GetGridPointCollections()[0].GetNGridPoints(), 1) == true) {
    const MDGridPointCollection& Collection = Engine->GetGridPointCollections()[0];
    Passed = Evaluate("GetXGrid()", "x = 2.1 cm", "The hit is in x strip 6", Collection.GetGridPointAt(0).GetXGrid(), 6U) && Passed;
    Passed = Evaluate("GetYGrid()", "y = -3.2 cm", "The hit is in y strip 1", Collection.GetGridPointAt(0).GetYGrid(), 1U) && Passed;
    Passed = EvaluateVectorNear("GetWorldPositionGridPointAt()", "translated wafer", "The world position is the center of the strip crossing", Collection.GetWorldPositionGridPointAt(0), MVector(2.0, -3.0, -10.0), 1e-12) && Passed;
  } else {
    Passed = false;
  }

  // The third wafer is rotated by 90 degrees around z: the world position must still be the center of the closest strip crossing, but the strips are different ones
  Engine->Reset();
  AddHit(Geometry, MVector(2.1, -3.2, -30.2), 100.0);
  Passed = EvaluateTrue("Run()", "rotated wafer", "The event triggers", RunEngine(Geometry)) && Passed;
  if (EvaluateSize("GetGridPointCollections()", "rotated wafer", "There is one collection", Engine->GetGridPointCollections().size(), 1) == true &&
      EvaluateSize("GetNGridPoints()", "rotated wafer", "The collection has one grid point", Engine->GetGridPointCollections()[0].GetNGridPoints(), 1) == true) {
    const MDGridPointCollection& Collection = Engine->GetGridPointCollections()[0];
    Passed = EvaluateVectorNear("GetWorldPositionGridPointAt()", "rotated wafer", "The world position is the center of the strip crossing", Collection.GetWorldPositionGridPointAt(0), MVector(2.0, -3.0, -30.0), 1e-12) && Passed;
    MVector InWafer = Geometry.GetVolumeSequence(MVector(2.1, -3.2, -30.2), false, true).GetPositionInSensitiveVolume();
    Passed = Evaluate("GetXGrid()", "rotated wafer", "The x strip follows from the position in the wafer", Collection.GetGridPointAt(0).GetXGrid(), static_cast<unsigned int>(floor(InWafer.X() + 4.5))) && Passed;
    Passed = Evaluate("GetYGrid()", "rotated wafer", "The y strip follows from the position in the wafer", Collection.GetGridPointAt(0).GetYGrid(), static_cast<unsigned int>(floor(InWafer.Y() + 4.5))) && Passed;
    Passed = EvaluateFalse("GetXGrid()", "rotated wafer", "The strips are not those of the unrotated wafer", Collection.GetGridPointAt(0).GetXGrid() == 6 && Collection.GetGridPointAt(0).GetYGrid() == 1) && Passed;
  } else {
    Passed = false;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test which hits are read out after an event triggered
bool UTDDetectorEffectsEngine::TestReadOut()
{
  bool Passed = true;

  // Each detector needs a trigger: only hits which a trigger takes count as above the trigger threshold
  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
  Lines += "Trigger S\nS.Veto false\nS.TriggerByChannel true\nS.Detector Strips 1\n";
  Lines += "Trigger H\nH.Veto false\nH.TriggerByChannel true\nH.Detector Shield 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "read out") == false) {
    return false;
  }
  MDDetectorEffectsEngine* Engine = Geometry.GetDetectorEffectsEngine();

  // The scintillator triggers. The strip hits are below their trigger threshold of 30 keV, but are read out above their noise threshold of 10 keV
  Engine->Reset();
  AddHit(Geometry, m_Scinti, 100.0);
  AddHit(Geometry, m_WaferA, 10.001);
  AddHit(Geometry, m_WaferAFar, 9.999);
  AddHit(Geometry, m_WaferB, 9.999);
  Passed = EvaluateTrue("Run()", "trigger in the scintillator, strip hits around the noise threshold", "The event triggers", RunEngine(Geometry)) && Passed;
  Passed = EvaluateSize("GetGridPointCollections()", "strip hits around the noise threshold", "The wafer without read out has no collection", Engine->GetGridPointCollections().size(), 2) && Passed;
  const MDGridPointCollection* A = FindCollection(Geometry, "HolderA");
  if (EvaluateTrue("GetGridPointCollections()", "strip hits around the noise threshold", "The first wafer has a collection", A != nullptr) == true) {
    Passed = EvaluateSize("GetNGridPoints()", "10.001 keV and 9.999 keV, noise threshold 10 keV", "Only the hit above the noise threshold is read out", A->GetNGridPoints(), 1) && Passed;
    Passed = EvaluateTrue("GetGridPointAt()", "10.001 keV", "The hit above the noise threshold is read out", HasGridPoint(A, 4, 4)) && Passed;
    if (A->GetNGridPoints() == 1) {
      Passed = EvaluateFalse("IsAboveTriggerThreshold()", "10.001 keV, trigger threshold 30 keV", "The hit is not above the trigger threshold", A->GetGridPointAt(0).IsAboveTriggerThreshold()) && Passed;
      Passed = EvaluateTrue("IsReadOut()", "10.001 keV", "The hit is read out", A->GetGridPointAt(0).IsReadOut()) && Passed;
      Passed = EvaluateNear("GetEnergy()", "10.001 keV", "The energy is kept", A->GetGridPointAt(0).GetEnergy(), 10.001, 1e-12) && Passed;
    }
  } else {
    Passed = false;
  }
  Passed = EvaluateTrue("GetGridPointCollections()", "9.999 keV", "The second wafer has no collection", FindCollection(Geometry, "HolderB") == nullptr) && Passed;

  // The shield has no separate noise threshold: it is only read out above its trigger threshold of 50 keV
  Engine->Reset();
  AddHit(Geometry, m_Scinti, 100.0);
  AddHit(Geometry, m_Shield, 49.999);
  Passed = EvaluateTrue("Run()", "trigger in the scintillator, 49.999 keV in the shield", "The event triggers", RunEngine(Geometry)) && Passed;
  Passed = EvaluateTrue("GetGridPointCollections()", "49.999 keV, noise threshold equals trigger threshold of 50 keV", "The shield is not read out", FindCollection(Geometry, "ShieldVolume") == nullptr) && Passed;

  Engine->Reset();
  AddHit(Geometry, m_Scinti, 100.0);
  AddHit(Geometry, m_Shield, 50.001);
  Passed = EvaluateTrue("Run()", "trigger in the scintillator, 50.001 keV in the shield", "The event triggers", RunEngine(Geometry)) && Passed;
  Passed = EvaluateTrue("GetGridPointCollections()", "50.001 keV, noise threshold equals trigger threshold of 50 keV", "The shield is read out", FindCollection(Geometry, "ShieldVolume") != nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the read out of the neighbors of a triggered channel
bool UTDDetectorEffectsEngine::TestNeighbors()
{
  bool Passed = true;

  // The neighbors need a Gaussian energy resolution: sigma is 0.001 keV
  MString Lines = "Trigger S\nS.Veto false\nS.TriggerByChannel true\nS.Detector Strips 1\n";
  MDGeometry Geometry;
  if (EvaluateTrue("ScanSetupFile()", "neighbors", "The geometry of the test can be read", ScanGeometry(Geometry, GetGeometry("Strips.EnergyResolution Gauss 1 1 0.001\nStrips.EnergyResolution Gauss 1000 1000 0.001\n") + Lines)) == false) {
    return false;
  }
  MDDetectorEffectsEngine* Engine = Geometry.GetDetectorEffectsEngine();

  // A triggered strip crossing, and hits of 8 keV below the noise threshold of 10 keV, which are 8000 sigma above the noise level
  Engine->Reset();
  AddHit(Geometry, m_WaferA, 100.0);
  AddHit(Geometry, m_WaferANeighbor, 8.0);
  AddHit(Geometry, m_WaferADiagonal, 8.0);
  AddHit(Geometry, m_WaferAFar, 8.0);
  AddHit(Geometry, MVector(2.1, 0.1, -20.0), 8.0);
  AddHit(Geometry, m_WaferB, 8.0);
  Passed = EvaluateTrue("Run()", "triggered strip crossing with hits below the noise threshold", "The event triggers", RunEngine(Geometry)) && Passed;
  const MDGridPointCollection* A = FindCollection(Geometry, "HolderA");
  if (EvaluateTrue("GetGridPointCollections()", "neighbors", "The first wafer has a collection", A != nullptr) == true) {
    Passed = EvaluateTrue("GetGridPointAt()", "triggered strip crossing (4, 4)", "The triggered strip crossing is read out", HasGridPoint(A, 4, 4)) && Passed;
    Passed = EvaluateTrue("GetGridPointAt()", "strip crossing (5, 4)", "The direct neighbor is read out", HasGridPoint(A, 5, 4)) && Passed;
    Passed = EvaluateTrue("GetGridPointAt()", "strip crossing (5, 5)", "The diagonal neighbor is read out", HasGridPoint(A, 5, 5)) && Passed;
    Passed = EvaluateFalse("GetGridPointAt()", "strip crossing (6, 4)", "The second neighbor is not read out", HasGridPoint(A, 6, 4)) && Passed;
    Passed = EvaluateFalse("GetGridPointAt()", "strip crossing (7, 4)", "A far strip crossing is not read out", HasGridPoint(A, 7, 4)) && Passed;
    Passed = EvaluateSize("GetNGridPoints()", "neighbors", "Three strip crossings are read out", A->GetNGridPoints(), 3) && Passed;
  } else {
    Passed = false;
  }
  Passed = EvaluateTrue("GetGridPointCollections()", "same strip crossing in the second wafer", "A neighbor in another wafer is not read out", FindCollection(Geometry, "HolderB") == nullptr) && Passed;

  // A neighbor less than 7 sigma above the noise level is not read out: 0.002 keV are 2 sigma
  Engine->Reset();
  AddHit(Geometry, m_WaferA, 100.0);
  AddHit(Geometry, m_WaferANeighbor, 0.002);
  Passed = EvaluateTrue("Run()", "triggered strip crossing and a neighbor with 2 sigma", "The event triggers", RunEngine(Geometry)) && Passed;
  A = FindCollection(Geometry, "HolderA");
  Passed = EvaluateTrue("GetGridPointAt()", "triggered strip crossing (4, 4)", "The triggered strip crossing is read out", HasGridPoint(A, 4, 4)) && Passed;
  Passed = EvaluateFalse("GetGridPointAt()", "neighbor with 2 sigma", "The neighbor is not read out", HasGridPoint(A, 5, 4)) && Passed;

  // Without a triggered strip crossing next to it nothing below the noise threshold is read out
  Engine->Reset();
  AddHit(Geometry, m_WaferB, 100.0);
  AddHit(Geometry, m_WaferANeighbor, 8.0);
  Passed = EvaluateTrue("Run()", "trigger in the second wafer, 8 keV in the first", "The event triggers", RunEngine(Geometry)) && Passed;
  Passed = EvaluateTrue("GetGridPointCollections()", "trigger in the second wafer, 8 keV in the first", "The first wafer is not read out", FindCollection(Geometry, "HolderA") == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the simple veto which stops the processing early
bool UTDDetectorEffectsEngine::TestSimpleVeto()
{
  bool Passed = true;

  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
  Lines += "Trigger V\nV.Veto true\nV.TriggerByChannel true\nV.Detector Shield 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "simple veto") == false) {
    return false;
  }
  MDDetectorEffectsEngine* Engine = Geometry.GetDetectorEffectsEngine();

  Engine->Reset();
  AddHit(Geometry, m_Scinti, 100.0);
  AddHit(Geometry, m_Shield, 50.001);
  Passed = EvaluateFalse("Run()", "trigger and 50.001 keV in the veto shield", "The event is vetoed", RunEngine(Geometry)) && Passed;
  Passed = EvaluateSize("GetGridPointCollections()", "vetoed event", "A vetoed event has no collections", Engine->GetGridPointCollections().size(), 0) && Passed;

  Engine->Reset();
  AddHit(Geometry, m_Scinti, 100.0);
  AddHit(Geometry, m_Shield, 49.999);
  Passed = EvaluateTrue("Run()", "trigger and 49.999 keV in the veto shield", "The event is not vetoed", RunEngine(Geometry)) && Passed;
  Passed = EvaluateSize("GetGridPointCollections()", "trigger and 49.999 keV in the veto shield", "Only the scintillator is read out", Engine->GetGridPointCollections().size(), 1) && Passed;

  // The shield hit is split in two below the threshold: the simple veto looks at the single hits, the full processing at the channel
  Engine->Reset();
  AddHit(Geometry, m_Scinti, 100.0);
  AddHit(Geometry, m_Shield, 30.0);
  AddHit(Geometry, m_Shield, 30.0);
  Passed = EvaluateFalse("Run()", "trigger and two hits of 30 keV in the veto shield, threshold 50 keV", "The event is vetoed by the sum of the hits", RunEngine(Geometry)) && Passed;

  // Only a veto: no trigger
  Engine->Reset();
  AddHit(Geometry, m_Shield, 100.0);
  Passed = EvaluateFalse("Run()", "veto without trigger", "The event does not trigger", RunEngine(Geometry)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the list of the simple veto detectors
bool UTDDetectorEffectsEngine::TestSimpleVetoDetectors()
{
  bool Passed = true;

  {
    MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
    Lines += "Trigger V\nV.Veto true\nV.TriggerByChannel true\nV.Detector Shield 1\n";
    MDGeometry Geometry;
    if (Setup(Geometry, Lines, "simple veto detectors") == false) {
      return false;
    }
    vector<MDDetector*> Detectors = Geometry.GetTriggerUnit()->GetSimpleVetoDetectors();
    if (EvaluateSize("GetSimpleVetoDetectors()", "veto on the shield, trigger on the scintillator", "There is one simple veto detector", Detectors.size(), 1) == true) {
      Passed = EvaluateTrue("GetSimpleVetoDetectors()", "veto on the shield, trigger on the scintillator", "The simple veto detector is the shield", Detectors[0] == Geometry.GetDetector("Shield")) && Passed;
    } else {
      Passed = false;
    }
  }

  {
    MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
    MDGeometry Geometry;
    if (Setup(Geometry, Lines, "no veto") == false) {
      return false;
    }
    Passed = EvaluateSize("GetSimpleVetoDetectors()", "no veto", "There is no simple veto detector", Geometry.GetTriggerUnit()->GetSimpleVetoDetectors().size(), 0) && Passed;
  }

  {
    MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
    Lines += "Trigger S\nS.Veto false\nS.TriggerByChannel true\nS.Detector Strips 2\n";
    Lines += "Trigger V\nV.Veto true\nV.TriggerByChannel true\nV.Detector Strips 1\n";
    MDGeometry Geometry;
    if (Setup(Geometry, Lines, "veto on strips") == false) {
      return false;
    }
    Passed = EvaluateSize("GetSimpleVetoDetectors()", "veto on a strip detector", "A strip detector is no simple veto detector", Geometry.GetTriggerUnit()->GetSimpleVetoDetectors().size(), 0) && Passed;
  }

  {
    MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
    Lines += "Trigger V\nV.Veto true\nV.TriggerByChannel true\nV.DetectorType Scintillator 1\n";
    MDGeometry Geometry;
    if (Setup(Geometry, Lines, "veto on detector type") == false) {
      return false;
    }
    Passed = EvaluateSize("GetSimpleVetoDetectors()", "veto on a detector type", "A veto on a detector type is no simple veto", Geometry.GetTriggerUnit()->GetSimpleVetoDetectors().size(), 0) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test guard ring hits
bool UTDDetectorEffectsEngine::TestGuardRing()
{
  bool Passed = true;

  MString Lines = "Trigger S\nS.Veto false\nS.TriggerByChannel true\nS.Detector Strips 1\n";
  Lines += "Trigger G\nG.Veto true\nG.TriggerByChannel true\nG.GuardRingDetector Strips 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "guard ring") == false) {
    return false;
  }
  MDDetectorEffectsEngine* Engine = Geometry.GetDetectorEffectsEngine();

  // The guard ring has a trigger threshold of 40 keV and a resolution of 0.001 keV
  Engine->Reset();
  AddHit(Geometry, m_WaferA, 100.0);
  if (EvaluateTrue("AddHit()", "guard ring position", "The detector at the position has a guard ring", AddGuardRingHit(Geometry, m_GuardRingA, 41.0)) == false) {
    return false;
  }
  Passed = EvaluateFalse("Run()", "strip hit and 41 keV in the guard ring, threshold 40 keV", "The guard ring vetoes the event", RunEngine(Geometry)) && Passed;
  Passed = EvaluateSize("GetGridPointCollections()", "vetoed event", "A vetoed event has no collections", Engine->GetGridPointCollections().size(), 0) && Passed;

  Engine->Reset();
  AddHit(Geometry, m_WaferA, 100.0);
  AddGuardRingHit(Geometry, m_GuardRingA, 39.0);
  // GEO-E-35: The guard ring still uses the engine 1, where each hit which survived the noising counts as above the trigger threshold
  Passed = ExpectedToFail(EvaluateTrue("Run()", "strip hit and 39 keV in the guard ring, threshold 40 keV", "The guard ring does not veto the event", RunEngine(Geometry))) && Passed;
  Passed = ExpectedToFail(EvaluateSize("GetGridPointCollections()", "strip hit and 39 keV in the guard ring", "The guard ring below its threshold is not read out", Engine->GetGridPointCollections().size(), 1)) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("GetGridPointAt()", "strip hit and 39 keV in the guard ring", "The strip hit is read out", HasGridPoint(FindCollection(Geometry, "HolderA"), 4, 4))) && Passed;

  // The guard ring of the other wafer vetoes too
  Engine->Reset();
  AddHit(Geometry, m_WaferA, 100.0);
  AddGuardRingHit(Geometry, MVector(4.8, 0.1, -10.0), 41.0);
  Passed = EvaluateFalse("Run()", "strip hit and 41 keV in the guard ring of the other wafer", "The guard ring vetoes the event", RunEngine(Geometry)) && Passed;

  // Two guard ring hits below the threshold in one guard ring add up
  Engine->Reset();
  AddHit(Geometry, m_WaferA, 100.0);
  AddGuardRingHit(Geometry, m_GuardRingA, 25.0);
  AddGuardRingHit(Geometry, MVector(-4.8, 0.1, -20.0), 25.0);
  Passed = EvaluateFalse("Run()", "strip hit and two hits of 25 keV in one guard ring, threshold 40 keV", "The sum of the guard ring hits vetoes the event", RunEngine(Geometry)) && Passed;

  // ... but not in the guard rings of two wafers
  Engine->Reset();
  AddHit(Geometry, m_WaferA, 100.0);
  AddGuardRingHit(Geometry, m_GuardRingA, 25.0);
  AddGuardRingHit(Geometry, MVector(4.8, 0.1, -10.0), 25.0);
  // GEO-E-35 again
  Passed = ExpectedToFail(EvaluateTrue("Run()", "strip hit and 25 keV in the guard rings of two wafers, threshold 40 keV", "The guard rings do not veto the event", RunEngine(Geometry))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the engine can run twice on the same event
bool UTDDetectorEffectsEngine::TestRunTwice()
{
  bool Passed = true;

  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
  Lines += "Trigger V\nV.Veto true\nV.TriggerByChannel true\nV.Detector Shield 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "run twice") == false) {
    return false;
  }
  MDDetectorEffectsEngine* Engine = Geometry.GetDetectorEffectsEngine();

  Engine->Reset();
  AddHit(Geometry, m_Scinti, 100.0);
  AddHit(Geometry, m_WaferA, 20.0);
  Passed = EvaluateTrue("Run()", "first run", "The event triggers", RunEngine(Geometry)) && Passed;
  Passed = EvaluateTrue("Run()", "second run without reset", "The event triggers again", RunEngine(Geometry)) && Passed;
  Passed = EvaluateSize("GetGridPointCollections()", "second run without reset", "The collections are not doubled", Engine->GetGridPointCollections().size(), 2) && Passed;
  const MDGridPointCollection* S = FindCollection(Geometry, "ScintiVolume");
  if (EvaluateTrue("GetGridPointCollections()", "second run without reset", "The scintillator has a collection", S != nullptr && S->GetNGridPoints() == 1) == true) {
    Passed = EvaluateNear("GetEnergy()", "second run without reset", "The energy is not doubled", S->GetGridPointAt(0).GetEnergy(), 100.0, 1e-12) && Passed;
  } else {
    Passed = false;
  }

  // A hit which is added later and vetoes the event: the collections of the first run must not survive
  AddHit(Geometry, m_Shield, 100.0);
  Passed = EvaluateFalse("Run()", "third run with an added veto hit", "The event is vetoed", RunEngine(Geometry)) && Passed;
  // GEO-E-37: The early return of the simple veto does not clear the collections
  Passed = ExpectedToFail(EvaluateSize("GetGridPointCollections()", "third run with an added veto hit", "A vetoed event has no collections", Engine->GetGridPointCollections().size(), 0)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the noise threshold is no longer applied in the noising of the detectors
bool UTDDetectorEffectsEngine::TestDetectorNoise()
{
  bool Passed = true;

  MString Default = GetGeometry();
  Default.ReplaceAllInPlace("DetectorEffectsEngine 2\n", "");

  // 5 keV in a strip detector with a noise threshold of 10 keV
  for (unsigned int Version = 1; Version <= 2; ++Version) {
    MDGeometry Geometry;
    MString Input = MString("DetectorEffectsEngine ") + Version;
    if (EvaluateTrue("ScanSetupFile()", Input, "The geometry of the test can be read", ScanGeometry(Geometry, Default + Input + "\n")) == false) {
      return false;
    }
    vector<MString> Names = { "Strips", "Scinti" };
    for (unsigned int n = 0; n < Names.size(); ++n) {
      MDDetector* Detector = Geometry.GetDetector(Names[n]);
      if (EvaluateTrue("GetDetector()", Names[n], "The detector exists", Detector != nullptr) == false) {
        return false;
      }
      MVector Position(0.1, 0.1, 0.0);
      double Energy = 5.0;
      double Time = 0.0;
      MString Flags;
      Detector->Noise(Position, Energy, Time, Flags, Detector->GetDetectorVolume());
      if (Version == 1) {
        Passed = EvaluateNear("Noise()", Names[n] + ", " + Input + ", 5 keV, noise threshold 10 keV", "The old engine removes the energy below the noise threshold", Energy, 0.0, 1e-15) && Passed;
      } else {
        Passed = EvaluateNear("Noise()", Names[n] + ", " + Input + ", 5 keV, noise threshold 10 keV", "The new engine keeps the energy: it applies the noise threshold itself", Energy, 5.0, 1e-15) && Passed;
      }
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a hit without detector
bool UTDDetectorEffectsEngine::TestHitWithoutDetector()
{
  bool Passed = true;

  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "hit without detector") == false) {
    return false;
  }
  MDDetectorEffectsEngine* Engine = Geometry.GetDetectorEffectsEngine();

  // The geometry writes the warning about the missing detector directly to the output:
  BeginCaptureOutput();
  MDVolumeSequence Nowhere = Geometry.GetVolumeSequence(m_Nowhere, false, true);
  EndCaptureOutput();
  Passed = EvaluateTrue("GetVolumeSequence()", "position in the world volume", "The position has no detector", Nowhere.GetDetector() == nullptr) && Passed;

  // GEO-E-38: AddHit() uses the detector of the volume sequence without check
  Engine->Reset();
  Passed = ExpectedToCrash(EvaluateTrue("AddHit()", "hit without detector", "A hit without detector does not crash", (Engine->AddHit(m_Nowhere, 100.0, 0.0, set<unsigned int>(), Nowhere), true))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test channels which are blocked from triggering
bool UTDDetectorEffectsEngine::TestBlockedChannels()
{
  bool Passed = true;

  // The strip crossing (4, 4) is blocked from triggering
  MString Lines = "Trigger T\nT.Veto false\nT.TriggerByChannel true\nT.Detector Scinti 1\n";
  Lines += "Trigger S\nS.Veto false\nS.TriggerByChannel true\nS.Detector Strips 1\n";
  Lines += "Strips.BlockTrigger 4 4\n";
  MDGeometry Geometry;
  if (Setup(Geometry, Lines, "blocked channel") == false) {
    return false;
  }
  MDDetectorEffectsEngine* Engine = Geometry.GetDetectorEffectsEngine();

  Engine->Reset();
  AddHit(Geometry, m_WaferA, 100.0);
  Passed = EvaluateFalse("Run()", "100 keV in a blocked strip crossing", "A blocked channel does not trigger", RunEngine(Geometry)) && Passed;

  Engine->Reset();
  AddHit(Geometry, m_WaferANeighbor, 100.0);
  Passed = EvaluateTrue("Run()", "100 keV in the neighbor of the blocked strip crossing", "Another channel triggers", RunEngine(Geometry)) && Passed;

  // Another detector triggers: the blocked channel is above its noise threshold and has to be read out as with the old engine
  Engine->Reset();
  AddHit(Geometry, m_Scinti, 100.0);
  AddHit(Geometry, m_WaferA, 100.0);
  AddHit(Geometry, m_WaferAFar, 20.0);
  Passed = EvaluateTrue("Run()", "trigger in the scintillator, 100 keV in the blocked strip crossing", "The event triggers", RunEngine(Geometry)) && Passed;
  const MDGridPointCollection* A = FindCollection(Geometry, "HolderA");
  Passed = EvaluateTrue("GetGridPointAt()", "20 keV in a strip crossing which is not blocked", "The hit above the noise threshold is read out", HasGridPoint(A, 7, 4)) && Passed;
  // Defect GEO-D-077: IsAboveNoiseThreshold() treats a channel which is blocked from triggering as below the noise threshold
  Passed = ExpectedToFail(EvaluateTrue("GetGridPointAt()", "100 keV in the blocked strip crossing", "The hit in the blocked channel is read out", HasGridPoint(A, 4, 4))) && Passed;

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

  UTDDetectorEffectsEngine Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDDetectorEffectsEngine.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
