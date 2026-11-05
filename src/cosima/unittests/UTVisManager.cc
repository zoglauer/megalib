/*
 * UTVisManager.cc
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


// The class is only compiled if G4VIS_USE is defined, as in the cosima makefile:
#ifndef G4VIS_USE
#define G4VIS_USE
#endif

// Cosima:
#include "MCVisManager.hh"

// MEGAlib:
#include "MUnitTest.h"

// Geant4:
#include "G4VisManager.hh"

// Standard lib:
#include <iostream>
#include <sstream>
using namespace std;


//! Class exposing the protected function of MCVisManager
class UTVisManagerAccess : public MCVisManager
{
public:
  //! Make the registration public
  using MCVisManager::RegisterGraphicsSystems;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test class for MCVisManager
class UTVisManager : public MUnitTest
{
public:
  //! Default constructor
  UTVisManager() : MUnitTest("UTVisManager"), m_CoutBuffer(nullptr) {}
  //! Default destructor
  virtual ~UTVisManager() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test construction, destruction, and the singleton of the base class
  bool TestLifecycle();
  //! Test the registration of the graphics systems
  bool TestRegisterGraphicsSystems();

  //! Redirect cout - Geant4 prints its banner through it
  void SilenceOutput();
  //! Restore cout
  void RestoreOutput();

  //! The original cout buffer while silenced
  streambuf* m_CoutBuffer;
  //! The sink for the silenced output
  ostringstream m_Sink;
};


////////////////////////////////////////////////////////////////////////////////


bool UTVisManager::Run()
{
  bool Passed = true;

  Passed = TestLifecycle() && Passed;
  Passed = TestRegisterGraphicsSystems() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


void UTVisManager::SilenceOutput()
{
  DisableDefaultStreams();
  m_Sink.str("");
  m_CoutBuffer = cout.rdbuf(m_Sink.rdbuf());
}


////////////////////////////////////////////////////////////////////////////////


void UTVisManager::RestoreOutput()
{
  if (m_CoutBuffer != nullptr) cout.rdbuf(m_CoutBuffer);
  m_CoutBuffer = nullptr;
  EnableDefaultStreams();
}


////////////////////////////////////////////////////////////////////////////////


bool UTVisManager::TestLifecycle()
{
  bool Passed = true;

  // The Geant4 visualization manager is a singleton which registers itself:
  SilenceOutput();
  MCVisManager* Manager = new MCVisManager();
  G4VisManager* Instance = G4VisManager::GetInstance();
  RestoreOutput();
  Passed = EvaluateTrue("MCVisManager()", "singleton", "The Geant4 visualization manager instance is the new manager", Instance == Manager) && Passed;
  G4VisManager* Base = Manager;
  Passed = EvaluateTrue("MCVisManager()", "base class", "The manager is a Geant4 visualization manager which can be cast back", dynamic_cast<MCVisManager*>(Base) == Manager) && Passed;

  SilenceOutput();
  delete Manager;
  RestoreOutput();
  Passed = EvaluateTrue("~MCVisManager()", "delete", "The manager can be deleted", true) && Passed;

  // A new manager can be created after the first one has been deleted:
  SilenceOutput();
  MCVisManager* Second = new MCVisManager();
  G4VisManager* SecondInstance = G4VisManager::GetInstance();
  delete Second;
  RestoreOutput();
  Passed = EvaluateTrue("MCVisManager()", "second manager", "A second manager can be created after the first was deleted", SecondInstance == Second) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTVisManager::TestRegisterGraphicsSystems()
{
  bool Passed = true;

  SilenceOutput();
  UTVisManagerAccess* Manager = new UTVisManagerAccess();
  const size_t NBefore = Manager->GetAvailableGraphicsSystems().size();
  Manager->RegisterGraphicsSystems();
  // Registering twice must not crash either:
  Manager->RegisterGraphicsSystems();
  const size_t NAfter = Manager->GetAvailableGraphicsSystems().size();
  delete Manager;
  RestoreOutput();

  Passed = EvaluateTrue("RegisterGraphicsSystems()", "call", "The graphics systems can be registered", true) && Passed;
  // Each compiled-in graphics system is registered - none of the G4VIS_USE_* switches is defined in the Geant4 11 build:
  Passed = EvaluateTrue("RegisterGraphicsSystems()", "count", "Registering never removes graphics systems", NAfter >= NBefore) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTVisManager Test;
  return Test.Run() == true ? 0 : 1;
}
