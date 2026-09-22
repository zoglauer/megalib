/*
 * MCPrimaryGeneratorAction.cc
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


