/*
 * MCPhysicsListEMLivermore.cxx
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
#include "MCPhysicsListEMLivermore.hh"

// Geant4:
#include "G4ProcessManager.hh"

#include "G4LowEnergyPhotoElectric.hh"
#include "G4LowEnergyCompton.hh"
#include "G4LowEnergyRayleigh.hh"
#include "G4LowEnergyGammaConversion.hh"
#include "G4LowEnergyIonisation.hh"
#include "G4LowEnergyBremsstrahlung.hh"

#include "G4eMultipleScattering.hh"
#include "G4MuMultipleScattering.hh"
#include "G4hMultipleScattering.hh"

#include "G4eIonisation.hh"
#include "G4eBremsstrahlung.hh"
#include "G4eplusAnnihilation.hh"

#include "G4MuIonisation.hh"
#include "G4MuBremsstrahlung.hh"
#include "G4MuPairProduction.hh"

#include "G4ionIonisation.hh"
#include "G4hLowEnergyIonisation.hh"

// MEGAlib:
#include "MStreams.h"


/******************************************************************************/


/******************************************************************************
 * Default constructor: Does nothing
 */
MCPhysicsListEMLivermore::MCPhysicsListEMLivermore(bool ActivateFluorescence) :
  G4VPhysicsConstructor("Livermore"), m_ActivateFluorescence(ActivateFluorescence)
{
  // Intentionally left blank
}


/******************************************************************************
 * Default destructor: Does nothing
 */
MCPhysicsListEMLivermore::~MCPhysicsListEMLivermore()
{
  // Intentionally left blank
}


/******************************************************************************
 * Create all particles which can appear during the simulation
 */
void MCPhysicsListEMLivermore::ConstructParticle()
{
  // Intentionally left blank
}


/******************************************************************************
 * Define all transportation processes for all particle types based on
 * electronmagnetic interaction
 */
void MCPhysicsListEMLivermore::ConstructProcess()
{
  // Initial comments for NON-discrete processes:
  // The three numbers correspond to 3 kind of actions: 
  // AtRest, AlongStep, PostStep
  // -1 means means no action, 0 first in the list, 1 second in the list
  // For along and post step G4Transportation is the default process, thus 
  // thus always start with 1 for AlongStep and PostStep

  theParticleIterator->reset();
  while ((*theParticleIterator)()) {
    G4ParticleDefinition* Particle = theParticleIterator->value();
    G4ProcessManager* Manager = Particle->GetProcessManager();
    G4String ParticleName = Particle->GetParticleName();
    
    // Concerning all photo effects:
    // According to A. Lechner at IEEE 2007 the thresholds 
    // have to be set at 250 eV in order to get the best results

    if (ParticleName == "gamma") {
      G4LowEnergyPhotoElectric* Photo = new G4LowEnergyPhotoElectric();
      // Fluorescence should be automatically active
      Photo->ActivateAuger(true);
      Photo->SetCutForLowEnSecPhotons(0.250 * keV);
      Photo->SetCutForLowEnSecElectrons(0.250 * keV);
      Photo->SetAngularGenerator("standard");
      Manager->AddDiscreteProcess(Photo);
      Manager->AddDiscreteProcess(new G4LowEnergyCompton());
      Manager->AddDiscreteProcess(new G4LowEnergyRayleigh());
      Manager->AddDiscreteProcess(new G4LowEnergyGammaConversion());
    } 
    // electrons
    else if (ParticleName == "e-") {
      // Unfortunately we need to include the STANDARD multiple scattering
      // package to take care of higher energies
      Manager->AddProcess(new G4eMultipleScattering(), -1, 1, 1);
      
      // Concerning low energy ionization processes
      // According to A. Lechner at IEEE 2007 the thresholds 
      // have to be set to at least 250 eV in order to get the best results
      
      G4LowEnergyIonisation* Ionisation = new G4LowEnergyIonisation();
      Ionisation->ActivateAuger(true);
      Ionisation->ActivateFluorescence(m_ActivateFluorescence);
      Ionisation->SetCutForLowEnSecPhotons(0.250 * keV);  
      Ionisation->SetCutForLowEnSecElectrons(0.250 * keV);
      Manager->AddProcess(Ionisation,                      -1, 2, 2);
      // For G4LowEnergyBremsstrahlung -1, -1, 3 is required
      Manager->AddProcess(new G4LowEnergyBremsstrahlung(), -1,-1, 3);
    }
    // Positron
    else if (ParticleName == "e+") {     
      Manager->AddProcess(new G4eMultipleScattering(),  -1, 1, 1);
      
      // Low energy part for positrons is not implemented correctly 
      // in Geant4.9.0 and lower....
      Manager->AddProcess(new G4eIonisation(),         -1, 2, 2);
      // For G4eBremsstrahlung -1, 3, 3 is suggested
      Manager->AddProcess(new G4eBremsstrahlung(),     -1, 3, 3);
      Manager->AddProcess(new G4eplusAnnihilation(),    0,-1, 4); 
    } 
    // Muons  
    else if (ParticleName == "mu+" || 
             ParticleName == "mu-") {
      Manager->AddProcess(new G4MuMultipleScattering(), -1, 1, 1);
      Manager->AddProcess(new G4MuIonisation(),        -1, 2, 2);
      Manager->AddProcess(new G4MuBremsstrahlung(),    -1, 3, 3);
      Manager->AddProcess(new G4MuPairProduction(),    -1, 4, 4);       
    }
    // Alpha particles and other ions
    else if (ParticleName == "alpha" || 
             ParticleName == "He3" ||
             ParticleName == "GenericIon" ) { 
      Manager->AddProcess(new G4hMultipleScattering,   -1, 1, 1);
      Manager->AddProcess(new G4ionIonisation,         -1, 2, 2);
    }
    // all others charged particles (hadrons) except geantino
    else if ((!Particle->IsShortLived()) &&
	       (Particle->GetPDGCharge() != 0.0) && 
	       (Particle->GetParticleName() != "chargedgeantino")) {

      Manager->AddProcess(new G4hMultipleScattering(), -1, 1, 1);
      
      // We seperate here between high-energy and low-energy packages: 
      G4hLowEnergyIonisation* Ionisation = new G4hLowEnergyIonisation;
      Ionisation->ActivateAugerElectronProduction(true);
      Ionisation->SetFluorescence(m_ActivateFluorescence);
      Ionisation->SetCutForSecondaryPhotons(0.250 * keV);  
      Ionisation->SetCutForAugerElectrons(0.250 * keV);
      Manager->AddProcess(Ionisation,                  -1, 2, 2);
    }
  } 

  return;
}


/*
 * MCPhysicsListEMLivermore.cc: the end...
 ******************************************************************************/
