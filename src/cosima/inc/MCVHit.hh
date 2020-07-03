/*
 * MCVHit.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Virtual base class for hits in Cosima.
 * In contrast to its base class it contains additional information like
 * the energy, the detector name and its type, which are common to all hits
 * in Cosima.
 *
 */

#ifndef ___MCVHit___
#define ___MCVHit___


// Standard lib:
#include <vector>
#include <set>
#include <string>
using namespace std;

// Geant4:
#include "globals.hh"
#include "G4VHit.hh"
#include "G4ThreeVector.hh"

// MEGAlib:
#include "MSimHT.h"


/******************************************************************************/

class MCVHit : public G4VHit
{
  // public interface:
public:
  /// Default constructor
  MCVHit();
  /// Default destructor
  virtual ~MCVHit();

  /// Return the energy deposited in this hit
  G4double GetEnergy() { return m_Energy; };
  /// Return the position in the world coordinate system
  inline G4ThreeVector GetPosition() { return m_Position; }
  /// Return the time of this hit since event start
  G4double GetTime() { return m_Time; };
  /// Return the name of the detector, in which the hit took place
  G4String GetDetectorName() { return m_Name; };
  /// Return the type of the detector, in which the hit has happend
  G4int GetDetectorType() { return m_DetectorType; };

  /// Add an origin to the list
  void AddOrigin(const int TrackId);
  /// Add several origins to the list
  void AddOrigins(const set<int>& Origins);

  /// Add an entry to the volume history:
  void AddVolumeHistory(string Name);

  /// Set the energy deposited during this hit
  inline void SetEnergy(G4double Energy) { m_Energy = Energy; };
  /// Set the location of the hit in the world coordinate system
  inline void SetPosition(G4ThreeVector Position) { m_Position = Position; }
  /// Set the name of the detector module in which this hit has happend
  inline void SetDetectorName(G4String Name) { m_Name = Name; };
  // Why is SetDetectorType missing???
  /// Set the time of this hit since event start
  inline void SetTime(G4double Time) { m_Time = Time; };
  
  /// Return the data as calibrated hits (as position and energy)
  virtual MSimHT* GetCalibrated();

  // protected methods:
protected:
  

  // protected members:
protected:

  /// Deposited energy
  G4double m_Energy;
  /// Absolute Position of the hit in the world reference frame
  G4ThreeVector m_Position;
  /// Shortest time since event start
  G4double m_Time;
  /// Name of the detector module
  G4String m_Name;
  /// Type of the detector module as defined in Geomega (D1, D2, D3, D4)
  G4int m_DetectorType;

  /// Array of Id of all tracks which have contributed
  set<int> m_Origins;
  ///
  int m_LastAddedOrigin;
  
  

  /// Array of the names in which this hit is inside:
  vector<string> m_VolumeHistory;

  // private members:
private:


};

#endif


/*
 * MCVHit.hh: the end...
 ******************************************************************************/
