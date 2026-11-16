/*
 * UTEREventClusterizer.cxx
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

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MEREventClusterizer.h"
#include "MGeometryRevan.h"
#include "MRawEventIncarnationList.h"
#include "MRawEventIncarnations.h"
#include "MUnitTest.h"


////////////////////////////////////////////////////////////////////////////////


//! Class making the abstract base class MEREventClusterizer concrete and giving access to its protected members
class UTEREventClusterizerAccess : public MEREventClusterizer
{
public:
  //! Return a fixed text - the base class has no options
  virtual MString ToString(bool CoreOnly = false) const { if (CoreOnly == true) { return "Core"; } return "All"; }

  using MERConstruction::m_Geometry;
  using MERConstruction::m_List;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the base class of the event clusterizers MEREventClusterizer of revan
class UTEREventClusterizer : public MUnitTest
{
public:
  //! Default constructor
  UTEREventClusterizer() : MUnitTest("UTEREventClusterizer") {}
  //! Default destructor
  virtual ~UTEREventClusterizer() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the state after the construction
  bool TestConstruction();
  //! Test the analysis functions, which the derived classes replace
  bool TestAnalyze();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTEREventClusterizer::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestAnalyze() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the state after the construction
bool UTEREventClusterizer::TestConstruction()
{
  bool Passed = true;

  UTEREventClusterizerAccess Clusterizer;
  Passed = EvaluateTrue("MEREventClusterizer()", "construction", "There is no geometry", Clusterizer.m_Geometry == nullptr) && Passed;
  Passed = EvaluateTrue("MEREventClusterizer()", "construction", "There is no list", Clusterizer.m_List == nullptr) && Passed;

  MGeometryRevan Geometry;
  Clusterizer.SetGeometry(&Geometry);
  Passed = EvaluateTrue("SetGeometry()", "geometry", "The geometry is stored", Clusterizer.m_Geometry == &Geometry) && Passed;
  Clusterizer.SetGeometry(nullptr);
  Passed = EvaluateTrue("SetGeometry()", "nullptr", "The geometry is removed", Clusterizer.m_Geometry == nullptr) && Passed;

  Passed = EvaluateTrue("PreAnalysis()", "default", "The pre-analysis succeeds", Clusterizer.PreAnalysis()) && Passed;
  Passed = EvaluateTrue("PostAnalysis()", "default", "The post-analysis succeeds", Clusterizer.PostAnalysis()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the analysis functions, which the derived classes replace
bool UTEREventClusterizer::TestAnalyze()
{
  bool Passed = true;

  UTEREventClusterizerAccess Clusterizer;

  // The base class has no algorithm: both analysis functions fail and do not touch their list
  MRawEventIncarnations Incarnations;
  Passed = EvaluateFalse("Analyze()", "incarnations", "The analysis of incarnations is not available", Clusterizer.Analyze(&Incarnations)) && Passed;
  Passed = EvaluateTrue("Analyze()", "incarnations", "The list is not stored", Clusterizer.m_List == nullptr) && Passed;
  Passed = Evaluate("Analyze()", "incarnations", "The incarnations stay empty", Incarnations.GetNRawEvents(), 0) && Passed;

  MRawEventIncarnationList List;
  Passed = EvaluateFalse("Analyze()", "incarnation list", "The analysis of an incarnation list is not available", Clusterizer.Analyze(&List)) && Passed;
  Passed = Evaluate("Analyze()", "incarnation list", "The list stays empty", List.Size(), 0U) && Passed;

  // A nullptr is not used
  Passed = EvaluateFalse("Analyze()", "no incarnations", "The analysis without incarnations fails", Clusterizer.Analyze(static_cast<MRawEventIncarnations*>(nullptr))) && Passed;
  Passed = EvaluateFalse("Analyze()", "no incarnation list", "The analysis without an incarnation list fails", Clusterizer.Analyze(static_cast<MRawEventIncarnationList*>(nullptr))) && Passed;

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

  UTEREventClusterizer Test;
  return Test.Run() == true ? 0 : 1;
}


// UTEREventClusterizer.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
