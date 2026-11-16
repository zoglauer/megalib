/*
 * UTERConstruction.cxx
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
#include <new>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MERConstruction.h"
#include "MGeometryRevan.h"
#include "MRawEventIncarnations.h"
#include "MString.h"
#include "MUnitTest.h"


////////////////////////////////////////////////////////////////////////////////


//! A minimal event reconstruction task: MERConstruction is abstract, and its members are protected
class UTERConstructionTask : public MERConstruction
{
public:
  //! Default constructor
  UTERConstructionTask() : MERConstruction(), m_NModifications(0), m_ListAtModification(nullptr) {}
  //! Default destructor
  virtual ~UTERConstructionTask() {}

  //! Dump what we have done into a string
  virtual MString ToString(bool CoreOnly = false) const
  {
    if (CoreOnly == true) return "Core";
    return "Task";
  }

  //! Return the stored list
  MRawEventIncarnations* GetList() { return m_List; }
  //! Return the stored geometry
  MGeometryRevan* GetGeometry() { return m_Geometry; }
  //! Return how often the event list was modified
  unsigned int GetNModifications() const { return m_NModifications; }
  //! Return the list which was stored when the event list was modified the last time
  MRawEventIncarnations* GetListAtModification() { return m_ListAtModification; }

protected:
  //! Count the calls, and call the base class
  virtual void ModifyEventList()
  {
    ++m_NModifications;
    m_ListAtModification = m_List;
    MERConstruction::ModifyEventList();
  }

private:
  //! The number of calls of ModifyEventList
  unsigned int m_NModifications;
  //! The list at the last call of ModifyEventList
  MRawEventIncarnations* m_ListAtModification;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the base class of the event reconstruction tasks MERConstruction
class UTERConstruction : public MUnitTest
{
public:
  //! Default constructor
  UTERConstruction() : MUnitTest("UTERConstruction") {}
  //! Default destructor
  virtual ~UTERConstruction() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test that the constructor sets the pointers on memory with a pattern
  bool TestConstruction();
  //! Test setting the geometry
  bool TestSetGeometry();
  //! Test the analysis
  bool TestAnalyze();
  //! Test the default pre- and post-analysis
  bool TestPreAndPostAnalysis();
  //! Test that the destructor does not delete the list and the geometry
  bool TestOwnership();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERConstruction::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestSetGeometry() && Passed;
  Passed = TestAnalyze() && Passed;
  Passed = TestPreAndPostAnalysis() && Passed;
  Passed = TestOwnership() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the constructor sets the pointers on memory with a pattern
bool UTERConstruction::TestConstruction()
{
  bool Passed = true;

  // 0xA5 cannot be a part of a null pointer
  alignas(UTERConstructionTask) unsigned char Block[sizeof(UTERConstructionTask)];
  FillMemory(Block, sizeof(Block), 0xA5);
  UTERConstructionTask* Task = new (Block) UTERConstructionTask();
  Passed = EvaluateTrue("MERConstruction()", "memory with 0xA5", "The list is a nullptr", Task->GetList() == nullptr) && Passed;
  Passed = EvaluateTrue("MERConstruction()", "memory with 0xA5", "The geometry is a nullptr", Task->GetGeometry() == nullptr) && Passed;
  Passed = Evaluate("MERConstruction()", "memory with 0xA5", "The construction does not modify an event list", Task->GetNModifications(), 0U) && Passed;
  Task->~UTERConstructionTask();

  // The derived class decides about the string
  UTERConstructionTask Named;
  Passed = Evaluate("ToString()", "default", "The string of the derived class is returned", Named.ToString(), MString("Task")) && Passed;
  Passed = Evaluate("ToString()", "core only", "The core string of the derived class is returned", Named.ToString(true), MString("Core")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test setting the geometry
bool UTERConstruction::TestSetGeometry()
{
  bool Passed = true;

  MGeometryRevan First;
  MGeometryRevan Second;

  UTERConstructionTask Task;
  Task.SetGeometry(&First);
  Passed = EvaluateTrue("SetGeometry()", "first geometry", "The geometry is stored", Task.GetGeometry() == &First) && Passed;
  Task.SetGeometry(&Second);
  Passed = EvaluateTrue("SetGeometry()", "second geometry", "The geometry is replaced", Task.GetGeometry() == &Second) && Passed;
  Task.SetGeometry(nullptr);
  Passed = EvaluateTrue("SetGeometry()", "nullptr", "The geometry can be removed", Task.GetGeometry() == nullptr) && Passed;
  Passed = EvaluateTrue("SetGeometry()", "nullptr", "The list is not changed", Task.GetList() == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the analysis
bool UTERConstruction::TestAnalyze()
{
  bool Passed = true;

  MRawEventIncarnations First;
  MRawEventIncarnations Second;

  UTERConstructionTask Task;
  Passed = EvaluateTrue("Analyze()", "first list", "The analysis of the base class succeeds", Task.Analyze(&First)) && Passed;
  Passed = EvaluateTrue("Analyze()", "first list", "The list is stored", Task.GetList() == &First) && Passed;
  Passed = Evaluate("Analyze()", "first list", "The event list is modified once", Task.GetNModifications(), 1U) && Passed;
  Passed = EvaluateTrue("Analyze()", "first list", "The list is stored before the event list is modified", Task.GetListAtModification() == &First) && Passed;
  Passed = EvaluateTrue("Analyze()", "first list", "The geometry is not changed", Task.GetGeometry() == nullptr) && Passed;
  Passed = Evaluate("Analyze()", "first list", "The base class does not add events to the list", First.GetNRawEvents(), 0) && Passed;

  // The same object analyzes the next list
  Passed = EvaluateTrue("Analyze()", "second list", "The analysis of the base class succeeds", Task.Analyze(&Second)) && Passed;
  Passed = EvaluateTrue("Analyze()", "second list", "The list is replaced", Task.GetList() == &Second) && Passed;
  Passed = Evaluate("Analyze()", "second list", "The event list is modified a second time", Task.GetNModifications(), 2U) && Passed;
  Passed = EvaluateTrue("Analyze()", "second list", "The list is stored before the event list is modified", Task.GetListAtModification() == &Second) && Passed;

  // The same list again
  Passed = EvaluateTrue("Analyze()", "second list again", "The analysis of the base class succeeds", Task.Analyze(&Second)) && Passed;
  Passed = Evaluate("Analyze()", "second list again", "The event list is modified a third time", Task.GetNModifications(), 3U) && Passed;

  // The base class does not use the list
  Passed = EvaluateTrue("Analyze()", "nullptr", "The analysis of the base class succeeds without a list", Task.Analyze(nullptr)) && Passed;
  Passed = EvaluateTrue("Analyze()", "nullptr", "The list is a nullptr", Task.GetList() == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default pre- and post-analysis
bool UTERConstruction::TestPreAndPostAnalysis()
{
  bool Passed = true;

  UTERConstructionTask Task;
  Passed = EvaluateTrue("PreAnalysis()", "default", "The default pre-analysis succeeds", Task.PreAnalysis()) && Passed;
  Passed = EvaluateTrue("PostAnalysis()", "default", "The default post-analysis succeeds", Task.PostAnalysis()) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "second call", "The default pre-analysis succeeds again", Task.PreAnalysis()) && Passed;
  Passed = EvaluateTrue("PostAnalysis()", "second call", "The default post-analysis succeeds again", Task.PostAnalysis()) && Passed;
  Passed = Evaluate("PreAnalysis()", "default", "The pre- and post-analysis do not modify an event list", Task.GetNModifications(), 0U) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "default", "The pre- and post-analysis do not set a list", Task.GetList() == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the destructor does not delete the list and the geometry
bool UTERConstruction::TestOwnership()
{
  bool Passed = true;

  // The list and the geometry are on the stack: a delete in the destructor of the task would end the test
  MGeometryRevan Geometry;
  MRawEventIncarnations List;
  {
    UTERConstructionTask Task;
    Task.SetGeometry(&Geometry);
    Task.Analyze(&List);
  }
  Passed = EvaluateFalse("~MERConstruction()", "geometry and list on the stack", "The geometry can still be used", Geometry.IsScanned()) && Passed;
  Passed = Evaluate("~MERConstruction()", "geometry and list on the stack", "The list can still be used", List.GetNRawEvents(), 0) && Passed;

  // Two tasks share the geometry and the list
  UTERConstructionTask A;
  UTERConstructionTask B;
  A.SetGeometry(&Geometry);
  B.SetGeometry(&Geometry);
  A.Analyze(&List);
  B.Analyze(&List);
  Passed = EvaluateTrue("SetGeometry()", "two tasks", "Both tasks have the same geometry", A.GetGeometry() == B.GetGeometry()) && Passed;
  Passed = EvaluateTrue("Analyze()", "two tasks", "Both tasks have the same list", A.GetList() == B.GetList()) && Passed;
  Passed = Evaluate("Analyze()", "two tasks", "Each task counts its own modifications", A.GetNModifications() + B.GetNModifications(), 2U) && Passed;

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

  UTERConstruction Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERConstruction.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
