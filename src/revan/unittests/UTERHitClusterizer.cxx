/*
 * UTERHitClusterizer.cxx
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
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MDDetector.h"
#include "MERHitClusterizer.h"
#include "MRawEventIncarnations.h"
#include "MRECluster.h"
#include "MREHit.h"
#include "MRERawEvent.h"
#include "MResponseMatrixO2.h"
#include "MResponseMatrixO3.h"
#include "UTRevanShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the hit clusterizer MERHitClusterizer of revan
class UTERHitClusterizer : public UTRevanBase
{
public:
  //! Default constructor
  UTERHitClusterizer() : UTRevanBase("UTERHitClusterizer") {}
  //! Default destructor
  virtual ~UTERHitClusterizer() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test a clusterizer without an algorithm
  bool TestNoAlgorithm();
  //! Test the options text
  bool TestToString();
  //! Test the levels of the clustering of adjacent voxels
  bool TestAdjacentLevels();
  //! Test the depth cut of the clustering of adjacent voxels
  bool TestAdjacentSigma();
  //! Test the content of a cluster
  bool TestClusterContent();
  //! Test the clustering by distance
  bool TestDistance();
  //! Test the reference of the distance to a cluster
  bool TestDistanceReference();
  //! Test the merging of two clusters
  bool TestClusterMerging();
  //! Test a list with several events
  bool TestSeveralEvents();
  //! Test the failures of reading the response files
  bool TestResponseFilesMissing();
  //! Test the clustering with the response files
  bool TestResponse();

  //! Add a new event with hits of 100 keV, 110 keV, ... at the given positions to the list, a nullptr if a hit cannot be created
  MRERawEvent* AddEvent(MRawEventIncarnations& List, MGeometryRevan& Geometry, const vector<MVector>& Positions, int DetectorType);
  //! Clusterize the list and hide the output of the cluster class
  bool Clusterize(MERHitClusterizer& Clusterizer, MRawEventIncarnations& List);
  //! Return the number of clusters in the event
  int GetNClusters(MRERawEvent* Event);
  //! Return the number of hits in the event, which are in no cluster
  int GetNSingleHits(MRERawEvent* Event);
  //! Return the first cluster with the given number of hits, or a nullptr
  MRESE* FindCluster(MRERawEvent* Event, int NHits);
  //! Write the four response files with the given prefix: two-hit clusters with a depth difference above 0.5 cm and larger clusters above 1000 keV are separable
  bool WriteResponseFiles(const MString& Prefix);

  //! The output of the last clustering
  MString m_Output;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERHitClusterizer::Run()
{
  bool Passed = true;

  Passed = TestNoAlgorithm() && Passed;
  Passed = TestToString() && Passed;
  Passed = TestAdjacentLevels() && Passed;
  Passed = TestAdjacentSigma() && Passed;
  Passed = TestClusterContent() && Passed;
  Passed = TestDistance() && Passed;
  Passed = TestDistanceReference() && Passed;
  Passed = TestClusterMerging() && Passed;
  Passed = TestSeveralEvents() && Passed;
  Passed = TestResponseFilesMissing() && Passed;
  Passed = TestResponse() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Add a new event with hits of 100 keV, 110 keV, ... at the given positions to the list, a nullptr if a hit cannot be created
MRERawEvent* UTERHitClusterizer::AddEvent(MRawEventIncarnations& List, MGeometryRevan& Geometry, const vector<MVector>& Positions, int DetectorType)
{
  // The list does not own the event: the test deletes it
  MRERawEvent* Event = new MRERawEvent(&Geometry);
  for (unsigned int p = 0; p < Positions.size(); ++p) {
    MREHit* Hit = CreateHit(Positions[p], 100.0 + 10.0*p, DetectorType, &Geometry);
    if (Hit == nullptr) {
      delete Event;
      return nullptr;
    }
    Event->AddRESE(Hit);
  }
  List.AddRawEvent(Event);

  return Event;
}


////////////////////////////////////////////////////////////////////////////////


//! Clusterize the list and hide the output of the cluster class
bool UTERHitClusterizer::Clusterize(MERHitClusterizer& Clusterizer, MRawEventIncarnations& List)
{
  // The cluster class prints with cout, the clusterizer with the MEGAlib streams
  DisableDefaultStreams();
  BeginCaptureOutput();
  bool Analyzed = Clusterizer.Analyze(&List);
  m_Output = EndCaptureOutput();
  EnableDefaultStreams();

  return Analyzed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the number of clusters in the event
int UTERHitClusterizer::GetNClusters(MRERawEvent* Event)
{
  int NClusters = 0;
  for (int r = 0; r < Event->GetNRESEs(); ++r) {
    if (Event->GetRESEAt(r) != nullptr && Event->GetRESEAt(r)->GetType() == MRESE::c_Cluster) {
      ++NClusters;
    }
  }

  return NClusters;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the number of hits in the event, which are in no cluster
int UTERHitClusterizer::GetNSingleHits(MRERawEvent* Event)
{
  int NHits = 0;
  for (int r = 0; r < Event->GetNRESEs(); ++r) {
    if (Event->GetRESEAt(r) != nullptr && Event->GetRESEAt(r)->GetType() == MRESE::c_Hit) {
      ++NHits;
    }
  }

  return NHits;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the first cluster with the given number of hits, or a nullptr
MRESE* UTERHitClusterizer::FindCluster(MRERawEvent* Event, int NHits)
{
  for (int r = 0; r < Event->GetNRESEs(); ++r) {
    if (Event->GetRESEAt(r) != nullptr && Event->GetRESEAt(r)->GetType() == MRESE::c_Cluster && Event->GetRESEAt(r)->GetNRESEs() == NHits) {
      return Event->GetRESEAt(r);
    }
  }

  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


//! Write the four response files with the given prefix: two-hit clusters with a depth difference above 0.5 cm and larger clusters above 1000 keV are separable
bool UTERHitClusterizer::WriteResponseFiles(const MString& Prefix)
{
  // Two-hit clusters: lower energy, higher energy, depth difference
  // The probability to be separable is yes/(yes + no): 1/(1 + 3) = 0.25 below 0.5 cm and 3/(3 + 1) = 0.75 above
  MResponseMatrixO3 DualYes;
  DualYes.SetAxis(vector<float>{ 0.0f, 10000.0f }, vector<float>{ 0.0f, 10000.0f }, vector<float>{ 0.0f, 0.5f, 10.0f });
  DualYes.SetBinContent(0, 0, 0, 1.0f);
  DualYes.SetBinContent(0, 0, 1, 3.0f);
  MResponseMatrixO3 DualNo;
  DualNo.SetAxis(vector<float>{ 0.0f, 10000.0f }, vector<float>{ 0.0f, 10000.0f }, vector<float>{ 0.0f, 0.5f, 10.0f });
  DualNo.SetBinContent(0, 0, 0, 3.0f);
  DualNo.SetBinContent(0, 0, 1, 1.0f);

  // All other clusters: energy, number of hits - 0.25 below 1000 keV and 0.75 above
  MResponseMatrixO2 AllYes;
  AllYes.SetAxis(vector<float>{ 0.0f, 1000.0f, 100000.0f }, vector<float>{ 0.0f, 100.0f });
  AllYes.SetBinContent(0, 0, 1.0f);
  AllYes.SetBinContent(1, 0, 3.0f);
  MResponseMatrixO2 AllNo;
  AllNo.SetAxis(vector<float>{ 0.0f, 1000.0f, 100000.0f }, vector<float>{ 0.0f, 100.0f });
  AllNo.SetBinContent(0, 0, 3.0f);
  AllNo.SetBinContent(1, 0, 1.0f);

  // Only the stream format can be written
  DisableDefaultStreams();
  bool Written = true;
  if (DualYes.Write(Prefix + ".dualseparable.yes.rsp", true) == false) Written = false;
  if (DualNo.Write(Prefix + ".dualseparable.no.rsp", true) == false) Written = false;
  if (AllYes.Write(Prefix + ".allseparable.yes.rsp", true) == false) Written = false;
  if (AllNo.Write(Prefix + ".allseparable.no.rsp", true) == false) Written = false;
  EnableDefaultStreams();

  return Written;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a clusterizer without an algorithm
bool UTERHitClusterizer::TestNoAlgorithm()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry)) == false) {
    return false;
  }

  // Two hits in adjacent strips of the tracker are not clustered without an algorithm
  MERHitClusterizer Clusterizer;
  Clusterizer.SetGeometry(&Geometry);
  MRawEventIncarnations List;
  MRERawEvent* Event = AddEvent(List, Geometry, { MVector(1.0, 2.0, 0.0), MVector(2.0, 2.0, 0.0) }, MDDetector::c_Strip2D);
  if (EvaluateTrue("CreateHit()", "two tracker hits", "The hits are created", Event != nullptr) == false) {
    return false;
  }
  Passed = EvaluateTrue("Analyze()", "no algorithm", "The analysis succeeds", Clusterize(Clusterizer, List)) && Passed;
  Passed = Evaluate("Analyze()", "no algorithm", "There are still two elements", Event->GetNRESEs(), 2) && Passed;
  Passed = Evaluate("Analyze()", "no algorithm", "Both are hits", GetNSingleHits(Event), 2) && Passed;

  // An empty list and an empty event
  MRawEventIncarnations EmptyList;
  Passed = EvaluateTrue("Analyze()", "empty list", "The analysis succeeds", Clusterize(Clusterizer, EmptyList)) && Passed;
  Clusterizer.SetParameters(2, -1.0);
  Passed = EvaluateTrue("Analyze()", "adjacent voxels, empty list", "The analysis succeeds", Clusterize(Clusterizer, EmptyList)) && Passed;
  MRERawEvent* Empty = new MRERawEvent(&Geometry);
  EmptyList.AddRawEvent(Empty);
  Passed = EvaluateTrue("Analyze()", "adjacent voxels, empty event", "The analysis succeeds", Clusterize(Clusterizer, EmptyList)) && Passed;
  Passed = Evaluate("Analyze()", "adjacent voxels, empty event", "The event stays empty", Empty->GetNRESEs(), 0) && Passed;

  // One hit
  MRawEventIncarnations SingleList;
  MRERawEvent* Single = AddEvent(SingleList, Geometry, { MVector(1.0, 2.0, 0.0) }, MDDetector::c_Strip2D);
  Passed = EvaluateTrue("Analyze()", "adjacent voxels, one hit", "The analysis succeeds", Clusterize(Clusterizer, SingleList)) && Passed;
  Passed = Evaluate("Analyze()", "adjacent voxels, one hit", "The hit stays a hit", GetNSingleHits(Single), 1) && Passed;

  delete Event;
  delete Empty;
  delete Single;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the options text
bool UTERHitClusterizer::TestToString()
{
  bool Passed = true;

  MERHitClusterizer Clusterizer;
  Passed = Evaluate("ToString()", "no algorithm", "The text has only the title", Clusterizer.ToString(), MString("# Clusterizer options:\n# \n# \n")) && Passed;

  Passed = EvaluateTrue("SetParameters()", "level 2, sigma 3", "The parameters are accepted", Clusterizer.SetParameters(2, 3.0)) && Passed;
  Passed = Evaluate("ToString()", "level 2, sigma 3", "The text has the level and the sigma", Clusterizer.ToString(), MString("# Clusterizer options:\n# \n# Clusterizing by adjacent voxels\n# Level: 2\n# Sigma: 3\n# \n")) && Passed;

  Passed = EvaluateTrue("SetParameters()", "distances 0.1 to 0.8", "The parameters are accepted", Clusterizer.SetParameters(0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, true)) && Passed;
  MString Expected = "# Clusterizer options:\n# \n# Clusterizing by distance\n";
  Expected += "# Min distance D1: 0.1\n# Min distance D2: 0.2\n# Min distance D3: 0.3\n# Min distance D4: 0.4\n";
  Expected += "# Min distance D5: 0.5\n# Min distance D6: 0.6\n# Min distance D7: 0.7\n# Min distance D8: 0.8\n# \n";
  Passed = Evaluate("ToString()", "distances 0.1 to 0.8", "The text has the eight distances", Clusterizer.ToString(), Expected) && Passed;
  Passed = Evaluate("ToString()", "core only", "The core text is the same", Clusterizer.ToString(true), Expected) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the levels of the clustering of adjacent voxels
bool UTERHitClusterizer::TestAdjacentLevels()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry)) == false) {
    return false;
  }

  // The strip centers of the tracker are 1 cm apart. The level is the squared distance in strips: 1 for a direct, 2 for a diagonal neighbor
  // A negative sigma switches the depth cut off

  // Level 1: direct neighbors only
  {
    MERHitClusterizer Clusterizer;
    Clusterizer.SetGeometry(&Geometry);
    Clusterizer.SetParameters(1, -1.0);

    MRawEventIncarnations List;
    MRERawEvent* Direct = AddEvent(List, Geometry, { MVector(1.0, 2.0, 0.0), MVector(2.0, 2.0, 0.0) }, MDDetector::c_Strip2D);
    MRERawEvent* Diagonal = AddEvent(List, Geometry, { MVector(1.0, 2.0, 0.0), MVector(2.0, 3.0, 0.0) }, MDDetector::c_Strip2D);
    MRERawEvent* Second = AddEvent(List, Geometry, { MVector(1.0, 2.0, 0.0), MVector(3.0, 2.0, 0.0) }, MDDetector::c_Strip2D);
    Passed = EvaluateTrue("Analyze()", "level 1", "The analysis succeeds", Clusterize(Clusterizer, List)) && Passed;
    Passed = Evaluate("Analyze()", "level 1, direct neighbors", "The hits are one cluster", Direct->GetNRESEs(), 1) && Passed;
    Passed = Evaluate("Analyze()", "level 1, direct neighbors", "The element is a cluster", GetNClusters(Direct), 1) && Passed;
    Passed = Evaluate("Analyze()", "level 1, diagonal neighbors", "The hits are not clustered", GetNSingleHits(Diagonal), 2) && Passed;
    Passed = Evaluate("Analyze()", "level 1, second neighbors", "The hits are not clustered", GetNSingleHits(Second), 2) && Passed;
    delete Direct;
    delete Diagonal;
    delete Second;
  }

  // Level 2: direct and diagonal neighbors
  {
    MERHitClusterizer Clusterizer;
    Clusterizer.SetGeometry(&Geometry);
    Clusterizer.SetParameters(2, -1.0);

    MRawEventIncarnations List;
    MRERawEvent* Direct = AddEvent(List, Geometry, { MVector(1.0, 2.0, 0.0), MVector(1.0, 3.0, 0.0) }, MDDetector::c_Strip2D);
    MRERawEvent* Diagonal = AddEvent(List, Geometry, { MVector(1.0, 2.0, 0.0), MVector(2.0, 3.0, 0.0) }, MDDetector::c_Strip2D);
    MRERawEvent* Second = AddEvent(List, Geometry, { MVector(1.0, 2.0, 0.0), MVector(3.0, 2.0, 0.0) }, MDDetector::c_Strip2D);
    Clusterize(Clusterizer, List);
    Passed = Evaluate("Analyze()", "level 2, direct neighbors", "The hits are one cluster", GetNClusters(Direct), 1) && Passed;
    Passed = Evaluate("Analyze()", "level 2, diagonal neighbors", "The hits are one cluster", GetNClusters(Diagonal), 1) && Passed;
    Passed = Evaluate("Analyze()", "level 2, diagonal neighbors", "No single hit is left", GetNSingleHits(Diagonal), 0) && Passed;
    Passed = Evaluate("Analyze()", "level 2, second neighbors", "The hits are not clustered", GetNSingleHits(Second), 2) && Passed;
    delete Direct;
    delete Diagonal;
    delete Second;
  }

  // Level 4: also the second neighbor in a row
  {
    MERHitClusterizer Clusterizer;
    Clusterizer.SetGeometry(&Geometry);
    Clusterizer.SetParameters(4, -1.0);

    MRawEventIncarnations List;
    MRERawEvent* Second = AddEvent(List, Geometry, { MVector(1.0, 2.0, 0.0), MVector(3.0, 2.0, 0.0) }, MDDetector::c_Strip2D);
    MRERawEvent* Knight = AddEvent(List, Geometry, { MVector(1.0, 2.0, 0.0), MVector(3.0, 3.0, 0.0) }, MDDetector::c_Strip2D);
    Clusterize(Clusterizer, List);
    Passed = Evaluate("Analyze()", "level 4, second neighbors", "The hits are one cluster", GetNClusters(Second), 1) && Passed;
    Passed = Evaluate("Analyze()", "level 4, two strips in x and one in y", "The hits are not clustered", GetNSingleHits(Knight), 2) && Passed;
    delete Second;
    delete Knight;
  }

  // A chain of neighbors is one cluster, a separate hit stays, and the same strips in the next layer are not adjacent
  {
    MERHitClusterizer Clusterizer;
    Clusterizer.SetGeometry(&Geometry);
    Clusterizer.SetParameters(1, -1.0);

    MRawEventIncarnations List;
    MRERawEvent* Chain = AddEvent(List, Geometry, { MVector(-4.0, 2.0, 0.0), MVector(-2.0, 2.0, 0.0), MVector(-3.0, 2.0, 0.0), MVector(4.0, -4.0, 0.0), MVector(-2.0, 1.0, 0.0) }, MDDetector::c_Strip2D);
    MRERawEvent* Layers = AddEvent(List, Geometry, { MVector(1.0, 2.0, GetTrackerLayerZ(0)), MVector(1.0, 2.0, GetTrackerLayerZ(1)), MVector(2.0, 2.0, GetTrackerLayerZ(1)) }, MDDetector::c_Strip2D);
    Clusterize(Clusterizer, List);
    Passed = Evaluate("Analyze()", "level 1, chain of four hits and a separate hit", "There are two elements", Chain->GetNRESEs(), 2) && Passed;
    Passed = EvaluateTrue("Analyze()", "level 1, chain of four hits and a separate hit", "The chain is one cluster of four hits", FindCluster(Chain, 4) != nullptr) && Passed;
    Passed = Evaluate("Analyze()", "level 1, chain of four hits and a separate hit", "The separate hit stays a hit", GetNSingleHits(Chain), 1) && Passed;
    // Energies 100 + 110 + 120 + 140 keV, the separate hit has 130 keV
    if (FindCluster(Chain, 4) != nullptr) {
      Passed = EvaluateNear("Analyze()", "level 1, chain of four hits and a separate hit", "The cluster has the sum of the energies", FindCluster(Chain, 4)->GetEnergy(), 470.0, 1e-12) && Passed;
    }
    Passed = EvaluateNear("GetEnergy()", "level 1, chain of four hits and a separate hit", "The energy of the event is not changed", Chain->GetEnergy(), 600.0, 1e-12) && Passed;

    Passed = Evaluate("Analyze()", "level 1, same strips in two layers", "There are two elements", Layers->GetNRESEs(), 2) && Passed;
    Passed = EvaluateTrue("Analyze()", "level 1, same strips in two layers", "The hits of layer 1 are a cluster", FindCluster(Layers, 2) != nullptr) && Passed;
    Passed = Evaluate("Analyze()", "level 1, same strips in two layers", "The hit of layer 0 stays a hit", GetNSingleHits(Layers), 1) && Passed;
    delete Chain;
    delete Layers;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the depth cut of the clustering of adjacent voxels
bool UTERHitClusterizer::TestAdjacentSigma()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry)) == false) {
    return false;
  }

  // The depth detector has a depth resolution of 0.1 cm. With 3 sigma two hits in adjacent strips are clustered,
  // if their depths differ by less than 3*(0.1 + 0.01) cm = 0.33 cm - 0.01 cm is the tolerance of the detector
  MERHitClusterizer Clusterizer;
  Clusterizer.SetGeometry(&Geometry);
  Clusterizer.SetParameters(1, 3.0);

  MRawEventIncarnations List;
  MRERawEvent* Inside = AddEvent(List, Geometry, { MVector(30.5, 0.5, 0.0), MVector(31.5, 0.5, 0.32) }, MDDetector::c_Strip3D);
  MRERawEvent* Outside = AddEvent(List, Geometry, { MVector(30.5, 0.5, 0.0), MVector(31.5, 0.5, 0.34) }, MDDetector::c_Strip3D);
  MRERawEvent* Below = AddEvent(List, Geometry, { MVector(30.5, 0.5, 0.0), MVector(31.5, 0.5, -0.34) }, MDDetector::c_Strip3D);
  if (EvaluateTrue("CreateHit()", "depth detector hits", "The hits are created", Inside != nullptr && Outside != nullptr && Below != nullptr) == false) {
    return false;
  }
  Clusterize(Clusterizer, List);
  Passed = Evaluate("Analyze()", "3 sigma, depth difference 0.32 cm", "The hits are one cluster", GetNClusters(Inside), 1) && Passed;
  Passed = Evaluate("Analyze()", "3 sigma, depth difference 0.34 cm", "The hits are not clustered", GetNSingleHits(Outside), 2) && Passed;
  Passed = Evaluate("Analyze()", "3 sigma, depth difference -0.34 cm", "The hits are not clustered", GetNSingleHits(Below), 2) && Passed;
  delete Inside;
  delete Outside;
  delete Below;

  // A negative sigma switches the depth cut off
  Clusterizer.SetParameters(1, -1.0);
  MRawEventIncarnations NoCutList;
  MRERawEvent* NoCut = AddEvent(NoCutList, Geometry, { MVector(30.5, 0.5, -0.9), MVector(31.5, 0.5, 0.9) }, MDDetector::c_Strip3D);
  Clusterize(Clusterizer, NoCutList);
  Passed = Evaluate("Analyze()", "negative sigma, depth difference 1.8 cm", "The hits are one cluster", GetNClusters(NoCut), 1) && Passed;
  delete NoCut;

  // The calorimeter is one block: all its hits are in the same voxel, its position resolution in z is 8 cm / sqrt(12) = 2.31 cm
  Clusterizer.SetParameters(1, 1.0);
  MRawEventIncarnations CalorimeterList;
  MRERawEvent* Near = AddEvent(CalorimeterList, Geometry, { MVector(-3.0, 2.0, -10.0), MVector(4.0, -1.0, -8.0) }, MDDetector::c_Calorimeter);
  MRERawEvent* Far = AddEvent(CalorimeterList, Geometry, { MVector(-3.0, 2.0, -13.0), MVector(4.0, -1.0, -10.0) }, MDDetector::c_Calorimeter);
  Clusterize(Clusterizer, CalorimeterList);
  Passed = Evaluate("Analyze()", "1 sigma, calorimeter hits 2 cm apart in z", "The hits are one cluster", GetNClusters(Near), 1) && Passed;
  Passed = Evaluate("Analyze()", "1 sigma, calorimeter hits 3 cm apart in z", "The hits are not clustered", GetNSingleHits(Far), 2) && Passed;
  delete Near;
  delete Far;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the content of a cluster
bool UTERHitClusterizer::TestClusterContent()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry)) == false) {
    return false;
  }
  MERHitClusterizer Clusterizer;
  Clusterizer.SetGeometry(&Geometry);
  Clusterizer.SetParameters(1, -1.0);

  // Hits of 100 keV at (1, 2, 0) and 110 keV at (2, 2, 0)
  MRawEventIncarnations List;
  MRERawEvent* Event = AddEvent(List, Geometry, { MVector(1.0, 2.0, 0.0), MVector(2.0, 2.0, 0.0) }, MDDetector::c_Strip2D);
  MRESE* First = Event->GetRESEAt(0);
  MRESE* Second = Event->GetRESEAt(1);
  Clusterize(Clusterizer, List);
  // Defect REV-E-12 (MRECluster, area B): adding the second hit to a cluster prints debug output with cout
  Passed = ExpectedToFail(Evaluate("Analyze()", "two adjacent tracker hits", "Nothing is printed", m_Output, MString(""))) && Passed;
  Passed = Evaluate("Analyze()", "two adjacent tracker hits", "There is one element", Event->GetNRESEs(), 1) && Passed;
  if (Event->GetNRESEs() == 1 && Event->GetRESEAt(0)->GetType() == MRESE::c_Cluster) {
    MRESE* Cluster = Event->GetRESEAt(0);
    Passed = Evaluate("Analyze()", "two adjacent tracker hits", "The cluster has two hits", Cluster->GetNRESEs(), 2) && Passed;
    Passed = EvaluateTrue("Analyze()", "two adjacent tracker hits", "The cluster contains the first hit", Cluster->ContainsRESE(First)) && Passed;
    Passed = EvaluateTrue("Analyze()", "two adjacent tracker hits", "The cluster contains the second hit", Cluster->ContainsRESE(Second)) && Passed;
    Passed = EvaluateNear("Analyze()", "two adjacent tracker hits", "The cluster has 210 keV", Cluster->GetEnergy(), 210.0, 1e-12) && Passed;
    Passed = Evaluate("Analyze()", "two adjacent tracker hits", "The detector type is Strip2D", Cluster->GetDetector(), static_cast<int>(MDDetector::c_Strip2D)) && Passed;
    // Energy resolution: 2 keV each, added in quadrature
    Passed = EvaluateNear("Analyze()", "two adjacent tracker hits", "The energy resolution is sqrt(2)*2 keV", Cluster->GetEnergyResolution(), sqrt(8.0), 1e-12) && Passed;
    // Defect REV-E-13 (MRECluster, area B): the center of the cluster is weighted with the energy of the cluster after the hit was added
    // Center of energy: x = (100*1 + 110*2)/210
    Passed = ExpectedToFail(EvaluateVectorNear("Analyze()", "two adjacent tracker hits", "The cluster is at the center of energy", Cluster->GetPosition(), MVector(320.0/210.0, 2.0, 0.0), 1e-12)) && Passed;
    Passed = EvaluateTrue("Analyze()", "two adjacent tracker hits", "The cluster is valid", Cluster->IsValid()) && Passed;
  } else {
    Passed = EvaluateTrue("Analyze()", "two adjacent tracker hits", "The element is a cluster", false) && Passed;
  }
  Passed = EvaluateNear("GetEnergy()", "two adjacent tracker hits", "The energy of the event is not changed", Event->GetEnergy(), 210.0, 1e-12) && Passed;

  // A second analysis of the same list changes nothing
  Clusterize(Clusterizer, List);
  Passed = Evaluate("Analyze()", "second analysis", "There is still one cluster", GetNClusters(Event), 1) && Passed;
  Passed = Evaluate("Analyze()", "second analysis", "There is still one element", Event->GetNRESEs(), 1) && Passed;
  if (Event->GetNRESEs() == 1) {
    Passed = Evaluate("Analyze()", "second analysis", "The cluster still has two hits", Event->GetRESEAt(0)->GetNRESEs(), 2) && Passed;
  }

  // A hit next to an existing cluster is added to it: the cluster is the first element, then the second
  MREHit* Third = CreateHit(MVector(3.0, 2.0, 0.0), 50.0, MDDetector::c_Strip2D, &Geometry);
  Event->AddRESE(Third);
  Clusterize(Clusterizer, List);
  Passed = Evaluate("Analyze()", "cluster and an adjacent hit", "There is one element", Event->GetNRESEs(), 1) && Passed;
  Passed = EvaluateTrue("Analyze()", "cluster and an adjacent hit", "The cluster has three hits", FindCluster(Event, 3) != nullptr) && Passed;
  MREHit* Fourth = CreateHit(MVector(0.0, 2.0, 0.0), 40.0, MDDetector::c_Strip2D, &Geometry);
  Event->RemoveRESE(Event->GetRESEAt(0));
  Event->CompressRESEs();
  Event->AddRESE(Fourth);
  MRECluster* Existing = new MRECluster();
  BeginCaptureOutput();
  Existing->AddRESE(First);
  Existing->AddRESE(Second);
  Existing->AddRESE(Third);
  EndCaptureOutput();
  Event->AddRESE(Existing);
  Clusterize(Clusterizer, List);
  Passed = Evaluate("Analyze()", "hit and an adjacent cluster", "There is one element", Event->GetNRESEs(), 1) && Passed;
  Passed = EvaluateTrue("Analyze()", "hit and an adjacent cluster", "The cluster has four hits", FindCluster(Event, 4) != nullptr) && Passed;
  Passed = EvaluateNear("GetEnergy()", "hit and an adjacent cluster", "The event has 300 keV", Event->GetEnergy(), 300.0, 1e-12) && Passed;

  delete Event;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the clustering by distance
bool UTERHitClusterizer::TestDistance()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry)) == false) {
    return false;
  }

  // Tracker hits which are less than the distance apart are clustered: the hits are 1 cm apart
  {
    MERHitClusterizer Clusterizer;
    Clusterizer.SetGeometry(&Geometry);
    Clusterizer.SetParameters(1.001, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, false);
    MRawEventIncarnations List;
    MRERawEvent* Event = AddEvent(List, Geometry, { MVector(1.0, 2.0, 0.0), MVector(2.0, 2.0, 0.0), MVector(4.0, 2.0, 0.0) }, MDDetector::c_Strip2D);
    Passed = EvaluateTrue("Analyze()", "distance 1.001 cm", "The analysis succeeds", Clusterize(Clusterizer, List)) && Passed;
    Passed = Evaluate("Analyze()", "distance 1.001 cm, hits 1 cm and 2 cm apart", "There are two elements", Event->GetNRESEs(), 2) && Passed;
    Passed = EvaluateTrue("Analyze()", "distance 1.001 cm, hits 1 cm and 2 cm apart", "The hits which are 1 cm apart are a cluster", FindCluster(Event, 2) != nullptr) && Passed;
    Passed = Evaluate("Analyze()", "distance 1.001 cm, hits 1 cm and 2 cm apart", "The hit which is 2 cm away stays a hit", GetNSingleHits(Event), 1) && Passed;
    delete Event;
  }
  {
    MERHitClusterizer Clusterizer;
    Clusterizer.SetGeometry(&Geometry);
    Clusterizer.SetParameters(0.999, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, false);
    MRawEventIncarnations List;
    MRERawEvent* Event = AddEvent(List, Geometry, { MVector(1.0, 2.0, 0.0), MVector(2.0, 2.0, 0.0) }, MDDetector::c_Strip2D);
    Clusterize(Clusterizer, List);
    Passed = Evaluate("Analyze()", "distance 0.999 cm, hits 1 cm apart", "The hits are not clustered", GetNSingleHits(Event), 2) && Passed;
    delete Event;
  }

  // The distance is not limited to one layer: the layers are 1 cm apart
  {
    MERHitClusterizer Clusterizer;
    Clusterizer.SetGeometry(&Geometry);
    Clusterizer.SetParameters(1.001, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, false);
    MRawEventIncarnations List;
    MRERawEvent* Event = AddEvent(List, Geometry, { MVector(1.0, 2.0, GetTrackerLayerZ(0)), MVector(1.0, 2.0, GetTrackerLayerZ(1)) }, MDDetector::c_Strip2D);
    Clusterize(Clusterizer, List);
    Passed = Evaluate("Analyze()", "distance 1.001 cm, same strips in two layers", "The hits are one cluster", GetNClusters(Event), 1) && Passed;
    delete Event;
  }

  // Each detector type has its own distance: only the tracker distance is set
  {
    MERHitClusterizer Clusterizer;
    Clusterizer.SetGeometry(&Geometry);
    Clusterizer.SetParameters(1.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, false);
    MRawEventIncarnations List;
    MRERawEvent* Calorimeter = AddEvent(List, Geometry, { MVector(0.0, 0.0, -10.0), MVector(1.0, 0.0, -10.0) }, MDDetector::c_Calorimeter);
    MRERawEvent* Depth = AddEvent(List, Geometry, { MVector(30.5, 0.5, 0.0), MVector(31.5, 0.5, 0.0) }, MDDetector::c_Strip3D);
    Clusterize(Clusterizer, List);
    Passed = Evaluate("Analyze()", "tracker distance 1.5 cm, calorimeter hits 1 cm apart", "The hits are not clustered", GetNSingleHits(Calorimeter), 2) && Passed;
    Passed = Evaluate("Analyze()", "tracker distance 1.5 cm, depth detector hits 1 cm apart", "The hits are not clustered", GetNSingleHits(Depth), 2) && Passed;
    delete Calorimeter;
    delete Depth;
  }
  // The second distance is the one of the calorimeter, the third the one of the depth detector
  {
    MERHitClusterizer Clusterizer;
    Clusterizer.SetGeometry(&Geometry);
    Clusterizer.SetParameters(0.0, 1.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, false);
    MRawEventIncarnations List;
    MRERawEvent* Calorimeter = AddEvent(List, Geometry, { MVector(0.0, 0.0, -10.0), MVector(1.0, 0.0, -10.0) }, MDDetector::c_Calorimeter);
    MRERawEvent* Depth = AddEvent(List, Geometry, { MVector(30.5, 0.5, 0.0), MVector(31.5, 0.5, 0.0) }, MDDetector::c_Strip3D);
    Clusterize(Clusterizer, List);
    Passed = Evaluate("Analyze()", "calorimeter distance 1.5 cm, calorimeter hits 1 cm apart", "The hits are one cluster", GetNClusters(Calorimeter), 1) && Passed;
    Passed = Evaluate("Analyze()", "calorimeter distance 1.5 cm, depth detector hits 1 cm apart", "The hits are not clustered", GetNSingleHits(Depth), 2) && Passed;
    delete Calorimeter;
    delete Depth;
  }
  {
    MERHitClusterizer Clusterizer;
    Clusterizer.SetGeometry(&Geometry);
    Clusterizer.SetParameters(0.0, 0.0, 1.5, 0.0, 0.0, 0.0, 0.0, 0.0, false);
    MRawEventIncarnations List;
    MRERawEvent* Calorimeter = AddEvent(List, Geometry, { MVector(0.0, 0.0, -10.0), MVector(1.0, 0.0, -10.0) }, MDDetector::c_Calorimeter);
    MRERawEvent* Depth = AddEvent(List, Geometry, { MVector(30.5, 0.5, 0.0), MVector(31.5, 0.5, 0.0) }, MDDetector::c_Strip3D);
    Clusterize(Clusterizer, List);
    Passed = Evaluate("Analyze()", "depth detector distance 1.5 cm, calorimeter hits 1 cm apart", "The hits are not clustered", GetNSingleHits(Calorimeter), 2) && Passed;
    Passed = Evaluate("Analyze()", "depth detector distance 1.5 cm, depth detector hits 1 cm apart", "The hits are one cluster", GetNClusters(Depth), 1) && Passed;
    delete Calorimeter;
    delete Depth;
  }

  // Hits of different detector types are not clustered: a tracker hit and a calorimeter hit 6.5 cm apart with distances of 10 cm
  {
    MERHitClusterizer Clusterizer;
    Clusterizer.SetGeometry(&Geometry);
    Clusterizer.SetParameters(10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, false);
    MRawEventIncarnations List;
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    Event->AddRESE(CreateHit(MVector(0.0, 0.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry));
    Event->AddRESE(CreateHit(MVector(0.0, 0.0, -6.5), 100.0, MDDetector::c_Calorimeter, &Geometry));
    List.AddRawEvent(Event);
    Clusterize(Clusterizer, List);
    Passed = Evaluate("Analyze()", "distances 10 cm, tracker hit and calorimeter hit 6.5 cm apart", "The hits are not clustered", GetNSingleHits(Event), 2) && Passed;
    delete Event;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the reference of the distance to a cluster
bool UTERHitClusterizer::TestDistanceReference()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry)) == false) {
    return false;
  }

  // Three hits in a row, 1 cm apart, and a distance of 1.4 cm: the first two are a cluster, whose center is at least 1.5 cm from the third hit
  // (1.5 cm for equal energies, more with the energies 100 keV and 110 keV of the first two)

  // The nearest hit of the cluster is the reference: it is 1 cm away
  {
    MERHitClusterizer Clusterizer;
    Clusterizer.SetGeometry(&Geometry);
    Clusterizer.SetParameters(1.4, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, false);
    MRawEventIncarnations List;
    MRERawEvent* Event = AddEvent(List, Geometry, { MVector(2.0, 2.0, 0.0), MVector(1.0, 2.0, 0.0), MVector(0.0, 2.0, 0.0) }, MDDetector::c_Strip2D);
    Clusterize(Clusterizer, List);
    Passed = Evaluate("Analyze()", "distance 1.4 cm to the nearest hit, three hits in a row", "There is one element", Event->GetNRESEs(), 1) && Passed;
    Passed = EvaluateTrue("Analyze()", "distance 1.4 cm to the nearest hit, three hits in a row", "All hits are one cluster", FindCluster(Event, 3) != nullptr) && Passed;
    delete Event;
  }

  // The center of the cluster is the reference
  {
    MERHitClusterizer Clusterizer;
    Clusterizer.SetGeometry(&Geometry);
    Clusterizer.SetParameters(1.4, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, true);
    MRawEventIncarnations List;
    MRERawEvent* Event = AddEvent(List, Geometry, { MVector(2.0, 2.0, 0.0), MVector(1.0, 2.0, 0.0), MVector(0.0, 2.0, 0.0) }, MDDetector::c_Strip2D);
    Clusterize(Clusterizer, List);
    Passed = Evaluate("Analyze()", "distance 1.4 cm to the center, three hits in a row", "There are two elements", Event->GetNRESEs(), 2) && Passed;
    Passed = EvaluateTrue("Analyze()", "distance 1.4 cm to the center, three hits in a row", "The first two hits are a cluster", FindCluster(Event, 2) != nullptr) && Passed;
    Passed = Evaluate("Analyze()", "distance 1.4 cm to the center, three hits in a row", "The third hit stays a hit", GetNSingleHits(Event), 1) && Passed;
    delete Event;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the merging of two clusters
bool UTERHitClusterizer::TestClusterMerging()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry)) == false) {
    return false;
  }

  // By distance: the hits at x = -4, -3 and at x = -1, 0 are clusters first, then the clusters are 2 cm apart
  {
    MERHitClusterizer Clusterizer;
    Clusterizer.SetGeometry(&Geometry);
    Clusterizer.SetParameters(2.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, false);
    MRawEventIncarnations List;
    MRERawEvent* Event = AddEvent(List, Geometry, { MVector(-4.0, 2.0, 0.0), MVector(-3.0, 2.0, 0.0), MVector(-1.0, 2.0, 0.0), MVector(0.0, 2.0, 0.0) }, MDDetector::c_Strip2D);
    Clusterize(Clusterizer, List);
    Passed = Evaluate("Analyze()", "distance 2.5 cm, two pairs 2 cm apart", "There is one element", Event->GetNRESEs(), 1) && Passed;
    Passed = EvaluateTrue("Analyze()", "distance 2.5 cm, two pairs 2 cm apart", "All four hits are one cluster", FindCluster(Event, 4) != nullptr) && Passed;
    // 100 + 110 + 120 + 130 keV
    Passed = EvaluateNear("GetEnergy()", "distance 2.5 cm, two pairs 2 cm apart", "The event has 460 keV", Event->GetEnergy(), 460.0, 1e-12) && Passed;
    if (FindCluster(Event, 4) != nullptr) {
      Passed = EvaluateNear("Analyze()", "distance 2.5 cm, two pairs 2 cm apart", "The cluster has 460 keV", FindCluster(Event, 4)->GetEnergy(), 460.0, 1e-12) && Passed;
    }
    delete Event;
  }

  // By adjacent voxels: two existing clusters in adjacent strips
  {
    MERHitClusterizer Clusterizer;
    Clusterizer.SetGeometry(&Geometry);
    Clusterizer.SetParameters(1, -1.0);
    MRawEventIncarnations List;
    MRERawEvent* Event = new MRERawEvent(&Geometry);
    MRECluster* Left = new MRECluster();
    MRECluster* Right = new MRECluster();
    BeginCaptureOutput();
    Left->AddRESE(CreateHit(MVector(-4.0, 2.0, 0.0), 100.0, MDDetector::c_Strip2D, &Geometry));
    Left->AddRESE(CreateHit(MVector(-3.0, 2.0, 0.0), 110.0, MDDetector::c_Strip2D, &Geometry));
    Right->AddRESE(CreateHit(MVector(-2.0, 2.0, 0.0), 120.0, MDDetector::c_Strip2D, &Geometry));
    Right->AddRESE(CreateHit(MVector(-1.0, 2.0, 0.0), 130.0, MDDetector::c_Strip2D, &Geometry));
    EndCaptureOutput();
    Event->AddRESE(Left);
    Event->AddRESE(Right);
    List.AddRawEvent(Event);
    Clusterize(Clusterizer, List);
    Passed = Evaluate("Analyze()", "level 1, two adjacent clusters", "There is one element", Event->GetNRESEs(), 1) && Passed;
    Passed = EvaluateTrue("Analyze()", "level 1, two adjacent clusters", "All four hits are one cluster", FindCluster(Event, 4) != nullptr) && Passed;
    Passed = EvaluateNear("GetEnergy()", "level 1, two adjacent clusters", "The event has 460 keV", Event->GetEnergy(), 460.0, 1e-12) && Passed;
    delete Event;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a list with several events
bool UTERHitClusterizer::TestSeveralEvents()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry)) == false) {
    return false;
  }
  MERHitClusterizer Clusterizer;
  Clusterizer.SetGeometry(&Geometry);
  Clusterizer.SetParameters(1, -1.0);

  // All events of the list are clusterized, each on its own: the hits at (2, 2, 0) of the first and (3, 2, 0) of the second event are adjacent
  MRawEventIncarnations List;
  MRERawEvent* First = AddEvent(List, Geometry, { MVector(1.0, 2.0, 0.0), MVector(2.0, 2.0, 0.0) }, MDDetector::c_Strip2D);
  MRERawEvent* Second = AddEvent(List, Geometry, { MVector(3.0, 2.0, 0.0), MVector(-3.0, -2.0, 0.0), MVector(-3.0, -3.0, 0.0) }, MDDetector::c_Strip2D);
  MRERawEvent* Third = AddEvent(List, Geometry, { MVector(0.0, 0.0, 0.0), MVector(2.0, 2.0, 0.0) }, MDDetector::c_Strip2D);
  Passed = EvaluateTrue("Analyze()", "three events", "The analysis succeeds", Clusterize(Clusterizer, List)) && Passed;
  Passed = Evaluate("Analyze()", "three events", "The list still has three events", List.GetNRawEvents(), 3) && Passed;
  Passed = Evaluate("Analyze()", "three events, first", "The first event has one cluster", First->GetNRESEs(), 1) && Passed;
  Passed = Evaluate("Analyze()", "three events, second", "The second event has two elements", Second->GetNRESEs(), 2) && Passed;
  Passed = EvaluateTrue("Analyze()", "three events, second", "The second event has a cluster of two hits", FindCluster(Second, 2) != nullptr) && Passed;
  Passed = Evaluate("Analyze()", "three events, third", "The third event has two hits", GetNSingleHits(Third), 2) && Passed;

  delete First;
  delete Second;
  delete Third;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the failures of reading the response files
bool UTERHitClusterizer::TestResponseFilesMissing()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry)) == false) {
    return false;
  }

  MERHitClusterizer Clusterizer;
  Clusterizer.SetGeometry(&Geometry);
  Clusterizer.SetParameters(1, -1.0);

  DisableDefaultStreams();
  bool Empty = Clusterizer.SetParameters(MString(""));
  bool Short = Clusterizer.SetParameters(MString("x"));
  bool Missing = Clusterizer.SetParameters(GetTemporaryFileName("Missing.dualseparable.yes.rsp"));
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetParameters()", "empty file name", "An empty file name is rejected", Empty) && Passed;
  Passed = EvaluateFalse("SetParameters()", "file name shorter than the suffix", "A missing file is rejected", Short) && Passed;
  Passed = EvaluateFalse("SetParameters()", "missing file", "A missing file is rejected", Missing) && Passed;

  // Only the first of the four files exists
  MString Prefix = GetTemporaryFileName("Partial");
  Passed = EvaluateTrue("WriteResponseFiles()", "response files", "The response files can be written", WriteResponseFiles(Prefix)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "all separable, no", "The last response file can be removed", RemoveTemporaryFile(Prefix + ".allseparable.no.rsp")) && Passed;
  DisableDefaultStreams();
  bool Partial = Clusterizer.SetParameters(Prefix + ".dualseparable.yes.rsp");
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetParameters()", "one of four files is missing", "An incomplete set of files is rejected", Partial) && Passed;

  // The failures do not change the algorithm: adjacent hits are still clustered
  MRawEventIncarnations List;
  MRERawEvent* Event = AddEvent(List, Geometry, { MVector(1.0, 2.0, 0.0), MVector(2.0, 2.0, 0.0) }, MDDetector::c_Strip2D);
  Clusterize(Clusterizer, List);
  Passed = Evaluate("Analyze()", "after failed response files", "The adjacent hits are one cluster", GetNClusters(Event), 1) && Passed;
  delete Event;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the clustering with the response files
bool UTERHitClusterizer::TestResponse()
{
  bool Passed = true;

  MGeometryRevan Geometry;
  if (EvaluateTrue("ScanSetupFile()", "telescope", "The telescope can be read", ScanTelescopeGeometry(Geometry)) == false) {
    return false;
  }

  MString Prefix = GetTemporaryFileName("Response");
  Passed = EvaluateTrue("WriteResponseFiles()", "response files", "The response files can be written", WriteResponseFiles(Prefix)) && Passed;
  MERHitClusterizer Clusterizer;
  Clusterizer.SetGeometry(&Geometry);
  DisableDefaultStreams();
  bool Read = Clusterizer.SetParameters(Prefix + ".dualseparable.yes.rsp");
  EnableDefaultStreams();
  if (EvaluateTrue("SetParameters()", "four response files", "The response files are read", Read) == false) {
    return false;
  }

  // The hits are first clustered with level 2 without depth cut, then a cluster is split again, if it is separable with a probability above 0.5
  MRawEventIncarnations List;
  // Two adjacent hits with a depth difference of 0.2 cm: probability 0.25
  MRERawEvent* Shallow = AddEvent(List, Geometry, { MVector(30.5, 0.5, 0.1), MVector(31.5, 1.5, 0.3) }, MDDetector::c_Strip3D);
  // Two adjacent hits with a depth difference of 1 cm: probability 0.75
  MRERawEvent* Deep = AddEvent(List, Geometry, { MVector(30.5, 0.5, -0.5), MVector(31.5, 1.5, 0.5) }, MDDetector::c_Strip3D);
  // Three adjacent hits with 100 + 110 + 120 = 330 keV: probability 0.25
  MRERawEvent* LowEnergy = AddEvent(List, Geometry, { MVector(1.0, 2.0, 0.0), MVector(2.0, 2.0, 0.0), MVector(3.0, 2.0, 0.0) }, MDDetector::c_Strip2D);
  // Three adjacent hits with 3000 keV: probability 0.75
  MRERawEvent* HighEnergy = new MRERawEvent(&Geometry);
  HighEnergy->AddRESE(CreateHit(MVector(1.0, 2.0, 0.0), 1000.0, MDDetector::c_Strip2D, &Geometry));
  HighEnergy->AddRESE(CreateHit(MVector(2.0, 2.0, 0.0), 1000.0, MDDetector::c_Strip2D, &Geometry));
  HighEnergy->AddRESE(CreateHit(MVector(3.0, 2.0, 0.0), 1000.0, MDDetector::c_Strip2D, &Geometry));
  List.AddRawEvent(HighEnergy);
  // Two hits which are not adjacent
  MRERawEvent* Apart = AddEvent(List, Geometry, { MVector(1.0, 2.0, 0.0), MVector(4.0, 2.0, 0.0) }, MDDetector::c_Strip2D);

  Passed = EvaluateTrue("Analyze()", "response files", "The analysis succeeds", Clusterize(Clusterizer, List)) && Passed;
  Passed = Evaluate("Analyze()", "response files, two hits with a depth difference of 0.2 cm", "The hits stay a cluster", GetNClusters(Shallow), 1) && Passed;
  Passed = Evaluate("Analyze()", "response files, two hits with a depth difference of 0.2 cm", "There is one element", Shallow->GetNRESEs(), 1) && Passed;
  Passed = Evaluate("Analyze()", "response files, two hits with a depth difference of 1 cm", "The cluster is split again", GetNSingleHits(Deep), 2) && Passed;
  Passed = Evaluate("Analyze()", "response files, two hits with a depth difference of 1 cm", "There are two elements", Deep->GetNRESEs(), 2) && Passed;
  Passed = EvaluateNear("GetEnergy()", "response files, two hits with a depth difference of 1 cm", "The event has 210 keV", Deep->GetEnergy(), 210.0, 1e-12) && Passed;
  Passed = EvaluateTrue("Analyze()", "response files, three hits with 330 keV", "The hits stay a cluster", FindCluster(LowEnergy, 3) != nullptr) && Passed;
  Passed = Evaluate("Analyze()", "response files, three hits with 3000 keV", "The cluster is split again", GetNSingleHits(HighEnergy), 3) && Passed;
  Passed = Evaluate("Analyze()", "response files, three hits with 3000 keV", "There are three elements", HighEnergy->GetNRESEs(), 3) && Passed;
  Passed = Evaluate("Analyze()", "response files, two hits which are not adjacent", "The hits are not clustered", GetNSingleHits(Apart), 2) && Passed;

  delete Shallow;
  delete Deep;
  delete LowEnergy;
  delete HighEnergy;
  delete Apart;

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

  UTERHitClusterizer Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERHitClusterizer.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
