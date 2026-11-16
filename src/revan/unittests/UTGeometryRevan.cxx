/*
 * UTGeometryRevan.cxx
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
#include <limits>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MDDetector.h"
#include "MDVolumeSequence.h"
#include "MGeometryRevan.h"
#include "MREHit.h"
#include "MString.h"
#include "MVector.h"
#include "UTRevanShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Give access to the protected functions of MGeometryRevan
class UTGeometryRevanAccess : public MGeometryRevan
{
public:
  using MGeometryRevan::AreInSameTracker;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the revan geometry MGeometryRevan: layers of the tracker and detector volumes
class UTGeometryRevan : public UTRevanBase
{
public:
  //! Default constructor
  UTGeometryRevan() : UTRevanBase("UTGeometryRevan") {}
  //! Default destructor
  virtual ~UTGeometryRevan() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the constants and the construction
  bool TestConstruction();
  //! Test the layer distance in the not rotated tracker
  bool TestLayerDistance();
  //! Test the layer distance for hits which are not in a strip detector
  bool TestLayerDistanceOtherDetectors();
  //! Test the layer distance in rotated trackers
  bool TestLayerDistanceRotated();
  //! Test the layer distance of the simple event reconstruction mode
  bool TestLayerDistanceSimple();
  //! Test IsAbove and IsBelow
  bool TestIsAboveIsBelow();
  //! Test AreInSameLayer
  bool TestAreInSameLayer();
  //! Test AreInSameDetectorVolume
  bool TestAreInSameDetectorVolume();
  //! Test AreInSameTracker
  bool TestAreInSameTracker();
  //! Test NoiseHit
  bool TestNoiseHit();

  //! Return the telescope with a layer distance (structural pitch in z) of the tracker, and two rotated towers:
  //! * TowerR: two holders rotated by 90 degrees around x at (0, 30, 0) and (0, 31, 0) cm, their wafer normals are along y
  //! * TowerT: two holders rotated by 40 degrees around x at (0, -30, 0) cm and 1 cm along its wafer normal (0, -sin 40, cos 40) from it
  MString GetLayeredGeometry(double LayerDistance) const;
  //! Return the layer distance expected from the positions: the distance along the normal of the wafer of the reference hit in units of the layer distance, rounded to the nearest integer
  int GetExpectedLayerDistance(MREHit* Reference, MREHit* Test, double LayerDistance) const;
  //! Call NoiseHit, which does not return
  bool CallNoiseHit();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTGeometryRevan::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestLayerDistance() && Passed;
  Passed = TestLayerDistanceOtherDetectors() && Passed;
  Passed = TestLayerDistanceRotated() && Passed;
  Passed = TestLayerDistanceSimple() && Passed;
  Passed = TestIsAboveIsBelow() && Passed;
  Passed = TestAreInSameLayer() && Passed;
  Passed = TestAreInSameDetectorVolume() && Passed;
  Passed = TestAreInSameTracker() && Passed;
  Passed = TestNoiseHit() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the constants and the construction
bool UTGeometryRevan::TestConstruction()
{
  bool Passed = true;

  Passed = Evaluate("c_DifferentTracker", "constant", "The marker of a different tracker is the largest integer", MGeometryRevan::c_DifferentTracker, numeric_limits<int>::max()) && Passed;

  MGeometryRevan Geometry;
  Passed = EvaluateFalse("MGeometryRevan()", "default", "A new geometry is not scanned", Geometry.IsScanned()) && Passed;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The revan geometry scans the telescope", ScanTelescopeGeometry(Geometry)) && Passed;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The geometry is scanned", Geometry.IsScanned()) && Passed;
  Passed = Evaluate("ScanTelescopeGeometry()", "telescope", "The telescope has four detectors", Geometry.GetNDetectors(), 4U) && Passed;

  // The geometry can be scanned a second time
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "second scan", "The revan geometry scans the telescope again", ScanTelescopeGeometry(Geometry)) && Passed;
  Passed = Evaluate("ScanTelescopeGeometry()", "second scan", "The telescope still has four detectors", Geometry.GetNDetectors(), 4U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the telescope with a layer distance (structural pitch in z) of the tracker, and two rotated towers:
//! * TowerR: two holders rotated by 90 degrees around x at (0, 30, 0) and (0, 31, 0) cm, their wafer normals are along y
//! * TowerT: two holders rotated by 40 degrees around x at (0, -30, 0) cm and 1 cm along its wafer normal (0, -sin 40, cos 40) from it
MString UTGeometryRevan::GetLayeredGeometry(double LayerDistance) const
{
  MString Lines;
  Lines += "Tracker.StructuralPitch 0.0 0.0 ";
  Lines += LayerDistance;
  Lines += "\n";

  Lines += "Holder.Copy HolderR0\n";
  Lines += "HolderR0.Position 0.0 30.0 0.0\n";
  Lines += "HolderR0.Rotation 90.0 0.0 0.0\n";
  Lines += "HolderR0.Mother World\n";
  Lines += "Holder.Copy HolderR1\n";
  Lines += "HolderR1.Position 0.0 31.0 0.0\n";
  Lines += "HolderR1.Rotation 90.0 0.0 0.0\n";
  Lines += "HolderR1.Mother World\n";

  // sin(40 deg) = 0.642787609686539, cos(40 deg) = 0.766044443118978
  Lines += "Holder.Copy HolderT0\n";
  Lines += "HolderT0.Position 0.0 -30.0 0.0\n";
  Lines += "HolderT0.Rotation 40.0 0.0 0.0\n";
  Lines += "HolderT0.Mother World\n";
  Lines += "Holder.Copy HolderT1\n";
  Lines += "HolderT1.Position 0.0 -30.642787609686539 0.766044443118978\n";
  Lines += "HolderT1.Rotation 40.0 0.0 0.0\n";
  Lines += "HolderT1.Mother World\n";

  return GetTelescopeGeometry(Lines);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the layer distance expected from the positions: the distance along the normal of the wafer of the reference hit in units of the layer distance, rounded to the nearest integer
int UTGeometryRevan::GetExpectedLayerDistance(MREHit* Reference, MREHit* Test, double LayerDistance) const
{
  // The normal of the wafer in world coordinates comes from the position transformation of the volume sequence, not from its rotation
  MDVolumeSequence* Sequence = Reference->GetVolumeSequence();
  MVector Normal = Sequence->GetPositionInFirstVolume(MVector(0.0, 0.0, 1.0), Sequence->GetDeepestVolume()) - Sequence->GetPositionInFirstVolume(MVector(0.0, 0.0, 0.0), Sequence->GetDeepestVolume());

  double Distance = (Test->GetPosition() - Reference->GetPosition()).Dot(Normal)/LayerDistance;

  return static_cast<int>(lround(Distance));
}


////////////////////////////////////////////////////////////////////////////////


//! Test the layer distance in the not rotated tracker
bool UTGeometryRevan::TestLayerDistance()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanGeometry()", "layer distance 1 cm", "The geometry with layers can be scanned", ScanGeometry(Geometry, GetLayeredGeometry(1.0))) && Passed;
  if (Geometry.IsScanned() == false) return Passed;

  // One hit per layer at different lateral positions, and a second one in layer 0
  MREHit* Layer0 = CreateHit(MVector(1.0, 2.0, GetTrackerLayerZ(0)), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Layer0Other = CreateHit(MVector(-4.0, 4.0, GetTrackerLayerZ(0)), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Layer1 = CreateHit(MVector(-3.0, 0.0, GetTrackerLayerZ(1)), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Layer2 = CreateHit(MVector(2.0, -4.0, GetTrackerLayerZ(2)), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Layer3 = CreateHit(MVector(0.0, 0.0, GetTrackerLayerZ(3)), 100.0, MDDetector::c_Strip2D, &Geometry);
  Passed = EvaluateTrue("CreateHit()", "tracker", "All tracker hits are created", Layer0 != nullptr && Layer0Other != nullptr && Layer1 != nullptr && Layer2 != nullptr && Layer3 != nullptr) && Passed;
  if (Layer0 == nullptr || Layer0Other == nullptr || Layer1 == nullptr || Layer2 == nullptr || Layer3 == nullptr) return Passed;

  // The layers are 1 cm apart along z, and the layer distance is 1 cm: the distance is the difference of the layer numbers, test minus reference
  Passed = Evaluate("GetLayerDistance()", "layer 0 to itself", "A hit has no distance to itself", Geometry.GetLayerDistance(Layer0, Layer0), 0) && Passed;
  Passed = Evaluate("GetLayerDistance()", "layer 0 to layer 0", "Two hits in the same layer have the distance 0", Geometry.GetLayerDistance(Layer0, Layer0Other), 0) && Passed;
  Passed = Evaluate("GetLayerDistance()", "layer 0 to 1", "The next layer along +z has the distance 1", Geometry.GetLayerDistance(Layer0, Layer1), 1) && Passed;
  Passed = Evaluate("GetLayerDistance()", "layer 0 to 2", "The lateral offset does not count: the distance is 2", Geometry.GetLayerDistance(Layer0, Layer2), 2) && Passed;
  Passed = Evaluate("GetLayerDistance()", "layer 0 to 3", "The distance is 3", Geometry.GetLayerDistance(Layer0, Layer3), 3) && Passed;
  Passed = Evaluate("GetLayerDistance()", "layer 1 to 0", "The layer along -z has the distance -1", Geometry.GetLayerDistance(Layer1, Layer0), -1) && Passed;
  Passed = Evaluate("GetLayerDistance()", "layer 3 to 0", "The distance is -3", Geometry.GetLayerDistance(Layer3, Layer0), -3) && Passed;
  Passed = Evaluate("GetLayerDistance()", "layer 2 to 1", "The distance is -1", Geometry.GetLayerDistance(Layer2, Layer1), -1) && Passed;
  Passed = Evaluate("GetLayerDistance()", "layer 0 to 2", "The result agrees with the positions", Geometry.GetLayerDistance(Layer0, Layer2), GetExpectedLayerDistance(Layer0, Layer2, 1.0)) && Passed;

  // Hits at the surfaces of the wafers (half thickness 0.025 cm): 0.95 and 1.05 layer distances are still one layer
  MREHit* Top0 = CreateHit(MVector(1.0, 2.0, 0.0249), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Bottom1 = CreateHit(MVector(1.0, 2.0, 0.9751), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Bottom0 = CreateHit(MVector(1.0, 2.0, -0.0249), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Top1 = CreateHit(MVector(1.0, 2.0, 1.0249), 100.0, MDDetector::c_Strip2D, &Geometry);
  Passed = EvaluateTrue("CreateHit()", "wafer surfaces", "The hits at the surfaces are created", Top0 != nullptr && Bottom1 != nullptr && Bottom0 != nullptr && Top1 != nullptr) && Passed;
  if (Top0 != nullptr && Bottom1 != nullptr && Bottom0 != nullptr && Top1 != nullptr) {
    Passed = Evaluate("GetLayerDistance()", "0.9502 layer distances", "The closest surfaces of neighboring wafers are one layer apart", Geometry.GetLayerDistance(Top0, Bottom1), 1) && Passed;
    Passed = Evaluate("GetLayerDistance()", "1.0498 layer distances", "The farthest surfaces of neighboring wafers are one layer apart", Geometry.GetLayerDistance(Bottom0, Top1), 1) && Passed;
    Passed = Evaluate("GetLayerDistance()", "0.0498 layer distances", "The two surfaces of one wafer are in the same layer", Geometry.GetLayerDistance(Bottom0, Top0), 0) && Passed;
  }
  delete Top0;
  delete Bottom1;
  delete Bottom0;
  delete Top1;

  delete Layer0;
  delete Layer0Other;
  delete Layer1;
  delete Layer2;
  delete Layer3;

  // A layer distance of 0.4 cm: the layers at 1, 2, 3 cm are 2.5, 5, and 7.5 layer distances away - rounded half away from zero: 3, 5, 8
  // (1/0.4 and 3/0.4 are 2.5 and 7.5 in double precision too)
  MGeometryRevan Fine;
  Passed = EvaluateTrue("ScanGeometry()", "layer distance 0.4 cm", "The geometry with layers can be scanned", ScanGeometry(Fine, GetLayeredGeometry(0.4))) && Passed;
  if (Fine.IsScanned() == false) return Passed;
  MREHit* Fine0 = CreateHit(MVector(1.0, 2.0, GetTrackerLayerZ(0)), 100.0, MDDetector::c_Strip2D, &Fine);
  MREHit* Fine2 = CreateHit(MVector(1.0, 2.0, GetTrackerLayerZ(2)), 100.0, MDDetector::c_Strip2D, &Fine);
  MREHit* Fine2Low = CreateHit(MVector(1.0, 2.0, GetTrackerLayerZ(2) - 0.02), 100.0, MDDetector::c_Strip2D, &Fine);
  MREHit* Fine2High = CreateHit(MVector(1.0, 2.0, GetTrackerLayerZ(2) + 0.02), 100.0, MDDetector::c_Strip2D, &Fine);
  Passed = EvaluateTrue("CreateHit()", "layer distance 0.4 cm", "The hits are created", Fine0 != nullptr && Fine2 != nullptr && Fine2Low != nullptr && Fine2High != nullptr) && Passed;
  if (Fine0 != nullptr && Fine2 != nullptr && Fine2Low != nullptr && Fine2High != nullptr) {
    Passed = Evaluate("GetLayerDistance()", "2 cm at 0.4 cm per layer", "The distance is 5 layers", Fine.GetLayerDistance(Fine0, Fine2), 5) && Passed;
    Passed = Evaluate("GetLayerDistance()", "-2 cm at 0.4 cm per layer", "The distance is -5 layers", Fine.GetLayerDistance(Fine2, Fine0), -5) && Passed;
    // 1.98 cm / 0.4 cm = 4.95 and 2.02 cm / 0.4 cm = 5.05
    Passed = Evaluate("GetLayerDistance()", "4.95 layer distances", "The distance is rounded up to 5", Fine.GetLayerDistance(Fine0, Fine2Low), 5) && Passed;
    Passed = Evaluate("GetLayerDistance()", "5.05 layer distances", "The distance is rounded down to 5", Fine.GetLayerDistance(Fine0, Fine2High), 5) && Passed;
    Passed = Evaluate("GetLayerDistance()", "-5.05 layer distances", "The negative distance is rounded to -5", Fine.GetLayerDistance(Fine2High, Fine0), -5) && Passed;
  }
  delete Fine0;
  delete Fine2;
  delete Fine2Low;
  delete Fine2High;

  // A layer distance of 0.8 cm: 1 cm is 1.25 layer distances (below the half: 1), 3 cm is 3.75 (above the half: 4)
  MGeometryRevan Coarse;
  Passed = EvaluateTrue("ScanGeometry()", "layer distance 0.8 cm", "The geometry with layers can be scanned", ScanGeometry(Coarse, GetLayeredGeometry(0.8))) && Passed;
  if (Coarse.IsScanned() == false) return Passed;
  MREHit* Coarse0 = CreateHit(MVector(1.0, 2.0, GetTrackerLayerZ(0)), 100.0, MDDetector::c_Strip2D, &Coarse);
  MREHit* Coarse1 = CreateHit(MVector(1.0, 2.0, GetTrackerLayerZ(1)), 100.0, MDDetector::c_Strip2D, &Coarse);
  MREHit* Coarse3 = CreateHit(MVector(1.0, 2.0, GetTrackerLayerZ(3)), 100.0, MDDetector::c_Strip2D, &Coarse);
  Passed = EvaluateTrue("CreateHit()", "layer distance 0.8 cm", "The hits are created", Coarse0 != nullptr && Coarse1 != nullptr && Coarse3 != nullptr) && Passed;
  if (Coarse0 != nullptr && Coarse1 != nullptr && Coarse3 != nullptr) {
    Passed = Evaluate("GetLayerDistance()", "1.25 layer distances", "The distance is rounded down to 1", Coarse.GetLayerDistance(Coarse0, Coarse1), 1) && Passed;
    Passed = Evaluate("GetLayerDistance()", "3.75 layer distances", "The distance is rounded up to 4", Coarse.GetLayerDistance(Coarse0, Coarse3), 4) && Passed;
    Passed = Evaluate("GetLayerDistance()", "-1.25 layer distances", "The distance is rounded to -1", Coarse.GetLayerDistance(Coarse1, Coarse0), -1) && Passed;
    Passed = Evaluate("GetLayerDistance()", "-3.75 layer distances", "The distance is rounded to -4", Coarse.GetLayerDistance(Coarse3, Coarse0), -4) && Passed;
  }
  delete Coarse0;
  delete Coarse1;
  delete Coarse3;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the layer distance for hits which are not in a strip detector
bool UTGeometryRevan::TestLayerDistanceOtherDetectors()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanGeometry()", "layer distance 1 cm", "The geometry with layers can be scanned", ScanGeometry(Geometry, GetLayeredGeometry(1.0))) && Passed;
  if (Geometry.IsScanned() == false) return Passed;

  MREHit* Tracker = CreateHit(MVector(1.0, 2.0, GetTrackerLayerZ(0)), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Calorimeter = CreateHit(MVector(1.0, 2.0, -10.0), 100.0, MDDetector::c_Calorimeter, &Geometry);
  MREHit* Shield = CreateHit(MVector(1.0, 2.0, 10.0), 100.0, MDDetector::c_Scintillator, &Geometry);
  Passed = EvaluateTrue("CreateHit()", "tracker, calorimeter, shield", "The hits are created", Tracker != nullptr && Calorimeter != nullptr && Shield != nullptr) && Passed;
  if (Tracker == nullptr || Calorimeter == nullptr || Shield == nullptr) return Passed;

  // Only strip detectors have layers
  Passed = Evaluate("GetLayerDistance()", "tracker to calorimeter", "A calorimeter hit is in a different tracker", Geometry.GetLayerDistance(Tracker, Calorimeter), MGeometryRevan::c_DifferentTracker) && Passed;
  Passed = Evaluate("GetLayerDistance()", "calorimeter to tracker", "A calorimeter hit is in a different tracker", Geometry.GetLayerDistance(Calorimeter, Tracker), MGeometryRevan::c_DifferentTracker) && Passed;
  Passed = Evaluate("GetLayerDistance()", "tracker to shield", "A scintillator hit is in a different tracker", Geometry.GetLayerDistance(Tracker, Shield), MGeometryRevan::c_DifferentTracker) && Passed;
  Passed = Evaluate("GetLayerDistance()", "calorimeter to calorimeter", "Calorimeter hits have no layers", Geometry.GetLayerDistance(Calorimeter, Calorimeter), MGeometryRevan::c_DifferentTracker) && Passed;

  // A hit which never got a volume sequence from the geometry has an empty one without detector
  MREHit* Outside = CreateHit(MVector(50.0, 50.0, 50.0), 100.0, MDDetector::c_Strip2D);
  DisableDefaultStreams();
  int OutsideDistance = Geometry.GetLayerDistance(Tracker, Outside);
  int OutsideReferenceDistance = Geometry.GetLayerDistance(Outside, Tracker);
  EnableDefaultStreams();
  Passed = Evaluate("GetLayerDistance()", "test hit without detector", "A hit without detector is in a different tracker", OutsideDistance, MGeometryRevan::c_DifferentTracker) && Passed;
  Passed = Evaluate("GetLayerDistance()", "reference hit without detector", "A hit without detector is in a different tracker", OutsideReferenceDistance, MGeometryRevan::c_DifferentTracker) && Passed;
  delete Outside;

  delete Tracker;
  delete Calorimeter;
  delete Shield;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the layer distance in rotated trackers
bool UTGeometryRevan::TestLayerDistanceRotated()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanGeometry()", "layer distance 1 cm", "The geometry with layers can be scanned", ScanGeometry(Geometry, GetLayeredGeometry(1.0))) && Passed;
  if (Geometry.IsScanned() == false) return Passed;

  // The tower rotated by 90 degrees around x: the wafers are 1 cm apart along y
  MREHit* R0 = CreateHit(MVector(1.0, 30.0, 2.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* R1 = CreateHit(MVector(-2.0, 31.0, -3.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  Passed = EvaluateTrue("CreateHit()", "tower rotated by 90 degrees", "The hits are created", R0 != nullptr && R1 != nullptr) && Passed;
  if (R0 != nullptr && R1 != nullptr) {
    int Expected = GetExpectedLayerDistance(R0, R1, 1.0);
    Passed = Evaluate("GetExpectedLayerDistance()", "tower rotated by 90 degrees", "The wafers are one layer apart", abs(Expected), 1) && Passed;
    // Defect REV-D-1: GetLayerDistance rotates the layer normal with the world-to-volume rotation instead of its inverse, so the sign is wrong for a tracker rotated by 90 degrees
    Passed = ExpectedToFail(Evaluate("GetLayerDistance()", "tower rotated by 90 degrees, layer 0 to 1", "The distance has the sign of the wafer normal in the world", Geometry.GetLayerDistance(R0, R1), Expected)) && Passed;
    Passed = ExpectedToFail(Evaluate("GetLayerDistance()", "tower rotated by 90 degrees, layer 1 to 0", "The distance has the sign of the wafer normal in the world", Geometry.GetLayerDistance(R1, R0), -Expected)) && Passed;
  }
  delete R0;
  delete R1;

  // The orientation of the tower rotated by 40 degrees around x, independent of any rotation matrix: 4 cm from the center of
  // the first wafer along (0, cos 40, sin 40) is inside the wafer (half length 5 cm, half thickness 0.025 cm), so its normal is (0, -sin 40, cos 40)
  // 4 cos(40 deg) = 3.064177772475912, 4 sin(40 deg) = 2.571150438746157
  MREHit* InPlane = CreateHit(MVector(0.0, -30.0 + 3.064177772475912, 2.571150438746157), 100.0, MDDetector::c_Strip2D, &Geometry);
  Passed = EvaluateTrue("CreateHit()", "tower rotated by 40 degrees, in plane", "The hit is created", InPlane != nullptr) && Passed;
  if (InPlane != nullptr) {
    Passed = EvaluateNear("GetPositionInSensitiveVolume()", "tower rotated by 40 degrees, in plane", "The hit is 4 cm from the center of the wafer", InPlane->GetVolumeSequence()->GetPositionInSensitiveVolume().Mag(), 4.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetPositionInSensitiveVolume()", "tower rotated by 40 degrees, in plane", "The hit is in the plane of the wafer", InPlane->GetVolumeSequence()->GetPositionInSensitiveVolume().Z(), 0.0, 1e-12) && Passed;
  }
  delete InPlane;

  // The tower rotated by 40 degrees around x: the wafers are 1 cm apart along their normal
  // The second hit is at the center of the second wafer, the first one 1 cm away from the center of the first wafer in x
  MREHit* T0 = CreateHit(MVector(1.0, -30.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* T1 = CreateHit(MVector(0.0, -30.642787609686539, 0.766044443118978), 100.0, MDDetector::c_Strip2D, &Geometry);
  Passed = EvaluateTrue("CreateHit()", "tower rotated by 40 degrees", "The hits are created", T0 != nullptr && T1 != nullptr) && Passed;
  if (T0 != nullptr && T1 != nullptr) {
    int Expected = GetExpectedLayerDistance(T0, T1, 1.0);
    Passed = Evaluate("GetExpectedLayerDistance()", "tower rotated by 40 degrees", "The second wafer is one layer above the first one", Expected, 1) && Passed;
    // Defect REV-D-1: with the wrong normal (0, sin 40, cos 40) the distance is cos(80 deg) = 0.17 layers instead of one layer
    Passed = ExpectedToFail(Evaluate("GetLayerDistance()", "tower rotated by 40 degrees, layer 0 to 1", "The distance is the one along the wafer normal in the world", Geometry.GetLayerDistance(T0, T1), Expected)) && Passed;
    Passed = ExpectedToFail(Evaluate("GetLayerDistance()", "tower rotated by 40 degrees, layer 1 to 0", "The distance is the one along the wafer normal in the world", Geometry.GetLayerDistance(T1, T0), -Expected)) && Passed;
  }
  delete T0;
  delete T1;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the layer distance of the simple event reconstruction mode
bool UTGeometryRevan::TestLayerDistanceSimple()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "ComplexER false", "The geometry with the simple event reconstruction can be scanned", ScanTelescopeGeometry(Geometry, "ComplexER false\n")) && Passed;
  if (Geometry.IsScanned() == false) return Passed;

  // The simple mode only uses the z distance of the positions in units of 1 cm, rounded half away from zero
  MREHit* Reference = CreateHit(MVector(1.0, 2.0, 0.0), 100.0, MDDetector::c_Strip2D);
  double Offsets[10] = { 0.0, 0.49, 0.51, 1.49, 1.51, 3.0, -0.49, -0.51, -1.49, -1.51 };
  int Expected[10] = { 0, 0, 1, 1, 2, 3, 0, -1, -1, -2 };
  for (unsigned int i = 0; i < 10; ++i) {
    MREHit* Test = CreateHit(MVector(-3.0, 4.0, Offsets[i]), 100.0, MDDetector::c_Strip2D);
    Passed = Evaluate("GetLayerDistance()", MString("simple mode, z distance ") + Offsets[i], "The distance is the rounded z distance in cm", Geometry.GetLayerDistance(Reference, Test), Expected[i]) && Passed;
    delete Test;
  }
  delete Reference;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test IsAbove and IsBelow
bool UTGeometryRevan::TestIsAboveIsBelow()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanGeometry()", "layer distance 1 cm", "The geometry with layers can be scanned", ScanGeometry(Geometry, GetLayeredGeometry(1.0))) && Passed;
  if (Geometry.IsScanned() == false) return Passed;

  MREHit* Layer0 = CreateHit(MVector(1.0, 2.0, GetTrackerLayerZ(0)), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Layer1 = CreateHit(MVector(-3.0, 0.0, GetTrackerLayerZ(1)), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Layer3 = CreateHit(MVector(0.0, 0.0, GetTrackerLayerZ(3)), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Calorimeter = CreateHit(MVector(1.0, 2.0, -10.0), 100.0, MDDetector::c_Calorimeter, &Geometry);
  Passed = EvaluateTrue("CreateHit()", "tracker and calorimeter", "The hits are created", Layer0 != nullptr && Layer1 != nullptr && Layer3 != nullptr && Calorimeter != nullptr) && Passed;
  if (Layer0 == nullptr || Layer1 == nullptr || Layer3 == nullptr || Calorimeter == nullptr) return Passed;

  // Above is along +z (the orientation of the strip detector), by default one layer
  Passed = EvaluateTrue("IsAbove()", "layer 0, layer 1", "Layer 1 is one layer above layer 0", Geometry.IsAbove(Layer0, Layer1)) && Passed;
  Passed = EvaluateFalse("IsAbove()", "layer 1, layer 0", "Layer 0 is not above layer 1", Geometry.IsAbove(Layer1, Layer0)) && Passed;
  Passed = EvaluateFalse("IsAbove()", "layer 0, layer 0", "A hit is not above itself", Geometry.IsAbove(Layer0, Layer0)) && Passed;
  Passed = EvaluateFalse("IsAbove()", "layer 0, layer 3, one layer", "Layer 3 is not one layer above layer 0", Geometry.IsAbove(Layer0, Layer3)) && Passed;
  Passed = EvaluateTrue("IsAbove()", "layer 0, layer 3, three layers", "Layer 3 is three layers above layer 0", Geometry.IsAbove(Layer0, Layer3, 3)) && Passed;
  Passed = EvaluateFalse("IsAbove()", "layer 0, layer 3, two layers", "Layer 3 is not two layers above layer 0", Geometry.IsAbove(Layer0, Layer3, 2)) && Passed;
  Passed = EvaluateFalse("IsAbove()", "layer 0, layer 3, four layers", "Layer 3 is not four layers above layer 0", Geometry.IsAbove(Layer0, Layer3, 4)) && Passed;
  Passed = EvaluateTrue("IsAbove()", "layer 1, layer 3, two layers", "Layer 3 is two layers above layer 1", Geometry.IsAbove(Layer1, Layer3, 2)) && Passed;
  Passed = EvaluateTrue("IsAbove()", "layer 0, layer 0, zero layers", "A hit is zero layers above a hit in its layer", Geometry.IsAbove(Layer0, Layer0, 0)) && Passed;
  Passed = EvaluateFalse("IsAbove()", "tracker, calorimeter", "A calorimeter hit is not above a tracker hit", Geometry.IsAbove(Layer0, Calorimeter)) && Passed;

  Passed = EvaluateTrue("IsBelow()", "layer 1, layer 0", "Layer 0 is one layer below layer 1", Geometry.IsBelow(Layer1, Layer0)) && Passed;
  Passed = EvaluateFalse("IsBelow()", "layer 0, layer 1", "Layer 1 is not below layer 0", Geometry.IsBelow(Layer0, Layer1)) && Passed;
  Passed = EvaluateFalse("IsBelow()", "layer 0, layer 0", "A hit is not below itself", Geometry.IsBelow(Layer0, Layer0)) && Passed;
  Passed = EvaluateFalse("IsBelow()", "layer 3, layer 0, one layer", "Layer 0 is not one layer below layer 3", Geometry.IsBelow(Layer3, Layer0)) && Passed;
  Passed = EvaluateTrue("IsBelow()", "layer 3, layer 0, three layers", "Layer 0 is three layers below layer 3", Geometry.IsBelow(Layer3, Layer0, 3)) && Passed;
  Passed = EvaluateFalse("IsBelow()", "layer 3, layer 0, two layers", "Layer 0 is not two layers below layer 3", Geometry.IsBelow(Layer3, Layer0, 2)) && Passed;
  Passed = EvaluateTrue("IsBelow()", "layer 3, layer 1, two layers", "Layer 1 is two layers below layer 3", Geometry.IsBelow(Layer3, Layer1, 2)) && Passed;
  // The calorimeter is 10 cm below the tracker, but it has no layers
  Passed = EvaluateFalse("IsBelow()", "tracker, calorimeter, ten layers", "A calorimeter hit is not below a tracker hit", Geometry.IsBelow(Layer0, Calorimeter, 10)) && Passed;

  // A negative number of layers reverses the direction
  Passed = EvaluateTrue("IsAbove()", "layer 1, layer 0, minus one layer", "Minus one layer above is one layer below", Geometry.IsAbove(Layer1, Layer0, -1)) && Passed;
  Passed = EvaluateTrue("IsBelow()", "layer 0, layer 1, minus one layer", "Minus one layer below is one layer above", Geometry.IsBelow(Layer0, Layer1, -1)) && Passed;

  delete Layer0;
  delete Layer1;
  delete Layer3;
  delete Calorimeter;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test AreInSameLayer
bool UTGeometryRevan::TestAreInSameLayer()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanGeometry()", "layer distance 1 cm", "The geometry with layers can be scanned", ScanGeometry(Geometry, GetLayeredGeometry(1.0))) && Passed;
  if (Geometry.IsScanned() == false) return Passed;

  MREHit* Layer0 = CreateHit(MVector(1.0, 2.0, GetTrackerLayerZ(0)), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Layer0Other = CreateHit(MVector(-4.0, 4.0, GetTrackerLayerZ(0)), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Layer1 = CreateHit(MVector(-3.0, 0.0, GetTrackerLayerZ(1)), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Depth = CreateHit(MVector(30.2, 0.3, 0.0), 100.0, MDDetector::c_Strip3D, &Geometry);
  MREHit* DepthOther = CreateHit(MVector(32.2, 2.3, 0.0), 100.0, MDDetector::c_Strip3D, &Geometry);
  MREHit* Calorimeter = CreateHit(MVector(1.0, 2.0, -10.0), 100.0, MDDetector::c_Calorimeter, &Geometry);
  Passed = EvaluateTrue("CreateHit()", "tracker, depth, calorimeter", "The hits are created", Layer0 != nullptr && Layer0Other != nullptr && Layer1 != nullptr && Depth != nullptr && DepthOther != nullptr && Calorimeter != nullptr) && Passed;
  if (Layer0 == nullptr || Layer0Other == nullptr || Layer1 == nullptr || Depth == nullptr || DepthOther == nullptr || Calorimeter == nullptr) return Passed;

  Passed = EvaluateTrue("AreInSameLayer()", "layer 0, layer 0", "Two hits in one wafer are in the same layer", Geometry.AreInSameLayer(Layer0, Layer0Other)) && Passed;
  Passed = EvaluateTrue("AreInSameLayer()", "layer 0, itself", "A hit is in its own layer", Geometry.AreInSameLayer(Layer0, Layer0)) && Passed;
  Passed = EvaluateFalse("AreInSameLayer()", "layer 0, layer 1", "Hits in neighboring wafers are not in the same layer", Geometry.AreInSameLayer(Layer0, Layer1)) && Passed;
  Passed = EvaluateFalse("AreInSameLayer()", "layer 1, layer 0", "Hits in neighboring wafers are not in the same layer", Geometry.AreInSameLayer(Layer1, Layer0)) && Passed;

  // Only hits of detector type 1 (Strip2D) are in layers
  Passed = EvaluateFalse("AreInSameLayer()", "tracker, calorimeter", "A calorimeter hit is in no layer", Geometry.AreInSameLayer(Layer0, Calorimeter)) && Passed;
  Passed = EvaluateFalse("AreInSameLayer()", "calorimeter, tracker", "A calorimeter hit is in no layer", Geometry.AreInSameLayer(Calorimeter, Layer0)) && Passed;
  Passed = EvaluateFalse("AreInSameLayer()", "calorimeter, calorimeter", "A calorimeter hit is in no layer", Geometry.AreInSameLayer(Calorimeter, Calorimeter)) && Passed;
  Passed = EvaluateFalse("AreInSameLayer()", "Strip3D, Strip3D", "Hits of detector type 3 are in no layer", Geometry.AreInSameLayer(Depth, DepthOther)) && Passed;
  Passed = EvaluateFalse("AreInSameLayer()", "tracker, Strip3D at the same height", "A hit of detector type 3 is in no layer", Geometry.AreInSameLayer(Layer0, Depth)) && Passed;

  delete Layer0;
  delete Layer0Other;
  delete Layer1;
  delete Depth;
  delete DepthOther;
  delete Calorimeter;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test AreInSameDetectorVolume
bool UTGeometryRevan::TestAreInSameDetectorVolume()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope can be scanned", ScanTelescopeGeometry(Geometry)) && Passed;
  if (Geometry.IsScanned() == false) return Passed;

  MREHit* Layer0 = CreateHit(MVector(1.0, 2.0, GetTrackerLayerZ(0)), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Layer0Other = CreateHit(MVector(-4.0, 4.0, GetTrackerLayerZ(0)), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Layer1 = CreateHit(MVector(1.0, 2.0, GetTrackerLayerZ(1)), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Calorimeter = CreateHit(MVector(1.0, 2.0, -10.0), 100.0, MDDetector::c_Calorimeter, &Geometry);
  MREHit* CalorimeterOther = CreateHit(MVector(-4.0, -4.0, -13.0), 100.0, MDDetector::c_Calorimeter, &Geometry);
  MREHit* Depth = CreateHit(MVector(30.2, 0.3, 0.5), 100.0, MDDetector::c_Strip3D, &Geometry);
  MREHit* Shield = CreateHit(MVector(1.0, 2.0, 10.0), 100.0, MDDetector::c_Scintillator, &Geometry);
  Passed = EvaluateTrue("CreateHit()", "all detectors", "The hits are created", Layer0 != nullptr && Layer0Other != nullptr && Layer1 != nullptr && Calorimeter != nullptr && CalorimeterOther != nullptr && Depth != nullptr && Shield != nullptr) && Passed;
  if (Layer0 == nullptr || Layer0Other == nullptr || Layer1 == nullptr || Calorimeter == nullptr || CalorimeterOther == nullptr || Depth == nullptr || Shield == nullptr) return Passed;

  Passed = EvaluateTrue("AreInSameDetectorVolume()", "same wafer", "Two hits in one wafer are in the same detector volume", Geometry.AreInSameDetectorVolume(Layer0, Layer0Other)) && Passed;
  Passed = EvaluateTrue("AreInSameDetectorVolume()", "same hit", "A hit is in its own detector volume", Geometry.AreInSameDetectorVolume(Layer0, Layer0)) && Passed;
  Passed = EvaluateTrue("AreInSameDetectorVolume()", "same calorimeter block", "Two hits in the calorimeter block are in the same detector volume", Geometry.AreInSameDetectorVolume(Calorimeter, CalorimeterOther)) && Passed;
  Passed = EvaluateFalse("AreInSameDetectorVolume()", "wafers of two layers", "The wafers of two layers are different detector volumes of the same detector", Geometry.AreInSameDetectorVolume(Layer0, Layer1)) && Passed;
  Passed = EvaluateFalse("AreInSameDetectorVolume()", "wafers of two layers, reversed", "The result does not depend on the order", Geometry.AreInSameDetectorVolume(Layer1, Layer0)) && Passed;
  Passed = EvaluateFalse("AreInSameDetectorVolume()", "tracker and calorimeter", "Different detectors are different detector volumes", Geometry.AreInSameDetectorVolume(Layer0, Calorimeter)) && Passed;
  Passed = EvaluateFalse("AreInSameDetectorVolume()", "calorimeter and tracker", "Different detectors are different detector volumes", Geometry.AreInSameDetectorVolume(Calorimeter, Layer0)) && Passed;
  Passed = EvaluateFalse("AreInSameDetectorVolume()", "depth detector and shield", "Different detectors are different detector volumes", Geometry.AreInSameDetectorVolume(Depth, Shield)) && Passed;

  // A hit which never got a volume sequence from the geometry has an empty one without detector
  MREHit* Outside = CreateHit(MVector(50.0, 50.0, 50.0), 100.0, MDDetector::c_Strip2D);
  Passed = EvaluateFalse("AreInSameDetectorVolume()", "second hit without detector", "A hit without detector is in no detector volume", Geometry.AreInSameDetectorVolume(Layer0, Outside)) && Passed;
  Passed = EvaluateFalse("AreInSameDetectorVolume()", "first hit without detector", "A hit without detector is in no detector volume", Geometry.AreInSameDetectorVolume(Outside, Layer0)) && Passed;
  Passed = EvaluateFalse("AreInSameDetectorVolume()", "both hits without detector", "Hits without detector are in no detector volume", Geometry.AreInSameDetectorVolume(Outside, Outside)) && Passed;
  delete Outside;

  delete Layer0;
  delete Layer0Other;
  delete Layer1;
  delete Calorimeter;
  delete CalorimeterOther;
  delete Depth;
  delete Shield;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test AreInSameTracker
bool UTGeometryRevan::TestAreInSameTracker()
{
  bool Passed = true;

  UTGeometryRevanAccess Geometry;
  Passed = EvaluateTrue("ScanGeometry()", "layer distance 1 cm", "The geometry with layers can be scanned", ScanGeometry(Geometry, GetLayeredGeometry(1.0))) && Passed;
  if (Geometry.IsScanned() == false) return Passed;

  MREHit* Layer0 = CreateHit(MVector(1.0, 2.0, GetTrackerLayerZ(0)), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Layer3 = CreateHit(MVector(0.0, 0.0, GetTrackerLayerZ(3)), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Rotated = CreateHit(MVector(1.0, 30.0, 2.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* RotatedOther = CreateHit(MVector(-2.0, 31.0, -3.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Calorimeter = CreateHit(MVector(1.0, 2.0, -10.0), 100.0, MDDetector::c_Calorimeter, &Geometry);
  Passed = EvaluateTrue("CreateHit()", "trackers and calorimeter", "The hits are created", Layer0 != nullptr && Layer3 != nullptr && Rotated != nullptr && RotatedOther != nullptr && Calorimeter != nullptr) && Passed;
  if (Layer0 == nullptr || Layer3 == nullptr || Rotated == nullptr || RotatedOther == nullptr || Calorimeter == nullptr) return Passed;

  // The function compares the detector type (1) and the rotation of the wafers
  Passed = EvaluateTrue("AreInSameTracker()", "layer 0, layer 3", "Hits in the parallel wafers of one tower are in the same tracker", Geometry.AreInSameTracker(Layer0, Layer3)) && Passed;
  Passed = EvaluateTrue("AreInSameTracker()", "same hit", "A hit is in its own tracker", Geometry.AreInSameTracker(Layer0, Layer0)) && Passed;
  Passed = EvaluateTrue("AreInSameTracker()", "rotated tower", "Hits in the parallel wafers of the rotated tower are in the same tracker", Geometry.AreInSameTracker(Rotated, RotatedOther)) && Passed;
  Passed = EvaluateFalse("AreInSameTracker()", "tower and rotated tower", "Hits in wafers with different rotations are not in the same tracker", Geometry.AreInSameTracker(Layer0, Rotated)) && Passed;
  Passed = EvaluateFalse("AreInSameTracker()", "rotated tower and tower", "Hits in wafers with different rotations are not in the same tracker", Geometry.AreInSameTracker(Rotated, Layer0)) && Passed;
  Passed = EvaluateFalse("AreInSameTracker()", "tracker and calorimeter", "A calorimeter hit is in no tracker", Geometry.AreInSameTracker(Layer0, Calorimeter)) && Passed;
  Passed = EvaluateFalse("AreInSameTracker()", "calorimeter and tracker", "A calorimeter hit is in no tracker", Geometry.AreInSameTracker(Calorimeter, Layer0)) && Passed;
  Passed = EvaluateFalse("AreInSameTracker()", "calorimeter and calorimeter", "Calorimeter hits are in no tracker", Geometry.AreInSameTracker(Calorimeter, Calorimeter)) && Passed;

  // A hit which never got a volume sequence from the geometry has an empty one without deepest volume
  MREHit* Outside = CreateHit(MVector(50.0, 50.0, 50.0), 100.0, MDDetector::c_Strip2D);
  Passed = EvaluateFalse("AreInSameTracker()", "second hit without volumes", "A hit without volumes is in no tracker", Geometry.AreInSameTracker(Layer0, Outside)) && Passed;
  Passed = EvaluateFalse("AreInSameTracker()", "first hit without volumes", "A hit without volumes is in no tracker", Geometry.AreInSameTracker(Outside, Layer0)) && Passed;
  delete Outside;

  delete Layer0;
  delete Layer3;
  delete Rotated;
  delete RotatedOther;
  delete Calorimeter;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Call NoiseHit, which does not return
bool UTGeometryRevan::CallNoiseHit()
{
  MGeometryRevan Geometry;
  ScanTelescopeGeometry(Geometry);
  MVector Position(1.0, 2.0, 0.0);
  double Energy = 100.0;

  return Geometry.NoiseHit(Position, Energy);
}


////////////////////////////////////////////////////////////////////////////////


//! Test NoiseHit
bool UTGeometryRevan::TestNoiseHit()
{
  bool Passed = true;

  // Defect REV-D-2: NoiseHit is a public function which only consists of massert(false): it ends the program
  Passed = ExpectedToCrash(EvaluateTrue("NoiseHit()", "tracker hit of 100 keV", "A hit above the thresholds is noised and kept", CallNoiseHit())) && Passed;

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

  UTGeometryRevan Test;
  return Test.Run() == true ? 0 : 1;
}


// UTGeometryRevan.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
