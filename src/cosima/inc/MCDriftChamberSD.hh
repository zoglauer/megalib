/*
 * MCDriftChamberSD.hh
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
 * Class representing the sensitive detector of a strip detector
 *
 */

#ifndef ___MCDriftChamberSD___
#define ___MCDriftChamberSD___

// Geant4:
#include "G4VSensitiveDetector.hh"
#include "globals.hh"

// Cosima:
#include "MCDriftChamberHit.hh"
#include "MC2DStripSD.hh"

// Forward declarations:
class G4HCofThisEvent;
class G4Step;

/******************************************************************************/


class MCDriftChamberSD : public MC2DStripSD
{
  // public interface:
public:
  /// Default constructor: Sets the name, type and default values for geometry
  MCDriftChamberSD(G4String Name);
  /// Default destructor: empty
  ~MCDriftChamberSD();
  
  /// Initialize the hit collection
  void Initialize(G4HCofThisEvent*);
  /// Analyze and store the hits
  G4bool ProcessHits(G4Step* astep, G4TouchableHistory* ROHist);
  /// Generate and store the hits 
  G4bool PostProcessHits(const G4Step* Step);
  /// Housekeeping
  void EndOfEvent(G4HCofThisEvent*);

  /// Set whether or not this is a 3D strip (z-resolution or not)
  void SetIs3D(bool Is3D = true);

  /// Set the light speed in the detector material
  void SetLightSpeed(const double LightSpeed) { m_LightSpeed = LightSpeed; }
  /// Set the position of the light sensitive detector
  void SetLightDetectorPosition(const int Pos) { m_LightDetectorPosition = Pos; }
  /// Set the drift constant for a * sqrt(L)
  void SetDriftConstant(const double DriftConstant) { m_DriftConstant = DriftConstant; }
  /// Set the energy per drifting electron
  void SetEnergyPerElectron(const double EnergyPerElectron) { m_EnergyPerElectron = EnergyPerElectron; }

  // protected methods:
protected:
  

  // protected members:
protected:


  // private members:
private:
  /// Container for the hits of one event
  MCDriftChamberHitsCollection* m_HitCollection;      

  /// Light speed in the detector material
  double m_LightSpeed;
  /// Position of the light sensitive detector
  int m_LightDetectorPosition;
  /// The drift constant for a * sqrt(L)
  double m_DriftConstant;
  /// Energy per drifting electron
  double m_EnergyPerElectron;
};

#endif


/*
 * MCDriftChamberSD.hh: the end...
 ******************************************************************************/





