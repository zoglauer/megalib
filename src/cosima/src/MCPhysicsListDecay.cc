/*
 * MCPhysicsListDecay.cxx
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
#include "MCPhysicsListDecay.hh"

// Geant4:
#include "G4ProcessManager.hh"

#include "G4Decay.hh"
#include "G4RadioactiveDecay.hh"

// MEGAlib:
#include "MStreams.h"


/******************************************************************************/


/******************************************************************************
 * Default constructor: Does nothing
 */
MCPhysicsListDecay::MCPhysicsListDecay(const G4String& name) :
  G4VPhysicsConstructor(name)
{
  // Intentionally left blank
}


/******************************************************************************
 * Default destructor: Does nothing
 */
MCPhysicsListDecay::~MCPhysicsListDecay()
{
  // Intentionally left blank
}


/******************************************************************************
 * Create all particles which can appear during the simulation
 */
void MCPhysicsListDecay::ConstructParticle()
{
  // Intentionally left blank
}


/******************************************************************************
 * Define all transportation processes for all particle types based on
 * electronmagnetic interaction
 */
void MCPhysicsListDecay::ConstructProcess()
{
  // Initial comments for NON-discrete processes:
  // The three numbers correspond to 3 kind of actions: 
  // AtRest, AlongStep, PostStep
  // -1 means means no action, 0 first in the list, 1 second in the list
  // For along and post step G4Transportation is the default process, thus 
  // thus always start with 1 for AlongStep and PostStep

  G4Decay* Decay = new G4Decay();
  G4RadioactiveDecay*  RadioactiveDecay = new G4RadioactiveDecay();

  theParticleIterator->reset();
  while( (*theParticleIterator)() ){
    G4ParticleDefinition* Particle = theParticleIterator->value();
    G4ProcessManager* Manager = Particle->GetProcessManager();
    G4String ParticleName = Particle->GetParticleName();

    if (ParticleName == "GenericIon") {
      Manager->AddProcess(RadioactiveDecay, 0, -1, 3);

    } else if (Decay->IsApplicable(*Particle)) { 

      Manager->AddProcess(Decay);

      // set ordering for PostStepDoIt and AtRestDoIt
      Manager->SetProcessOrdering(Decay, idxPostStep);
      Manager->SetProcessOrdering(Decay, idxAtRest);

    }
  }

  return;
}


/*
 * MCPhysicsListDecay.cc: the end...
 ******************************************************************************/
