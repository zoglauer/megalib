/*
 * MCDriftChamberSD.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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





