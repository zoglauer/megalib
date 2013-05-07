/*
 * MCPhysicsListEMStandard.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Implementation of MCPhysicsListEMStandard:
 * Cares about the initialization of particles, processes and cuts
 *
 */

#ifndef ___MCPhysicsListEMStandard___
#define ___MCPhysicsListEMStandard___

// Geant4:
#include "globals.hh"
#include "G4VPhysicsConstructor.hh"


/******************************************************************************/



class MCPhysicsListEMStandard: public G4VPhysicsConstructor
{
  // public interface:
 public:
  /// Default constructor
  explicit MCPhysicsListEMStandard(const G4String& name = "EMStandard");
  /// Default destructor
  virtual ~MCPhysicsListEMStandard();

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
 * MCPhysicsListEMStandard.hh: the end...
 ******************************************************************************/







