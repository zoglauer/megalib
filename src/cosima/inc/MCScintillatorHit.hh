/*
 * MCScintillatorHit.hh
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
 * Class representing a hit in a scintillation detector (e.g. ACS).
 * The represented data is the detector name
 * as well as the energy in ADCounts and keV
 *
 */

#ifndef ___MCScintillatorHit___
#define ___MCScintillatorHit___

// Geant4:
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

// Cosima:
#include "MCVHit.hh"

// Forward declarations:


/******************************************************************************/


class MCScintillatorHit : public MCVHit
{
public:  
  /// Default constructor
  MCScintillatorHit();
  /// Default destructor
  ~MCScintillatorHit();
  /// Default copy constructor
  MCScintillatorHit(const MCScintillatorHit&);
  /// Default assignment constructor
  const MCScintillatorHit& operator=(const MCScintillatorHit&);

  /// Is-Equal operator
  int operator==(MCScintillatorHit&);
  /// Addition operator
  const MCScintillatorHit& operator+=(const MCScintillatorHit&);
  
  /// Special Geant4 memory allocation
  inline void* operator new(size_t);
  /// Special Geant4 memory deallocation
  inline void  operator delete(void*);
  
  /// Set the number of ADCcounts evoked by this hit
  inline void SetADCCounts(G4double ADCCounts) { m_ADCCounts = ADCCounts; };
  /// Return the number of ADCCounts
  inline G4double GetADCCounts() { return m_ADCCounts; };

  /// Dump the hit
  void Print();

private:
  /// Absolute Position of the hit in the world reference frame, 
  /// if multiple hits have been added to this one, 
  /// then this is the position of the lasted added hit!!! 
  // G4ThreeVector m_Position;

  /// Number of ADC counts registered in the scintillator
  G4double m_ADCCounts;
};


/******************************************************************************/


typedef G4THitsCollection<MCScintillatorHit> MCScintillatorHitsCollection;

extern G4Allocator<MCScintillatorHit> MCScintillatorHitAllocator;

inline void* MCScintillatorHit::operator new(size_t)
{
  void* NewHit;
  NewHit = (void*) MCScintillatorHitAllocator.MallocSingle();
  return NewHit;
}

inline void MCScintillatorHit::operator delete(void* Hit)
{
  MCScintillatorHitAllocator.FreeSingle((MCScintillatorHit*) Hit);
}

#endif


/*
 * MCScintillatorHit.hh: the end...
 ******************************************************************************/








