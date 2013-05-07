/*
 * MCPhysicsListDecay.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Implementation of MCPhysicsListDecay:
 * Cares about the initialization of particles, processes and cuts
 *
 */

#ifndef ___MCPhysicsListDecay___
#define ___MCPhysicsListDecay___

// Geant4:
#include "globals.hh"
#include "G4VPhysicsConstructor.hh"


/******************************************************************************/



class MCPhysicsListDecay: public G4VPhysicsConstructor
{
  // public interface:
 public:
  /// Default constructor
  explicit MCPhysicsListDecay(const G4String& name = "Decay");
  /// Default destructor
  virtual ~MCPhysicsListDecay();

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
 * MCPhysicsListDecay.hh: the end...
 ******************************************************************************/







