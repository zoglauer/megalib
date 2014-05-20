/*
 * MCPrimaryGeneratorAction.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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

