/*
 * MCDriftChamberHit.hh
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
 * Class representing a hit in a 2D strip detector.
 * The represented data is the position in strip numbers and absolute position
 * as well as the energy in ADCounts and keV
 *
 */

#ifndef ___MCDriftChamberHit___
#define ___MCDriftChamberHit___

// Geant4:
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

// Cosima:
#include "MC2DStripHit.hh"


/******************************************************************************/


class MCDriftChamberHit : public MC2DStripHit
{
  // Public interface:
public:
  /// Default constructor
  explicit MCDriftChamberHit();
  /// Default destructor
  ~MCDriftChamberHit();
  /// Default copy constructor
  MCDriftChamberHit(const MCDriftChamberHit&);
  /// Default assignment constructor
  const MCDriftChamberHit& operator=(const MCDriftChamberHit&);

  /// Is-Equal operator
  int operator==(MCDriftChamberHit&);
  /// Addition operator
  const MCDriftChamberHit& operator+=(const MCDriftChamberHit&);
  
  /// Special Geant4 memory allocation
  inline void* operator new(size_t);
  /// Special Geant4 memory deallocation
  inline void  operator delete(void*);
  
  /// Return the data as calibrated hit (as position and energy)
  MSimHT* GetCalibrated();

  /// Set the number of ADCcounts evoked by this hit
  inline void SetADCCounts(G4double ADCCounts) { m_ADCCounts = ADCCounts; }
  /// Set the strip number in x-direction
  inline void SetXStrip(G4int XStrip) { m_XStrip = XStrip; } 
  /// Set the strip number in x-direction
  inline G4int GetXStrip() { return m_XStrip; }
  /// Set the strip number in y-direction
  inline void SetYStrip(G4int YStrip) { m_YStrip = YStrip; } 
   /// Set the strip number in y-direction
  inline G4int GetYStrip() { return m_YStrip; }
 
  /// Return the number of ADCCounts
  inline G4double GetADCCounts() { return m_ADCCounts; }

  /// Dump the hit
  void Print();

private:
  /// x-Strip of the LAYER (not wafer) where the hit has happend
  G4int m_XStrip;
  /// y-Strip of the LAYER (not wafer) where the hit has happend
  G4int m_YStrip;

  /// Number of ADC counts registered in the strips
  G4double m_ADCCounts;
};


/******************************************************************************/


typedef G4THitsCollection<MCDriftChamberHit> MCDriftChamberHitsCollection;

extern G4Allocator<MCDriftChamberHit> MCDriftChamberHitAllocator;

inline void* MCDriftChamberHit::operator new(size_t)
{
  void* NewHit;
  NewHit = (void*) MCDriftChamberHitAllocator.MallocSingle();
  return NewHit;
}

inline void MCDriftChamberHit::operator delete(void* Hit)
{
  MCDriftChamberHitAllocator.FreeSingle((MCDriftChamberHit*) Hit);
}

#endif


/*
 * MCDriftChamberHit.hh: the end...
 ******************************************************************************/








