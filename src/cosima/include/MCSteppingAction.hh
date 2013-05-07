/*
 * MCSteppingAction.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Implementation of G4UserSteppingAction:
 * Cares about the real interactions of the primary track and sends the info
 * to the event action class
 *
 */

#ifndef ___MCSteppingAction___
#define ___MCSteppingAction___

// Geant4:
#include "globals.hh"
#include "G4UserSteppingAction.hh"
#include "G4ThreeVector.hh"
#include "G4StepPoint.hh"

// Cosima:
#include "MCParameterFile.hh"

// Standard lib:
#include <vector>
using namespace std;

/******************************************************************************/

class MCSteppingAction : public G4UserSteppingAction
{
  // public interface:
public:
  /// Standard constructor
  MCSteppingAction(MCParameterFile& RunParameters);
  /// Default destructor
  virtual ~MCSteppingAction();

  /// Actions at each step of the event tracking
  virtual void UserSteppingAction(const G4Step* Step);

  /// Set the number of initial particles
  void SetInitialParticles(vector<int> InitialParticles) { m_InitialParticles = InitialParticles; m_InteractionId = m_InitialParticles.size(); }

  /// Set if the origin of the particles is a build-up source
  void SetParticleOriginIsBuildUpSource(bool IsBuildUpSource) { m_ParticleOriginIsBuildUpSource = IsBuildUpSource; }

  /// Reset the number of generated particles
  void SetDecayMode(int DecayMode) { m_DecayMode = DecayMode; }

  /// Returns the MEGAlib internal particle Id (partly identical with G3)
  /// This is definitely not the right place for this information
  static int GetParticleId(G4String Name);

  /// Returns the MEGAlib internal particle Id (partly identical with G3)
  /// This is definitely not the right place for this information
  static int GetParticleId(G4ParticleDefinition* Definition);

  // protected methods:
protected:
  /// Return the Id of the detector, we are currently in
  int GetDetectorId(const G4StepPoint* StepPoint);

  // protected members:
protected:


  // private members:
private:
  /// True, if the simulation info should be stored or not 
  bool m_StoreSimulationInfo;
  /// True, if the ionization info (IA IONI) should be stored or not 
  bool m_StoreIonization;

  /// List of watched volumes
  vector<MString> m_WatchedVolumes;

  /// List of black absorbers
  vector<MString> m_BlackAbsorbers;

  /// ID of the current interaction (IA == new particle is generated)
  G4int m_InteractionId;

  /// TrackID of last UserSteppingAction cycle
  int m_TrackId;
  /// Total number of secondaries of that track during last cycle
  int m_NSecondaries;
  
  /// Decay mode (normal, ignore, build-up, decay)
  int m_DecayMode;

  /// The original particles:
  vector<int> m_InitialParticles;
  /// True if the initial particles originate from a build-up event list:
  bool m_ParticleOriginIsBuildUpSource;

  /// The activation time constant: The time during which decays are consindered coincident
  double m_DetectorTimeConstant;
};

#endif


/*
 * MCSteppingAction.hh: the end...
 ******************************************************************************/
