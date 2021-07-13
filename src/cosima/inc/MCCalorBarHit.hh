/*
 * MCCalorBarHit.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Class representing a hit in a calorimeter bar
 * The represented data is the position in bar numbers and absolute position
 * as well as the energy in ADCounts and keV
 *
 */

#ifndef ___MCCalorBarHit___
#define ___MCCalorBarHit___

// Geant4:
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

// Cosima:
#include "MCVHit.hh"


/******************************************************************************/


class MCCalorBarHit : public MCVHit
{
public:
  /// Default constructor
  MCCalorBarHit();
  /// Default destructor
  ~MCCalorBarHit();
  /// Default copy constructor
  MCCalorBarHit(const MCCalorBarHit&);
  /// Default assignment constructor
  const MCCalorBarHit& operator=(const MCCalorBarHit&);

  /// Is-Equal operator
  int operator==(MCCalorBarHit&);
  /// Addition operator
  const MCCalorBarHit& operator+=(const MCCalorBarHit&);
  
  /// Special Geant4 memory allocation
  inline void* operator new(size_t);
  /// Special Geant4 memory deallocation
  inline void  operator delete(void*);

  /// Set the number of ADCcounts evoked by this hit
  inline void SetADCCounts(G4double ADCCountsTop, G4double ADCCountsBottom = -1) 
  { m_ADCCountsTop = ADCCountsTop; m_ADCCountsBottom = ADCCountsBottom; }
  /// Set the bar number in x-direction
  inline void SetXBar(G4int XBar) { m_XBar = XBar; } 
  /// Set the bar number in y-direction
  inline void SetYBar(G4int YBar) { m_YBar = YBar; } 
  
  /// Return the number of ADCCounts
  inline G4double GetADCCounts()     { return m_ADCCountsTop; };
  /// Return the number of ADCCounts
  inline G4double GetADCCountsTop()     { return m_ADCCountsTop; };
  /// Return the number of ADCCounts
  inline G4double GetADCCountsBottom()     { return m_ADCCountsBottom; };

  /// Dump the hit
  void Print();

private:
  /// x-bar of the module where the hit has happend
  G4int m_XBar;
  /// y-bar of the module where the hit has happend
  G4int m_YBar;

  /// Number of ADC counts registered in the bar (top diode) 
  G4double m_ADCCountsTop;
  /// Number of ADC counts registered in the bar (bottom diode)
  G4double m_ADCCountsBottom;
};


/******************************************************************************/


typedef G4THitsCollection<MCCalorBarHit> MCCalorBarHitsCollection;

extern G4Allocator<MCCalorBarHit> MCCalorBarHitAllocator;

inline void* MCCalorBarHit::operator new(size_t)
{
  void* NewHit;
  NewHit = (void*) MCCalorBarHitAllocator.MallocSingle();
  return NewHit;
}

inline void MCCalorBarHit::operator delete(void* Hit)
{
  MCCalorBarHitAllocator.FreeSingle((MCCalorBarHit*) Hit);
}

#endif


/*
 * MCCalorBarHit.hh: the end...
 ******************************************************************************/








