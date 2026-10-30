/*
 * MCRunManager.hh
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
 * Own version of the Geant4 run manager.
 * Since G4RunManager is a singleton, also this is a singleton!
 * Provides addition information like access to the runs etc.
 *
 */

#ifndef ___MCRunManager___
#define ___MCRunManager___

// Geant4:
#include "G4RunManager.hh"

// Cosima:
#include "MCParameterFile.hh"

// MEGAlib:
#include "MTimer.h"

// Forward declarations:
class MCPrimaryGeneratorAction;
class MCSteppingAction;
class MCTrackingAction;
class MCEventAction;
class MCPhysicsList;
class MCDetectorConstruction;

/******************************************************************************/

class MCRunManager : public G4RunManager
{
  // public interface:
public:
  /// Default constructor
  MCRunManager(MCParameterFile& RunParameters);
  /// Default destructor
  virtual ~MCRunManager();

  /// Return the singletion class
  static MCRunManager* GetMCRunManager();
  /// Return the event generator
  MCPrimaryGeneratorAction* GetPrimaryGeneratorAction();
  /// Return the stepping action
  MCSteppingAction* GetSteppingAction();
  /// Return the stepping action
  MCTrackingAction* GetTrackingAction();
  /// Return the event action
  MCEventAction* GetEventAction();
  /// Return the physics list
  MCPhysicsList* GetPhysicsList();
  /// Return the detector constructor
  MCDetectorConstruction* GetDetectorConstruction();

  /// Return a list of the runs
  vector<MCRun>& GetRuns();

  /// Return the current run
  MCRun& GetCurrentRun() { return m_RunParameters.GetCurrentRun(); };
  
  /// Start all the action
  virtual void StartBeam();
  /// Abort all the action
  virtual void AbortRun(G4bool softAbort = false);

  /// Do some initializations for the next run which are common to macro and normal operation mode
  bool PrepareNextRun();

  // protected methods:
protected:
  

  // protected members:
protected:


  // private members:
private:
  /// Singleton representation
  static MCRunManager* s_RunManager;
  /// The parameter file
  MCParameterFile& m_RunParameters;
};

#endif


/*
 * MCRunManager.hh: the end...
 ******************************************************************************/
