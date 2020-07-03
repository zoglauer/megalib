/*
 * MCVoxel3DHit.hh
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

#ifndef ___MCVoxel3DHit___
#define ___MCVoxel3DHit___

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


class MCVoxel3DHit : public MCVHit
{
  // Public interface:
public:
  /// Default constructor
  explicit MCVoxel3DHit();
  /// Default destructor
  ~MCVoxel3DHit();
  /// Default copy constructor
  MCVoxel3DHit(const MCVoxel3DHit&);
  /// Default assignment constructor
  const MCVoxel3DHit& operator=(const MCVoxel3DHit&);

  /// Is-Equal operator
  int operator==(MCVoxel3DHit&);
  /// Addition operator
  const MCVoxel3DHit& operator+=(const MCVoxel3DHit&);
  
  /// Special Geant4 memory allocation
  inline void* operator new(size_t);
  /// Special Geant4 memory deallocation
  inline void  operator delete(void*);
  
  /// Return the data as calibrated hit (as position and energy) if its no guardring hit
  MSimGR* GetGuardringCalibrated();
  /// Return the data as calibrated hit (as position and energy)
  MSimHT* GetCalibrated();

  /// Set if the hit happened in guardring 
  inline void SetIsGuardringHit(G4bool IsGuardringHit) { m_IsGuardringHit = IsGuardringHit; }
  /// Return if the hit happened in the guardring 
  inline bool GetIsGuardringHit() { return m_IsGuardringHit; }

  /// Set the number of ADCcounts evoked by this hit
  inline void SetADCCounts(G4double ADCCounts) { m_ADCCounts = ADCCounts; }
  /// Set the voxel number in x-direction
  inline void SetXVoxel(G4int XVoxel) { m_XVoxel = XVoxel; } 
  /// Set the voxel number in x-direction
  inline G4int GetXVoxel() { return m_XVoxel; }
  /// Set the voxel number in y-direction
  inline void SetYVoxel(G4int YVoxel) { m_YVoxel = YVoxel; } 
   /// Set the voxel number in y-direction
  inline G4int GetYVoxel() { return m_YVoxel; }
  /// Set the voxel number in z-direction
  inline void SetZVoxel(G4int ZVoxel) { m_ZVoxel = ZVoxel; } 
   /// Set the voxel number in z-direction
  inline G4int GetZVoxel() { return m_ZVoxel; }
 
  /// Return the number of ADCCounts
  inline G4double GetADCCounts() { return m_ADCCounts; }

  /// Dump the hit
  void Print();

protected:
  /// True if the interaction happened in the guardring
  G4bool m_IsGuardringHit;

  /// x-Voxel of the detector where the hit has happend
  G4int m_XVoxel;
  /// y-Voxel of the detector where the hit has happend
  G4int m_YVoxel;
  /// z-Voxel of the detector where the hit has happend
  G4int m_ZVoxel;

  /// Number of ADC counts registered in the voxels
  G4double m_ADCCounts;
};


/******************************************************************************/


typedef G4THitsCollection<MCVoxel3DHit> MCVoxel3DHitsCollection;

extern G4Allocator<MCVoxel3DHit> MCVoxel3DHitAllocator;

inline void* MCVoxel3DHit::operator new(size_t)
{
  void* NewHit;
  NewHit = (void*) MCVoxel3DHitAllocator.MallocSingle();
  return NewHit;
}

inline void MCVoxel3DHit::operator delete(void* Hit)
{
  MCVoxel3DHitAllocator.FreeSingle((MCVoxel3DHit*) Hit);
}

#endif


/*
 * MCVoxel3DHit.hh: the end...
 ******************************************************************************/








