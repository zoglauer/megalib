/*
 * UTREStripHit.cxx
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
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MDDetector.h"
#include "MDGridPoint.h"
#include "MDGridPointCollection.h"
#include "MDVolumeSequence.h"
#include "MGeometryRevan.h"
#include "MGlobal.h"
#include "MREHit.h"
#include "MRESE.h"
#include "MREStripHit.h"
#include "UTRevanShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the strip hit class MREStripHit
class UTREStripHit : public UTRevanBase
{
public:
  //! Default constructor
  UTREStripHit() : UTRevanBase("UTREStripHit") {}
  //! Default destructor
  virtual ~UTREStripHit() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return a new strip hit of the given side and strip in the detector with the given ID
  MREStripHit* CreateStripHit(bool IsXStrip, int StripID, long DetectorID, double Energy);
  //! Return the result of the gridding of a strip hit without a detector in its volume sequence
  bool GridWithoutDetector();

  //! Test the state of a new strip hit
  bool TestDefaultConstruction();
  //! Test the setters and getters of the strip data
  bool TestStripData();
  //! Test the copy constructor and the duplication
  bool TestCopyAndDuplicate();
  //! Test parsing a line
  bool TestParseLine();
  //! Test the distances and the adjacency of the base class interface
  bool TestDistancesAndAreAdjacent();
  //! Test the adjacency of strips
  bool TestAreStripsAdjacent();
  //! Test that no sub elements can be added
  bool TestAddRESE();
  //! Test the text output
  bool TestToString();
  //! Test updating the volume sequence and retrieving the resolutions
  bool TestVolumeSequenceAndResolutions();
  //! Test the noising
  bool TestNoise();
  //! Test the gridding
  bool TestGrid();
  //! Test strip hits made from the grid points of a hit as the noising does
  bool TestFromGridPoints();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTREStripHit::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestStripData() && Passed;
  Passed = TestCopyAndDuplicate() && Passed;
  Passed = TestParseLine() && Passed;
  Passed = TestDistancesAndAreAdjacent() && Passed;
  Passed = TestAreStripsAdjacent() && Passed;
  Passed = TestAddRESE() && Passed;
  Passed = TestToString() && Passed;
  Passed = TestVolumeSequenceAndResolutions() && Passed;
  Passed = TestNoise() && Passed;
  Passed = TestGrid() && Passed;
  Passed = TestFromGridPoints() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a new strip hit of the given side and strip in the detector with the given ID
MREStripHit* UTREStripHit::CreateStripHit(bool IsXStrip, int StripID, long DetectorID, double Energy)
{
  MREStripHit* Hit = new MREStripHit();
  Hit->IsXStrip(IsXStrip);
  Hit->SetStripID(StripID);
  Hit->SetDetectorID(DetectorID);
  Hit->SetEnergy(Energy);
  Hit->SetDetector(MDDetector::c_Strip3D);

  return Hit;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the result of the gridding of a strip hit without a detector in its volume sequence
bool UTREStripHit::GridWithoutDetector()
{
  MGeometryRevan Geometry;
  if (ScanTelescopeGeometry(Geometry) == false) {
    return false;
  }

  MREStripHit Hit;
  Hit.SetPosition(MVector(30.2, 0.3, 0.5));
  Hit.SetEnergy(100.0);
  Hit.SetDetector(MDDetector::c_NoDetectorType);
  MDGridPointCollection Collection = Hit.Grid(&Geometry);

  if (Collection.GetNGridPoints() != 0) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the state of a new strip hit
bool UTREStripHit::TestDefaultConstruction()
{
  bool Passed = true;

  MREStripHit Hit;
  Passed = Evaluate("MREStripHit()", "default", "The type is a strip hit", Hit.GetType(), MRESE::c_StripHit) && Passed;
  Passed = EvaluateTrue("MREStripHit()", "default", "The strip hit is valid", Hit.IsValid()) && Passed;
  Passed = EvaluateFalse("MREStripHit()", "default", "The resolutions are not fixed", Hit.HasFixedResolutions()) && Passed;
  Passed = EvaluateTrue("MREStripHit()", "default", "The position is not defined", Hit.GetPosition() == g_VectorNotDefined) && Passed;
  Passed = EvaluateTrue("MREStripHit()", "default", "It is an x strip", Hit.IsXStrip()) && Passed;
  Passed = EvaluateNear("MREStripHit()", "default", "The non-strip position is not defined", Hit.GetNonStripPosition(), g_DoubleNotDefined, 0.0) && Passed;
  Passed = EvaluateNear("MREStripHit()", "default", "The depth position is not defined", Hit.GetDepthPosition(), g_DoubleNotDefined, 0.0) && Passed;
  Passed = EvaluateNear("MREStripHit()", "default", "The energy is zero", Hit.GetEnergy(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("MREStripHit()", "default", "The energy resolution is zero", Hit.GetEnergyResolution(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("MREStripHit()", "default", "The time is zero", Hit.GetTime(), 0.0, 0.0) && Passed;
  Passed = Evaluate("MREStripHit()", "default", "There is no detector type", Hit.GetDetector(), MDDetector::c_NoDetectorType) && Passed;
  Passed = Evaluate("MREStripHit()", "default", "There are no sub elements", Hit.GetNRESEs(), 0) && Passed;
  Passed = Evaluate("MREStripHit()", "default", "There are no links", Hit.GetNLinks(), 0) && Passed;

  // All members are initialized, also in a block of memory which is not zero:
  alignas(MREStripHit) unsigned char Block[sizeof(MREStripHit)];
  FillMemory(Block, sizeof(MREStripHit), 0xA5);
  MREStripHit* Placed = new (Block) MREStripHit();
  Passed = EvaluateFalse("MREStripHit()", "patterned memory", "The resolutions are not fixed", Placed->HasFixedResolutions()) && Passed;
  Passed = EvaluateTrue("MREStripHit()", "patterned memory", "It is an x strip", Placed->IsXStrip()) && Passed;
  Passed = Evaluate("MREStripHit()", "patterned memory", "The strip ID is the one of a default strip hit", Placed->GetStripID(), Hit.GetStripID()) && Passed;
  Passed = Evaluate("MREStripHit()", "patterned memory", "The detector ID is the one of a default strip hit", Placed->GetDetectorID(), Hit.GetDetectorID()) && Passed;
  Passed = EvaluateNear("MREStripHit()", "patterned memory", "The non-strip position is not defined", Placed->GetNonStripPosition(), g_DoubleNotDefined, 0.0) && Passed;
  Passed = EvaluateNear("MREStripHit()", "patterned memory", "The depth position is not defined", Placed->GetDepthPosition(), g_DoubleNotDefined, 0.0) && Passed;
  Passed = EvaluateTrue("MREStripHit()", "patterned memory", "The strip hit is valid", Placed->IsValid()) && Passed;
  Placed->~MREStripHit();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the setters and getters of the strip data
bool UTREStripHit::TestStripData()
{
  bool Passed = true;

  MREStripHit Hit;

  Hit.IsXStrip(false);
  Passed = EvaluateFalse("IsXStrip()", "false", "It is a y strip", Hit.IsXStrip()) && Passed;
  Hit.IsXStrip(true);
  Passed = EvaluateTrue("IsXStrip()", "true", "It is an x strip again", Hit.IsXStrip()) && Passed;

  Hit.SetStripID(17);
  Passed = Evaluate("SetStripID()/GetStripID()", "17", "The strip ID is 17", Hit.GetStripID(), 17) && Passed;
  Hit.SetStripID(0);
  Passed = Evaluate("SetStripID()/GetStripID()", "0", "The strip ID is 0", Hit.GetStripID(), 0) && Passed;

  // The detector ID is a hash of the volume names, thus large:
  Hit.SetDetectorID(123456789012345L);
  Passed = Evaluate("SetDetectorID()/GetDetectorID()", "123456789012345", "The detector ID is kept with all digits", Hit.GetDetectorID(), 123456789012345L) && Passed;
  Hit.SetDetectorID(-5L);
  Passed = Evaluate("SetDetectorID()/GetDetectorID()", "-5", "A negative detector ID is kept", Hit.GetDetectorID(), -5L) && Passed;

  Hit.SetNonStripPosition(-3.25);
  Passed = EvaluateNear("SetNonStripPosition()/GetNonStripPosition()", "-3.25 cm", "The non-strip position is -3.25 cm", Hit.GetNonStripPosition(), -3.25, 0.0) && Passed;
  Hit.SetDepthPosition(0.75);
  Passed = EvaluateNear("SetDepthPosition()/GetDepthPosition()", "0.75 cm", "The depth position is 0.75 cm", Hit.GetDepthPosition(), 0.75, 0.0) && Passed;
  Passed = EvaluateNear("SetDepthPosition()", "0.75 cm", "The non-strip position is not changed", Hit.GetNonStripPosition(), -3.25, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the copy constructor and the duplication
bool UTREStripHit::TestCopyAndDuplicate()
{
  bool Passed = true;

  MREStripHit Original;
  Original.IsXStrip(false);
  Original.SetStripID(7);
  Original.SetDetectorID(987654321L);
  Original.SetNonStripPosition(1.5);
  Original.SetDepthPosition(-0.25);
  Original.SetPosition(MVector(30.5, 1.5, -0.25));
  Original.SetEnergy(250.0);
  Original.SetEnergyResolution(1.5);
  Original.SetTime(0.5);
  Original.SetDetector(MDDetector::c_Strip3D);
  Original.SetNoiseFlags(" NODEPTH");
  Original.AddOriginID(3);
  Original.SetValid(false);

  MREStripHit Copy(&Original);
  Passed = Evaluate("MREStripHit(MREStripHit*)", "copy", "The ID is kept", Copy.GetID(), Original.GetID()) && Passed;
  Passed = Evaluate("MREStripHit(MREStripHit*)", "copy", "The type is a strip hit", Copy.GetType(), MRESE::c_StripHit) && Passed;
  Passed = EvaluateFalse("MREStripHit(MREStripHit*)", "copy", "The strip side is copied", Copy.IsXStrip()) && Passed;
  Passed = Evaluate("MREStripHit(MREStripHit*)", "copy", "The strip ID is copied", Copy.GetStripID(), 7) && Passed;
  Passed = Evaluate("MREStripHit(MREStripHit*)", "copy", "The detector ID is copied", Copy.GetDetectorID(), 987654321L) && Passed;
  Passed = EvaluateNear("MREStripHit(MREStripHit*)", "copy", "The non-strip position is copied", Copy.GetNonStripPosition(), 1.5, 0.0) && Passed;
  Passed = EvaluateNear("MREStripHit(MREStripHit*)", "copy", "The depth position is copied", Copy.GetDepthPosition(), -0.25, 0.0) && Passed;
  Passed = EvaluateVectorNear("MREStripHit(MREStripHit*)", "copy", "The position is copied", Copy.GetPosition(), MVector(30.5, 1.5, -0.25), 0.0) && Passed;
  Passed = EvaluateNear("MREStripHit(MREStripHit*)", "copy", "The energy is copied", Copy.GetEnergy(), 250.0, 0.0) && Passed;
  Passed = EvaluateNear("MREStripHit(MREStripHit*)", "copy", "The energy resolution is copied", Copy.GetEnergyResolution(), 1.5, 0.0) && Passed;
  Passed = EvaluateNear("MREStripHit(MREStripHit*)", "copy", "The time is copied", Copy.GetTime(), 0.5, 0.0) && Passed;
  Passed = Evaluate("MREStripHit(MREStripHit*)", "copy", "The detector type is copied", Copy.GetDetector(), MDDetector::c_Strip3D) && Passed;
  Passed = Evaluate("MREStripHit(MREStripHit*)", "copy", "The noise flags are copied", Copy.GetNoiseFlags(), MString(" NODEPTH")) && Passed;
  Passed = EvaluateSize("MREStripHit(MREStripHit*)", "copy", "The origin IDs are copied", Copy.GetOriginIDs().size(), 1) && Passed;
  Passed = EvaluateFalse("MREStripHit(MREStripHit*)", "copy", "The validity is copied", Copy.IsValid()) && Passed;
  Passed = EvaluateFalse("MREStripHit(MREStripHit*)", "copy", "The resolutions are not fixed", Copy.HasFixedResolutions()) && Passed;

  // The copy is independent:
  Copy.SetStripID(8);
  Copy.IsXStrip(true);
  Copy.SetDepthPosition(0.5);
  Passed = Evaluate("MREStripHit(MREStripHit*)", "copy changed", "The strip ID of the original is unchanged", Original.GetStripID(), 7) && Passed;
  Passed = EvaluateFalse("MREStripHit(MREStripHit*)", "copy changed", "The strip side of the original is unchanged", Original.IsXStrip()) && Passed;
  Passed = EvaluateNear("MREStripHit(MREStripHit*)", "copy changed", "The depth position of the original is unchanged", Original.GetDepthPosition(), -0.25, 0.0) && Passed;

  MREStripHit* Duplicate = Original.Duplicate();
  Passed = EvaluateTrue("Duplicate()", "strip hit", "A new strip hit is returned", Duplicate != nullptr && Duplicate != &Original) && Passed;
  if (Duplicate != nullptr) {
    Passed = Evaluate("Duplicate()", "strip hit", "The ID is kept", Duplicate->GetID(), Original.GetID()) && Passed;
    Passed = Evaluate("Duplicate()", "strip hit", "The strip ID is copied", Duplicate->GetStripID(), 7) && Passed;
    Passed = Evaluate("Duplicate()", "strip hit", "The detector ID is copied", Duplicate->GetDetectorID(), 987654321L) && Passed;
    Passed = EvaluateFalse("Duplicate()", "strip hit", "The strip side is copied", Duplicate->IsXStrip()) && Passed;
    delete Duplicate;
  }

  // Through the base class pointer the strip hit is duplicated as a strip hit:
  MRESE* Base = &Original;
  MRESE* BaseDuplicate = Base->Duplicate();
  MREStripHit* StripDuplicate = dynamic_cast<MREStripHit*>(BaseDuplicate);
  Passed = EvaluateTrue("Duplicate()", "base class pointer", "The duplicate is a strip hit", StripDuplicate != nullptr) && Passed;
  if (StripDuplicate != nullptr) {
    Passed = Evaluate("Duplicate()", "base class pointer", "The strip ID is copied", StripDuplicate->GetStripID(), 7) && Passed;
  }
  delete BaseDuplicate;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parsing a line
bool UTREStripHit::TestParseLine()
{
  bool Passed = true;

  // There is no line format for strip hits yet: each line is rejected and the strip hit is not valid
  MREStripHit Hit;
  Hit.SetEnergy(100.0);
  Passed = EvaluateFalse("ParseLine()", "hit line", "The line of a hit is rejected", Hit.ParseLine("HT 3;1.5;-2.5;3.5;100.25;0.1;0.2;0.3;2.5", 200)) && Passed;
  Passed = EvaluateFalse("ParseLine()", "hit line", "The strip hit is not valid", Hit.IsValid()) && Passed;
  Passed = EvaluateNear("ParseLine()", "hit line", "The energy from before is removed", Hit.GetEnergy(), 0.0, 0.0) && Passed;
  Passed = EvaluateFalse("ParseLine()", "empty line", "An empty line is rejected", Hit.ParseLine("")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the distances and the adjacency of the base class interface
bool UTREStripHit::TestDistancesAndAreAdjacent()
{
  bool Passed = true;

  // A strip has no position, thus there are no distances - the DBL_MAX components are the same as for hits of unknown types
  MREStripHit* A = CreateStripHit(true, 4, 1L, 100.0);
  MREStripHit* B = CreateStripHit(true, 5, 1L, 100.0);
  MREHit Hit;
  Passed = EvaluateVectorNear("ComputeMinDistanceVector()", "strip hit to strip hit", "All components are DBL_MAX", A->ComputeMinDistanceVector(B), MVector(DBL_MAX, DBL_MAX, DBL_MAX), 0.0) && Passed;
  Passed = EvaluateVectorNear("ComputeMinDistanceVector()", "strip hit to hit", "All components are DBL_MAX", A->ComputeMinDistanceVector(&Hit), MVector(DBL_MAX, DBL_MAX, DBL_MAX), 0.0) && Passed;
  Passed = EvaluateTrue("ComputeMinDistance()", "strip hit to strip hit", "The distance is larger than every real distance", A->ComputeMinDistance(B) > 1e300) && Passed;

  // The adjacency of positions does not apply to strips, also for neighboring strips:
  Passed = EvaluateFalse("AreAdjacent()", "neighboring strips", "Strip hits are never adjacent in the sense of hits", A->AreAdjacent(B)) && Passed;
  Passed = EvaluateFalse("AreAdjacent()", "hit", "A strip hit is not adjacent to a hit", A->AreAdjacent(&Hit, -1, 100)) && Passed;
  MRESE* Base = A;
  Passed = EvaluateFalse("AreAdjacent()", "base class pointer", "The same is true through the base class", Base->AreAdjacent(B)) && Passed;

  delete A;
  delete B;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the adjacency of strips
bool UTREStripHit::TestAreStripsAdjacent()
{
  bool Passed = true;

  MREStripHit* X4 = CreateStripHit(true, 4, 100L, 100.0);
  MREStripHit* X5 = CreateStripHit(true, 5, 100L, 100.0);
  MREStripHit* X6 = CreateStripHit(true, 6, 100L, 100.0);
  MREStripHit* X0 = CreateStripHit(true, 0, 100L, 100.0);
  MREStripHit* X1 = CreateStripHit(true, 1, 100L, 100.0);
  MREStripHit* Y5 = CreateStripHit(false, 5, 100L, 100.0);
  MREStripHit* Y6 = CreateStripHit(false, 6, 100L, 100.0);
  MREStripHit* OtherX5 = CreateStripHit(true, 5, 101L, 100.0);

  Passed = EvaluateTrue("AreStripsAdjacent()", "x strips 4 and 5", "Neighboring strips are adjacent", X4->AreStripsAdjacent(X5)) && Passed;
  Passed = EvaluateTrue("AreStripsAdjacent()", "x strips 5 and 4", "The check is symmetric", X5->AreStripsAdjacent(X4)) && Passed;
  Passed = EvaluateTrue("AreStripsAdjacent()", "x strips 0 and 1", "The first two strips are adjacent", X0->AreStripsAdjacent(X1)) && Passed;
  Passed = EvaluateTrue("AreStripsAdjacent()", "y strips 5 and 6", "Neighboring y strips are adjacent", Y5->AreStripsAdjacent(Y6)) && Passed;
  Passed = EvaluateFalse("AreStripsAdjacent()", "x strips 4 and 6", "Strips with a strip between them are not adjacent", X4->AreStripsAdjacent(X6)) && Passed;
  Passed = EvaluateFalse("AreStripsAdjacent()", "x strip 5 with itself", "A strip is not adjacent to itself", X5->AreStripsAdjacent(X5)) && Passed;
  Passed = EvaluateFalse("AreStripsAdjacent()", "x strip 4 and y strip 5", "Strips of different sides are not adjacent", X4->AreStripsAdjacent(Y5)) && Passed;
  Passed = EvaluateFalse("AreStripsAdjacent()", "x strip 5 and y strip 5", "The same strip number of the other side is not adjacent", X5->AreStripsAdjacent(Y5)) && Passed;
  Passed = EvaluateFalse("AreStripsAdjacent()", "x strips 4 and 5 of different detectors", "Strips of different detectors are not adjacent", X4->AreStripsAdjacent(OtherX5)) && Passed;

  delete X4;
  delete X5;
  delete X6;
  delete X0;
  delete X1;
  delete Y5;
  delete Y6;
  delete OtherX5;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that no sub elements can be added
bool UTREStripHit::TestAddRESE()
{
  bool Passed = true;

  MREStripHit Hit;
  MREStripHit Other;
  DisableDefaultStreams();
  Hit.AddRESE(&Other);
  EnableDefaultStreams();
  Passed = Evaluate("AddRESE()", "strip hit to strip hit", "A strip hit does not take sub elements", Hit.GetNRESEs(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output
bool UTREStripHit::TestToString()
{
  bool Passed = true;

  MRESE::ResetIDCounter();
  MREStripHit Hit;
  Hit.IsXStrip(true);
  Hit.SetStripID(5);
  Hit.SetDetectorID(7L);
  Hit.SetEnergy(100.0);
  Hit.SetEnergyResolution(2.0);
  Hit.SetDetector(MDDetector::c_Strip3D);

  MString Expected = "StripHit (1) at x-strip 5 with 100.00+-2.00 keV in detector 3 (ID: 7)\n";
  Passed = Evaluate("ToString()", "x strip", "The text has the ID, side, strip, energy, and detector", Hit.ToString(), Expected) && Passed;
  Passed = Evaluate("ToString()", "x strip, level 1", "Each level indents by three blanks", Hit.ToString(true, 1), MString("   ") + Expected) && Passed;

  Hit.IsXStrip(false);
  Passed = Evaluate("ToString()", "y strip", "The side is y", Hit.ToString(), MString("StripHit (1) at y-strip 5 with 100.00+-2.00 keV in detector 3 (ID: 7)\n")) && Passed;

  // The second strip hit has ID 2:
  MREStripHit Link;
  Hit.AddLink(&Link);
  Passed = Evaluate("ToString()", "one link", "The link is listed in a second line", Hit.ToString(), MString("StripHit (1) at y-strip 5 with 100.00+-2.00 keV in detector 3 (ID: 7)\n   Linked with:   (2)\n")) && Passed;
  Passed = Evaluate("ToString()", "one link, without links", "The link is not listed", Hit.ToString(false), MString("StripHit (1) at y-strip 5 with 100.00+-2.00 keV in detector 3 (ID: 7)\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test updating the volume sequence and retrieving the resolutions
bool UTREStripHit::TestVolumeSequenceAndResolutions()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope is scanned", ScanTelescopeGeometry(Geometry)) && Passed;

  // The depth detector is centered at (30, 0, 0) cm
  MREStripHit Hit;
  Hit.SetPosition(MVector(30.5, 1.5, -0.25));
  Hit.SetEnergy(250.0);
  Hit.SetDetector(MDDetector::c_Strip3D);
  bool Updated = Hit.UpdateVolumeSequence(&Geometry);
  Passed = EvaluateTrue("UpdateVolumeSequence()", "depth detector", "The volume sequence is found", Updated) && Passed;
  Passed = EvaluateTrue("UpdateVolumeSequence()", "depth detector", "The detector is the depth detector", Hit.GetVolumeSequence()->GetDetector() != nullptr && Hit.GetVolumeSequence()->GetDetector()->GetName() == "Depth") && Passed;
  Passed = EvaluateVectorNear("UpdateVolumeSequence()", "depth detector", "The position in the detector is relative to the wafer", Hit.GetVolumeSequence()->GetPositionInDetector(), MVector(0.5, 1.5, -0.25), 1e-12) && Passed;
  Passed = EvaluateNear("UpdateVolumeSequence()", "depth detector", "The energy resolution is not retrieved", Hit.GetEnergyResolution(), 0.0, 0.0) && Passed;

  // Pitch 1 cm, depth resolution 0.1 cm, energy resolution 1 keV
  bool Retrieved = Hit.RetrieveResolutions(&Geometry);
  Passed = EvaluateTrue("RetrieveResolutions()", "depth detector", "The resolutions are retrieved", Retrieved) && Passed;
  Passed = EvaluateNear("RetrieveResolutions()", "depth detector", "The energy resolution is 1 keV", Hit.GetEnergyResolution(), 1.0, 1e-12) && Passed;
  Passed = EvaluateVectorNear("RetrieveResolutions()", "depth detector", "The position resolution is the pitch over sqrt(12) and the depth resolution", Hit.GetPositionResolution(), MVector(1.0/sqrt(12.0), 1.0/sqrt(12.0), 0.1), 1e-12) && Passed;
  Passed = EvaluateNear("RetrieveResolutions()", "depth detector", "The energy is unchanged", Hit.GetEnergy(), 250.0, 0.0) && Passed;

  // A position without a detector: the calls fail and the last volume sequence is kept
  Hit.SetPosition(MVector(50.0, 50.0, 50.0));
  DisableDefaultStreams();
  BeginCaptureOutput();
  Updated = Hit.UpdateVolumeSequence(&Geometry);
  Retrieved = Hit.RetrieveResolutions(&Geometry);
  EndCaptureOutput();
  EnableDefaultStreams();
  Passed = EvaluateFalse("UpdateVolumeSequence()", "position in the world volume", "No volume sequence with a detector is found", Updated) && Passed;
  Passed = EvaluateFalse("RetrieveResolutions()", "position in the world volume", "The resolutions are not retrieved", Retrieved) && Passed;
  Passed = EvaluateTrue("UpdateVolumeSequence()", "position in the world volume", "The last volume sequence is kept", Hit.GetVolumeSequence()->GetDetector() != nullptr && Hit.GetVolumeSequence()->GetDetector()->GetName() == "Depth") && Passed;
  Passed = EvaluateNear("RetrieveResolutions()", "position in the world volume", "The energy resolution is unchanged", Hit.GetEnergyResolution(), 1.0, 1e-12) && Passed;

  // A new strip hit has no defined position, thus no detector:
  MREStripHit Undefined;
  DisableDefaultStreams();
  BeginCaptureOutput();
  Updated = Undefined.UpdateVolumeSequence(&Geometry);
  EndCaptureOutput();
  EnableDefaultStreams();
  Passed = EvaluateFalse("UpdateVolumeSequence()", "new strip hit", "No volume sequence is found for the undefined position", Updated) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the noising
bool UTREStripHit::TestNoise()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope is scanned", ScanTelescopeGeometry(Geometry)) && Passed;

  // Strip hits are made from noised grid points, they are never noised themselves: the function does nothing and returns false
  MREStripHit Hit;
  Hit.SetPosition(MVector(30.5, 1.5, -0.25));
  Hit.SetEnergy(250.0);
  Hit.SetTime(0.5);
  Hit.SetDetector(MDDetector::c_Strip3D);
  Hit.UpdateVolumeSequence(&Geometry);
  Hit.SetNoiseFlags(" OLD");
  Passed = EvaluateFalse("Noise()", "depth detector", "The noising is rejected", Hit.Noise(&Geometry)) && Passed;
  Passed = EvaluateNear("Noise()", "depth detector", "The energy is unchanged", Hit.GetEnergy(), 250.0, 0.0) && Passed;
  Passed = EvaluateVectorNear("Noise()", "depth detector", "The position is unchanged", Hit.GetPosition(), MVector(30.5, 1.5, -0.25), 0.0) && Passed;
  Passed = EvaluateNear("Noise()", "depth detector", "The time is unchanged", Hit.GetTime(), 0.5, 0.0) && Passed;
  Passed = Evaluate("Noise()", "depth detector", "The noise flags are unchanged", Hit.GetNoiseFlags(), MString(" OLD")) && Passed;
  Passed = EvaluateFalse("Noise()", "no geometry", "The noising is rejected without a geometry too", Hit.Noise(nullptr)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the gridding
bool UTREStripHit::TestGrid()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope, engine 2", "The telescope is scanned", ScanTelescopeGeometry(Geometry, "DetectorEffectsEngine 2\n")) && Passed;

  // Depth detector: 10 strips of 1 cm from -5 cm around x = 30 cm, so x = 30.2 cm is strip 5 and y = -2.7 cm is strip 2
  MREStripHit Hit;
  Hit.SetPosition(MVector(30.2, -2.7, 0.5));
  Hit.SetEnergy(400.0);
  Hit.SetTime(0.25);
  Hit.SetDetector(MDDetector::c_Strip3D);
  Passed = EvaluateTrue("UpdateVolumeSequence()", "depth detector", "The volume sequence is found", Hit.UpdateVolumeSequence(&Geometry)) && Passed;

  BeginCaptureOutput();
  MDGridPointCollection Collection = Hit.Grid(&Geometry);
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
        Passed = Evaluate("Grid()", "depth detector, y strip", "The y strip is 2", Point.GetYGrid(), 2U) && Passed;
      }
      Passed = EvaluateNear("Grid()", "depth detector", "Each strip side has the full energy", Point.GetEnergy(), 400.0, 1e-9) && Passed;
      Passed = EvaluateNear("Grid()", "depth detector", "Each strip side has the time of the strip hit", Point.GetTime(), 0.25, 1e-12) && Passed;
    }
    Passed = Evaluate("Grid()", "depth detector", "There is one x strip", XStrips, 1U) && Passed;
    Passed = Evaluate("Grid()", "depth detector", "There is one y strip", YStrips, 1U) && Passed;
  }
  Passed = EvaluateVectorNear("Grid()", "depth detector", "The position of the strip hit is unchanged", Hit.GetPosition(), MVector(30.2, -2.7, 0.5), 1e-12) && Passed;
  Passed = EvaluateNear("Grid()", "depth detector", "The energy of the strip hit is unchanged", Hit.GetEnergy(), 400.0, 0.0) && Passed;

  // Without a geometry and with an unknown detector type the collection is empty:
  MDGridPointCollection NoGeometry = Hit.Grid(nullptr);
  Passed = Evaluate("Grid()", "no geometry", "There is no grid point", NoGeometry.GetNGridPoints(), 0U) && Passed;
  Passed = EvaluateTrue("Grid()", "no geometry", "The collection has the detector of the strip hit", NoGeometry.GetDetector() != nullptr && NoGeometry.GetDetector()->GetName() == "Depth") && Passed;

  Hit.SetDetector(MDDetector::c_MinDetector - 1);
  DisableDefaultStreams();
  BeginCaptureOutput();
  MDGridPointCollection UnknownType = Hit.Grid(&Geometry);
  EndCaptureOutput();
  EnableDefaultStreams();
  Passed = Evaluate("Grid()", "detector type 0", "There is no grid point", UnknownType.GetNGridPoints(), 0U) && Passed;

  // Defect REV-B-5 (geomega, GEO-B-44): A strip hit without a detector in its volume sequence cannot be gridded, since the grid point collection needs a detector
  Passed = ExpectedToCrash(EvaluateTrue("Grid()", "strip hit without volume sequence", "The collection is empty", GridWithoutDetector())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test strip hits made from the grid points of a hit as the noising does
bool UTREStripHit::TestFromGridPoints()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope, engine 2", "The telescope is scanned", ScanTelescopeGeometry(Geometry, "DetectorEffectsEngine 2\n")) && Passed;

  // Two hits in neighboring x strips (5 and 6) and the same y strip (2) of the depth detector
  MVector Positions[2] = { MVector(30.2, -2.7, 0.5), MVector(31.4, -2.2, -0.5) };
  vector<MREStripHit*> XStrips;
  vector<MREStripHit*> YStrips;
  for (unsigned int h = 0; h < 2; ++h) {
    MREHit* Hit = CreateHit(Positions[h], 200.0, MDDetector::c_Strip3D, &Geometry);
    Passed = EvaluateTrue("CreateHit()", "depth detector", "The hit is created", Hit != nullptr) && Passed;
    if (Hit == nullptr) {
      continue;
    }
    BeginCaptureOutput();
    MDGridPointCollection Collection = Hit->Grid(&Geometry);
    EndCaptureOutput();
    for (unsigned int p = 0; p < Collection.GetNGridPoints(); ++p) {
      const MDGridPoint& Point = Collection.GetGridPointAt(p);
      // The same steps as in the noising:
      MREStripHit* Strip = new MREStripHit();
      Strip->SetEnergy(Point.GetEnergy());
      Strip->SetVolumeSequence(new MDVolumeSequence(Collection.GetVolumeSequence()));
      Strip->SetDetector(Strip->GetVolumeSequence()->GetDetector()->GetType());
      MString DetectorName = "";
      for (unsigned int v = 0; v < Strip->GetVolumeSequence()->GetNVolumes(); ++v) {
        DetectorName += Strip->GetVolumeSequence()->GetVolumeAt(v)->GetName();
      }
      Strip->SetDetectorID(DetectorName.GetHash());
      if (Point.GetType() == MDGridPoint::c_XStrip) {
        Strip->IsXStrip(true);
        Strip->SetStripID(Point.GetXGrid());
        XStrips.push_back(Strip);
      } else {
        Strip->IsXStrip(false);
        Strip->SetStripID(Point.GetYGrid());
        YStrips.push_back(Strip);
      }
      Strip->SetGridPoint(Point);
    }
    delete Hit;
  }

  Passed = EvaluateSize("Grid()", "two hits in the depth detector", "There are two x strip hits", XStrips.size(), 2) && Passed;
  Passed = EvaluateSize("Grid()", "two hits in the depth detector", "There are two y strip hits", YStrips.size(), 2) && Passed;
  if (XStrips.size() == 2 && YStrips.size() == 2) {
    Passed = Evaluate("SetStripID()", "first hit", "The x strip is 5", XStrips[0]->GetStripID(), 5) && Passed;
    Passed = Evaluate("SetStripID()", "second hit", "The x strip is 6", XStrips[1]->GetStripID(), 6) && Passed;
    Passed = Evaluate("SetStripID()", "first hit", "The y strip is 2", YStrips[0]->GetStripID(), 2) && Passed;
    Passed = Evaluate("SetStripID()", "second hit", "The y strip is 2", YStrips[1]->GetStripID(), 2) && Passed;
    Passed = Evaluate("SetDetectorID()", "same wafer", "Both hits have the same detector ID", XStrips[0]->GetDetectorID(), XStrips[1]->GetDetectorID()) && Passed;
    Passed = EvaluateTrue("AreStripsAdjacent()", "x strips 5 and 6", "The x strips are adjacent", XStrips[0]->AreStripsAdjacent(XStrips[1])) && Passed;
    Passed = EvaluateFalse("AreStripsAdjacent()", "y strips 2 and 2", "The same y strip is not adjacent to itself", YStrips[0]->AreStripsAdjacent(YStrips[1])) && Passed;
    Passed = EvaluateFalse("AreStripsAdjacent()", "x strip 5 and y strip 2", "Strips of different sides are not adjacent", XStrips[0]->AreStripsAdjacent(YStrips[0])) && Passed;
    Passed = Evaluate("SetGridPoint()", "x strip", "The grid point of the strip hit is the x strip", XStrips[0]->GetGridPoint().GetType(), MDGridPoint::c_XStrip) && Passed;
    Passed = Evaluate("SetDetector()", "x strip", "The detector type is the one of a 3D strip detector", XStrips[0]->GetDetector(), MDDetector::c_Strip3D) && Passed;
  }

  for (MREStripHit* Strip: XStrips) {
    delete Strip;
  }
  for (MREStripHit* Strip: YStrips) {
    delete Strip;
  }

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

  UTREStripHit Test;
  return Test.Run() == true ? 0 : 1;
}


// UTREStripHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
