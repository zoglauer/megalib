/*
 * UTDStrip2D.cxx
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
using namespace std;

// MEGAlib libs:
#include "UTDDetectorShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the two dimensional strip detector MDStrip2D
class UTDStrip2D : public UTDDetectorBase
{
public:
  //! Default constructor
  UTDStrip2D() : UTDDetectorBase("UTDStrip2D") {}
  //! Default destructor
  virtual ~UTDStrip2D() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the fixture with one wafer: half size 3 x 2 x 1, offset 0.5 x 0.25, 10 x 7 strips - pitch 0.5 x 0.5
  MString GetSingleWafer() const;
  //! Return the fixture with 2 x 3 wafers: half size 1.5 x 1 x 0.5, gap 0.2 x 0.1, no offset, 3 x 4 strips - pitch 1 x 0.5
  MString GetManyWafers(bool WithOffset = false) const;
  //! Return true if a geometry can be used and destroyed after the guard ring of its detector is switched off
  bool IsUsableWithoutGuardRing();

  //! Test the construction, setters, and getters
  bool TestConstruction();
  //! Test the values derived in the validation
  bool TestDerivedValues();
  //! Test the position to grid point conversion and back for one wafer
  bool TestGridOneWafer();
  //! Test the position to grid point conversion and back for many wafers
  bool TestGridManyWafers();
  //! Test the discretization
  bool TestDiscretize();
  //! Test the guard ring handling
  bool TestGuardRing();
  //! Test the blocked channels
  bool TestBlockedChannels();
  //! Test the neighbor search
  bool TestAreNear();
  //! Test the position resolution
  bool TestPositionResolution();
  //! Test the keyword parsing and validation
  bool TestKeywords();
  //! Test the energy loss map
  bool TestEnergyLossMap();
  //! Test the copy and the clone
  bool TestCopy();
  //! Test named detectors with their own settings
  bool TestNamedDetectors();
  //! Test the text output
  bool TestText();
  //! Test the noising via the volume tree
  bool TestVolumeNoise();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDStrip2D::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestDerivedValues() && Passed;
  Passed = TestCommonNoiseChain(GetSingleWafer(), MVector(0.25, 0.25, 0.5)) && Passed;
  Passed = TestGridOneWafer() && Passed;
  Passed = TestGridManyWafers() && Passed;
  Passed = TestDiscretize() && Passed;
  Passed = TestGuardRing() && Passed;
  Passed = TestBlockedChannels() && Passed;
  Passed = TestAreNear() && Passed;
  Passed = TestPositionResolution() && Passed;
  Passed = TestKeywords() && Passed;
  Passed = TestEnergyLossMap() && Passed;
  Passed = TestCopy() && Passed;
  Passed = TestNamedDetectors() && Passed;
  Passed = TestText() && Passed;
  Passed = TestVolumeNoise() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the fixture with one wafer: half size 3 x 2 x 1, offset 0.5 x 0.25, 10 x 7 strips - pitch 0.5 x 0.5
MString UTDStrip2D::GetSingleWafer() const
{
  return GetBoxVolume("Box", 3.0, 2.0, 1.0) + "Strip2D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.5 0.25\nD.StripNumber 10 7\n";
}


////////////////////////////////////////////////////////////////////////////////


//! Return the fixture with 2 x 3 wafers: half size 1.5 x 1 x 0.5, gap 0.2 x 0.1, no offset, 3 x 4 strips - pitch 1 x 0.5
MString UTDStrip2D::GetManyWafers(bool WithOffset) const
{
  MString Body;
  Body += GetBoxVolume("Container", 3.1, 3.1, 0.5);
  Body += "Volume Wafer\nWafer.Material Silicon\nWafer.Shape BRIK 1.5 1.0 0.5\nWafer.Position -1.6 -2.1 0.0\nWafer.Mother Container\n";
  Body += "Wafer.Copy W2\nW2.Position 1.6 -2.1 0.0\nW2.Mother Container\n";
  Body += "Wafer.Copy W3\nW3.Position -1.6 0.0 0.0\nW3.Mother Container\n";
  Body += "Wafer.Copy W4\nW4.Position 1.6 0.0 0.0\nW4.Mother Container\n";
  Body += "Wafer.Copy W5\nW5.Position -1.6 2.1 0.0\nW5.Mother Container\n";
  Body += "Wafer.Copy W6\nW6.Position 1.6 2.1 0.0\nW6.Mother Container\n";
  Body += "Strip2D D\nD.SensitiveVolume Wafer\nD.DetectorVolume Container\nD.StructuralPitch 0.2 0.1 0.0\nD.StructuralOffset 0.0 0.0 0.0\nD.StripNumber 3 4\n";
  if (WithOffset == true) {
    Body += "D.Offset 0.3 0.2\n";
  } else {
    Body += "D.Offset 0.0 0.0\n";
  }
  return Body;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a geometry can be used and destroyed after the guard ring of its detector is switched off
bool UTDStrip2D::IsUsableWithoutGuardRing()
{
  MDGeometry* Local = new MDGeometry();
  if (Scan(*Local, GetSingleWafer(), "D.GuardRingEnergyResolution 100 1\nD.GuardRingEnergyResolution 200 1\n") == false) {
    delete Local;
    return false;
  }
  dynamic_cast<MDStrip2D*>(Local->GetDetector("D"))->HasGuardRing(false);
  MString Name = Local->GetDetectorAt(Local->GetNDetectors() - 1)->GetName();
  delete Local;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the construction, setters, and getters
bool UTDStrip2D::TestConstruction()
{
  bool Passed = true;

  MDStrip2D Strip("Strip");
  Passed = EvaluateTrue("MDStrip2D()", "name", "The name is stored", Strip.GetName() == "Strip") && Passed;
  Passed = Evaluate("MDStrip2D()", "type", "The type is Strip2D", Strip.GetType(), MDDetector::c_Strip2D) && Passed;
  Passed = EvaluateTrue("MDStrip2D()", "type name", "The type name is Strip2D", Strip.GetTypeName() == MDDetector::c_Strip2DName) && Passed;
  Passed = Evaluate("MDStrip2D()", "orientation", "The default orientation is the z-axis", Strip.GetOrientation(), 2) && Passed;
  Passed = EvaluateFalse("MDStrip2D()", "guard ring", "There is no guard ring", Strip.HasGuardRing()) && Passed;
  Passed = Evaluate("MDStrip2D()", "strips x", "The number of strips is not defined", Strip.GetNStripsX(), g_IntNotDefined) && Passed;
  Passed = Evaluate("MDStrip2D()", "strips y", "The number of strips is not defined", Strip.GetNStripsY(), g_IntNotDefined) && Passed;
  Passed = Evaluate("MDStrip2D()", "wafers x", "The number of wafers is not defined", Strip.GetNWafersX(), g_IntNotDefined) && Passed;
  Passed = Evaluate("MDStrip2D()", "wafers y", "The number of wafers is not defined", Strip.GetNWafersY(), g_IntNotDefined) && Passed;
  Passed = EvaluateNear("MDStrip2D()", "width x", "The width is not defined", Strip.GetWidthX(), g_DoubleNotDefined, 1e-9*g_DoubleNotDefined) && Passed;

  Strip.SetWidth(1.5, 2.5);
  Passed = EvaluateNear("SetWidth()", "x", "The x value is stored", Strip.GetWidthX(), 1.5, 1e-12) && Passed;
  Passed = EvaluateNear("SetWidth()", "y", "The y value is stored", Strip.GetWidthY(), 2.5, 1e-12) && Passed;
  Strip.SetOffset(3.5, 4.5);
  Passed = EvaluateNear("SetOffset()", "x", "The x value is stored", Strip.GetOffsetX(), 3.5, 1e-12) && Passed;
  Passed = EvaluateNear("SetOffset()", "y", "The y value is stored", Strip.GetOffsetY(), 4.5, 1e-12) && Passed;
  Strip.SetPitch(5.5, 6.5);
  Passed = EvaluateNear("SetPitch()", "x", "The x value is stored", Strip.GetPitchX(), 5.5, 1e-12) && Passed;
  Passed = EvaluateNear("SetPitch()", "y", "The y value is stored", Strip.GetPitchY(), 6.5, 1e-12) && Passed;
  Strip.SetStripLength(7.5, 8.5);
  Passed = EvaluateNear("SetStripLength()", "x", "The x value is stored", Strip.GetStripLengthX(), 7.5, 1e-12) && Passed;
  Passed = EvaluateNear("SetStripLength()", "y", "The y value is stored", Strip.GetStripLengthY(), 8.5, 1e-12) && Passed;
  Strip.SetNStrips(9, 10);
  Passed = Evaluate("SetNStrips()", "x", "The x value is stored", Strip.GetNStripsX(), 9) && Passed;
  Passed = Evaluate("SetNStrips()", "y", "The y value is stored", Strip.GetNStripsY(), 10) && Passed;
  Strip.SetOrientation(1);
  Passed = Evaluate("SetOrientation()", "1", "The value is stored", Strip.GetOrientation(), 1) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the values derived in the validation
bool UTDStrip2D::TestDerivedValues()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "one wafer", "The fixture is accepted", Scan(Geometry, GetSingleWafer())) && Passed;
  if (Passed == false) {
    return false;
  }
  MDStrip2D* D = dynamic_cast<MDStrip2D*>(Geometry.GetDetector("D"));
  Passed = EvaluateTrue("GetDetector()", "one wafer", "The detector is a strip detector", D != nullptr) && Passed;

  Passed = EvaluateNear("Validate()", "width x", "The width is twice the half size", D->GetWidthX(), 6.0, 1e-12) && Passed;
  Passed = EvaluateNear("Validate()", "width y", "The width is twice the half size", D->GetWidthY(), 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("Validate()", "strip length x", "The active length is the width minus twice the offset", D->GetStripLengthX(), 5.0, 1e-12) && Passed;
  Passed = EvaluateNear("Validate()", "strip length y", "The active length is the width minus twice the offset", D->GetStripLengthY(), 3.5, 1e-12) && Passed;
  Passed = EvaluateNear("Validate()", "pitch x", "The pitch is 5/10", D->GetPitchX(), 0.5, 1e-12) && Passed;
  Passed = EvaluateNear("Validate()", "pitch y", "The pitch is 3.5/7", D->GetPitchY(), 0.5, 1e-12) && Passed;
  Passed = EvaluateVectorNear("GetStructuralSize()", "one wafer", "The structural size is the half size of the sensitive volume", D->GetStructuralSize(), MVector(3.0, 2.0, 1.0), 1e-12) && Passed;

  // Defect GEO-D-023: The number of wafers uses the sum of the half sizes without the last pitch, so that a detector volume completely filled with wafers gets one wafer too many
  Passed = ExpectedToFail(Evaluate("GetNWafersX()", "one wafer filling the detector volume", "There is one wafer in x", D->GetNWafersX(), 1)) && Passed;
  Passed = ExpectedToFail(Evaluate("GetNWafersY()", "one wafer filling the detector volume", "There is one wafer in y", D->GetNWafersY(), 1)) && Passed;

  MDGeometry Many;
  Passed = EvaluateTrue("ScanSetupFile()", "many wafers", "The fixture is accepted", Scan(Many, GetManyWafers())) && Passed;
  if (CheckDetector(Passed, "Many", Many, "D", MDDetector::c_Strip2D)) {
    D = dynamic_cast<MDStrip2D*>(Many.GetDetector("D"));
    Passed = Evaluate("GetNWafersX()", "two wafers with a gap", "There are two wafers in x", D->GetNWafersX(), 2) && Passed;
    Passed = Evaluate("GetNWafersY()", "three wafers with a gap", "There are three wafers in y", D->GetNWafersY(), 3) && Passed;
    Passed = EvaluateNear("Validate()", "pitch x", "The pitch is 3/3", D->GetPitchX(), 1.0, 1e-12) && Passed;
    Passed = EvaluateNear("Validate()", "pitch y", "The pitch is 2/4", D->GetPitchY(), 0.5, 1e-12) && Passed;
    Passed = EvaluateVectorNear("GetStructuralPitch()", "many wafers", "The pitch is read", D->GetStructuralPitch(), MVector(0.2, 0.1, 0.0), 1e-12) && Passed;
    Passed = EvaluateVectorNear("GetStructuralOffset()", "many wafers", "The offset is read", D->GetStructuralOffset(), MVector(0.0, 0.0, 0.0), 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the position to grid point conversion and back for one wafer
bool UTDStrip2D::TestGridOneWafer()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "one wafer", "The fixture is accepted", Scan(Geometry, GetSingleWafer())) && Passed;
  if (Passed == false) {
    return false;
  }
  MDStrip2D* D = dynamic_cast<MDStrip2D*>(Geometry.GetDetector("D"));
  MDVolume* V = Geometry.GetVolume("Box");

  // All strip centers by hand: x = -2.5 + (i + 0.5)*0.5, y = -1.75 + (j + 0.5)*0.5, z is the center of the wafer
  for (unsigned int i = 0; i < 10; ++i) {
    for (unsigned int j = 0; j < 7; ++j) {
      MVector Center(-2.5 + (i + 0.5)*0.5, -1.75 + (j + 0.5)*0.5, 0.0);
      Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "strip center", "The center of the strip is returned", D->GetPositionInDetectorVolume(i, j, 0, MVector(0.0, 0.0, 0.0), MDGridPoint::c_Voxel, V), Center, 1e-9) && Passed;
      MDGridPoint Point = D->GetGridPoint(Center);
      Passed = Evaluate("GetGridPoint()", "strip center x", "The strip in x is found", Point.GetXGrid(), i) && Passed;
      Passed = Evaluate("GetGridPoint()", "strip center y", "The strip in y is found", Point.GetYGrid(), j) && Passed;
      Passed = Evaluate("GetGridPoint()", "strip center z", "A strip detector has no z-grid", Point.GetZGrid(), 0u) && Passed;
      Passed = Evaluate("GetGridPoint()", "strip center type", "The type is voxel", Point.GetType(), MDGridPoint::c_Voxel) && Passed;
    }
  }

  // Just inside and outside of a strip boundary
  Passed = Evaluate("GetGridPoint()", "just below the boundary", "The hit is in strip 4", D->GetGridPoint(MVector(-0.0001, 0.0, 0.0)).GetXGrid(), 4u) && Passed;
  Passed = Evaluate("GetGridPoint()", "just above the boundary", "The hit is in strip 5", D->GetGridPoint(MVector(0.0001, 0.0, 0.0)).GetXGrid(), 5u) && Passed;

  // Just inside and outside of the active area
  Passed = Evaluate("GetGridPoint()", "inside the lowest strip", "The hit is in the first strip", D->GetGridPoint(MVector(-2.4999, 0.0, 0.0)).GetXGrid(), 0u) && Passed;
  Passed = Evaluate("GetGridPoint()", "inside the lowest strip", "The hit is a voxel", D->GetGridPoint(MVector(-2.4999, 0.0, 0.0)).GetType(), MDGridPoint::c_Voxel) && Passed;
  Passed = Evaluate("GetGridPoint()", "outside the lowest strip", "The hit is in the guard ring", D->GetGridPoint(MVector(-2.5001, 0.0, 0.0)).GetType(), MDGridPoint::c_GuardRing) && Passed;
  Passed = Evaluate("GetGridPoint()", "inside the highest strip", "The hit is in the last strip", D->GetGridPoint(MVector(2.4999, 0.0, 0.0)).GetXGrid(), 9u) && Passed;
  Passed = Evaluate("GetGridPoint()", "outside the highest strip", "The hit is in the guard ring", D->GetGridPoint(MVector(2.5001, 0.0, 0.0)).GetType(), MDGridPoint::c_GuardRing) && Passed;
  Passed = Evaluate("GetGridPoint()", "inside the highest y-strip", "The hit is in the last strip", D->GetGridPoint(MVector(0.0, 1.7499, 0.0)).GetYGrid(), 6u) && Passed;
  Passed = Evaluate("GetGridPoint()", "outside the highest y-strip", "The hit is in the guard ring", D->GetGridPoint(MVector(0.0, 1.7501, 0.0)).GetType(), MDGridPoint::c_GuardRing) && Passed;
  Passed = Evaluate("GetGridPoint()", "in a guard ring corner", "The hit is in the guard ring", D->GetGridPoint(MVector(-2.9, -1.9, 0.0)).GetType(), MDGridPoint::c_GuardRing) && Passed;

  // Exactly on the boundary of the active area
  DisableDefaultStreams();
  unsigned int TypeOnUpperEdge = D->GetGridPoint(MVector(2.5, 0.0, 0.0)).GetType();
  EnableDefaultStreams();
  // Defect GEO-D-033: A hit exactly on the upper boundary of the active area gets the strip number N, which is not valid, and the hit is lost
  Passed = ExpectedToFail(Evaluate("GetGridPoint()", "exactly on the upper edge of the active area", "The hit belongs to the last strip", TypeOnUpperEdge, MDGridPoint::c_Voxel)) && Passed;

  // Outside of the wafer
  DisableDefaultStreams();
  unsigned int TypeBelow = D->GetGridPoint(MVector(-3.5, 0.0, 0.0)).GetType();
  unsigned int TypeBelowY = D->GetGridPoint(MVector(0.0, -2.5, 0.0)).GetType();
  unsigned int TypeFarBelow = D->GetGridPoint(MVector(-30.0, 0.0, 0.0)).GetType();
  unsigned int TypeFarAbove = D->GetGridPoint(MVector(30.0, 0.0, 0.0)).GetType();
  unsigned int TypeAbove = D->GetGridPoint(MVector(3.6, 0.0, 0.0)).GetType();
  unsigned int TypeAboveY = D->GetGridPoint(MVector(0.0, 2.5, 0.0)).GetType();
  EnableDefaultStreams();
  Passed = Evaluate("GetGridPoint()", "below the wafer in x", "A hit outside of the wafer is unknown", TypeBelow, MDGridPoint::c_Unknown) && Passed;
  Passed = Evaluate("GetGridPoint()", "below the wafer in y", "A hit outside of the wafer is unknown", TypeBelowY, MDGridPoint::c_Unknown) && Passed;
  Passed = Evaluate("GetGridPoint()", "far below the wafer", "A hit outside of the wafer is unknown", TypeFarBelow, MDGridPoint::c_Unknown) && Passed;
  Passed = Evaluate("GetGridPoint()", "far above the wafer", "A hit outside of the wafer is unknown", TypeFarAbove, MDGridPoint::c_Unknown) && Passed;
  // Defect GEO-D-025: Consequence of the wrong number of wafers: a hit outside of the wafer is mapped to strips of a wafer which does not exist
  Passed = ExpectedToFail(Evaluate("GetGridPoint()", "above the wafer in x", "A hit outside of the wafer is unknown", TypeAbove, MDGridPoint::c_Unknown)) && Passed;
  Passed = ExpectedToFail(Evaluate("GetGridPoint()", "above the wafer in y", "A hit outside of the wafer is unknown", TypeAboveY, MDGridPoint::c_Unknown)) && Passed;

  // A wafer without a guard ring, i.e. an active area up to the edge
  MDGeometry NoOffset;
  Passed = EvaluateTrue("ScanSetupFile()", "no offset", "The fixture is accepted", Scan(NoOffset, GetBoxVolume("Box", 2.0, 2.0, 1.0), "Strip2D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.0 0.0\nD.StripNumber 4 8\n")) && Passed;
  if (CheckDetector(Passed, "NoOffset", NoOffset, "D", MDDetector::c_Strip2D)) {
    D = dynamic_cast<MDStrip2D*>(NoOffset.GetDetector("D"));
    Passed = EvaluateFalse("HasGuardRing()", "no offset", "Without an offset there is no guard ring", D->HasGuardRing()) && Passed;
    Passed = Evaluate("GetGridPoint()", "negative edge", "The hit on the negative edge is in the first strip", D->GetGridPoint(MVector(-2.0, 0.0, 0.0)).GetXGrid(), 0u) && Passed;
    // Defect GEO-D-025: Consequence of the wrong number of wafers: a hit exactly on the positive edge is in the first strip of a wafer which does not exist
    Passed = ExpectedToFail(Evaluate("GetGridPoint()", "positive edge", "The hit on the positive edge is in the last strip", D->GetGridPoint(MVector(2.0, 0.0, 0.0)).GetXGrid(), 3u)) && Passed;
    Passed = Evaluate("GetGridPoint()", "z is free", "The z coordinate does not matter", D->GetGridPoint(MVector(-1.5, 0.0, 0.9)).GetXGrid(), 0u) && Passed;
  }

  // One strip only: the pitch is the width
  MDGeometry OneStrip;
  Passed = EvaluateTrue("ScanSetupFile()", "one strip", "The fixture is accepted", Scan(OneStrip, GetBoxVolume("Box", 2.0, 2.0, 1.0), "Strip2D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.0 0.0\nD.StripNumber 1 1\n")) && Passed;
  if (CheckDetector(Passed, "OneStrip", OneStrip, "D", MDDetector::c_Strip2D)) {
    D = dynamic_cast<MDStrip2D*>(OneStrip.GetDetector("D"));
    Passed = EvaluateNear("Validate()", "one strip", "The pitch is the full width", D->GetPitchX(), 4.0, 1e-12) && Passed;
    Passed = Evaluate("GetGridPoint()", "one strip", "Everything is in the strip", D->GetGridPoint(MVector(1.9, -1.9, 0.0)).GetXGrid(), 0u) && Passed;
    Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "one strip", "The position is the center of the wafer", D->GetPositionInDetectorVolume(0, 0, 0, MVector(0.0, 0.0, 0.0), MDGridPoint::c_Voxel, OneStrip.GetVolume("Box")), MVector(0.0, 0.0, 0.0), 1e-9) && Passed;
  }

  // DetermineStrips
  MDGeometry Determine;
  Passed = EvaluateTrue("ScanSetupFile()", "one wafer", "The fixture is accepted", Scan(Determine, GetSingleWafer())) && Passed;
  if (CheckDetector(Passed, "Determine", Determine, "D", MDDetector::c_Strip2D)) {
    D = dynamic_cast<MDStrip2D*>(Determine.GetDetector("D"));
    int X = -1;
    int Y = -1;
    DisableDefaultStreams();
    bool Found = D->DetermineStrips(MVector(0.25, 0.25, 0.0), X, Y);
    EnableDefaultStreams();
    Passed = EvaluateTrue("DetermineStrips()", "inside", "The strips are found", Found) && Passed;
    // x: (0.25 + 2.5)/0.5 = 5.5, y: (0.25 + 1.75)/0.5 = 4
    Passed = Evaluate("DetermineStrips()", "inside", "The x-strip is correct", X, 5) && Passed;
    Passed = Evaluate("DetermineStrips()", "inside", "The y-strip is correct", Y, 4) && Passed;
    DisableDefaultStreams();
    Found = D->DetermineStrips(MVector(-3.5, 0.0, 0.0), X, Y);
    EnableDefaultStreams();
    Passed = EvaluateFalse("DetermineStrips()", "outside", "Nothing is found outside of the wafer", Found) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the position to grid point conversion and back for many wafers
bool UTDStrip2D::TestGridManyWafers()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "many wafers", "The fixture is accepted", Scan(Geometry, GetManyWafers())) && Passed;
  if (Passed == false) {
    return false;
  }
  MDStrip2D* D = dynamic_cast<MDStrip2D*>(Geometry.GetDetector("D"));
  MDVolume* V = Geometry.GetVolume("Container");

  // Strip 1 of wafer 1 in x, i.e. grid 4 - wafer center is -3.1 + 1.5 + 3.2 = 1.6, strip center is 0 relative to the wafer center
  // Strip 1 of wafer 2 in y, i.e. grid 9 - wafer center is -3.1 + 1.0 + 2*2.1 = 2.1, strip center is -0.25 relative to the wafer center
  Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "wafer 1, 2", "The position by hand is returned", D->GetPositionInDetectorVolume(4, 9, 0, MVector(0.0, 0.0, 0.0), MDGridPoint::c_Voxel, V), MVector(1.6, 1.85, 0.0), 1e-9) && Passed;

  // The round trip for all strips
  for (unsigned int i = 0; i < 6; ++i) {
    for (unsigned int j = 0; j < 12; ++j) {
      MVector Position = D->GetPositionInDetectorVolume(i, j, 0, MVector(0.0, 0.0, 0.0), MDGridPoint::c_Voxel, V);
      MDGridPoint Point = D->GetGridPoint(Position);
      Passed = Evaluate("GetGridPoint()", "round trip x", "The round trip restores the x-strip", Point.GetXGrid(), i) && Passed;
      Passed = Evaluate("GetGridPoint()", "round trip y", "The round trip restores the y-strip", Point.GetYGrid(), j) && Passed;
      Passed = Evaluate("GetGridPoint()", "round trip type", "The round trip gives a voxel", Point.GetType(), MDGridPoint::c_Voxel) && Passed;
    }
  }

  // A position in the gap between the wafers: x in 0.1 is 0.1 above the upper edge of the first wafer at -0.1 + ... = 0.0 +- 0.1
  DisableDefaultStreams();
  unsigned int TypeGap = D->GetGridPoint(MVector(-0.05, 0.0, 0.0)).GetType();
  EnableDefaultStreams();
  Passed = Evaluate("GetGridPoint()", "gap between wafers", "A hit in the gap is unknown", TypeGap, MDGridPoint::c_Unknown) && Passed;

  // Structural offset of 0.1 in x: Everything moves by 0.1
  MString Shifted;
  Shifted += GetBoxVolume("Container", 3.2, 3.1, 0.5);
  Shifted += "Volume Wafer\nWafer.Material Silicon\nWafer.Shape BRIK 1.5 1.0 0.5\nWafer.Position -1.6 -2.1 0.0\nWafer.Mother Container\n";
  Shifted += "Strip2D D\nD.SensitiveVolume Wafer\nD.DetectorVolume Container\nD.StructuralPitch 0.2 0.1 0.0\nD.StructuralOffset 0.1 0.0 0.0\nD.StripNumber 3 4\nD.Offset 0.0 0.0\n";
  MDGeometry WithOffset;
  Passed = EvaluateTrue("ScanSetupFile()", "structural offset", "The fixture is accepted", Scan(WithOffset, Shifted)) && Passed;
  if (CheckDetector(Passed, "WithOffset", WithOffset, "D", MDDetector::c_Strip2D)) {
    D = dynamic_cast<MDStrip2D*>(WithOffset.GetDetector("D"));
    // Strip 0: The first wafer starts at -3.2 + 0.1 = -3.1 and the strip center is 0.5 above it
    Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "structural offset", "The first strip moved by the offset", D->GetPositionInDetectorVolume(0, 0, 0, MVector(0.0, 0.0, 0.0), MDGridPoint::c_Voxel, WithOffset.GetVolume("Container")), MVector(-2.6, -2.85, 0.0), 1e-9) && Passed;
    Passed = Evaluate("GetGridPoint()", "structural offset", "The strip is restored", D->GetGridPoint(MVector(-2.6, -2.85, 0.0)).GetXGrid(), 0u) && Passed;
    DisableDefaultStreams();
    unsigned int TypeInOffset = D->GetGridPoint(MVector(-3.15, 0.0, 0.0)).GetType();
    EnableDefaultStreams();
    Passed = Evaluate("GetGridPoint()", "in the structural offset", "A hit in the structural offset is unknown", TypeInOffset, MDGridPoint::c_Unknown) && Passed;
  }

  // Wafers with their own guard rings
  MDGeometry Offset;
  Passed = EvaluateTrue("ScanSetupFile()", "many wafers with offsets", "The fixture is accepted", Scan(Offset, GetManyWafers(true))) && Passed;
  if (CheckDetector(Passed, "Offset", Offset, "D", MDDetector::c_Strip2D)) {
    D = dynamic_cast<MDStrip2D*>(Offset.GetDetector("D"));
    V = Offset.GetVolume("Container");
    // Pitches: (3 - 0.6)/3 = 0.8, (2 - 0.4)/4 = 0.4
    Passed = EvaluateNear("Validate()", "pitch x", "The pitch is 2.4/3", D->GetPitchX(), 0.8, 1e-12) && Passed;
    Passed = EvaluateNear("Validate()", "pitch y", "The pitch is 1.6/4", D->GetPitchY(), 0.4, 1e-12) && Passed;
    // Grid 4: wafer center 1.6, strip 1 center is the center of the wafer, grid 9: wafer center 2.1, strip 1 is -0.2 from the center
    Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "wafer 1, 2", "The position by hand is returned", D->GetPositionInDetectorVolume(4, 9, 0, MVector(0.0, 0.0, 0.0), MDGridPoint::c_Voxel, V), MVector(1.6, 1.9, 0.0), 1e-9) && Passed;
    for (unsigned int i = 0; i < 6; ++i) {
      for (unsigned int j = 0; j < 12; ++j) {
        MDGridPoint Point = D->GetGridPoint(D->GetPositionInDetectorVolume(i, j, 0, MVector(0.0, 0.0, 0.0), MDGridPoint::c_Voxel, V));
        Passed = Evaluate("GetGridPoint()", "round trip x with offset", "The round trip restores the x-strip", Point.GetXGrid(), i) && Passed;
        Passed = Evaluate("GetGridPoint()", "round trip y with offset", "The round trip restores the y-strip", Point.GetYGrid(), j) && Passed;
      }
    }
    // Upper edge of the first wafer x = -0.1: in the guard ring of the first wafer
    Passed = Evaluate("GetGridPoint()", "guard ring of the first wafer", "The hit is in a guard ring", D->GetGridPoint(MVector(-0.15, 0.0, 0.0)).GetType(), MDGridPoint::c_GuardRing) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the discretization
bool UTDStrip2D::TestDiscretize()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "one wafer", "The fixture is accepted", Scan(Geometry, GetSingleWafer())) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDetector* D = Geometry.GetDetector("D");
  MDVolume* V = Geometry.GetVolume("Box");

  vector<MDGridPoint> Points = D->Grid(MVector(0.25, 0.25, 0.5), 123.0, 45.0, V);
  Passed = EvaluateSize("Grid()", "active area", "There is one grid point", Points.size(), 1) && Passed;
  if (Points.size() == 1) {
    Passed = Evaluate("Grid()", "x-strip", "The x-strip is correct", Points[0].GetXGrid(), 5u) && Passed;
    Passed = Evaluate("Grid()", "y-strip", "The y-strip is correct", Points[0].GetYGrid(), 4u) && Passed;
    Passed = EvaluateNear("Grid()", "energy", "The energy is stored", Points[0].GetEnergy(), 123.0, 1e-12) && Passed;
    Passed = EvaluateNear("Grid()", "time", "The time is stored", Points[0].GetTime(), 45.0, 1e-12) && Passed;
  }

  Points = D->Grid(MVector(2.75, 0.0, 0.0), 123.0, 45.0, V);
  Passed = EvaluateSize("Grid()", "guard ring", "There is one grid point in the guard ring", Points.size(), 1) && Passed;
  if (Points.size() == 1) {
    Passed = Evaluate("Grid()", "guard ring", "The type is guard ring", Points[0].GetType(), MDGridPoint::c_GuardRing) && Passed;
  }

  DisableDefaultStreams();
  Points = D->Grid(MVector(-3.5, 0.0, 0.0), 123.0, 45.0, V);
  EnableDefaultStreams();
  Passed = EvaluateSize("Grid()", "outside", "There is no grid point outside of the wafer", Points.size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the guard ring handling
bool UTDStrip2D::TestGuardRing()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "one wafer", "The fixture is accepted", Scan(Geometry, GetSingleWafer())) && Passed;
  if (Passed == false) {
    return false;
  }
  MDStrip2D* D = dynamic_cast<MDStrip2D*>(Geometry.GetDetector("D"));
  MDVolume* V = Geometry.GetVolume("Box");

  Passed = EvaluateTrue("HasGuardRing()", "with offset", "An offset creates a guard ring", D->HasGuardRing()) && Passed;
  Passed = EvaluateTrue("GetGuardRing()", "with offset", "The guard ring exists", D->GetGuardRing() != nullptr) && Passed;
  const MDDetector* ConstD = D;
  Passed = EvaluateTrue("GetGuardRing() const", "with offset", "The const version returns the same guard ring", ConstD->GetGuardRing() == D->GetGuardRing()) && Passed;
  if (D->GetGuardRing() == nullptr) {
    return false;
  }
  Passed = EvaluateFalse("IsActive()", "no guard ring keywords", "The automatically created guard ring is passive", D->GetGuardRing()->IsActive()) && Passed;
  // Middle of the guard ring: -3 + 0.5/2 and -2 + 0.25/2
  Passed = EvaluateVectorNear("GetUniquePosition()", "with offset", "The position is in the middle of the guard ring band", D->GetGuardRing()->GetUniquePosition(), MVector(-2.75, -1.875, 0.0), 1e-9) && Passed;

  // The guard ring position in the detector volume
  Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "guard ring", "The unique position is returned for guard ring hits", D->GetPositionInDetectorVolume(0, 0, 0, MVector(0.0, 0.0, 0.0), MDGridPoint::c_GuardRing, V), MVector(-2.75, -1.875, 0.0), 1e-9) && Passed;
  Passed = Evaluate("GetGridPoint()", "the unique position", "The unique position is a guard ring position", D->GetGridPoint(MVector(-2.75, -1.875, 0.0)).GetType(), MDGridPoint::c_GuardRing) && Passed;

  // Removing and adding
  MDStrip2D Direct("Direct");
  Direct.HasGuardRing(true);
  Passed = EvaluateTrue("HasGuardRing(bool)", "true", "The guard ring exists", Direct.HasGuardRing()) && Passed;
  MDGuardRing* First = Direct.GetGuardRing();
  Passed = EvaluateTrue("HasGuardRing(bool)", "true", "The guard ring has the name of the detector", First->GetName() == "Direct_GuardRing") && Passed;
  Direct.HasGuardRing(true);
  Passed = EvaluateTrue("HasGuardRing(bool)", "second true", "The existing guard ring stays", Direct.GetGuardRing() == First) && Passed;
  Direct.HasGuardRing(false);
  Passed = EvaluateFalse("HasGuardRing(bool)", "false", "The guard ring is gone", Direct.HasGuardRing()) && Passed;
  Passed = EvaluateTrue("HasGuardRing(bool)", "false", "The guard ring pointer is empty", Direct.GetGuardRing() == nullptr) && Passed;

  // Without a guard ring
  MDGeometry NoRing;
  Passed = EvaluateTrue("ScanSetupFile()", "no offset", "The fixture is accepted", Scan(NoRing, GetBoxVolume("Box", 2.0, 2.0, 1.0), "Strip2D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.0 0.0\nD.StripNumber 4 8\n")) && Passed;
  if (CheckDetector(Passed, "NoRing", NoRing, "D", MDDetector::c_Strip2D)) {
    DisableDefaultStreams();
    MVector Position = NoRing.GetDetector("D")->GetPositionInDetectorVolume(0, 0, 0, MVector(0.0, 0.0, 0.0), MDGridPoint::c_GuardRing, NoRing.GetVolume("Box"));
    EnableDefaultStreams();
    Passed = EvaluateVectorNear("GetPositionInDetectorVolume()", "guard ring type without a guard ring", "The center of the volume is returned", Position, MVector(0.0, 0.0, 0.0), 1e-9) && Passed;
  }
  Passed = EvaluateFalse("ScanSetupFile()", "guard ring keywords without offset", "Guard ring keywords without an offset are rejected", IsScanOK(GetBoxVolume("Box", 2.0, 2.0, 1.0), "Strip2D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.0 0.0\nD.StripNumber 4 8\nD.GuardRingEnergyResolution 100 1\nD.GuardRingEnergyResolution 200 1\n")) && Passed;

  // Two wafers far apart: the search for the position of the guard ring looks at the wafer frame position in the detector volume frame
  MString FarApart;
  FarApart += GetBoxVolume("Container", 4.5, 1.0, 0.5);
  FarApart += "Volume Wafer\nWafer.Material Silicon\nWafer.Shape BRIK 1.0 1.0 0.5\nWafer.Position -3.5 0.0 0.0\nWafer.Mother Container\n";
  FarApart += "Wafer.Copy WaferB\nWaferB.Position 3.5 0.0 0.0\nWaferB.Mother Container\n";
  FarApart += "Strip2D D\nD.SensitiveVolume Wafer\nD.DetectorVolume Container\nD.StructuralPitch 5.0 0.0 0.0\nD.StructuralOffset 0.0 0.0 0.0\nD.StripNumber 3 3\nD.Offset 0.2 0.2\n";
  // Defect GEO-D-034: The search of the unique guard ring position uses the frame of the sensitive volume in the detector volume and rejects valid detectors whose wafers are away from the center of the detector volume
  Passed = ExpectedToFail(EvaluateTrue("ScanSetupFile()", "two wafers with a gap larger than the wafers", "A valid structure with a guard ring is accepted", IsScanOK(FarApart))) && Passed;

  // A guard ring band only along one axis
  const MString Box = GetBoxVolume("Box", 2.0, 2.0, 1.0) + "Strip2D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.5 0.0\nD.StripNumber 4 8\n";
  MDGeometry OneAxis;
  Passed = EvaluateTrue("ScanSetupFile()", "offset only in x", "The fixture is accepted", Scan(OneAxis, Box)) && Passed;
  if (CheckDetector(Passed, "OneAxis", OneAxis, "D", MDDetector::c_Strip2D)) {
    Passed = Evaluate("GetGridPoint()", "offset only in x", "A hit in the band is a guard ring hit", OneAxis.GetDetector("D")->GetGridPoint(MVector(1.75, 0.0, 0.0)).GetType(), MDGridPoint::c_GuardRing) && Passed;
    // Defect GEO-D-026: A guard ring is only created if both offsets are nonzero, but the grid points of the band along the one axis are guard ring hits
    Passed = ExpectedToFail(EvaluateTrue("HasGuardRing()", "offset only in x", "There is a guard ring because hits in the band are guard ring hits", OneAxis.GetDetector("D")->HasGuardRing())) && Passed;
  }
  // Defect GEO-D-026: A guard ring is only accepted if both offsets are nonzero
  Passed = ExpectedToFail(EvaluateTrue("ScanSetupFile()", "guard ring keywords with an offset only in x", "A guard ring band along one axis can be configured", IsScanOK(Box, "D.GuardRingEnergyResolution 100 1\nD.GuardRingEnergyResolution 200 1\n"))) && Passed;

  // HasGuardRing(false) must not leave a dangling pointer in the geometry
  // Defect GEO-D-027: HasGuardRing(false) deletes a guard ring which the geometry also owns and has in its list of detectors - use after free and double free
  Passed = ExpectedToCrash(EvaluateTrue("HasGuardRing(bool)", "false after the guard ring was added to the geometry", "The geometry stays usable", IsUsableWithoutGuardRing())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the blocked channels
bool UTDStrip2D::TestBlockedChannels()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "BlockTrigger", "The fixture is accepted", Scan(Geometry, GetSingleWafer(), "D.BlockTrigger 2 3\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDetector* D = Geometry.GetDetector("D");
  Passed = EvaluateFalse("IsAboveTriggerThreshold()", "blocked channel", "The blocked channel does not trigger", D->IsAboveTriggerThreshold(1000.0, MDGridPoint(2, 3, 0))) && Passed;
  Passed = EvaluateTrue("IsAboveTriggerThreshold()", "neighbor in x", "The neighbor triggers", D->IsAboveTriggerThreshold(1000.0, MDGridPoint(3, 3, 0))) && Passed;
  Passed = EvaluateTrue("IsAboveTriggerThreshold()", "neighbor in y", "The neighbor triggers", D->IsAboveTriggerThreshold(1000.0, MDGridPoint(2, 2, 0))) && Passed;
  // x + y*Nx must not be mixed up
  Passed = EvaluateTrue("IsAboveTriggerThreshold()", "swapped indices", "The channel with the swapped indices triggers", D->IsAboveTriggerThreshold(1000.0, MDGridPoint(3, 2, 0))) && Passed;
  Passed = EvaluateTrue("BlockTriggerChannel()", "last channel", "The last channel can be blocked", D->BlockTriggerChannel(9, 6)) && Passed;
  Passed = EvaluateFalse("IsAboveTriggerThreshold()", "last channel", "The last channel does not trigger", D->IsAboveTriggerThreshold(1000.0, MDGridPoint(9, 6, 0))) && Passed;

  // The noise is not influenced
  double Energy = NoiseOnce(D, Geometry.GetVolume("Box"), MVector(0.0, 0.0, 0.0), 100.0);
  Passed = EvaluateNear("Noise()", "blocked channel", "A blocked channel still measures the energy", Energy, 100.0, 1e-12) && Passed;

  // Many wafers
  MDGeometry Many;
  Passed = EvaluateTrue("ScanSetupFile()", "BlockTrigger many wafers", "The fixture is accepted", Scan(Many, GetManyWafers(), "D.BlockTrigger 5 11\n")) && Passed;
  if (CheckDetector(Passed, "Many", Many, "D", MDDetector::c_Strip2D)) {
    Passed = EvaluateFalse("IsAboveTriggerThreshold()", "channel in the last wafer", "The channel of the last wafer is blocked", Many.GetDetector("D")->IsAboveTriggerThreshold(1000.0, MDGridPoint(5, 11, 0))) && Passed;
    Passed = EvaluateTrue("IsAboveTriggerThreshold()", "first channel", "The first channel is not blocked", Many.GetDetector("D")->IsAboveTriggerThreshold(1000.0, MDGridPoint(0, 0, 0))) && Passed;
    DisableDefaultStreams();
    bool OutOfRange = Many.GetDetector("D")->BlockTriggerChannel(6, 0);
    EnableDefaultStreams();
    Passed = EvaluateFalse("BlockTriggerChannel()", "channel behind the last one", "A channel which does not exist is rejected", OutOfRange) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the neighbor search
bool UTDStrip2D::TestAreNear()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "one wafer", "The fixture is accepted", Scan(Geometry, GetSingleWafer())) && Passed;
  if (Passed == false) {
    return false;
  }
  MDDetector* D = Geometry.GetDetector("D");
  MVector Null(0.0, 0.0, 0.0);
  MVector DZ(0.0, 0.0, 0.1);

  // Squared distance in pitches below level + 0.01, with the pitch 0.5
  Passed = EvaluateTrue("AreNear()", "next strip, level 1", "The next strip is near", D->AreNear(Null, Null, MVector(0.5, 0.0, 0.0), Null, -1.0, 1)) && Passed;
  Passed = EvaluateTrue("AreNear()", "just inside", "A distance of 1.004 pitches is near", D->AreNear(Null, Null, MVector(0.502, 0.0, 0.0), Null, -1.0, 1)) && Passed;
  Passed = EvaluateFalse("AreNear()", "just outside", "A distance of 1.006 pitches is not near", D->AreNear(Null, Null, MVector(0.503, 0.0, 0.0), Null, -1.0, 1)) && Passed;
  Passed = EvaluateFalse("AreNear()", "diagonal, level 1", "The diagonal strip is not near at level 1", D->AreNear(Null, Null, MVector(0.5, 0.5, 0.0), Null, -1.0, 1)) && Passed;
  Passed = EvaluateTrue("AreNear()", "diagonal, level 2", "The diagonal strip is near at level 2", D->AreNear(Null, Null, MVector(0.5, 0.5, 0.0), Null, -1.0, 2)) && Passed;
  Passed = EvaluateFalse("AreNear()", "second strip, level 2", "The second next strip is not near at level 2", D->AreNear(Null, Null, MVector(1.0, 0.0, 0.0), Null, -1.0, 2)) && Passed;
  Passed = EvaluateTrue("AreNear()", "second strip, level 4", "The second next strip is near at level 4", D->AreNear(Null, Null, MVector(1.0, 0.0, 0.0), Null, -1.0, 4)) && Passed;
  Passed = EvaluateTrue("AreNear()", "identical strip, level 0", "The same strip is near at level 0", D->AreNear(Null, Null, Null, Null, -1.0, 0)) && Passed;

  // Depth: |dz| below sigma*(dz + 0.01), here with sigma 1 and dz 0.1: 0.11
  Passed = EvaluateTrue("AreNear()", "depth just inside", "A depth difference of 0.105 is near", D->AreNear(Null, DZ, MVector(0.0, 0.0, 0.105), DZ, 1.0, 1)) && Passed;
  Passed = EvaluateFalse("AreNear()", "depth just outside", "A depth difference of 0.115 is not near", D->AreNear(Null, DZ, MVector(0.0, 0.0, 0.115), DZ, 1.0, 1)) && Passed;
  Passed = EvaluateTrue("AreNear()", "depth ignored", "A negative sigma ignores the depth", D->AreNear(Null, DZ, MVector(0.0, 0.0, 0.5), DZ, -1.0, 1)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the position resolution
bool UTDStrip2D::TestPositionResolution()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "one wafer", "The fixture is accepted", Scan(Geometry, GetSingleWafer())) && Passed;
  if (Passed == false) {
    return false;
  }
  MVector Resolution = Geometry.GetDetector("D")->GetPositionResolution(MVector(0.0, 0.0, 0.0), 100.0);

  // Standard deviation of a uniform distribution: (b - a)/sqrt(12)
  Passed = EvaluateNear("GetPositionResolution()", "x", "The resolution is the pitch/sqrt(12)", Resolution.X(), 0.5/sqrt(12.0), 1e-12) && Passed;
  Passed = EvaluateNear("GetPositionResolution()", "y", "The resolution is the pitch/sqrt(12)", Resolution.Y(), 0.5/sqrt(12.0), 1e-12) && Passed;
  Passed = EvaluateNear("GetPositionResolution()", "z", "The resolution is the thickness/sqrt(12)", Resolution.Z(), 2.0*1.0/sqrt(12.0), 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the keyword parsing and validation
bool UTDStrip2D::TestKeywords()
{
  bool Passed = true;

  const MString Volume = GetBoxVolume("Box", 3.0, 2.0, 1.0);
  const MString Head = Volume + "Strip2D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\n";

  Passed = EvaluateTrue("ScanSetupFile()", "complete", "A complete detector is accepted", IsScanOK(Head, "D.Offset 0.1 0.1\nD.StripNumber 2 2\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "alias", "The class name is an alias of the type", IsScanOK(Volume, "MDStrip2D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.1 0.1\nD.StripNumber 2 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "no offset", "A missing offset is rejected", IsScanOK(Head, "D.StripNumber 2 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "no strips", "A missing strip number is rejected", IsScanOK(Head, "D.Offset 0.1 0.1\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "negative x-offset", "A negative offset is rejected", IsScanOK(Head, "D.Offset -0.1 0.1\nD.StripNumber 2 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "negative y-offset", "A negative offset is rejected", IsScanOK(Head, "D.Offset 0.1 -0.1\nD.StripNumber 2 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "zero strips in x", "Zero strips are rejected", IsScanOK(Head, "D.Offset 0.1 0.1\nD.StripNumber 0 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "zero strips in y", "Zero strips are rejected", IsScanOK(Head, "D.Offset 0.1 0.1\nD.StripNumber 2 0\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "negative strips", "Negative numbers of strips are rejected", IsScanOK(Head, "D.Offset 0.1 0.1\nD.StripNumber -1 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "Offset with one value", "The wrong number of tokens is rejected", IsScanOK(Head, "D.Offset 0.1\nD.StripNumber 2 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "Offset with three values", "The wrong number of tokens is rejected", IsScanOK(Head, "D.Offset 0.1 0.1 0.1\nD.StripNumber 2 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "StripNumber with one value", "The wrong number of tokens is rejected", IsScanOK(Head, "D.Offset 0.1 0.1\nD.StripNumber 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "StripNumber with three values", "The wrong number of tokens is rejected", IsScanOK(Head, "D.Offset 0.1 0.1\nD.StripNumber 2 2 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "Voxels", "A keyword of the voxel detector is rejected", IsScanOK(Head, "D.Offset 0.1 0.1\nD.StripNumber 2 2\nD.Voxels 2 2 2\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "DepthResolution", "A keyword of a 3D detector is rejected", IsScanOK(Head, "D.Offset 0.1 0.1\nD.StripNumber 2 2\nD.DepthResolution 100 1\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "GuardRingTriggerThreshold without value", "The wrong number of tokens is rejected", IsScanOK(Head, "D.Offset 0.1 0.1\nD.StripNumber 2 2\nD.GuardRingTriggerThreshold\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "GuardRingEnergyResolution with one value", "The wrong number of tokens is rejected", IsScanOK(Head, "D.Offset 0.1 0.1\nD.StripNumber 2 2\nD.GuardRingEnergyResolution 100 1 1\n")) && Passed;

  // Defect GEO-D-029: An offset larger than half of the wafer results in a negative pitch and is accepted
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "offset larger than the half width", "The offset must leave an active area", IsScanOK(Head, "D.Offset 3.5 0.1\nD.StripNumber 2 2\n"))) && Passed;
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "offset equal to the half width", "The offset must leave an active area", IsScanOK(Head, "D.Offset 3.0 0.1\nD.StripNumber 2 2\n"))) && Passed;

  // Shapes
  const MString Sphere = "Volume Ball\nBall.Material Silicon\nBall.Shape SPHE 0 2 0 180 0 360\nBall.Position 0 0 0\nBall.Mother World\n";
  Passed = EvaluateFalse("ScanSetupFile()", "sphere as detector", "A sphere as detector volume is rejected", IsScanOK(Sphere, "Strip2D D\nD.SensitiveVolume Ball\nD.DetectorVolume Ball\nD.Offset 0.1 0.1\nD.StripNumber 2 2\n")) && Passed;

  MString Boolean;
  Boolean += "Shape Sphere S1\nS1.Parameters 0 0.5\n";
  Boolean += "Shape Sphere S2\nS2.Parameters 0 10.0\n";
  Boolean += "Shape Box B1\nB1.Parameters 3.0 2.0 1.0\n";
  Boolean += "Orientation O1\nO1.Position 0 0 0\n";
  Boolean += "Shape Subtraction Minus\nMinus.Parameters B1 S1 O1\n";
  Boolean += "Shape Subtraction Other\nOther.Parameters S2 B1 O1\n";
  Boolean += "Volume Hole\nHole.Material Silicon\nHole.Shape Minus\nHole.Position 0 0 0\nHole.Mother World\n";
  MDGeometry Subtraction;
  Passed = EvaluateTrue("ScanSetupFile()", "subtraction from a box", "A box with a hole is accepted", Scan(Subtraction, Boolean, "Strip2D D\nD.SensitiveVolume Hole\nD.DetectorVolume Hole\nD.Offset 0.5 0.25\nD.StripNumber 10 7\n")) && Passed;
  if (CheckDetector(Passed, "Subtraction", Subtraction, "D", MDDetector::c_Strip2D)) {
    Passed = EvaluateNear("Validate()", "subtraction from a box", "The pitch is derived from the box", dynamic_cast<MDStrip2D*>(Subtraction.GetDetector("D"))->GetPitchX(), 0.5, 1e-12) && Passed;
  }

  MDGeometry WrongFirst;
  MString BooleanOther = Boolean;
  BooleanOther.ReplaceAll("Hole.Shape Minus", "Hole.Shape Other");
  Passed = EvaluateFalse("ScanSetupFile()", "subtraction from a sphere", "A sphere with a hole is rejected", Scan(WrongFirst, BooleanOther, "Strip2D D\nD.SensitiveVolume Hole\nD.DetectorVolume Hole\nD.Offset 0.5 0.25\nD.StripNumber 10 7\n")) && Passed;

  // Two sensitive volumes
  MString Two;
  Two += GetBoxVolume("Container", 5.0, 5.0, 5.0);
  Two += GetBoxVolume("Part1", 1.0, 1.0, 1.0, 2.0, "Container");
  Two += GetBoxVolume("Part2", 1.0, 1.0, 1.0, -2.0, "Container");
  Two += "Strip2D D\nD.SensitiveVolume Part1\nD.SensitiveVolume Part2\nD.DetectorVolume Container\nD.StructuralPitch 0 0 0\nD.StructuralOffset 0 0 0\nD.Offset 0.1 0.1\nD.StripNumber 2 2\n";
  Passed = EvaluateFalse("ScanSetupFile()", "two sensitive volumes", "Two sensitive volumes are rejected", IsScanOK(Two)) && Passed;

  // Separate detector volume without the structure
  MString Separate;
  Separate += GetBoxVolume("Container", 5.0, 5.0, 5.0);
  Separate += GetBoxVolume("Part", 1.0, 1.0, 1.0, 0.0, "Container");
  Separate += "Strip2D D\nD.SensitiveVolume Part\nD.DetectorVolume Container\nD.Offset 0.1 0.1\nD.StripNumber 2 2\n";
  Passed = EvaluateFalse("ScanSetupFile()", "no structural keywords", "The structural keywords are required for a separate detector volume", IsScanOK(Separate)) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "structural keywords", "The structural keywords are sufficient", IsScanOK(Separate, "D.StructuralPitch 0 0 0\nD.StructuralOffset 0 0 0\n")) && Passed;

  // Validate without any volumes in a direct call
  MDStrip2D NoVolumes("NoVolumes");
  DisableDefaultStreams();
  bool Result = NoVolumes.Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "no volumes", "A strip detector without volumes is invalid", Result) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the energy loss map
bool UTDStrip2D::TestEnergyLossMap()
{
  bool Passed = true;

  // A map with the size of the active area - here 2.5 x 1.75 x 1, and one with another size
  const MString Matching = GetTemporaryFileName("Matching.dat");
  const MString Other = GetTemporaryFileName("Other.dat");
  MString Text;
  vector<double> MatchingX = { -2.5, 2.5 };
  vector<double> MatchingY = { -1.75, 1.75 };
  vector<double> OtherX = { -5.0, 5.0 };
  vector<double> OtherY = { -5.0, 5.0 };
  vector<double> Z = { -1.0, 1.0 };
  auto BuildMap = [&](const vector<double>& X, const vector<double>& Y) {
    ostringstream Out;
    Out<<"XA "<<X[0]<<" "<<X[1]<<endl<<"YA "<<Y[0]<<" "<<Y[1]<<endl<<"ZA "<<Z[0]<<" "<<Z[1]<<endl;
    for (double x: X) {
      for (double y: Y) {
        for (double z: Z) {
          Out<<"DP "<<x<<" "<<y<<" "<<z<<" 0.75"<<endl;
        }
      }
    }
    Out<<"EN"<<endl;
    return MString(Out.str().c_str());
  };
  Passed = EvaluateTrue("WriteTextFile()", "map", "The matching map can be created", WriteTextFile(Matching, BuildMap(MatchingX, MatchingY))) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "map", "The other map can be created", WriteTextFile(Other, BuildMap(OtherX, OtherY))) && Passed;

  MDGeometry GeometryMatching;
  Passed = EvaluateTrue("ScanSetupFile()", "matching map", "The fixture is accepted", Scan(GeometryMatching, GetSingleWafer(), MString("D.EnergyLossMap ") + Matching + "\n")) && Passed;
  if (CheckDetector(Passed, "GeometryMatching", GeometryMatching, "D", MDDetector::c_Strip2D)) {
    Passed = Evaluate("GetEnergyLossType()", "matching map", "The type is map", GeometryMatching.GetDetector("D")->GetEnergyLossType(), MDDetector::c_EnergyLossTypeMap) && Passed;
    Passed = EvaluateNear("GetEnergyLossMap()", "matching map", "The value of the map is returned", GeometryMatching.GetDetector("D")->GetEnergyLossMap().Evaluate(1.0, 1.0, 0.0), 0.75, 1e-9) && Passed;
    // The energy loss is applied in the simulation and not during the noising
    Passed = EvaluateNear("Noise()", "matching map", "The noising does not apply the energy loss", NoiseOnce(GeometryMatching.GetDetector("D"), GeometryMatching.GetVolume("Box"), MVector(1.0, 1.0, 0.0), 100.0), 100.0, 1e-9) && Passed;
  }

  MDGeometry GeometryOther;
  Passed = EvaluateTrue("ScanSetupFile()", "other map", "The fixture is accepted", Scan(GeometryOther, GetSingleWafer(), MString("D.EnergyLossMap ") + Other + "\n")) && Passed;
  if (CheckDetector(Passed, "GeometryOther", GeometryOther, "D", MDDetector::c_Strip2D)) {
    MFunction3D Map = GeometryOther.GetDetector("D")->GetEnergyLossMap();
    Passed = EvaluateNear("Validate()", "other map, x min", "The map is rescaled to the active area", Map.GetXMin(), -2.5, 1e-9) && Passed;
    Passed = EvaluateNear("Validate()", "other map, x max", "The map is rescaled to the active area", Map.GetXMax(), 2.5, 1e-9) && Passed;
    Passed = EvaluateNear("Validate()", "other map, y min", "The map is rescaled to the active area", Map.GetYMin(), -1.75, 1e-9) && Passed;
    Passed = EvaluateNear("Validate()", "other map, y max", "The map is rescaled to the active area", Map.GetYMax(), 1.75, 1e-9) && Passed;
    Passed = EvaluateNear("Validate()", "other map, z min", "The map is rescaled to the thickness", Map.GetZMin(), -1.0, 1e-9) && Passed;
    Passed = EvaluateNear("Validate()", "other map, z max", "The map is rescaled to the thickness", Map.GetZMax(), 1.0, 1e-9) && Passed;
  }

  Passed = EvaluateFalse("ScanSetupFile()", "energy loss map for a named detector", "A named detector inherits the map", IsScanOK(GetSingleWafer(), MString("D.Named N\nN.Assign World.Box\nN.EnergyLossMap ") + Matching + "\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the copy and the clone
bool UTDStrip2D::TestCopy()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "one wafer", "The fixture is accepted", Scan(Geometry, GetSingleWafer(), "D.GuardRingEnergyResolution 100 1\nD.GuardRingEnergyResolution 200 2\nD.GuardRingTriggerThreshold 30 3\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MDStrip2D* D = dynamic_cast<MDStrip2D*>(Geometry.GetDetector("D"));
  D->SetOrientation(1);

  Passed = CheckCloneIndependence("Strip2D", D) && Passed;
  unique_ptr<MDDetector> Cloned(D->Clone());
  MDStrip2D* Copy = dynamic_cast<MDStrip2D*>(Cloned.get());
  if (EvaluateTrue("Clone()", "strip detector", "The clone is a strip detector", Copy != nullptr) == false) {
    return false;
  }
  Passed = EvaluateNear("MDStrip2D(const MDStrip2D&)", "width", "The width is copied", Copy->GetWidthX(), 6.0, 1e-12) && Passed;
  Passed = EvaluateNear("MDStrip2D(const MDStrip2D&)", "width", "The width is copied", Copy->GetWidthY(), 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("MDStrip2D(const MDStrip2D&)", "offset", "The offset is copied", Copy->GetOffsetX(), 0.5, 1e-12) && Passed;
  Passed = EvaluateNear("MDStrip2D(const MDStrip2D&)", "offset", "The offset is copied", Copy->GetOffsetY(), 0.25, 1e-12) && Passed;
  Passed = EvaluateNear("MDStrip2D(const MDStrip2D&)", "pitch", "The pitch is copied", Copy->GetPitchX(), 0.5, 1e-12) && Passed;
  Passed = EvaluateNear("MDStrip2D(const MDStrip2D&)", "pitch", "The pitch is copied", Copy->GetPitchY(), 0.5, 1e-12) && Passed;
  Passed = EvaluateNear("MDStrip2D(const MDStrip2D&)", "strip length", "The strip length is copied", Copy->GetStripLengthX(), 5.0, 1e-12) && Passed;
  Passed = EvaluateNear("MDStrip2D(const MDStrip2D&)", "strip length", "The strip length is copied", Copy->GetStripLengthY(), 3.5, 1e-12) && Passed;
  Passed = Evaluate("MDStrip2D(const MDStrip2D&)", "strips", "The number of strips is copied", Copy->GetNStripsX(), 10) && Passed;
  Passed = Evaluate("MDStrip2D(const MDStrip2D&)", "strips", "The number of strips is copied", Copy->GetNStripsY(), 7) && Passed;
  Passed = Evaluate("MDStrip2D(const MDStrip2D&)", "wafers", "The number of wafers is copied", Copy->GetNWafersX(), D->GetNWafersX()) && Passed;
  Passed = Evaluate("MDStrip2D(const MDStrip2D&)", "wafers", "The number of wafers is copied", Copy->GetNWafersY(), D->GetNWafersY()) && Passed;
  Passed = Evaluate("MDStrip2D(const MDStrip2D&)", "orientation", "The orientation is copied", Copy->GetOrientation(), 1) && Passed;
  Passed = EvaluateTrue("MDStrip2D(const MDStrip2D&)", "guard ring", "The guard ring is copied", Copy->HasGuardRing()) && Passed;
  Passed = EvaluateTrue("MDStrip2D(const MDStrip2D&)", "guard ring", "The guard ring is a new object", Copy->GetGuardRing() != D->GetGuardRing()) && Passed;
  Passed = EvaluateNear("MDStrip2D(const MDStrip2D&)", "guard ring", "The trigger threshold of the guard ring is copied", Copy->GetGuardRing()->GetTriggerThreshold(), 30.0, 1e-12) && Passed;

  // Attention: The guard ring of the copy is not owned by anything
  delete Copy->GetGuardRing();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test named detectors with their own settings
bool UTDStrip2D::TestNamedDetectors()
{
  bool Passed = true;
  MString Flags;

  Passed = CheckCopyDataToNamedDetectors<MDStrip2D>("Strip2D", [](MDStrip2D& D) { D.SetNStrips(3, 4); }, [](MDStrip2D& D) { return D.GetNStripsX() == 3 && D.GetNStripsY() == 4; }) && Passed;
  gRandom->SetSeed(4001);

  // Two copies of a detector volume, each with its own name:
  MString Body;
  Body += "Volume Stack\nStack.Material Vacuum\nStack.Shape BRIK 4.0 4.0 2.0\nStack.Position -5.0 0.0 0.0\nStack.Mother World\n";
  Body += "Stack.Copy StackB\nStackB.Position 5.0 0.0 0.0\nStackB.Mother World\n";
  Body += GetBoxVolume("Wafer", 1.5, 1.0, 0.5, 0.0, "Stack");
  Body += "Strip2D D\nD.SensitiveVolume Wafer\nD.DetectorVolume Wafer\nD.StripNumber 3 2\nD.Offset 0.3 0.2\n";
  Body += "D.EnergyResolution Gauss 100 100 1\nD.EnergyResolution Gauss 200 200 1\nD.NoiseThreshold 10\nD.TriggerThreshold 20\n";
  Body += "D.Named NA\nNA.Assign World.Stack.Wafer\nD.Named NB\nNB.Assign World.StackB.Wafer\n";
  Body += "NA.EnergyResolution Gauss 100 100 3\nNA.EnergyResolution Gauss 200 200 3\nNA.NoiseThreshold 11\n";
  Body += "NB.TriggerThreshold 44\n";

  MDGeometryQuest Geometry;
  bool ScanResult = Scan(Geometry, Body);
  Passed = EvaluateTrue("ScanSetupFile()", "named detectors", "The fixture is accepted", ScanResult) && Passed;
  if (ScanResult == false) {
    return false;
  }

  MDStrip2D* D = dynamic_cast<MDStrip2D*>(Geometry.GetDetector("D"));
  MDStrip2D* NA = dynamic_cast<MDStrip2D*>(Geometry.GetDetector("NA"));
  MDStrip2D* NB = dynamic_cast<MDStrip2D*>(Geometry.GetDetector("NB"));

  Passed = Evaluate("GetNNamedDetectors()", "two names", "Two named detectors exist", D->GetNNamedDetectors(), 2u) && Passed;
  // Inherited strip data:
  Passed = Evaluate("CopyDataToNamedDetectors()", "strips", "The number of strips is inherited", NA->GetNStripsX(), 3) && Passed;
  Passed = Evaluate("CopyDataToNamedDetectors()", "strips", "The number of strips is inherited", NB->GetNStripsY(), 2) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "offset", "The offset is inherited", NA->GetOffsetX(), 0.3, 1e-12) && Passed;
  Passed = EvaluateNear("Validate()", "pitch", "The pitch of the named detector is derived", NB->GetPitchX(), 0.8, 1e-12) && Passed;
  Passed = Evaluate("Validate()", "orientation", "The orientation is inherited", NB->GetOrientation(), 2) && Passed;
  Passed = EvaluateTrue("Validate()", "guard ring", "The named detector has a guard ring", NB->HasGuardRing()) && Passed;
  // Own and inherited settings
  Passed = EvaluateNear("GetNoiseThreshold()", "NA", "The own value is used", NA->GetNoiseThreshold(), 11.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetNoiseThreshold()", "NB", "The inherited value is used", NB->GetNoiseThreshold(), 10.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetTriggerThreshold()", "NA", "The inherited value is used", NA->GetTriggerThreshold(), 20.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetTriggerThreshold()", "NB", "The own value is used", NB->GetTriggerThreshold(), 44.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetEnergyResolutionWidth1()", "NA", "The own table is used", NA->GetEnergyResolutionWidth1(150.0), 3.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetEnergyResolutionWidth1()", "NB", "The inherited table is used", NB->GetEnergyResolutionWidth1(150.0), 1.0, 1e-9) && Passed;

  // The volume tree points to the correct named detector
  MDVolumeSequence SA = Geometry.GetVolumeSequence(MVector(-5.0, 0.0, 0.0), true, true);
  MDVolumeSequence SB = Geometry.GetVolumeSequence(MVector(5.0, 0.0, 0.0), true, true);
  Passed = EvaluateTrue("MDVolumeSequence::GetDetector()", "first wafer", "The first wafer belongs to the first named detector", SA.GetDetector() == NA) && Passed;
  Passed = EvaluateTrue("MDVolumeSequence::GetDetector()", "second wafer", "The second wafer belongs to the second named detector", SB.GetDetector() == NB) && Passed;

  // Noising of the geometry uses the named detector
  vector<double> InA;
  vector<double> InB;
  for (unsigned int i = 0; i < c_Samples; ++i) {
    MVector Position(-5.0, 0.0, 0.0);
    double Energy = 150.0;
    double Time = 0.0;
    Geometry.Noise(Position, Energy, Time, Flags, SA);
    InA.push_back(Energy);
    Position = MVector(5.0, 0.0, 0.0);
    Energy = 150.0;
    Time = 0.0;
    Geometry.Noise(Position, Energy, Time, Flags, SB);
    InB.push_back(Energy);
  }
  Passed = CheckGauss("MDGeometryQuest::Noise()", "first named detector", InA, 150.0, 3.0) && Passed;
  Passed = CheckGauss("MDGeometryQuest::Noise()", "second named detector", InB, 150.0, 1.0) && Passed;

  // The noise of the base detector stays untouched
  Passed = CheckGauss("Noise()", "base detector", SampleEnergy(D, Geometry.GetVolume("Wafer"), MVector(0.0, 0.0, 0.0), 150.0), 150.0, 1.0) && Passed;

  // Trigger
  Passed = EvaluateTrue("IsAboveTriggerThreshold()", "NA, 30 keV", "The first named detector triggers", NA->IsAboveTriggerThreshold(30.0, SA.GetGridPoint())) && Passed;
  Passed = EvaluateFalse("IsAboveTriggerThreshold()", "NB, 30 keV", "The second named detector does not trigger", NB->IsAboveTriggerThreshold(30.0, SB.GetGridPoint())) && Passed;

  // Two named detectors for the same detector volume
  MString SameVolume = GetSingleWafer();
  SameVolume += "D.Named N1\nN1.Assign World.Box\nD.Named N2\nN2.Assign World.Box\n";
  // Defect GEO-D-035: Two named detectors can be assigned to the same detector volume, the settings of the second one are never used
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "two named detectors for one detector volume", "The second assignment to a detector volume is rejected", IsScanOK(SameVolume))) && Passed;

  // Own guard ring settings of a named detector
  MString OwnRing;
  OwnRing += GetSingleWafer();
  OwnRing += "D.Named N\nN.Assign World.Box\nN.GuardRingEnergyResolution 100 1\nN.GuardRingEnergyResolution 200 1\nN.GuardRingTriggerThreshold 30 3\n";
  MDGeometry GeometryRing;
  Passed = EvaluateTrue("ScanSetupFile()", "own guard ring", "The fixture is accepted", Scan(GeometryRing, OwnRing)) && Passed;
  if (GeometryRing.GetDetector("N") != nullptr && GeometryRing.GetDetector("N")->GetGuardRing() != nullptr) {
    // Defect GEO-D-030: The copy to the named detectors replaces their own guard ring with the one of the base detector and thus silently discards its settings
    Passed = ExpectedToFail(EvaluateTrue("CopyDataToNamedDetectors()", "own guard ring settings", "A named detector keeps its active guard ring", GeometryRing.GetDetector("N")->GetGuardRing()->IsActive())) && Passed;
  }

  // Inherited guard ring settings: the named detector gets a clone
  MString BaseRing;
  BaseRing += GetSingleWafer();
  BaseRing += "D.GuardRingEnergyResolution 100 1\nD.GuardRingEnergyResolution 200 1\nD.GuardRingTriggerThreshold 30 3\n";
  BaseRing += "D.Named N\nN.Assign World.Box\n";
  MDGeometry GeometryBaseRing;
  Passed = EvaluateTrue("ScanSetupFile()", "inherited guard ring", "The fixture is accepted", Scan(GeometryBaseRing, BaseRing)) && Passed;
  if (CheckDetector(Passed, "GeometryBaseRing", GeometryBaseRing, "N", MDDetector::c_Strip2D)) {
    MDDetector* N = GeometryBaseRing.GetDetector("N");
    MDDetector* Base = GeometryBaseRing.GetDetector("D");
    Passed = EvaluateTrue("CopyDataToNamedDetectors()", "inherited guard ring", "The named detector has a guard ring", N->GetGuardRing() != nullptr) && Passed;
    Passed = EvaluateTrue("CopyDataToNamedDetectors()", "inherited guard ring", "The guard ring is another object", N->GetGuardRing() != Base->GetGuardRing()) && Passed;
    Passed = EvaluateTrue("CopyDataToNamedDetectors()", "inherited guard ring", "The guard ring has its own name", N->GetGuardRing()->GetName() == "N_GuardRing") && Passed;
    Passed = EvaluateTrue("CopyDataToNamedDetectors()", "inherited guard ring", "The guard ring of the named detector is in the geometry", GeometryBaseRing.GetDetector("N_GuardRing") == N->GetGuardRing()) && Passed;
    Passed = EvaluateTrue("IsActive()", "inherited guard ring", "The guard ring of the named detector is active", N->GetGuardRing()->IsActive()) && Passed;
    Passed = EvaluateNear("GetTriggerThreshold()", "inherited guard ring", "The trigger threshold of the guard ring is inherited", N->GetGuardRing()->GetTriggerThreshold(), 30.0, 1e-12) && Passed;
    Passed = EvaluateTrue("IsNamedDetector()", "inherited guard ring", "The guard ring is a named detector", N->GetGuardRing()->IsNamedDetector()) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output
bool UTDStrip2D::TestText()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "one wafer", "The fixture is accepted", Scan(Geometry, GetSingleWafer(), "D.EnergyResolution Gauss 100 100 1\nD.EnergyResolution Gauss 200 200 2\nD.NoiseThreshold 10 1\nD.TriggerThreshold 20 2\n")) && Passed;
  if (Passed == false) {
    return false;
  }
  MDStrip2D* D = dynamic_cast<MDStrip2D*>(Geometry.GetDetector("D"));

  MString Text = D->GetGeomega();
  Passed = EvaluateTrue("GetGeomega()", "type", "The type is written", Text.Contains("Strip2D D")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "offset", "The offset is written", Text.Contains("D.Offset 0.5 0.25")) && Passed;
  Passed = EvaluateTrue("GetGeomega()", "strips", "The number of strips is written", Text.Contains("D.StripNumber 10 7")) && Passed;

  // Round trip
  MDGeometry Second;
  Passed = EvaluateTrue("ScanSetupFile()", "round trip", "The text can be scanned again", Scan(Second, GetBoxVolume("Box", 3.0, 2.0, 1.0), Text)) && Passed;
  if (CheckDetector(Passed, "Second", Second, "D", MDDetector::c_Strip2D)) {
    Passed = Evaluate("GetGeomega()", "round trip, all fields", "The text written after the round trip is identical, so that no written field is lost", dynamic_cast<MDStrip2D*>(Second.GetDetector("D"))->GetGeomega(), Text) && Passed;
    MDStrip2D* R = dynamic_cast<MDStrip2D*>(Second.GetDetector("D"));
    Passed = Evaluate("GetGeomega()", "round trip", "The type is identical", R->GetType(), MDDetector::c_Strip2D) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The pitch is identical", R->GetPitchX(), 0.5, 1e-12) && Passed;
    Passed = Evaluate("GetGeomega()", "round trip", "The strips are identical", R->GetNStripsY(), 7) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The energy resolution is identical", R->GetEnergyResolutionWidth1(150.0), 1.5, 1e-9) && Passed;
    Passed = EvaluateNear("GetGeomega()", "round trip", "The noise threshold is identical", R->GetNoiseThreshold(), 10.0, 1e-12) && Passed;
  }

  // Guard ring settings
  MDGeometry Ring;
  Passed = EvaluateTrue("ScanSetupFile()", "guard ring", "The fixture is accepted", Scan(Ring, GetSingleWafer(), "D.GuardRingEnergyResolution 100 1\nD.GuardRingEnergyResolution 200 2\nD.GuardRingTriggerThreshold 30 3\n")) && Passed;
  if (CheckDetector(Passed, "Ring", Ring, "D", MDDetector::c_Strip2D)) {
    // Defect GEO-D-031: GetGeomega does not write the guard ring keywords, the guard ring settings are lost in a round trip
    Passed = ExpectedToFail(EvaluateTrue("GetGeomega()", "guard ring", "The guard ring settings are written", dynamic_cast<MDStrip2D*>(Ring.GetDetector("D"))->GetGeomega().Contains("GuardRingTriggerThreshold"))) && Passed;
  }

  // Dump
  MString Dump = D->ToString();
  Passed = EvaluateTrue("ToString()", "name", "The name is written", Dump.Contains("\"D\"")) && Passed;
  Passed = EvaluateTrue("ToString()", "type", "The type is written", Dump.Contains("Strip2D")) && Passed;
  Passed = EvaluateTrue("ToString()", "pitch", "The pitch is written", Dump.Contains("pitch: 0.5, 0.5")) && Passed;
  Passed = EvaluateTrue("ToString()", "strip number", "The strip number is written", Dump.Contains("stripnumber: 10, 7")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the noising via the volume tree
bool UTDStrip2D::TestVolumeNoise()
{
  bool Passed = true;
  MString Flags;

  // Strip detector at z = 5 with a passive daughter in a corner:
  MString Body;
  Body += GetBoxVolume("Box", 3.0, 2.0, 1.0, 5.0);
  Body += "Volume Inner\nInner.Material Vacuum\nInner.Shape BRIK 0.1 0.1 0.1\nInner.Position 2.0 1.0 0.5\nInner.Mother Box\n";
  Body += "Strip2D D\nD.SensitiveVolume Box\nD.DetectorVolume Box\nD.Offset 0.5 0.25\nD.StripNumber 10 7\nD.NoiseThreshold 10\n";

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "volume tree", "The fixture is accepted", Scan(Geometry, Body)) && Passed;
  if (Passed == false) {
    return false;
  }

  MDVolume* World = Geometry.GetWorldVolume();
  MVector Position(0.25, 0.25, 5.5);
  double Energy = 100.0;
  double Time = 0.0;
  bool Result = World->Noise(Position, Energy, Time, Flags);
  Passed = EvaluateTrue("MDVolume::Noise()", "in the detector, not in the daughter", "The hit is noised", Result) && Passed;
  Passed = EvaluateNear("MDVolume::Noise()", "in the detector, not in the daughter", "The energy stays", Energy, 100.0, 1e-12) && Passed;

  // Defect GEO-D-032: MDVolume::Noise does not restore the position after the last daughter did not contain the hit - the position is noised and returned in the frame of the daughter
  Passed = ExpectedToFail(EvaluateVectorNear("MDVolume::Noise()", "in the detector, not in the daughter", "The position in the world is unchanged for a strip detector", Position, MVector(0.25, 0.25, 5.5), 1e-9)) && Passed;

  // Outside of everything:
  Position = MVector(10.0, 10.0, 10.0);
  Energy = 100.0;
  Result = World->Noise(Position, Energy, Time, Flags);
  Passed = EvaluateFalse("MDVolume::Noise()", "outside of all detectors", "The hit is not in a detector", Result) && Passed;
  Passed = EvaluateNear("MDVolume::Noise()", "outside of all detectors", "The energy stays unchanged", Energy, 100.0, 1e-12) && Passed;

  // Defect GEO-D-032: Same defect in the world volume, which has the detector as last daughter
  Passed = ExpectedToFail(EvaluateVectorNear("MDVolume::Noise()", "outside of all detectors", "The position stays unchanged", Position, MVector(10.0, 10.0, 10.0), 1e-9)) && Passed;

  // A hit below the threshold
  Position = MVector(0.25, 0.25, 5.5);
  Energy = 5.0;
  World->Noise(Position, Energy, Time, Flags);
  Passed = EvaluateNear("MDVolume::Noise()", "below the threshold", "The energy is zero", Energy, 0.0, 1e-12) && Passed;

  // A hit in the daughter: The daughter is passive, so the search fails there - the energy is not noised
  Position = MVector(2.0, 1.0, 5.5);
  Energy = 5.0;
  Result = World->Noise(Position, Energy, Time, Flags);
  Passed = EvaluateFalse("MDVolume::Noise()", "in the passive daughter", "A hit in a passive daughter has no detector", Result && Geometry.GetVolume("Inner")->GetDetector() == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  gROOT->SetBatch(true);
  gErrorIgnoreLevel = kWarning;
  TApplication ROOT("ROOT", 0, 0);

  UTDStrip2D Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDStrip2D.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
