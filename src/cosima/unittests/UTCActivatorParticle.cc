/*
 * UTCActivatorParticle.cc
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
#include "MCActivatorParticle.hh"

// MEGAlib:
#include "MStreams.h"
#include "MString.h"
#include "MUnitTest.h"

// Geant4:
#include "G4BaryonConstructor.hh"
#include "G4BosonConstructor.hh"
#include "G4Gamma.hh"
#include "G4GenericIon.hh"
#include "G4Ions.hh"
#include "G4IonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4NuclideTable.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"
#include "G4SystemOfUnits.hh"

// Standard lib:
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>
using namespace std;


//! Unit test class for MCActivatorParticle
class UTCActivatorParticle : public MUnitTest
{
public:
  //! Default constructor
  UTCActivatorParticle() : MUnitTest("UTCActivatorParticle"), m_CoutBuffer(nullptr) {}
  //! Default destructor
  virtual ~UTCActivatorParticle() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the default constructor
  bool TestDefaultConstruction();
  //! Test the simple setters and getters
  bool TestGettersSetters();
  //! Test the creation of the particle definition
  bool TestSetIDAndExcitation();
  //! Test copy construction and assignment
  bool TestCopyAndAssignment();
  //! Test the equal operator
  bool TestEqualOperator();
  //! Test the decay constant
  bool TestDecayConstant();
  //! Test the stability check
  bool TestIsStable();
  //! Test the decay paths
  bool TestPaths();

  //! Initialize the Geant4 particles and the nuclide table as done in cosima
  void InitializeGeant4();
  //! Redirect cout and the MEGAlib streams
  void SilenceOutput();
  //! Restore cout and the MEGAlib streams
  void RestoreOutput();
  //! Create a particle silently
  MCActivatorParticle Create(unsigned int ID, double Excitation);
  //! Return the excitation of the Geant4 definition of the particle (keV), or -1 if it is not an ion
  double GetDefinitionExcitation(const MCActivatorParticle& P);

  //! The original cout buffer while silenced
  streambuf* m_CoutBuffer;
  //! The sink for the silenced output
  ostringstream m_Sink;
};


////////////////////////////////////////////////////////////////////////////////


bool UTCActivatorParticle::Run()
{
  bool Passed = true;

  InitializeGeant4();

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestSetIDAndExcitation() && Passed;
  Passed = TestCopyAndAssignment() && Passed;
  Passed = TestEqualOperator() && Passed;
  Passed = TestDecayConstant() && Passed;
  Passed = TestIsStable() && Passed;
  Passed = TestPaths() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


void UTCActivatorParticle::InitializeGeant4()
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


void UTCActivatorParticle::SilenceOutput()
{
  DisableDefaultStreams();
  m_Sink.str("");
  m_CoutBuffer = cout.rdbuf(m_Sink.rdbuf());
}


////////////////////////////////////////////////////////////////////////////////


void UTCActivatorParticle::RestoreOutput()
{
  if (m_CoutBuffer != nullptr) cout.rdbuf(m_CoutBuffer);
  m_CoutBuffer = nullptr;
  EnableDefaultStreams();
}


////////////////////////////////////////////////////////////////////////////////


MCActivatorParticle UTCActivatorParticle::Create(unsigned int ID, double Excitation)
{
  SilenceOutput();
  MCActivatorParticle P;
  P.SetIDAndExcitation(ID, Excitation);
  RestoreOutput();
  return P;
}


////////////////////////////////////////////////////////////////////////////////


double UTCActivatorParticle::GetDefinitionExcitation(const MCActivatorParticle& P)
{
  const G4Ions* Ion = dynamic_cast<const G4Ions*>(P.GetDefinition());
  if (Ion == nullptr) return -1;
  return Ion->GetExcitationEnergy()/keV;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivatorParticle::TestDefaultConstruction()
{
  bool Passed = true;

  MCActivatorParticle P;
  Passed = Evaluate("MCActivatorParticle()", "default", "The default ID is zero", P.GetID(), 0U) && Passed;
  Passed = EvaluateNear("MCActivatorParticle()", "default", "The default excitation is zero", P.GetExcitation(), 0.0, 1e-12) && Passed;
  Passed = EvaluateTrue("MCActivatorParticle()", "default", "There is no default particle definition", P.GetDefinition() == nullptr) && Passed;
  Passed = Evaluate("MCActivatorParticle()", "default", "The name tells that the particle is not yet defined", MString(P.GetName()), MString("Not yet defined!")) && Passed;
  Passed = EvaluateNear("MCActivatorParticle()", "default", "The default half life is zero", P.GetHalfLife(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCActivatorParticle()", "default", "The default branching ratio is one", P.GetBranchingRatio(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCActivatorParticle()", "default", "The default production rate is one", P.GetProductionRate(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCActivatorParticle()", "default", "The default counts are zero", P.GetCounts(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCActivatorParticle()", "default", "The default activation is zero", P.GetActivation(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("MCActivatorParticle()", "default", "The default storage marker is zero", P.GetStorageMarker(), 0U) && Passed;
  Passed = EvaluateSize("MCActivatorParticle()", "default", "A default particle represents one path", P.GetPaths().size(), 1) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivatorParticle::TestGettersSetters()
{
  bool Passed = true;

  MCActivatorParticle P;
  P.SetHalfLife(12.5*s);
  Passed = EvaluateNear("SetHalfLife/GetHalfLife", "12.5 s", "The half life is stored", P.GetHalfLife()/s, 12.5, 1e-12) && Passed;
  P.SetBranchingRatio(0.3);
  Passed = EvaluateNear("SetBranchingRatio/GetBranchingRatio", "0.3", "The branching ratio is stored", P.GetBranchingRatio(), 0.3, 1e-12) && Passed;
  P.AddBranchingRatio(0.45);
  Passed = EvaluateNear("AddBranchingRatio()", "0.3 + 0.45", "The branching ratio is added", P.GetBranchingRatio(), 0.75, 1e-12) && Passed;
  P.SetProductionRate(7.0/s);
  Passed = EvaluateNear("SetProductionRate/GetProductionRate", "7/s", "The production rate is stored", P.GetProductionRate()*s, 7.0, 1e-12) && Passed;
  P.SetCounts(123.0);
  Passed = EvaluateNear("SetCounts/GetCounts", "123", "The counts are stored", P.GetCounts(), 123.0, 1e-12) && Passed;
  P.SetActivation(4.5/s);
  Passed = EvaluateNear("SetActivation/GetActivation", "4.5 Bq", "The activation is stored", P.GetActivation()*s, 4.5, 1e-12) && Passed;
  P.SetStorageMarker(3);
  Passed = Evaluate("SetStorageMarker/GetStorageMarker", "3", "The storage marker is stored", P.GetStorageMarker(), 3U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivatorParticle::TestSetIDAndExcitation()
{
  bool Passed = true;

  // Ground state:
  MCActivatorParticle Na24 = Create(11024, 0.0);
  Passed = Evaluate("SetIDAndExcitation()", "Na24", "The ID is stored", Na24.GetID(), 11024U) && Passed;
  Passed = EvaluateTrue("SetIDAndExcitation()", "Na24", "The particle definition is created", Na24.GetDefinition() != nullptr) && Passed;
  Passed = Evaluate("SetIDAndExcitation()", "Na24", "The name is the one of the Geant4 ion", MString(Na24.GetName()), MString("Na24")) && Passed;

  // Isomer:
  MCActivatorParticle Ge77m = Create(32077, 159.71*keV);
  Passed = EvaluateNear("SetIDAndExcitation()", "Ge77[159.71]", "The excitation is stored", Ge77m.GetExcitation()/keV, 159.71, 1e-9) && Passed;
  Passed = EvaluateNear("SetIDAndExcitation()", "Ge77[159.71]", "The definition has the excitation of the level", GetDefinitionExcitation(Ge77m), 159.71, 0.01) && Passed;

  // The same level gives the same definition:
  MCActivatorParticle Ge77m2 = Create(32077, 159.71*keV);
  Passed = EvaluateTrue("SetIDAndExcitation()", "Ge77[159.71] twice", "The same level gives the same Geant4 definition", Ge77m.GetDefinition() == Ge77m2.GetDefinition()) && Passed;

  // A level below the nuclide table threshold (Li8[3210], 4e-21 s) still gets a definition:
  MCActivatorParticle Li8 = Create(3008, 3210*keV);
  Passed = EvaluateTrue("SetIDAndExcitation()", "Li8[3210]", "A level below the nuclide table threshold gets a definition", Li8.GetDefinition() != nullptr) && Passed;
  Passed = EvaluateNear("SetIDAndExcitation()", "Li8[3210]", "The definition has the excitation of the level", GetDefinitionExcitation(Li8), 3210, 0.1) && Passed;

  // Negative excitations are the ground state:
  MCActivatorParticle Negative = Create(11024, -5*keV);
  Passed = EvaluateNear("SetIDAndExcitation()", "negative excitation", "A negative excitation is set to zero", Negative.GetExcitation(), 0.0, 1e-12) && Passed;
  Passed = EvaluateTrue("SetIDAndExcitation()", "negative excitation", "A negative excitation gives the ground state definition", Negative.GetDefinition() == Na24.GetDefinition()) && Passed;

  // Reusing a particle for another isotope must not keep the old definition:
  SilenceOutput();
  MCActivatorParticle Reused;
  Reused.SetIDAndExcitation(11024, 0.0);
  Reused.SetIDAndExcitation(3008, 3210*keV);
  RestoreOutput();
  Passed = Evaluate("SetIDAndExcitation()", "reuse", "The ID of the new isotope is stored", Reused.GetID(), 3008U) && Passed;
  Passed = EvaluateTrue("SetIDAndExcitation()", "reuse", "The definition of the new isotope replaces the old one", Reused.GetDefinition() == Li8.GetDefinition()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivatorParticle::TestCopyAndAssignment()
{
  bool Passed = true;

  MCActivatorParticle A = Create(27060, 58.59*keV);
  A.SetHalfLife(628*s);
  A.SetBranchingRatio(0.4);
  A.SetProductionRate(2.0);
  A.SetCounts(5.0);
  A.SetActivation(6.0);
  A.SetStorageMarker(1);

  MCActivatorParticle B(A);
  Passed = Evaluate("MCActivatorParticle(const MCActivatorParticle&)", "Co60m", "The copy keeps the ID", B.GetID(), 27060U) && Passed;
  Passed = EvaluateNear("MCActivatorParticle(const MCActivatorParticle&)", "Co60m", "The copy keeps the excitation", B.GetExcitation()/keV, 58.59, 1e-9) && Passed;
  Passed = EvaluateTrue("MCActivatorParticle(const MCActivatorParticle&)", "Co60m", "The copy keeps the definition", B.GetDefinition() == A.GetDefinition()) && Passed;
  Passed = EvaluateNear("MCActivatorParticle(const MCActivatorParticle&)", "Co60m", "The copy keeps the half life", B.GetHalfLife()/s, 628.0, 1e-9) && Passed;
  Passed = EvaluateNear("MCActivatorParticle(const MCActivatorParticle&)", "Co60m", "The copy keeps the branching ratio", B.GetBranchingRatio(), 0.4, 1e-12) && Passed;
  Passed = EvaluateNear("MCActivatorParticle(const MCActivatorParticle&)", "Co60m", "The copy keeps the production rate", B.GetProductionRate(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCActivatorParticle(const MCActivatorParticle&)", "Co60m", "The copy keeps the counts", B.GetCounts(), 5.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCActivatorParticle(const MCActivatorParticle&)", "Co60m", "The copy keeps the activation", B.GetActivation(), 6.0, 1e-12) && Passed;
  Passed = Evaluate("MCActivatorParticle(const MCActivatorParticle&)", "Co60m", "The copy keeps the storage marker", B.GetStorageMarker(), 1U) && Passed;
  Passed = EvaluateTrue("MCActivatorParticle(const MCActivatorParticle&)", "Co60m", "The copy represents the same decay path", B.GetPaths().begin()->first == A.GetPaths().begin()->first) && Passed;

  B.SetBranchingRatio(0.9);
  Passed = EvaluateNear("MCActivatorParticle(const MCActivatorParticle&)", "independence", "Changing the copy does not change the original", A.GetBranchingRatio(), 0.4, 1e-12) && Passed;

  MCActivatorParticle C = Create(11024, 0.0);
  C = A;
  Passed = Evaluate("operator=", "Co60m", "The assigned particle has the ID", C.GetID(), 27060U) && Passed;
  Passed = EvaluateTrue("operator=", "Co60m", "The assigned particle has the definition", C.GetDefinition() == A.GetDefinition()) && Passed;
  Passed = EvaluateTrue("operator=", "Co60m", "The assigned particle represents the same decay path", C.GetPaths().begin()->first == A.GetPaths().begin()->first) && Passed;

  // Merged paths are copied too:
  MCActivatorParticle D = Create(27060, 58.59*keV);
  D.SetBranchingRatio(0.1);
  A.MergePaths(D);
  MCActivatorParticle E(A);
  Passed = EvaluateSize("MCActivatorParticle(const MCActivatorParticle&)", "merged", "The copy keeps all merged paths", E.GetPaths().size(), 2) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivatorParticle::TestEqualOperator()
{
  bool Passed = true;

  MCActivatorParticle A = Create(27060, 58.59*keV);
  A.SetHalfLife(628*s);
  A.SetBranchingRatio(0.4);

  MCActivatorParticle B = Create(27060, 58.59*keV);
  B.SetHalfLife(628*s);
  B.SetBranchingRatio(0.4);
  B.SetProductionRate(99.0);
  B.SetCounts(99.0);
  B.SetActivation(99.0);
  B.SetStorageMarker(1);
  Passed = EvaluateTrue("operator==", "same base data", "Particles with the same base data are equal - production rate, counts, activation, and marker are ignored", A == B) && Passed;

  MCActivatorParticle C = B;
  C.SetHalfLife(629*s);
  Passed = EvaluateFalse("operator==", "different half life", "Particles with different half lives are not equal", A == C) && Passed;
  C = B;
  C.SetBranchingRatio(0.5);
  Passed = EvaluateFalse("operator==", "different branching ratio", "Particles with different branching ratios are not equal", A == C) && Passed;
  MCActivatorParticle D = Create(27060, 0.0);
  D.SetHalfLife(628*s);
  D.SetBranchingRatio(0.4);
  Passed = EvaluateFalse("operator==", "different excitation", "Particles with different excitations are not equal", A == D) && Passed;
  MCActivatorParticle E = Create(26060, 58.59*keV);
  E.SetHalfLife(628*s);
  E.SetBranchingRatio(0.4);
  Passed = EvaluateFalse("operator==", "different ID", "Particles with different IDs are not equal", A == E) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivatorParticle::TestDecayConstant()
{
  bool Passed = true;

  MCActivatorParticle P;
  P.SetHalfLife(10*s);
  Passed = EvaluateNear("GetDecayConstant()", "10 s", "The decay constant is ln 2 divided by the half life", P.GetDecayConstant()*s, log(2.0)/10.0, 1e-12) && Passed;
  P.SetHalfLife(numeric_limits<double>::max());
  Passed = EvaluateNear("GetDecayConstant()", "stable", "A stable particle has decay constant zero", P.GetDecayConstant(), 0.0, 1e-300) && Passed;
  P.SetHalfLife(0.0);
  Passed = EvaluateTrue("GetDecayConstant()", "immediate", "An immediately decaying particle has the maximum decay constant", P.GetDecayConstant() == numeric_limits<double>::max()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivatorParticle::TestIsStable()
{
  bool Passed = true;

  Passed = EvaluateTrue("IsStable()", "O16", "A stable isotope is stable", MCActivatorParticle::IsStable(Create(8016, 0.0).GetDefinition())) && Passed;
  Passed = EvaluateFalse("IsStable()", "Na24", "A radioactive isotope is not stable", MCActivatorParticle::IsStable(Create(11024, 0.0).GetDefinition())) && Passed;
  Passed = EvaluateFalse("IsStable()", "Co60[58.59]", "An isomer is not stable", MCActivatorParticle::IsStable(Create(27060, 58.59*keV).GetDefinition())) && Passed;
  Passed = EvaluateTrue("IsStable()", "gamma", "A photon is stable", MCActivatorParticle::IsStable(G4Gamma::Definition())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCActivatorParticle::TestPaths()
{
  bool Passed = true;

  // A new particle represents its own path with its current values:
  MCActivatorParticle A = Create(27060, 0.0);
  A.SetBranchingRatio(0.2);
  A.SetProductionRate(2.0);
  auto PathsA = A.GetPaths();
  Passed = EvaluateSize("GetPaths()", "new particle", "A new particle represents one path", PathsA.size(), 1) && Passed;
  Passed = EvaluateNear("GetPaths()", "new particle", "The path has the current branching ratio", PathsA.begin()->second.first, 0.2, 1e-12) && Passed;
  Passed = EvaluateNear("GetPaths()", "new particle", "The path has the current production rate", PathsA.begin()->second.second, 2.0, 1e-12) && Passed;

  // Every SetIDAndExcitation creates a new path:
  MCActivatorParticle B = Create(27060, 0.0);
  Passed = EvaluateTrue("SetIDAndExcitation()", "two particles", "Two new particles represent different paths", A.GetPaths().begin()->first != B.GetPaths().begin()->first) && Passed;

  // Merging separate paths adds their values:
  B.SetBranchingRatio(0.3);
  B.SetProductionRate(3.0);
  MCActivatorParticle AB = A;
  AB.MergePaths(B);
  Passed = EvaluateSize("MergePaths()", "separate paths", "The merged particle represents both paths", AB.GetPaths().size(), 2) && Passed;
  Passed = EvaluateNear("MergePaths()", "separate paths", "The branching ratios are added", AB.GetBranchingRatio(), 0.5, 1e-12) && Passed;
  Passed = EvaluateNear("MergePaths()", "separate paths", "The production rates are added", AB.GetProductionRate(), 5.0, 1e-12) && Passed;

  // Merging a copy of the same path adds nothing:
  MCActivatorParticle ACopy = A;
  MCActivatorParticle AA = A;
  AA.MergePaths(ACopy);
  Passed = EvaluateSize("MergePaths()", "same path", "Merging the same path keeps one path", AA.GetPaths().size(), 1) && Passed;
  Passed = EvaluateNear("MergePaths()", "same path", "Merging the same path does not add the branching ratio", AA.GetBranchingRatio(), 0.2, 1e-12) && Passed;

  // Partial overlap: {A, B} and {B, C} give {A, B, C}:
  MCActivatorParticle C = Create(27060, 0.0);
  C.SetBranchingRatio(0.5);
  C.SetProductionRate(5.0);
  MCActivatorParticle BC = B;
  BC.MergePaths(C);
  MCActivatorParticle ABC = AB;
  ABC.MergePaths(BC);
  Passed = EvaluateSize("MergePaths()", "partial overlap", "The merged particle represents three paths", ABC.GetPaths().size(), 3) && Passed;
  Passed = EvaluateNear("MergePaths()", "partial overlap", "The shared path is counted once", ABC.GetBranchingRatio(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("MergePaths()", "partial overlap", "The production rates of the shared path are counted once", ABC.GetProductionRate(), 10.0, 1e-12) && Passed;

  // The merged particle keeps its identity:
  Passed = Evaluate("MergePaths()", "partial overlap", "The ID is kept", ABC.GetID(), 27060U) && Passed;

  // A new SetIDAndExcitation forgets the merged paths:
  SilenceOutput();
  ABC.SetIDAndExcitation(11024, 0.0);
  RestoreOutput();
  Passed = EvaluateSize("SetIDAndExcitation()", "after merge", "A new particle represents only its own path", ABC.GetPaths().size(), 1) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTCActivatorParticle Test;
  return Test.Run() == true ? 0 : 1;
}
