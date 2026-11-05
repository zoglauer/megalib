/*
 * UTPhysicsListParticles.cc
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
#include "MCPhysicsListParticles.hh"

// MEGAlib:
#include "MUnitTest.h"

// Geant4:
#include "G4Alpha.hh"
#include "G4Gamma.hh"
#include "G4GenericIon.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VPhysicsConstructor.hh"

// Standard lib:
#include <string>
#include <vector>
using namespace std;


//! Unit test class for MCPhysicsListParticles
class UTPhysicsListParticles : public MUnitTest
{
public:
  //! Default constructor
  UTPhysicsListParticles() : MUnitTest("UTPhysicsListParticles") {}
  //! Default destructor
  virtual ~UTPhysicsListParticles() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test construction, the name, and the base class
  bool TestConstruction();
  //! Test that all particles which can appear in a simulation are created
  bool TestConstructParticle();
  //! Test the masses and charges of important particles
  bool TestParticleProperties();
  //! Test calling ConstructParticle and ConstructProcess repeatedly
  bool TestRepeatedCalls();
  //! Test that ConstructProcess adds no processes
  bool TestConstructProcess();
};


////////////////////////////////////////////////////////////////////////////////


bool UTPhysicsListParticles::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestConstructParticle() && Passed;
  Passed = TestParticleProperties() && Passed;
  Passed = TestRepeatedCalls() && Passed;
  Passed = TestConstructProcess() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPhysicsListParticles::TestConstruction()
{
  bool Passed = true;

  MCPhysicsListParticles Default;
  Passed = Evaluate("MCPhysicsListParticles()", "default", "The default name of the physics constructor is Particles", string(Default.GetPhysicsName().c_str()), string("Particles")) && Passed;
  G4VPhysicsConstructor* Base = &Default;
  Passed = EvaluateTrue("MCPhysicsListParticles()", "base class", "The class is a Geant4 physics constructor which can be cast back", dynamic_cast<MCPhysicsListParticles*>(Base) == &Default) && Passed;

  MCPhysicsListParticles Named("MyParticles");
  Passed = Evaluate("MCPhysicsListParticles(name)", "MyParticles", "The name is stored", string(Named.GetPhysicsName().c_str()), string("MyParticles")) && Passed;

  // The constructor must not create particles - that is done in ConstructParticle:
  Passed = EvaluateTrue("MCPhysicsListParticles()", "no particles yet", "No particles exist before ConstructParticle is called", G4ParticleTable::GetParticleTable()->FindParticle("gamma") == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPhysicsListParticles::TestConstructParticle()
{
  bool Passed = true;

  MCPhysicsListParticles Particles;
  Particles.ConstructParticle();

  G4ParticleTable* Table = G4ParticleTable::GetParticleTable();
  vector<string> Names = {
    // Bosons:
    "gamma", "geantino", "opticalphoton",
    // Leptons:
    "e-", "e+", "mu-", "mu+", "tau-", "tau+", "nu_e", "anti_nu_e", "nu_mu", "anti_nu_mu", "nu_tau", "anti_nu_tau",
    // Mesons:
    "pi+", "pi-", "pi0", "kaon+", "kaon-", "kaon0", "anti_kaon0", "kaon0L", "kaon0S", "eta", "eta_prime",
    // Baryons:
    "proton", "anti_proton", "neutron", "anti_neutron", "lambda", "anti_lambda", "sigma+", "sigma-", "sigma0", "xi-", "xi0", "omega-",
    // Light ions and the generic ion:
    "deuteron", "triton", "He3", "alpha", "GenericIon"
  };
  for (const string& Name: Names) {
    Passed = EvaluateTrue("ConstructParticle()", Name.c_str(), MString("The particle ") + Name.c_str() + " is created", Table->FindParticle(Name) != nullptr) && Passed;
  }

  // Short lived resonances:
  Passed = EvaluateTrue("ConstructParticle()", "short lived", "Short lived particles are created", Table->FindParticle("rho0") != nullptr || Table->FindParticle("B+") != nullptr || Table->FindParticle("D+") != nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPhysicsListParticles::TestParticleProperties()
{
  bool Passed = true;

  G4ParticleTable* Table = G4ParticleTable::GetParticleTable();
  G4ParticleDefinition* Electron = Table->FindParticle("e-");
  G4ParticleDefinition* Positron = Table->FindParticle("e+");
  G4ParticleDefinition* Proton = Table->FindParticle("proton");
  G4ParticleDefinition* Neutron = Table->FindParticle("neutron");
  G4ParticleDefinition* Alpha = Table->FindParticle("alpha");
  G4ParticleDefinition* Deuteron = Table->FindParticle("deuteron");
  G4ParticleDefinition* Triton = Table->FindParticle("triton");
  G4ParticleDefinition* He3 = Table->FindParticle("He3");

  Passed = EvaluateTrue("ConstructParticle()", "properties", "The particles for the property checks exist", Electron != nullptr && Positron != nullptr && Proton != nullptr && Neutron != nullptr && Alpha != nullptr && Deuteron != nullptr && Triton != nullptr && He3 != nullptr) && Passed;
  if (Electron == nullptr || Positron == nullptr || Proton == nullptr || Neutron == nullptr || Alpha == nullptr || Deuteron == nullptr || Triton == nullptr || He3 == nullptr) return false;

  Passed = EvaluateNear("ConstructParticle()", "electron", "The electron mass is 511 keV", Electron->GetPDGMass()/keV, 510.999, 0.01) && Passed;
  Passed = EvaluateNear("ConstructParticle()", "electron", "The electron charge is -1", Electron->GetPDGCharge()/eplus, -1.0, 1e-12) && Passed;
  Passed = EvaluateNear("ConstructParticle()", "positron", "The positron charge is +1", Positron->GetPDGCharge()/eplus, 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("ConstructParticle()", "proton", "The proton mass is 938.27 MeV", Proton->GetPDGMass()/MeV, 938.272, 0.001) && Passed;
  Passed = EvaluateNear("ConstructParticle()", "neutron", "The neutron mass is 939.565 MeV", Neutron->GetPDGMass()/MeV, 939.565, 0.001) && Passed;
  Passed = EvaluateNear("ConstructParticle()", "neutron", "The neutron is neutral", Neutron->GetPDGCharge(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("ConstructParticle()", "alpha", "The alpha particle has charge 2", Alpha->GetPDGCharge()/eplus, 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("ConstructParticle()", "alpha", "The alpha particle mass is 3727.4 MeV", Alpha->GetPDGMass()/MeV, 3727.38, 0.05) && Passed;
  Passed = EvaluateNear("ConstructParticle()", "deuteron", "The deuteron has charge 1", Deuteron->GetPDGCharge()/eplus, 1.0, 1e-12) && Passed;
  Passed = Evaluate("ConstructParticle()", "deuteron", "The deuteron has baryon number 2", Deuteron->GetBaryonNumber(), 2) && Passed;
  Passed = Evaluate("ConstructParticle()", "triton", "The triton has baryon number 3", Triton->GetBaryonNumber(), 3) && Passed;
  Passed = EvaluateNear("ConstructParticle()", "He3", "The He3 has charge 2", He3->GetPDGCharge()/eplus, 2.0, 1e-12) && Passed;
  Passed = Evaluate("ConstructParticle()", "He3", "The He3 has baryon number 3", He3->GetBaryonNumber(), 3) && Passed;

  // The same definitions are returned by the Geant4 singletons:
  Passed = EvaluateTrue("ConstructParticle()", "singletons", "The particle table and the singletons agree on the gamma", Table->FindParticle("gamma") == G4Gamma::Definition()) && Passed;
  Passed = EvaluateTrue("ConstructParticle()", "singletons", "The particle table and the singletons agree on the alpha", Alpha == G4Alpha::Definition()) && Passed;
  Passed = EvaluateTrue("ConstructParticle()", "singletons", "The particle table and the singletons agree on the generic ion", Table->FindParticle("GenericIon") == G4GenericIon::Definition()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPhysicsListParticles::TestRepeatedCalls()
{
  bool Passed = true;

  G4ParticleTable* Table = G4ParticleTable::GetParticleTable();
  int NBefore = Table->size();
  G4ParticleDefinition* GammaBefore = Table->FindParticle("gamma");

  MCPhysicsListParticles Particles;
  Particles.ConstructParticle();
  Particles.ConstructParticle();
  Passed = Evaluate("ConstructParticle()", "twice", "Calling ConstructParticle again does not create additional particles", (int) Table->size(), NBefore) && Passed;
  Passed = EvaluateTrue("ConstructParticle()", "twice", "Calling ConstructParticle again keeps the particle definitions", Table->FindParticle("gamma") == GammaBefore) && Passed;

  // A second constructor object does the same:
  MCPhysicsListParticles Other("Other");
  Other.ConstructParticle();
  Passed = Evaluate("ConstructParticle()", "second object", "A second physics constructor does not create additional particles", (int) Table->size(), NBefore) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTPhysicsListParticles::TestConstructProcess()
{
  bool Passed = true;

  MCPhysicsListParticles Particles;
  G4ParticleDefinition* Gamma = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
  Passed = EvaluateTrue("ConstructProcess()", "setup", "The gamma exists", Gamma != nullptr) && Passed;
  if (Gamma == nullptr) return false;

  // The physics processes are defined in their own classes, thus this function does nothing:
  Particles.ConstructProcess();
  Passed = EvaluateTrue("ConstructProcess()", "gamma", "ConstructProcess does not attach a process manager or processes", Gamma->GetProcessManager() == nullptr || Gamma->GetProcessManager()->GetProcessList()->size() == 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTPhysicsListParticles Test;
  return Test.Run() == true ? 0 : 1;
}
