/*
 * UTREHit.cxx
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
#include <cfloat>
#include <cmath>
#include <iostream>
#include <new>
#include <set>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TRandom.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MDDetector.h"
#include "MDGridPoint.h"
#include "MDGridPointCollection.h"
#include "MDVolume.h"
#include "MDVolumeSequence.h"
#include "MGeometryRevan.h"
#include "MRECluster.h"
#include "MREHit.h"
#include "MRESE.h"
#include "MREStripHit.h"
#include "MRETrack.h"
#include "UTRevanShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the hit class MREHit
class UTREHit : public UTRevanBase
{
public:
  //! Default constructor
  UTREHit() : UTRevanBase("UTREHit") {}
  //! Default destructor
  virtual ~UTREHit() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the result of the gridding of a hit without a detector in its volume sequence
  bool GridWithoutDetector();

  //! Test the state of a new hit
  bool TestDefaultConstruction();
  //! Test parsing the lines of the simulation file versions
  bool TestParseLineSim();
  //! Test parsing the lines of the evta file versions
  bool TestParseLineEvta();
  //! Test parsing the lines of the MEGA Duke format
  bool TestParseLineDuke();
  //! Test parsing wrong lines and reusing the hit
  bool TestParseLineFailures();
  //! Test the copy constructor and the duplication
  bool TestCopyAndDuplicate();
  //! Test the distances to hits, clusters, and tracks
  bool TestDistances();
  //! Test the adjacency check
  bool TestAreAdjacent();
  //! Test that no sub elements can be added
  bool TestAddRESE();
  //! Test the text output
  bool TestToString();
  //! Test the evta text output
  bool TestToEvtaString();
  //! Test updating the volume sequence
  bool TestUpdateVolumeSequence();
  //! Test retrieving the resolutions from the geometry
  bool TestRetrieveResolutions();
  //! Test the noising with the detector effects engine version 1
  bool TestNoise();
  //! Test the noising with the detector effects engine version 2
  bool TestNoiseEngineVersion2();
  //! Test the gridding
  bool TestGrid();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTREHit::Run()
{
  bool Passed = true;

  // The noise tests use Gaussian energy resolutions
  gRandom->SetSeed(1);

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestParseLineSim() && Passed;
  Passed = TestParseLineEvta() && Passed;
  Passed = TestParseLineDuke() && Passed;
  Passed = TestParseLineFailures() && Passed;
  Passed = TestCopyAndDuplicate() && Passed;
  Passed = TestDistances() && Passed;
  Passed = TestAreAdjacent() && Passed;
  Passed = TestAddRESE() && Passed;
  Passed = TestToString() && Passed;
  Passed = TestToEvtaString() && Passed;
  Passed = TestUpdateVolumeSequence() && Passed;
  Passed = TestRetrieveResolutions() && Passed;
  Passed = TestNoise() && Passed;
  Passed = TestNoiseEngineVersion2() && Passed;
  Passed = TestGrid() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the result of the gridding of a hit without a detector in its volume sequence
bool UTREHit::GridWithoutDetector()
{
  MGeometryRevan Geometry;
  if (ScanTelescopeGeometry(Geometry) == false) {
    return false;
  }

  MREHit Hit;
  Hit.SetPosition(MVector(1.2, 2.3, 0.0));
  Hit.SetEnergy(100.0);
  Hit.SetDetector(MDDetector::c_NoDetectorType);
  MDGridPointCollection Collection = Hit.Grid(&Geometry);

  if (Collection.GetNGridPoints() != 0) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the state of a new hit
bool UTREHit::TestDefaultConstruction()
{
  bool Passed = true;

  MREHit Hit;
  Passed = Evaluate("MREHit()", "default", "The type is a hit", Hit.GetType(), MRESE::c_Hit) && Passed;
  Passed = EvaluateTrue("MREHit()", "default", "The hit is valid", Hit.IsValid()) && Passed;
  Passed = EvaluateFalse("MREHit()", "default", "The resolutions are not fixed", Hit.HasFixedResolutions()) && Passed;
  Passed = EvaluateVectorNear("MREHit()", "default", "The position is the origin", Hit.GetPosition(), MVector(0, 0, 0), 0.0) && Passed;
  Passed = EvaluateVectorNear("MREHit()", "default", "The position resolution is zero", Hit.GetPositionResolution(), MVector(0, 0, 0), 0.0) && Passed;
  Passed = EvaluateNear("MREHit()", "default", "The energy is zero", Hit.GetEnergy(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("MREHit()", "default", "The energy resolution is zero", Hit.GetEnergyResolution(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("MREHit()", "default", "The time is zero", Hit.GetTime(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("MREHit()", "default", "The time resolution is zero", Hit.GetTimeResolution(), 0.0, 0.0) && Passed;
  Passed = Evaluate("MREHit()", "default", "There is no detector type", Hit.GetDetector(), MDDetector::c_NoDetectorType) && Passed;
  Passed = Evaluate("MREHit()", "default", "There are no sub elements", Hit.GetNRESEs(), 0) && Passed;
  Passed = Evaluate("MREHit()", "default", "There are no links", Hit.GetNLinks(), 0) && Passed;
  Passed = EvaluateTrue("MREHit()", "default", "The noise flags are empty", Hit.GetNoiseFlags().IsEmpty()) && Passed;
  Passed = EvaluateSize("MREHit()", "default", "There are no origin IDs", Hit.GetOriginIDs().size(), 0) && Passed;
  Passed = EvaluateTrue("MREHit()", "default", "There is a volume sequence", Hit.GetVolumeSequence() != nullptr) && Passed;
  if (Hit.GetVolumeSequence() != nullptr) {
    Passed = EvaluateTrue("MREHit()", "default", "The volume sequence has no detector", Hit.GetVolumeSequence()->GetDetector() == nullptr) && Passed;
  }

  // The IDs are unique and increase:
  MREHit Second;
  Passed = Evaluate("MREHit()", "second hit", "The ID of the next hit is larger by one", Second.GetID(), Hit.GetID() + 1) && Passed;

  // The members of the hit itself are initialized, also in a block of memory which is not zero:
  alignas(MREHit) unsigned char Block[sizeof(MREHit)];
  FillMemory(Block, sizeof(MREHit), 0xA5);
  MREHit* Placed = new (Block) MREHit();
  Passed = EvaluateFalse("MREHit()", "patterned memory", "The resolutions are not fixed", Placed->HasFixedResolutions()) && Passed;
  Passed = EvaluateTrue("MREHit()", "patterned memory", "The hit is valid", Placed->IsValid()) && Passed;
  Passed = Evaluate("MREHit()", "patterned memory", "The type is a hit", Placed->GetType(), MRESE::c_Hit) && Passed;
  Placed->~MREHit();

  // The flag of the fixed resolutions:
  Hit.FixResolutions(true);
  Passed = EvaluateTrue("FixResolutions()", "true", "The resolutions are fixed", Hit.HasFixedResolutions()) && Passed;
  Hit.FixResolutions(false);
  Passed = EvaluateFalse("FixResolutions()", "false", "The resolutions are no longer fixed", Hit.HasFixedResolutions()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parsing the lines of the simulation file versions
bool UTREHit::TestParseLineSim()
{
  bool Passed = true;

  // Version 100: HTsim <detector>; x; y; z; energy; origin IDs
  {
    MREHit Hit;
    bool Parsed = Hit.ParseLine("HTsim 1; 1.5; -2.5; 3.5; 100.25; 7; 9", 100);
    Passed = EvaluateTrue("ParseLine()", "version 100", "The line is parsed", Parsed) && Passed;
    Passed = EvaluateTrue("ParseLine()", "version 100", "The hit is valid", Hit.IsValid()) && Passed;
    Passed = Evaluate("ParseLine()", "version 100", "The detector type is 1", Hit.GetDetector(), 1) && Passed;
    Passed = EvaluateVectorNear("ParseLine()", "version 100", "The position is (1.5, -2.5, 3.5)", Hit.GetPosition(), MVector(1.5, -2.5, 3.5), 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 100", "The energy is 100.25 keV", Hit.GetEnergy(), 100.25, 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 100", "The time is zero", Hit.GetTime(), 0.0, 0.0) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 100", "The energy resolution is zero", Hit.GetEnergyResolution(), 0.0, 0.0) && Passed;
    Passed = EvaluateVectorNear("ParseLine()", "version 100", "The position resolution is zero", Hit.GetPositionResolution(), MVector(0, 0, 0), 0.0) && Passed;
    Passed = EvaluateFalse("ParseLine()", "version 100", "The resolutions are not fixed", Hit.HasFixedResolutions()) && Passed;
    set<unsigned int> Origins = Hit.GetOriginIDs();
    Passed = EvaluateSize("ParseLine()", "version 100", "There are two origin IDs", Origins.size(), 2) && Passed;
    Passed = EvaluateTrue("ParseLine()", "version 100", "The origin IDs are 7 and 9", Origins.count(7) == 1 && Origins.count(9) == 1) && Passed;
  }

  // Version 100 without origins:
  {
    MREHit Hit;
    bool Parsed = Hit.ParseLine("HTsim 2;0.25;0.5;-0.75;12.5", 100);
    Passed = EvaluateTrue("ParseLine()", "version 100, no origins", "The line is parsed", Parsed) && Passed;
    Passed = Evaluate("ParseLine()", "version 100, no origins", "The detector type is 2", Hit.GetDetector(), 2) && Passed;
    Passed = EvaluateVectorNear("ParseLine()", "version 100, no origins", "The position is (0.25, 0.5, -0.75)", Hit.GetPosition(), MVector(0.25, 0.5, -0.75), 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 100, no origins", "The energy is 12.5 keV", Hit.GetEnergy(), 12.5, 1e-12) && Passed;
    Passed = EvaluateSize("ParseLine()", "version 100, no origins", "There are no origin IDs", Hit.GetOriginIDs().size(), 0) && Passed;
  }

  // Version 101: HTsim <detector>; x; y; z; energy; time; origin IDs
  {
    MREHit Hit;
    bool Parsed = Hit.ParseLine("HTsim 2; 1.5; -2.5; 3.5; 100.25; 0.125; 7; 9; 7", 101);
    Passed = EvaluateTrue("ParseLine()", "version 101", "The line is parsed", Parsed) && Passed;
    Passed = Evaluate("ParseLine()", "version 101", "The detector type is 2", Hit.GetDetector(), 2) && Passed;
    Passed = EvaluateVectorNear("ParseLine()", "version 101", "The position is (1.5, -2.5, 3.5)", Hit.GetPosition(), MVector(1.5, -2.5, 3.5), 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 101", "The energy is 100.25 keV", Hit.GetEnergy(), 100.25, 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 101", "The time is 0.125 s", Hit.GetTime(), 0.125, 1e-12) && Passed;
    Passed = EvaluateFalse("ParseLine()", "version 101", "The resolutions are not fixed", Hit.HasFixedResolutions()) && Passed;
    set<unsigned int> Origins = Hit.GetOriginIDs();
    Passed = EvaluateSize("ParseLine()", "version 101", "The origin ID given twice is stored once", Origins.size(), 2) && Passed;
    Passed = EvaluateTrue("ParseLine()", "version 101", "The origin IDs are 7 and 9", Origins.count(7) == 1 && Origins.count(9) == 1) && Passed;
  }

  // The version of the sim file, 25, is read as version 101 if the line starts with HTsim:
  {
    MREHit Hit;
    bool Parsed = Hit.ParseLine("HTsim 4;-10.5;20.25;30.125;661.7;1.5e-06;3", 25);
    Passed = EvaluateTrue("ParseLine()", "version 25, HTsim", "The line is parsed", Parsed) && Passed;
    Passed = Evaluate("ParseLine()", "version 25, HTsim", "The detector type is 4", Hit.GetDetector(), 4) && Passed;
    Passed = EvaluateVectorNear("ParseLine()", "version 25, HTsim", "The position is (-10.5, 20.25, 30.125)", Hit.GetPosition(), MVector(-10.5, 20.25, 30.125), 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 25, HTsim", "The energy is 661.7 keV", Hit.GetEnergy(), 661.7, 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 25, HTsim", "The time is 1.5 microseconds", Hit.GetTime(), 1.5e-06, 1e-18) && Passed;
    Passed = EvaluateSize("ParseLine()", "version 25, HTsim", "There is one origin ID", Hit.GetOriginIDs().size(), 1) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parsing the lines of the evta file versions
bool UTREHit::TestParseLineEvta()
{
  bool Passed = true;

  // Version 200: HT <detector>; x; y; z; energy; dx; dy; dz; denergy
  {
    MREHit Hit;
    bool Parsed = Hit.ParseLine("HT 3;1.5;-2.5;3.5;100.25;0.1;0.2;0.3;2.5", 200);
    Passed = EvaluateTrue("ParseLine()", "version 200", "The line is parsed", Parsed) && Passed;
    Passed = Evaluate("ParseLine()", "version 200", "The detector type is 3", Hit.GetDetector(), 3) && Passed;
    Passed = EvaluateVectorNear("ParseLine()", "version 200", "The position is (1.5, -2.5, 3.5)", Hit.GetPosition(), MVector(1.5, -2.5, 3.5), 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 200", "The energy is 100.25 keV", Hit.GetEnergy(), 100.25, 1e-12) && Passed;
    Passed = EvaluateVectorNear("ParseLine()", "version 200", "The position resolution is (0.1, 0.2, 0.3)", Hit.GetPositionResolution(), MVector(0.1, 0.2, 0.3), 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 200", "The energy resolution is 2.5 keV", Hit.GetEnergyResolution(), 2.5, 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 200", "The time is zero", Hit.GetTime(), 0.0, 0.0) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 200", "The time resolution is zero", Hit.GetTimeResolution(), 0.0, 0.0) && Passed;
    Passed = EvaluateTrue("ParseLine()", "version 200", "The resolutions are fixed", Hit.HasFixedResolutions()) && Passed;
  }

  // The version of the evta file, 21, is read as version 200 if the line starts with HT:
  {
    MREHit Hit;
    bool Parsed = Hit.ParseLine("HT 1;-0.5;0.25;12.75;59.5;0.03;0.03;0.01;1.25", 21);
    Passed = EvaluateTrue("ParseLine()", "version 21, HT", "The line is parsed", Parsed) && Passed;
    Passed = Evaluate("ParseLine()", "version 21, HT", "The detector type is 1", Hit.GetDetector(), 1) && Passed;
    Passed = EvaluateVectorNear("ParseLine()", "version 21, HT", "The position is (-0.5, 0.25, 12.75)", Hit.GetPosition(), MVector(-0.5, 0.25, 12.75), 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 21, HT", "The energy is 59.5 keV", Hit.GetEnergy(), 59.5, 1e-12) && Passed;
    Passed = EvaluateVectorNear("ParseLine()", "version 21, HT", "The position resolution is (0.03, 0.03, 0.01)", Hit.GetPositionResolution(), MVector(0.03, 0.03, 0.01), 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 21, HT", "The energy resolution is 1.25 keV", Hit.GetEnergyResolution(), 1.25, 1e-12) && Passed;
    Passed = EvaluateTrue("ParseLine()", "version 21, HT", "The resolutions are fixed", Hit.HasFixedResolutions()) && Passed;
  }

  // Version 201: HT <detector>; x; y; z; energy; time; dx; dy; dz; denergy; dtime
  {
    MREHit Hit;
    bool Parsed = Hit.ParseLine("HT 3;1.5;-2.5;3.5;100.25;0.125;0.1;0.2;0.3;2.5;0.001", 201);
    Passed = EvaluateTrue("ParseLine()", "version 201", "The line is parsed", Parsed) && Passed;
    Passed = Evaluate("ParseLine()", "version 201", "The detector type is 3", Hit.GetDetector(), 3) && Passed;
    Passed = EvaluateVectorNear("ParseLine()", "version 201", "The position is (1.5, -2.5, 3.5)", Hit.GetPosition(), MVector(1.5, -2.5, 3.5), 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 201", "The energy is 100.25 keV", Hit.GetEnergy(), 100.25, 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 201", "The time is 0.125 s", Hit.GetTime(), 0.125, 1e-12) && Passed;
    Passed = EvaluateVectorNear("ParseLine()", "version 201", "The position resolution is (0.1, 0.2, 0.3)", Hit.GetPositionResolution(), MVector(0.1, 0.2, 0.3), 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 201", "The energy resolution is 2.5 keV", Hit.GetEnergyResolution(), 2.5, 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 201", "The time resolution is 0.001 s", Hit.GetTimeResolution(), 0.001, 1e-12) && Passed;
    Passed = EvaluateTrue("ParseLine()", "version 201", "The resolutions are fixed", Hit.HasFixedResolutions()) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parsing the lines of the MEGA Duke format
bool UTREHit::TestParseLineDuke()
{
  bool Passed = true;

  // Version 4: HT<4 characters> <detector>; x; y; z; energy; flag; dx; dy; dz; denergy - the lines follow the parser, no original file was at hand
  {
    MREHit Hit;
    bool Parsed = Hit.ParseLine("HTchip 2;1.5;-2.5;3.5;100.25;OK;0.1;0.2;0.3;2.5", 4);
    Passed = EvaluateTrue("ParseLine()", "version 4, OK", "The line is parsed", Parsed) && Passed;
    Passed = Evaluate("ParseLine()", "version 4, OK", "The detector type is 2", Hit.GetDetector(), 2) && Passed;
    Passed = EvaluateVectorNear("ParseLine()", "version 4, OK", "The position is (1.5, -2.5, 3.5)", Hit.GetPosition(), MVector(1.5, -2.5, 3.5), 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 4, OK", "The energy is 100.25 keV", Hit.GetEnergy(), 100.25, 1e-12) && Passed;
    Passed = EvaluateVectorNear("ParseLine()", "version 4, OK", "The position resolution is (0.1, 0.2, 0.3)", Hit.GetPositionResolution(), MVector(0.1, 0.2, 0.3), 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "version 4, OK", "The energy resolution is 2.5 keV", Hit.GetEnergyResolution(), 2.5, 1e-12) && Passed;
  }

  // An overflow is still a valid hit:
  {
    MREHit Hit;
    Passed = EvaluateTrue("ParseLine()", "version 4, OF", "A hit with an overflow is valid", Hit.ParseLine("HTchip 1;1.5;-2.5;3.5;100.25;OF;0.1;0.2;0.3;2.5", 4)) && Passed;
  }

  // Bad hits and hits with only one strip side are not valid:
  {
    MREHit Hit;
    Passed = EvaluateFalse("ParseLine()", "version 4, BA", "A bad hit is not valid", Hit.ParseLine("HTchip 1;1.5;-2.5;3.5;100.25;BA;0.1;0.2;0.3;2.5", 4)) && Passed;
    Passed = EvaluateFalse("ParseLine()", "version 4, BA", "IsValid() returns false too", Hit.IsValid()) && Passed;
    Passed = EvaluateFalse("ParseLine()", "version 4, XO", "A hit with only the x strip is not valid", Hit.ParseLine("HTchip 1;1.5;-2.5;3.5;100.25;XO;0.1;0.2;0.3;2.5", 4)) && Passed;
    Passed = EvaluateFalse("ParseLine()", "version 4, YO", "A hit with only the y strip is not valid", Hit.ParseLine("HTchip 1;1.5;-2.5;3.5;100.25;YO;0.1;0.2;0.3;2.5", 4)) && Passed;
    // The same hit can be used for a good line afterwards:
    Passed = EvaluateTrue("ParseLine()", "version 4, OK after BA", "The hit is valid again", Hit.ParseLine("HTchip 1;1.5;-2.5;3.5;100.25;OK;0.1;0.2;0.3;2.5", 4)) && Passed;
  }

  // The number of fields is checked:
  {
    MREHit Hit;
    Passed = EvaluateFalse("ParseLine()", "version 4, 9 fields", "A line with a missing field is rejected", Hit.ParseLine("HTchip 1;1.5;-2.5;3.5;100.25;OK;0.1;0.2;0.3", 4)) && Passed;
    Passed = EvaluateFalse("ParseLine()", "version 4, 11 fields", "A line with an additional field is rejected", Hit.ParseLine("HTchip 1;1.5;-2.5;3.5;100.25;OK;0.1;0.2;0.3;2.5;1.0", 4)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parsing wrong lines and reusing the hit
bool UTREHit::TestParseLineFailures()
{
  bool Passed = true;

  // An unknown version is rejected with a message:
  {
    MREHit Hit;
    DisableDefaultStreams();
    bool Parsed = Hit.ParseLine("HTsim 1;1.5;-2.5;3.5;100.25", 0);
    EnableDefaultStreams();
    Passed = EvaluateFalse("ParseLine()", "version 0", "The default version 0 is unknown and rejected", Parsed) && Passed;
    Passed = EvaluateFalse("ParseLine()", "version 0", "The hit is not valid", Hit.IsValid()) && Passed;

    DisableDefaultStreams();
    Parsed = Hit.ParseLine("HT 1;1.5;-2.5;3.5;100.25;0.1;0.2;0.3;2.5", 25);
    EnableDefaultStreams();
    Passed = EvaluateFalse("ParseLine()", "version 25, HT", "Version 25 only reads HTsim lines", Parsed) && Passed;

    DisableDefaultStreams();
    Parsed = Hit.ParseLine("HTsim 1;1.5;-2.5;3.5;100.25;0.0", 21);
    EnableDefaultStreams();
    Passed = EvaluateFalse("ParseLine()", "version 21, HTsim", "Version 21 only reads HT lines", Parsed) && Passed;
  }

  // Defect REV-B-1: ParseLine sets the hit valid before it reads the line, so a line with too few fields or a wrong keyword is accepted
  {
    MREHit Hit;
    Passed = ExpectedToFail(EvaluateFalse("ParseLine()", "version 100, 4 fields", "A line without energy is rejected", Hit.ParseLine("HTsim 1;1.5;-2.5;3.5", 100))) && Passed;
    Passed = ExpectedToFail(EvaluateFalse("ParseLine()", "version 100, no detector", "A line without a detector type is rejected", Hit.ParseLine("HTsim;1.5;-2.5;3.5;100.25", 100))) && Passed;
    Passed = ExpectedToFail(EvaluateFalse("ParseLine()", "version 101, 5 fields", "A line without time is rejected", Hit.ParseLine("HTsim 1;1.5;-2.5;3.5;100.25", 101))) && Passed;
    Passed = ExpectedToFail(EvaluateFalse("ParseLine()", "version 200, 8 fields", "A line without energy resolution is rejected", Hit.ParseLine("HT 3;1.5;-2.5;3.5;100.25;0.1;0.2;0.3", 200))) && Passed;
    Passed = ExpectedToFail(EvaluateFalse("ParseLine()", "version 200, wrong keyword", "A line with another keyword is rejected", Hit.ParseLine("XX 3;1.5;-2.5;3.5;100.25;0.1;0.2;0.3;2.5", 200))) && Passed;
    Passed = ExpectedToFail(EvaluateFalse("ParseLine()", "version 201, 10 fields", "A line without time resolution is rejected", Hit.ParseLine("HT 3;1.5;-2.5;3.5;100.25;0.125;0.1;0.2;0.3;2.5", 201))) && Passed;
    Passed = ExpectedToFail(EvaluateFalse("ParseLine()", "version 200, empty", "An empty line is rejected", Hit.ParseLine("", 200))) && Passed;
  }

  // A second line replaces everything of the first one:
  {
    MREHit Hit;
    Hit.ParseLine("HTsim 2; 1.5; -2.5; 3.5; 100.25; 0.125; 7; 9", 101);
    Hit.SetNoiseFlags(" NODEPTH");
    bool Parsed = Hit.ParseLine("HT 1;0.5;0.25;0.125;50.0;0.1;0.2;0.3;2.5", 200);
    Passed = EvaluateTrue("ParseLine()", "second line", "The second line is parsed", Parsed) && Passed;
    Passed = Evaluate("ParseLine()", "second line", "The detector type is the one of the second line", Hit.GetDetector(), 1) && Passed;
    Passed = EvaluateVectorNear("ParseLine()", "second line", "The position is the one of the second line", Hit.GetPosition(), MVector(0.5, 0.25, 0.125), 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "second line", "The energy is the one of the second line", Hit.GetEnergy(), 50.0, 1e-12) && Passed;
    Passed = EvaluateNear("ParseLine()", "second line", "The time of the first line is gone", Hit.GetTime(), 0.0, 0.0) && Passed;
    Passed = EvaluateSize("ParseLine()", "second line", "The origin IDs of the first line are gone", Hit.GetOriginIDs().size(), 0) && Passed;
    Passed = EvaluateTrue("ParseLine()", "second line", "The noise flags are gone", Hit.GetNoiseFlags().IsEmpty()) && Passed;
    Passed = EvaluateTrue("ParseLine()", "second line", "The resolutions are fixed", Hit.HasFixedResolutions()) && Passed;

    // And back to a line without resolutions:
    Parsed = Hit.ParseLine("HTsim 2; 1.5; -2.5; 3.5; 100.25; 0.125", 101);
    Passed = EvaluateTrue("ParseLine()", "third line", "The third line is parsed", Parsed) && Passed;
    Passed = EvaluateFalse("ParseLine()", "third line", "The resolutions are no longer fixed", Hit.HasFixedResolutions()) && Passed;
    Passed = EvaluateVectorNear("ParseLine()", "third line", "The position resolution of the second line is gone", Hit.GetPositionResolution(), MVector(0, 0, 0), 0.0) && Passed;
    Passed = EvaluateNear("ParseLine()", "third line", "The energy resolution of the second line is gone", Hit.GetEnergyResolution(), 0.0, 0.0) && Passed;
  }

  // After a rejected line nothing of the line before is left:
  {
    MREHit Hit;
    Hit.ParseLine("HTsim 2; 1.5; -2.5; 3.5; 100.25; 0.125; 7; 9", 101);
    DisableDefaultStreams();
    Hit.ParseLine("HTsim 2; 1.5; -2.5; 3.5; 100.25; 0.125; 7; 9", 0);
    EnableDefaultStreams();
    Passed = EvaluateNear("ParseLine()", "after a rejected line", "The energy is zero", Hit.GetEnergy(), 0.0, 0.0) && Passed;
    Passed = EvaluateVectorNear("ParseLine()", "after a rejected line", "The position is the origin", Hit.GetPosition(), MVector(0, 0, 0), 0.0) && Passed;
    Passed = EvaluateSize("ParseLine()", "after a rejected line", "There are no origin IDs", Hit.GetOriginIDs().size(), 0) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the copy constructor and the duplication
bool UTREHit::TestCopyAndDuplicate()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope is scanned", ScanTelescopeGeometry(Geometry)) && Passed;

  MREHit* Original = CreateHit(MVector(1.2, 2.3, 1.01), 100.0, MDDetector::c_Strip2D, &Geometry, 0.5);
  Passed = EvaluateTrue("CreateHit()", "tracker layer 1", "The hit is created", Original != nullptr) && Passed;
  if (Original == nullptr) {
    return Passed;
  }
  Original->SetTimeResolution(0.01);
  Original->SetNoiseFlags(" NODEPTH");
  Original->AddOriginID(4);
  Original->AddOriginID(11);
  Original->FixResolutions(true);
  Original->SetValid(false);
  Original->AllowOverwrite(false);
  MDGridPoint Point(5, 6, 0, MDGridPoint::c_Voxel, MVector(1.0, 2.0, 0.0), 100.0, 0.5);
  Original->SetGridPoint(Point);

  MREHit Copy(Original);
  Passed = Evaluate("MREHit(MREHit*)", "copy", "The ID is kept", Copy.GetID(), Original->GetID()) && Passed;
  Passed = Evaluate("MREHit(MREHit*)", "copy", "The type is a hit", Copy.GetType(), MRESE::c_Hit) && Passed;
  Passed = EvaluateVectorNear("MREHit(MREHit*)", "copy", "The position is copied", Copy.GetPosition(), MVector(1.2, 2.3, 1.01), 0.0) && Passed;
  Passed = EvaluateNear("MREHit(MREHit*)", "copy", "The energy is copied", Copy.GetEnergy(), 100.0, 0.0) && Passed;
  Passed = EvaluateNear("MREHit(MREHit*)", "copy", "The time is copied", Copy.GetTime(), 0.5, 0.0) && Passed;
  Passed = EvaluateNear("MREHit(MREHit*)", "copy", "The energy resolution is copied", Copy.GetEnergyResolution(), Original->GetEnergyResolution(), 0.0) && Passed;
  Passed = EvaluateVectorNear("MREHit(MREHit*)", "copy", "The position resolution is copied", Copy.GetPositionResolution(), Original->GetPositionResolution(), 0.0) && Passed;
  Passed = EvaluateNear("MREHit(MREHit*)", "copy", "The time resolution is copied", Copy.GetTimeResolution(), 0.01, 0.0) && Passed;
  Passed = Evaluate("MREHit(MREHit*)", "copy", "The detector type is copied", Copy.GetDetector(), MDDetector::c_Strip2D) && Passed;
  Passed = Evaluate("MREHit(MREHit*)", "copy", "The noise flags are copied", Copy.GetNoiseFlags(), MString(" NODEPTH")) && Passed;
  Passed = EvaluateSize("MREHit(MREHit*)", "copy", "The origin IDs are copied", Copy.GetOriginIDs().size(), 2) && Passed;
  Passed = EvaluateTrue("MREHit(MREHit*)", "copy", "The fixed resolution flag is copied", Copy.HasFixedResolutions()) && Passed;
  Passed = EvaluateFalse("MREHit(MREHit*)", "copy", "The validity is copied", Copy.IsValid()) && Passed;
  Passed = EvaluateFalse("MREHit(MREHit*)", "copy", "The overwrite flag is copied", Copy.AllowOverwrite()) && Passed;
  Passed = EvaluateTrue("MREHit(MREHit*)", "copy", "The volume sequence is an own object", Copy.GetVolumeSequence() != Original->GetVolumeSequence()) && Passed;
  Passed = EvaluateTrue("MREHit(MREHit*)", "copy", "The volume sequence has the same detector", Copy.GetVolumeSequence()->GetDetector() == Original->GetVolumeSequence()->GetDetector()) && Passed;
  Passed = EvaluateVectorNear("MREHit(MREHit*)", "copy", "The position in the detector is copied", Copy.GetVolumeSequence()->GetPositionInDetector(), MVector(1.2, 2.3, 0.01), 1e-12) && Passed;
  // Defect REV-B-2: The copy constructor of MRESE does not copy the grid point, so a duplicated hit has lost its strips (base class, relevant for every duplicate)
  Passed = ExpectedToFail(Evaluate("MREHit(MREHit*)", "copy", "The x grid of the grid point is copied", Copy.GetGridPoint().GetXGrid(), 5U)) && Passed;
  Passed = ExpectedToFail(Evaluate("MREHit(MREHit*)", "copy", "The y grid of the grid point is copied", Copy.GetGridPoint().GetYGrid(), 6U)) && Passed;
  Passed = ExpectedToFail(EvaluateNear("MREHit(MREHit*)", "copy", "The energy of the grid point is copied", Copy.GetGridPoint().GetEnergy(), 100.0, 0.0)) && Passed;

  // The copy is independent:
  Copy.SetEnergy(55.0);
  Copy.SetPosition(MVector(0, 0, 0));
  Copy.FixResolutions(false);
  Passed = EvaluateNear("MREHit(MREHit*)", "copy changed", "The energy of the original is unchanged", Original->GetEnergy(), 100.0, 0.0) && Passed;
  Passed = EvaluateVectorNear("MREHit(MREHit*)", "copy changed", "The position of the original is unchanged", Original->GetPosition(), MVector(1.2, 2.3, 1.01), 0.0) && Passed;
  Passed = EvaluateTrue("MREHit(MREHit*)", "copy changed", "The fixed resolution flag of the original is unchanged", Original->HasFixedResolutions()) && Passed;

  // Links are not copied:
  MREHit Other;
  Original->AddLink(&Other);
  MREHit* Duplicate = Original->Duplicate();
  Passed = EvaluateTrue("Duplicate()", "linked hit", "A new hit is returned", Duplicate != nullptr && Duplicate != Original) && Passed;
  if (Duplicate != nullptr) {
    Passed = Evaluate("Duplicate()", "linked hit", "The ID is kept", Duplicate->GetID(), Original->GetID()) && Passed;
    Passed = Evaluate("Duplicate()", "linked hit", "The links are not duplicated", Duplicate->GetNLinks(), 0) && Passed;
    Passed = EvaluateNear("Duplicate()", "linked hit", "The energy is copied", Duplicate->GetEnergy(), 100.0, 0.0) && Passed;
    Passed = EvaluateTrue("Duplicate()", "linked hit", "The fixed resolution flag is copied", Duplicate->HasFixedResolutions()) && Passed;
    Passed = EvaluateTrue("Duplicate()", "linked hit", "The duplicate equals the original", (*Duplicate == *Original)) && Passed;
    delete Duplicate;
  }

  // Through the base class pointer the hit is duplicated as a hit:
  MRESE* Base = Original;
  MRESE* BaseDuplicate = Base->Duplicate();
  Passed = EvaluateTrue("Duplicate()", "base class pointer", "The duplicate is a hit", dynamic_cast<MREHit*>(BaseDuplicate) != nullptr) && Passed;
  delete BaseDuplicate;

  delete Original;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the distances to hits, clusters, and tracks
bool UTREHit::TestDistances()
{
  bool Passed = true;

  MREHit* A = CreateHit(MVector(1.0, 2.0, 3.0), 100.0, MDDetector::c_Strip2D);
  MREHit* B = CreateHit(MVector(4.0, 6.0, 15.0), 50.0, MDDetector::c_Strip2D);
  MREHit* C = CreateHit(MVector(1.0, 2.0, 5.0), 25.0, MDDetector::c_Strip2D);

  // The vector points from this hit to the other one: (4, 6, 15) - (1, 2, 3) = (3, 4, 12) with length sqrt(9 + 16 + 144) = 13
  Passed = EvaluateVectorNear("ComputeMinDistanceVector()", "hit to hit", "The vector points to the other hit", A->ComputeMinDistanceVector(B), MVector(3.0, 4.0, 12.0), 1e-12) && Passed;
  Passed = EvaluateVectorNear("ComputeMinDistanceVector()", "hit to hit, reversed", "The reversed vector has the opposite sign", B->ComputeMinDistanceVector(A), MVector(-3.0, -4.0, -12.0), 1e-12) && Passed;
  Passed = EvaluateNear("ComputeMinDistance()", "hit to hit", "The distance is 13 cm", A->ComputeMinDistance(B), 13.0, 1e-12) && Passed;
  Passed = EvaluateNear("ComputeMinDistance()", "hit to itself", "The distance to itself is zero", A->ComputeMinDistance(A), 0.0, 0.0) && Passed;

  // Through the base class pointer the same function is called:
  MRESE* Base = A;
  Passed = EvaluateNear("ComputeMinDistance()", "base class pointer", "The distance is 13 cm", Base->ComputeMinDistance(B), 13.0, 1e-12) && Passed;

  // A cluster of B and C: C is the closest hit of the cluster, (1, 2, 5) - (1, 2, 3) = (0, 0, 2)
  MRECluster Cluster;
  BeginCaptureOutput();
  Cluster.AddRESE(B);
  Cluster.AddRESE(C);
  EndCaptureOutput();
  Passed = EvaluateVectorNear("ComputeMinDistanceVector()", "hit to cluster", "The vector points to the closest hit of the cluster", A->ComputeMinDistanceVector(&Cluster), MVector(0.0, 0.0, 2.0), 1e-12) && Passed;
  Passed = EvaluateNear("ComputeMinDistance()", "hit to cluster", "The distance to the closest hit of the cluster is 2 cm", A->ComputeMinDistance(&Cluster), 2.0, 1e-12) && Passed;

  // An empty cluster has no distance:
  MRECluster Empty;
  Passed = EvaluateVectorNear("ComputeMinDistanceVector()", "hit to empty cluster", "All components are DBL_MAX", A->ComputeMinDistanceVector(&Empty), MVector(DBL_MAX, DBL_MAX, DBL_MAX), 0.0) && Passed;

  // A track of B and C:
  MRETrack Track;
  Track.AddRESE(Cluster.RemoveRESE(B));
  Track.AddRESE(Cluster.RemoveRESE(C));
  Passed = EvaluateVectorNear("ComputeMinDistanceVector()", "hit to track", "The vector points to the closest hit of the track", A->ComputeMinDistanceVector(&Track), MVector(0.0, 0.0, 2.0), 1e-12) && Passed;
  Passed = EvaluateNear("ComputeMinDistance()", "hit to track", "The distance to the closest hit of the track is 2 cm", A->ComputeMinDistance(&Track), 2.0, 1e-12) && Passed;

  // Other types have no distance - the DBL_MAX components are intended, see the comment in the function
  MREStripHit Strip;
  Passed = EvaluateVectorNear("ComputeMinDistanceVector()", "hit to strip hit", "All components are DBL_MAX", A->ComputeMinDistanceVector(&Strip), MVector(DBL_MAX, DBL_MAX, DBL_MAX), 0.0) && Passed;

  delete A;
  delete B;
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the adjacency check
bool UTREHit::TestAreAdjacent()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope is scanned", ScanTelescopeGeometry(Geometry)) && Passed;

  // Tracker layer 0, pitch 1 cm: the squared distance in units of the pitch is compared with the level plus 0.01
  MREHit* Center = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Side = CreateHit(MVector(1.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Diagonal = CreateHit(MVector(1.0, 1.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* TwoAway = CreateHit(MVector(0.0, -2.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* JustInside = CreateHit(MVector(1.004, 0.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* JustOutside = CreateHit(MVector(1.006, 0.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* NextLayer = CreateHit(MVector(0.0, 0.0, 1.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* NoSequence = CreateHit(MVector(1.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  bool AllCreated = false;
  if (Center != nullptr && Side != nullptr && Diagonal != nullptr && TwoAway != nullptr && JustInside != nullptr && JustOutside != nullptr && NextLayer != nullptr && NoSequence != nullptr) {
    AllCreated = true;
  }
  Passed = EvaluateTrue("CreateHit()", "tracker hits", "All hits are created", AllCreated) && Passed;
  if (AllCreated == false) {
    return Passed;
  }

  Passed = EvaluateTrue("AreAdjacent()", "side neighbor, level 1", "A hit one strip away is adjacent (1 < 1.01)", Center->AreAdjacent(Side)) && Passed;
  Passed = EvaluateTrue("AreAdjacent()", "side neighbor, reversed", "The check is symmetric", Side->AreAdjacent(Center)) && Passed;
  Passed = EvaluateFalse("AreAdjacent()", "diagonal neighbor, level 1", "A diagonal neighbor is not adjacent at level 1 (2 > 1.01)", Center->AreAdjacent(Diagonal, 3, 1)) && Passed;
  Passed = EvaluateTrue("AreAdjacent()", "diagonal neighbor, level 2", "A diagonal neighbor is adjacent at level 2 (2 < 2.01)", Center->AreAdjacent(Diagonal, 3, 2)) && Passed;
  Passed = EvaluateFalse("AreAdjacent()", "two strips away, level 2", "A hit two strips away is not adjacent at level 2 (4 > 2.01)", Center->AreAdjacent(TwoAway, 3, 2)) && Passed;
  Passed = EvaluateTrue("AreAdjacent()", "two strips away, level 4", "A hit two strips away is adjacent at level 4 (4 < 4.01)", Center->AreAdjacent(TwoAway, 3, 4)) && Passed;
  // 1.004^2 = 1.008016 < 1.01 and 1.006^2 = 1.012036 > 1.01
  Passed = EvaluateTrue("AreAdjacent()", "1.004 pitches, level 1", "A hit just inside the limit is adjacent", Center->AreAdjacent(JustInside, 3, 1)) && Passed;
  Passed = EvaluateFalse("AreAdjacent()", "1.006 pitches, level 1", "A hit just outside the limit is not adjacent", Center->AreAdjacent(JustOutside, 3, 1)) && Passed;
  Passed = EvaluateTrue("AreAdjacent()", "itself", "A hit is adjacent to itself", Center->AreAdjacent(Center)) && Passed;

  // Hits in different wafers are never adjacent:
  Passed = EvaluateFalse("AreAdjacent()", "next layer", "The same strips of the next layer are not adjacent", Center->AreAdjacent(NextLayer, 3, 1)) && Passed;
  Passed = EvaluateFalse("AreAdjacent()", "next layer, no depth check", "Not even without the depth check", Center->AreAdjacent(NextLayer, -1, 10)) && Passed;

  // A hit without a detector in its volume sequence is never adjacent:
  Passed = EvaluateFalse("AreAdjacent()", "hit without volume sequence", "A hit without a detector is not adjacent", Center->AreAdjacent(NoSequence)) && Passed;
  Passed = EvaluateFalse("AreAdjacent()", "from a hit without volume sequence", "The reversed check is false too", NoSequence->AreAdjacent(Center)) && Passed;

  // The depth: calorimeter hits 6 cm apart in z with a depth resolution of 8 cm / sqrt(12) = 2.3094 cm
  MREHit* Top = CreateHit(MVector(0.0, 0.0, -7.0), 100.0, MDDetector::c_Calorimeter, &Geometry);
  MREHit* Bottom = CreateHit(MVector(0.0, 0.0, -13.0), 100.0, MDDetector::c_Calorimeter, &Geometry);
  Passed = EvaluateTrue("CreateHit()", "calorimeter hits", "The hits are created", Top != nullptr && Bottom != nullptr) && Passed;
  if (Top != nullptr && Bottom != nullptr) {
    // 3 sigma: 6.928 cm > 6 cm, 2 sigma: 4.619 cm < 6 cm
    Passed = EvaluateTrue("AreAdjacent()", "6 cm in depth, 3 sigma", "The hits are within 3 sigma of the depth resolution", Top->AreAdjacent(Bottom, 3, 1)) && Passed;
    Passed = EvaluateFalse("AreAdjacent()", "6 cm in depth, 2 sigma", "The hits are not within 2 sigma of the depth resolution", Top->AreAdjacent(Bottom, 2, 1)) && Passed;
    Passed = EvaluateTrue("AreAdjacent()", "6 cm in depth, negative sigma", "A negative sigma switches the depth check off", Top->AreAdjacent(Bottom, -1, 1)) && Passed;
    Passed = EvaluateFalse("AreAdjacent()", "tracker and calorimeter", "Hits in different detectors are not adjacent", Center->AreAdjacent(Top, -1, 100)) && Passed;
  }

  // A cluster and a track are adjacent if one of their hits is:
  MRECluster Cluster;
  BeginCaptureOutput();
  Cluster.AddRESE(Diagonal);
  Cluster.AddRESE(Side);
  EndCaptureOutput();
  Passed = EvaluateTrue("AreAdjacent()", "cluster with a neighbor", "The hit is adjacent to the cluster", Center->AreAdjacent(&Cluster, 3, 1)) && Passed;
  Passed = EvaluateFalse("AreAdjacent()", "cluster without a neighbor", "A hit two strips from all cluster hits is not adjacent", TwoAway->AreAdjacent(&Cluster, 3, 1)) && Passed;
  Cluster.RemoveRESE(Diagonal);
  Cluster.RemoveRESE(Side);

  MRETrack Track;
  Track.AddRESE(Diagonal);
  Track.AddRESE(Side);
  Passed = EvaluateTrue("AreAdjacent()", "track with a neighbor", "The hit is adjacent to the track", Center->AreAdjacent(&Track, 3, 1)) && Passed;
  Passed = EvaluateFalse("AreAdjacent()", "track without a neighbor", "A hit two strips from all track hits is not adjacent", TwoAway->AreAdjacent(&Track, 3, 1)) && Passed;

  // Other types are never adjacent:
  MREStripHit Strip;
  Passed = EvaluateFalse("AreAdjacent()", "strip hit", "A strip hit is not adjacent to a hit", Center->AreAdjacent(&Strip)) && Passed;

  delete Center;
  delete Side;
  delete Diagonal;
  delete TwoAway;
  delete JustInside;
  delete JustOutside;
  delete NextLayer;
  delete NoSequence;
  delete Top;
  delete Bottom;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that no sub elements can be added
bool UTREHit::TestAddRESE()
{
  bool Passed = true;

  MREHit Hit;
  MREHit Other;
  DisableDefaultStreams();
  Hit.AddRESE(&Other);
  EnableDefaultStreams();
  Passed = Evaluate("AddRESE()", "hit to hit", "A hit does not take sub elements", Hit.GetNRESEs(), 0) && Passed;

  // Through the base class pointer too:
  MRESE* Base = &Hit;
  DisableDefaultStreams();
  Base->AddRESE(&Other);
  EnableDefaultStreams();
  Passed = Evaluate("AddRESE()", "base class pointer", "A hit does not take sub elements", Hit.GetNRESEs(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output
bool UTREHit::TestToString()
{
  bool Passed = true;

  MRESE::ResetIDCounter();
  MREHit Hit;
  Hit.SetPosition(MVector(1.0, -2.0, 3.5));
  Hit.SetPositionResolution(MVector(0.1, 0.2, 0.3));
  Hit.SetEnergy(100.0);
  Hit.SetEnergyResolution(2.0);
  Hit.SetDetector(MDDetector::c_Strip2D);

  MString Expected = "Hit (1) at (1.000, -2.000, 3.500)+-(0.100, 0.200, 0.300) with 100.00+-2.00 keV in detector 1\n";
  Passed = Evaluate("ToString()", "no links", "The text has the ID, position, energy, and detector", Hit.ToString(), Expected) && Passed;
  Passed = Evaluate("ToString()", "level 2", "Each level indents by three blanks", Hit.ToString(true, 2), MString("      ") + Expected) && Passed;

  // The second hit has ID 2:
  MREHit Link;
  Hit.AddLink(&Link);
  Passed = Evaluate("ToString()", "one link", "The link is listed in a second line", Hit.ToString(), Expected + "   Linked with:   (2)\n") && Passed;
  Passed = Evaluate("ToString()", "one link, level 1", "The link line is indented one level deeper", Hit.ToString(true, 1), MString("   ") + Expected + "      Linked with:   (2)\n") && Passed;
  Passed = Evaluate("ToString()", "one link, without links", "The link is not listed", Hit.ToString(false), Expected) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the evta text output
bool UTREHit::TestToEvtaString()
{
  bool Passed = true;

  MREHit Hit;
  Hit.SetPosition(MVector(1.5, -2.25, 30.125));
  Hit.SetEnergy(100.25);
  Hit.SetTime(0.125);
  Hit.SetDetector(MDDetector::c_Strip3D);

  // Without scientific precision: fixed with 5 digits, positions and energy 10 characters wide, time scientific and 11 characters wide
  Passed = Evaluate("ToEvtaString()", "precision 0", "The line is fixed with 5 digits", Hit.ToEvtaString(0), MString("HT 3;   1.50000;  -2.25000;  30.12500; 100.25000;1.25000e-01\n")) && Passed;
  Passed = Evaluate("ToEvtaString()", "negative precision", "A negative precision is the fixed format too", Hit.ToEvtaString(-1), MString("HT 3;   1.50000;  -2.25000;  30.12500; 100.25000;1.25000e-01\n")) && Passed;

  // Scientific with 3 digits: positions 10 characters wide, energy and time 9 characters wide
  Passed = Evaluate("ToEvtaString()", "precision 3", "The line is scientific with 3 digits", Hit.ToEvtaString(3), MString("HT 3; 1.500e+00;-2.250e+00; 3.012e+01;1.002e+02;1.250e-01\n")) && Passed;

  // The version does not change the line:
  Passed = Evaluate("ToEvtaString()", "version 26", "The version has no influence", Hit.ToEvtaString(3, 26), Hit.ToEvtaString(3, 1)) && Passed;

  // Through the base class pointer the line of the hit is written:
  MRESE* Base = &Hit;
  Passed = Evaluate("ToEvtaString()", "base class pointer", "The line of the hit is written", Base->ToEvtaString(0), Hit.ToEvtaString(0)) && Passed;

  // Round trip: the line is written with the keyword HT, the detector type, the position, the energy, and the time
  // Defect REV-B-3: No version of ParseLine reads the line of ToEvtaString - the HT versions 200 and 201 need resolutions, the versions with a time need the keyword HTsim
  MREHit Read200;
  bool Parsed200 = Read200.ParseLine(Hit.ToEvtaString(9), 200);
  Passed = ExpectedToFail(EvaluateNear("ToEvtaString() -> ParseLine()", "round trip, version 200", "The time is read back", Read200.GetTime(), 0.125, 1e-9)) && Passed;
  Passed = ExpectedToFail(EvaluateVectorNear("ToEvtaString() -> ParseLine()", "round trip, version 200", "The time is not read as position resolution", Read200.GetPositionResolution(), MVector(0, 0, 0), 0.0)) && Passed;
  Passed = EvaluateTrue("ToEvtaString() -> ParseLine()", "round trip, version 200", "The line is accepted", Parsed200) && Passed;
  Passed = EvaluateVectorNear("ToEvtaString() -> ParseLine()", "round trip, version 200", "The position is read back", Read200.GetPosition(), MVector(1.5, -2.25, 30.125), 1e-9) && Passed;
  Passed = EvaluateNear("ToEvtaString() -> ParseLine()", "round trip, version 200", "The energy is read back", Read200.GetEnergy(), 100.25, 1e-9) && Passed;

  MREHit Read26;
  DisableDefaultStreams();
  bool Parsed26 = Read26.ParseLine(Hit.ToEvtaString(9, 26), 26);
  EnableDefaultStreams();
  Passed = ExpectedToFail(EvaluateTrue("ToEvtaString() -> ParseLine()", "round trip, version 26", "The line written for version 26 is read with version 26", Parsed26)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test updating the volume sequence
bool UTREHit::TestUpdateVolumeSequence()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope is scanned", ScanTelescopeGeometry(Geometry)) && Passed;

  MREHit Hit;
  Hit.SetPosition(MVector(1.2, 2.3, 2.01));
  Hit.SetEnergy(100.0);
  Hit.SetDetector(MDDetector::c_Strip2D);
  bool Updated = Hit.UpdateVolumeSequence(&Geometry);
  Passed = EvaluateTrue("UpdateVolumeSequence()", "tracker layer 2", "The volume sequence is found", Updated) && Passed;
  MDVolumeSequence* Sequence = Hit.GetVolumeSequence();
  Passed = EvaluateTrue("UpdateVolumeSequence()", "tracker layer 2", "The detector is the tracker", Sequence->GetDetector() != nullptr && Sequence->GetDetector()->GetName() == "Tracker") && Passed;
  Passed = EvaluateTrue("UpdateVolumeSequence()", "tracker layer 2", "The sensitive volume is the wafer", Sequence->GetSensitiveVolume() != nullptr && Sequence->GetSensitiveVolume()->GetName() == "Wafer") && Passed;
  // World - holder of layer 2 - wafer
  Passed = Evaluate("UpdateVolumeSequence()", "tracker layer 2", "The sequence has three volumes", Sequence->GetNVolumes(), 3U) && Passed;
  if (Sequence->GetNVolumes() == 3) {
    Passed = Evaluate("UpdateVolumeSequence()", "tracker layer 2", "The second volume is the holder of layer 2", Sequence->GetVolumeAt(1)->GetName(), MString("Holder2")) && Passed;
  }
  // The wafer of layer 2 is centered at z = 2 cm
  Passed = EvaluateVectorNear("UpdateVolumeSequence()", "tracker layer 2", "The position in the detector is relative to the wafer", Sequence->GetPositionInDetector(), MVector(1.2, 2.3, 0.01), 1e-12) && Passed;
  // The energy and the resolutions are not touched:
  Passed = EvaluateNear("UpdateVolumeSequence()", "tracker layer 2", "The energy resolution is not retrieved", Hit.GetEnergyResolution(), 0.0, 0.0) && Passed;

  // Move the hit into the calorimeter:
  Hit.SetPosition(MVector(-3.0, 4.0, -12.0));
  Updated = Hit.UpdateVolumeSequence(&Geometry);
  Passed = EvaluateTrue("UpdateVolumeSequence()", "moved to the calorimeter", "The volume sequence is found", Updated) && Passed;
  Passed = EvaluateTrue("UpdateVolumeSequence()", "moved to the calorimeter", "The detector is the calorimeter", Hit.GetVolumeSequence()->GetDetector() != nullptr && Hit.GetVolumeSequence()->GetDetector()->GetName() == "Calo") && Passed;
  Passed = EvaluateVectorNear("UpdateVolumeSequence()", "moved to the calorimeter", "The position in the detector is relative to the block at z = -10 cm", Hit.GetVolumeSequence()->GetPositionInDetector(), MVector(-3.0, 4.0, -2.0), 1e-12) && Passed;

  // A position without a detector: the hit keeps its last volume sequence
  Hit.SetPosition(MVector(50.0, 50.0, 50.0));
  DisableDefaultStreams();
  BeginCaptureOutput();
  Updated = Hit.UpdateVolumeSequence(&Geometry);
  EndCaptureOutput();
  EnableDefaultStreams();
  Passed = EvaluateFalse("UpdateVolumeSequence()", "position in the world volume", "No volume sequence with a detector is found", Updated) && Passed;
  Passed = EvaluateTrue("UpdateVolumeSequence()", "position in the world volume", "The last volume sequence is kept", Hit.GetVolumeSequence()->GetDetector() != nullptr && Hit.GetVolumeSequence()->GetDetector()->GetName() == "Calo") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test retrieving the resolutions from the geometry
bool UTREHit::TestRetrieveResolutions()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope is scanned", ScanTelescopeGeometry(Geometry)) && Passed;

  const double Root12 = sqrt(12.0);

  // Tracker: pitch 1 cm, wafer thickness 0.05 cm, energy resolution 2 keV
  {
    MREHit Hit;
    Hit.SetPosition(MVector(1.2, 2.3, 3.01));
    Hit.SetEnergy(100.0);
    Hit.SetDetector(MDDetector::c_Strip2D);
    bool Retrieved = Hit.RetrieveResolutions(&Geometry);
    Passed = EvaluateTrue("RetrieveResolutions()", "tracker layer 3", "The resolutions are retrieved", Retrieved) && Passed;
    Passed = EvaluateNear("RetrieveResolutions()", "tracker layer 3", "The energy resolution is 2 keV", Hit.GetEnergyResolution(), 2.0, 1e-12) && Passed;
    Passed = EvaluateVectorNear("RetrieveResolutions()", "tracker layer 3", "The position resolution is pitch and thickness over sqrt(12)", Hit.GetPositionResolution(), MVector(1.0/Root12, 1.0/Root12, 0.05/Root12), 1e-12) && Passed;
    Passed = EvaluateTrue("RetrieveResolutions()", "tracker layer 3", "The volume sequence is updated", Hit.GetVolumeSequence()->GetDetector() != nullptr && Hit.GetVolumeSequence()->GetDetector()->GetName() == "Tracker") && Passed;
    // The hit itself is not changed:
    Passed = EvaluateVectorNear("RetrieveResolutions()", "tracker layer 3", "The position is unchanged", Hit.GetPosition(), MVector(1.2, 2.3, 3.01), 0.0) && Passed;
    Passed = EvaluateNear("RetrieveResolutions()", "tracker layer 3", "The energy is unchanged", Hit.GetEnergy(), 100.0, 0.0) && Passed;

    // The same hit moved into the calorimeter: block of 10 x 10 x 8 cm, energy resolution 10 keV
    Hit.SetPosition(MVector(-3.0, 4.0, -12.0));
    Retrieved = Hit.RetrieveResolutions(&Geometry);
    Passed = EvaluateTrue("RetrieveResolutions()", "moved to the calorimeter", "The resolutions are retrieved", Retrieved) && Passed;
    Passed = EvaluateNear("RetrieveResolutions()", "moved to the calorimeter", "The energy resolution is 10 keV", Hit.GetEnergyResolution(), 10.0, 1e-12) && Passed;
    Passed = EvaluateVectorNear("RetrieveResolutions()", "moved to the calorimeter", "The position resolution is the block size over sqrt(12)", Hit.GetPositionResolution(), MVector(10.0/Root12, 10.0/Root12, 8.0/Root12), 1e-12) && Passed;
  }

  // Depth detector: pitch 1 cm, depth resolution 0.1 cm, energy resolution 1 keV
  {
    MREHit Hit;
    Hit.SetPosition(MVector(30.2, 0.3, 0.5));
    Hit.SetEnergy(500.0);
    Hit.SetDetector(MDDetector::c_Strip3D);
    bool Retrieved = Hit.RetrieveResolutions(&Geometry);
    Passed = EvaluateTrue("RetrieveResolutions()", "depth detector", "The resolutions are retrieved", Retrieved) && Passed;
    Passed = EvaluateNear("RetrieveResolutions()", "depth detector", "The energy resolution is 1 keV", Hit.GetEnergyResolution(), 1.0, 1e-12) && Passed;
    Passed = EvaluateVectorNear("RetrieveResolutions()", "depth detector", "The position resolution is the pitch over sqrt(12) and the depth resolution", Hit.GetPositionResolution(), MVector(1.0/Root12, 1.0/Root12, 0.1), 1e-12) && Passed;
  }

  // Fixed resolutions are kept, and the geometry is not asked at all:
  {
    MREHit Hit;
    Hit.ParseLine("HT 1;1.2;2.3;3.01;100.0;0.5;0.6;0.7;5.5", 200);
    bool Retrieved = Hit.RetrieveResolutions(&Geometry);
    Passed = EvaluateTrue("RetrieveResolutions()", "fixed resolutions", "The call succeeds", Retrieved) && Passed;
    Passed = EvaluateNear("RetrieveResolutions()", "fixed resolutions", "The energy resolution of the line is kept", Hit.GetEnergyResolution(), 5.5, 1e-12) && Passed;
    Passed = EvaluateVectorNear("RetrieveResolutions()", "fixed resolutions", "The position resolution of the line is kept", Hit.GetPositionResolution(), MVector(0.5, 0.6, 0.7), 1e-12) && Passed;
    // Intended: the caller (MRERawEvent::ParseLine) updates the volume sequence of hits with fixed resolutions itself
    Passed = EvaluateTrue("RetrieveResolutions()", "fixed resolutions", "The volume sequence is not updated", Hit.GetVolumeSequence()->GetDetector() == nullptr) && Passed;

    // Outside of all detectors the fixed resolutions still succeed:
    Hit.SetPosition(MVector(50.0, 50.0, 50.0));
    Passed = EvaluateTrue("RetrieveResolutions()", "fixed resolutions, no detector", "The call succeeds without a detector", Hit.RetrieveResolutions(&Geometry)) && Passed;
  }

  // Without a detector at the position the call fails and keeps the resolutions:
  {
    MREHit Hit;
    Hit.SetPosition(MVector(50.0, 50.0, 50.0));
    Hit.SetEnergy(100.0);
    Hit.SetEnergyResolution(7.0);
    DisableDefaultStreams();
    BeginCaptureOutput();
    bool Retrieved = Hit.RetrieveResolutions(&Geometry);
    EndCaptureOutput();
    EnableDefaultStreams();
    Passed = EvaluateFalse("RetrieveResolutions()", "position in the world volume", "The call fails", Retrieved) && Passed;
    Passed = EvaluateNear("RetrieveResolutions()", "position in the world volume", "The energy resolution is unchanged", Hit.GetEnergyResolution(), 7.0, 0.0) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the noising with the detector effects engine version 1
bool UTREHit::TestNoise()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope is scanned", ScanTelescopeGeometry(Geometry, "Depth.DepthResolutionThreshold 200\n")) && Passed;

  // Shield: ideal energy resolution, noise threshold 50 keV
  {
    MREHit* Hit = CreateHit(MVector(1.0, 2.0, 10.1), 100.0, MDDetector::c_Scintillator, &Geometry, 0.25);
    Passed = EvaluateTrue("CreateHit()", "shield", "The hit is created", Hit != nullptr) && Passed;
    if (Hit != nullptr) {
      Hit->SetNoiseFlags(" OLD");
      bool Noised = Hit->Noise(&Geometry);
      Passed = EvaluateTrue("Noise()", "shield, 100 keV", "The hit is above the noise threshold", Noised) && Passed;
      Passed = EvaluateNear("Noise()", "shield, 100 keV", "The ideal energy resolution keeps the energy", Hit->GetEnergy(), 100.0, 1e-12) && Passed;
      Passed = EvaluateTrue("Noise()", "shield, 100 keV", "The old noise flags are removed", Hit->GetNoiseFlags().IsEmpty()) && Passed;
      // Intended in geomega: a detector without a time resolution does not measure the time and sets it to zero
      Passed = EvaluateNear("Noise()", "shield, 100 keV", "The time is zero without a time resolution", Hit->GetTime(), 0.0, 0.0) && Passed;
      delete Hit;
    }

    // Just above and just below the noise threshold of 50 keV:
    Hit = CreateHit(MVector(1.0, 2.0, 10.1), 50.5, MDDetector::c_Scintillator, &Geometry);
    if (Hit != nullptr) {
      Passed = EvaluateTrue("Noise()", "shield, 50.5 keV", "A hit just above the noise threshold stays", Hit->Noise(&Geometry)) && Passed;
      Passed = EvaluateNear("Noise()", "shield, 50.5 keV", "The energy is kept", Hit->GetEnergy(), 50.5, 1e-12) && Passed;
      delete Hit;
    }
    Hit = CreateHit(MVector(1.0, 2.0, 10.1), 49.5, MDDetector::c_Scintillator, &Geometry);
    if (Hit != nullptr) {
      Passed = EvaluateFalse("Noise()", "shield, 49.5 keV", "A hit just below the noise threshold is removed", Hit->Noise(&Geometry)) && Passed;
      Passed = EvaluateNear("Noise()", "shield, 49.5 keV", "The energy is zero", Hit->GetEnergy(), 0.0, 0.0) && Passed;
      delete Hit;
    }
  }

  // Tracker: Gaussian energy resolution of 2 keV, the position is not changed by a 2D strip detector
  {
    unsigned int Trials = 2000;
    double Sum = 0.0;
    double Sum2 = 0.0;
    bool AllNoised = true;
    bool AllPositionsKept = true;
    for (unsigned int i = 0; i < Trials; ++i) {
      MREHit* Hit = CreateHit(MVector(1.2, 2.3, 1.01), 500.0, MDDetector::c_Strip2D, &Geometry);
      if (Hit == nullptr) {
        AllNoised = false;
        continue;
      }
      if (Hit->Noise(&Geometry) == false) {
        AllNoised = false;
      }
      if ((Hit->GetPosition() - MVector(1.2, 2.3, 1.01)).Mag() > 1e-12) {
        AllPositionsKept = false;
      }
      Sum += Hit->GetEnergy();
      Sum2 += Hit->GetEnergy()*Hit->GetEnergy();
      delete Hit;
    }
    double Mean = Sum/Trials;
    double Sigma = sqrt(Sum2/Trials - Mean*Mean);
    Passed = EvaluateTrue("Noise()", "tracker, 500 keV, 2000 hits", "All hits are above the noise threshold", AllNoised) && Passed;
    Passed = EvaluateTrue("Noise()", "tracker, 500 keV, 2000 hits", "The positions are unchanged", AllPositionsKept) && Passed;
    // The error of the mean is 2/sqrt(2000) = 0.045 keV, the error of the sigma is 2/sqrt(4000) = 0.032 keV: 5 sigma limits
    Passed = EvaluateNear("Noise()", "tracker, 500 keV, 2000 hits", "The mean energy is 500 keV", Mean, 500.0, 0.225) && Passed;
    Passed = EvaluateNear("Noise()", "tracker, 500 keV, 2000 hits", "The width of the energies is 2 keV", Sigma, 2.0, 0.16) && Passed;
  }

  // Depth detector below the depth resolution threshold of 200 keV: the depth is set to the center of the wafer and flagged
  {
    MREHit* Hit = CreateHit(MVector(30.2, 0.3, 0.5), 100.0, MDDetector::c_Strip3D, &Geometry);
    Passed = EvaluateTrue("CreateHit()", "depth detector", "The hit is created", Hit != nullptr) && Passed;
    if (Hit != nullptr) {
      bool Noised = Hit->Noise(&Geometry);
      Passed = EvaluateTrue("Noise()", "depth detector, below the depth threshold", "The hit is above the noise threshold", Noised) && Passed;
      Passed = EvaluateTrue("Noise()", "depth detector, below the depth threshold", "The hit is flagged as without depth", Hit->GetNoiseFlags().Contains("NODEPTH")) && Passed;
      Passed = EvaluateVectorNear("Noise()", "depth detector, below the depth threshold", "The depth is the center of the wafer, x and y are kept", Hit->GetPosition(), MVector(30.2, 0.3, 0.0), 1e-12) && Passed;
      delete Hit;
    }
  }

  // Without a geometry nothing is noised:
  {
    MREHit* Hit = CreateHit(MVector(1.2, 2.3, 1.01), 5.0, MDDetector::c_Strip2D, &Geometry);
    if (Hit != nullptr) {
      Hit->SetNoiseFlags(" OLD");
      bool Noised = Hit->Noise(nullptr);
      Passed = EvaluateTrue("Noise()", "no geometry", "A hit with positive energy stays", Noised) && Passed;
      Passed = EvaluateNear("Noise()", "no geometry", "The energy is unchanged, also below the noise threshold", Hit->GetEnergy(), 5.0, 0.0) && Passed;
      Passed = Evaluate("Noise()", "no geometry", "The noise flags are unchanged", Hit->GetNoiseFlags(), MString(" OLD")) && Passed;

      Hit->SetEnergy(0.0);
      Passed = EvaluateFalse("Noise()", "no geometry, zero energy", "A hit without energy is removed", Hit->Noise(nullptr)) && Passed;
      Hit->SetEnergy(-1.0);
      Passed = EvaluateFalse("Noise()", "no geometry, negative energy", "A hit with negative energy is removed", Hit->Noise(nullptr)) && Passed;

      // The detector type must be between 1 and 9:
      Hit->SetEnergy(100.0);
      Hit->SetDetector(MDDetector::c_MaxDetector);
      Passed = EvaluateTrue("Noise()", "no geometry, detector type 9", "The largest detector type is accepted", Hit->Noise(nullptr)) && Passed;
      Hit->SetDetector(MDDetector::c_MaxDetector + 1);
      DisableDefaultStreams();
      Noised = Hit->Noise(nullptr);
      EnableDefaultStreams();
      Passed = EvaluateFalse("Noise()", "no geometry, detector type 10", "A detector type above the maximum is rejected", Noised) && Passed;
      Hit->SetDetector(MDDetector::c_MinDetector - 1);
      DisableDefaultStreams();
      Noised = Hit->Noise(nullptr);
      EnableDefaultStreams();
      Passed = EvaluateFalse("Noise()", "no geometry, detector type 0", "A detector type below the minimum is rejected", Noised) && Passed;
      delete Hit;
    }
  }

  // A hit without a detector in its volume sequence gets no energy:
  {
    MREHit* Hit = CreateHit(MVector(1.2, 2.3, 1.01), 100.0, MDDetector::c_Strip2D);
    DisableDefaultStreams();
    bool Noised = Hit->Noise(&Geometry);
    EnableDefaultStreams();
    Passed = EvaluateFalse("Noise()", "no volume sequence", "The hit is removed", Noised) && Passed;
    Passed = EvaluateNear("Noise()", "no volume sequence", "The energy is zero", Hit->GetEnergy(), 0.0, 0.0) && Passed;
    delete Hit;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the noising with the detector effects engine version 2
bool UTREHit::TestNoiseEngineVersion2()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope, engine 2", "The telescope is scanned", ScanTelescopeGeometry(Geometry, "DetectorEffectsEngine 2\nTracker.FailureRate 1.0\n")) && Passed;

  // With the engine version 2 the noise threshold is applied later, on the grid points: the hit below the threshold of 50 keV keeps its energy
  {
    MREHit* Hit = CreateHit(MVector(1.0, 2.0, 10.1), 49.5, MDDetector::c_Scintillator, &Geometry);
    Passed = EvaluateTrue("CreateHit()", "shield, engine 2", "The hit is created", Hit != nullptr) && Passed;
    if (Hit != nullptr) {
      Passed = EvaluateTrue("Noise()", "shield, 49.5 keV, engine 2", "A hit below the noise threshold stays", Hit->Noise(&Geometry)) && Passed;
      Passed = EvaluateNear("Noise()", "shield, 49.5 keV, engine 2", "The energy is kept", Hit->GetEnergy(), 49.5, 1e-12) && Passed;
      Passed = EvaluateTrue("Noise()", "shield, 49.5 keV, engine 2", "There are no noise flags", Hit->GetNoiseFlags().IsEmpty()) && Passed;
      delete Hit;
    }
  }

  // A failure rate of 1: the hit is flagged and removed
  {
    MREHit* Hit = CreateHit(MVector(1.2, 2.3, 1.01), 100.0, MDDetector::c_Strip2D, &Geometry);
    Passed = EvaluateTrue("CreateHit()", "tracker, engine 2", "The hit is created", Hit != nullptr) && Passed;
    if (Hit != nullptr) {
      Passed = EvaluateFalse("Noise()", "tracker, failure rate 1", "The failed hit is removed", Hit->Noise(&Geometry)) && Passed;
      Passed = EvaluateNear("Noise()", "tracker, failure rate 1", "The energy is zero", Hit->GetEnergy(), 0.0, 0.0) && Passed;
      Passed = Evaluate("Noise()", "tracker, failure rate 1", "The hit is flagged as failure", Hit->GetNoiseFlags(), MString(" FAILURE")) && Passed;

      // A second call starts with new flags:
      Hit->SetEnergy(100.0);
      Hit->Noise(&Geometry);
      Passed = Evaluate("Noise()", "tracker, failure rate 1, second call", "The flag is not doubled", Hit->GetNoiseFlags(), MString(" FAILURE")) && Passed;
      delete Hit;
    }
  }

  // Depth detector: the depth is noised with 0.1 cm and stays within the wafer of +-1 cm, x and y are kept
  {
    unsigned int Trials = 2000;
    double Sum = 0.0;
    double Sum2 = 0.0;
    bool AllInside = true;
    bool AllFlagsEmpty = true;
    bool AllXYKept = true;
    for (unsigned int i = 0; i < Trials; ++i) {
      MREHit* Hit = CreateHit(MVector(30.2, 0.3, 0.5), 500.0, MDDetector::c_Strip3D, &Geometry);
      if (Hit == nullptr) {
        AllInside = false;
        continue;
      }
      Hit->Noise(&Geometry);
      if (fabs(Hit->GetPosition().Z()) > 1.0) {
        AllInside = false;
      }
      if (Hit->GetNoiseFlags().IsEmpty() == false) {
        AllFlagsEmpty = false;
      }
      if (fabs(Hit->GetPosition().X() - 30.2) > 1e-12 || fabs(Hit->GetPosition().Y() - 0.3) > 1e-12) {
        AllXYKept = false;
      }
      Sum += Hit->GetPosition().Z();
      Sum2 += Hit->GetPosition().Z()*Hit->GetPosition().Z();
      delete Hit;
    }
    double Mean = Sum/Trials;
    double Sigma = sqrt(Sum2/Trials - Mean*Mean);
    Passed = EvaluateTrue("Noise()", "depth detector, engine 2, 2000 hits", "All depths are inside the wafer", AllInside) && Passed;
    Passed = EvaluateTrue("Noise()", "depth detector, engine 2, 2000 hits", "No hit is flagged", AllFlagsEmpty) && Passed;
    Passed = EvaluateTrue("Noise()", "depth detector, engine 2, 2000 hits", "x and y are unchanged", AllXYKept) && Passed;
    // The error of the mean is 0.1/sqrt(2000) = 0.0022 cm, the error of the sigma is 0.1/sqrt(4000) = 0.0016 cm: 5 sigma limits
    Passed = EvaluateNear("Noise()", "depth detector, engine 2, 2000 hits", "The mean depth is 0.5 cm", Mean, 0.5, 0.011) && Passed;
    Passed = EvaluateNear("Noise()", "depth detector, engine 2, 2000 hits", "The width of the depths is 0.1 cm", Sigma, 0.1, 0.008) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the gridding
bool UTREHit::TestGrid()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope, engine 2", "The telescope is scanned", ScanTelescopeGeometry(Geometry, "DetectorEffectsEngine 2\n")) && Passed;

  // Tracker layer 1: 9 strips of 1 cm from -4.5 cm, so x = 1.2 cm is strip 5 and y = 2.3 cm is strip 6
  {
    MREHit* Hit = CreateHit(MVector(1.2, 2.3, 1.01), 100.0, MDDetector::c_Strip2D, &Geometry, 0.25);
    Passed = EvaluateTrue("CreateHit()", "tracker layer 1", "The hit is created", Hit != nullptr) && Passed;
    if (Hit != nullptr) {
      BeginCaptureOutput();
      MDGridPointCollection Collection = Hit->Grid(&Geometry);
      MString Output = EndCaptureOutput();
      Passed = Evaluate("Grid()", "tracker layer 1", "There is one grid point", Collection.GetNGridPoints(), 1U) && Passed;
      if (Collection.GetNGridPoints() == 1) {
        const MDGridPoint& Point = Collection.GetGridPointAt(0);
        Passed = Evaluate("Grid()", "tracker layer 1", "The x strip is 5", Point.GetXGrid(), 5U) && Passed;
        Passed = Evaluate("Grid()", "tracker layer 1", "The y strip is 6", Point.GetYGrid(), 6U) && Passed;
        Passed = EvaluateNear("Grid()", "tracker layer 1", "The grid point has the energy of the hit", Point.GetEnergy(), 100.0, 1e-12) && Passed;
        Passed = EvaluateNear("Grid()", "tracker layer 1", "The grid point has the time of the hit", Point.GetTime(), 0.25, 1e-12) && Passed;
      }
      Passed = EvaluateTrue("Grid()", "tracker layer 1", "The collection has the detector of the hit", Collection.GetDetector() != nullptr && Collection.GetDetector()->GetName() == "Tracker") && Passed;
      // The hit itself is not changed:
      Passed = EvaluateVectorNear("Grid()", "tracker layer 1", "The position of the hit is unchanged", Hit->GetPosition(), MVector(1.2, 2.3, 1.01), 1e-12) && Passed;
      Passed = EvaluateNear("Grid()", "tracker layer 1", "The energy of the hit is unchanged", Hit->GetEnergy(), 100.0, 0.0) && Passed;
      // Defect REV-B-4 (geomega, GEO-B-43): MDGeometryQuest::Grid prints "Grid points: N" to the standard output for every hit
      Passed = ExpectedToFail(EvaluateTrue("Grid()", "tracker layer 1", "Nothing is printed to the standard output", Output.IsEmpty())) && Passed;

      // In the guard ring (x beyond 4.5 cm) there is no strip, the grid point is the one of the guard ring:
      Hit->SetPosition(MVector(4.8, 2.3, 1.01));
      Hit->UpdateVolumeSequence(&Geometry);
      DisableDefaultStreams();
      BeginCaptureOutput();
      MDGridPointCollection GuardRing = Hit->Grid(&Geometry);
      EndCaptureOutput();
      EnableDefaultStreams();
      Passed = Evaluate("Grid()", "tracker guard ring", "There is one grid point", GuardRing.GetNGridPoints(), 1U) && Passed;
      if (GuardRing.GetNGridPoints() == 1) {
        Passed = Evaluate("Grid()", "tracker guard ring", "The grid point is a guard ring point", GuardRing.GetGridPointAt(0).GetType(), MDGridPoint::c_GuardRing) && Passed;
        Passed = EvaluateNear("Grid()", "tracker guard ring", "The grid point has the energy of the hit", GuardRing.GetGridPointAt(0).GetEnergy(), 100.0, 1e-12) && Passed;
      }

      // Without a geometry and with an unknown detector type the collection is empty:
      Hit->SetPosition(MVector(1.2, 2.3, 1.01));
      Hit->UpdateVolumeSequence(&Geometry);
      MDGridPointCollection NoGeometry = Hit->Grid(nullptr);
      Passed = Evaluate("Grid()", "no geometry", "There is no grid point", NoGeometry.GetNGridPoints(), 0U) && Passed;
      Passed = EvaluateTrue("Grid()", "no geometry", "The collection has the detector of the hit", NoGeometry.GetDetector() != nullptr && NoGeometry.GetDetector()->GetName() == "Tracker") && Passed;

      Hit->SetDetector(MDDetector::c_MaxDetector + 1);
      DisableDefaultStreams();
      BeginCaptureOutput();
      MDGridPointCollection UnknownType = Hit->Grid(&Geometry);
      EndCaptureOutput();
      EnableDefaultStreams();
      Passed = Evaluate("Grid()", "detector type 10", "There is no grid point", UnknownType.GetNGridPoints(), 0U) && Passed;
      delete Hit;
    }
  }

  // Depth detector: 10 strips of 1 cm from -5 cm around x = 30 cm, so x = 30.2 cm is strip 5 and y = 0.3 cm is strip 5; one point per strip side
  {
    MREHit* Hit = CreateHit(MVector(30.2, 0.3, 0.5), 400.0, MDDetector::c_Strip3D, &Geometry);
    Passed = EvaluateTrue("CreateHit()", "depth detector", "The hit is created", Hit != nullptr) && Passed;
    if (Hit != nullptr) {
      BeginCaptureOutput();
      MDGridPointCollection Collection = Hit->Grid(&Geometry);
      EndCaptureOutput();
      Passed = Evaluate("Grid()", "depth detector", "There are two grid points, one per strip side", Collection.GetNGridPoints(), 2U) && Passed;
      if (Collection.GetNGridPoints() == 2) {
        unsigned int XStrips = 0;
        unsigned int YStrips = 0;
        for (unsigned int p = 0; p < 2; ++p) {
          const MDGridPoint& Point = Collection.GetGridPointAt(p);
          if (Point.GetType() == MDGridPoint::c_XStrip) {
            ++XStrips;
            Passed = Evaluate("Grid()", "depth detector, x strip", "The x strip is 5", Point.GetXGrid(), 5U) && Passed;
          } else if (Point.GetType() == MDGridPoint::c_YStrip) {
            ++YStrips;
            Passed = Evaluate("Grid()", "depth detector, y strip", "The y strip is 5", Point.GetYGrid(), 5U) && Passed;
          }
          Passed = EvaluateNear("Grid()", "depth detector", "Each strip side has the full energy", Point.GetEnergy(), 400.0, 1e-9) && Passed;
        }
        Passed = Evaluate("Grid()", "depth detector", "There is one x strip", XStrips, 1U) && Passed;
        Passed = Evaluate("Grid()", "depth detector", "There is one y strip", YStrips, 1U) && Passed;
      }
      delete Hit;
    }
  }

  // Defect REV-B-5 (geomega, GEO-B-44): A hit without a detector in its volume sequence cannot be gridded, since the grid point collection needs a detector
  Passed = ExpectedToCrash(EvaluateTrue("Grid()", "hit without volume sequence", "The collection is empty", GridWithoutDetector())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  // Hide the information messages of ROOT
  gErrorIgnoreLevel = kWarning;

  UTREHit Test;
  return Test.Run() == true ? 0 : 1;
}


// UTREHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
