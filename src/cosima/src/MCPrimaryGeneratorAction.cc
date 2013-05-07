/*
 * MCPrimaryGeneratorAction.cxx
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
#include "MCCommon.hh"
#include "MCPrimaryGeneratorAction.hh"
#include "MCRun.hh"
#include "MCSource.hh"
#include "MCEventAction.hh"
#include "MCRunManager.hh"
#include "MCSteppingAction.hh"
#include "MCTrackingAction.hh"

// Geant4:
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4GeneralParticleSource.hh"
#include "G4UImanager.hh"
#include "G4EventManager.hh"
#include "globals.hh"
#include "Randomize.hh"

// MEGAlib:
#include "MStreams.h"

// Standard lib:
#include <limits>
using namespace std;


/******************************************************************************
 * Default constructor
 */
MCPrimaryGeneratorAction::MCPrimaryGeneratorAction(MCParameterFile& Run) : 
  m_RunParameters(Run)
{
  m_ParticleGun = new G4GeneralParticleSource();
}


/******************************************************************************
 * Default destructor
 */
MCPrimaryGeneratorAction::~MCPrimaryGeneratorAction()
{
  delete m_ParticleGun;
}


/******************************************************************************
 * Generate the start vertex (energy, position, momentum, polarisation)
 * of the new event
 */
void MCPrimaryGeneratorAction::GeneratePrimaries(G4Event* Event)
{
  m_RunParameters.GetCurrentRun().GeneratePrimaries(Event, m_ParticleGun);
}  


/*
 * MCPhysicsList.cc: the end...
 ******************************************************************************/


