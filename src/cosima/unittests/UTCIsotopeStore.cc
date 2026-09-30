/*
 * UTCIsotopeStore.cc
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


// Cosima:
#include "MCIsotopeStore.hh"
#include "MCSource.hh"

// MEGAlib:
#include "MStreams.h"
#include "MString.h"
#include "MUnitTest.h"

// Geant4:
#include "G4BaryonConstructor.hh"
#include "G4BosonConstructor.hh"
#include "G4GenericIon.hh"
#include "G4IonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4NuclideTable.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"
#include "G4SystemOfUnits.hh"

// Standard lib:
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;


//! Unit test class for MCIsotopeStore
class UTCIsotopeStore : public MUnitTest
{
public:
  //! Default constructor
  UTCIsotopeStore() : MUnitTest("UTCIsotopeStore"), m_CoutBuffer(nullptr), m_CerrBuffer(nullptr) {}
  //! Default destructor
  virtual ~UTCIsotopeStore() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the default constructor
  bool TestDefaultConstruction();
  //! Test adding single isotopes and the getters
  bool TestAddAndGetters();
  //! Test reset and reuse
  bool TestReset();
  //! Test removing single entries
  bool TestRemove();
  //! Test the save and load round trip
  bool TestSaveLoadRoundTrip();
  //! Test the failure paths of save and load
  bool TestSaveLoadFailures();
  //! Test loading unusual but valid and malformed files
  bool TestLoadParsingDetails();
  //! Test scaling
  bool TestScale();
  //! Test adding another store
  bool TestAddStore();
  //! Test sorting
  bool TestSort();
  //! Test removing stable elements
  bool TestRemoveStableElements();
  //! Test the particle definitions
  bool TestParticleDefinitions();
  //! Test the source list creation
  bool TestSourceLists();
  //! Test the stream operator
  bool TestStream();

  //! Initialize the Geant4 particles and the nuclide table as done in cosima
  void InitializeGeant4();
  //! Redirect cout, cerr and the MEGAlib streams - the store prints a lot
  void SilenceOutput();
  //! Restore cout, cerr and the MEGAlib streams
  void RestoreOutput();
  //! Save a store silently
  bool SaveSilently(MCIsotopeStore& Store, const MString& FileName);
  //! Load a store silently
  bool LoadSilently(MCIsotopeStore& Store, const MString& FileName);
  //! Return the value of the given entry, or -1 if it is absent
  double Find(const MCIsotopeStore& Store, const MString& Volume, int ID, double Excitation, double Tolerance = 1e-9*keV);
  //! Return the number of entries
  unsigned int Count(const MCIsotopeStore& Store);

  //! The original cout buffer while silenced
  streambuf* m_CoutBuffer;
  //! The original cerr buffer while silenced
  streambuf* m_CerrBuffer;
  //! The sink for the silenced output
  ostringstream m_Sink;
};


////////////////////////////////////////////////////////////////////////////////


bool UTCIsotopeStore::Run()
{
  bool Passed = true;

  InitializeGeant4();

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestAddAndGetters() && Passed;
  Passed = TestReset() && Passed;
  Passed = TestRemove() && Passed;
  Passed = TestSaveLoadRoundTrip() && Passed;
  Passed = TestSaveLoadFailures() && Passed;
  Passed = TestLoadParsingDetails() && Passed;
  Passed = TestScale() && Passed;
  Passed = TestAddStore() && Passed;
  Passed = TestSort() && Passed;
  Passed = TestRemoveStableElements() && Passed;
  Passed = TestParticleDefinitions() && Passed;
  Passed = TestSourceLists() && Passed;
  Passed = TestStream() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


void UTCIsotopeStore::InitializeGeant4()
{
  SilenceOutput();

  G4BosonConstructor().ConstructParticle();
  G4LeptonConstructor().ConstructParticle();
  G4BaryonConstructor().ConstructParticle();
  G4IonConstructor().ConstructParticle();

  // The generic ion needs a process manager before ions can be created:
  G4ParticleDefinition* GenericIon = G4GenericIon::Definition();
  GenericIon->SetProcessManager(new G4ProcessManager(GenericIon));
  G4ParticleTable::GetParticleTable()->SetReadiness();

  // Same settings as in the cosima physics list:
  G4NuclideTable::GetInstance()->SetLevelTolerance(100*eV);
  G4NuclideTable::GetInstance()->SetThresholdOfHalfLife(0.0001*ns);

  RestoreOutput();
}


////////////////////////////////////////////////////////////////////////////////


void UTCIsotopeStore::SilenceOutput()
{
  DisableDefaultStreams();
  mimp.Enable(false);
  m_Sink.str("");
  m_CoutBuffer = cout.rdbuf(m_Sink.rdbuf());
  m_CerrBuffer = cerr.rdbuf(m_Sink.rdbuf());
}


////////////////////////////////////////////////////////////////////////////////


void UTCIsotopeStore::RestoreOutput()
{
  if (m_CoutBuffer != nullptr) cout.rdbuf(m_CoutBuffer);
  if (m_CerrBuffer != nullptr) cerr.rdbuf(m_CerrBuffer);
  m_CoutBuffer = nullptr;
  m_CerrBuffer = nullptr;
  mimp.Enable(true);
  EnableDefaultStreams();
}


////////////////////////////////////////////////////////////////////////////////


bool UTCIsotopeStore::SaveSilently(MCIsotopeStore& Store, const MString& FileName)
{
  SilenceOutput();
  bool OK = Store.Save(FileName);
  RestoreOutput();
  return OK;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCIsotopeStore::LoadSilently(MCIsotopeStore& Store, const MString& FileName)
{
  SilenceOutput();
  bool OK = Store.Load(FileName);
  RestoreOutput();
  return OK;
}


////////////////////////////////////////////////////////////////////////////////


double UTCIsotopeStore::Find(const MCIsotopeStore& Store, const MString& Volume, int ID, double Excitation, double Tolerance)
{
  for (unsigned int v = 0; v < Store.GetNVolumes(); ++v) {
    if (Store.GetVolume(v) != Volume) continue;
    for (unsigned int i = 0; i < Store.GetNIDs(v); ++i) {
      if (Store.GetID(v, i) != ID) continue;
      for (unsigned int e = 0; e < Store.GetNExcitations(v, i); ++e) {
        if (fabs(Store.GetExcitation(v, i, e) - Excitation) <= Tolerance) return Store.GetValue(v, i, e);
      }
    }
  }
  return -1;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int UTCIsotopeStore::Count(const MCIsotopeStore& Store)
{
  unsigned int N = 0;
  for (unsigned int v = 0; v < Store.GetNVolumes(); ++v) {
    for (unsigned int i = 0; i < Store.GetNIDs(v); ++i) {
      N += Store.GetNExcitations(v, i);
    }
  }
  return N;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCIsotopeStore::TestDefaultConstruction()
{
  bool Passed = true;

  MCIsotopeStore Store;
  Passed = Evaluate("MCIsotopeStore()", "default", "A new store has no volumes", Store.GetNVolumes(), 0U) && Passed;
  Passed = EvaluateNear("MCIsotopeStore()", "default", "A new store has no time", Store.GetTime(), 0.0, 1e-12) && Passed;

  Store.SetTime(12.5*s);
  Passed = EvaluateNear("SetTime/GetTime", "12.5 s", "GetTime returns the time set before", Store.GetTime()/s, 12.5, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCIsotopeStore::TestAddAndGetters()
{
  bool Passed = true;

  MCIsotopeStore Store;
  Store.Add("Crystal", 32077, 159.71*keV, 3.0);
  Passed = Evaluate("Add()", "first isotope", "The first isotope creates a volume", Store.GetNVolumes(), 1U) && Passed;
  Passed = Evaluate("GetVolume()", "first isotope", "The volume name is stored", Store.GetVolume(0), MString("Crystal")) && Passed;
  Passed = Evaluate("GetNIDs()", "first isotope", "The volume has one isotope", Store.GetNIDs(0), 1U) && Passed;
  Passed = Evaluate("GetID()", "first isotope", "The isotope ID is stored", Store.GetID(0, 0), 32077) && Passed;
  Passed = Evaluate("GetNExcitations()", "first isotope", "The isotope has one excitation", Store.GetNExcitations(0, 0), 1U) && Passed;
  Passed = EvaluateNear("GetExcitation()", "first isotope", "The excitation is stored", Store.GetExcitation(0, 0, 0)/keV, 159.71, 1e-9) && Passed;
  Passed = EvaluateNear("GetValue()", "first isotope", "The value is stored", Store.GetValue(0, 0, 0), 3.0, 1e-12) && Passed;

  // Same entry again - the values are added:
  Store.Add("Crystal", 32077, 159.71*keV, 2.0);
  Passed = Evaluate("Add()", "same entry", "The same entry does not create a new one", Count(Store), 1U) && Passed;
  Passed = EvaluateNear("Add()", "same entry", "The values of the same entry are added", Store.GetValue(0, 0, 0), 5.0, 1e-12) && Passed;

  // Almost the same excitation (below the 0.01 keV file precision) is the same level:
  Store.Add("Crystal", 32077, 159.712*keV, 1.0);
  Passed = Evaluate("Add()", "near-equal excitation", "An excitation 0.002 keV off does not create a new entry", Count(Store), 1U) && Passed;
  Passed = EvaluateNear("Add()", "near-equal excitation", "The value of the near-equal excitation is added", Store.GetValue(0, 0, 0), 6.0, 1e-12) && Passed;

  // Boundary of the level matching - clearly inside and outside, not exactly at 0.005 keV where the floating point representation decides:
  {
    MCIsotopeStore Boundary;
    Boundary.Add("Boundary", 11024, 100.000*keV, 1.0);
    Boundary.Add("Boundary", 11024, 100.004*keV, 1.0);
    Boundary.Add("Boundary", 11024, 100.010*keV, 1.0);
    Passed = Evaluate("Add()", "level boundary", "0.004 keV apart is the same level, 0.01 keV apart is a neighbor level", Count(Boundary), 2U) && Passed;
    Passed = EvaluateNear("Add()", "level boundary", "The values of the same level are added", Find(Boundary, "Boundary", 11024, 100.000*keV), 2.0, 1e-12) && Passed;
    Passed = EvaluateNear("Add()", "level boundary", "The neighbor level keeps its own value", Find(Boundary, "Boundary", 11024, 100.010*keV), 1.0, 1e-12) && Passed;
  }

  // Default value:
  Store.Add("Crystal", 32077, 0.0);
  Passed = EvaluateNear("Add()", "default value", "The default value is one", Find(Store, "Crystal", 32077, 0.0), 1.0, 1e-12) && Passed;

  // New excitation, new ID, new volume:
  Store.Add("Crystal", 11024, 0.0, 7.0);
  Store.Add("Shield", 11024, 0.0, 4.0);
  Passed = Evaluate("Add()", "new ID and volume", "All entries are stored separately", Count(Store), 4U) && Passed;
  Passed = Evaluate("GetNVolumes()", "new volume", "A new volume is created", Store.GetNVolumes(), 2U) && Passed;
  Passed = Evaluate("GetNExcitations()", "two excitations", "Ge77 has two excitations", Store.GetNExcitations(0, 0), 2U) && Passed;
  Passed = EvaluateNear("Add()", "new volume", "The value in the new volume is stored", Find(Store, "Shield", 11024, 0.0), 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("Add()", "new ID", "The value of the new ID is stored", Find(Store, "Crystal", 11024, 0.0), 7.0, 1e-12) && Passed;

  // Zero and negative values are stored as they are:
  Store.Add("Crystal", 27060, 0.0, 0.0);
  Store.Add("Crystal", 27060, 58.59*keV, -2.0);
  Passed = EvaluateNear("Add()", "zero value", "A zero value is stored", Find(Store, "Crystal", 27060, 0.0), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Add()", "negative value", "A negative value is stored", Find(Store, "Crystal", 27060, 58.59*keV), -2.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCIsotopeStore::TestReset()
{
  bool Passed = true;

  MCIsotopeStore Store;
  Store.SetTime(10*s);
  Store.Add("Crystal", 11024, 0.0, 7.0);
  Store.Reset();
  Passed = Evaluate("Reset()", "filled store", "Reset removes all volumes", Store.GetNVolumes(), 0U) && Passed;
  Passed = EvaluateNear("Reset()", "filled store", "Reset removes the time", Store.GetTime(), 0.0, 1e-12) && Passed;

  // Reuse:
  Store.Add("Shield", 27060, 0.0, 2.0);
  Passed = Evaluate("Reset()", "reuse", "The store can be reused after a reset", Count(Store), 1U) && Passed;
  Passed = EvaluateNear("Reset()", "reuse", "The reused store holds only the new value", Find(Store, "Shield", 27060, 0.0), 2.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCIsotopeStore::TestRemove()
{
  bool Passed = true;

  MCIsotopeStore Store;
  Store.Add("Crystal", 32077, 0.0, 1.0);
  Store.Add("Crystal", 32077, 159.71*keV, 2.0);
  Store.Add("Crystal", 32077, 300.0*keV, 3.0);

  // Remove the middle and then the last entry:
  SilenceOutput();
  Store.Remove(0, 0, 1);
  RestoreOutput();
  Passed = Evaluate("Remove()", "middle entry", "One entry is removed", Store.GetNExcitations(0, 0), 2U) && Passed;
  Passed = EvaluateNear("Remove()", "middle entry", "The removed entry is gone", Find(Store, "Crystal", 32077, 159.71*keV), -1.0, 1e-12) && Passed;
  Passed = EvaluateNear("Remove()", "middle entry", "The other entries are kept", Find(Store, "Crystal", 32077, 300.0*keV), 3.0, 1e-12) && Passed;

  SilenceOutput();
  Store.Remove(0, 0, 1);
  RestoreOutput();
  Passed = Evaluate("Remove()", "last entry", "The last entry is removed", Store.GetNExcitations(0, 0), 1U) && Passed;
  Passed = EvaluateNear("Remove()", "last entry", "The first entry is kept", Find(Store, "Crystal", 32077, 0.0), 1.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCIsotopeStore::TestSaveLoadRoundTrip()
{
  bool Passed = true;

  MCIsotopeStore Store;
  Store.SetTime(123.5*s);
  Store.Add("Crystal", 32077, 0.0, 1.5e3);
  Store.Add("Crystal", 32077, 159.71*keV, 2.25);
  Store.Add("Crystal", 11024, 472.2*keV, 3.125e-7);
  Store.Add("Shield", 27060, 58.59*keV, 4.0e12);

  const MString FileName = GetTemporaryFileName("RoundTrip.dat");
  Passed = EvaluateTrue("Save()", "round trip", "The store can be saved", SaveSilently(Store, FileName)) && Passed;

  MCIsotopeStore Loaded;
  Passed = EvaluateTrue("Load()", "round trip", "The saved store can be loaded", LoadSilently(Loaded, FileName)) && Passed;
  Passed = EvaluateNear("Load()", "round trip", "The time survives the round trip", Loaded.GetTime()/s, 123.5, 1e-4) && Passed;
  Passed = Evaluate("Load()", "round trip", "All entries survive the round trip", Count(Loaded), 4U) && Passed;
  // Excitations are saved with 0.01 keV precision, values with 6 significant digits:
  Passed = EvaluateNear("Load()", "Ge77", "The Ge77 value survives", Find(Loaded, "Crystal", 32077, 0.0, 0.005*keV), 1.5e3, 1e-5*1.5e3) && Passed;
  Passed = EvaluateNear("Load()", "Ge77m", "The Ge77m value survives", Find(Loaded, "Crystal", 32077, 159.71*keV, 0.005*keV), 2.25, 1e-5*2.25) && Passed;
  Passed = EvaluateNear("Load()", "Na24m", "A small value survives", Find(Loaded, "Crystal", 11024, 472.2*keV, 0.005*keV), 3.125e-7, 1e-5*3.125e-7) && Passed;
  Passed = EvaluateNear("Load()", "Co60m", "A large value in a second volume survives", Find(Loaded, "Shield", 27060, 58.59*keV, 0.005*keV), 4.0e12, 1e-5*4.0e12) && Passed;

  // Saving the loaded store again gives the same file:
  const MString FileName2 = GetTemporaryFileName("RoundTrip2.dat");
  Passed = EvaluateTrue("Save()", "second round trip", "The loaded store can be saved", SaveSilently(Loaded, FileName2)) && Passed;
  Passed = EvaluateFilesIdentical("Save()", "second round trip", "Saving a loaded store reproduces the file", FileName2, FileName) && Passed;

  // Adding to a loaded entry uses the same entry:
  Loaded.Add("Crystal", 32077, 159.71*keV, 1.0);
  Passed = Evaluate("Add()", "after load", "Adding the same isotope after loading does not create a duplicate", Loaded.GetNExcitations(0, 1), 2U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCIsotopeStore::TestSaveLoadFailures()
{
  bool Passed = true;

  MCIsotopeStore Store;
  Store.Add("Crystal", 11024, 0.0, 1.0);
  Passed = EvaluateFalse("Save()", "missing directory", "Saving into a missing directory fails", SaveSilently(Store, GetTemporaryFileName("Missing") + "/Store.dat")) && Passed;

  // A file which exists but cannot be read:
  {
    const MString FileName = GetTemporaryFileName("Unreadable.dat");
    Passed = EvaluateTrue("WriteTextFile()", "unreadable", "The fixture can be written", WriteTextFile(FileName, "TT 1\nVN Crystal\nRP 11024 0.0 1.0\nEN\n")) && Passed;
    std::error_code Error;
    std::filesystem::permissions(FileName.Data(), std::filesystem::perms::none, Error);
    ifstream Probe(FileName.Data());
    // Root can read everything - only test if the file is really unreadable:
    if (Error.value() == 0 && Probe.is_open() == false) {
      MCIsotopeStore Unreadable;
      Passed = EvaluateFalse("Load()", "unreadable file", "Loading an existing but unreadable file fails", LoadSilently(Unreadable, FileName)) && Passed;
    }
    Probe.close();
    std::filesystem::permissions(FileName.Data(), std::filesystem::perms::owner_read | std::filesystem::perms::owner_write, Error);
  }

  // A failed load leaves no stale content behind:
  Passed = EvaluateFalse("Load()", "missing file", "Loading a missing file fails", LoadSilently(Store, GetTemporaryFileName("Missing.dat"))) && Passed;
  Passed = Evaluate("Load()", "missing file", "A failed load leaves no stale entries", Count(Store), 0U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCIsotopeStore::TestLoadParsingDetails()
{
  bool Passed = true;

  // Loading replaces the previous content, including the time:
  {
    const MString FileName = GetTemporaryFileName("NoTime.dat");
    Passed = EvaluateTrue("WriteTextFile()", "no time", "The fixture can be written", WriteTextFile(FileName, "VN Crystal\nRP 11024 0.0 2.0\nEN\n")) && Passed;
    MCIsotopeStore Store;
    Store.SetTime(50*s);
    Store.Add("Shield", 27060, 0.0, 9.0);
    Passed = EvaluateTrue("Load()", "no time", "A file without time can be loaded", LoadSilently(Store, FileName)) && Passed;
    Passed = Evaluate("Load()", "reuse", "Loading replaces the previous entries", Count(Store), 1U) && Passed;
    Passed = EvaluateNear("Load()", "reuse", "Loading a file without time does not keep the previous time", Store.GetTime(), 0.0, 1e-12) && Passed;
  }

  // IDs which are not consecutive in the file:
  {
    const MString FileName = GetTemporaryFileName("Unsorted.dat");
    Passed = EvaluateTrue("WriteTextFile()", "unsorted", "The fixture can be written", WriteTextFile(FileName, "TT 1\nVN Crystal\nRP 11024 0.0 1.0\nRP 27060 0.0 2.0\nRP 11024 472.2 3.0\nEN\n")) && Passed;
    MCIsotopeStore Store;
    Passed = EvaluateTrue("Load()", "unsorted", "A file with non-consecutive IDs can be loaded", LoadSilently(Store, FileName)) && Passed;
    Passed = EvaluateNear("Load()", "unsorted", "The second Na24 entry belongs to Na24", Find(Store, "Crystal", 11024, 472.2*keV, 0.005*keV), 3.0, 1e-9) && Passed;
    Passed = EvaluateNear("Load()", "unsorted", "Co60 keeps only its own entry", Find(Store, "Crystal", 27060, 472.2*keV, 0.005*keV), -1.0, 1e-9) && Passed;
  }

  // The same entry twice is added up:
  {
    const MString FileName = GetTemporaryFileName("Duplicate.dat");
    Passed = EvaluateTrue("WriteTextFile()", "duplicate", "The fixture can be written", WriteTextFile(FileName, "TT 1\nVN Crystal\nRP 11024 0.0 1.0\nRP 11024 0.0 2.0\nEN\n")) && Passed;
    MCIsotopeStore Store;
    Passed = EvaluateTrue("Load()", "duplicate", "A file with a duplicate entry can be loaded", LoadSilently(Store, FileName)) && Passed;
    Passed = Evaluate("Load()", "duplicate", "A duplicate entry does not create a second entry", Count(Store), 1U) && Passed;
    Passed = EvaluateNear("Load()", "duplicate", "The values of a duplicate entry are added", Find(Store, "Crystal", 11024, 0.0), 3.0, 1e-9) && Passed;
  }

  // The same level with slightly different excitations (below the 0.01 keV file precision) is added up:
  {
    const MString FileName = GetTemporaryFileName("NearDuplicate.dat");
    Passed = EvaluateTrue("WriteTextFile()", "near duplicate", "The fixture can be written", WriteTextFile(FileName, "TT 1\nVN Crystal\nRP 11024 472.200 1.0\nRP 11024 472.204 2.0\nRP 11024 472.300 4.0\nEN\n")) && Passed;
    MCIsotopeStore Store;
    Passed = EvaluateTrue("Load()", "near duplicate", "A file with near-equal excitations can be loaded", LoadSilently(Store, FileName)) && Passed;
    Passed = Evaluate("Load()", "near duplicate", "Near-equal excitations are one level, a 0.1 keV difference is another", Count(Store), 2U) && Passed;
    Passed = EvaluateNear("Load()", "near duplicate", "The values of near-equal excitations are added", Find(Store, "Crystal", 11024, 472.2*keV, 0.005*keV), 3.0, 1e-9) && Passed;
  }

  // Repeated volume blocks are merged:
  {
    const MString FileName = GetTemporaryFileName("RepeatedVolumes.dat");
    Passed = EvaluateTrue("WriteTextFile()", "repeated volumes", "The fixture can be written", WriteTextFile(FileName, "TT 1\nVN Crystal\nRP 11024 0.0 1.0\nVN Shield\nRP 27060 0.0 2.0\nVN Crystal\nRP 11024 0.0 3.0\nRP 32077 0.0 4.0\nEN\n")) && Passed;
    MCIsotopeStore Store;
    Passed = EvaluateTrue("Load()", "repeated volumes", "A file with repeated volume blocks can be loaded", LoadSilently(Store, FileName)) && Passed;
    Passed = Evaluate("Load()", "repeated volumes", "A repeated volume block does not create a second volume", Store.GetNVolumes(), 2U) && Passed;
    Passed = EvaluateNear("Load()", "repeated volumes", "The Na24 values of both blocks are added", Find(Store, "Crystal", 11024, 0.0), 4.0, 1e-9) && Passed;
    Passed = EvaluateNear("Load()", "repeated volumes", "The second block adds new isotopes to the first", Find(Store, "Crystal", 32077, 0.0), 4.0, 1e-9) && Passed;
    Passed = EvaluateNear("Load()", "repeated volumes", "The other volume is kept", Find(Store, "Shield", 27060, 0.0), 2.0, 1e-9) && Passed;
  }

  // Malformed lines are ignored:
  {
    const MString FileName = GetTemporaryFileName("Malformed.dat");
    Passed = EvaluateTrue("WriteTextFile()", "malformed", "The fixture can be written", WriteTextFile(FileName, "TT 1\nRP 11024 0.0 5.0\nVN Crystal\nRP 11024\nRP 11024 0.0\nXX 1 2 3\nRP 27060 0.0 2.0\nEN\n")) && Passed;
    MCIsotopeStore Store;
    Passed = EvaluateTrue("Load()", "malformed", "A file with malformed lines can be loaded", LoadSilently(Store, FileName)) && Passed;
    Passed = Evaluate("Load()", "malformed", "Only the valid entry is loaded", Count(Store), 1U) && Passed;
    Passed = EvaluateNear("Load()", "malformed", "The valid entry is loaded", Find(Store, "Crystal", 27060, 0.0), 2.0, 1e-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCIsotopeStore::TestScale()
{
  bool Passed = true;

  MCIsotopeStore Store;
  Store.Add("Crystal", 32077, 0.0, 2.0);
  Store.Add("Shield", 27060, 58.59*keV, 8.0);
  Store.Scale(0.25);
  Passed = EvaluateNear("Scale()", "0.25", "The first value is scaled", Find(Store, "Crystal", 32077, 0.0), 0.5, 1e-12) && Passed;
  Passed = EvaluateNear("Scale()", "0.25", "The value in the second volume is scaled", Find(Store, "Shield", 27060, 58.59*keV), 2.0, 1e-12) && Passed;
  Store.Scale(0.0);
  Passed = EvaluateNear("Scale()", "zero", "Scaling with zero gives zero", Find(Store, "Crystal", 32077, 0.0), 0.0, 1e-12) && Passed;

  MCIsotopeStore Empty;
  Empty.Scale(3.0);
  Passed = Evaluate("Scale()", "empty store", "Scaling an empty store keeps it empty", Empty.GetNVolumes(), 0U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCIsotopeStore::TestAddStore()
{
  bool Passed = true;

  MCIsotopeStore A;
  A.SetTime(10*s);
  A.Add("Crystal", 32077, 0.0, 1.0);
  A.Add("Crystal", 31073, 0.0, 5.0);

  MCIsotopeStore B;
  B.SetTime(30*s);
  B.Add("Crystal", 32077, 0.0, 2.0);        // same entry
  B.Add("Crystal", 32077, 159.71*keV, 3.0); // same ID, new excitation
  B.Add("Crystal", 11024, 0.0, 4.0);        // new ID
  B.Add("Shield", 27060, 0.0, 6.0);         // new volume
  B.Add("Crystal", 31073, 0.3*keV, 7.0);    // Ga73[0.3 keV] - a different level 0.3 keV above the ground state

  A.Add(B);
  Passed = EvaluateNear("Add(MCIsotopeStore)", "times", "The times are added", A.GetTime()/s, 40.0, 1e-12) && Passed;
  Passed = EvaluateNear("Add(MCIsotopeStore)", "same entry", "The values of the same entry are added", Find(A, "Crystal", 32077, 0.0), 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("Add(MCIsotopeStore)", "new excitation", "A new excitation is added", Find(A, "Crystal", 32077, 159.71*keV), 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("Add(MCIsotopeStore)", "new ID", "A new ID is added", Find(A, "Crystal", 11024, 0.0), 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("Add(MCIsotopeStore)", "new volume", "A new volume is added", Find(A, "Shield", 27060, 0.0), 6.0, 1e-12) && Passed;
  Passed = EvaluateNear("Add(MCIsotopeStore)", "close levels", "The Ga73 ground state is not merged with Ga73[0.3 keV]", Find(A, "Crystal", 31073, 0.0), 5.0, 1e-12) && Passed;
  Passed = EvaluateNear("Add(MCIsotopeStore)", "close levels", "Ga73[0.3 keV] is kept as its own level", Find(A, "Crystal", 31073, 0.3*keV), 7.0, 1e-12) && Passed;
  Passed = Evaluate("Add(MCIsotopeStore)", "total", "All distinct entries are present", Count(A), 6U) && Passed;

  // Near-equal levels across stores are merged, neighbor levels at the file precision are not:
  {
    MCIsotopeStore C;
    C.Add("Crystal", 11024, 472.200*keV, 1.0);
    MCIsotopeStore D;
    D.Add("Crystal", 11024, 472.204*keV, 2.0);
    D.Add("Crystal", 11024, 472.210*keV, 5.0);
    C.Add(D);
    Passed = Evaluate("Add(MCIsotopeStore)", "near-equal levels", "Near-equal levels are merged, neighbor levels are not", Count(C), 2U) && Passed;
    Passed = EvaluateNear("Add(MCIsotopeStore)", "near-equal levels", "The values of near-equal levels are added", Find(C, "Crystal", 11024, 472.200*keV), 3.0, 1e-12) && Passed;
    Passed = EvaluateNear("Add(MCIsotopeStore)", "near-equal levels", "The neighbor level keeps its own value", Find(C, "Crystal", 11024, 472.210*keV), 5.0, 1e-12) && Passed;
  }

  // The added store is not modified:
  Passed = Evaluate("Add(MCIsotopeStore)", "source store", "The added store keeps its entries", Count(B), 5U) && Passed;

  // Adding to an empty store copies everything:
  MCIsotopeStore Empty;
  Empty.Add(B);
  Passed = Evaluate("Add(MCIsotopeStore)", "empty target", "Adding to an empty store copies all entries", Count(Empty), 5U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCIsotopeStore::TestSort()
{
  bool Passed = true;

  // Volumes, IDs, and excitations in reverse order:
  MCIsotopeStore Store;
  vector<MString> Volumes = { "E", "D", "C", "B", "A" };
  vector<int> IDs = { 50000, 40000, 30000, 20000, 10000 };
  vector<double> Excitations = { 1*keV, 2*keV, 3*keV, 4*keV, 5*keV };
  for (unsigned int v = 0; v < Volumes.size(); ++v) {
    for (unsigned int i = 0; i < IDs.size(); ++i) {
      for (unsigned int e = 0; e < Excitations.size(); ++e) {
        Store.Add(Volumes[v], IDs[i] + v, Excitations[e], 100*v + 10*i + e);
      }
    }
  }

  SilenceOutput();
  Store.Sort();
  RestoreOutput();

  bool VolumesSorted = true;
  bool IDsSorted = true;
  bool ExcitationsSorted = true;
  bool ValuesKept = true;
  for (unsigned int v = 0; v < Store.GetNVolumes(); ++v) {
    if (v > 0 && !(Store.GetVolume(v-1) < Store.GetVolume(v))) VolumesSorted = false;
    for (unsigned int i = 0; i < Store.GetNIDs(v); ++i) {
      if (i > 0 && Store.GetID(v, i-1) >= Store.GetID(v, i)) IDsSorted = false;
      for (unsigned int e = 0; e < Store.GetNExcitations(v, i); ++e) {
        if (e > 0 && Store.GetExcitation(v, i, e-1) <= Store.GetExcitation(v, i, e)) ExcitationsSorted = false;
        // The value encodes its original position:
        unsigned int OrigV = 4 - v;
        unsigned int OrigI = (unsigned int) ((50000 - (Store.GetID(v, i) - OrigV))/10000);
        unsigned int OrigE = (unsigned int) (Store.GetExcitation(v, i, e)/keV + 0.5) - 1;
        if (fabs(Store.GetValue(v, i, e) - (100*OrigV + 10*OrigI + OrigE)) > 1e-9) ValuesKept = false;
      }
    }
  }
  Passed = Evaluate("Sort()", "reverse order", "All entries are kept", Count(Store), 125U) && Passed;
  Passed = EvaluateTrue("Sort()", "reverse order", "The volumes are sorted alphabetically", VolumesSorted) && Passed;
  Passed = EvaluateTrue("Sort()", "reverse order", "The IDs are sorted ascending", IDsSorted) && Passed;
  Passed = EvaluateTrue("Sort()", "reverse order", "The excitations are sorted descending", ExcitationsSorted) && Passed;
  Passed = EvaluateTrue("Sort()", "reverse order", "Every value stays with its volume, ID, and excitation", ValuesKept) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCIsotopeStore::TestRemoveStableElements()
{
  bool Passed = true;

  MCIsotopeStore Store;
  Store.Add("Crystal", 32077, 0.0, 1.0);   // radioactive
  Store.Add("Crystal", 8016, 0.0, 3.0);    // stable
  Store.Add("Crystal", 1004, 0.0, 4.0);    // unknown to Geant4
  Store.Add("Crystal", 27060, 58.59*keV, 5.0); // isomer

  SilenceOutput();
  Store.RemoveStableElements();
  RestoreOutput();
  Passed = EvaluateNear("RemoveStableElements()", "Ge77", "Radioactive isotopes are kept", Find(Store, "Crystal", 32077, 0.0), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("RemoveStableElements()", "Co60m", "Isomers are kept", Find(Store, "Crystal", 27060, 58.59*keV), 5.0, 1e-12) && Passed;
  Passed = EvaluateNear("RemoveStableElements()", "O16", "Stable isotopes are removed", Find(Store, "Crystal", 8016, 0.0), -1.0, 1e-12) && Passed;
  Passed = EvaluateNear("RemoveStableElements()", "H4", "Isotopes unknown to Geant4 are removed", Find(Store, "Crystal", 1004, 0.0), -1.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCIsotopeStore::TestParticleDefinitions()
{
  bool Passed = true;

  SilenceOutput();
  G4ParticleDefinition* Na24 = MCIsotopeStore::GetParticleDefinition(11024, 0.0);
  G4ParticleDefinition* Ge77m = MCIsotopeStore::GetParticleDefinition(32077, 159.71*keV);
  G4ParticleDefinition* H4 = MCIsotopeStore::GetParticleDefinition(1004, 0.0);
  RestoreOutput();
  Passed = EvaluateTrue("GetParticleDefinition()", "Na24", "A ground state is found", Na24 != nullptr) && Passed;
  if (Na24 != nullptr) {
    Passed = Evaluate("GetParticleDefinition()", "Na24", "The ground state has the right name", MString(Na24->GetParticleName()), MString("Na24")) && Passed;
  }
  Passed = EvaluateTrue("GetParticleDefinition()", "Ge77m", "An isomer is found", Ge77m != nullptr) && Passed;
  Passed = EvaluateTrue("GetParticleDefinition()", "H4", "An isotope unknown to Geant4 returns nullptr", H4 == nullptr) && Passed;

  MCIsotopeStore Store;
  Store.Add("Crystal", 32077, 159.71*keV, 1.0);
  SilenceOutput();
  G4ParticleDefinition* Entry = Store.GetParticleDefinition(0, 0, 0);
  RestoreOutput();
  Passed = EvaluateTrue("GetParticleDefinition(v, i, e)", "Ge77m", "The definition of a stored entry is the one of its ID and excitation", Entry == Ge77m) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCIsotopeStore::TestSourceLists()
{
  bool Passed = true;

  MCIsotopeStore Store;
  Store.Add("Crystal", 32077, 159.71*keV, 2.5);
  Store.Add("Crystal", 11024, 0.0, 0.0);   // no activity - no source
  Store.Add("Shield", 27060, 0.0, 1.5e3);

  SilenceOutput();
  vector<MCSource*> ByActivity = Store.CreateSourceListByActivity();
  RestoreOutput();
  Passed = EvaluateSize("CreateSourceListByActivity()", "three entries, one zero", "Only entries with a positive value create sources", ByActivity.size(), 2) && Passed;
  bool FoundGe = false;
  bool FoundCo = false;
  for (MCSource* S: ByActivity) {
    if (S->GetParticleType() == 32077) {
      FoundGe = true;
      Passed = EvaluateNear("CreateSourceListByActivity()", "Ge77m", "The flux is the activity in Bq", S->GetFlux()*s, 2.5, 1e-9) && Passed;
      Passed = EvaluateNear("CreateSourceListByActivity()", "Ge77m", "The excitation is set", S->GetParticleExcitation()/keV, 159.71, 1e-9) && Passed;
      Passed = Evaluate("CreateSourceListByActivity()", "Ge77m", "The volume is set", S->GetVolume(), MString("Crystal")) && Passed;
      Passed = Evaluate("CreateSourceListByActivity()", "Ge77m", "The spectral type is activation", S->GetSpectralType(), (int) MCSource::c_Activation) && Passed;
    }
    if (S->GetParticleType() == 27060) {
      FoundCo = true;
      Passed = EvaluateNear("CreateSourceListByActivity()", "Co60", "The flux of the second volume is set", S->GetFlux()*s, 1.5e3, 1e-9) && Passed;
      Passed = Evaluate("CreateSourceListByActivity()", "Co60", "The volume of the second source is set", S->GetVolume(), MString("Shield")) && Passed;
    }
    delete S;
  }
  Passed = EvaluateTrue("CreateSourceListByActivity()", "Ge77m", "A source for Ge77m exists", FoundGe) && Passed;
  Passed = EvaluateTrue("CreateSourceListByActivity()", "Co60", "A source for Co60 exists", FoundCo) && Passed;

  SilenceOutput();
  vector<MCSource*> ByCount = Store.CreateSourceListByIsotopeCount();
  RestoreOutput();
  Passed = EvaluateSize("CreateSourceListByIsotopeCount()", "three entries, one zero", "Only entries with a positive value create sources", ByCount.size(), 2) && Passed;
  for (MCSource* S: ByCount) delete S;

  MCIsotopeStore Empty;
  SilenceOutput();
  vector<MCSource*> None = Empty.CreateSourceListByActivity();
  RestoreOutput();
  Passed = EvaluateSize("CreateSourceListByActivity()", "empty store", "An empty store creates no sources", None.size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCIsotopeStore::TestStream()
{
  bool Passed = true;

  MCIsotopeStore Store;
  Store.Add("Crystal", 32077, 159.71*keV, 2.5);
  ostringstream Out;
  Out<<Store;
  MString S(Out.str().c_str());
  Passed = EvaluateTrue("operator<<", "one entry", "The output contains the volume", S.Contains("Crystal")) && Passed;
  Passed = EvaluateTrue("operator<<", "one entry", "The output contains the isotope ID", S.Contains("32077")) && Passed;
  Passed = EvaluateTrue("operator<<", "one entry", "The output contains the excitation", S.Contains("159.71")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTCIsotopeStore Test;
  return Test.Run() == true ? 0 : 1;
}
