/*
 * MCScintillatorHit.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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
  
  /// Return the data as calibrated hit (as position and energy)
  MSimHT* GetCalibrated();

  /// Set the number of ADCcounts evoked by this hit
  inline void SetADCCounts(G4double ADCCounts) { m_ADCCounts = ADCCounts; };
  /// Return the number of ADCCounts
  inline G4double GetADCCounts() { return m_ADCCounts; };

  /// Set the location of the hit in the world coordinate system
  inline void SetPosition(G4ThreeVector Position) { m_Position = Position; }
  /// Return the position in the world coordinate system
  inline G4ThreeVector GetPosition() { return m_Position; }

  /// Dump the hit
  void Print();

private:
  /// Absolute Position of the hit in the world reference frame, 
  /// if multiple hits have been added to this one, 
  /// then this is the position of the lasted added hit!!! 
  G4ThreeVector m_Position;

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








