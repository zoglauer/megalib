/*
 * MCScintillatorSD.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Class representing the sensitive detector of a scintillator
 *
 */

#ifndef ___MCScintillatorSD___
#define ___MCScintillatorSD___

// Standard lib:
#include <map>
using namespace std;

// Geant4:
#include "G4String.hh"
#include "G4VSensitiveDetector.hh"
#include "globals.hh"

// Cosima:
#include "MCScintillatorHit.hh"
#include "MCSD.hh"

// Forward declarations:
class G4HCofThisEvent;
class G4Step;


/******************************************************************************/


class MCScintillatorSD : public MCSD
{
  // public interface:
public:
  ///  Set name, type and collection 
  MCScintillatorSD(G4String Name);
  /// Default constructor
  ~MCScintillatorSD();
  
  /// Create hit collection if not yet done
  void Initialize(G4HCofThisEvent*);
  /// Analyze and store the hits
  G4bool ProcessHits(G4Step* astep,G4TouchableHistory* ROHist);
  /// Generate and store the hits 
  G4bool PostProcessHits(const G4Step* Step);
  /// Trensfer hits to geant
  void EndOfEvent(G4HCofThisEvent*);
  /// For hit centering: add the unique position in this detector
  void SetUniquePosition(string Name, G4ThreeVector Position);
  /// For hit centering: add the unique position in this detector
  void SetCommonVolumeName(string Name) { m_CommonVolumeName = G4String(Name) + "Log"; }

  // protected methods:
protected:
  

  // protected members:
protected:


  // private members:
private:
  /// Container for the hits of one event
  MCScintillatorHitsCollection* m_HitCollection;      

  /// For hit centering: the translated position
  map<string, G4ThreeVector> m_UniquePositions;

  /// The name of the common volume:
  G4String m_CommonVolumeName;
};

#endif


/*
 * MCScintillatorSD.hh: the end...
 ******************************************************************************/





