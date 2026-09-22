/*
 * MCCalorBarSD.hh
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
 * Class representing the sensitive detector of a calorimeter bar
 *
 */

#ifndef ___MCCalorBarSD___
#define ___MCCalorBarSD___

// Geant4:
#include "G4VSensitiveDetector.hh"
#include "globals.hh"

// Cosima:
#include "MCCalorBarHit.hh"
#include "MCSD.hh"

// Forward declarations:
class G4HCofThisEvent;
class G4Step;


/******************************************************************************/


class MCCalorBarSD : public MCSD
{
  // public interface:
public:
  ///  Set name, type and collection 
  MCCalorBarSD(G4String Name);
  /// Default constructor
  ~MCCalorBarSD();
  
  /// Set whether or not this is a 3D calorimeter bar (z-resolution or not)
  void SetIs3D(bool Is3D = true) { m_Is3D = Is3D; }
  
  /// Create hit collection if not yet done
  void Initialize(G4HCofThisEvent* HCof);
  /// Analyze and store the hits
  G4bool ProcessHits(G4Step* astep, G4TouchableHistory* ROHist);
  /// Generate and store the hits 
  G4bool PostProcessHits(const G4Step* Step);
  /// Transfer hits to geant
  void EndOfEvent(G4HCofThisEvent* HCof);


  // protected methods:
protected:
  

  // protected members:
protected:


  // private members:
private:
  /// Container for the hits of one event
  MCCalorBarHitsCollection* m_HitCollection;      

  /// True if this calorimeter has z-resolution:
  bool m_Is3D;
};

#endif


/*
 * MCCalorBarSD.hh: the end...
 ******************************************************************************/





