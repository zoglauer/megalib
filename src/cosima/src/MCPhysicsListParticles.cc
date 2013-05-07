/*
 * MCPhysicsListParticles.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Cosima:
#include "MCPhysicsListParticles.hh"

// Geant4:
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4ParticleTable.hh"
#include "G4LeptonConstructor.hh"
#include "G4BosonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4BaryonConstructor.hh"
#include "G4ShortLivedConstructor.hh"

// MEGAlib:
#include "MStreams.h"


/******************************************************************************/


/******************************************************************************
 * Default constructor: Does nothing
 */
MCPhysicsListParticles::MCPhysicsListParticles(const G4String& name) :
  G4VPhysicsConstructor(name)
{
  // Intentionally left blank
}


/******************************************************************************
 * Default destructor: Does nothing
 */
MCPhysicsListParticles::~MCPhysicsListParticles()
{
  // Intentionally left blank
}


/******************************************************************************
 * Create all particles which can appear during the simulation
 */
void MCPhysicsListParticles::ConstructParticle()
{
  // Define the main particle classes
  G4LeptonConstructor lepton;
  lepton.ConstructParticle();
 
  G4BosonConstructor boson;
  boson.ConstructParticle();

  G4MesonConstructor meson;
  meson.ConstructParticle();

  G4BaryonConstructor baryon;
  baryon.ConstructParticle();

  G4ShortLivedConstructor shortLived;
  shortLived.ConstructParticle();

  // All the other stuff not defined in the above particle classes
  G4Deuteron::DeuteronDefinition();
  G4Triton::TritonDefinition();
  G4He3::He3Definition();
  G4Alpha::AlphaDefinition();
  G4GenericIon::GenericIonDefinition();
}


/******************************************************************************
 * Define all transportation processes for all particle types based on
 * electronmagnetic interaction
 */
void MCPhysicsListParticles::ConstructProcess()
{
  // Intentionally left blank
  // The physics processes are defined it their own classes
}


/*
 * MCPhysicsListParticles.cc: the end...
 ******************************************************************************/
