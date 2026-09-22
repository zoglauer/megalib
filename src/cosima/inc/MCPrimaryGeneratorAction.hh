/*
 * MCPrimaryGeneratorAction.hh
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



/******************************************************************************
 *
 * The primary generator action 
 *
 */

#ifndef ___MCPrimaryGeneratorAction___
#define ___MCPrimaryGeneratorAction___

// Geant4:
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ThreeVector.hh"
#include "G4ParticleDefinition.hh"
#include "globals.hh"

// Cosima:
#include "MCParameterFile.hh"

// Forward declarations:
class G4GeneralParticleSource;
class G4Event;

/******************************************************************************/

class MCPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  // public interface:
public:
  /// Constructor, which takes the run parameters as input
  MCPrimaryGeneratorAction(MCParameterFile& RunParameters);
  /// Default destructor
  virtual ~MCPrimaryGeneratorAction();

  /// Generate the initial tracks of this event
  void GeneratePrimaries(G4Event* Event);

  // protected methods:
protected:
  
  // public enumerations:
public:


  // protected members:
protected:


  // private members:
private:
  /// The run parameters
  MCParameterFile& m_RunParameters;

  /// The particle generator
  G4GeneralParticleSource* m_ParticleGun;
};

#endif


/*
 * MCPhysicsList.hh: the end...
 ******************************************************************************/

