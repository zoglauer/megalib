/*
 * MCPhysicsListParticles.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Implementation of MCPhysicsListParticles:
 * Cares about the initialization of particles, processes and cuts
 *
 */

#ifndef ___MCPhysicsListParticles___
#define ___MCPhysicsListParticles___

// Geant4:
#include "globals.hh"
#include "G4VPhysicsConstructor.hh"


/******************************************************************************/



class MCPhysicsListParticles: public G4VPhysicsConstructor
{
  // public interface:
 public:
  /// Default constructor
  explicit MCPhysicsListParticles(const G4String& name = "Particles");
  /// Default destructor
  virtual ~MCPhysicsListParticles();

  /// Automatically invoked during Construct(): 
  /// all particle types will be instantiated
  virtual void ConstructParticle();
 
  /// Automatically invoked during Construct(): 
  /// all physics processes will be instantiated and registered to the process manager of each particle type 
  virtual void ConstructProcess();


  // protected methods:
protected:
  

  // protected members:
protected:


  // private members:
private:

 
};

#endif


/*
 * MCPhysicsListParticles.hh: the end...
 ******************************************************************************/







