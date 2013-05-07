/*
 * MCPhysicsListEMLivermore.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Implementation of MCPhysicsListEMLivermore:
 * Cares about the initialization of particles, processes and cuts
 *
 */

#ifndef ___MCPhysicsListEMLivermore___
#define ___MCPhysicsListEMLivermore___

// Geant4:
#include "globals.hh"
#include "G4VPhysicsConstructor.hh"


/******************************************************************************/



class MCPhysicsListEMLivermore: public G4VPhysicsConstructor
{
  // public interface:
 public:
  /// Default constructor
  explicit MCPhysicsListEMLivermore(bool ActivateFluorescence = false);
  /// Default destructor
  virtual ~MCPhysicsListEMLivermore();

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
  /// Use Fluorescence
  bool m_ActivateFluorescence;

 
};

#endif


/*
 * MCPhysicsListEMLivermore.hh: the end...
 ******************************************************************************/







