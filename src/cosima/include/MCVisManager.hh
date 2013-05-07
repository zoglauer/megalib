/*
 * MCVisManager.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Class representing a visualization manager
 *
 */


#ifndef ___MCVisManager___
#define ___MCVisManager___

#ifdef G4VIS_USE

// Geant4:
#include "G4VisManager.hh"


/******************************************************************************/


class MCVisManager: public G4VisManager 
{
  // public interface:
public:
  /// Default constructor
  MCVisManager();

  // protected methods:
protected:
  /// Register all graphics systems...
  void RegisterGraphicsSystems();
  

  // protected members:
protected:


  // private members:
private:


};

#endif

#endif
