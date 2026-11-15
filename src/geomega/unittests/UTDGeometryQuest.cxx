/*
 * UTDGeometryQuest.cxx
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
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

// POSIX libs:
// Process control has no equivalent in the C++ standard library
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"
#include "TRandom.h"

// MEGAlib libs:
#include "MComptonEvent.h"
#include "MDDetector.h"
#include "MDGeometryQuest.h"
#include "MDMaterial.h"
#include "MDVolume.h"
#include "MDVolumeSequence.h"
#include "UTGeomegaShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the geometry queries in MDGeometryQuest
class UTDGeometryQuest : public UTGeomegaBase
{
public:
  //! Default constructor
  UTDGeometryQuest() : UTGeomegaBase("UTDGeometryQuest") {}
  //! Default destructor
  virtual ~UTDGeometryQuest() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the text of the geometry for the path lengths
  MString GetPathGeometry() const;
  //! Return the text of the geometry with detectors
  MString GetDetectorGeometry() const;
  //! Return the length in one material or zero
  double GetLength(map<MDMaterial*, double>& Lengths, MDMaterial* Material) const;
  //! Return the result of the Compton intersection test with one box at the given position
  bool IntersectsBox(const MComptonEvent& Compton, const MVector& BoxPosition, double HalfLength);
  //! Return a position on the Compton cone (40 cm from the first interaction), at the given azimuth around the axis
  MVector GetPositionOnCone(const MComptonEvent& Compton, double Azimuth) const;

  //! Test the search of volumes and materials
  bool TestVolumeLookup();
  //! Test the sensitive check
  bool TestIsSensitive();
  //! Test the functions of a geometry which is not scanned
  bool TestNotScanned();
  //! Test the noising of hits
  bool TestNoise();
  //! Test the global options of the detectors
  bool TestGlobalOptions();
  //! Test the pulse shape interface
  bool TestApplyPulseShape();
  //! Test the resolutions
  bool TestGetResolutions();
  //! Test the energy resolution with a depth dependence
  bool TestGetResolutionsDepthCorrection();
  //! Test the name of the detector at a position
  bool TestGetDetectorName();
  //! Test the voxel comparison
  bool TestAreInSameVoxel();
  //! Test the number of volumes in the hierarchy
  bool TestGetNVolumesInHierarchy();
  //! Test the hits
  bool TestHits();
  //! Test the links
  bool TestLinks();
  //! Test drawing
  bool TestDrawGeometry();
  //! Test the path lengths in simple shapes with analytic results
  bool TestGetPathLengths();
  //! Test the path lengths in shapes which a straight line enters more than once
  bool TestGetPathLengthsReentry();
  //! Test the absorption probabilities against the analytic formula
  bool TestAbsorptionProbabilities();
  //! Test the Compton cone intersections
  bool TestGetComptonIntersection();
  //! Test the overlap check
  bool TestCheckOverlaps();
  //! Test the virtualization of volumes without a detector
  bool TestVirtualization();
  //! Test the gridding of a hit
  bool TestGrid();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDGeometryQuest::Run()
{
  bool Passed = true;

  Passed = TestVolumeLookup() && Passed;
  Passed = TestIsSensitive() && Passed;
  Passed = TestNotScanned() && Passed;
  Passed = TestNoise() && Passed;
  Passed = TestGlobalOptions() && Passed;
  Passed = TestApplyPulseShape() && Passed;
  Passed = TestGetResolutions() && Passed;
  Passed = TestGetResolutionsDepthCorrection() && Passed;
  Passed = TestGetDetectorName() && Passed;
  Passed = TestAreInSameVoxel() && Passed;
  Passed = TestGetNVolumesInHierarchy() && Passed;
  Passed = TestHits() && Passed;
  Passed = TestLinks() && Passed;
  Passed = TestDrawGeometry() && Passed;
  Passed = TestGetPathLengths() && Passed;
  Passed = TestGetPathLengthsReentry() && Passed;
  Passed = TestAbsorptionProbabilities() && Passed;
  Passed = TestGetComptonIntersection() && Passed;
  Passed = TestCheckOverlaps() && Passed;
  Passed = TestVirtualization() && Passed;
  Passed = TestGrid() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the text of the geometry for the path lengths
MString UTDGeometryQuest::GetPathGeometry() const
{
  MString Text = GetGeometryHead();

  // Two slabs along the z axis:
  Text += "Volume Slab\n";
  Text += "Slab.Material Aluminium\n";
  Text += "Slab.Shape BRIK 5.0 5.0 1.0\n";
  Text += "Slab.Position 0.0 0.0 10.0\n";
  Text += "Slab.Mother World\n";
  Text += "Volume Block\n";
  Text += "Block.Material Germanium\n";
  Text += "Block.Shape BRIK 5.0 5.0 1.5\n";
  Text += "Block.Position 0.0 0.0 20.0\n";
  Text += "Block.Mother World\n";

  // A rotated box:
  Text += "Volume Tilted\n";
  Text += "Tilted.Material Aluminium\n";
  Text += "Tilted.Shape BRIK 5.0 1.0 1.0\n";
  Text += "Tilted.Position 50.0 0.0 0.0\n";
  Text += "Tilted.Rotation 0.0 0.0 45.0\n";
  Text += "Tilted.Mother World\n";

  // A hollow tube around z and a hollow sphere:
  Text += "Volume Hollow\n";
  Text += "Hollow.Material Silicon\n";
  Text += "Hollow.Shape TUBS 3.0 5.0 10.0 0.0 360.0\n";
  Text += "Hollow.Position 0.0 50.0 0.0\n";
  Text += "Hollow.Mother World\n";
  Text += "Volume Shell\n";
  Text += "Shell.Material Silicon\n";
  Text += "Shell.Shape SPHE 2.0 5.0 0.0 180.0 0.0 360.0\n";
  Text += "Shell.Position 0.0 -50.0 0.0\n";
  Text += "Shell.Mother World\n";

  // Three disks with radius 5 on a common axis, connected by a thin rod with radius 1:
  Text += "Volume Comb\n";
  Text += "Comb.Material Silicon\n";
  Text += "Comb.Shape PCON 0.0 360.0 10  0.0 0.0 5.0  1.0 0.0 5.0  1.0 0.0 1.0  2.0 0.0 1.0  2.0 0.0 5.0  3.0 0.0 5.0  3.0 0.0 1.0  4.0 0.0 1.0  4.0 0.0 5.0  5.0 0.0 5.0\n";
  Text += "Comb.Position -50.0 0.0 0.0\n";
  Text += "Comb.Mother World\n";

  // A daughter in a rotated mother:
  Text += "Volume Nest\n";
  Text += "Nest.Material Aluminium\n";
  Text += "Nest.Shape BRIK 5.0 5.0 5.0\n";
  Text += "Nest.Position 0.0 0.0 -50.0\n";
  Text += "Nest.Rotation 90.0 0.0 0.0\n";
  Text += "Nest.Mother World\n";
  Text += "Volume Egg\n";
  Text += "Egg.Material Germanium\n";
  Text += "Egg.Shape BRIK 1.0 2.0 1.0\n";
  Text += "Egg.Position 0.0 0.0 0.0\n";
  Text += "Egg.Mother Nest\n";

  // A virtual group with a real volume:
  Text += "Volume Group\n";
  Text += "Group.Material Vacuum\n";
  Text += "Group.Shape BRIK 10.0 10.0 10.0\n";
  Text += "Group.Position 50.0 50.0 0.0\n";
  Text += "Group.Rotation 0.0 0.0 90.0\n";
  Text += "Group.Virtual true\n";
  Text += "Group.Mother World\n";
  Text += "Volume Member\n";
  Text += "Member.Material Germanium\n";
  Text += "Member.Shape BRIK 4.0 1.0 1.0\n";
  Text += "Member.Position 3.0 0.0 0.0\n";
  Text += "Member.Mother Group\n";

  return Text;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the text of the geometry with detectors
MString UTDGeometryQuest::GetDetectorGeometry() const
{
  // Two rotated placements of a cell, the detector volume, which contains a wafer, the sensitive volume, and a passive plug
  // The wafer has strips of pitch 1 cm in x and 2 cm in y
  MString Text = GetGeometryHead();
  Text += "Volume Holder\n";
  Text += "Holder.Material Aluminium\n";
  Text += "Holder.Shape BRIK 8.0 8.0 8.0\n";
  Text += "Volume Wafer\n";
  Text += "Wafer.Material Germanium\n";
  Text += "Wafer.Shape BRIK 5.0 5.0 1.0\n";
  Text += "Wafer.Position 0.0 0.0 2.0\n";
  Text += "Wafer.Mother Holder\n";
  Text += "Volume Plug\n";
  Text += "Plug.Material Aluminium\n";
  Text += "Plug.Shape BRIK 1.0 1.0 0.5\n";
  Text += "Plug.Position 3.0 3.0 0.0\n";
  Text += "Plug.Mother Wafer\n";
  Text += "Holder.Copy HolderA\n";
  Text += "HolderA.Position 20.0 0.0 0.0\n";
  Text += "HolderA.Rotation 0.0 0.0 90.0\n";
  Text += "HolderA.Mother World\n";
  Text += "Holder.Copy HolderB\n";
  Text += "HolderB.Position -20.0 0.0 0.0\n";
  Text += "HolderB.Mother World\n";
  Text += "Volume Passive\n";
  Text += "Passive.Material Aluminium\n";
  Text += "Passive.Shape BRIK 3.0 3.0 3.0\n";
  Text += "Passive.Position 0.0 50.0 0.0\n";
  Text += "Passive.Mother World\n";
  Text += "MDStrip2D Det\n";
  Text += "Det.DetectorVolume Wafer\n";
  Text += "Det.SensitiveVolume Wafer\n";
  Text += "Det.StructuralPitch 0.0 0.0 0.0\n";
  Text += "Det.StructuralOffset 0.0 0.0 0.0\n";
  Text += "Det.Offset 0.0 0.0\n";
  Text += "Det.StripNumber 10 5\n";
  Text += "Det.NoiseThreshold 1.0\n";
  Text += "Det.TriggerThreshold 1.0\n";
  Text += "Det.EnergyResolution Gauss 100.0 100.0 2.0\n";
  Text += "Det.EnergyResolution Gauss 1000.0 1000.0 2.0\n";
  Text += "Det.TimeResolutionAt 100.0 0.001\n";
  Text += "Det.TimeResolutionAt 1000.0 0.001\n";
  Text += "Trigger T\n";
  Text += "T.Veto false\n";
  Text += "T.TriggerByChannel true\n";
  Text += "T.Detector Det 1\n";
  return Text;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the length in one material or zero
double UTDGeometryQuest::GetLength(map<MDMaterial*, double>& Lengths, MDMaterial* Material) const
{
  map<MDMaterial*, double>::iterator Iter = Lengths.find(Material);
  if (Iter == Lengths.end()) {
    return 0.0;
  }
  return Iter->second;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the result of the Compton intersection test with one box at the given position
bool UTDGeometryQuest::IntersectsBox(const MComptonEvent& Compton, const MVector& BoxPosition, double HalfLength)
{
  ostringstream Text;
  Text<<GetGeometryHead();
  Text<<"Volume Box\n";
  Text<<"Box.Material Aluminium\n";
  Text<<"Box.Shape BRIK "<<HalfLength<<" "<<HalfLength<<" "<<HalfLength<<"\n";
  Text<<"Box.Position "<<BoxPosition.X()<<" "<<BoxPosition.Y()<<" "<<BoxPosition.Z()<<"\n";
  Text<<"Box.Mother World\n";

  MDGeometryQuest Geometry;
  if (ScanGeometry(Geometry, Text.str().c_str(), "Compton.geo.setup") == false) {
    return false;
  }

  DisableDefaultStreams();
  bool Result = Geometry.GetComptonIntersection(Compton);
  EnableDefaultStreams();

  return Result;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a position on the Compton cone (40 cm from the first interaction), at the given azimuth around the axis
MVector UTDGeometryQuest::GetPositionOnCone(const MComptonEvent& Compton, double Azimuth) const
{
  // The gamma ray came from the direction -Dg, the cone has the opening angle Phi around it
  MVector Axis = -1.0*Compton.Dg();

  // Create an orthonormal system around the axis:
  MVector Helper(1, 0, 0);
  if (fabs(Axis.X()) > 0.9) {
    Helper = MVector(0, 1, 0);
  }
  MVector U = Axis.Cross(Helper).Unit();
  MVector V = Axis.Cross(U).Unit();

  MVector Direction = cos(Compton.Phi())*Axis + sin(Compton.Phi())*(cos(Azimuth)*U + sin(Azimuth)*V);

  return Compton.C1() + 40.0*Direction;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search of volumes and materials
bool UTDGeometryQuest::TestVolumeLookup()
{
  bool Passed = true;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "path geometry", "The geometry can be read", ScanGeometry(Geometry, GetPathGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  // By position:
  Passed = EvaluateTrue("GetVolume()", "in the slab", "The slab is found", Geometry.GetVolume(MVector(0, 0, 10)) == Geometry.GetVolume("Slab")) && Passed;
  Passed = EvaluateTrue("GetVolume()", "in the world", "The world is found", Geometry.GetVolume(MVector(0, 0, 0)) == Geometry.GetWorldVolume()) && Passed;
  Passed = EvaluateTrue("GetVolume()", "outside", "Outside of the world zero is returned", Geometry.GetVolume(MVector(1000, 0, 0)) == nullptr) && Passed;
  Passed = EvaluateTrue("GetVolume()", "by name", "The search by name still works", Geometry.GetVolume("Slab") != nullptr) && Passed;

  // Boundaries:
  Passed = EvaluateTrue("GetVolume()", "1e-9 inside of the slab", "A point just inside is in the slab", Geometry.GetVolume(MVector(0, 0, 11.0 - 1e-9)) == Geometry.GetVolume("Slab")) && Passed;
  Passed = EvaluateTrue("GetVolume()", "1e-9 outside of the slab", "A point just outside is in the world", Geometry.GetVolume(MVector(0, 0, 11.0 + 1e-9)) == Geometry.GetWorldVolume()) && Passed;

  // A rotated mother with half length (1, 2, 1) in a volume rotated by 90 deg around x: In the world y is +-1, z is -50 +- 2
  Passed = EvaluateTrue("GetVolume()", "rotated", "The rotated long axis is along z", Geometry.GetVolume(MVector(0, 0, -51.5)) == Geometry.GetVolume("Egg")) && Passed;
  Passed = EvaluateTrue("GetVolume()", "rotated", "The rotated short axis is along y", Geometry.GetVolume(MVector(0, 1.5, -50)) == Geometry.GetVolume("Nest")) && Passed;

  // Embedding volume:
  Passed = EvaluateTrue("GetEmbeddingVolume()", "in the slab", "The slab is found", Geometry.GetEmbeddingVolume(MVector(0, 0, 10)) == Geometry.GetVolume("Slab")) && Passed;
  Passed = EvaluateTrue("GetEmbeddingVolume()", "outside", "Outside of the world zero is returned", Geometry.GetEmbeddingVolume(MVector(1000, 0, 0)) == nullptr) && Passed;

  // Material:
  Passed = EvaluateTrue("GetMaterial()", "in the slab", "The material of the slab is found", Geometry.GetMaterial(MVector(0, 0, 10)) == Geometry.GetMaterial("Aluminium")) && Passed;
  Passed = EvaluateTrue("GetMaterial()", "in the block", "The material of the block is found", Geometry.GetMaterial(MVector(0, 0, 20)) == Geometry.GetMaterial("Germanium")) && Passed;
  Passed = EvaluateTrue("GetMaterial()", "in the world", "The material of the world is found", Geometry.GetMaterial(MVector(0, 0, 0)) == Geometry.GetMaterial("Vacuum")) && Passed;
  Passed = EvaluateTrue("GetMaterial()", "outside", "Outside of the world zero is returned", Geometry.GetMaterial(MVector(1000, 0, 0)) == nullptr) && Passed;

  // In a virtual group: The member is rotated with the group and at (50, 53, 0), with half length 1 in x and 4 in y
  Passed = EvaluateTrue("GetVolume()", "real in a virtual volume", "The real volume is found", Geometry.GetVolume(MVector(50.5, 56.5, 0.5)) == Geometry.GetVolume("Member")) && Passed;
  Passed = EvaluateTrue("GetVolume()", "virtual volume", "The former virtual volume does not exist anymore in the tree", Geometry.GetVolume(MVector(55, 45, 0)) == Geometry.GetWorldVolume()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the sensitive check
bool UTDGeometryQuest::TestIsSensitive()
{
  bool Passed = true;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "detector geometry", "The geometry can be read", ScanGeometry(Geometry, GetDetectorGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  // The wafer in holder B is at (-20, 0, 2) with half lengths (5, 5, 1)
  Passed = EvaluateTrue("IsSensitive()", "in the wafer", "A position in the wafer is sensitive", Geometry.IsSensitive(MVector(-20, 0, 2))) && Passed;
  Passed = EvaluateFalse("IsSensitive()", "in the holder", "A position in the holder is not sensitive", Geometry.IsSensitive(MVector(-20, 7, 0))) && Passed;
  Passed = EvaluateFalse("IsSensitive()", "in the world", "A position in the world is not sensitive", Geometry.IsSensitive(MVector(0, 0, 0))) && Passed;
  Passed = EvaluateFalse("IsSensitive()", "passive", "A position in a passive volume is not sensitive", Geometry.IsSensitive(MVector(0, 50, 0))) && Passed;

  // The plug in the wafer is at (3, 3, 0) in the wafer, thus at (-17, 3, 2)
  // Defect GEO-B-28: IsSensitive checks for a detector, which the passive daughters of a detector volume have too, and not for a sensitive volume
  Passed = ExpectedToFail(EvaluateFalse("IsSensitive()", "in the plug", "A position in the passive plug in the wafer is not sensitive", Geometry.IsSensitive(MVector(-17, 3, 2)))) && Passed;

  // Defect GEO-B-29: IsSensitive dereferences the volume without a check, thus a position outside of the world crashes
  Passed = ExpectedToCrash(EvaluateFalse("IsSensitive()", "outside of the world", "A position outside of the world is not sensitive, and the call does not crash", Geometry.IsSensitive(MVector(1000, 0, 0)))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the functions of a geometry which is not scanned
bool UTDGeometryQuest::TestNotScanned()
{
  bool Passed = true;
  MString Flags;

  MDGeometryQuest Geometry;

  // These functions have a check:
  DisableDefaultStreams();
  bool Sensitive = Geometry.IsSensitive(MVector(0, 0, 0));
  MDVolume* Volume = Geometry.GetVolume(MVector(0, 0, 0));
  MVector Position(1, 2, 3);
  double Energy = 100.0;
  double Time = 0.0;
  MDVolumeSequence VS;
  bool Noised = Geometry.Noise(Position, Energy, Time, Flags, VS);
  Geometry.ApplyPulseShape(0.0, Position, Energy);
  bool Drawn = Geometry.DrawGeometry();
  EnableDefaultStreams();
  Passed = EvaluateFalse("IsSensitive()", "not scanned", "A geometry which is not scanned has no sensitive volumes", Sensitive) && Passed;
  Passed = EvaluateTrue("GetVolume()", "not scanned", "A geometry which is not scanned has no volumes", Volume == nullptr) && Passed;
  Passed = EvaluateFalse("Noise()", "not scanned", "A geometry which is not scanned cannot noise", Noised) && Passed;
  Passed = EvaluateVectorNear("ApplyPulseShape()", "not scanned", "The position is not changed", Position, MVector(1, 2, 3), 1e-15) && Passed;
  Passed = EvaluateFalse("DrawGeometry()", "not scanned", "A geometry which is not scanned cannot be drawn", Drawn) && Passed;

  // These ones do not have the check of the others:
  // Defect GEO-B-30: A number of functions dereference the world volume of a geometry which is not scanned without a check
  Passed = ExpectedToCrash(EvaluateTrue("GetMaterial()", "not scanned", "The call returns no material and does not crash", Geometry.GetMaterial(MVector(0, 0, 0)) == nullptr)) && Passed;
  // Defect GEO-B-30: A number of functions dereference the world volume of a geometry which is not scanned without a check
  Passed = ExpectedToCrash(EvaluateTrue("GetEmbeddingVolume()", "not scanned", "The call returns no volume and does not crash", Geometry.GetEmbeddingVolume(MVector(0, 0, 0)) == nullptr)) && Passed;
  // Defect GEO-B-30: A number of functions dereference the world volume of a geometry which is not scanned without a check
  Passed = ExpectedToCrash(EvaluateTrue("GetPathLengths()", "not scanned", "The call returns no path lengths and does not crash", Geometry.GetPathLengths(MVector(0, 0, 0), MVector(1, 0, 0)).empty())) && Passed;
  // Defect GEO-B-30: A number of functions dereference the world volume of a geometry which is not scanned without a check
  Passed = ExpectedToCrash(EvaluateTrue("GetAbsorptionProbability()", "not scanned", "The call does not crash", (Geometry.GetAbsorptionProbability(MVector(0, 0, 0), MVector(1, 0, 0), 100.0), true))) && Passed;
  // Defect GEO-B-30: A number of functions dereference the world volume of a geometry which is not scanned without a check
  Passed = ExpectedToCrash(EvaluateTrue("GetDetectorName()", "not scanned", "The call returns an empty name and does not crash", Geometry.GetDetectorName(MVector(0, 0, 0)).IsEmpty())) && Passed;
  // Defect GEO-B-30: A number of functions dereference the world volume of a geometry which is not scanned without a check
  MVector NoisePosition(0, 0, 0);
  double NoiseEnergy = 0.0;
  double NoiseTime = 0.0;
  Passed = ExpectedToCrash(EvaluateFalse("Noise()", "not scanned, no sequence", "The call returns false and does not crash", Geometry.Noise(NoisePosition, NoiseEnergy, NoiseTime, Flags))) && Passed;
  // Defect GEO-B-30: A number of functions dereference the world volume of a geometry which is not scanned without a check
  MVector PositionResolution;
  double EnergyResolution = 0.0;
  double TimeResolution = 0.0;
  Passed = ExpectedToCrash(EvaluateTrue("GetResolutions()", "not scanned, no sequence", "The call does not crash", (Geometry.GetResolutions(MVector(0, 0, 0), 100.0, 0.0, PositionResolution, EnergyResolution, TimeResolution), true))) && Passed;
  // Defect GEO-B-30: A number of functions dereference the world volume of a geometry which is not scanned without a check
  Passed = ExpectedToCrash(EvaluateTrue("RemoveAllHits()", "not scanned, one hit", "The call does not crash", (Geometry.AddHit("Hit", MVector(0, 0, 0), 1.0), Geometry.RemoveAllHits(), true))) && Passed;
  // Defect GEO-B-30: A number of functions dereference the world volume of a geometry which is not scanned without a check
  Passed = ExpectedToCrash(EvaluateTrue("RemoveAllLinks()", "not scanned, one link", "The call does not crash", (Geometry.AddLink(MVector(0, 0, 0), MVector(0, 0, 1), 1.0), Geometry.RemoveAllLinks(), true))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the noising of hits
bool UTDGeometryQuest::TestNoise()
{
  bool Passed = true;
  MString Flags;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "detector geometry", "The geometry can be read", ScanGeometry(Geometry, GetDetectorGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  // The wafer in holder A is rotated by 90 deg around z: The point (1, 2, 2.5) in the holder is at (-2, 1, 2.5) + (20, 0, 0)
  const MVector InWafer(18, 1, 2.5);

  // The strip detector keeps the position: Down and up has to restore it
  gRandom->SetSeed(4711);
  MVector Position = InWafer;
  double Energy = 100.0;
  double Time = 1.0;
  DisableDefaultStreams();
  bool Result = Geometry.Noise(Position, Energy, Time, Flags);
  EnableDefaultStreams();
  Passed = EvaluateTrue("Noise()", "in the wafer", "The hit is noised", Result) && Passed;
  Passed = EvaluateVectorNear("Noise()", "in the wafer", "The position is restored", Position, InWafer, 1e-13) && Passed;

  // Statistics: Gauss with sigma 2 keV, 5 standard errors are allowed
  gRandom->SetSeed(4712);
  const unsigned int N = 2000;
  const double Sigma = 2.0;
  double Sum = 0.0;
  double Sum2 = 0.0;
  MDVolumeSequence VS = Geometry.GetVolumeSequence(InWafer, false, false);
  for (unsigned int i = 0; i < N; ++i) {
    MVector P = InWafer;
    double E = 100.0;
    double T = 1.0;
    Geometry.Noise(P, E, T, Flags, VS);
    Sum += E;
    Sum2 += E*E;
  }
  const double Mean = Sum/N;
  const double RMS = sqrt(Sum2/N - Mean*Mean);
  Passed = EvaluateNear("Noise()", "2000 trials", "The mean energy is the input energy", Mean, 100.0, 5.0*Sigma/sqrt(static_cast<double>(N))) && Passed;
  Passed = EvaluateNear("Noise()", "2000 trials", "The width of the energy is the resolution", RMS, Sigma, 5.0*Sigma/sqrt(2.0*N)) && Passed;

  // No detector:
  MVector PassivePosition(0, 50, 0);
  double PassiveEnergy = 100.0;
  double PassiveTime = 1.0;
  MDVolumeSequence PassiveVS = Geometry.GetVolumeSequence(PassivePosition, false, false);
  DisableDefaultStreams();
  Result = Geometry.Noise(PassivePosition, PassiveEnergy, PassiveTime, Flags, PassiveVS);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Noise()", "passive", "A hit in a passive volume is not noised", Result) && Passed;
  Passed = EvaluateNear("Noise()", "passive", "The energy of a hit in a passive volume is zero", PassiveEnergy, 0.0, 1e-15) && Passed;
  Passed = EvaluateVectorNear("Noise()", "passive", "The position of a hit in a passive volume is not changed", PassivePosition, MVector(0, 50, 0), 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the global options of the detectors
bool UTDGeometryQuest::TestGlobalOptions()
{
  bool Passed = true;
  MString Flags;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("GetActivateNoising()", "new", "The noising is active by default", Geometry.GetActivateNoising()) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "detector geometry", "The geometry can be read", ScanGeometry(Geometry, GetDetectorGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  const MVector InWafer(18, 1, 2.5);

  // No noising: The energy stays
  Geometry.ActivateNoising(false);
  Passed = EvaluateFalse("ActivateNoising()", "false", "The flag is stored", Geometry.GetActivateNoising()) && Passed;
  MVector Position = InWafer;
  double Energy = 100.0;
  double Time = 1.0;
  DisableDefaultStreams();
  Geometry.Noise(Position, Energy, Time, Flags);
  EnableDefaultStreams();
  Passed = EvaluateNear("ActivateNoising()", "false", "Without noising the energy stays", Energy, 100.0, 1e-13) && Passed;
  Geometry.ActivateNoising();
  Passed = EvaluateTrue("ActivateNoising()", "default argument", "The default argument activates the noising", Geometry.GetActivateNoising()) && Passed;

  // Failure rate:
  Geometry.SetGlobalFailureRate(1.0);
  Passed = EvaluateNear("SetGlobalFailureRate()", "1", "The detector has the failure rate", Geometry.GetDetectorAt(0)->GetFailureRate(), 1.0, 1e-15) && Passed;
  Energy = 100.0;
  Position = InWafer;
  DisableDefaultStreams();
  Geometry.Noise(Position, Energy, Time, Flags);
  EnableDefaultStreams();
  Passed = EvaluateNear("SetGlobalFailureRate()", "1", "A failed detector measures nothing", Energy, 0.0, 1e-15) && Passed;
  Geometry.SetGlobalFailureRate();
  Passed = EvaluateNear("SetGlobalFailureRate()", "default argument", "The default argument is rate 0", Geometry.GetDetectorAt(0)->GetFailureRate(), 0.0, 1e-15) && Passed;

  // There is no way to read the flag:
  Geometry.AllwaysAssumeTrigger(true);
  Geometry.AllwaysAssumeTrigger(false);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the pulse shape interface
bool UTDGeometryQuest::TestApplyPulseShape()
{
  bool Passed = true;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "detector geometry", "The geometry can be read", ScanGeometry(Geometry, GetDetectorGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  // The position comes back unchanged also if the volume has a daughter it is not in
  const MVector InWafer(18, 1, 2.5);
  MVector Position = InWafer;
  double Energy = 100.0;
  Geometry.ApplyPulseShape(0.0, Position, Energy);
  Passed = EvaluateVectorNear("ApplyPulseShape()", "in the wafer", "The position is restored", Position, InWafer, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the resolutions
bool UTDGeometryQuest::TestGetResolutions()
{
  bool Passed = true;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "detector geometry", "The geometry can be read", ScanGeometry(Geometry, GetDetectorGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  const double Root12 = sqrt(12.0);

  // Holder B is not rotated: pitch / sqrt(12)
  MVector PRes;
  double ERes = 0.0;
  double TRes = 0.0;
  Geometry.GetResolutions(MVector(-20, 0, 2), 100.0, 1.0, PRes, ERes, TRes);
  Passed = EvaluateNear("GetResolutions()", "not rotated", "The x resolution is the x pitch of 1 cm / sqrt(12)", PRes.X(), 1.0/Root12, 1e-13) && Passed;
  Passed = EvaluateNear("GetResolutions()", "not rotated", "The y resolution is the y pitch of 2 cm / sqrt(12)", PRes.Y(), 2.0/Root12, 1e-13) && Passed;
  Passed = EvaluateNear("GetResolutions()", "not rotated", "The energy resolution is the one of the file", ERes, 2.0, 1e-13) && Passed;
  Passed = EvaluateNear("GetResolutions()", "not rotated", "The time resolution is the one of the file", TRes, 0.001, 1e-13) && Passed;
  // Defect GEO-B-31: MDStrip2D::GetPositionResolution divides the half thickness and not the full thickness by sqrt(12)
  Passed = ExpectedToFail(EvaluateNear("GetResolutions()", "not rotated", "The z resolution is the full thickness of 2 cm / sqrt(12)", PRes.Z(), 2.0/Root12, 1e-13)) && Passed;

  // Holder A is rotated by 90 deg around z: x and y are exchanged
  MVector RotatedPRes;
  Geometry.GetResolutions(MVector(20, 0, 2), 100.0, 1.0, RotatedPRes, ERes, TRes);
  Passed = EvaluateNear("GetResolutions()", "rotated by 90 deg", "The x resolution is from the y pitch", RotatedPRes.X(), 2.0/Root12, 1e-13) && Passed;
  Passed = EvaluateNear("GetResolutions()", "rotated by 90 deg", "The y resolution is from the x pitch", RotatedPRes.Y(), 1.0/Root12, 1e-13) && Passed;

  // With a given sequence:
  MDVolumeSequence VS = Geometry.GetVolumeSequence(MVector(-20, 0, 2), false, false);
  MVector SequencePRes;
  Geometry.GetResolutions(MVector(-20, 0, 2), 100.0, 1.0, VS, SequencePRes, ERes, TRes);
  Passed = EvaluateVectorNear("GetResolutions()", "with sequence", "The result is the same as without the sequence", SequencePRes, PRes, 1e-15) && Passed;

  // Outside of a detector everything is zero and it is reported
  MVector PassivePRes(1, 1, 1);
  ERes = 1.0;
  TRes = 1.0;
  DisableDefaultStreams();
  Geometry.GetResolutions(MVector(0, 50, 0), 100.0, 1.0, PassivePRes, ERes, TRes);
  EnableDefaultStreams();
  Passed = EvaluateVectorNear("GetResolutions()", "passive", "The position resolution is zero", PassivePRes, MVector(0, 0, 0), 1e-15) && Passed;
  Passed = EvaluateNear("GetResolutions()", "passive", "The energy resolution is zero", ERes, 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetResolutions()", "passive", "The time resolution is zero", TRes, 0.0, 1e-15) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the energy resolution with a depth dependence
bool UTDGeometryQuest::TestGetResolutionsDepthCorrection()
{
  bool Passed = true;

  // The energy resolution scales with 1 at the bottom and 3 at the top of a detector which is far from the origin in z
  MString Text = GetGeometryHead();
  Text += "Volume Wafer\n";
  Text += "Wafer.Material Germanium\n";
  Text += "Wafer.Shape BRIK 5.0 5.0 1.0\n";
  Text += "Wafer.Position 0.0 0.0 30.0\n";
  Text += "Wafer.Mother World\n";
  Text += "MDStrip3D Det\n";
  Text += "Det.DetectorVolume Wafer\n";
  Text += "Det.SensitiveVolume Wafer\n";
  Text += "Det.StructuralPitch 0.0 0.0 0.0\n";
  Text += "Det.StructuralOffset 0.0 0.0 0.0\n";
  Text += "Det.Offset 0.0 0.0\n";
  Text += "Det.StripNumber 10 10\n";
  Text += "Det.NoiseThreshold 1.0\n";
  Text += "Det.TriggerThreshold 1.0\n";
  Text += "Det.DepthResolution 100.0 0.1\n";
  Text += "Det.DepthResolution 1000.0 0.1\n";
  Text += "Det.EnergyResolution Gauss 100.0 100.0 2.0\n";
  Text += "Det.EnergyResolution Gauss 1000.0 1000.0 2.0\n";
  Text += "Det.EnergyResolutionDepthCorrection Gauss -1.0 1.0 1.0\n";
  Text += "Det.EnergyResolutionDepthCorrection Gauss 1.0 1.0 3.0\n";

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "depth correction", "The geometry can be read", ScanGeometry(Geometry, Text)) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  // The world position (0, 0, 30) is the center of the detector: The factor is 2, thus the resolution 4 keV
  MVector PRes;
  double ERes = 0.0;
  double TRes = 0.0;
  Geometry.GetResolutions(MVector(0, 0, 30), 100.0, 1.0, PRes, ERes, TRes);
  // Defect GEO-B-32: GetResolutions hands the position in the world and not the position in the detector to the energy resolution
  Passed = ExpectedToFail(EvaluateNear("GetResolutions()", "center of a detector at z = 30 cm", "The energy resolution is the one at the depth in the detector", ERes, 4.0, 1e-10)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the name of the detector at a position
bool UTDGeometryQuest::TestGetDetectorName()
{
  bool Passed = true;

  // The detector volume is a volume with clones here:
  MString Text = GetGeometryHead();
  Text += "Volume Holder\n";
  Text += "Holder.Material Aluminium\n";
  Text += "Holder.Shape BRIK 8.0 8.0 8.0\n";
  Text += "Volume Wafer\n";
  Text += "Wafer.Material Germanium\n";
  Text += "Wafer.Shape BRIK 5.0 5.0 1.0\n";
  Text += "Wafer.Position 0.0 0.0 0.0\n";
  Text += "Wafer.Mother Holder\n";
  Text += "Wafer.Copy WaferCopy\n";
  Text += "WaferCopy.Position 0.0 0.0 4.0\n";
  Text += "WaferCopy.Mother Holder\n";
  Text += "Holder.Copy HolderA\n";
  Text += "HolderA.Position 20.0 0.0 0.0\n";
  Text += "HolderA.Mother World\n";
  Text += "Volume Single\n";
  Text += "Single.Material Germanium\n";
  Text += "Single.Shape BRIK 5.0 5.0 1.0\n";
  Text += "Single.Position -20.0 0.0 0.0\n";
  Text += "Single.Mother World\n";
  Text += "MDStrip2D Det\n";
  Text += "Det.DetectorVolume Wafer\n";
  Text += "Det.SensitiveVolume Wafer\n";
  Text += "Det.StructuralPitch 0.0 0.0 0.0\n";
  Text += "Det.StructuralOffset 0.0 0.0 0.0\n";
  Text += "Det.Offset 0.0 0.0\n";
  Text += "Det.StripNumber 10 10\n";
  Text += "Det.NoiseThreshold 1.0\n";
  Text += "Det.TriggerThreshold 1.0\n";
  Text += "Det.EnergyResolution Ideal\n";
  Text += "MDStrip2D SingleDet\n";
  Text += "SingleDet.DetectorVolume Single\n";
  Text += "SingleDet.SensitiveVolume Single\n";
  Text += "SingleDet.StructuralPitch 0.0 0.0 0.0\n";
  Text += "SingleDet.StructuralOffset 0.0 0.0 0.0\n";
  Text += "SingleDet.Offset 0.0 0.0\n";
  Text += "SingleDet.StripNumber 10 10\n";
  Text += "SingleDet.NoiseThreshold 1.0\n";
  Text += "SingleDet.TriggerThreshold 1.0\n";
  Text += "SingleDet.EnergyResolution Ideal\n";

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "detector with clones", "The geometry can be read", ScanGeometry(Geometry, Text)) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  Passed = EvaluateTrue("GetDetectorName()", "in the template", "The name of the template is returned", Geometry.GetDetectorName(MVector(20, 0, 0)) == "Wafer") && Passed;
  Passed = EvaluateTrue("GetDetectorName()", "in the clone", "The name of the clone is returned", Geometry.GetDetectorName(MVector(20, 0, 4)) == "WaferCopy") && Passed;
  Passed = EvaluateTrue("GetDetectorName()", "without clones", "The name of the volume is returned", Geometry.GetDetectorName(MVector(-20, 0, 0)) == "Single") && Passed;
  Passed = EvaluateTrue("GetDetectorName()", "in the holder", "A position without detector has no name", Geometry.GetDetectorName(MVector(20, 7, 0)) == "") && Passed;
  Passed = EvaluateTrue("GetDetectorName()", "outside", "A position outside has no name", Geometry.GetDetectorName(MVector(1000, 0, 0)) == "") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the voxel comparison
bool UTDGeometryQuest::TestAreInSameVoxel()
{
  bool Passed = true;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "detector geometry", "The geometry can be read", ScanGeometry(Geometry, GetDetectorGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  // In the wafer of holder B: Pitch 1 cm in x, the strip boundaries are at integer positions
  const MVector First(-22.5, 0.5, 2.0);
  const MVector SameStrip(-22.45, 0.55, 2.0);
  const MVector OtherStrip(-20.5, 0.5, 2.0);
  const MVector OtherHolder(20.0, 0.5, 2.0);

  Passed = EvaluateFalse("AreInSameVoxel()", "far apart in x", "Two positions which are two strips apart are not in the same voxel", Geometry.AreInSameVoxel(First, OtherStrip)) && Passed;
  Passed = EvaluateFalse("AreInSameVoxel()", "in two placements", "Two positions in two placements are not in the same voxel", Geometry.AreInSameVoxel(First, OtherHolder)) && Passed;

  // Defect GEO-B-33: AreInSameVoxel calls AreNear with sigma 0, which never is true since the depth difference has to be below 0
  Passed = ExpectedToFail(EvaluateTrue("AreInSameVoxel()", "identical position", "A position is in the same voxel as itself", Geometry.AreInSameVoxel(First, First))) && Passed;
  // Defect GEO-B-33: AreInSameVoxel calls AreNear with sigma 0, which never is true since the depth difference has to be below 0
  Passed = ExpectedToFail(EvaluateTrue("AreInSameVoxel()", "same strips", "Two positions in the same strips are in the same voxel", Geometry.AreInSameVoxel(First, SameStrip))) && Passed;

  // Defect GEO-B-34: AreInSameVoxel dereferences the detector of a passive volume without a check
  Passed = ExpectedToCrash(EvaluateFalse("AreInSameVoxel()", "two passive positions", "Two positions without a detector are not in the same voxel, and the call does not crash", Geometry.AreInSameVoxel(MVector(0, 50, 0), MVector(0, 51, 0)))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the number of volumes in the hierarchy
bool UTDGeometryQuest::TestGetNVolumesInHierarchy()
{
  bool Passed = true;

  MDGeometryQuest Geometry;
  MString Text = GetGeometryHead();
  Text += "Volume Box\n";
  Text += "Box.Material Aluminium\n";
  Text += "Box.Shape BRIK 2.0 3.0 4.0\n";
  Text += "Box.Mother World\n";
  Passed = EvaluateTrue("ScanSetupFile()", "world and one box", "The geometry can be read", ScanGeometry(Geometry, Text)) && Passed;
  if (Geometry.IsScanned() == true) {
    // Defect GEO-B-35: GetNVolumesInHierarchy is an empty function which always returns zero
    Passed = ExpectedToFail(Evaluate("GetNVolumesInHierarchy()", "world and one box", "Two volumes are in the hierarchy", Geometry.GetNVolumesInHierarchy(), 2)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the hits
bool UTDGeometryQuest::TestHits()
{
  bool Passed = true;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "path geometry", "The geometry can be read", ScanGeometry(Geometry, GetPathGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  MDVolume* World = Geometry.GetWorldVolume();
  const unsigned int NDaughters = World->GetNDaughters();

  DisableDefaultStreams();
  Geometry.AddHit("HitA", MVector(30, 30, 30), 1.5, 3);
  Geometry.AddHit("HitB", MVector(-30, -30, -30), 1.0);
  EnableDefaultStreams();
  Passed = Evaluate("AddHit()", "two hits", "The hits are daughters of the world", World->GetNDaughters(), NDaughters + 2) && Passed;

  MDVolume* Hit = Geometry.GetVolume(MVector(30, 30, 31.4));
  Passed = EvaluateTrue("AddHit()", "1.4 from the center", "A point in the sphere is in the hit", Hit != nullptr && Hit->GetName() == "HitA") && Passed;
  Passed = EvaluateTrue("AddHit()", "1.6 from the center", "A point outside of the sphere is not in the hit", Geometry.GetVolume(MVector(30, 30, 31.6)) == World) && Passed;
  if (Hit != nullptr && Hit->GetName() == "HitA") {
    Passed = Evaluate("AddHit()", "color 3", "The color is stored", Hit->GetColor(), 3) && Passed;
    Passed = Evaluate("AddHit()", "visible", "A hit is visible", Hit->GetVisibility(), 1) && Passed;
  }

  Geometry.RemoveAllHits();
  Passed = Evaluate("RemoveAllHits()", "two hits", "The hits are removed from the world", World->GetNDaughters(), NDaughters) && Passed;
  Passed = EvaluateTrue("RemoveAllHits()", "two hits", "The position is in the world again", Geometry.GetVolume(MVector(30, 30, 30)) == World) && Passed;

  // Empty and repeated:
  Geometry.RemoveAllHits();
  Passed = Evaluate("RemoveAllHits()", "no hits", "Nothing happens without hits", World->GetNDaughters(), NDaughters) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the links
bool UTDGeometryQuest::TestLinks()
{
  bool Passed = true;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "path geometry", "The geometry can be read", ScanGeometry(Geometry, GetPathGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  MDVolume* World = Geometry.GetWorldVolume();
  const unsigned int NDaughters = World->GetNDaughters();

  // A link in a general direction - the tube of radius 0.5 between the two points
  const MVector A(30, 30, 30);
  const MVector B(40, 25, 42);
  DisableDefaultStreams();
  Geometry.AddLink(A, B, 0.5, 4);
  EnableDefaultStreams();
  Passed = Evaluate("AddLink()", "one link", "The link is a daughter of the world", World->GetNDaughters(), NDaughters + 1) && Passed;

  const MVector Direction = (B - A).Unit();
  const MVector Perpendicular = Direction.Cross(MVector(0, 0, 1)).Unit();
  for (double Fraction : { 0.01, 0.25, 0.5, 0.75, 0.99 }) {
    MVector OnLink = A + Fraction*(B - A);
    MDVolume* Found = Geometry.GetVolume(OnLink);
    Passed = EvaluateTrue("AddLink()", "on the axis", "A point on the line between both points is in the link", Found != nullptr && Found->GetName() == "A Link") && Passed;
    Found = Geometry.GetVolume(OnLink + 0.4*Perpendicular);
    Passed = EvaluateTrue("AddLink()", "0.4 beside the axis", "A point within the radius is in the link", Found != nullptr && Found->GetName() == "A Link") && Passed;
    Passed = EvaluateTrue("AddLink()", "0.6 beside the axis", "A point outside of the radius is not in the link", Geometry.GetVolume(OnLink + 0.6*Perpendicular) == World) && Passed;
  }
  Passed = EvaluateTrue("AddLink()", "behind the end", "A point behind the second point is not in the link", Geometry.GetVolume(B + 0.1*Direction) == World) && Passed;
  Passed = EvaluateTrue("AddLink()", "before the start", "A point before the first point is not in the link", Geometry.GetVolume(A - 0.1*Direction) == World) && Passed;

  Geometry.RemoveAllLinks();
  Passed = Evaluate("RemoveAllLinks()", "one link", "The link is removed from the world", World->GetNDaughters(), NDaughters) && Passed;
  Geometry.RemoveAllLinks();
  Passed = Evaluate("RemoveAllLinks()", "no links", "Nothing happens without links", World->GetNDaughters(), NDaughters) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test drawing
bool UTDGeometryQuest::TestDrawGeometry()
{
  bool Passed = true;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "path geometry", "The geometry can be read", ScanGeometry(Geometry, GetPathGeometry())) && Passed;
  if (Geometry.IsScanned() == true) {
    // Defect GEO-A-68: The drawing always opens the OpenGL viewer, which crashes in batch mode
    Passed = ExpectedToCrash(EvaluateTrue("DrawGeometry()", "scanned, batch mode", "The geometry can be drawn in batch mode", Geometry.DrawGeometry())) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the path lengths in simple shapes with analytic results
bool UTDGeometryQuest::TestGetPathLengths()
{
  bool Passed = true;

  // Each crossing into a volume starts 1e-7 cm behind the surface, thus the lengths are exact up to a few 1e-7 cm
  const double Tolerance = 1e-6;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "path geometry", "The geometry can be read", ScanGeometry(Geometry, GetPathGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  MDMaterial* Aluminium = Geometry.GetMaterial("Aluminium");
  MDMaterial* Germanium = Geometry.GetMaterial("Germanium");
  MDMaterial* Silicon = Geometry.GetMaterial("Silicon");
  MDMaterial* Vacuum = Geometry.GetMaterial("Vacuum");

  // Along the z axis through both slabs:
  map<MDMaterial*, double> Lengths = Geometry.GetPathLengths(MVector(0, 0, 0), MVector(0, 0, 30));
  Passed = EvaluateNear("GetPathLengths()", "through two slabs", "The aluminium slab is 2 cm thick", GetLength(Lengths, Aluminium), 2.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetPathLengths()", "through two slabs", "The germanium slab is 3 cm thick", GetLength(Lengths, Germanium), 3.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetPathLengths()", "through two slabs", "The rest is in the vacuum", GetLength(Lengths, Vacuum), 25.0, Tolerance) && Passed;

  // A tilted line: Through the slab with the factor 1 / cos(theta), start and stop are on the line through (0, 0, 10)
  const double Tilt = 30.0*c_Rad;
  const MVector Direction(sin(Tilt), 0, cos(Tilt));
  Lengths = Geometry.GetPathLengths(MVector(0, 0, 10) - 4.0*Direction, MVector(0, 0, 10) + 4.0*Direction);
  Passed = EvaluateNear("GetPathLengths()", "tilted by 30 deg", "The path in the 2 cm slab is 2 / cos(30 deg)", GetLength(Lengths, Aluminium), 2.0/cos(Tilt), Tolerance) && Passed;
  Passed = EvaluateNear("GetPathLengths()", "tilted by 30 deg", "The total path is the distance", GetLength(Lengths, Aluminium) + GetLength(Lengths, Vacuum), 8.0, Tolerance) && Passed;

  // Start and stop in the volume:
  Lengths = Geometry.GetPathLengths(MVector(0, 0, 9.5), MVector(0, 0, 10.5));
  Passed = EvaluateNear("GetPathLengths()", "only in the slab", "The path is the distance", GetLength(Lengths, Aluminium), 1.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetPathLengths()", "only in the slab", "Nothing is in the world", GetLength(Lengths, Vacuum), 0.0, Tolerance) && Passed;

  // Rotated box with half width 1 by 45 deg: 2 * 1 / sin(45 deg), line along the x axis through its center
  Lengths = Geometry.GetPathLengths(MVector(40, 0, 0), MVector(60, 0, 0));
  Passed = EvaluateNear("GetPathLengths()", "rotated box", "The path through the box rotated by 45 deg is 2 sqrt(2)", GetLength(Lengths, Aluminium), 2.0*sqrt(2.0), Tolerance) && Passed;

  // Rotated mother with a daughter: In the mother (half length 5) the local y axis is the z axis in the world
  // The egg has the half length 2 along the local y axis
  Lengths = Geometry.GetPathLengths(MVector(0, 0, -60), MVector(0, 0, -40));
  Passed = EvaluateNear("GetPathLengths()", "daughter in a rotated mother", "The path in the egg is 4 cm", GetLength(Lengths, Germanium), 4.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetPathLengths()", "daughter in a rotated mother", "The path in the mother is 10 - 4 cm", GetLength(Lengths, Aluminium), 6.0, Tolerance) && Passed;

  // The same stop inside of the daughter:
  Lengths = Geometry.GetPathLengths(MVector(0, 0, -60), MVector(0, 0, -50));
  Passed = EvaluateNear("GetPathLengths()", "stop in the daughter", "The path in the egg is 2 cm", GetLength(Lengths, Germanium), 2.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetPathLengths()", "stop in the daughter", "The path in the mother is 3 cm", GetLength(Lengths, Aluminium), 3.0, Tolerance) && Passed;

  // The real volume of a virtual volume: at (50, 53, 0), half length 1 in x (after the rotation by 90 deg)
  Lengths = Geometry.GetPathLengths(MVector(45, 53, 0), MVector(55, 53, 0));
  Passed = EvaluateNear("GetPathLengths()", "volume from a virtual volume", "The path in the member is 2 cm", GetLength(Lengths, Germanium), 2.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetPathLengths()", "volume from a virtual volume", "The rest is in the world, not in the virtual volume", GetLength(Lengths, Vacuum), 8.0, Tolerance) && Passed;

  // Start outside of the world, the part outside of the world does not count:
  Lengths = Geometry.GetPathLengths(MVector(-99, 99, 0), MVector(-99, 199, 0));
  Passed = EvaluateNear("GetPathLengths()", "start in the world, stop outside", "Only the part in the world is counted", GetLength(Lengths, Vacuum), 1.0, Tolerance) && Passed;
  Lengths = Geometry.GetPathLengths(MVector(-99, 199, 0), MVector(-99, 99, 0));
  Passed = EvaluateNear("GetPathLengths()", "start outside, stop in the world", "Only the part in the world is counted", GetLength(Lengths, Vacuum), 1.0, Tolerance) && Passed;

  // Start equals stop:
  Lengths = Geometry.GetPathLengths(MVector(0, 0, 10), MVector(0, 0, 10));
  Passed = EvaluateSize("GetPathLengths()", "start equals stop", "There is no path", Lengths.size(), 0) && Passed;

  // Silicon has no entry when missed:
  Lengths = Geometry.GetPathLengths(MVector(0, 0, 0), MVector(0, 0, 30));
  Passed = EvaluateTrue("GetPathLengths()", "missed", "A material which is not reached has no entry", Lengths.find(Silicon) == Lengths.end()) && Passed;

  // Round trip: The direction does not matter
  Lengths = Geometry.GetPathLengths(MVector(0, 0, 30), MVector(0, 0, 0));
  Passed = EvaluateNear("GetPathLengths()", "reversed", "The aluminium slab is 2 cm thick in both directions", GetLength(Lengths, Aluminium), 2.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetPathLengths()", "reversed", "The germanium slab is 3 cm thick in both directions", GetLength(Lengths, Germanium), 3.0, Tolerance) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the path lengths in shapes which a straight line enters more than once
bool UTDGeometryQuest::TestGetPathLengthsReentry()
{
  bool Passed = true;

  const double Tolerance = 1e-6;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "path geometry", "The geometry can be read", ScanGeometry(Geometry, GetPathGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  MDMaterial* Silicon = Geometry.GetMaterial("Silicon");

  // The tube has the center (0, 50, 0), the radii 3 and 5:
  // Through the center:
  map<MDMaterial*, double> Lengths = Geometry.GetPathLengths(MVector(-10, 50, 0), MVector(10, 50, 0));
  Passed = EvaluateNear("GetPathLengths()", "hollow tube, through the center", "Two walls of 2 cm", GetLength(Lengths, Silicon), 4.0, Tolerance) && Passed;

  // Through the hole, offset by 2: 2 sqrt(5^2 - 2^2) - 2 sqrt(3^2 - 2^2)
  Lengths = Geometry.GetPathLengths(MVector(-10, 52, 0), MVector(10, 52, 0));
  Passed = EvaluateNear("GetPathLengths()", "hollow tube, offset 2", "Two chords", GetLength(Lengths, Silicon), 2.0*sqrt(21.0) - 2.0*sqrt(5.0), Tolerance) && Passed;

  // Beside the hole, offset by 4: 2 sqrt(5^2 - 4^2)
  Lengths = Geometry.GetPathLengths(MVector(-10, 54, 0), MVector(10, 54, 0));
  Passed = EvaluateNear("GetPathLengths()", "hollow tube, offset 4", "One chord", GetLength(Lengths, Silicon), 6.0, Tolerance) && Passed;

  // Start in the hole:
  Lengths = Geometry.GetPathLengths(MVector(0, 50, 0), MVector(10, 50, 0));
  Passed = EvaluateNear("GetPathLengths()", "hollow tube, start in the hole", "One wall", GetLength(Lengths, Silicon), 2.0, Tolerance) && Passed;

  // Start in one wall, stop in the other one:
  Lengths = Geometry.GetPathLengths(MVector(-4, 50, 0), MVector(4, 50, 0));
  Passed = EvaluateNear("GetPathLengths()", "hollow tube, in the walls", "Half of both walls", GetLength(Lengths, Silicon), 2.0, Tolerance) && Passed;

  // Start in one wall, through the other one:
  Lengths = Geometry.GetPathLengths(MVector(-4, 50, 0), MVector(10, 50, 0));
  Passed = EvaluateNear("GetPathLengths()", "hollow tube, start in the wall", "Half of the first and the full second wall", GetLength(Lengths, Silicon), 3.0, Tolerance) && Passed;

  // Start outside, stop in the second wall:
  Lengths = Geometry.GetPathLengths(MVector(-10, 50, 0), MVector(4, 50, 0));
  Passed = EvaluateNear("GetPathLengths()", "hollow tube, stop in the wall", "The full first and half of the second wall", GetLength(Lengths, Silicon), 3.0, Tolerance) && Passed;

  // Stop in the hole:
  Lengths = Geometry.GetPathLengths(MVector(-10, 50, 0), MVector(0, 50, 0));
  Passed = EvaluateNear("GetPathLengths()", "hollow tube, stop in the hole", "One wall", GetLength(Lengths, Silicon), 2.0, Tolerance) && Passed;

  // The sphere has the center (0, -50, 0), the radii 2 and 5:
  Lengths = Geometry.GetPathLengths(MVector(-10, -50, 0), MVector(10, -50, 0));
  Passed = EvaluateNear("GetPathLengths()", "hollow sphere, through the center", "Two walls of 3 cm", GetLength(Lengths, Silicon), 6.0, Tolerance) && Passed;

  // Three disks of thickness 1 at z = 0 ... 1, 2 ... 3, and 4 ... 5, the line has distance 3 from the axis:
  // Defect GEO-B-36: GetAbsorptionLengths only handles two entries into a shape, the path in any further part of the shape is assigned to the mother
  Lengths = Geometry.GetPathLengths(MVector(-47, 0, -5), MVector(-47, 0, 10));
  Passed = ExpectedToFail(EvaluateNear("GetPathLengths()", "three disks, from the outside", "The path is three times the disk thickness", GetLength(Lengths, Silicon), 3.0, Tolerance)) && Passed;
  Lengths = Geometry.GetPathLengths(MVector(-47, 0, 0.5), MVector(-47, 0, 10));
  // Defect GEO-B-36: GetAbsorptionLengths only handles two entries into a shape, the path in any further part of the shape is assigned to the mother
  Passed = ExpectedToFail(EvaluateNear("GetPathLengths()", "three disks, from the inside", "The path is 2.5 times the disk thickness", GetLength(Lengths, Silicon), 2.5, Tolerance)) && Passed;

  // Only two disks are reached: This is handled
  Lengths = Geometry.GetPathLengths(MVector(-47, 0, -5), MVector(-47, 0, 3.5));
  Passed = EvaluateNear("GetPathLengths()", "two of three disks", "The path is two times the disk thickness", GetLength(Lengths, Silicon), 2.0, Tolerance) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the absorption probabilities against the analytic formula
bool UTDGeometryQuest::TestAbsorptionProbabilities()
{
  bool Passed = true;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "path geometry", "The geometry can be read", ScanGeometry(Geometry, GetPathGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  MDMaterial* Aluminium = Geometry.GetMaterial("Aluminium");
  MDMaterial* Germanium = Geometry.GetMaterial("Germanium");
  MDMaterial* Vacuum = Geometry.GetMaterial("Vacuum");

  // Write cross section tables with known values into the directory of the geometry file
  // The germanium table has a step above 1000 keV to see that the energy is used
  // Only multiples of powers of 2 are used, since the tables only have float precision
  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "cross sections", "The directory can be created", PrepareTemporaryDirectory("cross-sections")) && Passed;
  vector<MString> Types = { "Total", "Photo", "Compton", "Pair", "Rayleigh" };
  vector<MDMaterial*> Materials = { Aluminium, Germanium, Vacuum };
  vector<MString> Names = { "Aluminium", "Germanium", "Vacuum" };
  vector<double> Factors = { 1.0, 2.0, 0.0078125 };
  vector<double> TypeValues = { 0.5, 0.125, 0.25, 0.0625, 0.03125 };
  for (unsigned int m = 0; m < Materials.size(); ++m) {
    for (unsigned int t = 0; t < Types.size(); ++t) {
      double Value = Factors[m]*TypeValues[t];
      ostringstream Table;
      Table<<"Type ResponseMatrixO1"<<endl;
      Table<<"Version 1"<<endl;
      Table<<endl;
      Table<<"NM Test table"<<endl;
      Table<<"CE false"<<endl;
      Table<<"HA "<<Materials[m]->GetHash()<<endl;
      Table<<endl;
      Table<<"R1 1.0 "<<Value<<endl;
      Table<<"R1 1000.0 "<<Value<<endl;
      if (m == 1) {
        Table<<"R1 1001.0 "<<2.0*Value<<endl;
        Table<<"R1 100000000.0 "<<2.0*Value<<endl;
      } else {
        Table<<"R1 100000000.0 "<<Value<<endl;
      }
      Table<<endl;
      MString FileName = GetTemporaryDirectoryName("cross-sections") + "/Xsection." + Types[t] + "." + Names[m] + ".rsp";
      Passed = EvaluateTrue("WriteTextFile()", "cross sections", "The table can be written", WriteTextFile(FileName, Table.str().c_str())) && Passed;
    }
    Passed = EvaluateTrue("LoadCrossSections()", Names[m], "The table can be read", Materials[m]->LoadCrossSections()) && Passed;
    Passed = EvaluateTrue("AreCrossSectionsPresent()", Names[m], "The tables are accepted", Materials[m]->AreCrossSectionsPresent()) && Passed;
  }

  // Beer-Lambert with the lengths of the fixture: 1 - exp(-sum(mu * length))
  // Lengths of 2, 3, and 25 cm, in units of the type value: 2 * 1 + 3 * 2 + 25 * 0.0078125
  // The lengths are known to about 1e-6 cm, and the sum of the coefficients is below 1
  const double Tolerance = 1e-6;
  const MVector Start(0, 0, 0);
  const MVector Stop(0, 0, 30);
  const double Energy = 500.0;
  const double Sum = 2.0*1.0 + 3.0*2.0 + 25.0*0.0078125;

  Passed = EvaluateNear("GetAbsorptionProbability()", "through two slabs", "The result is Beer-Lambert with the total coefficient", Geometry.GetAbsorptionProbability(Start, Stop, Energy), 1.0 - exp(-Sum*0.5), Tolerance) && Passed;
  Passed = EvaluateNear("GetPhotoAbsorptionProbability()", "through two slabs", "The result is Beer-Lambert with the photo coefficient", Geometry.GetPhotoAbsorptionProbability(Start, Stop, Energy), 1.0 - exp(-Sum*0.125), Tolerance) && Passed;
  Passed = EvaluateNear("GetComptonAbsorptionProbability()", "through two slabs", "The result is Beer-Lambert with the Compton coefficient", Geometry.GetComptonAbsorptionProbability(Start, Stop, Energy), 1.0 - exp(-Sum*0.25), Tolerance) && Passed;
  Passed = EvaluateNear("GetPairAbsorptionProbability()", "through two slabs", "The result is Beer-Lambert with the pair coefficient", Geometry.GetPairAbsorptionProbability(Start, Stop, Energy), 1.0 - exp(-Sum*0.0625), Tolerance) && Passed;
  Passed = EvaluateNear("GetRayleighAbsorptionProbability()", "through two slabs", "The result is Beer-Lambert with the Rayleigh coefficient", Geometry.GetRayleighAbsorptionProbability(Start, Stop, Energy), 1.0 - exp(-Sum*0.03125), Tolerance) && Passed;

  // The energy: Above 1000 keV the germanium coefficient is two times larger:
  const double HighSum = 2.0*1.0 + 3.0*4.0 + 25.0*0.0078125;
  Passed = EvaluateNear("GetAbsorptionProbability()", "2000 keV", "The coefficient for the energy of the photon is used", Geometry.GetAbsorptionProbability(Start, Stop, 2000.0), 1.0 - exp(-HighSum*0.5), Tolerance) && Passed;

  // Only in the slab:
  Passed = EvaluateNear("GetAbsorptionProbability()", "1 cm in the slab", "Only the slab is used", Geometry.GetAbsorptionProbability(MVector(0, 0, 9.5), MVector(0, 0, 10.5), Energy), 1.0 - exp(-0.5), Tolerance) && Passed;

  // Zero length:
  Passed = EvaluateNear("GetAbsorptionProbability()", "start equals stop", "Without a path there is no absorption", Geometry.GetAbsorptionProbability(MVector(0, 0, 10), MVector(0, 0, 10), Energy), 0.0, 1e-15) && Passed;

  // Direction does not matter:
  Passed = EvaluateNear("GetAbsorptionProbability()", "reversed", "The direction does not matter", Geometry.GetAbsorptionProbability(Stop, Start, Energy), 1.0 - exp(-Sum*0.5), Tolerance) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the Compton cone intersections
bool UTDGeometryQuest::TestGetComptonIntersection()
{
  bool Passed = true;

  // Ee = 200 keV, Eg = 300 keV: cos(phi) = 1 - E0*(1/Eg - 1/(Ee + Eg))
  const double Ee = 200.0;
  const double Eg = 300.0;
  const double ExpectedPhi = acos(1.0 - c_E0*(1.0/Eg - 1.0/(Ee + Eg)));

  // A photon from above: Dg = (0, 0, -1), and the cone around +z
  MComptonEvent Down;
  Passed = EvaluateTrue("Assimilate()", "from above", "The event is valid", Down.Assimilate(MVector(0, 0, 0), MVector(0, 0, -5), MVector(0, 0, 0), Ee, Eg)) && Passed;
  Passed = EvaluateNear("Phi()", "from above", "The Compton angle has the analytic value", Down.Phi(), ExpectedPhi, 1e-12) && Passed;

  Passed = EvaluateTrue("GetComptonIntersection()", "cone along z, box on the cone", "A box on the cone is found", IntersectsBox(Down, MVector(40*sin(ExpectedPhi), 0, 40*cos(ExpectedPhi)), 1.5)) && Passed;
  Passed = EvaluateTrue("GetComptonIntersection()", "cone along z, box on the cone at 200 deg", "A box on the cone at another azimuth is found", IntersectsBox(Down, MVector(40*sin(ExpectedPhi)*cos(200*c_Rad), 40*sin(ExpectedPhi)*sin(200*c_Rad), 40*cos(ExpectedPhi)), 1.5)) && Passed;
  Passed = EvaluateFalse("GetComptonIntersection()", "cone along z, box on the axis", "A box on the axis of the cone is not found", IntersectsBox(Down, MVector(0, 0, 40), 1.5)) && Passed;
  Passed = EvaluateFalse("GetComptonIntersection()", "cone along z, box below", "A box in the other half is not found", IntersectsBox(Down, MVector(40*sin(ExpectedPhi), 0, -40*cos(ExpectedPhi)), 1.5)) && Passed;

  // A photon along an axis perpendicular to z:
  MComptonEvent Side;
  Side.Assimilate(MVector(0, 0, 0), MVector(5, 0, 0), MVector(0, 0, 0), Ee, Eg);
  Passed = EvaluateTrue("GetComptonIntersection()", "cone along x, box on the cone", "A box on the cone is found", IntersectsBox(Side, GetPositionOnCone(Side, 1.0), 1.5)) && Passed;
  Passed = EvaluateFalse("GetComptonIntersection()", "cone along x, box on the axis", "A box on the axis of the cone is not found", IntersectsBox(Side, MVector(-40, 0, 0), 1.5)) && Passed;

  // A wide angle of more than 90 deg: Ee = 150, Eg = 200: cos(phi) = 1 - 510.999*(1/200 - 1/350) = -0.095
  MComptonEvent Wide;
  Wide.Assimilate(MVector(0, 0, 0), MVector(0, 0, -5), MVector(0, 0, 0), 150.0, 200.0);
  Passed = EvaluateTrue("Phi()", "wide angle", "The Compton angle is larger than 90 deg", Wide.Phi() > c_Pi/2) && Passed;
  Passed = EvaluateTrue("GetComptonIntersection()", "angle above 90 deg, box on the cone", "A box on the cone is found", IntersectsBox(Wide, GetPositionOnCone(Wide, 2.0), 1.5)) && Passed;

  // Interaction point not at the origin: The box and the cone move with it
  MComptonEvent Shifted;
  Shifted.Assimilate(MVector(10, -20, 5), MVector(10, -20, 0), MVector(0, 0, 0), Ee, Eg);
  Passed = EvaluateTrue("GetComptonIntersection()", "shifted, box on the cone", "A box on the cone is found", IntersectsBox(Shifted, GetPositionOnCone(Shifted, 4.0), 1.5)) && Passed;
  Passed = EvaluateFalse("GetComptonIntersection()", "shifted, box on the axis", "A box on the axis of the cone is not found", IntersectsBox(Shifted, MVector(10, -20, 45), 1.5)) && Passed;

  // A direction which is no axis of the coordinate system:
  MComptonEvent Diagonal;
  Diagonal.Assimilate(MVector(0, 0, 0), MVector(5, 5, 5), MVector(0, 0, 0), Ee, Eg);
  bool AllFound = true;
  for (unsigned int a = 0; a < 8; ++a) {
    if (IntersectsBox(Diagonal, GetPositionOnCone(Diagonal, a*45.0*c_Rad), 1.5) == false) {
      AllFound = false;
    }
  }
  // Defect GEO-B-37: GetComptonIntersection uses a matrix which is no rotation in general, thus the sampled cone is wrong for a direction which is not along an axis
  Passed = ExpectedToFail(EvaluateTrue("GetComptonIntersection()", "diagonal direction, boxes on the cone", "Boxes on the cone at eight azimuth angles are found", AllFound)) && Passed;

  // A direction for which the matrix is singular:
  MComptonEvent Singular;
  Singular.Assimilate(MVector(0, 0, 0), MVector(5, -5, 0), MVector(0, 0, 0), Ee, Eg);
  bool SingularFound = IntersectsBox(Singular, GetPositionOnCone(Singular, 1.0), 1.5);
  // Defect GEO-B-37: GetComptonIntersection uses a matrix which is no rotation in general, thus the sampled cone is wrong for a direction which is not along an axis
  Passed = ExpectedToFail(EvaluateTrue("GetComptonIntersection()", "direction (1, -1, 0), box on the cone", "The box on the cone is found also for this direction", SingularFound)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the overlap check
bool UTDGeometryQuest::TestCheckOverlaps()
{
  bool Passed = true;

  MString Good = GetGeometryHead();
  Good += "Volume A\n";
  Good += "A.Material Aluminium\n";
  Good += "A.Shape BRIK 1.0 1.0 1.0\n";
  Good += "A.Position 0.0 0.0 0.0\n";
  Good += "A.Mother World\n";
  Good += "Volume B\n";
  Good += "B.Material Aluminium\n";
  Good += "B.Shape BRIK 1.0 1.0 1.0\n";
  Good += "B.Position 3.0 0.0 0.0\n";
  Good += "B.Mother World\n";

  MDGeometryQuest GoodGeometry;
  Passed = EvaluateTrue("ScanSetupFile()", "no overlap", "The geometry can be read", ScanGeometry(GoodGeometry, Good)) && Passed;
  if (GoodGeometry.IsScanned() == true) {
    ostringstream Diagnostics;
    DisableDefaultStreams();
    bool Result = GoodGeometry.CheckOverlaps(Diagnostics);
    EnableDefaultStreams();
    Passed = EvaluateTrue("CheckOverlaps()", "no overlap", "A geometry without overlaps passes", Result) && Passed;
  }

  // The rotated box is near A: Without the rotation there would be no overlap
  MString Bad = GetGeometryHead();
  Bad += "Volume A\n";
  Bad += "A.Material Aluminium\n";
  Bad += "A.Shape BRIK 1.0 1.0 1.0\n";
  Bad += "A.Position 0.0 0.0 0.0\n";
  Bad += "A.Mother World\n";
  Bad += "Volume B\n";
  Bad += "B.Material Aluminium\n";
  Bad += "B.Shape BRIK 3.0 0.5 0.5\n";
  Bad += "B.Position 0.0 3.0 0.0\n";
  Bad += "B.Rotation 0.0 0.0 90.0\n";
  Bad += "B.Mother World\n";

  MDGeometryQuest BadGeometry;
  Passed = EvaluateTrue("ScanSetupFile()", "overlap", "The geometry can be read", ScanGeometry(BadGeometry, Bad, "Bad.geo.setup")) && Passed;
  if (BadGeometry.IsScanned() == true) {
    ostringstream Diagnostics;
    DisableDefaultStreams();
    bool Result = BadGeometry.CheckOverlaps(Diagnostics);
    EnableDefaultStreams();
    Passed = EvaluateFalse("CheckOverlaps()", "overlap", "An overlap of a rotated volume is found", Result) && Passed;
    Passed = EvaluateTrue("CheckOverlaps()", "overlap", "The overlap is reported", MString(Diagnostics.str().c_str()).Contains("Overlap")) && Passed;

    // The search at one position finds both:
    vector<MDVolume*> Overlaps;
    BadGeometry.GetWorldVolume()->FindOverlaps(MVector(0, 0.5, 0), Overlaps);
    Passed = EvaluateSize("FindOverlaps()", "in the overlap", "Both volumes are found", Overlaps.size(), 2) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the virtualization of volumes without a detector
bool UTDGeometryQuest::TestVirtualization()
{
  bool Passed = true;

  MString FileName = GetTemporaryFileName("Virtualization.geo.setup");
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The fixture can be written", WriteTextFile(FileName, GetDetectorGeometry())) && Passed;

  MDGeometryQuest Geometry;
  DisableDefaultStreams();
  bool Read = Geometry.ScanSetupFile(FileName, false, true, false);
  EnableDefaultStreams();
  Passed = EvaluateTrue("ScanSetupFile()", "virtualize", "The geometry can be read with virtualization", Read) && Passed;
  if (Read == false) {
    return Passed;
  }

  // The passive volume has gone, thus the world is there, the detector is still at its place
  Passed = EvaluateTrue("GetVolume()", "virtualized passive volume", "The world is found at the position of the passive volume", Geometry.GetVolume(MVector(0, 50, 0)) == Geometry.GetWorldVolume()) && Passed;
  Passed = EvaluateTrue("GetDetectorName()", "virtualization", "The detector is still found", Geometry.GetDetectorName(MVector(-20, 0, 2)) == "Wafer") && Passed;

  // Position of the detector in the rotated holder in the world after the virtualization:
  MDVolumeSequence VS = Geometry.GetVolumeSequence(MVector(18, 1, 2.5), false, false);
  Passed = EvaluateTrue("GetVolumeSequence()", "virtualization", "The sensitive volume is found", VS.GetSensitiveVolume() != nullptr) && Passed;
  Passed = EvaluateVectorNear("GetVolumeSequence()", "virtualization", "The position in the sensitive volume stays the one derived by hand", VS.GetPositionInSensitiveVolume(), MVector(1, 2, 0.5), 1e-13) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the gridding of a hit
bool UTDGeometryQuest::TestGrid()
{
  bool Passed = true;

  MDGeometryQuest Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "detector geometry", "The geometry can be read", ScanGeometry(Geometry, GetDetectorGeometry())) && Passed;
  if (Geometry.IsScanned() == false) {
    return Passed;
  }

  // The wafer in holder A is rotated by 90 deg around z: The point (1, 2, 2.5) in the holder is at (-2, 1, 2.5) + (20, 0, 0)
  // In the wafer this is (1, 2, 0.5): strip 6 of pitch 1 cm in x with its center at 1.5 cm, and strip 3 of pitch 2 cm in y with its center at 2 cm
  const MVector InWafer(18, 1, 2.5);
  MVector Position = InWafer;
  double Energy = 100.0;
  double Time = 1.0;
  MDVolumeSequence VS = Geometry.GetVolumeSequence(InWafer, false, false);

  BeginCaptureOutput();
  DisableDefaultStreams();
  MDGridPointCollection Collection = Geometry.Grid(Position, Energy, Time, VS);
  EnableDefaultStreams();
  MString Output = EndCaptureOutput();

  Passed = EvaluateVectorNear("Grid()", "in the wafer", "The position is restored", Position, InWafer, 1e-13) && Passed;
  Passed = EvaluateNear("Grid()", "in the wafer", "The energy is not changed", Energy, 100.0, 1e-15) && Passed;
  Passed = EvaluateNear("Grid()", "in the wafer", "The time is not changed", Time, 1.0, 1e-15) && Passed;
  Passed = EvaluateTrue("Grid()", "in the wafer", "The collection has the detector of the hit", Collection.GetVolumeSequence().GetDetector() == Geometry.GetDetector("Det")) && Passed;
  if (Evaluate("Grid()", "in the wafer", "The collection has one grid point", Collection.GetNGridPoints(), 1U) == true) {
    MDGridPoint Point = Collection.GetGridPointAt(0);
    Passed = Evaluate("Grid()", "in the wafer", "The hit is in x strip 6", Point.GetXGrid(), 6U) && Passed;
    Passed = Evaluate("Grid()", "in the wafer", "The hit is in y strip 3", Point.GetYGrid(), 3U) && Passed;
    Passed = EvaluateNear("Grid()", "in the wafer", "The grid point has the energy", Point.GetEnergy(), 100.0, 1e-12) && Passed;
    Passed = EvaluateNear("Grid()", "in the wafer", "The grid point has the time", Point.GetTime(), 1.0, 1e-12) && Passed;
    // The center of the strip crossing (1.5, 2, 0) in the wafer is (1.5, 2, 2) in the holder and (18, 1.5, 2) in the world
    Passed = EvaluateVectorNear("Grid()", "in the wafer", "The world position of the grid point is the center of the strip crossing", Collection.GetWorldPositionGridPointAt(0), MVector(18.0, 1.5, 2.0), 1e-12) && Passed;
  } else {
    Passed = false;
  }
  // Defect GEO-B-43: Grid() prints the number of grid points to the standard output
  Passed = ExpectedToFail(EvaluateTrue("Grid()", "in the wafer", "Nothing is written to the standard output", Output.IsEmpty())) && Passed;

  // The other placement of the wafer is not rotated: (-19, 2, 2.5) is (1, 2, 0.5) in the wafer too
  const MVector InWaferB(-19, 2, 2.5);
  Position = InWaferB;
  MDVolumeSequence VSB = Geometry.GetVolumeSequence(InWaferB, false, false);
  BeginCaptureOutput();
  DisableDefaultStreams();
  MDGridPointCollection CollectionB = Geometry.Grid(Position, Energy, Time, VSB);
  EnableDefaultStreams();
  EndCaptureOutput();
  Passed = EvaluateVectorNear("Grid()", "in the second wafer", "The position is restored", Position, InWaferB, 1e-13) && Passed;
  if (Evaluate("Grid()", "in the second wafer", "The collection has one grid point", CollectionB.GetNGridPoints(), 1U) == true) {
    Passed = Evaluate("Grid()", "in the second wafer", "The hit is in x strip 6", CollectionB.GetGridPointAt(0).GetXGrid(), 6U) && Passed;
    Passed = Evaluate("Grid()", "in the second wafer", "The hit is in y strip 3", CollectionB.GetGridPointAt(0).GetYGrid(), 3U) && Passed;
    Passed = EvaluateVectorNear("Grid()", "in the second wafer", "The world position of the grid point is the center of the strip crossing", CollectionB.GetWorldPositionGridPointAt(0), MVector(-18.5, 2.0, 2.0), 1e-12) && Passed;
  } else {
    Passed = false;
  }
  Passed = EvaluateFalse("HasSameDetector()", "two wafers", "The collections of the two wafers are in different detectors", Collection.HasSameDetector(CollectionB)) && Passed;

  // No detector:
  MVector PassivePosition(0, 50, 0);
  double PassiveEnergy = 100.0;
  double PassiveTime = 1.0;
  MDVolumeSequence PassiveVS = Geometry.GetVolumeSequence(PassivePosition, false, false);
  Passed = EvaluateTrue("GetVolumeSequence()", "passive material", "The position has no detector", PassiveVS.GetDetector() == nullptr) && Passed;
  // Defect GEO-B-44: Without a detector Grid() returns a collection made from the volume sequence, whose constructor asserts that there is a detector
  Passed = ExpectedToCrash(EvaluateTrue("Grid()", "passive material", "A hit without detector gives an empty collection", Geometry.Grid(PassivePosition, PassiveEnergy, PassiveTime, PassiveVS).GetNGridPoints() == 0)) && Passed;

  // Not scanned:
  MDGeometryQuest NotScanned;
  MDVolumeSequence EmptyVS;
  // Defect GEO-B-44: The same for a geometry which is not scanned
  Passed = ExpectedToCrash(EvaluateTrue("Grid()", "not scanned", "A geometry which is not scanned gives an empty collection", NotScanned.Grid(PassivePosition, PassiveEnergy, PassiveTime, EmptyVS).GetNGridPoints() == 0)) && Passed;

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

  UTDGeometryQuest Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDGeometryQuest.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
