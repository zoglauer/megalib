/*
 * MC2DStripHit.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Class representing a hit in a 2D strip detector.
 * The represented data is the position in strip numbers and absolute position
 * as well as the energy in ADCounts and keV
 *
 */

#ifndef ___MC2DStripHit___
#define ___MC2DStripHit___

// Geant4:
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

// MEGAlib:
#include "MSimGR.h"

// Cosima:
#include "MCVHit.hh"


/******************************************************************************/


class MC2DStripHit : public MCVHit
{
  // Public interface:
public:
  /// Default constructor
  explicit MC2DStripHit(bool Is3D = false);
  /// Default destructor
  ~MC2DStripHit();
  /// Default copy constructor
  MC2DStripHit(const MC2DStripHit&);
  /// Default assignment constructor
  const MC2DStripHit& operator=(const MC2DStripHit&);

  /// Is-Equal operator
  int operator==(MC2DStripHit&);
  /// Addition operator
  const MC2DStripHit& operator+=(const MC2DStripHit&);
  
  /// Special Geant4 memory allocation
  inline void* operator new(size_t);
  /// Special Geant4 memory deallocation
  inline void  operator delete(void*);
  
  /// Return the data as calibrated hit (as position and energy) if its no guardring hit
  MSimGR* GetGuardringCalibrated();
  /// Return the data as calibrated hit (as position and energy) if its no guardring hit
  MSimHT* GetCalibrated();

  /// Set if the hit happened in guardring 
  inline void SetIsGuardringHit(G4bool IsGuardringHit) { m_IsGuardringHit = IsGuardringHit; }
  /// Return if the hit happened in the guardring 
  inline bool GetIsGuardringHit() { return m_IsGuardringHit; }

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

protected:
  /// True if the interaction happened in the guardring
  G4bool m_IsGuardringHit;

  /// x-Strip of the LAYER (not wafer) where the hit has happend
  G4int m_XStrip;
  /// y-Strip of the LAYER (not wafer) where the hit has happend
  G4int m_YStrip;

  /// Number of ADC counts registered in the strips
  G4double m_ADCCounts;

  /// True, if the strip has depth resolution
  G4bool m_Is3D;
};


/******************************************************************************/


typedef G4THitsCollection<MC2DStripHit> MC2DStripHitsCollection;

extern G4Allocator<MC2DStripHit> MC2DStripHitAllocator;

inline void* MC2DStripHit::operator new(size_t)
{
  void* NewHit;
  NewHit = (void*) MC2DStripHitAllocator.MallocSingle();
  return NewHit;
}

inline void MC2DStripHit::operator delete(void* Hit)
{
  MC2DStripHitAllocator.FreeSingle((MC2DStripHit*) Hit);
}

#endif


/*
 * MC2DStripHit.hh: the end...
 ******************************************************************************/








