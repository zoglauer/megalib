/*
 * UTCActivator.cc
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
#include "MCActivator.hh"
#include "MCActivatorParticle.hh"
#include "MCIsotopeStore.hh"

// MEGAlib:
#include "MStreams.h"
#include "MString.h"
#include "MUnitTest.h"

// Geant4:
#include "G4Alpha.hh"
#include "G4BaryonConstructor.hh"
#include "G4BosonConstructor.hh"
#include "G4Gamma.hh"
#include "G4GenericIon.hh"
#include "G4IonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4Neutron.hh"
#include "G4NuclideTable.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"
#include "G4RadioactiveDecay.hh"
#include "G4SystemOfUnits.hh"
#include "G4Version.hh"
#include "Randomize.hh"

// ROOT:
#include "TRandom.h"

// Standard lib:
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>
using namespace std;


//! Class exposing the protected functions of MCActivator to the unit test
class UTCActivatorAccess : public MCActivator
{
public:
  using MCActivator::ActivateByEquations;
  using MCActivator::ActivateByNumericalIntegration;
  using MCActivator::ActivateByPartialSimulation;
  using MCActivator::ActivateBySimulation;
  using MCActivator::CleanDecayChains;
  using MCActivator::CreateDeexcitationBranches;
  using MCActivator::DumpTree;
  using MCActivator::HasNoGammaTransitions;
  using MCActivator::HasNonITDecayChannels;
  using MCActivator::CountsO1;
  using MCActivator::CountsO2;
  using MCActivator::CountsO3;
  using MCActivator::CountsO4;
  using MCActivator::CountsO5;
  using MCActivator::ActivationO1;
  using MCActivator::ActivationO2;
  using MCActivator::ActivationO3;
  using MCActivator::ActivationO4;
  using MCActivator::CooldownO1;
  using MCActivator::CooldownO2;
  using MCActivator::CooldownO3;
  using MCActivator::CooldownO4;
  using MCActivator::CooldownOn;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test class for MCActivator
class UTCActivator : public MUnitTest
{
public:
  //! Default constructor
  UTCActivator() : MUnitTest("UTCActivator"), m_Decay(nullptr), m_CoutBuffer(nullptr), m_CerrBuffer(nullptr) {}
  //! Default destructor
  virtual ~UTCActivator() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the Geant4 version the nuclear data expectations are derived from
  bool TestGeant4Version();
  //! Test the default constructor
  bool TestDefaultConstruction();
  //! Test the simple setters and getters
  bool TestGettersSetters();
  //! Test copy construction and assignment
  bool TestCopyAndAssignment();
  //! Test the activation mode setters
  bool TestActivationModes();
  //! Test the half life determination
  bool TestDetermineHalfLife();
  //! Test adding and loading counts files
  bool TestCountsFiles();
  //! Test the activation of a single radioactive isotope
  bool TestSingleIsotope();
  //! Test the activation of a two-element decay chain
  bool TestDecayChain();
  //! Test multiple volumes and the reuse of already calculated trees
  bool TestMultipleVolumes();
  //! Test repeated calculations with the same activator
  bool TestRepeatedCalculation();
  //! Test isomers without gamma transitions (Al26m)
  bool TestIsomerWithoutGammas();
  //! Test isomers with IT and radioactive decay branches
  bool TestMixedDecayIsomers();
  //! Test the cooldown of an IT daughter
  bool TestMixedDecayIsomerCooldown();
  //! Test decays emitting neutrons and light ions
  bool TestParticleEmission();
  //! Test the detection of levels without gamma transitions
  bool TestHasNoGammaTransitions();
  //! Test the detection of levels with non-IT decay channels
  bool TestHasNonITDecayChannels();
  //! Test the creation of de-excitation branches
  bool TestCreateDeexcitationBranches();
  //! Test the cleaning of decay chains
  bool TestCleanDecayChains();
  //! Test the tree dump
  bool TestDumpTree();
  //! Test the analytic counts and activation solutions against a numerical integration
  bool TestAnalyticBuildUp();
  //! Test the analytic cooldown solutions against a numerical integration
  bool TestAnalyticCooldown();
  //! Test the activation by equations
  bool TestActivateByEquations();
  //! Test the activation by (partial) simulation
  bool TestActivateBySimulation();
  //! Test counts file failure paths and stale state
  bool TestCountsFileFailures();
  //! Test saving to an invalid location
  bool TestSaveOutputFileFailure();
  //! Test several excitations of the same isotope in one volume
  bool TestSeveralExcitations();
  //! Test the particle output mode for a decay chain
  bool TestParticleOutputModeChain();
  //! Test the general Bateman cooldown solution
  bool TestCooldownOn();
  //! Test longer chains in the activation by equations
  bool TestActivateByEquationsLongChains();
  //! Test the cooldown of the partial simulation with branching
  bool TestPartialSimulationCooldown();

  //! Initialize the Geant4 particles and the nuclide table as done in cosima
  bool InitializeGeant4();
  //! Redirect cout, cerr and the MEGAlib streams - the activator prints a lot
  void SilenceOutput();
  //! Restore cout, cerr and the MEGAlib streams
  void RestoreOutput();
  //! Create a particle with its half life from the activator
  MCActivatorParticle CreateParticle(unsigned int ID, double Excitation, double HalfLifeCutOff = 1*ns);
  //! Write a counts file with one isotope
  bool WriteCountsFile(const MString& FileName, int ID, double Excitation, double Counts, double Time, const MString& Volume = "Volume");
  //! Load the counts file, calculate the activation and save it - silenced
  bool RunActivation(MCActivator& Activator, const MString& CountsFile, const MString& OutputFile);
  //! Return the value stored for the isotope in the given file, or -1 if it is absent
  double GetStoredValue(const MString& FileName, int ID, double Excitation, const MString& Volume = "Volume");
  //! Return the number of stored isotope entries in the given file
  unsigned int GetNumberOfStoredEntries(const MString& FileName);
  //! Integrate the number of nuclei of a decay chain with production rate R into the first element (RK4)
  vector<double> IntegrateChain(double R, const vector<double>& D, const vector<double>& B, const vector<double>& N0, double Time);

  //! The radioactive decay process used to retrieve the decay tables
  G4RadioactiveDecay* m_Decay;
  //! The original cout buffer while silenced
  streambuf* m_CoutBuffer;
  //! The original cerr buffer while silenced
  streambuf* m_CerrBuffer;
  //! The sink for the silenced output
  ostringstream m_Sink;
};


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::Run()
{
  bool Passed = true;

  if (InitializeGeant4() == false) {
    Passed = EvaluateTrue("InitializeGeant4()", "particles", "Geant4 particles and nuclide table can be initialized", false) && Passed;
    Summarize();
    return Passed;
  }

  Passed = TestGeant4Version() && Passed;
  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestCopyAndAssignment() && Passed;
  Passed = TestActivationModes() && Passed;
  Passed = TestDetermineHalfLife() && Passed;
  Passed = TestCountsFiles() && Passed;
  Passed = TestSingleIsotope() && Passed;
  Passed = TestDecayChain() && Passed;
  Passed = TestMultipleVolumes() && Passed;
  Passed = TestRepeatedCalculation() && Passed;
  Passed = TestIsomerWithoutGammas() && Passed;
  Passed = TestMixedDecayIsomers() && Passed;
  Passed = TestMixedDecayIsomerCooldown() && Passed;
  Passed = TestParticleEmission() && Passed;
  Passed = TestHasNoGammaTransitions() && Passed;
  Passed = TestHasNonITDecayChannels() && Passed;
  Passed = TestCreateDeexcitationBranches() && Passed;
  Passed = TestCleanDecayChains() && Passed;
  Passed = TestDumpTree() && Passed;
  Passed = TestAnalyticBuildUp() && Passed;
  Passed = TestAnalyticCooldown() && Passed;
  Passed = TestActivateByEquations() && Passed;
  Passed = TestActivateBySimulation() && Passed;
  Passed = TestCountsFileFailures() && Passed;
  Passed = TestSaveOutputFileFailure() && Passed;
  Passed = TestSeveralExcitations() && Passed;
  Passed = TestParticleOutputModeChain() && Passed;
  Passed = TestCooldownOn() && Passed;
  Passed = TestActivateByEquationsLongChains() && Passed;
  Passed = TestPartialSimulationCooldown() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::InitializeGeant4()
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

  m_Decay = new G4RadioactiveDecay();

  RestoreOutput();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void UTCActivator::SilenceOutput()
{
  DisableDefaultStreams();
  m_Sink.str("");
  m_CoutBuffer = cout.rdbuf(m_Sink.rdbuf());
  m_CerrBuffer = cerr.rdbuf(m_Sink.rdbuf());
}


////////////////////////////////////////////////////////////////////////////////


void UTCActivator::RestoreOutput()
{
  if (m_CoutBuffer != nullptr) cout.rdbuf(m_CoutBuffer);
  if (m_CerrBuffer != nullptr) cerr.rdbuf(m_CerrBuffer);
  m_CoutBuffer = nullptr;
  m_CerrBuffer = nullptr;
  EnableDefaultStreams();
}


////////////////////////////////////////////////////////////////////////////////


MCActivatorParticle UTCActivator::CreateParticle(unsigned int ID, double Excitation, double HalfLifeCutOff)
{
  MCActivator Activator;
  Activator.SetHalfLifeCutOff(HalfLifeCutOff);

  SilenceOutput();
  MCActivatorParticle P;
  P.SetIDAndExcitation(ID, Excitation);
  double HalfLife = 0.0;
  double LevelEnergy = 0.0;
  Activator.DetermineHalfLife(P.GetDefinition(), HalfLife, LevelEnergy, false);
  RestoreOutput();

  P.SetHalfLife(HalfLife);
  P.SetBranchingRatio(1.0);
  P.SetProductionRate(1.0);

  return P;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::WriteCountsFile(const MString& FileName, int ID, double Excitation, double Counts, double Time, const MString& Volume)
{
  SilenceOutput();
  MCIsotopeStore Store;
  Store.SetTime(Time);
  Store.Add(Volume, ID, Excitation, Counts);
  bool Saved = Store.Save(FileName);
  RestoreOutput();

  return Saved;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::RunActivation(MCActivator& Activator, const MString& CountsFile, const MString& OutputFile)
{
  SilenceOutput();
  bool OK = Activator.AddCountsFile(CountsFile);
  OK = Activator.LoadCountsFiles() && OK;
  OK = Activator.CalculateEquilibriumRates() && OK;
  Activator.SetOutputFileName(OutputFile);
  OK = Activator.SaveOutputFile() && OK;
  RestoreOutput();

  return OK;
}


////////////////////////////////////////////////////////////////////////////////


double UTCActivator::GetStoredValue(const MString& FileName, int ID, double Excitation, const MString& Volume)
{
  SilenceOutput();
  MCIsotopeStore Store;
  bool Loaded = Store.Load(FileName);
  RestoreOutput();
  if (Loaded == false) return -1;

  // The excitation is stored with 0.01 keV precision:
  for (unsigned int v = 0; v < Store.GetNVolumes(); ++v) {
    if (Store.GetVolume(v) != Volume) continue;
    for (unsigned int i = 0; i < Store.GetNIDs(v); ++i) {
      if (Store.GetID(v, i) != ID) continue;
      for (unsigned int e = 0; e < Store.GetNExcitations(v, i); ++e) {
        if (fabs(Store.GetExcitation(v, i, e) - Excitation) < 0.01*keV) {
          return Store.GetValue(v, i, e);
        }
      }
    }
  }

  return -1;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int UTCActivator::GetNumberOfStoredEntries(const MString& FileName)
{
  SilenceOutput();
  MCIsotopeStore Store;
  Store.Load(FileName);
  RestoreOutput();

  unsigned int N = 0;
  for (unsigned int v = 0; v < Store.GetNVolumes(); ++v) {
    for (unsigned int i = 0; i < Store.GetNIDs(v); ++i) {
      N += Store.GetNExcitations(v, i);
    }
  }

  return N;
}


////////////////////////////////////////////////////////////////////////////////


vector<double> UTCActivator::IntegrateChain(double R, const vector<double>& D, const vector<double>& B, const vector<double>& N0, double Time)
{
  // dN1/dt = R - D1 N1, dNk/dt = Bk D(k-1) N(k-1) - Dk Nk
  // B[k] is the branching from element k-1 to element k, B[0] is unused
  auto Derivative = [&](const vector<double>& N) {
    vector<double> dN(N.size());
    for (unsigned int k = 0; k < N.size(); ++k) {
      double Source = (k == 0) ? R : B[k]*D[k-1]*N[k-1];
      dN[k] = Source - D[k]*N[k];
    }
    return dN;
  };

  const unsigned int Steps = 200000;
  const double h = Time/Steps;
  vector<double> N = N0;
  for (unsigned int s = 0; s < Steps; ++s) {
    vector<double> K1 = Derivative(N);
    vector<double> T(N.size());
    for (unsigned int k = 0; k < N.size(); ++k) T[k] = N[k] + 0.5*h*K1[k];
    vector<double> K2 = Derivative(T);
    for (unsigned int k = 0; k < N.size(); ++k) T[k] = N[k] + 0.5*h*K2[k];
    vector<double> K3 = Derivative(T);
    for (unsigned int k = 0; k < N.size(); ++k) T[k] = N[k] + h*K3[k];
    vector<double> K4 = Derivative(T);
    for (unsigned int k = 0; k < N.size(); ++k) N[k] += h/6.0*(K1[k] + 2*K2[k] + 2*K3[k] + K4[k]);
  }

  return N;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestGeant4Version()
{
  bool Passed = true;

  // The branching ratios and half lives below are taken from the Geant4 11.4 data sets
  // (RadioactiveDecay6.1.2, PhotonEvaporation6.1.2, G4ENSDFSTATE3.0):
  Passed = Evaluate("G4VERSION_NUMBER", "Geant4 version", "The nuclear data expectations of this test are derived from Geant4 11.4", G4VERSION_NUMBER/10, 114) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestDefaultConstruction()
{
  bool Passed = true;

  MCActivator A;
  Passed = Evaluate("MCActivator()", "default name", "The default name is empty", A.GetName(), MString("")) && Passed;
  Passed = Evaluate("MCActivator()", "default output file", "The default output file name is Activation.act", A.GetOutputFileName(), MString("Activation.act")) && Passed;
  Passed = EvaluateTrue("MCActivator()", "default output mode", "The default output mode is activities", A.GetOutputModeActivities()) && Passed;
  Passed = EvaluateFalse("MCActivator()", "default output mode", "The default output mode is not particles", A.GetOutputModeParticles()) && Passed;
  Passed = EvaluateNear("MCActivator()", "default activation time", "The default activation time is zero", A.GetActivationTime(), 0.0, 1e-12) && Passed;

  Passed = Evaluate("c_ConstantIrradiation", "mode ID", "The constant irradiation mode ID is 0", MCActivator::c_ConstantIrradiation, 0U) && Passed;
  Passed = Evaluate("c_ConstantIrradiationWithCooldown", "mode ID", "The constant irradiation with cooldown mode ID is 1", MCActivator::c_ConstantIrradiationWithCooldown, 1U) && Passed;
  Passed = Evaluate("c_TimeProfile", "mode ID", "The time profile mode ID is 2", MCActivator::c_TimeProfile, 2U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestGettersSetters()
{
  bool Passed = true;

  MCActivator A;

  A.SetName("Activator_Ge");
  Passed = Evaluate("SetName/GetName", "representative name", "GetName returns the name set before", A.GetName(), MString("Activator_Ge")) && Passed;
  A.SetName("");
  Passed = Evaluate("SetName/GetName", "empty name", "GetName returns an empty name after setting it", A.GetName(), MString("")) && Passed;

  Passed = EvaluateTrue("SetOutputFileName()", "representative name", "SetOutputFileName returns true", A.SetOutputFileName("Ge.act")) && Passed;
  Passed = Evaluate("GetOutputFileName()", "representative name", "GetOutputFileName returns the name set before", A.GetOutputFileName(), MString("Ge.act")) && Passed;

  Passed = EvaluateTrue("SetOutputModeParticles()", "particles", "SetOutputModeParticles returns true", A.SetOutputModeParticles()) && Passed;
  Passed = EvaluateTrue("GetOutputModeParticles()", "particles", "The particle output mode is active", A.GetOutputModeParticles()) && Passed;
  Passed = EvaluateFalse("GetOutputModeActivities()", "particles", "The activity output mode is inactive in particle mode", A.GetOutputModeActivities()) && Passed;
  Passed = EvaluateTrue("SetOutputModeActivities()", "activities", "SetOutputModeActivities returns true", A.SetOutputModeActivities()) && Passed;
  Passed = EvaluateTrue("GetOutputModeActivities()", "activities", "The activity output mode is active again", A.GetOutputModeActivities()) && Passed;
  Passed = EvaluateFalse("GetOutputModeParticles()", "activities", "The particle output mode is inactive in activity mode", A.GetOutputModeParticles()) && Passed;

  Passed = EvaluateTrue("SetActivationTime()", "1234.5 s", "SetActivationTime returns true", A.SetActivationTime(1234.5*s)) && Passed;
  Passed = EvaluateNear("GetActivationTime()", "1234.5 s", "GetActivationTime returns the time set before", A.GetActivationTime()/s, 1234.5, 1e-9) && Passed;
  Passed = EvaluateTrue("SetActivationTime()", "zero", "SetActivationTime accepts zero", A.SetActivationTime(0.0)) && Passed;
  Passed = EvaluateNear("GetActivationTime()", "zero", "GetActivationTime returns zero after setting it", A.GetActivationTime(), 0.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestCopyAndAssignment()
{
  bool Passed = true;

  // cosima keeps its activators in a vector, thus they are copied:
  MCActivator A;
  A.SetName("Original");
  A.SetOutputFileName("Original.act");
  A.SetOutputModeParticles();
  A.SetConstantIrradiation(100*s);

  MCActivator B(A);
  Passed = Evaluate("MCActivator(const MCActivator&)", "name", "The copy keeps the name", B.GetName(), MString("Original")) && Passed;
  Passed = Evaluate("MCActivator(const MCActivator&)", "output file", "The copy keeps the output file name", B.GetOutputFileName(), MString("Original.act")) && Passed;
  Passed = EvaluateTrue("MCActivator(const MCActivator&)", "output mode", "The copy keeps the particle output mode", B.GetOutputModeParticles()) && Passed;
  Passed = EvaluateNear("MCActivator(const MCActivator&)", "activation time", "The copy keeps the activation time", B.GetActivationTime()/s, 100.0, 1e-9) && Passed;

  B.SetName("Copy");
  Passed = Evaluate("MCActivator(const MCActivator&)", "independence", "Changing the copy does not change the original", A.GetName(), MString("Original")) && Passed;

  MCActivator C;
  C = A;
  Passed = Evaluate("operator=", "name", "The assigned activator keeps the name", C.GetName(), MString("Original")) && Passed;
  Passed = EvaluateNear("operator=", "activation time", "The assigned activator keeps the activation time", C.GetActivationTime()/s, 100.0, 1e-9) && Passed;
  C.SetOutputModeActivities();
  Passed = EvaluateTrue("operator=", "independence", "Changing the assigned activator does not change the original", A.GetOutputModeParticles()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestActivationModes()
{
  bool Passed = true;

  MCActivator A;
  Passed = EvaluateTrue("SetConstantIrradiation()", "100 s", "A positive activation time is accepted", A.SetConstantIrradiation(100*s)) && Passed;
  Passed = EvaluateNear("SetConstantIrradiation()", "100 s", "The activation time is stored", A.GetActivationTime()/s, 100.0, 1e-9) && Passed;

  DisableDefaultStreams();
  bool Zero = A.SetConstantIrradiation(0.0);
  bool Negative = A.SetConstantIrradiation(-1*s);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetConstantIrradiation()", "zero", "A zero activation time is rejected", Zero) && Passed;
  Passed = EvaluateFalse("SetConstantIrradiation()", "negative", "A negative activation time is rejected", Negative) && Passed;
  Passed = EvaluateNear("SetConstantIrradiation()", "after rejection", "A rejected activation time does not change the stored one", A.GetActivationTime()/s, 100.0, 1e-9) && Passed;

  MCActivator B;
  Passed = EvaluateTrue("SetConstantIrradiationWithCooldown()", "100 s, 50 s", "Positive activation and cooldown times are accepted", B.SetConstantIrradiationWithCooldown(100*s, 50*s)) && Passed;
  Passed = EvaluateNear("SetConstantIrradiationWithCooldown()", "100 s, 50 s", "The activation time is stored", B.GetActivationTime()/s, 100.0, 1e-9) && Passed;

  DisableDefaultStreams();
  bool ZeroActivation = B.SetConstantIrradiationWithCooldown(0.0, 50*s);
  bool NegativeActivation = B.SetConstantIrradiationWithCooldown(-1*s, 50*s);
  bool ZeroCooldown = B.SetConstantIrradiationWithCooldown(200*s, 0.0);
  bool NegativeCooldown = B.SetConstantIrradiationWithCooldown(200*s, -1*s);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetConstantIrradiationWithCooldown()", "zero activation", "A zero activation time is rejected", ZeroActivation) && Passed;
  Passed = EvaluateFalse("SetConstantIrradiationWithCooldown()", "negative activation", "A negative activation time is rejected", NegativeActivation) && Passed;
  Passed = EvaluateFalse("SetConstantIrradiationWithCooldown()", "zero cooldown", "A zero cooldown time is rejected", ZeroCooldown) && Passed;
  Passed = EvaluateFalse("SetConstantIrradiationWithCooldown()", "negative cooldown", "A negative cooldown time is rejected", NegativeCooldown) && Passed;
  Passed = EvaluateNear("SetConstantIrradiationWithCooldown()", "after rejection", "Rejected times do not change the stored activation time", B.GetActivationTime()/s, 100.0, 1e-9) && Passed;

  // The time profile mode is not implemented:
  MCActivator C;
  DisableDefaultStreams();
  bool MissingProfile = C.SetTimeProfile("/this/file/does/not/exist.dat", 100*s);
  bool NegativeProfile = C.SetTimeProfile("/this/file/does/not/exist.dat", -1*s);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetTimeProfile()", "missing file", "A missing time profile file is rejected", MissingProfile) && Passed;
  Passed = EvaluateFalse("SetTimeProfile()", "negative time", "A negative activation time is rejected", NegativeProfile) && Passed;

  const MString ProfileFile = GetTemporaryFileName("Profile.dat");
  Passed = EvaluateTrue("WriteTextFile()", "profile", "The time profile fixture can be written", WriteTextFile(ProfileFile, "0 1\n100 1\n")) && Passed;
  DisableDefaultStreams();
  bool ExistingProfile = C.SetTimeProfile(ProfileFile.Data(), 100*s);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetTimeProfile()", "existing file", "The not implemented time profile mode is rejected", ExistingProfile) && Passed;

  // A rejected time profile must not change the activation mode:
  const MString CountsFile = GetTemporaryFileName("ProfileCounts.dat");
  const MString OutputFile = GetTemporaryFileName("ProfileOutput.dat");
  Passed = EvaluateTrue("WriteCountsFile()", "Na24", "The counts fixture can be written", WriteCountsFile(CountsFile, 11024, 0.0, 10, 1*s)) && Passed;
  MCActivator D;
  D.SetConstantIrradiation(3600*s);
  DisableDefaultStreams();
  D.SetTimeProfile(ProfileFile.Data(), 3600*s);
  EnableDefaultStreams();
  Passed = EvaluateTrue("SetTimeProfile()", "calculation after rejection", "The calculation succeeds after a rejected time profile", RunActivation(D, CountsFile, OutputFile)) && Passed;
  Passed = EvaluateTrue("SetTimeProfile()", "activation after rejection", "A rejected time profile keeps the constant irradiation mode, i.e. Na24 is activated", GetStoredValue(OutputFile, 11024, 0.0) > 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestDetermineHalfLife()
{
  bool Passed = true;

  MCActivator A; // Default cut off: 1 ns
  double HalfLife = -1;
  double Excitation = -1;
  bool OK = false;

  // Ground state (ENSDF: Na24 14.956 h):
  MCActivatorParticle Na24;
  SilenceOutput();
  Na24.SetIDAndExcitation(11024, 0.0);
  OK = A.DetermineHalfLife(Na24.GetDefinition(), HalfLife, Excitation, false);
  RestoreOutput();
  Passed = EvaluateTrue("DetermineHalfLife()", "Na24", "The half life of a radioactive ground state is found", OK) && Passed;
  Passed = EvaluateNear("DetermineHalfLife()", "Na24", "The half life of Na24 is 14.956 h", HalfLife/s, 14.956*3600, 0.001*14.956*3600) && Passed;
  Passed = EvaluateNear("DetermineHalfLife()", "Na24", "The excitation of a ground state is zero", Excitation, 0.0, 1e-12) && Passed;

  // Stable ground state:
  MCActivatorParticle O16;
  SilenceOutput();
  O16.SetIDAndExcitation(8016, 0.0);
  OK = A.DetermineHalfLife(O16.GetDefinition(), HalfLife, Excitation, false);
  RestoreOutput();
  Passed = EvaluateTrue("DetermineHalfLife()", "O16", "The half life of a stable ground state is found", OK) && Passed;
  Passed = EvaluateTrue("DetermineHalfLife()", "O16", "A stable isotope has the maximum double as half life", HalfLife == numeric_limits<double>::max()) && Passed;

  // Long lived isomer - the level data contains the mean life, not the half life (G4ENSDFSTATE: 6.346 s):
  MCActivatorParticle Al26m;
  SilenceOutput();
  Al26m.SetIDAndExcitation(13026, 228.305*keV);
  OK = A.DetermineHalfLife(Al26m.GetDefinition(), HalfLife, Excitation, false);
  RestoreOutput();
  Passed = EvaluateTrue("DetermineHalfLife()", "Al26[228.305]", "The half life of an isomer is found", OK) && Passed;
  Passed = EvaluateNear("DetermineHalfLife()", "Al26[228.305]", "The half life of Al26m is 6.346 s, not its mean life of 9.155 s", HalfLife/s, 6.346, 0.001) && Passed;
  Passed = EvaluateNear("DetermineHalfLife()", "Al26[228.305]", "The excitation of the level is returned in keV", Excitation/keV, 228.305, 0.001) && Passed;

  // Short lived level (PhotonEvaporation: Al26[1057.739] 2.5e-14 s) below the cut off:
  MCActivatorParticle Al26Short;
  SilenceOutput();
  Al26Short.SetIDAndExcitation(13026, 1057.739*keV);
  OK = A.DetermineHalfLife(Al26Short.GetDefinition(), HalfLife, Excitation, false);
  RestoreOutput();
  Passed = EvaluateTrue("DetermineHalfLife()", "Al26[1057.739]", "The half life of a short lived level is found", OK) && Passed;
  Passed = EvaluateNear("DetermineHalfLife()", "Al26[1057.739]", "Half lives below the cut off are set to zero", HalfLife, 0.0, 1e-30) && Passed;

  SilenceOutput();
  OK = A.DetermineHalfLife(Al26Short.GetDefinition(), HalfLife, Excitation, true);
  RestoreOutput();
  Passed = EvaluateTrue("DetermineHalfLife()", "Al26[1057.739], ignore cut off", "The half life is found when ignoring the cut off", OK) && Passed;
  Passed = EvaluateNear("DetermineHalfLife()", "Al26[1057.739], ignore cut off", "The half life of 2.5e-14 s is returned when ignoring the cut off", HalfLife/s/2.5e-14, 1.0, 0.001) && Passed;

  // The cut off is configurable:
  MCActivator B;
  B.SetHalfLifeCutOff(10*s);
  SilenceOutput();
  OK = B.DetermineHalfLife(Al26m.GetDefinition(), HalfLife, Excitation, false);
  RestoreOutput();
  Passed = EvaluateNear("SetHalfLifeCutOff()", "10 s, Al26[228.305]", "A 6.346 s half life is below a 10 s cut off and set to zero", HalfLife, 0.0, 1e-30) && Passed;

  // Non-nuclei:
  SilenceOutput();
  OK = A.DetermineHalfLife(G4Gamma::Definition(), HalfLife, Excitation, false);
  RestoreOutput();
  Passed = EvaluateTrue("DetermineHalfLife()", "gamma", "The half life of a photon is found", OK) && Passed;
  Passed = EvaluateTrue("DetermineHalfLife()", "gamma", "A photon is stable", HalfLife == numeric_limits<double>::max()) && Passed;

  SilenceOutput();
  OK = A.DetermineHalfLife(G4Neutron::Definition(), HalfLife, Excitation, false);
  RestoreOutput();
  Passed = EvaluateTrue("DetermineHalfLife()", "neutron", "The half life of a neutron is found", OK) && Passed;
  Passed = EvaluateNear("DetermineHalfLife()", "neutron", "The half life of the neutron is its PDG life time times ln 2", HalfLife, G4Neutron::Definition()->GetPDGLifeTime()*log(2.0), 1e-6*HalfLife) && Passed;

  SilenceOutput();
  OK = A.DetermineHalfLife(G4Alpha::Definition(), HalfLife, Excitation, false);
  RestoreOutput();
  Passed = EvaluateTrue("DetermineHalfLife()", "alpha", "The half life of an alpha is found", OK) && Passed;
  Passed = EvaluateTrue("DetermineHalfLife()", "alpha", "An alpha is stable", HalfLife == numeric_limits<double>::max()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestCountsFiles()
{
  bool Passed = true;

  const double TNa24 = CreateParticle(11024, 0.0).GetHalfLife();
  const double DNa24 = log(2.0)/TNa24;
  const double Activation = 3600*s;

  // No counts file at all:
  {
    MCActivator A;
    A.SetConstantIrradiation(Activation);
    const MString OutputFile = GetTemporaryFileName("NoCounts.act");
    SilenceOutput();
    bool Loaded = A.LoadCountsFiles();
    bool Calculated = A.CalculateEquilibriumRates();
    A.SetOutputFileName(OutputFile);
    bool Saved = A.SaveOutputFile();
    RestoreOutput();
    Passed = EvaluateFalse("LoadCountsFiles()", "no files", "Loading without counts files fails", Loaded) && Passed;
    Passed = EvaluateTrue("CalculateEquilibriumRates()", "no files", "The calculation without counts succeeds", Calculated) && Passed;
    Passed = EvaluateTrue("SaveOutputFile()", "no files", "The empty output can be saved", Saved) && Passed;
    Passed = Evaluate("SaveOutputFile()", "no files", "The output contains no isotopes", GetNumberOfStoredEntries(OutputFile), 0U) && Passed;
  }

  // Missing counts file:
  {
    MCActivator A;
    A.SetConstantIrradiation(Activation);
    const MString OutputFile = GetTemporaryFileName("MissingCounts.act");
    SilenceOutput();
    bool Added = A.AddCountsFile(GetTemporaryFileName("DoesNotExist.dat"));
    bool Loaded = A.LoadCountsFiles();
    RestoreOutput();
    Passed = EvaluateFalse("AddCountsFile()", "missing file", "A missing counts file is rejected", Added) && Passed;
    Passed = EvaluateFalse("LoadCountsFiles()", "missing file", "Without the rejected file there is nothing to load", Loaded) && Passed;
  }

  // Counts file without time:
  {
    const MString CountsFile = GetTemporaryFileName("NoTime.dat");
    Passed = EvaluateTrue("WriteCountsFile()", "no time", "The counts fixture without time can be written", WriteCountsFile(CountsFile, 11024, 0.0, 10, 0.0)) && Passed;
    MCActivator A;
    SilenceOutput();
    bool Added = A.AddCountsFile(CountsFile);
    bool Loaded = A.LoadCountsFiles();
    RestoreOutput();
    Passed = EvaluateTrue("AddCountsFile()", "existing file", "An existing counts file is added", Added) && Passed;
    Passed = EvaluateFalse("LoadCountsFiles()", "no time", "A counts file without time is rejected", Loaded) && Passed;
  }

  // Only stable isotopes:
  {
    const MString CountsFile = GetTemporaryFileName("Stable.dat");
    const MString OutputFile = GetTemporaryFileName("Stable.act");
    Passed = EvaluateTrue("WriteCountsFile()", "O16", "The stable counts fixture can be written", WriteCountsFile(CountsFile, 8016, 0.0, 10, 1*s)) && Passed;
    MCActivator A;
    A.SetConstantIrradiation(Activation);
    Passed = EvaluateTrue("CalculateEquilibriumRates()", "O16", "The calculation for a stable isotope succeeds", RunActivation(A, CountsFile, OutputFile)) && Passed;
    Passed = Evaluate("CalculateEquilibriumRates()", "O16", "Stable isotopes are not activated", GetNumberOfStoredEntries(OutputFile), 0U) && Passed;
  }

  // Two files are merged and normalized by the total time: (10 + 30)/(5 s + 15 s) = 2/s
  {
    const MString CountsFile1 = GetTemporaryFileName("Merge1.dat");
    const MString CountsFile2 = GetTemporaryFileName("Merge2.dat");
    const MString OutputFile = GetTemporaryFileName("Merge.act");
    Passed = EvaluateTrue("WriteCountsFile()", "merge 1", "The first counts fixture can be written", WriteCountsFile(CountsFile1, 11024, 0.0, 10, 5*s)) && Passed;
    Passed = EvaluateTrue("WriteCountsFile()", "merge 2", "The second counts fixture can be written", WriteCountsFile(CountsFile2, 11024, 0.0, 30, 15*s)) && Passed;
    MCActivator A;
    A.SetConstantIrradiation(Activation);
    SilenceOutput();
    A.AddCountsFile(CountsFile1);
    RestoreOutput();
    Passed = EvaluateTrue("AddCountsFile()", "two files", "The merged calculation succeeds", RunActivation(A, CountsFile2, OutputFile)) && Passed;
    double Expected = 2.0*(1 - exp(-DNa24*Activation));
    Passed = EvaluateNear("LoadCountsFiles()", "two files", "The rates of both files are added and divided by the total time", GetStoredValue(OutputFile, 11024, 0.0), Expected, 1e-5*Expected) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestSingleIsotope()
{
  bool Passed = true;

  const double TNa24 = CreateParticle(11024, 0.0).GetHalfLife();
  const double DNa24 = log(2.0)/TNa24;
  const double Rate = 2.0/s;
  const double Activation = 3600*s;
  const double Cooldown = 7200*s;

  const MString CountsFile = GetTemporaryFileName("Na24.dat");
  Passed = EvaluateTrue("WriteCountsFile()", "Na24", "The counts fixture can be written", WriteCountsFile(CountsFile, 11024, 0.0, 20, 10*s)) && Passed;

  // Activities:
  {
    const MString OutputFile = GetTemporaryFileName("Na24.act");
    MCActivator A;
    A.SetConstantIrradiation(Activation);
    Passed = EvaluateTrue("CalculateEquilibriumRates()", "Na24, 1 h", "The activation of Na24 succeeds", RunActivation(A, CountsFile, OutputFile)) && Passed;
    double Expected = Rate*(1 - exp(-DNa24*Activation))*s;
    Passed = EvaluateNear("CalculateEquilibriumRates()", "Na24, 1 h", "The Na24 activity is R (1 - exp(-lambda t)) in Bq", GetStoredValue(OutputFile, 11024, 0.0), Expected, 1e-5*Expected) && Passed;
    Passed = Evaluate("CalculateEquilibriumRates()", "Na24, 1 h", "Only Na24 is activated, Mg24 is stable", GetNumberOfStoredEntries(OutputFile), 1U) && Passed;
  }

  // Number of nuclei:
  {
    const MString OutputFile = GetTemporaryFileName("Na24Particles.act");
    MCActivator A;
    A.SetConstantIrradiation(Activation);
    A.SetOutputModeParticles();
    Passed = EvaluateTrue("SetOutputModeParticles()", "Na24, 1 h", "The activation of Na24 in particle mode succeeds", RunActivation(A, CountsFile, OutputFile)) && Passed;
    double Expected = floor(0.5 + Rate*(1 - exp(-DNa24*Activation))/DNa24);
    Passed = EvaluateNear("SetOutputModeParticles()", "Na24, 1 h", "The number of Na24 nuclei is the rounded activity divided by lambda", GetStoredValue(OutputFile, 11024, 0.0), Expected, 0.5) && Passed;
  }

  // Cooldown:
  {
    const MString OutputFile = GetTemporaryFileName("Na24Cooldown.act");
    MCActivator A;
    A.SetConstantIrradiationWithCooldown(Activation, Cooldown);
    Passed = EvaluateTrue("SetConstantIrradiationWithCooldown()", "Na24, 1 h + 2 h", "The activation of Na24 with cooldown succeeds", RunActivation(A, CountsFile, OutputFile)) && Passed;
    double Expected = Rate*(1 - exp(-DNa24*Activation))*exp(-DNa24*Cooldown)*s;
    Passed = EvaluateNear("SetConstantIrradiationWithCooldown()", "Na24, 1 h + 2 h", "The Na24 activity decays exponentially during cooldown", GetStoredValue(OutputFile, 11024, 0.0), Expected, 1e-5*Expected) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestDecayChain()
{
  bool Passed = true;

  // Mg28 (20.9 h) -> Al28 (2.24 min) -> Si28, all intermediate Al28 levels are prompt:
  const double D1 = log(2.0)/CreateParticle(12028, 0.0).GetHalfLife();
  const double D2 = log(2.0)/CreateParticle(13028, 0.0).GetHalfLife();
  const double Rate = 1.0/s;
  const double Activation = 6*3600*s;
  const double Cooldown = 600*s;

  const MString CountsFile = GetTemporaryFileName("Mg28.dat");
  Passed = EvaluateTrue("WriteCountsFile()", "Mg28", "The counts fixture can be written", WriteCountsFile(CountsFile, 12028, 0.0, 10, 10*s)) && Passed;

  // Bateman solution for constant production:
  double A1 = Rate*(1 - exp(-D1*Activation));
  double A2 = Rate*(1 + (D1*exp(-D2*Activation) - D2*exp(-D1*Activation))/(D2 - D1));
  {
    const MString OutputFile = GetTemporaryFileName("Mg28.act");
    MCActivator A;
    A.SetConstantIrradiation(Activation);
    Passed = EvaluateTrue("CalculateEquilibriumRates()", "Mg28, 6 h", "The activation of Mg28 succeeds", RunActivation(A, CountsFile, OutputFile)) && Passed;
    Passed = EvaluateNear("CalculateEquilibriumRates()", "Mg28, 6 h", "The Mg28 activity follows the build-up", GetStoredValue(OutputFile, 12028, 0.0), A1*s, 1e-5*A1*s) && Passed;
    Passed = EvaluateNear("CalculateEquilibriumRates()", "Mg28, 6 h", "The Al28 activity follows the Bateman solution", GetStoredValue(OutputFile, 13028, 0.0), A2*s, 1e-5*A2*s) && Passed;
    Passed = EvaluateTrue("CalculateEquilibriumRates()", "Mg28, 6 h", "Al28[30.64] (2.07 ns) is above the 1 ns cut off and stored", GetStoredValue(OutputFile, 13028, 30.64*keV) > 0) && Passed;
    Passed = Evaluate("CalculateEquilibriumRates()", "Mg28, 6 h", "Only Mg28, Al28 and Al28[30.64] are stored, the prompt Al28 levels are not", GetNumberOfStoredEntries(OutputFile), 3U) && Passed;
  }

  // Bateman solution for the cooldown - the branch via Al28[30.64] has 4 elements and uses the partial simulation:
  {
    const MString OutputFile = GetTemporaryFileName("Mg28Cooldown.act");
    MCActivator A;
    A.SetConstantIrradiationWithCooldown(Activation, Cooldown);
    UInt_t OldSeed = gRandom->GetSeed();
    gRandom->SetSeed(4711);
    CLHEP::HepRandom::setTheSeed(4711);
    Passed = EvaluateTrue("SetConstantIrradiationWithCooldown()", "Mg28, 6 h + 10 min", "The activation of Mg28 with cooldown succeeds", RunActivation(A, CountsFile, OutputFile)) && Passed;
    gRandom->SetSeed(OldSeed);
    double C1 = A1*exp(-D1*Cooldown);
    double C2 = A2*exp(-D2*Cooldown) + A1*D2/(D2 - D1)*(exp(-D1*Cooldown) - exp(-D2*Cooldown));
    Passed = EvaluateNear("SetConstantIrradiationWithCooldown()", "Mg28, 6 h + 10 min", "The Mg28 activity decays during cooldown", GetStoredValue(OutputFile, 12028, 0.0), C1*s, 1e-5*C1*s) && Passed;
    // The partial simulation has an accuracy of about 0.2%:
    Passed = EvaluateNear("SetConstantIrradiationWithCooldown()", "Mg28, 6 h + 10 min", "The Al28 activity follows the Bateman solution during cooldown within 1%", GetStoredValue(OutputFile, 13028, 0.0), C2*s, 0.01*C2*s) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestMultipleVolumes()
{
  bool Passed = true;

  // The second volume reuses the already calculated tree, scaled by the production rate:
  const double DNa24 = log(2.0)/CreateParticle(11024, 0.0).GetHalfLife();
  const double Activation = 3600*s;

  const MString CountsFile = GetTemporaryFileName("Volumes.dat");
  const MString OutputFile = GetTemporaryFileName("Volumes.act");
  MCIsotopeStore Store;
  Store.SetTime(10*s);
  Store.Add("VolumeA", 11024, 0.0, 10);
  Store.Add("VolumeB", 11024, 0.0, 30);
  SilenceOutput();
  bool Saved = Store.Save(CountsFile);
  RestoreOutput();
  Passed = EvaluateTrue("MCIsotopeStore::Save()", "two volumes", "The two-volume counts fixture can be written", Saved) && Passed;

  MCActivator A;
  A.SetConstantIrradiation(Activation);
  Passed = EvaluateTrue("CalculateEquilibriumRates()", "two volumes", "The activation in two volumes succeeds", RunActivation(A, CountsFile, OutputFile)) && Passed;
  double ExpectedA = 1.0*(1 - exp(-DNa24*Activation));
  double ExpectedB = 3.0*(1 - exp(-DNa24*Activation));
  Passed = EvaluateNear("CalculateEquilibriumRates()", "volume A", "The first volume gets its own activity", GetStoredValue(OutputFile, 11024, 0.0, "VolumeA"), ExpectedA, 1e-5*ExpectedA) && Passed;
  Passed = EvaluateNear("CalculateEquilibriumRates()", "volume B", "The reused tree is scaled to the production rate of the second volume", GetStoredValue(OutputFile, 11024, 0.0, "VolumeB"), ExpectedB, 1e-5*ExpectedB) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestRepeatedCalculation()
{
  bool Passed = true;

  const double DNa24 = log(2.0)/CreateParticle(11024, 0.0).GetHalfLife();
  const double Activation = 3600*s;
  double Expected = 1.0*(1 - exp(-DNa24*Activation));

  const MString CountsFile = GetTemporaryFileName("Repeated.dat");
  Passed = EvaluateTrue("WriteCountsFile()", "Na24", "The counts fixture can be written", WriteCountsFile(CountsFile, 11024, 0.0, 10, 10*s)) && Passed;

  MCActivator A;
  A.SetConstantIrradiation(Activation);
  SilenceOutput();
  A.AddCountsFile(CountsFile);
  bool Loaded1 = A.LoadCountsFiles();
  bool Loaded2 = A.LoadCountsFiles();
  bool Calculated1 = A.CalculateEquilibriumRates();
  bool Calculated2 = A.CalculateEquilibriumRates();
  const MString OutputFile = GetTemporaryFileName("Repeated.act");
  A.SetOutputFileName(OutputFile);
  bool Saved = A.SaveOutputFile();
  RestoreOutput();

  Passed = EvaluateTrue("LoadCountsFiles()", "twice", "Loading the counts files twice succeeds", Loaded1 && Loaded2) && Passed;
  Passed = EvaluateTrue("CalculateEquilibriumRates()", "twice", "Calculating twice succeeds", Calculated1 && Calculated2) && Passed;
  Passed = EvaluateTrue("SaveOutputFile()", "twice", "Saving after two calculations succeeds", Saved) && Passed;
  Passed = EvaluateNear("CalculateEquilibriumRates()", "twice", "A repeated calculation does not accumulate the activities of the previous one", GetStoredValue(OutputFile, 11024, 0.0), Expected, 1e-5*Expected) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestIsomerWithoutGammas()
{
  bool Passed = true;

  // Al26[228.305] (6.346 s) has no gamma transitions and decays by beta+ to Mg26:
  const double DAl26m = log(2.0)/(6.346*s);
  const double Activation = 100*s;

  const MString CountsFile = GetTemporaryFileName("Al26m.dat");
  Passed = EvaluateTrue("WriteCountsFile()", "Al26m", "The counts fixture can be written", WriteCountsFile(CountsFile, 13026, 228.305*keV, 10, 10*s)) && Passed;

  {
    const MString OutputFile = GetTemporaryFileName("Al26m.act");
    MCActivator A;
    A.SetConstantIrradiation(Activation);
    Passed = EvaluateTrue("CalculateEquilibriumRates()", "Al26m, 1 ns cut off", "The activation of Al26m terminates", RunActivation(A, CountsFile, OutputFile)) && Passed;
    double Expected = 1.0*(1 - exp(-DAl26m*Activation));
    Passed = EvaluateNear("CalculateEquilibriumRates()", "Al26m, 1 ns cut off", "The Al26m activity follows its 6.346 s half life", GetStoredValue(OutputFile, 13026, 228.305*keV), Expected, 1e-4*Expected) && Passed;
    Passed = EvaluateTrue("CalculateEquilibriumRates()", "Al26m, 1 ns cut off", "Al26m decays to Mg26, not to the Al26 ground state", GetStoredValue(OutputFile, 13026, 0.0) < 0) && Passed;
    Passed = Evaluate("CalculateEquilibriumRates()", "Al26m, 1 ns cut off", "Only Al26m is activated", GetNumberOfStoredEntries(OutputFile), 1U) && Passed;
  }

  {
    const MString OutputFile = GetTemporaryFileName("Al26m10s.act");
    MCActivator A;
    A.SetConstantIrradiation(Activation);
    A.SetHalfLifeCutOff(10*s);
    Passed = EvaluateTrue("CalculateEquilibriumRates()", "Al26m, 10 s cut off", "The activation of Al26m with a large cut off terminates", RunActivation(A, CountsFile, OutputFile)) && Passed;
    Passed = Evaluate("CalculateEquilibriumRates()", "Al26m, 10 s cut off", "Al26m decays immediately to stable Mg26, not to the Al26 ground state", GetNumberOfStoredEntries(OutputFile), 0U) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestMixedDecayIsomers()
{
  bool Passed = true;

  // RadioactiveDecay6.1.2: the IT fraction of the isomer feeds the ground state
  // In equilibrium the ground state activity is the IT fraction times the production rate
  struct Isomer {
    MString Name;
    int ID;
    double Excitation;
    double ITFraction;
    double Activation;
  };
  vector<Isomer> Isomers = {
    { "Ge77[159.71]", 32077, 159.71*keV, 0.19, 1e7*s },
    { "K38[130.22]", 19038, 130.22*keV, 0.00033, 1e5*s },
    { "Co60[58.59]", 27060, 58.59*keV, 0.9975, 1e11*s },
    { "Cl34[146.36]", 17034, 146.36*keV, 0.446, 1e6*s }
  };

  const double Rate = 1000.0/s;
  for (const Isomer& I: Isomers) {
    const MString CountsFile = GetTemporaryFileName(MString("Isomer") + I.ID + ".dat");
    const MString OutputFile = GetTemporaryFileName(MString("Isomer") + I.ID + ".act");
    Passed = EvaluateTrue("WriteCountsFile()", I.Name, "The counts fixture can be written", WriteCountsFile(CountsFile, I.ID, I.Excitation, 1000, 1*s)) && Passed;

    MCActivator A;
    A.SetConstantIrradiation(I.Activation);
    Passed = EvaluateTrue("CalculateEquilibriumRates()", I.Name, "The activation of the isomer succeeds", RunActivation(A, CountsFile, OutputFile)) && Passed;
    double Expected = I.ITFraction*Rate*s;
    Passed = EvaluateNear("CalculateEquilibriumRates()", I.Name, "The ground state activity in equilibrium is the IT fraction of the production rate", GetStoredValue(OutputFile, I.ID, 0.0), Expected, 1e-4*Expected) && Passed;
  }

  // Ge77m: As77 is fed directly (81%) and via Ge77 (19%), in equilibrium its activity is the full production rate
  // The second path has branching points above As77 - its relative branching ratios must be derived from the absolute ones
  {
    const MString CountsFile = GetTemporaryFileName("Ge77mAs77.dat");
    const MString OutputFile = GetTemporaryFileName("Ge77mAs77.act");
    Passed = EvaluateTrue("WriteCountsFile()", "Ge77m", "The counts fixture can be written", WriteCountsFile(CountsFile, 32077, 159.71*keV, 1000, 1*s)) && Passed;
    MCActivator A;
    A.SetConstantIrradiation(1e7*s);
    Passed = EvaluateTrue("CalculateEquilibriumRates()", "Ge77m", "The activation of Ge77m succeeds", RunActivation(A, CountsFile, OutputFile)) && Passed;
    double Expected = Rate*s;
    Passed = EvaluateNear("CalculateEquilibriumRates()", "Ge77m -> As77", "In equilibrium the As77 activity from all paths is the production rate", GetStoredValue(OutputFile, 33077, 0.0), Expected, 1e-4*Expected) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestMixedDecayIsomerCooldown()
{
  bool Passed = true;

  // Ge77[159.71] (53.7 s): 19% IT to Ge77 (11.2 h), 81% beta- to As77
  const double D1 = log(2.0)/CreateParticle(32077, 159.71*keV).GetHalfLife();
  const double D2 = log(2.0)/CreateParticle(32077, 0.0).GetHalfLife();
  const double B = 0.19;
  const double Rate = 1000.0/s;
  const double Activation = 1e7*s;
  const double Cooldown = 3600*s;

  const MString CountsFile = GetTemporaryFileName("Ge77m.dat");
  const MString OutputFile = GetTemporaryFileName("Ge77mCooldown.act");
  Passed = EvaluateTrue("WriteCountsFile()", "Ge77m", "The counts fixture can be written", WriteCountsFile(CountsFile, 32077, 159.71*keV, 1000, 1*s)) && Passed;

  // The chain Ge77m -> Ge77 -> As77 -> Se77 has 4 elements and uses the partial simulation:
  MCActivator A;
  A.SetConstantIrradiationWithCooldown(Activation, Cooldown);
  UInt_t OldSeed = gRandom->GetSeed();
  gRandom->SetSeed(4711);
  CLHEP::HepRandom::setTheSeed(4711);
  Passed = EvaluateTrue("SetConstantIrradiationWithCooldown()", "Ge77m, 1e7 s + 1 h", "The activation of Ge77m with cooldown succeeds", RunActivation(A, CountsFile, OutputFile)) && Passed;
  gRandom->SetSeed(OldSeed);

  // Bateman solution - A2 at the end of the irradiation already contains the branching ratio:
  double A1 = Rate*(1 - exp(-D1*Activation));
  double A2 = B*Rate*(1 + (D1*exp(-D2*Activation) - D2*exp(-D1*Activation))/(D2 - D1));
  double C2 = A2*exp(-D2*Cooldown) + B*A1*D2/(D2 - D1)*(exp(-D1*Cooldown) - exp(-D2*Cooldown));
  Passed = EvaluateNear("SetConstantIrradiationWithCooldown()", "Ge77m, 1e7 s + 1 h", "The Ge77 ground state activity after cooldown follows the Bateman solution within 1%", GetStoredValue(OutputFile, 32077, 0.0), C2*s, 0.01*C2*s) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestParticleEmission()
{
  bool Passed = true;

  // N17: beta- delayed neutron emission to O16 and gamma cascades to O17 - both stable
  {
    const MString CountsFile = GetTemporaryFileName("N17.dat");
    const MString OutputFile = GetTemporaryFileName("N17.act");
    Passed = EvaluateTrue("WriteCountsFile()", "N17", "The counts fixture can be written", WriteCountsFile(CountsFile, 7017, 0.0, 10, 10*s)) && Passed;
    MCActivator A;
    A.SetConstantIrradiation(100*s);
    Passed = EvaluateTrue("CalculateEquilibriumRates()", "N17", "The activation of the delayed neutron emitter N17 succeeds", RunActivation(A, CountsFile, OutputFile)) && Passed;
    Passed = Evaluate("CalculateEquilibriumRates()", "N17", "Only N17 is activated, no excited O17 level remains", GetNumberOfStoredEntries(OutputFile), 1U) && Passed;
  }

  // He8: 83.1% to Li8 (839.9 ms), the rest via neutron or triton emission to stable Li7 and He4
  {
    const MString CountsFile = GetTemporaryFileName("He8.dat");
    const MString OutputFile = GetTemporaryFileName("He8.act");
    Passed = EvaluateTrue("WriteCountsFile()", "He8", "The counts fixture can be written", WriteCountsFile(CountsFile, 2008, 0.0, 1000, 1*s)) && Passed;
    MCActivator A;
    A.SetConstantIrradiation(1000*s);
    Passed = EvaluateTrue("CalculateEquilibriumRates()", "He8", "The activation of He8 with triton emission succeeds", RunActivation(A, CountsFile, OutputFile)) && Passed;
    double Expected = 0.831*1000;
    Passed = EvaluateNear("CalculateEquilibriumRates()", "He8", "The Li8 activity in equilibrium is 83.1% of the He8 production rate", GetStoredValue(OutputFile, 3008, 0.0), Expected, 1e-4*Expected) && Passed;
  }

  // He10 decays by two-neutron emission to He8 - He9 is not known to Geant4 and gets a zero half life as ground state:
  {
    const MString CountsFile = GetTemporaryFileName("He10.dat");
    const MString OutputFile = GetTemporaryFileName("He10.act");
    Passed = EvaluateTrue("WriteCountsFile()", "He10", "The counts fixture can be written", WriteCountsFile(CountsFile, 2010, 0.0, 1000, 1*s)) && Passed;
    MCActivator A;
    A.SetConstantIrradiation(1000*s);
    Passed = EvaluateTrue("CalculateEquilibriumRates()", "He10", "The activation of He10 terminates", RunActivation(A, CountsFile, OutputFile)) && Passed;
    Passed = EvaluateNear("CalculateEquilibriumRates()", "He10", "In equilibrium the He8 activity is the He10 production rate", GetStoredValue(OutputFile, 2008, 0.0), 1000.0, 1e-4*1000.0) && Passed;
  }

  // Be11: the beta-delayed alpha level B11[9873] (20 ns) cannot be created by the isotope store and is removed:
  {
    const double DBe11 = log(2.0)/CreateParticle(4011, 0.0).GetHalfLife();
    const MString CountsFile = GetTemporaryFileName("Be11.dat");
    const MString OutputFile = GetTemporaryFileName("Be11.act");
    Passed = EvaluateTrue("WriteCountsFile()", "Be11", "The counts fixture can be written", WriteCountsFile(CountsFile, 4011, 0.0, 10, 10*s)) && Passed;
    MCActivator A;
    A.SetConstantIrradiation(10*s);
    Passed = EvaluateTrue("CalculateEquilibriumRates()", "Be11", "The activation of Be11 succeeds", RunActivation(A, CountsFile, OutputFile)) && Passed;
    double Expected = 1.0*(1 - exp(-DBe11*10*s));
    Passed = EvaluateNear("CalculateEquilibriumRates()", "Be11", "The Be11 activity follows the build-up", GetStoredValue(OutputFile, 4011, 0.0), Expected, 1e-4*Expected) && Passed;
    Passed = Evaluate("CalculateEquilibriumRates()", "Be11", "Only Be11 is stored, the unknown B11[9873] level is removed", GetNumberOfStoredEntries(OutputFile), 1U) && Passed;
  }

  // F19[197.143] (89 ns) de-excites via two paths to the ground state - its production rate must not double:
  {
    const MString CountsFile = GetTemporaryFileName("F19m.dat");
    const MString OutputFile = GetTemporaryFileName("F19m.act");
    Passed = EvaluateTrue("WriteCountsFile()", "F19[197.143]", "The counts fixture can be written", WriteCountsFile(CountsFile, 9019, 197.143*keV, 10, 10*s)) && Passed;
    MCActivator A;
    A.SetConstantIrradiation(100*s);
    Passed = EvaluateTrue("CalculateEquilibriumRates()", "F19[197.143]", "The activation of F19[197.143] succeeds", RunActivation(A, CountsFile, OutputFile)) && Passed;
    Passed = EvaluateNear("CalculateEquilibriumRates()", "F19[197.143]", "In equilibrium the F19[197.143] activity is its production rate", GetStoredValue(OutputFile, 9019, 197.143*keV), 1.0, 1e-4) && Passed;
  }

  // H4 is unknown to Geant4 - it is removed from the counts:
  {
    const MString CountsFile = GetTemporaryFileName("H4.dat");
    const MString OutputFile = GetTemporaryFileName("H4.act");
    Passed = EvaluateTrue("WriteCountsFile()", "H4", "The counts fixture can be written", WriteCountsFile(CountsFile, 1004, 0.0, 10, 10*s)) && Passed;
    MCActivator A;
    A.SetConstantIrradiation(100*s);
    Passed = EvaluateTrue("CalculateEquilibriumRates()", "H4", "The activation of an isotope unknown to Geant4 succeeds", RunActivation(A, CountsFile, OutputFile)) && Passed;
    Passed = Evaluate("CalculateEquilibriumRates()", "H4", "Isotopes unknown to Geant4 are not activated", GetNumberOfStoredEntries(OutputFile), 0U) && Passed;
  }

  // Be8 and He5 disintegrate completely into alphas (and a neutron):
  vector<int> IDs = { 4008, 2005 };
  for (int ID: IDs) {
    const MString CountsFile = GetTemporaryFileName(MString("Disintegrate") + ID + ".dat");
    const MString OutputFile = GetTemporaryFileName(MString("Disintegrate") + ID + ".act");
    Passed = EvaluateTrue("WriteCountsFile()", MString(ID), "The counts fixture can be written", WriteCountsFile(CountsFile, ID, 0.0, 10, 10*s)) && Passed;
    MCActivator A;
    A.SetConstantIrradiation(100*s);
    Passed = EvaluateTrue("CalculateEquilibriumRates()", MString(ID), "The activation of a completely disintegrating nucleus succeeds", RunActivation(A, CountsFile, OutputFile)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestHasNoGammaTransitions()
{
  bool Passed = true;

  UTCActivatorAccess A;
  Passed = EvaluateTrue("HasNoGammaTransitions()", "Al26[228.305]", "Al26m has no gamma transitions", A.HasNoGammaTransitions(CreateParticle(13026, 228.305*keV))) && Passed;
  Passed = EvaluateTrue("HasNoGammaTransitions()", "O17[5387.1]", "The neutron unbound O17[5387.1] level has no gamma transitions", A.HasNoGammaTransitions(CreateParticle(8017, 5387.1*keV))) && Passed;
  Passed = EvaluateFalse("HasNoGammaTransitions()", "Al26[416.852]", "Al26[416.852] has a gamma transition", A.HasNoGammaTransitions(CreateParticle(13026, 416.852*keV))) && Passed;
  Passed = EvaluateFalse("HasNoGammaTransitions()", "O17[4551.8]", "O17[4551.8] has gamma transitions", A.HasNoGammaTransitions(CreateParticle(8017, 4551.8*keV))) && Passed;
  Passed = EvaluateFalse("HasNoGammaTransitions()", "Na24", "A ground state is never considered", A.HasNoGammaTransitions(CreateParticle(11024, 0.0))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestHasNonITDecayChannels()
{
  bool Passed = true;

  UTCActivatorAccess A;
  Passed = EvaluateTrue("HasNonITDecayChannels()", "Ge77[159.71]", "Ge77m has a beta- channel", A.HasNonITDecayChannels(CreateParticle(32077, 159.71*keV), m_Decay)) && Passed;
  Passed = EvaluateTrue("HasNonITDecayChannels()", "O17[4551.8]", "O17[4551.8] has a neutron channel", A.HasNonITDecayChannels(CreateParticle(8017, 4551.8*keV), m_Decay)) && Passed;
  Passed = EvaluateTrue("HasNonITDecayChannels()", "Al26[228.305]", "Al26m has a beta+ channel", A.HasNonITDecayChannels(CreateParticle(13026, 228.305*keV), m_Decay)) && Passed;
  Passed = EvaluateFalse("HasNonITDecayChannels()", "Al26[416.852]", "Al26[416.852] only has an IT channel", A.HasNonITDecayChannels(CreateParticle(13026, 416.852*keV), m_Decay)) && Passed;
  Passed = EvaluateFalse("HasNonITDecayChannels()", "Na24[1341.5]", "A level without decay data only gets the IT channel", A.HasNonITDecayChannels(CreateParticle(11024, 1341.5*keV), m_Decay)) && Passed;
  Passed = EvaluateFalse("HasNonITDecayChannels()", "Na24", "A ground state is never considered", A.HasNonITDecayChannels(CreateParticle(11024, 0.0), m_Decay)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestCreateDeexcitationBranches()
{
  bool Passed = true;

  UTCActivatorAccess A;

  // Single transition, prompt level: the last element is replaced
  {
    vector<MCActivatorParticle> Branch = { CreateParticle(13026, 1057.739*keV) };
    vector<vector<MCActivatorParticle> > NewBranches;
    SilenceOutput();
    bool OK = A.CreateDeexcitationBranches(Branch, 1.0, NewBranches);
    RestoreOutput();
    Passed = EvaluateTrue("CreateDeexcitationBranches()", "Al26[1057.739]", "The de-excitation of a level with one transition succeeds", OK) && Passed;
    Passed = EvaluateSize("CreateDeexcitationBranches()", "Al26[1057.739]", "One transition creates one branch", NewBranches.size(), 1) && Passed;
    if (NewBranches.size() == 1) {
      Passed = EvaluateSize("CreateDeexcitationBranches()", "Al26[1057.739]", "A prompt level is replaced, not appended", NewBranches[0].size(), 1) && Passed;
      Passed = EvaluateNear("CreateDeexcitationBranches()", "Al26[1057.739]", "The new level is Al26[228.305]", NewBranches[0].back().GetExcitation()/keV, 228.305, 0.001) && Passed;
      Passed = EvaluateNear("CreateDeexcitationBranches()", "Al26[1057.739]", "The single transition has the full branching ratio", NewBranches[0].back().GetBranchingRatio(), 1.0, 1e-6) && Passed;
    }
  }

  // Scaled by an IT branching and appended after a long lived level:
  {
    vector<MCActivatorParticle> Branch = { CreateParticle(27060, 58.59*keV) };
    Branch[0].SetProductionRate(2.0);
    vector<vector<MCActivatorParticle> > NewBranches;
    SilenceOutput();
    bool OK = A.CreateDeexcitationBranches(Branch, 0.9975, NewBranches);
    RestoreOutput();
    Passed = EvaluateTrue("CreateDeexcitationBranches()", "Co60[58.59], scale 0.9975", "The IT de-excitation of Co60m succeeds", OK) && Passed;
    Passed = EvaluateSize("CreateDeexcitationBranches()", "Co60[58.59], scale 0.9975", "Co60m has one transition", NewBranches.size(), 1) && Passed;
    if (NewBranches.size() == 1) {
      Passed = EvaluateSize("CreateDeexcitationBranches()", "Co60[58.59], scale 0.9975", "A long lived level is kept and the new level is appended", NewBranches[0].size(), 2) && Passed;
      Passed = EvaluateNear("CreateDeexcitationBranches()", "Co60[58.59], scale 0.9975", "The new level is the Co60 ground state", NewBranches[0].back().GetExcitation(), 0.0, 1e-9) && Passed;
      Passed = EvaluateNear("CreateDeexcitationBranches()", "Co60[58.59], scale 0.9975", "The branching ratio is scaled by the IT fraction", NewBranches[0].back().GetBranchingRatio(), 0.9975, 1e-6) && Passed;
      Passed = EvaluateNear("CreateDeexcitationBranches()", "Co60[58.59], scale 0.9975", "The production rate is scaled by the IT fraction", NewBranches[0].back().GetProductionRate(), 2.0*0.9975, 1e-6) && Passed;
    }
  }

  // Several transitions: the transition probabilities add up to the scale
  {
    vector<MCActivatorParticle> Branch = { CreateParticle(8017, 4551.8*keV) };
    vector<vector<MCActivatorParticle> > NewBranches;
    SilenceOutput();
    bool OK = A.CreateDeexcitationBranches(Branch, 0.5, NewBranches);
    RestoreOutput();
    Passed = EvaluateTrue("CreateDeexcitationBranches()", "O17[4551.8], scale 0.5", "The de-excitation of a level with two transitions succeeds", OK) && Passed;
    Passed = EvaluateSize("CreateDeexcitationBranches()", "O17[4551.8], scale 0.5", "Two transitions create two branches", NewBranches.size(), 2) && Passed;
    double Sum = 0.0;
    for (auto& B: NewBranches) Sum += B.back().GetBranchingRatio();
    Passed = EvaluateNear("CreateDeexcitationBranches()", "O17[4551.8], scale 0.5", "The branching ratios of all transitions add up to the scale", Sum, 0.5, 1e-6) && Passed;
  }

  // Transitions without intensity do not create branches - all intensities zero: only the last transition is used
  {
    vector<MCActivatorParticle> Branch = { CreateParticle(26056, 4812.68*keV) };
    Branch[0].SetProductionRate(3.0);
    vector<vector<MCActivatorParticle> > NewBranches;
    SilenceOutput();
    bool OK = A.CreateDeexcitationBranches(Branch, 0.7, NewBranches);
    RestoreOutput();
    Passed = EvaluateTrue("CreateDeexcitationBranches()", "Fe56[4812.68], zero intensities", "The de-excitation of a level without transition intensities succeeds", OK) && Passed;
    Passed = EvaluateSize("CreateDeexcitationBranches()", "Fe56[4812.68], zero intensities", "Three transitions without intensity create exactly one branch", NewBranches.size(), 1) && Passed;
    if (NewBranches.size() == 1) {
      Passed = EvaluateSize("CreateDeexcitationBranches()", "Fe56[4812.68], zero intensities", "The prompt level is replaced", NewBranches[0].size(), 1) && Passed;
      Passed = EvaluateNear("CreateDeexcitationBranches()", "Fe56[4812.68], zero intensities", "The single branch carries the full scaled branching ratio", NewBranches[0].back().GetBranchingRatio(), 0.7, 1e-6) && Passed;
      Passed = EvaluateNear("CreateDeexcitationBranches()", "Fe56[4812.68], zero intensities", "The single branch carries the full scaled production rate", NewBranches[0].back().GetProductionRate(), 3.0*0.7, 1e-6) && Passed;
    }
  }

  // One of five transitions without intensity: four branches with positive weights
  {
    vector<MCActivatorParticle> Branch = { CreateParticle(26056, 4683.04*keV) };
    Branch[0].SetProductionRate(3.0);
    vector<vector<MCActivatorParticle> > NewBranches;
    SilenceOutput();
    bool OK = A.CreateDeexcitationBranches(Branch, 0.7, NewBranches);
    RestoreOutput();
    Passed = EvaluateTrue("CreateDeexcitationBranches()", "Fe56[4683.04], one zero intensity", "The de-excitation of a level with one zero intensity succeeds", OK) && Passed;
    Passed = EvaluateSize("CreateDeexcitationBranches()", "Fe56[4683.04], one zero intensity", "Five transitions with one zero intensity create four branches", NewBranches.size(), 4) && Passed;
    double SumBR = 0.0;
    double SumPR = 0.0;
    bool AllPositive = true;
    for (auto& B: NewBranches) {
      if (B.back().GetBranchingRatio() <= 0.0) AllPositive = false;
      SumBR += B.back().GetBranchingRatio();
      SumPR += B.back().GetProductionRate();
    }
    Passed = EvaluateTrue("CreateDeexcitationBranches()", "Fe56[4683.04], one zero intensity", "All created branches have a positive branching ratio", AllPositive) && Passed;
    Passed = EvaluateNear("CreateDeexcitationBranches()", "Fe56[4683.04], one zero intensity", "The branching ratios add up to the scale", SumBR, 0.7, 1e-6) && Passed;
    Passed = EvaluateNear("CreateDeexcitationBranches()", "Fe56[4683.04], one zero intensity", "The production rates add up to the scaled production rate", SumPR, 3.0*0.7, 1e-6) && Passed;
  }

  // The highest tabulated level also de-excites to excited levels - here Ga74[1085.72] to the isomer Ga74[59.571]:
  {
    vector<MCActivatorParticle> Branch = { CreateParticle(31074, 1085.72*keV) };
    vector<vector<MCActivatorParticle> > NewBranches;
    SilenceOutput();
    bool OK = A.CreateDeexcitationBranches(Branch, 1.0, NewBranches);
    RestoreOutput();
    Passed = EvaluateTrue("CreateDeexcitationBranches()", "Ga74[1085.72], highest level", "The de-excitation of the highest tabulated level succeeds", OK) && Passed;
    bool IsomerFound = false;
    for (auto& B: NewBranches) {
      if (fabs(B.back().GetExcitation() - 59.571*keV) < 0.01*keV) IsomerFound = true;
    }
    Passed = EvaluateTrue("CreateDeexcitationBranches()", "Ga74[1085.72], highest level", "The transitions of the highest level lead to the isomer Ga74[59.571], not only to the ground state", IsomerFound) && Passed;
  }

  // Existing branches are kept:
  {
    vector<MCActivatorParticle> Branch = { CreateParticle(13026, 1057.739*keV) };
    vector<vector<MCActivatorParticle> > NewBranches = { { CreateParticle(11024, 0.0) } };
    SilenceOutput();
    A.CreateDeexcitationBranches(Branch, 1.0, NewBranches);
    RestoreOutput();
    Passed = EvaluateSize("CreateDeexcitationBranches()", "non-empty output", "The new branches are appended to the existing ones", NewBranches.size(), 2) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestCleanDecayChains()
{
  bool Passed = true;

  UTCActivatorAccess A;

  // Identical branches are merged:
  {
    MCActivatorParticle Mother = CreateParticle(12028, 0.0);
    MCActivatorParticle Daughter1 = CreateParticle(13028, 0.0);
    Daughter1.SetBranchingRatio(0.3);
    Daughter1.SetProductionRate(0.3);
    MCActivatorParticle Daughter2 = CreateParticle(13028, 0.0);
    Daughter2.SetBranchingRatio(0.7);
    Daughter2.SetProductionRate(0.7);
    vector<vector<MCActivatorParticle> > Tree = { { Mother, Daughter1 }, { Mother, Daughter2 } };
    bool Changed = A.CleanDecayChains(Tree);
    Passed = EvaluateTrue("CleanDecayChains()", "two identical branches", "Merging identical branches changes the tree", Changed) && Passed;
    Passed = EvaluateSize("CleanDecayChains()", "two identical branches", "Identical branches are merged into one", Tree.size(), 1) && Passed;
    if (Tree.size() == 1) {
      Passed = EvaluateNear("CleanDecayChains()", "two identical branches", "The branching ratios of the last element are added", Tree[0].back().GetBranchingRatio(), 1.0, 1e-9) && Passed;
      Passed = EvaluateNear("CleanDecayChains()", "two identical branches", "The production rates of the last element are added", Tree[0].back().GetProductionRate(), 1.0, 1e-9) && Passed;
    }
  }

  // A shared mother of the same isotope (F19[197.143] -> F19 via two paths) is not added twice:
  {
    MCActivatorParticle Mother = CreateParticle(9019, 197.143*keV);
    MCActivatorParticle Daughter1 = CreateParticle(9019, 0.0);
    Daughter1.SetBranchingRatio(0.4);
    Daughter1.SetProductionRate(0.4);
    MCActivatorParticle Daughter2 = CreateParticle(9019, 0.0);
    Daughter2.SetBranchingRatio(0.6);
    Daughter2.SetProductionRate(0.6);
    vector<vector<MCActivatorParticle> > Tree = { { Mother, Daughter1 }, { Mother, Daughter2 } };
    A.CleanDecayChains(Tree);
    Passed = EvaluateSize("CleanDecayChains()", "shared mother", "Identical branches are merged into one", Tree.size(), 1) && Passed;
    if (Tree.size() == 1) {
      Passed = EvaluateNear("CleanDecayChains()", "shared mother", "The production rate of the shared mother is not added", Tree[0][0].GetProductionRate(), 1.0, 1e-9) && Passed;
      Passed = EvaluateNear("CleanDecayChains()", "shared mother", "The branching ratio of the shared mother is not added", Tree[0][0].GetBranchingRatio(), 1.0, 1e-9) && Passed;
      Passed = EvaluateNear("CleanDecayChains()", "shared mother", "The branching ratios of the separate daughters are added", Tree[0][1].GetBranchingRatio(), 1.0, 1e-9) && Passed;
    }
  }

  // Separate paths already at the first element (e.g. replaced prompt levels) are added from the start:
  {
    MCActivatorParticle First1 = CreateParticle(12028, 0.0);
    First1.SetBranchingRatio(0.3);
    First1.SetProductionRate(0.3);
    MCActivatorParticle First2 = CreateParticle(12028, 0.0);
    First2.SetBranchingRatio(0.7);
    First2.SetProductionRate(0.7);
    MCActivatorParticle Daughter = CreateParticle(13028, 0.0);
    Daughter.SetBranchingRatio(0.3);
    MCActivatorParticle Daughter2 = Daughter;
    Daughter2.SetBranchingRatio(0.7);
    vector<vector<MCActivatorParticle> > Tree = { { First1, Daughter }, { First2, Daughter2 } };
    A.CleanDecayChains(Tree);
    Passed = EvaluateSize("CleanDecayChains()", "separate first elements", "Identical branches are merged into one", Tree.size(), 1) && Passed;
    if (Tree.size() == 1) {
      Passed = EvaluateNear("CleanDecayChains()", "separate first elements", "The production rates of separate first elements are added", Tree[0][0].GetProductionRate(), 1.0, 1e-9) && Passed;
      Passed = EvaluateNear("CleanDecayChains()", "separate first elements", "The branching ratios of separate first elements are added", Tree[0][0].GetBranchingRatio(), 1.0, 1e-9) && Passed;
    }
  }

  // Partially overlapping paths: X represents {A, B}, Y represents {B, C} - B must only be counted once:
  {
    MCActivatorParticle Mother = CreateParticle(12028, 0.0);
    MCActivatorParticle PA = CreateParticle(13028, 0.0);
    PA.SetBranchingRatio(0.2);
    MCActivatorParticle PB = CreateParticle(13028, 0.0);
    PB.SetBranchingRatio(0.3);
    MCActivatorParticle PC = CreateParticle(13028, 0.0);
    PC.SetBranchingRatio(0.5);
    MCActivatorParticle EndB1 = CreateParticle(14028, 0.0);
    EndB1.SetBranchingRatio(0.1);
    MCActivatorParticle EndB2 = CreateParticle(14028, 0.0);
    EndB2.SetBranchingRatio(0.2);
    MCActivatorParticle EndA = CreateParticle(14028, 0.0);
    EndA.SetBranchingRatio(0.2);
    MCActivatorParticle EndC = CreateParticle(14028, 0.0);
    EndC.SetBranchingRatio(0.5);

    // Merge {A, B} and {B, C} separately, then merge the results:
    vector<vector<MCActivatorParticle> > X = { { Mother, PA, EndA }, { Mother, PB, EndB1 } };
    vector<vector<MCActivatorParticle> > Y = { { Mother, PB, EndB2 }, { Mother, PC, EndC } };
    A.CleanDecayChains(X);
    A.CleanDecayChains(Y);
    vector<vector<MCActivatorParticle> > Tree = { X[0], Y[0] };
    A.CleanDecayChains(Tree);
    Passed = EvaluateSize("CleanDecayChains()", "partial overlap", "Identical branches are merged into one", Tree.size(), 1) && Passed;
    if (Tree.size() == 1) {
      Passed = EvaluateNear("CleanDecayChains()", "partial overlap", "The shared mother is not added", Tree[0][0].GetBranchingRatio(), 1.0, 1e-9) && Passed;
      Passed = EvaluateNear("CleanDecayChains()", "partial overlap", "The intermediate paths A, B and C are each counted once", Tree[0][1].GetBranchingRatio(), 1.0, 1e-9) && Passed;
      Passed = EvaluateNear("CleanDecayChains()", "partial overlap", "The four end paths are each counted once", Tree[0][2].GetBranchingRatio(), 1.0, 1e-9) && Passed;
    }
  }

  // Different branches are kept, empty ones removed:
  {
    vector<vector<MCActivatorParticle> > Tree = { { CreateParticle(12028, 0.0), CreateParticle(13028, 0.0) }, { }, { CreateParticle(11024, 0.0) } };
    bool Changed = A.CleanDecayChains(Tree);
    Passed = EvaluateTrue("CleanDecayChains()", "one empty branch", "Removing an empty branch changes the tree", Changed) && Passed;
    Passed = EvaluateSize("CleanDecayChains()", "one empty branch", "Only the empty branch is removed", Tree.size(), 2) && Passed;
  }

  // Nothing to do:
  {
    vector<vector<MCActivatorParticle> > Tree = { { CreateParticle(11024, 0.0) } };
    Passed = EvaluateFalse("CleanDecayChains()", "single branch", "A clean tree is not changed", A.CleanDecayChains(Tree)) && Passed;
    Passed = EvaluateSize("CleanDecayChains()", "single branch", "A clean tree keeps its branch", Tree.size(), 1) && Passed;

    vector<vector<MCActivatorParticle> > Empty;
    Passed = EvaluateFalse("CleanDecayChains()", "empty tree", "An empty tree is not changed", A.CleanDecayChains(Empty)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestDumpTree()
{
  bool Passed = true;

  UTCActivatorAccess A;
  vector<vector<MCActivatorParticle> > Tree = { { CreateParticle(12028, 0.0), CreateParticle(13028, 0.0) } };

  SilenceOutput();
  A.DumpTree(Tree, "Intro line");
  MString Dump = m_Sink.str().c_str();
  RestoreOutput();

  Passed = EvaluateTrue("DumpTree()", "Mg28 -> Al28", "The dump starts with the intro", Dump.BeginsWith("Intro line")) && Passed;
  Passed = EvaluateTrue("DumpTree()", "Mg28 -> Al28", "The dump contains the mother", Dump.Contains("Mg28")) && Passed;
  Passed = EvaluateTrue("DumpTree()", "Mg28 -> Al28", "The dump contains the daughter", Dump.Contains("Al28")) && Passed;
  Passed = EvaluateTrue("DumpTree()", "Mg28 -> Al28", "The dump links mother and daughter", Dump.Contains("->")) && Passed;

  SilenceOutput();
  A.DumpTree(Tree);
  MString NoIntro = m_Sink.str().c_str();
  RestoreOutput();
  Passed = EvaluateTrue("DumpTree()", "no intro", "Without intro the dump starts with the first branch", NoIntro.BeginsWith("  0: ")) && Passed;

  vector<vector<MCActivatorParticle> > Empty;
  SilenceOutput();
  A.DumpTree(Empty);
  MString EmptyDump = m_Sink.str().c_str();
  RestoreOutput();
  Passed = EvaluateTrue("DumpTree()", "empty tree", "An empty tree without intro prints nothing", EmptyDump.Length() == 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestAnalyticBuildUp()
{
  bool Passed = true;

  UTCActivatorAccess A;

  // Representative, well separated decay constants and branching ratios (arbitrary time unit):
  const double R = 5.0;
  const double t = 7.3;
  const vector<double> D = { 0.3, 0.07, 0.9, 0.02, 0.15 };
  const vector<double> B = { 1.0, 0.8, 0.6, 0.5, 0.9 };
  const double Tolerance = 1e-6;

  vector<double> N = IntegrateChain(R, D, B, vector<double>(5, 0.0), t);

  Passed = EvaluateNear("CountsO1()", "interior", "The counts of the first element match the integration", A.CountsO1(R, D[0], t), N[0], Tolerance*N[0]) && Passed;
  Passed = EvaluateNear("CountsO2()", "interior", "The counts of the second element match the integration", A.CountsO2(R, D[0], B[1], D[1], t), N[1], Tolerance*N[1]) && Passed;
  Passed = EvaluateNear("CountsO3()", "interior", "The counts of the third element match the integration", A.CountsO3(R, D[0], B[1], D[1], B[2], D[2], t), N[2], Tolerance*N[2]) && Passed;
  Passed = EvaluateNear("CountsO4()", "interior", "The counts of the fourth element match the integration", A.CountsO4(R, D[0], B[1], D[1], B[2], D[2], B[3], D[3], t), N[3], Tolerance*N[3]) && Passed;
  SilenceOutput();
  double UnstableFifth = A.CountsO5(R, D[0], B[1], D[1], B[2], D[2], B[3], D[3], B[4], D[4], t);
  RestoreOutput();
  Passed = EvaluateNear("CountsO5()", "unstable fifth element", "More than 4 unstable elements are not allowed and return zero", UnstableFifth, 0.0, 1e-12) && Passed;

  Passed = EvaluateNear("ActivationO1()", "interior", "The activation of the first element is D1 N1", A.ActivationO1(R, D[0], t), D[0]*N[0], Tolerance*D[0]*N[0]) && Passed;
  Passed = EvaluateNear("ActivationO2()", "interior", "The activation of the second element is D2 N2", A.ActivationO2(R, D[0], B[1], D[1], t), D[1]*N[1], Tolerance*D[1]*N[1]) && Passed;
  Passed = EvaluateNear("ActivationO3()", "interior", "The activation of the third element is D3 N3", A.ActivationO3(R, D[0], B[1], D[1], B[2], D[2], t), D[2]*N[2], Tolerance*D[2]*N[2]) && Passed;
  Passed = EvaluateNear("ActivationO4()", "interior", "The activation of the fourth element is D4 N4", A.ActivationO4(R, D[0], B[1], D[1], B[2], D[2], B[3], D[3], t), D[3]*N[3], Tolerance*D[3]*N[3]) && Passed;

  // Stable last element:
  for (unsigned int n = 1; n < 5; ++n) {
    vector<double> DS = D;
    DS[n] = 0.0;
    vector<double> NS = IntegrateChain(R, DS, B, vector<double>(5, 0.0), t);
    double Counts = 0.0;
    if (n == 1) Counts = A.CountsO2(R, DS[0], B[1], DS[1], t);
    if (n == 2) Counts = A.CountsO3(R, DS[0], B[1], DS[1], B[2], DS[2], t);
    if (n == 3) Counts = A.CountsO4(R, DS[0], B[1], DS[1], B[2], DS[2], B[3], DS[3], t);
    if (n == 4) Counts = A.CountsO5(R, DS[0], B[1], DS[1], B[2], DS[2], B[3], DS[3], B[4], DS[4], t);
    Passed = EvaluateNear(MString("CountsO") + (n+1) + "()", "stable last element", "The counts of a stable last element match the integration", Counts, NS[n], Tolerance*NS[n]) && Passed;
  }

  // Stable first element:
  Passed = EvaluateNear("CountsO1()", "stable", "A stable element accumulates R t", A.CountsO1(R, 0.0, t), R*t, 1e-12) && Passed;
  Passed = EvaluateNear("ActivationO1()", "stable", "A stable element has no activity", A.ActivationO1(R, 0.0, t), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("ActivationO2()", "stable daughter", "A stable daughter has no activity", A.ActivationO2(R, D[0], B[1], 0.0, t), 0.0, 1e-12) && Passed;

  // Start and equilibrium:
  Passed = EvaluateNear("ActivationO1()", "t = 0", "There is no activity at the start", A.ActivationO1(R, D[0], 0.0), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("ActivationO2()", "t = 0", "There is no activity at the start", A.ActivationO2(R, D[0], B[1], D[1], 0.0), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("ActivationO3()", "t = 0", "There is no activity at the start", A.ActivationO3(R, D[0], B[1], D[1], B[2], D[2], 0.0), 0.0, 1e-9) && Passed;
  Passed = EvaluateNear("ActivationO2()", "equilibrium", "The equilibrium activity is B12 R", A.ActivationO2(R, D[0], B[1], D[1], 1e4), B[1]*R, 1e-9) && Passed;
  Passed = EvaluateNear("ActivationO3()", "equilibrium", "The equilibrium activity is B12 B23 R", A.ActivationO3(R, D[0], B[1], D[1], B[2], D[2], 1e4), B[1]*B[2]*R, 1e-6) && Passed;
  Passed = EvaluateNear("ActivationO4()", "equilibrium", "The equilibrium activity is B12 B23 B34 R", A.ActivationO4(R, D[0], B[1], D[1], B[2], D[2], B[3], D[3], 1e4), B[1]*B[2]*B[3]*R, 1e-6) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestAnalyticCooldown()
{
  bool Passed = true;

  UTCActivatorAccess A;

  const double R = 5.0;
  const double T = 7.3;
  const double t = 4.1;
  const vector<double> D = { 0.3, 0.07, 0.9, 0.02 };
  const vector<double> B = { 1.0, 0.8, 0.6, 0.5 };
  const double Tolerance = 1e-6;

  // Activities at the end of the irradiation:
  vector<double> N0 = IntegrateChain(R, D, B, vector<double>(4, 0.0), T);
  vector<double> A0(4);
  for (unsigned int k = 0; k < 4; ++k) A0[k] = D[k]*N0[k];

  // Decay without production:
  vector<double> N = IntegrateChain(0.0, D, B, N0, t);
  vector<double> Expected(4);
  for (unsigned int k = 0; k < 4; ++k) Expected[k] = D[k]*N[k];

  Passed = EvaluateNear("CooldownO1()", "interior", "The cooldown of the first element matches the integration", A.CooldownO1(A0[0], D[0], t), Expected[0], Tolerance*Expected[0]) && Passed;
  Passed = EvaluateNear("CooldownO2()", "interior", "The cooldown of the second element matches the integration", A.CooldownO2(A0[0], D[0], B[1], A0[1], D[1], t), Expected[1], Tolerance*Expected[1]) && Passed;
  Passed = EvaluateNear("CooldownO3()", "interior", "The cooldown of the third element matches the integration", A.CooldownO3(A0[0], D[0], B[1], A0[1], D[1], B[2], A0[2], D[2], t), Expected[2], Tolerance*Expected[2]) && Passed;
  Passed = EvaluateNear("CooldownO4()", "interior", "The cooldown of the fourth element matches the integration", A.CooldownO4(A0[0], D[0], B[1], A0[1], D[1], B[2], A0[2], D[2], B[3], A0[3], D[3], t), Expected[3], Tolerance*Expected[3]) && Passed;

  // No cooldown time keeps the activities:
  Passed = EvaluateNear("CooldownO1()", "t = 0", "Without cooldown the activity is unchanged", A.CooldownO1(A0[0], D[0], 0.0), A0[0], Tolerance*A0[0]) && Passed;
  Passed = EvaluateNear("CooldownO2()", "t = 0", "Without cooldown the activity is unchanged", A.CooldownO2(A0[0], D[0], B[1], A0[1], D[1], 0.0), A0[1], Tolerance*A0[1]) && Passed;
  Passed = EvaluateNear("CooldownO3()", "t = 0", "Without cooldown the activity is unchanged", A.CooldownO3(A0[0], D[0], B[1], A0[1], D[1], B[2], A0[2], D[2], 0.0), A0[2], Tolerance*A0[2]) && Passed;
  Passed = EvaluateNear("CooldownO4()", "t = 0", "Without cooldown the activity is unchanged", A.CooldownO4(A0[0], D[0], B[1], A0[1], D[1], B[2], A0[2], D[2], B[3], A0[3], D[3], 0.0), A0[3], Tolerance*A0[3]) && Passed;

  // Stable elements:
  Passed = EvaluateNear("CooldownO1()", "stable", "A stable element has no activity", A.CooldownO1(A0[0], 0.0, t), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("CooldownO2()", "stable daughter", "A stable daughter has no activity", A.CooldownO2(A0[0], D[0], B[1], A0[1], 0.0, t), 0.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestActivateByEquations()
{
  bool Passed = true;

  UTCActivatorAccess A;

  // Mg28 -> Al28:
  MCActivatorParticle Mg28 = CreateParticle(12028, 0.0);
  MCActivatorParticle Al28 = CreateParticle(13028, 0.0);
  Mg28.SetProductionRate(1.0/s);
  vector<MCActivatorParticle> Chain = { Mg28, Al28 };
  SilenceOutput();
  bool OK = A.ActivateByEquations(Chain, 3600*s, 0.0);
  RestoreOutput();
  Passed = EvaluateTrue("ActivateByEquations()", "Mg28 -> Al28", "A two-element chain is calculated", OK) && Passed;
  double Expected1 = A.ActivationO1(1.0/s, Mg28.GetDecayConstant(), 3600*s);
  double Expected2 = A.ActivationO2(1.0/s, Mg28.GetDecayConstant(), 1.0, Al28.GetDecayConstant(), 3600*s);
  Passed = EvaluateNear("ActivateByEquations()", "Mg28 -> Al28", "The mother activity is set", Chain[0].GetActivation(), Expected1, 1e-9*Expected1) && Passed;
  Passed = EvaluateNear("ActivateByEquations()", "Mg28 -> Al28", "The daughter activity is set", Chain[1].GetActivation(), Expected2, 1e-9*Expected2) && Passed;

  // Too many elements:
  vector<MCActivatorParticle> Long(6, Mg28);
  for (unsigned int p = 0; p < Long.size(); ++p) Long[p].SetHalfLife((p+1)*1000*s);
  SilenceOutput();
  bool TooLong = A.ActivateByEquations(Long, 3600*s, 0.0);
  RestoreOutput();
  Passed = EvaluateFalse("ActivateByEquations()", "6 elements", "Chains with more than 5 elements are rejected", TooLong) && Passed;

  vector<MCActivatorParticle> Four(Long.begin(), Long.begin() + 4);
  SilenceOutput();
  bool FourCooldown = A.ActivateByEquations(Four, 3600*s, 60*s);
  RestoreOutput();
  Passed = EvaluateFalse("ActivateByEquations()", "4 elements with cooldown", "The cooldown is only available for up to 3 elements", FourCooldown) && Passed;

  // Almost identical decay constants:
  vector<MCActivatorParticle> Identical = { Mg28, Mg28 };
  Identical[1].SetHalfLife(Mg28.GetHalfLife()*1.001);
  SilenceOutput();
  bool IdenticalOK = A.ActivateByEquations(Identical, 3600*s, 0.0);
  RestoreOutput();
  Passed = EvaluateFalse("ActivateByEquations()", "identical decay constants", "Almost identical decay constants are rejected", IdenticalOK) && Passed;

  // Not implemented:
  SilenceOutput();
  bool Numerical = A.ActivateByNumericalIntegration(Chain, 3600*s, 0.0);
  RestoreOutput();
  Passed = EvaluateFalse("ActivateByNumericalIntegration()", "Mg28 -> Al28", "The numerical integration is not implemented", Numerical) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestActivateBySimulation()
{
  bool Passed = true;

  UTCActivatorAccess A;

  // Fixed seed - restored afterwards:
  UInt_t OldSeed = gRandom->GetSeed();
  gRandom->SetSeed(4711);
  CLHEP::HepRandom::setTheSeed(4711);

  // Mg28 -> Al28 at 6 h, compared to the analytic solution:
  MCActivatorParticle Mg28 = CreateParticle(12028, 0.0);
  MCActivatorParticle Al28 = CreateParticle(13028, 0.0);
  Mg28.SetProductionRate(1.0/s);
  const double Activation = 6*3600*s;
  double Expected1 = A.ActivationO1(1.0/s, Mg28.GetDecayConstant(), Activation);
  double Expected2 = A.ActivationO2(1.0/s, Mg28.GetDecayConstant(), 1.0, Al28.GetDecayConstant(), Activation);

  vector<MCActivatorParticle> Chain = { Mg28, Al28 };
  SilenceOutput();
  bool OK = A.ActivateByPartialSimulation(Chain, Activation, 0.0);
  RestoreOutput();
  Passed = EvaluateTrue("ActivateByPartialSimulation()", "Mg28 -> Al28", "The partial simulation succeeds", OK) && Passed;
  Passed = EvaluateNear("ActivateByPartialSimulation()", "Mg28 -> Al28", "The simulated mother activity agrees with the analytic one within 2%", Chain[0].GetActivation(), Expected1, 0.02*Expected1) && Passed;
  Passed = EvaluateNear("ActivateByPartialSimulation()", "Mg28 -> Al28", "The simulated daughter activity agrees with the analytic one within 2%", Chain[1].GetActivation(), Expected2, 0.02*Expected2) && Passed;

  Chain = { Mg28, Al28 };
  SilenceOutput();
  OK = A.ActivateBySimulation(Chain, Activation, 0.0);
  RestoreOutput();
  Passed = EvaluateTrue("ActivateBySimulation()", "Mg28 -> Al28", "The simulation succeeds", OK) && Passed;
  Passed = EvaluateNear("ActivateBySimulation()", "Mg28 -> Al28", "The simulated mother activity agrees with the analytic one within 2%", Chain[0].GetActivation(), Expected1, 0.02*Expected1) && Passed;
  Passed = EvaluateNear("ActivateBySimulation()", "Mg28 -> Al28", "The simulated daughter activity agrees with the analytic one within 2%", Chain[1].GetActivation(), Expected2, 0.02*Expected2) && Passed;

  gRandom->SetSeed(OldSeed);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestCountsFileFailures()
{
  bool Passed = true;

  // Garbage file - no time:
  {
    const MString CountsFile = GetTemporaryFileName("Garbage.dat");
    Passed = EvaluateTrue("WriteTextFile()", "garbage", "The garbage counts fixture can be written", WriteTextFile(CountsFile, "This is not an isotope file\n12 34 56\n")) && Passed;
    MCActivator A;
    SilenceOutput();
    A.AddCountsFile(CountsFile);
    bool Loaded = A.LoadCountsFiles();
    RestoreOutput();
    Passed = EvaluateFalse("LoadCountsFiles()", "garbage file", "A file without isotope data and time is rejected", Loaded) && Passed;
  }

  // A failed load must not leave the rates of the files loaded before it:
  {
    const MString GoodFile = GetTemporaryFileName("StaleGood.dat");
    const MString BadFile = GetTemporaryFileName("StaleBad.dat");
    const MString OutputFile = GetTemporaryFileName("Stale.act");
    Passed = EvaluateTrue("WriteCountsFile()", "good file", "The good counts fixture can be written", WriteCountsFile(GoodFile, 11024, 0.0, 10, 1*s)) && Passed;
    Passed = EvaluateTrue("WriteCountsFile()", "bad file", "The counts fixture without time can be written", WriteCountsFile(BadFile, 11024, 0.0, 10, 0.0)) && Passed;
    MCActivator A;
    A.SetConstantIrradiation(3600*s);
    SilenceOutput();
    A.AddCountsFile(GoodFile);
    A.AddCountsFile(BadFile);
    bool Loaded = A.LoadCountsFiles();
    bool Calculated = A.CalculateEquilibriumRates();
    A.SetOutputFileName(OutputFile);
    A.SaveOutputFile();
    RestoreOutput();
    Passed = EvaluateFalse("LoadCountsFiles()", "good + bad file", "Loading fails if one file has no time", Loaded) && Passed;
    Passed = EvaluateTrue("CalculateEquilibriumRates()", "after failed load", "The calculation after a failed load succeeds", Calculated) && Passed;
    Passed = Evaluate("LoadCountsFiles()", "after failed load", "A failed load leaves no partially loaded rates behind", GetNumberOfStoredEntries(OutputFile), 0U) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestSaveOutputFileFailure()
{
  bool Passed = true;

  MCActivator A;
  A.SetOutputFileName(GetTemporaryFileName("DoesNotExist") + "/Output.act");
  SilenceOutput();
  bool Saved = A.SaveOutputFile();
  RestoreOutput();
  Passed = EvaluateFalse("SaveOutputFile()", "missing directory", "Saving into a missing directory fails", Saved) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestSeveralExcitations()
{
  bool Passed = true;

  // Co60 and Co60[58.59] in the same volume - in equilibrium the Co60 activity is R(Co60) + 0.9975 R(Co60m):
  const MString CountsFile = GetTemporaryFileName("Co60Both.dat");
  const MString OutputFile = GetTemporaryFileName("Co60Both.act");
  MCIsotopeStore Store;
  Store.SetTime(1*s);
  Store.Add("Volume", 27060, 0.0, 200);
  Store.Add("Volume", 27060, 58.59*keV, 1000);
  SilenceOutput();
  bool Saved = Store.Save(CountsFile);
  RestoreOutput();
  Passed = EvaluateTrue("MCIsotopeStore::Save()", "Co60 + Co60m", "The counts fixture can be written", Saved) && Passed;

  MCActivator A;
  A.SetConstantIrradiation(1e11*s);
  Passed = EvaluateTrue("CalculateEquilibriumRates()", "Co60 + Co60m", "The activation of two excitations of the same isotope succeeds", RunActivation(A, CountsFile, OutputFile)) && Passed;
  double Expected = 200 + 0.9975*1000;
  Passed = EvaluateNear("CalculateEquilibriumRates()", "Co60 + Co60m", "The Co60 activity is the sum of its direct and its IT production", GetStoredValue(OutputFile, 27060, 0.0), Expected, 1e-4*Expected) && Passed;
  Passed = EvaluateNear("CalculateEquilibriumRates()", "Co60 + Co60m", "The Co60m activity is its production rate", GetStoredValue(OutputFile, 27060, 58.59*keV), 1000.0, 1e-4*1000.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestParticleOutputModeChain()
{
  bool Passed = true;

  // Mg28 -> Al28 in particle mode: N = A/lambda, rounded
  const double D1 = log(2.0)/CreateParticle(12028, 0.0).GetHalfLife();
  const double D2 = log(2.0)/CreateParticle(13028, 0.0).GetHalfLife();
  const double Rate = 1.0/s;
  const double Activation = 6*3600*s;

  const MString CountsFile = GetTemporaryFileName("Mg28Particles.dat");
  const MString OutputFile = GetTemporaryFileName("Mg28Particles.act");
  Passed = EvaluateTrue("WriteCountsFile()", "Mg28", "The counts fixture can be written", WriteCountsFile(CountsFile, 12028, 0.0, 10, 10*s)) && Passed;

  MCActivator A;
  A.SetConstantIrradiation(Activation);
  A.SetOutputModeParticles();
  Passed = EvaluateTrue("SetOutputModeParticles()", "Mg28, 6 h", "The activation of Mg28 in particle mode succeeds", RunActivation(A, CountsFile, OutputFile)) && Passed;

  double A1 = Rate*(1 - exp(-D1*Activation));
  double A2 = Rate*(1 + (D1*exp(-D2*Activation) - D2*exp(-D1*Activation))/(D2 - D1));
  Passed = EvaluateNear("SetOutputModeParticles()", "Mg28, 6 h", "The number of Mg28 nuclei is A1/lambda1", GetStoredValue(OutputFile, 12028, 0.0), floor(0.5 + A1/D1), 1.5) && Passed;
  // Al28 is stored per branch (direct and via Al28[30.64]), each rounded:
  Passed = EvaluateNear("SetOutputModeParticles()", "Mg28, 6 h", "The number of Al28 nuclei is A2/lambda2", GetStoredValue(OutputFile, 13028, 0.0), A2/D2, 1.5) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestCooldownOn()
{
  bool Passed = true;

  UTCActivatorAccess A;

  // Single element:
  Passed = EvaluateNear("CooldownOn()", "one element", "One element decays exponentially", A.CooldownOn({ 3.0 }, { 0.2 }, { 1.0 }, 4.0), 3.0*exp(-0.8), 1e-12) && Passed;

  // Five elements against the integration:
  const vector<double> D = { 0.3, 0.07, 0.9, 0.02, 0.15 };
  const vector<double> B = { 1.0, 0.8, 0.6, 0.5, 0.9 };
  const vector<double> N0 = { 2.0, 5.0, 1.0, 7.0, 3.0 };
  const double t = 4.1;
  vector<double> A0(5);
  for (unsigned int k = 0; k < 5; ++k) A0[k] = D[k]*N0[k];
  vector<double> N = IntegrateChain(0.0, D, B, N0, t);
  for (unsigned int n = 1; n <= 5; ++n) {
    vector<double> An(A0.begin(), A0.begin() + n);
    vector<double> Dn(D.begin(), D.begin() + n);
    vector<double> Bn(B.begin(), B.begin() + n);
    double Expected = D[n-1]*N[n-1];
    Passed = EvaluateNear("CooldownOn()", MString(n) + " elements", "The cooldown of the last element matches the integration", A.CooldownOn(An, Dn, Bn, t), Expected, 1e-6*Expected) && Passed;
  }

  // The start activity of the last element is not branched again:
  Passed = EvaluateNear("CooldownOn()", "t = 0", "Without cooldown the last activity is unchanged", A.CooldownOn(A0, D, B, 0.0), A0[4], 1e-9*A0[4]) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestActivateByEquationsLongChains()
{
  bool Passed = true;

  UTCActivatorAccess A;

  // Three radioactive elements with branching and cooldown - uses the analytic cooldown:
  vector<MCActivatorParticle> Chain(3, CreateParticle(12028, 0.0));
  Chain[0].SetHalfLife(1000*s);
  Chain[1].SetHalfLife(300*s);
  Chain[1].SetBranchingRatio(0.4);
  Chain[2].SetHalfLife(5000*s);
  Chain[2].SetBranchingRatio(0.7);
  Chain[0].SetProductionRate(2.0/s);
  const double Activation = 2000*s;
  const double Cooldown = 700*s;
  SilenceOutput();
  bool OK = A.ActivateByEquations(Chain, Activation, Cooldown);
  RestoreOutput();
  Passed = EvaluateTrue("ActivateByEquations()", "3 elements with cooldown", "A three-element chain with cooldown is calculated", OK) && Passed;

  vector<double> D = { log(2.0)/(1000*s), log(2.0)/(300*s), log(2.0)/(5000*s) };
  vector<double> B = { 1.0, 0.4, 0.7 };
  vector<double> N = IntegrateChain(2.0/s, D, B, { 0.0, 0.0, 0.0 }, Activation);
  N = IntegrateChain(0.0, D, B, N, Cooldown);
  for (unsigned int k = 0; k < 3; ++k) {
    double Expected = D[k]*N[k];
    Passed = EvaluateNear("ActivateByEquations()", MString("3 elements with cooldown, element ") + (k+1), "The activity after cooldown matches the integration", Chain[k].GetActivation(), Expected, 1e-6*Expected) && Passed;
  }

  // Five elements - the last one is stable:
  vector<MCActivatorParticle> Five(5, CreateParticle(12028, 0.0));
  vector<double> HalfLives = { 1000, 300, 5000, 70, 0 };
  for (unsigned int p = 0; p < 5; ++p) {
    Five[p].SetHalfLife(HalfLives[p] > 0 ? HalfLives[p]*s : numeric_limits<double>::max());
  }
  Five[0].SetProductionRate(1.0/s);
  SilenceOutput();
  OK = A.ActivateByEquations(Five, Activation, 0.0);
  RestoreOutput();
  Passed = EvaluateTrue("ActivateByEquations()", "5 elements", "A five-element chain with a stable end is calculated", OK) && Passed;
  Passed = EvaluateNear("ActivateByEquations()", "5 elements", "The stable last element has no activity", Five[4].GetActivation(), 0.0, 1e-30) && Passed;
  double Expected4 = A.ActivationO4(1.0/s, log(2.0)/(1000*s), 1.0, log(2.0)/(300*s), 1.0, log(2.0)/(5000*s), 1.0, log(2.0)/(70*s), Activation);
  Passed = EvaluateNear("ActivateByEquations()", "5 elements", "The fourth element follows the analytic activation", Five[3].GetActivation(), Expected4, 1e-9*Expected4) && Passed;

  // Five elements with an unstable last one cannot be calculated with the equations:
  Five[4].SetHalfLife(100*s);
  SilenceOutput();
  OK = A.ActivateByEquations(Five, Activation, 0.0);
  RestoreOutput();
  Passed = EvaluateFalse("ActivateByEquations()", "5 elements, unstable end", "A five-element chain with an unstable end is rejected", OK) && Passed;

  // Single element:
  vector<MCActivatorParticle> One = { CreateParticle(11024, 0.0) };
  One[0].SetProductionRate(1.0/s);
  SilenceOutput();
  OK = A.ActivateByEquations(One, 3600*s, 0.0);
  RestoreOutput();
  Passed = EvaluateTrue("ActivateByEquations()", "1 element", "A single element is calculated", OK) && Passed;
  double Expected1 = A.ActivationO1(1.0/s, One[0].GetDecayConstant(), 3600*s);
  Passed = EvaluateNear("ActivateByEquations()", "1 element", "The single element follows the build-up", One[0].GetActivation(), Expected1, 1e-9*Expected1) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivator::TestPartialSimulationCooldown()
{
  bool Passed = true;

  UTCActivatorAccess A;

  UInt_t OldSeed = gRandom->GetSeed();
  gRandom->SetSeed(4711);
  CLHEP::HepRandom::setTheSeed(4711);

  // Two elements with a branching ratio below one:
  vector<MCActivatorParticle> Chain(2, CreateParticle(12028, 0.0));
  Chain[0].SetHalfLife(1000*s);
  Chain[1].SetHalfLife(5000*s);
  Chain[1].SetBranchingRatio(0.3);
  Chain[0].SetProductionRate(1.0/s);
  const double Activation = 20000*s;
  const double Cooldown = 3000*s;
  SilenceOutput();
  bool OK = A.ActivateByPartialSimulation(Chain, Activation, Cooldown);
  RestoreOutput();
  Passed = EvaluateTrue("ActivateByPartialSimulation()", "2 elements, BR 0.3, cooldown", "The partial simulation with cooldown succeeds", OK) && Passed;

  vector<double> D = { log(2.0)/(1000*s), log(2.0)/(5000*s) };
  vector<double> B = { 1.0, 0.3 };
  vector<double> N = IntegrateChain(1.0/s, D, B, { 0.0, 0.0 }, Activation);
  N = IntegrateChain(0.0, D, B, N, Cooldown);
  for (unsigned int k = 0; k < 2; ++k) {
    double Expected = D[k]*N[k];
    Passed = EvaluateNear("ActivateByPartialSimulation()", MString("2 elements, BR 0.3, cooldown, element ") + (k+1), "The simulated activity after cooldown agrees with the integration within 2%", Chain[k].GetActivation(), Expected, 0.02*Expected) && Passed;
  }

  // A single stable element:
  vector<MCActivatorParticle> Stable = { CreateParticle(8016, 0.0) };
  SilenceOutput();
  OK = A.ActivateByPartialSimulation(Stable, Activation, 0.0);
  RestoreOutput();
  Passed = EvaluateTrue("ActivateByPartialSimulation()", "stable", "The partial simulation of a stable element succeeds", OK) && Passed;
  Passed = EvaluateNear("ActivateByPartialSimulation()", "stable", "A stable element has no activity", Stable[0].GetActivation(), 0.0, 1e-30) && Passed;

  gRandom->SetSeed(OldSeed);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTCActivator Test;
  return Test.Run() == true ? 0 : 1;
}
