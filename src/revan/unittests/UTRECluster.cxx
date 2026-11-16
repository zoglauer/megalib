/*
 * UTRECluster.cxx
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
#include "MDVolumeSequence.h"
#include "MGeometryRevan.h"
#include "MRECluster.h"
#include "MREHit.h"
#include "MRESE.h"
#include "MRETrack.h"
#include "UTRevanShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the cluster class MRECluster
class UTRECluster : public UTRevanBase
{
public:
  //! Default constructor
  UTRECluster() : UTRevanBase("UTRECluster") {}
  //! Default destructor
  virtual ~UTRECluster() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Add the element to the cluster and return what the cluster printed to the standard output
  MString AddQuietly(MRECluster& Cluster, MRESE* RESE);
  //! Return true if a cluster of one hit can be added to a cluster of two hits
  bool AddSmallerCluster();
  //! Return true if adding a track to a cluster is survived and adds nothing
  bool AddTrack();
  //! Return true if removing a hit which is not in the cluster is survived and returns a nullptr
  bool RemoveUnknownHit();
  //! Return true if a hit can be added after another one was removed
  bool AddAfterRemove();
  //! Return true if the text of a cluster with a removed hit can be created
  bool ToStringAfterRemove();

  //! Test the state of a new cluster
  bool TestDefaultConstruction();
  //! Test adding hits: energy, time, detector type, and resolutions
  bool TestAddHits();
  //! Test the center of energy of the hits
  bool TestCenterOfEnergy();
  //! Test the handling of hits without depth
  bool TestMissingDepth();
  //! Test adding clusters and tracks
  bool TestAddClustersAndTracks();
  //! Test adding hits to a cluster which may not be overwritten
  bool TestAllowOverwrite();
  //! Test removing hits
  bool TestRemoveRESE();
  //! Test the distances to hits, clusters, and tracks
  bool TestDistances();
  //! Test the adjacency check
  bool TestAreAdjacent();
  //! Test the copy constructor and the duplication
  bool TestCopyAndDuplicate();
  //! Test the validity
  bool TestIsValid();
  //! Test the text output
  bool TestToString();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTRECluster::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestAddHits() && Passed;
  Passed = TestCenterOfEnergy() && Passed;
  Passed = TestMissingDepth() && Passed;
  Passed = TestAddClustersAndTracks() && Passed;
  Passed = TestAllowOverwrite() && Passed;
  Passed = TestRemoveRESE() && Passed;
  Passed = TestDistances() && Passed;
  Passed = TestAreAdjacent() && Passed;
  Passed = TestCopyAndDuplicate() && Passed;
  Passed = TestIsValid() && Passed;
  Passed = TestToString() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Add the element to the cluster and return what the cluster printed to the standard output
MString UTRECluster::AddQuietly(MRECluster& Cluster, MRESE* RESE)
{
  BeginCaptureOutput();
  Cluster.AddRESE(RESE);
  return EndCaptureOutput();
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a cluster of one hit can be added to a cluster of two hits
bool UTRECluster::AddSmallerCluster()
{
  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  MREHit* B = CreateHit(MVector(1.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  MREHit* C = CreateHit(MVector(2.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);

  MRECluster Large;
  AddQuietly(Large, A);
  AddQuietly(Large, B);
  MRECluster Small;
  AddQuietly(Small, C);

  AddQuietly(Large, &Small);
  bool Added = false;
  if (Large.GetNRESEs() == 3 && Large.ContainsRESE(C) == true) {
    Added = true;
  }

  delete A;
  delete B;
  delete C;

  return Added;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if adding a track to a cluster is survived and adds nothing
bool UTRECluster::AddTrack()
{
  MRECluster Cluster;
  MRETrack Track;
  DisableDefaultStreams();
  Cluster.AddRESE(&Track);
  EnableDefaultStreams();

  if (Cluster.GetNRESEs() != 0) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if removing a hit which is not in the cluster is survived and returns a nullptr
bool UTRECluster::RemoveUnknownHit()
{
  MRECluster Cluster;
  MREHit Hit;
  DisableDefaultStreams();
  MRESE* Removed = Cluster.RemoveRESE(&Hit);
  EnableDefaultStreams();

  if (Removed != nullptr) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if a hit can be added after another one was removed
bool UTRECluster::AddAfterRemove()
{
  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  MREHit* B = CreateHit(MVector(1.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  MREHit* C = CreateHit(MVector(2.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);

  MRECluster Cluster;
  AddQuietly(Cluster, A);
  AddQuietly(Cluster, B);
  Cluster.RemoveRESE(A);
  AddQuietly(Cluster, C);
  bool Added = Cluster.ContainsRESE(C);

  delete A;
  delete B;
  delete C;

  return Added;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the text of a cluster with a removed hit can be created
bool UTRECluster::ToStringAfterRemove()
{
  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  MREHit* B = CreateHit(MVector(1.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);

  MRECluster Cluster;
  AddQuietly(Cluster, A);
  AddQuietly(Cluster, B);
  Cluster.RemoveRESE(A);
  bool HasText = false;
  if (Cluster.ToString().IsEmpty() == false && Cluster.IsValid() == true) {
    HasText = true;
  }

  delete A;
  delete B;

  return HasText;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the state of a new cluster
bool UTRECluster::TestDefaultConstruction()
{
  bool Passed = true;

  MRECluster Cluster;
  Passed = Evaluate("MRECluster()", "default", "The type is a cluster", Cluster.GetType(), MRESE::c_Cluster) && Passed;
  Passed = EvaluateVectorNear("MRECluster()", "default", "The position is the origin", Cluster.GetPosition(), MVector(0, 0, 0), 0.0) && Passed;
  Passed = EvaluateNear("MRECluster()", "default", "The energy is zero", Cluster.GetEnergy(), 0.0, 0.0) && Passed;
  // -1 marks a cluster which has no hit yet
  Passed = Evaluate("MRECluster()", "default", "The detector type is not adjusted", Cluster.GetDetector(), -1) && Passed;
  Passed = Evaluate("MRECluster()", "default", "There are no hits", Cluster.GetNRESEs(), 0) && Passed;
  Passed = EvaluateTrue("MRECluster()", "default", "The empty cluster is valid", Cluster.IsValid()) && Passed;
  Passed = EvaluateTrue("MRECluster()", "default", "The noise flags are empty", Cluster.GetNoiseFlags().IsEmpty()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding hits: energy, time, detector type, and resolutions
bool UTRECluster::TestAddHits()
{
  bool Passed = true;

  MREHit* A = CreateHit(MVector(1.0, 2.0, 3.0), 100.0, MDDetector::c_Strip2D, nullptr, 5.0);
  A->SetEnergyResolution(3.0);
  A->SetTimeResolution(0.1);
  A->AddOriginID(1);
  MREHit* B = CreateHit(MVector(2.0, 2.0, 3.0), 300.0, MDDetector::c_Strip2D, nullptr, 4.0);
  B->SetEnergyResolution(4.0);
  B->SetTimeResolution(0.2);
  B->AddOriginID(2);
  MREHit* C = CreateHit(MVector(2.0, 3.0, 3.0), 50.0, MDDetector::c_Calorimeter, nullptr, 6.0);
  C->SetEnergyResolution(12.0);
  C->SetTimeResolution(0.3);
  C->AddOriginID(2);

  MRECluster Cluster;
  MString Output = AddQuietly(Cluster, A);
  Passed = Evaluate("AddRESE()", "first hit", "There is one hit", Cluster.GetNRESEs(), 1) && Passed;
  Passed = EvaluateTrue("AddRESE()", "first hit", "The cluster contains the hit", Cluster.ContainsRESE(A)) && Passed;
  Passed = EvaluateVectorNear("AddRESE()", "first hit", "The position is the one of the hit", Cluster.GetPosition(), MVector(1.0, 2.0, 3.0), 0.0) && Passed;
  Passed = EvaluateNear("AddRESE()", "first hit", "The energy is the one of the hit", Cluster.GetEnergy(), 100.0, 0.0) && Passed;
  Passed = EvaluateNear("AddRESE()", "first hit", "The time is the one of the hit", Cluster.GetTime(), 5.0, 0.0) && Passed;
  Passed = Evaluate("AddRESE()", "first hit", "The detector type is the one of the hit", Cluster.GetDetector(), MDDetector::c_Strip2D) && Passed;
  Passed = EvaluateNear("AddRESE()", "first hit", "The energy resolution is the one of the hit", Cluster.GetEnergyResolution(), 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("AddRESE()", "first hit", "The time resolution is the one of the hit", Cluster.GetTimeResolution(), 0.1, 0.0) && Passed;
  Passed = EvaluateTrue("AddRESE()", "first hit", "Nothing is printed to the standard output", Output.IsEmpty()) && Passed;

  Output = AddQuietly(Cluster, B);
  Passed = Evaluate("AddRESE()", "second hit", "There are two hits", Cluster.GetNRESEs(), 2) && Passed;
  Passed = EvaluateNear("AddRESE()", "second hit", "The energy is the sum, 400 keV", Cluster.GetEnergy(), 400.0, 1e-12) && Passed;
  Passed = EvaluateNear("AddRESE()", "second hit", "The time is the earliest one, 4 s", Cluster.GetTime(), 4.0, 0.0) && Passed;
  Passed = Evaluate("AddRESE()", "second hit", "The detector type is still the one of the hits", Cluster.GetDetector(), MDDetector::c_Strip2D) && Passed;
  // The energy resolutions add in quadrature: sqrt(3^2 + 4^2) = 5
  Passed = EvaluateNear("AddRESE()", "second hit", "The energy resolution is the quadratic sum, 5 keV", Cluster.GetEnergyResolution(), 5.0, 1e-12) && Passed;
  Passed = EvaluateNear("AddRESE()", "second hit", "The time resolution is the one of the earliest hit", Cluster.GetTimeResolution(), 0.2, 0.0) && Passed;
  Passed = EvaluateSize("AddRESE()", "second hit", "The origin IDs of both hits are collected", Cluster.GetOriginIDs().size(), 2) && Passed;
  // Defect REV-B-7: AddHit prints debug lines ("NF: ...", "Ignore new depth", "Ignore old depth") to the standard output for each hit after the first one
  Passed = ExpectedToFail(EvaluateTrue("AddRESE()", "second hit", "Nothing is printed to the standard output", Output.IsEmpty())) && Passed;

  AddQuietly(Cluster, C);
  Passed = Evaluate("AddRESE()", "third hit, other detector type", "There are three hits", Cluster.GetNRESEs(), 3) && Passed;
  Passed = EvaluateNear("AddRESE()", "third hit, other detector type", "The energy is the sum, 450 keV", Cluster.GetEnergy(), 450.0, 1e-12) && Passed;
  Passed = EvaluateNear("AddRESE()", "third hit, other detector type", "The time is still the earliest one, 4 s", Cluster.GetTime(), 4.0, 0.0) && Passed;
  // 0 marks hits in several detector types
  Passed = Evaluate("AddRESE()", "third hit, other detector type", "The detector type is zero for mixed detector types", Cluster.GetDetector(), 0) && Passed;
  // sqrt(3^2 + 4^2 + 12^2) = 13
  Passed = EvaluateNear("AddRESE()", "third hit, other detector type", "The energy resolution is the quadratic sum, 13 keV", Cluster.GetEnergyResolution(), 13.0, 1e-12) && Passed;
  Passed = EvaluateSize("AddRESE()", "third hit, other detector type", "An origin ID in two hits is stored once", Cluster.GetOriginIDs().size(), 2) && Passed;

  // The destructor of the cluster does not delete the hits
  delete A;
  delete B;
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the center of energy of the hits
bool UTRECluster::TestCenterOfEnergy()
{
  bool Passed = true;

  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  MREHit* B = CreateHit(MVector(4.0, 8.0, -2.0), 300.0, MDDetector::c_Strip2D);
  MREHit* C = CreateHit(MVector(-6.0, 1.0, 5.0), 400.0, MDDetector::c_Strip2D);

  MRECluster Cluster;
  AddQuietly(Cluster, A);
  AddQuietly(Cluster, B);
  // (100*(0, 0, 0) + 300*(4, 8, -2))/400 = (3, 6, -1.5)
  // Defect REV-B-6: AddHit adds the energy of the new hit to the cluster energy before it weights the positions, so the old position gets the weight of both and the cluster is not at the center of energy
  Passed = ExpectedToFail(EvaluateVectorNear("AddRESE()", "100 keV at (0, 0, 0) and 300 keV at (4, 8, -2)", "The position is the center of energy (3, 6, -1.5)", Cluster.GetPosition(), MVector(3.0, 6.0, -1.5), 1e-12)) && Passed;

  AddQuietly(Cluster, C);
  // (100*(0, 0, 0) + 300*(4, 8, -2) + 400*(-6, 1, 5))/800 = (-1.5, 3.5, 1.75)
  Passed = ExpectedToFail(EvaluateVectorNear("AddRESE()", "plus 400 keV at (-6, 1, 5)", "The position is the center of energy (-1.5, 3.5, 1.75)", Cluster.GetPosition(), MVector(-1.5, 3.5, 1.75), 1e-12)) && Passed;
  Passed = EvaluateNear("AddRESE()", "three hits", "The energy is 800 keV", Cluster.GetEnergy(), 800.0, 1e-12) && Passed;

  // Two hits of the same energy: the center is the mid point, independent of the order
  MREHit* D = CreateHit(MVector(1.0, 1.0, 1.0), 250.0, MDDetector::c_Strip2D);
  MREHit* E = CreateHit(MVector(2.0, 3.0, 5.0), 250.0, MDDetector::c_Strip2D);
  MRECluster Forward;
  AddQuietly(Forward, D);
  AddQuietly(Forward, E);
  MRECluster Backward;
  AddQuietly(Backward, E);
  AddQuietly(Backward, D);
  Passed = ExpectedToFail(EvaluateVectorNear("AddRESE()", "two hits of 250 keV", "The position is the mid point (1.5, 2, 3)", Forward.GetPosition(), MVector(1.5, 2.0, 3.0), 1e-12)) && Passed;
  Passed = ExpectedToFail(EvaluateVectorNear("AddRESE()", "two hits of 250 keV, reversed order", "The position does not depend on the order of the hits", Backward.GetPosition(), Forward.GetPosition(), 1e-12)) && Passed;

  delete A;
  delete B;
  delete C;
  delete D;
  delete E;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the handling of hits without depth
bool UTRECluster::TestMissingDepth()
{
  bool Passed = true;

  // A hit without depth sits at the center of its wafer, here z = 0; the hits with depth are at z = 0.5 and z = 0.9
  // Hits of the same energy: the center in x and y is the mid point

  // A first hit without depth flags the cluster:
  {
    MREHit* NoDepth = CreateHit(MVector(30.0, 0.0, 0.0), 200.0, MDDetector::c_Strip3D);
    NoDepth->SetNoiseFlags(" NODEPTH");
    MRECluster Cluster;
    AddQuietly(Cluster, NoDepth);
    Passed = Evaluate("AddRESE()", "first hit without depth", "The cluster is flagged as without depth", Cluster.GetNoiseFlags(), MString(" NODEPTH")) && Passed;
    delete NoDepth;
  }

  // A cluster with depth plus a hit without depth: the depth of the cluster is kept
  {
    MREHit* Depth = CreateHit(MVector(30.0, 0.0, 0.5), 200.0, MDDetector::c_Strip3D);
    MREHit* NoDepth = CreateHit(MVector(31.0, 2.0, 0.0), 200.0, MDDetector::c_Strip3D);
    NoDepth->SetNoiseFlags(" NODEPTH");
    MRECluster Cluster;
    AddQuietly(Cluster, Depth);
    AddQuietly(Cluster, NoDepth);
    Passed = EvaluateNear("AddRESE()", "depth, then no depth", "The depth of the cluster stays 0.5 cm", Cluster.GetPosition().Z(), 0.5, 1e-12) && Passed;
    Passed = EvaluateFalse("AddRESE()", "depth, then no depth", "The cluster is not flagged as without depth", Cluster.GetNoiseFlags().Contains("NODEPTH")) && Passed;
    // Defect REV-B-6, see TestCenterOfEnergy
    Passed = ExpectedToFail(EvaluateNear("AddRESE()", "depth, then no depth", "x is the mid point 30.5 cm", Cluster.GetPosition().X(), 30.5, 1e-12)) && Passed;
    Passed = ExpectedToFail(EvaluateNear("AddRESE()", "depth, then no depth", "y is the mid point 1 cm", Cluster.GetPosition().Y(), 1.0, 1e-12)) && Passed;
    delete Depth;
    delete NoDepth;
  }

  // A cluster without depth plus a hit with depth: the depth of the hit is taken and the flag is removed
  {
    MREHit* NoDepth = CreateHit(MVector(31.0, 2.0, 0.0), 200.0, MDDetector::c_Strip3D);
    NoDepth->SetNoiseFlags(" NODEPTH");
    MREHit* Depth = CreateHit(MVector(30.0, 0.0, 0.5), 200.0, MDDetector::c_Strip3D);
    MRECluster Cluster;
    AddQuietly(Cluster, NoDepth);
    AddQuietly(Cluster, Depth);
    Passed = EvaluateNear("AddRESE()", "no depth, then depth", "The depth of the cluster is the one of the hit with depth, 0.5 cm", Cluster.GetPosition().Z(), 0.5, 1e-12) && Passed;
    Passed = EvaluateFalse("AddRESE()", "no depth, then depth", "The cluster is no longer flagged as without depth", Cluster.GetNoiseFlags().Contains("NODEPTH")) && Passed;
    Passed = ExpectedToFail(EvaluateNear("AddRESE()", "no depth, then depth", "x is the mid point 30.5 cm", Cluster.GetPosition().X(), 30.5, 1e-12)) && Passed;
    delete NoDepth;
    delete Depth;
  }

  // Two hits with depth: the depth is the center of energy
  {
    MREHit* DepthA = CreateHit(MVector(30.0, 0.0, 0.5), 200.0, MDDetector::c_Strip3D);
    MREHit* DepthB = CreateHit(MVector(31.0, 2.0, 0.9), 200.0, MDDetector::c_Strip3D);
    MRECluster Cluster;
    AddQuietly(Cluster, DepthA);
    AddQuietly(Cluster, DepthB);
    Passed = ExpectedToFail(EvaluateNear("AddRESE()", "two hits with depth", "The depth is the mid point 0.7 cm", Cluster.GetPosition().Z(), 0.7, 1e-12)) && Passed;
    Passed = EvaluateFalse("AddRESE()", "two hits with depth", "The cluster is not flagged as without depth", Cluster.GetNoiseFlags().Contains("NODEPTH")) && Passed;
    delete DepthA;
    delete DepthB;
  }

  // Two hits without depth: the cluster stays flagged, and the flag is not doubled
  {
    MREHit* NoDepthA = CreateHit(MVector(30.0, 0.0, 0.0), 200.0, MDDetector::c_Strip3D);
    NoDepthA->SetNoiseFlags(" NODEPTH");
    MREHit* NoDepthB = CreateHit(MVector(31.0, 2.0, 0.0), 200.0, MDDetector::c_Strip3D);
    NoDepthB->SetNoiseFlags(" NODEPTH");
    MRECluster Cluster;
    AddQuietly(Cluster, NoDepthA);
    AddQuietly(Cluster, NoDepthB);
    Passed = Evaluate("AddRESE()", "two hits without depth", "The cluster is flagged once as without depth", Cluster.GetNoiseFlags(), MString(" NODEPTH")) && Passed;
    Passed = EvaluateNear("AddRESE()", "two hits without depth", "The depth is the center of the wafer", Cluster.GetPosition().Z(), 0.0, 1e-12) && Passed;
    delete NoDepthA;
    delete NoDepthB;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding clusters and tracks
bool UTRECluster::TestAddClustersAndTracks()
{
  bool Passed = true;

  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  MREHit* B = CreateHit(MVector(1.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);

  MRECluster Source;
  AddQuietly(Source, A);
  AddQuietly(Source, B);

  // Adding a cluster adds its hits:
  MRECluster Target;
  AddQuietly(Target, &Source);
  // Defect REV-B-8: AddRESE loops over the number of hits of this cluster and not of the added cluster, so an empty cluster takes no hit, and a larger cluster reads beyond the end of the smaller one
  Passed = ExpectedToFail(Evaluate("AddRESE()", "cluster of two hits to an empty cluster", "The hits of the added cluster are added", Target.GetNRESEs(), 2)) && Passed;
  Passed = ExpectedToFail(EvaluateNear("AddRESE()", "cluster of two hits to an empty cluster", "The energy is the one of the added cluster", Target.GetEnergy(), 200.0, 1e-12)) && Passed;
  Passed = ExpectedToCrash(EvaluateTrue("AddRESE()", "cluster of one hit to a cluster of two hits", "The hit of the added cluster is added", AddSmallerCluster())) && Passed;

  // Defect REV-B-9: Adding a track, and removing a hit which is not in the cluster, end the program with Fatal() instead of returning an error
  Passed = ExpectedToCrash(EvaluateTrue("AddRESE()", "track", "A track is rejected and the program goes on", AddTrack())) && Passed;
  Passed = ExpectedToCrash(EvaluateTrue("RemoveRESE()", "hit of another cluster", "A nullptr is returned and the program goes on", RemoveUnknownHit())) && Passed;

  delete A;
  delete B;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding hits to a cluster which may not be overwritten
bool UTRECluster::TestAllowOverwrite()
{
  bool Passed = true;

  MREHit* A = CreateHit(MVector(1.0, 2.0, 3.0), 100.0, MDDetector::c_Strip2D, nullptr, 5.0);
  A->SetEnergyResolution(3.0);
  MREHit* B = CreateHit(MVector(4.0, 8.0, -2.0), 300.0, MDDetector::c_Strip2D, nullptr, 4.0);
  B->SetEnergyResolution(4.0);

  // The data of the cluster was set from outside and is kept:
  MRECluster Cluster;
  Cluster.SetPosition(MVector(7.0, 7.0, 7.0));
  Cluster.SetEnergy(555.0);
  Cluster.SetTime(9.0);
  Cluster.SetEnergyResolution(11.0);
  Cluster.SetDetector(MDDetector::c_Calorimeter);
  Cluster.AllowOverwrite(false);
  MString Output = AddQuietly(Cluster, A);
  Output += AddQuietly(Cluster, B);
  Passed = Evaluate("AddRESE()", "no overwrite", "The hits are added", Cluster.GetNRESEs(), 2) && Passed;
  Passed = EvaluateVectorNear("AddRESE()", "no overwrite", "The position is kept", Cluster.GetPosition(), MVector(7.0, 7.0, 7.0), 0.0) && Passed;
  Passed = EvaluateNear("AddRESE()", "no overwrite", "The energy is kept", Cluster.GetEnergy(), 555.0, 0.0) && Passed;
  Passed = EvaluateNear("AddRESE()", "no overwrite", "The time is kept", Cluster.GetTime(), 9.0, 0.0) && Passed;
  Passed = EvaluateNear("AddRESE()", "no overwrite", "The energy resolution is kept", Cluster.GetEnergyResolution(), 11.0, 0.0) && Passed;
  Passed = Evaluate("AddRESE()", "no overwrite", "The detector type is kept", Cluster.GetDetector(), MDDetector::c_Calorimeter) && Passed;
  Passed = EvaluateTrue("AddRESE()", "no overwrite", "Nothing is printed to the standard output", Output.IsEmpty()) && Passed;

  // Removing a hit keeps the data too:
  Cluster.RemoveRESE(B);
  Passed = EvaluateFalse("RemoveRESE()", "no overwrite", "The hit is removed", Cluster.ContainsRESE(B)) && Passed;
  Passed = EvaluateVectorNear("RemoveRESE()", "no overwrite", "The position is kept", Cluster.GetPosition(), MVector(7.0, 7.0, 7.0), 0.0) && Passed;
  Passed = EvaluateNear("RemoveRESE()", "no overwrite", "The energy is kept", Cluster.GetEnergy(), 555.0, 0.0) && Passed;

  delete A;
  delete B;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test removing hits
bool UTRECluster::TestRemoveRESE()
{
  bool Passed = true;

  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  A->SetEnergyResolution(3.0);
  MREHit* B = CreateHit(MVector(4.0, 8.0, -2.0), 300.0, MDDetector::c_Strip2D);
  B->SetEnergyResolution(4.0);

  MRECluster Cluster;
  AddQuietly(Cluster, A);
  AddQuietly(Cluster, B);
  // The center of energy is set by hand, since AddRESE does not find it (defect REV-B-6): (100*(0, 0, 0) + 300*(4, 8, -2))/400 = (3, 6, -1.5)
  Cluster.SetPosition(MVector(3.0, 6.0, -1.5));

  MRESE* Removed = Cluster.RemoveRESE(B);
  Passed = EvaluateTrue("RemoveRESE()", "second of two hits", "The removed hit is returned", Removed == B) && Passed;
  Passed = EvaluateFalse("RemoveRESE()", "second of two hits", "The cluster no longer contains the hit", Cluster.ContainsRESE(B)) && Passed;
  Passed = EvaluateTrue("RemoveRESE()", "second of two hits", "The cluster still contains the other hit", Cluster.ContainsRESE(A)) && Passed;
  Passed = EvaluateNear("RemoveRESE()", "second of two hits", "The energy is the one of the remaining hit, 100 keV", Cluster.GetEnergy(), 100.0, 1e-12) && Passed;
  // (400*(3, 6, -1.5) - 300*(4, 8, -2))/100 = (0, 0, 0)
  Passed = EvaluateVectorNear("RemoveRESE()", "second of two hits", "The position is the one of the remaining hit", Cluster.GetPosition(), MVector(0.0, 0.0, 0.0), 1e-12) && Passed;
  Passed = EvaluateNear("RemoveRESE()", "second of two hits", "The energy resolution is the one of the remaining hit", Cluster.GetEnergyResolution(), 3.0, 1e-12) && Passed;
  // Intended: the slot of the removed hit stays empty until the cluster is compressed
  Passed = Evaluate("RemoveRESE()", "second of two hits", "The slot of the removed hit stays", Cluster.GetNRESEs(), 2) && Passed;
  Cluster.CompressRESEs();
  Passed = Evaluate("CompressRESEs()", "after the removal", "There is one hit", Cluster.GetNRESEs(), 1) && Passed;

  // Defect REV-B-10: Removing the last hit divides by the remaining energy of zero, the position of the empty cluster is not a number
  Cluster.RemoveRESE(A);
  Cluster.CompressRESEs();
  Passed = EvaluateNear("RemoveRESE()", "last hit", "The energy is zero", Cluster.GetEnergy(), 0.0, 1e-12) && Passed;
  Passed = ExpectedToFail(EvaluateVectorNear("RemoveRESE()", "last hit", "The position of the empty cluster is the origin again", Cluster.GetPosition(), MVector(0.0, 0.0, 0.0), 1e-12)) && Passed;

  // A hit without depth does not change the depth of the cluster when it is removed:
  MREHit* Depth = CreateHit(MVector(30.0, 0.0, 0.5), 200.0, MDDetector::c_Strip3D);
  MREHit* NoDepth = CreateHit(MVector(31.0, 2.0, 0.0), 200.0, MDDetector::c_Strip3D);
  NoDepth->SetNoiseFlags(" NODEPTH");
  MRECluster DepthCluster;
  AddQuietly(DepthCluster, Depth);
  AddQuietly(DepthCluster, NoDepth);
  // The center of energy in x and y, and the depth of the hit with depth
  DepthCluster.SetPosition(MVector(30.5, 1.0, 0.5));
  DepthCluster.RemoveRESE(NoDepth);
  // x: (400*30.5 - 200*31)/200 = 30, y: (400*1 - 200*2)/200 = 0
  Passed = EvaluateVectorNear("RemoveRESE()", "hit without depth", "The position is the one of the remaining hit with its depth", DepthCluster.GetPosition(), MVector(30.0, 0.0, 0.5), 1e-12) && Passed;

  // Defect REV-B-11: After a removal the functions of the cluster use the empty slot without a check
  Passed = ExpectedToCrash(EvaluateTrue("AddRESE()", "after RemoveRESE()", "A hit can be added after another one was removed", AddAfterRemove())) && Passed;
  Passed = ExpectedToCrash(EvaluateTrue("ToString()/IsValid()", "after RemoveRESE()", "The text and the validity are available after a hit was removed", ToStringAfterRemove())) && Passed;

  delete A;
  delete B;
  delete Depth;
  delete NoDepth;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the distances to hits, clusters, and tracks
bool UTRECluster::TestDistances()
{
  bool Passed = true;

  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  MREHit* B = CreateHit(MVector(1.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  MREHit* Far = CreateHit(MVector(4.0, 4.0, 12.0), 100.0, MDDetector::c_Strip2D);
  MREHit* Farther = CreateHit(MVector(4.0, 4.0, 20.0), 100.0, MDDetector::c_Strip2D);

  MRECluster Cluster;
  AddQuietly(Cluster, A);
  AddQuietly(Cluster, B);

  // The closest hit of the cluster is B: (4, 4, 12) - (1, 0, 0) = (3, 4, 12) with length 13
  Passed = EvaluateNear("ComputeMinDistance()", "cluster to hit", "The distance from the closest hit of the cluster is 13 cm", Cluster.ComputeMinDistance(Far), 13.0, 1e-12) && Passed;
  Passed = EvaluateNear("ComputeMinDistance()", "cluster to its own hit", "The distance to a hit of the cluster is zero", Cluster.ComputeMinDistance(A), 0.0, 0.0) && Passed;
  // Defect REV-B-12: The vector of a cluster (and of a track) points from the other element to the cluster - the comment in the function and the hits have it point to the other element
  Passed = ExpectedToFail(EvaluateVectorNear("ComputeMinDistanceVector()", "cluster to hit", "The vector points from the closest hit of the cluster to the hit", Cluster.ComputeMinDistanceVector(Far), MVector(3.0, 4.0, 12.0), 1e-12)) && Passed;
  // The hit has it the other way round, as it should be:
  Passed = EvaluateVectorNear("ComputeMinDistanceVector()", "hit to cluster", "The vector points from the hit to the closest hit of the cluster", Far->ComputeMinDistanceVector(&Cluster), MVector(-3.0, -4.0, -12.0), 1e-12) && Passed;

  // Cluster to cluster: the closest pair is B and Far
  MRECluster Other;
  AddQuietly(Other, Farther);
  AddQuietly(Other, Far);
  Passed = EvaluateNear("ComputeMinDistance()", "cluster to cluster", "The distance of the closest pair of hits is 13 cm", Cluster.ComputeMinDistance(&Other), 13.0, 1e-12) && Passed;
  Passed = EvaluateNear("ComputeMinDistance()", "cluster to cluster, reversed", "The distance is symmetric", Other.ComputeMinDistance(&Cluster), 13.0, 1e-12) && Passed;
  Passed = ExpectedToFail(EvaluateVectorNear("ComputeMinDistanceVector()", "cluster to cluster", "The vector points to the other cluster", Cluster.ComputeMinDistanceVector(&Other), MVector(3.0, 4.0, 12.0), 1e-12)) && Passed;

  // Cluster to track:
  Other.RemoveRESE(Far);
  Other.RemoveRESE(Farther);
  MRETrack Track;
  Track.AddRESE(Far);
  Track.AddRESE(Farther);
  Passed = EvaluateNear("ComputeMinDistance()", "cluster to track", "The distance of the closest pair of hits is 13 cm", Cluster.ComputeMinDistance(&Track), 13.0, 1e-12) && Passed;

  // An empty cluster has no distance:
  MRECluster Empty;
  Passed = EvaluateVectorNear("ComputeMinDistanceVector()", "empty cluster to hit", "All components are DBL_MAX", Empty.ComputeMinDistanceVector(Far), MVector(DBL_MAX, DBL_MAX, DBL_MAX), 0.0) && Passed;

  delete A;
  delete B;
  delete Far;
  delete Farther;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the adjacency check
bool UTRECluster::TestAreAdjacent()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope is scanned", ScanTelescopeGeometry(Geometry)) && Passed;

  // Tracker layer 0, pitch 1 cm: a cluster of the strips (0, 0) and (1, 0)
  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* B = CreateHit(MVector(1.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Neighbor = CreateHit(MVector(2.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Diagonal = CreateHit(MVector(2.0, 1.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* Away = CreateHit(MVector(3.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  MREHit* NextLayer = CreateHit(MVector(1.0, 0.0, 1.0), 100.0, MDDetector::c_Strip2D, &Geometry);
  bool AllCreated = false;
  if (A != nullptr && B != nullptr && Neighbor != nullptr && Diagonal != nullptr && Away != nullptr && NextLayer != nullptr) {
    AllCreated = true;
  }
  Passed = EvaluateTrue("CreateHit()", "tracker hits", "All hits are created", AllCreated) && Passed;
  if (AllCreated == false) {
    return Passed;
  }

  MRECluster Cluster;
  AddQuietly(Cluster, A);
  AddQuietly(Cluster, B);

  Passed = EvaluateTrue("AreAdjacent()", "hit beside the cluster", "A hit one strip from a hit of the cluster is adjacent", Cluster.AreAdjacent(Neighbor, 3, 1)) && Passed;
  Passed = EvaluateFalse("AreAdjacent()", "diagonal hit, level 1", "A diagonal hit is not adjacent at level 1", Cluster.AreAdjacent(Diagonal, 3, 1)) && Passed;
  Passed = EvaluateTrue("AreAdjacent()", "diagonal hit, level 2", "A diagonal hit is adjacent at level 2", Cluster.AreAdjacent(Diagonal, 3, 2)) && Passed;
  Passed = EvaluateFalse("AreAdjacent()", "hit two strips away", "A hit two strips from all hits of the cluster is not adjacent", Cluster.AreAdjacent(Away, 3, 1)) && Passed;
  Passed = EvaluateFalse("AreAdjacent()", "hit in the next layer", "A hit in another wafer is not adjacent", Cluster.AreAdjacent(NextLayer, -1, 1)) && Passed;

  // Cluster to cluster and cluster to track:
  MRECluster Other;
  AddQuietly(Other, Away);
  AddQuietly(Other, Neighbor);
  Passed = EvaluateTrue("AreAdjacent()", "cluster with an adjacent hit", "The clusters are adjacent", Cluster.AreAdjacent(&Other, 3, 1)) && Passed;
  Passed = EvaluateTrue("AreAdjacent()", "cluster with an adjacent hit, reversed", "The check is symmetric", Other.AreAdjacent(&Cluster, 3, 1)) && Passed;
  Other.RemoveRESE(Away);
  Other.RemoveRESE(Neighbor);

  MRETrack Track;
  Track.AddRESE(Away);
  Track.AddRESE(Neighbor);
  Passed = EvaluateTrue("AreAdjacent()", "track with an adjacent hit", "The cluster is adjacent to the track", Cluster.AreAdjacent(&Track, 3, 1)) && Passed;

  MRECluster Single;
  AddQuietly(Single, Away);
  Passed = EvaluateFalse("AreAdjacent()", "cluster without an adjacent hit", "The clusters are not adjacent", Cluster.AreAdjacent(&Single, 3, 1)) && Passed;

  // An empty cluster is adjacent to nothing:
  MRECluster Empty;
  Passed = EvaluateFalse("AreAdjacent()", "empty cluster", "An empty cluster is not adjacent to a hit", Empty.AreAdjacent(A, 3, 1)) && Passed;

  delete A;
  delete B;
  delete Neighbor;
  delete Diagonal;
  delete Away;
  delete NextLayer;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the copy constructor and the duplication
bool UTRECluster::TestCopyAndDuplicate()
{
  bool Passed = true;

  MREHit* A = CreateHit(MVector(30.0, 0.0, 0.5), 200.0, MDDetector::c_Strip3D, nullptr, 2.0);
  A->SetEnergyResolution(3.0);
  MREHit* B = CreateHit(MVector(31.0, 2.0, 0.9), 200.0, MDDetector::c_Strip3D, nullptr, 1.0);
  B->SetEnergyResolution(4.0);

  MRECluster Original;
  AddQuietly(Original, A);
  AddQuietly(Original, B);
  Original.SetPosition(MVector(30.5, 1.0, 0.7));

  MRECluster* Duplicate = Original.Duplicate();
  Passed = EvaluateTrue("Duplicate()", "cluster of two hits", "A new cluster is returned", Duplicate != nullptr && Duplicate != &Original) && Passed;
  if (Duplicate != nullptr) {
    Passed = Evaluate("Duplicate()", "cluster of two hits", "The type is a cluster", Duplicate->GetType(), MRESE::c_Cluster) && Passed;
    Passed = Evaluate("Duplicate()", "cluster of two hits", "The ID is kept", Duplicate->GetID(), Original.GetID()) && Passed;
    Passed = EvaluateVectorNear("Duplicate()", "cluster of two hits", "The position is copied", Duplicate->GetPosition(), MVector(30.5, 1.0, 0.7), 0.0) && Passed;
    Passed = EvaluateNear("Duplicate()", "cluster of two hits", "The energy is copied", Duplicate->GetEnergy(), 400.0, 0.0) && Passed;
    Passed = EvaluateNear("Duplicate()", "cluster of two hits", "The energy resolution is copied", Duplicate->GetEnergyResolution(), 5.0, 1e-12) && Passed;
    Passed = EvaluateNear("Duplicate()", "cluster of two hits", "The time is copied", Duplicate->GetTime(), 1.0, 0.0) && Passed;
    Passed = Evaluate("Duplicate()", "cluster of two hits", "The detector type is copied", Duplicate->GetDetector(), MDDetector::c_Strip3D) && Passed;
    Passed = Evaluate("Duplicate()", "cluster of two hits", "There are two hits", Duplicate->GetNRESEs(), 2) && Passed;
    if (Duplicate->GetNRESEs() == 2) {
      // The hits are duplicated too:
      Passed = EvaluateTrue("Duplicate()", "cluster of two hits", "The hits are own objects", Duplicate->GetRESEAt(0) != A && Duplicate->GetRESEAt(1) != B) && Passed;
      Passed = Evaluate("Duplicate()", "cluster of two hits", "The first hit has the ID of the original one", Duplicate->GetRESEAt(0)->GetID(), A->GetID()) && Passed;
      Passed = Evaluate("Duplicate()", "cluster of two hits", "The hits are hits", Duplicate->GetRESEAt(1)->GetType(), MRESE::c_Hit) && Passed;
      Passed = EvaluateVectorNear("Duplicate()", "cluster of two hits", "The position of the second hit is copied", Duplicate->GetRESEAt(1)->GetPosition(), MVector(31.0, 2.0, 0.9), 0.0) && Passed;

      // The duplicate is independent:
      Duplicate->GetRESEAt(0)->SetEnergy(1.0);
      Passed = EvaluateNear("Duplicate()", "hit of the duplicate changed", "The hit of the original is unchanged", A->GetEnergy(), 200.0, 0.0) && Passed;
    }
    Duplicate->SetEnergy(1.0);
    Passed = EvaluateNear("Duplicate()", "duplicate changed", "The original is unchanged", Original.GetEnergy(), 400.0, 0.0) && Passed;

    // The duplicated hits belong to the duplicate:
    Duplicate->DeleteAll();
    delete Duplicate;
  }

  // Through the base class pointer the cluster is duplicated as a cluster:
  MRESE* Base = &Original;
  MRESE* BaseDuplicate = Base->Duplicate();
  Passed = EvaluateTrue("Duplicate()", "base class pointer", "The duplicate is a cluster", dynamic_cast<MRECluster*>(BaseDuplicate) != nullptr) && Passed;
  BaseDuplicate->DeleteAll();
  delete BaseDuplicate;

  // Defect REV-B-13: The copy constructor does not set the flag which ignores missing depths, so the copy handles hits without depth by what is in its memory
  // A copy in memory filled with zeros has the flag switched off: a zero block is used on purpose, a set bit would hide the missing initialization
  alignas(MRECluster) unsigned char Block[sizeof(MRECluster)];
  FillMemory(Block, sizeof(MRECluster), 0x00);
  MRECluster* Placed = new (Block) MRECluster(&Original);
  MREHit* NoDepth = CreateHit(MVector(31.0, 2.0, 0.0), 200.0, MDDetector::c_Strip3D);
  NoDepth->SetNoiseFlags(" NODEPTH");
  AddQuietly(*Placed, NoDepth);
  Passed = ExpectedToFail(EvaluateNear("MRECluster(MRECluster*)", "copy in zeroed memory, hit without depth added", "The depth of the copy is kept, as for the original", Placed->GetPosition().Z(), 0.7, 1e-12)) && Passed;
  Placed->RemoveRESE(NoDepth);
  Placed->DeleteAll();
  Placed->~MRECluster();
  delete NoDepth;

  delete A;
  delete B;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the validity
bool UTRECluster::TestIsValid()
{
  bool Passed = true;

  MREHit* A = CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);
  MREHit* B = CreateHit(MVector(1.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D);

  MRECluster Cluster;
  AddQuietly(Cluster, A);
  AddQuietly(Cluster, B);
  Passed = EvaluateTrue("IsValid()", "two valid hits", "The cluster is valid", Cluster.IsValid()) && Passed;

  B->SetValid(false);
  Passed = EvaluateFalse("IsValid()", "second hit not valid", "The cluster is not valid", Cluster.IsValid()) && Passed;
  B->SetValid(true);
  A->SetValid(false);
  Passed = EvaluateFalse("IsValid()", "first hit not valid", "The cluster is not valid", Cluster.IsValid()) && Passed;
  A->SetValid(true);
  Passed = EvaluateTrue("IsValid()", "both hits valid again", "The cluster is valid again", Cluster.IsValid()) && Passed;

  delete A;
  delete B;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the text output
bool UTRECluster::TestToString()
{
  bool Passed = true;

  MRESE::ResetIDCounter();
  MRECluster Cluster;
  MREHit* A = CreateHit(MVector(1.0, 2.0, 3.0), 100.0, MDDetector::c_Strip2D);
  A->SetEnergyResolution(3.0);
  MREHit* B = CreateHit(MVector(2.0, 2.0, 3.0), 300.0, MDDetector::c_Strip2D);
  B->SetEnergyResolution(4.0);
  AddQuietly(Cluster, A);
  AddQuietly(Cluster, B);
  Cluster.SetPosition(MVector(1.75, 2.0, 3.0));

  // The cluster has ID 1, the hits 2 and 3
  MString ClusterLine = "Cluster (1) at (1.750, 2.000, 3.000)+-(0.000, 0.000, 0.000) with 400.00+-5.00 keV in 1 the following hits:\n";
  MString HitLineA = "Hit (2) at (1.000, 2.000, 3.000)+-(0.000, 0.000, 0.000) with 100.00+-3.00 keV in detector 1\n";
  MString HitLineB = "Hit (3) at (2.000, 2.000, 3.000)+-(0.000, 0.000, 0.000) with 300.00+-4.00 keV in detector 1\n";
  Passed = Evaluate("ToString()", "cluster of two hits", "The text has the cluster and its hits one level deeper", Cluster.ToString(), ClusterLine + "   " + HitLineA + "   " + HitLineB) && Passed;
  Passed = Evaluate("ToString()", "cluster of two hits, level 1", "Each level indents by three blanks", Cluster.ToString(true, 1), MString("   ") + ClusterLine + "      " + HitLineA + "      " + HitLineB) && Passed;

  // The fourth element has ID 4:
  MRECluster Link;
  Cluster.AddLink(&Link);
  Passed = Evaluate("ToString()", "one link", "The link is listed in the last line", Cluster.ToString(), ClusterLine + "   " + HitLineA + "   " + HitLineB + "   Linked with:   (4)\n") && Passed;
  Passed = Evaluate("ToString()", "one link, without links", "The link is not listed", Cluster.ToString(false), ClusterLine + "   " + HitLineA + "   " + HitLineB) && Passed;

  // An empty cluster:
  MRECluster Empty;
  Passed = Evaluate("ToString()", "empty cluster", "Only the line of the cluster is written", Empty.ToString(), MString("Cluster (5) at (0.000, 0.000, 0.000)+-(0.000, 0.000, 0.000) with 0.00+-0.00 keV in -1 the following hits:\n")) && Passed;

  delete A;
  delete B;

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

  UTRECluster Test;
  return Test.Run() == true ? 0 : 1;
}


// UTRECluster.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
