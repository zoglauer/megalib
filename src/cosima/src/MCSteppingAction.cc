/*
 * MCSteppingAction.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Cosima:
#include "MCSteppingAction.hh"
#include "MCEventAction.hh"
#include "MCTrackInformation.hh"
#include "MCRunManager.hh"
#include "MCPrimaryGeneratorAction.hh"
#include "MCSD.hh"
#include "MCSource.hh"

// Geant4:
#include "G4SystemOfUnits.hh"
#include "G4SteppingManager.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4TrackStatus.hh"
#include "G4VPhysicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4EventManager.hh"
//#include "G4NuclearLevel.hh" //no longer exist in g4 v11
//#include "G4NuclearLevelManager.hh" // same
//#include "G4NuclearLevelStore.hh" // same
#include "G4NuclearLevelData.hh" //replace libs upper ? see release note 10.2
#include "G4LevelManager.hh"
#include "G4NucLevel.hh"

#include "G4Ions.hh"
#include "G4IonTable.hh"

// MEGAlib
#include "MStreams.h"
#include "MAssert.h"
#include "MDMaterial.h"
#include "MSimEvent.h"

// Standard lib:
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <functional>
using namespace std;


/******************************************************************************/

const int MCSteppingAction::c_ProcessIDUncovered              = 0;
const int MCSteppingAction::c_ProcessIDCompton                = 1;
const int MCSteppingAction::c_ProcessIDPhoto                  = 2;
const int MCSteppingAction::c_ProcessIDPair                   = 3;
const int MCSteppingAction::c_ProcessIDRayleigh               = 4;
const int MCSteppingAction::c_ProcessIDAnnihilation           = 5;
const int MCSteppingAction::c_ProcessIDBremsstrahlung         = 6;
const int MCSteppingAction::c_ProcessIDElasticScattering      = 7;
const int MCSteppingAction::c_ProcessIDInelasticScattering    = 8;
const int MCSteppingAction::c_ProcessIDFission                = 9;
const int MCSteppingAction::c_ProcessIDCapture                = 10;
const int MCSteppingAction::c_ProcessIDDecay                  = 11;
const int MCSteppingAction::c_ProcessIDRadioactiveDecay       = 12;
const int MCSteppingAction::c_ProcessIDIonization             = 13;
const int MCSteppingAction::c_ProcessIDTransportation         = 14;
const int MCSteppingAction::c_ProcessIDNoProcess              = 15;


/******************************************************************************
 * Only calls the base class constructor
 */
MCSteppingAction::MCSteppingAction(MCParameterFile& RunParameters) : 
  G4UserSteppingAction(), m_InteractionId(0), m_TrackId(0), 
  m_NSecondaries(0), m_DecayMode(MCParameterFile::c_DecayModeIgnore), 
  m_ParticleOriginIsBuildUpSource(false)
{
  // Retrieve some additonal data from the parameter file

  if (RunParameters.StoreSimulationInfo() == MSimEvent::c_StoreSimulationInfoAll ||
      RunParameters.StoreSimulationInfo() == MSimEvent::c_StoreSimulationInfoDepositsOnly) {
    m_StoreSimulationInfo = true;
  } else {
    m_StoreSimulationInfo = false;
  }
  m_StoreIonization = RunParameters.StoreSimulationInfoIonization();

  m_WatchedVolumes = RunParameters.GetStoreSimulationInfoWatchedVolumes();
  m_WatchedVolumesLog.clear();
  for (auto S: m_WatchedVolumes) {
    m_WatchedVolumesLog.push_back(S + "Log");
  }

  m_BlackAbsorbers = RunParameters.GetBlackAbsorbers();

  m_DecayMode = RunParameters.GetDecayMode();

  m_DetectorTimeConstant = RunParameters.GetDetectorTimeConstant();
  
  m_Regions = RunParameters.GetRegionList();
  
  m_KnownProcess.push_back("polarLowEnCompt"); m_KnownProcessID.push_back(c_ProcessIDCompton);
  m_KnownProcess.push_back("PenCompton"); m_KnownProcessID.push_back(c_ProcessIDCompton);
  m_KnownProcess.push_back("LowEnCompton"); m_KnownProcessID.push_back(c_ProcessIDCompton);
  m_KnownProcess.push_back("compt"); m_KnownProcessID.push_back(c_ProcessIDCompton);

  m_KnownProcess.push_back("conv"); m_KnownProcessID.push_back(c_ProcessIDPair);
  m_KnownProcess.push_back("hPairProd"); m_KnownProcessID.push_back(c_ProcessIDPair);
  m_KnownProcess.push_back("PenConversion"); m_KnownProcessID.push_back(c_ProcessIDPair);
  m_KnownProcess.push_back("LowEnConversion"); m_KnownProcessID.push_back(c_ProcessIDPair);
  m_KnownProcess.push_back("LowEnPolarizConversion"); m_KnownProcessID.push_back(c_ProcessIDPair);
  m_KnownProcess.push_back("muPairProd"); m_KnownProcessID.push_back(c_ProcessIDPair);
  m_KnownProcess.push_back("ePairProd"); m_KnownProcessID.push_back(c_ProcessIDPair);

  
  m_KnownProcess.push_back("annihil"); m_KnownProcessID.push_back(c_ProcessIDAnnihilation);
  m_KnownProcess.push_back("PenAnnih"); m_KnownProcessID.push_back(c_ProcessIDAnnihilation);
  
  m_KnownProcess.push_back("eBrem"); m_KnownProcessID.push_back(c_ProcessIDBremsstrahlung);
  m_KnownProcess.push_back("hBrems"); m_KnownProcessID.push_back(c_ProcessIDBremsstrahlung);
  m_KnownProcess.push_back("PenelopeBrem"); m_KnownProcessID.push_back(c_ProcessIDBremsstrahlung);
  m_KnownProcess.push_back("LowEnBrem"); m_KnownProcessID.push_back(c_ProcessIDBremsstrahlung);
  m_KnownProcess.push_back("muBrems"); m_KnownProcessID.push_back(c_ProcessIDBremsstrahlung);
  
  m_KnownProcess.push_back("Rayl"); m_KnownProcessID.push_back(c_ProcessIDRayleigh);
  m_KnownProcess.push_back("PenRayleigh"); m_KnownProcessID.push_back(c_ProcessIDRayleigh);
  m_KnownProcess.push_back("LowEnRayleigh"); m_KnownProcessID.push_back(c_ProcessIDRayleigh);
  
  m_KnownProcess.push_back("LowEnPhotoElec"); m_KnownProcessID.push_back(c_ProcessIDPhoto);
  m_KnownProcess.push_back("PenPhotoElec"); m_KnownProcessID.push_back(c_ProcessIDPhoto);
  m_KnownProcess.push_back("phot"); m_KnownProcessID.push_back(c_ProcessIDPhoto);
  
  m_KnownProcess.push_back("hadElastic"); m_KnownProcessID.push_back(c_ProcessIDElasticScattering);
  m_KnownProcess.push_back("ionElastic"); m_KnownProcessID.push_back(c_ProcessIDElasticScattering);

  m_KnownProcess.push_back("nFission"); m_KnownProcessID.push_back(c_ProcessIDFission);
  
  m_KnownProcess.push_back("PhotonInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("NeutronInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("AntiNeutronInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("ProtonInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("AntiProtonInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("PionPlusInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("PionMinusInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("dInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("tInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("KaonZeroLInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("KaonZeroSInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("KaonPlusInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("KaonMinusInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("LambdaInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("anti_lambdaInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("AntiLambdaInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("SigmaMinusInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("SigmaPlusInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("He3Inelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("alphaInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("PositronNuclear"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("ElectroNuclear"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("ionInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("protonInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("neutronInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("pi+Inelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("pi-Inelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("kaon-Inelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("kaon+Inelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("kaon0LInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("kaon0SInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("anti_neutronInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("anti_protonInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("sigma+Inelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("sigma-Inelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("anti_sigma+Inelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("anti_sigma-Inelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("lambdaInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("anti-lambdaInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("photonNuclear"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("electronNuclear"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("positronNuclear"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("xi-Inelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("xi+Inelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("xi0Inelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("anti_xi-Inelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);

  
  m_KnownProcess.push_back("HadronCapture"); m_KnownProcessID.push_back(c_ProcessIDCapture);
  m_KnownProcess.push_back("nCapture"); m_KnownProcessID.push_back(c_ProcessIDCapture);
  m_KnownProcess.push_back("CHIPSNuclearCaptureAtRest"); m_KnownProcessID.push_back(c_ProcessIDCapture);
  m_KnownProcess.push_back("muMinusCaptureAtRest"); m_KnownProcessID.push_back(c_ProcessIDCapture);
  m_KnownProcess.push_back("hBertiniCaptureAtRest"); m_KnownProcessID.push_back(c_ProcessIDCapture);
  
  m_KnownProcess.push_back("Decay"); m_KnownProcessID.push_back(c_ProcessIDDecay);
  
  m_KnownProcess.push_back("RadioactiveDecay"); m_KnownProcessID.push_back(c_ProcessIDRadioactiveDecay);
  m_KnownProcess.push_back("Radioactivation"); m_KnownProcessID.push_back(c_ProcessIDRadioactiveDecay);

  m_KnownProcess.push_back("LowEnergyIoni"); m_KnownProcessID.push_back(c_ProcessIDIonization);
  m_KnownProcess.push_back("hLowEIoni"); m_KnownProcessID.push_back(c_ProcessIDIonization);
  m_KnownProcess.push_back("ionIoni"); m_KnownProcessID.push_back(c_ProcessIDIonization);
  m_KnownProcess.push_back("eIoni"); m_KnownProcessID.push_back(c_ProcessIDIonization);
  m_KnownProcess.push_back("hIoni"); m_KnownProcessID.push_back(c_ProcessIDIonization);
  m_KnownProcess.push_back("muIoni"); m_KnownProcessID.push_back(c_ProcessIDIonization);
  m_KnownProcess.push_back("muMsc"); m_KnownProcessID.push_back(c_ProcessIDIonization);
  m_KnownProcess.push_back("msc"); m_KnownProcessID.push_back(c_ProcessIDIonization);
  m_KnownProcess.push_back("PenelopeIoni"); m_KnownProcessID.push_back(c_ProcessIDIonization);
  m_KnownProcess.push_back("CoulombScat"); m_KnownProcessID.push_back(c_ProcessIDIonization);

  m_KnownProcess.push_back("Transportation"); m_KnownProcessID.push_back(c_ProcessIDTransportation);
  m_KnownProcess.push_back("NoProcess"); m_KnownProcessID.push_back(c_ProcessIDNoProcess);


  // To lower all processes:
  for (unsigned int i = 0; i < m_KnownProcess.size(); ++i) m_KnownProcessFrequency.push_back(0);  
  
  for (unsigned int i = 0; i < m_KnownProcess.size(); ++i) m_KnownProcessFrequency.push_back(0);
  m_KnownProcessCounter = 0;
  m_KnownProcessUpdateCounter = 1000;
  
  m_PreventEventStuckBugCounter = 0;
}


/******************************************************************************
 * Does nothing
 */
MCSteppingAction::~MCSteppingAction()
{
  // Intentionally left blank
}


/******************************************************************************
 * If during the step a Compton or pair process happend in the primary track,
 * the process information (positions, direction, energies) are transmitted 
 * to the event action class, for later file output
 */
void MCSteppingAction::UserSteppingAction(const G4Step* Step)
{
  G4Track* Track = Step->GetTrack();

  massert(Track->GetUserInformation() != 0);
  
  if (Track->GetTrackID() != m_TrackId) {
    m_NSecondaries = 0;
    m_TrackId = Track->GetTrackID();
  }
  int GeneratedSecondaries = 0;
  if ((int) fpSteppingManager->GetSecondary()->size() > m_NSecondaries) {
    GeneratedSecondaries = fpSteppingManager->GetSecondary()->size() - m_NSecondaries;
  }
  m_NSecondaries = fpSteppingManager->GetSecondary()->size();
  
  MCEventAction* EventAction = (MCEventAction *) (G4EventManager::GetEventManager()->GetUserEventAction());
 
  double Time = Step->GetPostStepPoint()->GetGlobalTime()/second;

  if (EventAction->IsAborted() == true) {
    Track->SetTrackStatus(fKillTrackAndSecondaries);
    return;
  }
  
  // Quick sanity check to prevent some Geant4 hick-ups:
  if (std::isnan(Track->GetPosition().getX())) {
    merr<<"Geant4 hick-up: Detected NaN! Aborting track!"<<endl;
    Track->SetTrackStatus(fStopAndKill);
    return;
  }
  
  if (Track->GetPosition() != m_PreventEventStuckBugPosition) {
    m_PreventEventStuckBugPosition = Track->GetPosition();
    m_PreventEventStuckBugCounter = 0;
  } else {
    m_PreventEventStuckBugCounter++;
    if (m_PreventEventStuckBugCounter > 1000) {
      merr<<"Geant4 hick-up: The event seems to be stuck at the same position for 1000 steps! Aborting track!"<<endl;
      Track->SetTrackStatus(fStopAndKill);
      return;
    }
  }
  
  if (Track->GetCurrentStepNumber() > 10000000) {
    merr<<"Geant4 hick-up: The event seems to be stuck 10,000,000 steps! Aborting track!"<<endl;
    Track->SetTrackStatus(fStopAndKill);
    return;
  }

  // Prepare the IA interactions:
  if (Step->GetPostStepPoint()->GetProcessDefinedStep() != 0) {
    G4String ProcessName = Step->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName();
    int ProcessID = GetProcessId(ProcessName);

      /*
      for (int ss = (int) fpSteppingManager->GetSecondary()->size()-1; 
           ss > (int) fpSteppingManager->GetSecondary()->size()-1 - 
             GeneratedSecondaries; --ss) {
        G4Track* TrackA = (*fpSteppingManager->GetSecondary())[ss];
        if (GetParticleType(TrackA->GetDefinition()) > 1000) {
          mout<<"Found radioactive particle, which has decayed: "<<Track->GetDefinition()->GetParticleName()<<"(=target or projectile) to "<<TrackA->GetDefinition()->GetParticleName()<<endl;
        }
      }
      */

    // The following is sorted by physical process
    // Although there is some ambiguity, definitely keep it this way, since changes are likely to differ for the different processes!!! 

    if (ProcessID == c_ProcessIDCompton) {

      // Take care of secondaries below the threshold, for which no new track
      // is generated:
      if (GeneratedSecondaries == 0) {

        m_InteractionId++;
        // The photon may deposit a small bit of energy locally
        // To aid the response generation, we add handle this energy as if it originated from the new electron track
        ((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

        EventAction->AddIA("COMP", 
                           m_InteractionId,
                           ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                           GetDetectorId(Step->GetPreStepPoint()),
                           Time,
                           Track->GetPosition(),
                           1,
                           Track->GetMomentumDirection(),
                           Track->GetPolarization(),
                           Track->GetKineticEnergy(),
                           3,
                           G4ThreeVector(0.0, 0.0, 0.0),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           Step->GetTotalEnergyDeposit());

      } else {

        for (int se = (int) fpSteppingManager->GetSecondary()->size() - GeneratedSecondaries; 
             se < (int) fpSteppingManager->GetSecondary()->size(); ++se) {
          
          m_InteractionId++;
          // The photon may deposit a small bit of energy locally
          // To aid the response generation, we add handle this energy as if it originated from the new electron track
          ((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);
            
          G4Track* TrackA = (*fpSteppingManager->GetSecondary())[se];
          TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));
          
          // Now write some data about the real interaction positions 
          // in order to compare with the measured ones:
          EventAction->AddIA("COMP", 
                             m_InteractionId,
                             ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                             GetDetectorId(Step->GetPreStepPoint()),
                             Time,
                             TrackA->GetPosition(),
                             1,
                             Track->GetMomentumDirection(),
                             Track->GetPolarization(),
                             Track->GetKineticEnergy(),
                             GetParticleType(TrackA->GetDefinition()),
                             TrackA->GetMomentumDirection(),
                             TrackA->GetPolarization(),
                             TrackA->GetKineticEnergy());
          
        }
      }

    } else if (ProcessID == c_ProcessIDPair) {

      if (GeneratedSecondaries != 2) {
        mdebug<<"Pair creation should generate two secondaries! Most likely your thresholds are too high!"<<endl
              <<"The simulation generated "<<GeneratedSecondaries<<" secondaires and "
              <<"a local energy deposit of "<<Step->GetTotalEnergyDeposit()/keV<<" keV"<<show;
      }

      for (int se = (int) fpSteppingManager->GetSecondary()->size()-1; 
           se > (int) fpSteppingManager->GetSecondary()->size()-1 - 
             GeneratedSecondaries; --se) {

        m_InteractionId++;
        //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);
 
        G4Track* TrackA = (*fpSteppingManager->GetSecondary())[se];
        TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));


        // Now write some data about the real interaction positions 
        // in order to compare with the measured ones:
        EventAction->AddIA("PAIR", 
                           m_InteractionId,
                           ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                           GetDetectorId(Step->GetPreStepPoint()),
                           Time,
                           TrackA->GetPosition(),
                           1,
                           G4ThreeVector(0.0, 0.0, 0.0),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           0.0,
                           GetParticleType(TrackA->GetDefinition()),
                           TrackA->GetMomentumDirection(),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           TrackA->GetKineticEnergy());
        
      }
    } else if (ProcessID == c_ProcessIDAnnihilation) {
      // Annihilation should generate at least two particles, e.g. two 511keV photons

      // Take care of secondaries below the threshold, 
      // for which no new track is generated:
      for (int se = (int) fpSteppingManager->GetSecondary()->size()-1; 
           se > (int) fpSteppingManager->GetSecondary()->size()-1 - 
             GeneratedSecondaries; --se) {

        m_InteractionId++;
        //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

        G4Track* TrackA = (*fpSteppingManager->GetSecondary())[se];
        TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));


        // Now write some data about the real interaction positions 
        // in order to compare with the measured ones:
        EventAction->AddIA("ANNI", 
                           m_InteractionId,
                           ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                           GetDetectorId(Step->GetPreStepPoint()),
                           Time,
                           TrackA->GetPosition(),
                           GetParticleType(Track->GetDefinition()),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           0.0,
                           GetParticleType(TrackA->GetDefinition()),
                           TrackA->GetMomentumDirection(),
                           TrackA->GetPolarization(),
                           TrackA->GetKineticEnergy());
 
      }     
    } else if (ProcessID == c_ProcessIDBremsstrahlung) {

      // Take care of secondaries below the threshold, for which no new track
      // is generated:
      if (GeneratedSecondaries == 0) {

        m_InteractionId++;
        //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

        EventAction->AddIA("BREM", 
                           m_InteractionId,
                           ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                           GetDetectorId(Step->GetPreStepPoint()),
                           Time,
                           Track->GetPosition(),
                           GetParticleType(Track->GetDefinition()),
                           Track->GetMomentumDirection(),
                           Track->GetPolarization(),
                           Track->GetKineticEnergy(),
                           0,
                           G4ThreeVector(0.0, 0.0, 0.0),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           0.0);

      } else {

        for (int se = (int) fpSteppingManager->GetSecondary()->size() - GeneratedSecondaries; 
             se < (int) fpSteppingManager->GetSecondary()->size(); ++se) {
          
          m_InteractionId++;
          //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);
            
          G4Track* TrackA = (*fpSteppingManager->GetSecondary())[se];
          TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));
          
          EventAction->AddIA("BREM", 
                             m_InteractionId,
                             ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                             GetDetectorId(Step->GetPreStepPoint()),
                             Time,
                             TrackA->GetPosition(),
                             GetParticleType(Track->GetDefinition()),
                             Track->GetMomentumDirection(),
                             Track->GetPolarization(),
                             Track->GetKineticEnergy(),
                             GetParticleType(TrackA->GetDefinition()),
                             TrackA->GetMomentumDirection(),
                             TrackA->GetPolarization(),
                             TrackA->GetKineticEnergy());
          
        }
      }

    } else if (ProcessID == c_ProcessIDRayleigh) {

      // No secondary should have been generated
      if (GeneratedSecondaries != 0) {
        merr<<"Rayleigh scattering generated "<<GeneratedSecondaries<<" secondaries!"<<show;
      }

      m_InteractionId++;
      //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

      // This process only modifies the direction of the photons:
     
      // Now write some data about the real interaction positions 
      // in order to compare with the measured ones:
      EventAction->AddIA("RAYL", 
                         m_InteractionId,
                         ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                         GetDetectorId(Step->GetPreStepPoint()),
                         Time,
                         Track->GetPosition(),
                         1,
                         Track->GetMomentumDirection(),
                         Track->GetPolarization(),
                         Track->GetKineticEnergy(),
                         0,
                         G4ThreeVector(0.0, 0.0, 0.0),
                         G4ThreeVector(0.0, 0.0, 0.0),
                         0.0);

    } else if (ProcessID == c_ProcessIDPhoto) {

      // Take care of secondaries below the threshold, for which no new track
      // is generated:
      if (GeneratedSecondaries == 0) {

        m_InteractionId++;
        // The photon may deposit a small bit of energy locally
        // To aid the response generation, we add handle this energy as if it originated from the new electron track
        ((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

        EventAction->AddIA("PHOT", 
                           m_InteractionId,
                           ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                           GetDetectorId(Step->GetPreStepPoint()),
                           Time,
                           Track->GetPosition(),
                           1,
                           G4ThreeVector(0.0, 0.0, 0.0),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           0.0,
                           0,
                           G4ThreeVector(0.0, 0.0, 0.0),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           0.0);

      } else {

        for (int se = (int) fpSteppingManager->GetSecondary()->size() - GeneratedSecondaries; 
             se < (int) fpSteppingManager->GetSecondary()->size(); ++se) {
          
          m_InteractionId++;
          // The photon may deposit a small bit of energy locally
          // To aid the response generation, we add handle this energy as if it originated from the new electron track
          ((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);
            
          G4Track* TrackA = (*fpSteppingManager->GetSecondary())[se];
          TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));
          
          // Now write some data about the real interaction positions 
          // in order to compare with the measured ones:
          EventAction->AddIA("PHOT", 
                             m_InteractionId,
                             ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                             GetDetectorId(Step->GetPreStepPoint()),
                             Time,
                             TrackA->GetPosition(),
                             1,
                             G4ThreeVector(0.0, 0.0, 0.0),
                             G4ThreeVector(0.0, 0.0, 0.0),
                             0.0,
                             GetParticleType(TrackA->GetDefinition()),
                             TrackA->GetMomentumDirection(),
                             TrackA->GetPolarization(),
                             TrackA->GetKineticEnergy());
          
        }
      }
    } else if (ProcessID == c_ProcessIDElasticScattering) {

      // There always has to be a generated secondary, since we generate a new nucleus
      //if (GeneratedSecondaries == 0) {
      //  mout<<"Hadronic elastic scattering without secondaries!"<<endl;
      //}

      // Take care of all secondary particles which are generated
      for (int se = (int) fpSteppingManager->GetSecondary()->size()-1; 
           se > (int) fpSteppingManager->GetSecondary()->size()-1 - GeneratedSecondaries; --se) {

        m_InteractionId++;
        //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

        G4Track* TrackA = (*fpSteppingManager->GetSecondary())[se];
        TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));

        // Now write some data about the real interaction positions 
        // in order to compare with the measured ones:
        EventAction->AddIA("ELAS", 
                           m_InteractionId,
                           ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                           GetDetectorId(Step->GetPreStepPoint()),
                           Time,
                           Track->GetPosition(),
                           GetParticleType(Track->GetDefinition()),
                           Track->GetMomentumDirection(),
                           Track->GetPolarization(),
                           Track->GetKineticEnergy(),
                           //TrackA->GetDefinition()->GetExcitationEnergy(),
                           //((MString(Track->GetDefinition()->GetParticleName().c_str()).Contains("[") == true) ? dynamic_cast<G4Ions*>(TrackA->GetDefinition())->GetExcitationEnergy() : 0.0),
                           GetParticleType(TrackA->GetDefinition()),
                           TrackA->GetMomentumDirection(),
                           TrackA->GetPolarization(),
                           TrackA->GetKineticEnergy());
      
      }
    } else if (ProcessID == c_ProcessIDFission) {

      // There always has to be a generated secondary, since we generate a new nuclei, etc.
      if (GeneratedSecondaries == 0) {
        mout<<"Fission without secondaries??? - Geant4 behaves sometimes peculiar..."<<endl;
        EventAction->AddIA("FISS", 
                          m_InteractionId,
                          ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                          GetDetectorId(Step->GetPreStepPoint()),
                          Time,
                          Track->GetPosition(),
                          GetParticleType(Track->GetDefinition()),
                          Track->GetMomentumDirection(),
                          Track->GetPolarization(),
                          Track->GetKineticEnergy(),
                          0,
                          G4ThreeVector(0.0, 0.0, 0.0),
                          G4ThreeVector(0.0, 0.0, 0.0),
                          0.0);
      } else {
        // Take care of all secondary particles which are generated
        for (int se = (int) fpSteppingManager->GetSecondary()->size()-1; 
           se > (int) fpSteppingManager->GetSecondary()->size()-1 - GeneratedSecondaries; --se) {

          m_InteractionId++;
          //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

          G4Track* TrackA = (*fpSteppingManager->GetSecondary())[se];
          TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));

          // Now write some data about the real interaction positions 
          // in order to compare with the measured ones:
          EventAction->AddIA("FISS", 
                            m_InteractionId,
                            ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                            GetDetectorId(Step->GetPreStepPoint()),
                            Time,
                            Track->GetPosition(),
                            GetParticleType(Track->GetDefinition()),
                            Track->GetMomentumDirection(),
                            Track->GetPolarization(),
                            Track->GetKineticEnergy(),
                            //TrackA->GetDefinition()->GetExcitationEnergy(),
                            //((MString(Track->GetDefinition()->GetParticleName().c_str()).Contains("[") == true) ? dynamic_cast<G4Ions*>(TrackA->GetDefinition())->GetExcitationEnergy() : 0.0),
                            GetParticleType(TrackA->GetDefinition()),
                            TrackA->GetMomentumDirection(),
                            TrackA->GetPolarization(),
                            TrackA->GetKineticEnergy());
      
        }
      }
    } else if (ProcessID == c_ProcessIDInelasticScattering) {

      // There always has to be a generated secondary, since we generate a new nucleus
      if (GeneratedSecondaries == 0) {
        mout<<"The Inelastic scattering of "<<Track->GetDefinition()->GetParticleName()<<" didn't generate secondaries! Either your thresholds for generating secondaires are too high or a simulation issue occurred!"<<endl;
      }

      // Take care of all secondary particles which are generated
      for (int se = (int) fpSteppingManager->GetSecondary()->size()-1; 
           se > (int) fpSteppingManager->GetSecondary()->size()-1 - GeneratedSecondaries; --se) {

        m_InteractionId++;
        //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

        G4Track* TrackA = (*fpSteppingManager->GetSecondary())[se];
        TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));

        // Now write some data about the real interaction positions 
        // in order to compare with the measured ones:
        EventAction->AddIA("INEL", 
                           m_InteractionId,
                           ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                           GetDetectorId(Step->GetPreStepPoint()),
                           Time,
                           Track->GetPosition(),
                           GetParticleType(Track->GetDefinition()),
                           Track->GetMomentumDirection(),
                           Track->GetPolarization(),
                           Track->GetKineticEnergy(),
                           //TrackA->GetDefinition()->GetExcitationEnergy(),
                           //((MString(Track->GetDefinition()->GetParticleName().c_str()).Contains("[") == true) ? dynamic_cast<G4Ions*>(TrackA->GetDefinition())->GetExcitationEnergy() : 0.0),
                           GetParticleType(TrackA->GetDefinition()),
                           TrackA->GetMomentumDirection(),
                           TrackA->GetPolarization(),
                           TrackA->GetKineticEnergy());
      
      }
    } else if (ProcessID == c_ProcessIDCapture) {

      // There always has to be a generated secondary, since we generate a new nucleus
      if (GeneratedSecondaries == 0) {
        mout<<"The hadron capture of "<<Track->GetDefinition()->GetParticleName()<<" didn't generate secondaries! Either your thresholds for generating secondaires are too high or a simulation issue occurred!"<<endl;
      }

      // Take care of secondaries below the threshold, 
      // for which no new track is generated:
      for (int se = (int) fpSteppingManager->GetSecondary()->size()-1; 
           se > (int) fpSteppingManager->GetSecondary()->size()-1 - GeneratedSecondaries; --se) {

        m_InteractionId++;
        //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

        G4Track* TrackA = (*fpSteppingManager->GetSecondary())[se];
        TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));


        // Now write some data about the real interaction positions 
        // in order to compare with the measured ones:
        EventAction->AddIA("CAPT", 
                           m_InteractionId,
                           ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                           GetDetectorId(Step->GetPreStepPoint()),
                           Time,
                           TrackA->GetPosition(),
                           GetParticleType(Track->GetDefinition()),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           0.0,
                           GetParticleType(TrackA->GetDefinition()),
                           TrackA->GetMomentumDirection(),
                           TrackA->GetPolarization(),
                           TrackA->GetKineticEnergy());
        
      }
    } else if (ProcessID == c_ProcessIDDecay) {

      // There always has to be a generated secondary, since something decays into something else
      // "19" doesn't generate secondaries in Geant4... 
      if (GeneratedSecondaries == 0 && GetParticleType(Track->GetDefinition()) != 19) {
        mout<<"The the decay of "<<Track->GetDefinition()->GetParticleName()<<" didn't generate secondaries! Either your thresholds for generating secondaires are too high or a simulation issue occurred!"<<endl;
      }

      // Take care of secondaries below the threshold, 
      // for which no new track is generated:
      for (int se = (int) fpSteppingManager->GetSecondary()->size()-1; 
           se > (int) fpSteppingManager->GetSecondary()->size()-1 - GeneratedSecondaries; --se) {

        m_InteractionId++;
        //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

        G4Track* TrackA = (*fpSteppingManager->GetSecondary())[se];
        TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));
      
        // Now write some data about the real interaction positions 
        // in order to compare with the measured ones:
        EventAction->AddIA("DECA", 
                           m_InteractionId,
                           ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                           GetDetectorId(Step->GetPreStepPoint()),
                           Time,
                           TrackA->GetPosition(),
                           GetParticleType(Track->GetDefinition()),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           0.0,
                           GetParticleType(TrackA->GetDefinition()),
                           TrackA->GetMomentumDirection(),
                           TrackA->GetPolarization(),
                           TrackA->GetKineticEnergy());
        
      }
    } else if (ProcessID == c_ProcessIDRadioactiveDecay) {

      if (GeneratedSecondaries == 0) {
        //cout<<"RadioactiveDecay without secondaries from particle: "<<Track->GetDefinition()->GetParticleName()<<endl;
      } else {
        
        //bool MetaStable = true;
        //if (MString(Track->GetDefinition()->GetParticleName().c_str()).Contains("[0.0]") == true) {
        //  MetaStable = false;
        //}
        
        /*
        if (m_PreventLastDecayBug == Track->GetDefinition()->GetParticleName().c_str()) {
          merr<<"Infinite decay loop for isotope "<<Track->GetDefinition()->GetParticleName().c_str()<<" detected. This is a Geant4 bug... Skipping track"<<endl;
          Track->SetTrackStatus(fKillTrackAndSecondaries);
          return;
        } else {
          m_PreventLastDecayBug = Track->GetDefinition()->GetParticleName().c_str();
        }
        */
  
        // The decay is the first process happening
        bool IsPrimaryDecay = false;
        if (((MCTrackInformation*) Track->GetUserInformation())->GetOriginId() <= int(m_InitialParticles.size())) {
          IsPrimaryDecay = true;
        }

        bool IsInitialParticleFromBuildUpSource = false;
        if (m_ParticleOriginIsBuildUpSource == true) {
          if (((MCTrackInformation*) Track->GetUserInformation())->GetOriginId() <= int(m_InitialParticles.size())) {
            for (unsigned int i = 0; i < m_InitialParticles.size(); ++i) {
              if (GetParticleType(Track->GetDefinition()) == m_InitialParticles[i]) {
                IsInitialParticleFromBuildUpSource = true;
                break;
              }
            }
          }
        }

        // How long after the initial particle did the decay happen?
        double TimeDelay = 0.0;
        if (GeneratedSecondaries > 0) {
          //cout<<"TrackG:"<<Track->GetGlobalTime()/second<<"sec vs.  SecG:"<<(*fpSteppingManager->GetSecondary())[0]->GetGlobalTime()/second<<"sec."<<endl;
          TimeDelay = (*fpSteppingManager->GetSecondary())[0]->GetGlobalTime() - Step->GetPreStepPoint()->GetGlobalTime();
        }
        //cout<<"TimeDelay: "<<TimeDelay/second<<"s max: "<<MaxTimeDelay/second<<"s"<<endl;

        //mout<<((MetaStable == true) ? "Meta" : "not meta")<<endl;
        //mout<<"Activation mode: "<<m_DecayMode<<endl;



        // Four flags govern the handling of the decay tracks:

        // Keep: Keep the track in the current event and handle all secondaries
        bool Keep = true;
        // Store: add the event to an isotope list (only true for DecayModeActivationBuildUp)
        bool Store = false;
        // FutureEvent: Delay processing of this decay to its proper time in the future
        bool FutureEvent = false;
        // DoNotStart: If the event is from an isotope list file, do not start the next event from this list, because this decay is forwarded to the
        // FurtureEvent list to be in correct decay-chain and time sequence.
        bool DoNotStart = false;
        
        if (m_DecayMode == MCParameterFile::c_DecayModeNormal) {
          Keep = true;
          Store = false;
          FutureEvent = false;
          DoNotStart = false;
        } else if (m_DecayMode == MCParameterFile::c_DecayModeIgnore) {
          Keep = false;
          Store = false;
          FutureEvent = false;
          DoNotStart = false;
        } else if (m_DecayMode == MCParameterFile::c_DecayModeBuildUp) {
          if (IsInitialParticleFromBuildUpSource == true) {
            // Simulate primaries now:
            Keep = true;
            Store = false;
            FutureEvent = false;
            DoNotStart = false;
          } else {
            // Delay secondary decays to the future:
            if (TimeDelay > m_DetectorTimeConstant) {
              Keep = false;
              Store = false;
              FutureEvent = true;
              DoNotStart = false;
            } else {
              Keep = true;
              Store = false;
              FutureEvent = false;
              DoNotStart = false;
            }
          }
        } else if (m_DecayMode == MCParameterFile::c_DecayModeActivationBuildUp) {
          // Only store non-instantaneous decays (instantaneous would be vetoes with incoming proton!)
          if (TimeDelay > m_DetectorTimeConstant) {
            Keep = false;
            Store = true;
            FutureEvent = false;
            DoNotStart = false;
          } else {
            Keep = true;
            Store = false;
            FutureEvent = false;
            DoNotStart = false;
          }
        } else if (m_DecayMode == MCParameterFile::c_DecayModeActivationDelayedDecay) {
          if (IsPrimaryDecay == true) {
            Keep = true;
            Store = false;
            FutureEvent = false;
            DoNotStart = false;
          } else {
            if (TimeDelay > m_DetectorTimeConstant) {
              Keep = false;
              Store = false;
              FutureEvent = true;
              DoNotStart = false;
            } else {
              Keep = true;
              Store = false;
              FutureEvent = false;
              DoNotStart = true;
            }
          }
        }
        //cout<<"  P:"<<int(IsPrimaryDecay)<<"  K:"<<int(Keep)<<"  S:"<<int(Store)<<"  F:"<<int(FutureEvent)<<endl;
        //cout<<"Handling: "<<dynamic_cast<G4Ions*>(Track->GetDefinition())->GetParticleName()<<endl;
        
        if (Store == true) {
          // Since we store only when we do not keep, this code occurs only once per decay
 
          // Make sure the excitation energy is unique to avoid storing nucleii with similar excitation energies
          G4Ions* Nucleus = dynamic_cast<G4Ions*>(Track->GetDefinition());
          //cout<<"Storing: "<<Nucleus->GetParticleName()<<endl;
          if (Nucleus->GetExcitationEnergy() > 1*keV) {
            //cout<<"Alignment > 1 keV"<<endl;
            const G4LevelManager* M = G4NuclearLevelData::GetInstance()->GetLevelManager(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass());
	    bool IsValid =true ;// IsValid() method no longer available for G4LevelManager, always true for now , need to check 
            if (IsValid == true) {
              const G4NucLevel* Level = M->NearestLevel(Nucleus->GetExcitationEnergy());
              if (Level != 0) {
                G4IonTable* Table = G4IonTable::GetIonTable();
		
		 if(Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::no_Float)!=-1001){ 
    Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::no_Float));}
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_X)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_X)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_Y)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_Y)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_Z)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_Z)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_U)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_U)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_V)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_V)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_W)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_W)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_R)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_R)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_S)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_S)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_T)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_T)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_T)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_T)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_A)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_A)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_B)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_B)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_C)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_C)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_D)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_D)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_E)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy()),G4Ions::G4FloatLevelBase::plus_E)); }
		
    else { cout<<"Isotope not found from MCSteppingAction"<<endl;
           cout<< Nucleus->GetAtomicNumber() << Nucleus->GetAtomicMass()<< M->NearestLevelEnergy(Nucleus->GetExcitationEnergy())<<endl;
           Nucleus = nullptr;
    }	 	
		
               // Nucleus = dynamic_cast<G4Ions*>(Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), M->NearestLevelEnergy(Nucleus->GetExcitationEnergy())));
              }
            }
          } else {
            //cout<<"Alignment < 1 keV"<<endl;
            G4IonTable* Table = G4IonTable::GetIonTable();
            //Nucleus = dynamic_cast<G4Ions*>(Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0));
	    
	    
	    if(Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::no_Float)!=-1001){ 
    Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::no_Float));}
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_X)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_X)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_Y)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_Y)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_Z)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_Z)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_U)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_U)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_V)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_V)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_W)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_W)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_R)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_R)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_S)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_S)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_T)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_T)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_T)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_T)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_A)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_A)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_B)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_B)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_C)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_C)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_D)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_D)); }
    
    else if( Table->GetLifeTime(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_E)!=-1001)
    { Nucleus = dynamic_cast<G4Ions*>( Table->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0,G4Ions::G4FloatLevelBase::plus_E)); }
		
    else { cout<<"Isotope not found from MCSteppingAction"<<endl;
           cout<< "Z : "<<Nucleus->GetAtomicNumber()<< "A :"<< Nucleus->GetAtomicMass()<<"E : " <<0.0<<endl;
           Nucleus = nullptr;
    }	 	
	    
	    
          }
          //cout<<"NE: "<<Nucleus->GetExcitationEnergy()/keV<<"keV"<<endl;

          G4TouchableHistory* Hist = (G4TouchableHistory*) (Step->GetPreStepPoint()->GetTouchable());
          MCRunManager::GetMCRunManager()->GetCurrentRun().AddIsotope(Nucleus, Hist);

          //mout<<"Storing isotope: "<<Nucleus->GetParticleName()<<endl;
          
          EventAction->AddComment("Storing isotope: " + Nucleus->GetParticleName());
        }
        
        if (Keep == true) {
          //cout<<"Keep"<<endl;
          // Take care of secondaries below the threshold, 
          // for which no new track is generated:
          
          //mout<<"Keeping isotope: "<<GetParticleType(Track->GetDefinition())<<endl;
          
          // There always has to be a generated secondary, since we generate a new nucleus
          //if (GeneratedSecondaries == 0) {
          //  merr<<"RadioactiveDecay didn't generate secondaries! Either your thresholds are insane or a simulation error occurred!"<<show;
          //}
          
          for (int se = (int) fpSteppingManager->GetSecondary()->size()-1; 
               se > (int) fpSteppingManager->GetSecondary()->size()-1 - 
                 GeneratedSecondaries; --se) {
            
            m_InteractionId++;
            //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);
            
            G4Track* TrackA = (*fpSteppingManager->GetSecondary())[se];
            TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));
            
            if (m_DecayMode == MCParameterFile::c_DecayModeBuildUp || 
                m_DecayMode == MCParameterFile::c_DecayModeActivationBuildUp ||
                m_DecayMode == MCParameterFile::c_DecayModeActivationDelayedDecay) {
              TrackA->SetGlobalTime(0.0);
            }

            // Now write some data about the real interaction positions 
            // in order to compare with the measured ones:
            EventAction->AddIA("DECA",
                               m_InteractionId,
                               ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                               GetDetectorId(Step->GetPreStepPoint()),
                               TrackA->GetGlobalTime()/second,
                               TrackA->GetPosition(),
                               GetParticleType(Track->GetDefinition()),
                               G4ThreeVector(0.0, 0.0, 0.0),
                               G4ThreeVector(0.0, 0.0, 0.0),
                               //dynamic_cast<G4Ions*>(Track->GetDefinition())->GetExcitationEnergy(),
                               0.0,
                               GetParticleType(TrackA->GetDefinition()),
                               TrackA->GetMomentumDirection(),
                               TrackA->GetPolarization(),
                               TrackA->GetKineticEnergy());
            
          }
        } else {
          // Remove the secondaries
          //cout<<"Don't keep - kill track"<<endl;
          Track->SetTrackStatus(fKillTrackAndSecondaries);
        }

        if (DoNotStart == true && TimeDelay > m_DetectorTimeConstant) {
          //cout<<"DoNotStart: "<<Track->GetDefinition()<<" at t="<<Step->GetPostStepPoint()->GetGlobalTime()/second<<"s  -  TD:"<<TimeDelay/second<<"  MTD:"<<m_DetectorTimeConstant/second<<endl;
          //cout<<Track->GetDefinition()->GetParticleName()<<" --> "<<Step->GetPostStepPoint()->GetGlobalTime()/second<<" sec "<<Step->GetPreStepPoint()->GetGlobalTime()/second<<" sec"<<endl;
          G4TouchableHistory* Hist = (G4TouchableHistory*) (Step->GetPreStepPoint()->GetTouchable());
          G4LogicalVolume* V = Hist->GetVolume(0)->GetLogicalVolume();
          G4String Name = V->GetName(); 
          Name.erase(Name.size()-3, 3);
          MCRunManager::GetMCRunManager()->GetCurrentRun().SkipOneEvent(Track->GetDefinition(), Name);
        }
        
        if (FutureEvent == true) { // && Track->GetDefinition()->GetParticleName() != "Ge73[13.3]") {
          //cout<<"Future: "<<Track->GetDefinition()->GetParticleName()<<" decays in "<<Step->GetPostStepPoint()->GetGlobalTime()/second<<" sec (now: "<<Step->GetPreStepPoint()->GetGlobalTime()/second<<" sec)"<<endl;
 
          // Get the volume name, so that we know from which volume we have to skip an event later:
          G4TouchableHistory* Hist = (G4TouchableHistory*) (Step->GetPreStepPoint()->GetTouchable());
          G4LogicalVolume* V = Hist->GetVolume(0)->GetLogicalVolume();
          G4String Name = V->GetName();
          Name.erase(Name.size()-3, 3);

          double GlobalTime = Step->GetPostStepPoint()->GetGlobalTime();
//           if (Track->GetDefinition()->GetParticleName() == "Ge73[66.7]") {
//             Time = gRandom->Exp(0.499*s/log(2));
//             cout<<"New time: "<<GlobalTime<<endl;
//           }

          MCRunManager::GetMCRunManager()->GetCurrentRun().AddToBuildUpEventList(0.0,
                                                                                 Track->GetPosition(),
                                                                                 G4ThreeVector(0.0, 0.0, 0.0),
                                                                                 G4ThreeVector(0.0, 0.0, 0.0),
                                                                                 GlobalTime,
                                                                                 //step->GetPostStepPoint()->GetGlobalTime(),
                                                                                 Track->GetDefinition(),
                                                                                 Name);
          //cout<<"Global time: "<<Step->GetPostStepPoint()->GetGlobalTime()/second<<"sec"<<endl;
          
          EventAction->AddComment("Future decay: " + Track->GetDefinition()->GetParticleName() + " at t=" + to_string(GlobalTime/second) + " sec");
        }
      }
    } else if (ProcessID == c_ProcessIDIonization && m_StoreIonization == true) {
      
      // Since this is a continuous process, replace (!) the user information 
      // in order to make the HTsim origin information relate to this hit
      //dynamic_cast<MCTrackInformation*>(Track->GetUserInformation())->SetId(m_InteractionId+1);
      
      if (GeneratedSecondaries == 0) {
       
        m_InteractionId++;
        //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

        EventAction->AddIA("IONI",
                           m_InteractionId,
                           ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                           GetDetectorId(Step->GetPreStepPoint()),
                           Time,
                           Track->GetPosition(), // This is the Step->GetPostStepPoint()
                           GetParticleType(Track->GetDefinition()),
                           Track->GetMomentumDirection(),
                           Track->GetPolarization(),
                           Track->GetKineticEnergy(),
                           0,
                           G4ThreeVector(0.0, 0.0, 0.0),
                           G4ThreeVector(0.0, 0.0, 0.0), 
                           0.0);
        
      } else {
        // Take care of secondary electrons generate by ionization
        for (int se = (int) fpSteppingManager->GetSecondary()->size() - GeneratedSecondaries; 
             se < (int) fpSteppingManager->GetSecondary()->size(); ++se) {
          
          m_InteractionId++;
          //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

          G4Track* TrackA = (*fpSteppingManager->GetSecondary())[se];
          MCTrackInformation* Info = 
            new MCTrackInformation(m_InteractionId, m_InteractionId);
          TrackA->SetUserInformation(Info);
          
          // Now write some data about the real interaction positions 
          // in order to compare with the measured ones:
          EventAction->AddIA("IONI",
                             m_InteractionId,
                             ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                             GetDetectorId(Step->GetPreStepPoint()),
                             Time,
                             TrackA->GetPosition(), // This is the Step->GetPostStepPoint()
                             GetParticleType(Track->GetDefinition()),
                             Track->GetMomentumDirection(),
                             Track->GetPolarization(),
                             Track->GetKineticEnergy(),
                             GetParticleType(TrackA->GetDefinition()),
                             TrackA->GetMomentumDirection(),
                             TrackA->GetPolarization(), 
                             TrackA->GetKineticEnergy());
        }
      }
    } else if (ProcessID == c_ProcessIDTransportation && m_StoreIonization == true) {
      // This is a special case:
      // If there is an ionization folled by a boundary transport,
      // the ionization keyword is overwritten with the Transportation keyword
      // (True for Geant4 version 911)
      // Thus we rename the transportation here to ionization

      //dynamic_cast<MCTrackInformation*>(Track->GetUserInformation())->SetId(m_InteractionId+1);

      if (GeneratedSecondaries == 0) {
        if (Step->GetTotalEnergyDeposit() > 1E-9*keV) { // avoid "vacuum" deposits

          m_InteractionId++;
          //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

          EventAction->AddIA("IONI",
                             m_InteractionId,
                             ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                             GetDetectorId(Step->GetPreStepPoint()),
                             Time,
                             Track->GetPosition(),
                             GetParticleType(Track->GetDefinition()),
                             Track->GetMomentumDirection(),
                             Track->GetPolarization(),
                             Track->GetKineticEnergy(),
                             0,
                             G4ThreeVector(0.0, 0.0, 0.0),
                             G4ThreeVector(0.0, 0.0, 0.0), 
                             0.0);
        }
      } else {
        // Take care of secondaries
        for (int se = (int) fpSteppingManager->GetSecondary()->size() - GeneratedSecondaries; 
             se < (int) fpSteppingManager->GetSecondary()->size(); ++se) {
          
          m_InteractionId++;
          //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);
          
          G4Track* TrackA = (*fpSteppingManager->GetSecondary())[se];
          TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));
          
          // Now write some data about the real interaction positions 
          // in order to compare with the measured ones:
          EventAction->AddIA("IONI",
                             m_InteractionId,
                             ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                             GetDetectorId(Step->GetPreStepPoint()),
                             Time,
                             TrackA->GetPosition(),
                             GetParticleType(Track->GetDefinition()),
                             Track->GetMomentumDirection(),
                             Track->GetPolarization(),
                             Track->GetKineticEnergy(),
                             GetParticleType(TrackA->GetDefinition()),
                             TrackA->GetMomentumDirection(),
                             TrackA->GetPolarization(), 
                             TrackA->GetKineticEnergy());
        }
      }
    } else {
      if (ProcessID == c_ProcessIDUncovered) {
        merr<<"Uncovered process: "<<ProcessName<<" - please inform the lead developers of this problem."<<endl
            <<"-> Your simulations are completely OK - you only might miss an IA line in the sim file."<<show;
      }

  
      // For speed ups:
      // We have a FIFO stack here, thus if we start from the end we can stop the search 
      // if we have found the first one with an entry
      if (fpSteppingManager->GetSecondary()->size() > 0) {
        for (unsigned int se = fpSteppingManager->GetSecondary()->size()-1; 
            se < fpSteppingManager->GetSecondary()->size(); --se) { 
          if (fpSteppingManager->GetSecondary()->at(se)->GetUserInformation() != 0) break;
 
          // Set the additional Track info about the ID's
          MCTrackInformation* Info = 
            new MCTrackInformation(((MCTrackInformation*) Track->
                                    GetUserInformation())->GetId(), 
                                  ((MCTrackInformation*) Track->
                                    GetUserInformation())->GetOriginId());
          fpSteppingManager->GetSecondary()->at(se)->SetUserInformation(Info);
        }
      }
    } // All processes
  } 

  // Check if the particle escapes the world volume:
  if (Track->GetNextVolume() == 0) {

    m_InteractionId++;

    EventAction->AddIA("ESCP",
                       m_InteractionId,
                       ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                       GetDetectorId(Step->GetPreStepPoint()),
                       Time,
                       Track->GetPosition(),
                       GetParticleType(Track->GetDefinition()),
                       Track->GetMomentumDirection(),
                       Track->GetPolarization(),
                       Track->GetKineticEnergy(),
                       0,
                       G4ThreeVector(0.0, 0.0, 0.0),
                       G4ThreeVector(0.0, 0.0, 0.0), 
                       0.0);
    if (EventAction->IsAborted() == true) {
      Track->SetTrackStatus(fKillTrackAndSecondaries);
    }
  } else {
    //cout<<Track->GetNextVolume()->GetName()<<endl;
  }

  // Check if we enter or leave one of the watched volumes: 
  if (m_WatchedVolumes.size() > 0) {
    if (Step->GetPreStepPoint()->GetPhysicalVolume() != Step->GetPostStepPoint()->GetPhysicalVolume()) {
      for (unsigned int w = 0; w < m_WatchedVolumes.size(); ++w) {
        bool FoundInPre = false;
        bool FoundInPost = false;
        
        const G4VTouchable* Touchable = Step->GetPreStepPoint()->GetTouchable();
        for (int h = 0; h < Touchable->GetHistoryDepth(); ++h) {
          if (Touchable->GetVolume(h)->GetLogicalVolume()->GetName().c_str() == m_WatchedVolumesLog[w] ||
              Touchable->GetVolume(h)->GetName().c_str() == m_WatchedVolumes[w]) {
            FoundInPre = true;
            break;
          }
        }
        
        Touchable = Step->GetPostStepPoint()->GetTouchable();
        for (int h = 0; h < Touchable->GetHistoryDepth(); ++h) {
          if (Touchable->GetVolume(h)->GetLogicalVolume()->GetName().c_str() == m_WatchedVolumesLog[w] ||
              Touchable->GetVolume(h)->GetName().c_str() == m_WatchedVolumes[w]) {
            FoundInPost = true;
            break;
          }
        }

        if (FoundInPre == false && FoundInPost == true) {

          m_InteractionId++;

          EventAction->AddIA("ENTR",
                             m_InteractionId,
                             ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                             GetDetectorId(Step->GetPostStepPoint()),
                             Time,
                             Step->GetPostStepPoint()->GetPosition(),
                             GetParticleType(Track->GetDefinition()),
                             Step->GetPostStepPoint()->GetMomentumDirection(),
                             Step->GetPostStepPoint()->GetPolarization(),
                             Step->GetPostStepPoint()->GetKineticEnergy(),
                             0,
                             G4ThreeVector(0.0, 0.0, 0.0),
                             G4ThreeVector(0.0, 0.0, 0.0), 
                             0.0);
        }
                
        if (FoundInPre == true && FoundInPost == false) {

          m_InteractionId++;

          EventAction->AddIA("EXIT",
                             m_InteractionId,
                             ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                             GetDetectorId(Step->GetPreStepPoint()),
                             Time,
                             Step->GetPostStepPoint()->GetPosition(),
                             GetParticleType(Track->GetDefinition()),
                             Step->GetPostStepPoint()->GetMomentumDirection(),
                             Step->GetPostStepPoint()->GetPolarization(),
                             Step->GetPostStepPoint()->GetKineticEnergy(),
                             0,
                             G4ThreeVector(0.0, 0.0, 0.0),
                             G4ThreeVector(0.0, 0.0, 0.0), 
                             0.0);

        }
      }
    }
  }

  // Check if we enter a black absorber:
  if (m_BlackAbsorbers.size() > 0) {
    if (Step->GetPreStepPoint()->GetPhysicalVolume() != Step->GetPostStepPoint()->GetPhysicalVolume()) {
      for (unsigned int w = 0; w < m_BlackAbsorbers.size(); ++w) {
        bool FoundInPre = false;
        bool FoundInPost = false;
        
        for (int h = 0; h < Step->GetPreStepPoint()->GetTouchable()->GetHistoryDepth(); ++h) {
          if (Step->GetPreStepPoint()->GetTouchable()->GetVolume(h)->GetName().c_str() == m_BlackAbsorbers[w]) {
            FoundInPre = true;
            break;
          }
        }
        
        for (int h = 0; h < Step->GetPostStepPoint()->GetTouchable()->GetHistoryDepth(); ++h) {
          if (Step->GetPostStepPoint()->GetTouchable()->GetVolume(h)->GetName().c_str() == m_BlackAbsorbers[w]) {
            FoundInPost = true;
            break;
          }
        }

        if (FoundInPre == false && FoundInPost == true) {

          m_InteractionId++;

          EventAction->AddIA("BLAK",
                             m_InteractionId,
                             ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                             GetDetectorId(Step->GetPostStepPoint()),
                             Time,
                             Step->GetPostStepPoint()->GetPosition(),
                             GetParticleType(Track->GetDefinition()),
                             Step->GetPostStepPoint()->GetMomentumDirection(),
                             Step->GetPostStepPoint()->GetPolarization(),
                             Step->GetPostStepPoint()->GetKineticEnergy(),
                             0,
                             G4ThreeVector(0.0, 0.0, 0.0),
                             G4ThreeVector(0.0, 0.0, 0.0), 
                             0.0);
  
          Track->SetTrackStatus(fStopAndKill);

        }
      }
    }
  }

  // Check region - cut all secondaries
  G4Region* Region = Step->GetPreStepPoint()->GetPhysicalVolume()->GetLogicalVolume()->GetRegion();
  for (auto& R: m_Regions) {
    if (R.GetName() == Region->GetName().c_str()) {
      if (R.GetCutAllSecondaries() == true) {
        if (GeneratedSecondaries > 0) {
          for (int se = (int) fpSteppingManager->GetSecondary()->size() - GeneratedSecondaries; 
             se < (int) fpSteppingManager->GetSecondary()->size(); ++se) {
            fpSteppingManager->GetSecondary()->at(se)->SetTrackStatus(fStopAndKill);
          }
        }
      }
    }
  }
  
  // For all sensitive detectors post process the hits:
  // This code is identical to the call in G4SteppingManager!
  if (Step->GetPreStepPoint()->GetPhysicalVolume() != 0 && 
      Step->GetControlFlag() != AvoidHitInvocation) {
    G4VSensitiveDetector* Sensitive = Step->GetPreStepPoint()->GetSensitiveDetector();
    if (Sensitive != 0) {
      dynamic_cast<MCSD*>(Sensitive)->PostProcessHits(Step);
    }
  }

  // Adding the deposits in passive material has to be last in oder to have the digitized info
  MCTrackInformation* TrackInfo = 
    (MCTrackInformation*) (Track->GetUserInformation());
  if (TrackInfo != 0) {
    if (TrackInfo->IsDigitized() == true) {
      TrackInfo->SetDigitized(false);
    } else {
      if (Step->GetTotalEnergyDeposit() > 0) {
        EventAction->AddDepositPassiveMaterial(Step->GetTotalEnergyDeposit(), 
                                               Track->GetVolume()->GetLogicalVolume()->GetMaterial()->GetName());
        if (EventAction->IsAborted() == true) {
          Track->SetTrackStatus(fKillTrackAndSecondaries);
        }
      }
    }
  }

  return;
}


/******************************************************************************
 * Return the Process ID
 */
struct MCSteppingActionCompareDesc { 
  vector<unsigned long long> V;
  bool operator() (unsigned int i1, unsigned int i2) { return V[i1] > V[i2]; }
} MyCompareDesc;

int MCSteppingAction::GetProcessId(const G4String& Name)
{
  // Attention: In theory m_KnownProcessCounter and m_KnownProcessFrequency can overflow, but...
  
  if (m_KnownProcessCounter % m_KnownProcessUpdateCounter == 0) {
    // Do some simple sorting...
    vector<unsigned int> KnownProcessOrder;
    for(unsigned int i = 0; i < m_KnownProcess.size(); ++i) { KnownProcessOrder.push_back(i); }
    
    // Sort decending...
    /*struct CompareDesc { 
      //CompareDesc(vector<unsigned long long> v) : V(v) {}
      vector<unsigned long long> V;
      bool operator() (unsigned int i1, unsigned int i2) { return V[i1] > V[i2]; }
    } MyCompareDesc; */
    MyCompareDesc.V = m_KnownProcessFrequency;
    sort(KnownProcessOrder.begin(), KnownProcessOrder.end(), MyCompareDesc);
    
    vector<G4String> tKnownProcess = m_KnownProcess;
    vector<int> tKnownProcessID = m_KnownProcessID;
    vector<unsigned long long> tKnownProcessFrequency = m_KnownProcessFrequency;
    for (unsigned int i = 0; i < m_KnownProcess.size(); ++i) {
      m_KnownProcess[i] = tKnownProcess[KnownProcessOrder[i]];
      m_KnownProcessID[i] = tKnownProcessID[KnownProcessOrder[i]];
      m_KnownProcessFrequency[i] = tKnownProcessFrequency[KnownProcessOrder[i]];
    }
    
    /*
    cout<<endl;
    cout<<"Newly sorted process IDs for speed up:"<<endl;
    for (unsigned int i = 0; i < m_KnownProcess.size(); ++i) {
      cout<<m_KnownProcess[i]<<" --> "<<m_KnownProcessFrequency[i]<<endl;
    }
    cout<<endl;
    */
    
    m_KnownProcessUpdateCounter *= 10;
  }
  
  for (unsigned int i = 0; i < m_KnownProcess.size(); ++i) {
    if (Name == m_KnownProcess[i]) {
      ++m_KnownProcessCounter;
      ++m_KnownProcessFrequency[i];
      return m_KnownProcessID[i];
    }
  }
  //cout<<Name<<endl;
  return c_ProcessIDUncovered;
}
  

/******************************************************************************
 * Return the Cosima particle type ID from the particle name
 */
int MCSteppingAction::GetParticleType(G4String Name)
{
  if (Name == "gamma") {
    return MCSource::c_Gamma;

  } else if (Name == "e-") {
    return MCSource::c_Electron;
  } else if (Name == "e+") {
    return MCSource::c_Positron;

  } else if (Name == "proton") {
    return MCSource::c_Proton;
  } else if (Name == "anti_proton") {
    return MCSource::c_AntiProton;
  } else if (Name == "neutron") {
    return MCSource::c_Neutron;
  } else if (Name == "anti_neutron") {
    return MCSource::c_AntiNeutron;

  } else if (Name == "mu+") {
    return MCSource::c_MuonPlus;
  } else if (Name == "mu-") {
    return MCSource::c_MuonMinus;
  } else if (Name == "tau+") {
    return MCSource::c_TauonPlus;
  } else if (Name == "tau-") {
    return MCSource::c_TauonMinus;

  } else if (Name == "nu_e") {
    return MCSource::c_ElectronNeutrino;
  } else if (Name == "anti_nu_e") {
    return MCSource::c_AntiElectronNeutrino;
  } else if (Name == "nu_mu") {
    return MCSource::c_MuonNeutrino;
  } else if (Name == "anti_nu_mu") {
    return MCSource::c_AntiMuonNeutrino;
  } else if (Name == "nu_tau") {
    return MCSource::c_TauonNeutrino;
  } else if (Name == "anti_nu_tau") {
    return MCSource::c_AntiTauonNeutrino;

  } else if (Name == "deuteron") {
    return MCSource::c_Deuteron;
  } else if (Name == "triton") {
    return MCSource::c_Triton;
  } else if (Name == "He3") {
    return MCSource::c_He3;
  } else if (Name == "alpha") {
    return MCSource::c_Alpha;

  } else if (Name == "GenericIon") {
    return MCSource::c_GenericIon;

  } else if (Name == "pi+") {
    return MCSource::c_PiPlus;
  } else if (Name == "pi-") {
    return MCSource::c_PiMinus;
  } else if (Name == "pi0") {
    return MCSource::c_PiZero;
  } else if (Name == "eta") {
    return MCSource::c_Eta;
  } else if (Name == "eta_prime") {
    return MCSource::c_EtaPrime;

  } else if (Name == "kaon+") {
    return MCSource::c_KaonPlus;
  } else if (Name == "kaon0") {
    return MCSource::c_KaonZero;
  } else if (Name == "anti_kaon0") {
    return MCSource::c_AntiKaonZero;
  } else if (Name == "kaon0S") {
    return MCSource::c_KaonZeroS;
  } else if (Name == "kaon0L") {
    return MCSource::c_KaonZeroL;
  } else if (Name == "kaon-") {
    return MCSource::c_KaonMinus;

  } else if (Name == "lambda") {
    return MCSource::c_Lambda;
  } else if (Name == "anti_lambda") {
    return MCSource::c_AntiLambda;

  } else if (Name == "sigma+") {
    return MCSource::c_SigmaPlus;
  } else if (Name == "anti_sigma+") {
    return MCSource::c_AntiSigmaPlus;
  } else if (Name == "sigma0") {
    return MCSource::c_SigmaZero;
  } else if (Name == "anti_sigma0") {
    return MCSource::c_AntiSigmaZero;
  } else if (Name == "sigma-") {
    return MCSource::c_SigmaMinus;
  } else if (Name == "anti_sigma-") {
    return MCSource::c_AntiSigmaMinus;

  } else if (Name == "xi0") {
    return MCSource::c_XiZero;
  } else if (Name == "anti_xi0") {
    return MCSource::c_AntiXiZero;
  } else if (Name == "xi-") {
    return MCSource::c_XiMinus;
  } else if (Name == "anti_xi-") {
    return MCSource::c_AntiXiMinus;

  } else if (Name == "omega-") {
    return MCSource::c_OmegaMinus;
  } else if (Name == "anti_omega-") {
    return MCSource::c_AntiOmegaMinus;

  } else if (Name == "rho+") {
    return MCSource::c_RhoPlus;
  } else if (Name == "rho-") {
    return MCSource::c_RhoMinus;
  } else if (Name == "rho0") {
    return MCSource::c_RhoZero;

  } else if (Name.compare(0, 5, "delta") == 0) {
    string Last = Name.substr(Name.size()-1, 1);
    if (Last == "-") {
      return MCSource::c_DeltaMinus;
    } else if (Last == "0") {
      return MCSource::c_DeltaZero;
    } else if (Last == "+") {
      string OneBeforeLast = Name.substr(Name.size()-2, 1);
      if (OneBeforeLast == "+") {
        return MCSource::c_DeltaPlusPlus;
      } else {
        return MCSource::c_DeltaPlus;
      }
    } else {
      merr<<"Unknown delta particle: "<<Name<<show; 
      return 0;
    }

  } else if (Name.compare(0, 2, "N(") == 0) {
    string Last = Name.substr(Name.size()-1, 1);
    if (Last == "-") {
      return MCSource::c_AntiProton;
    } else if (Last == "0") {
      return MCSource::c_Neutron;
    } else if (Last == "+") {
      return MCSource::c_Proton;
    } else {
      merr<<"Unknown nucleon: "<<Name<<show; 
      return 0;
    }

  } else if (Name.compare(0, 5, "omega") == 0) {
    return MCSource::c_Omega;

  } else {
    // Check if we have an element
    //cout<<"Name: "<<Name<<endl;
    
    G4String Start;
    G4String Stop;
    size_t loc = Name.find("[", 0);
    if (loc != G4String::npos) {
      Start = Name.substr(0, loc);
      Stop = Name.substr(loc+1, Name.find("]", 0)-1);
    } else {
      Start = Name; 
    }

    G4String sZ;
    G4String sA;
    for (unsigned int i = 0; i < Start.size(); ++i) {
      if (isalpha(Start.c_str()[i])) {
        sZ += Start[i];
      } else {
        sA += Start[i];
      }
    }
    int Z = MDMaterial::ConvertNameToAtomicNumber(sZ);
    int A = atoi(sA);
    
    if (Z > 0 && Z < 120 && A > 0 && A < 1000) {
      //cout<<1000*Z+A<<endl;
      return 1000*Z+A;
    } else {
      merr<<"Unknown particle name: "<<Name<<show; 
      return 0;
    }
  }

  merr<<"Unknown particle name: "<<Name<<show; 
  return 0;
}


/******************************************************************************
 * Return the cosima particle type ID from the particle definition
 */
int MCSteppingAction::GetParticleType(G4ParticleDefinition* Definition)
{
  if (Definition == G4Gamma::Definition()) {
    return MCSource::c_Gamma;

  } else if (Definition ==  G4Electron::Definition()) {
    return MCSource::c_Electron;
  } else if (Definition == G4Positron::Definition()) {
    return MCSource::c_Positron;

  } else if (Definition == G4Proton::Definition()) {
    return MCSource::c_Proton;
  } else if (Definition == G4AntiProton::Definition()) {
    return MCSource::c_AntiProton;
  } else if (Definition == G4Neutron::Definition()) {
    return MCSource::c_Neutron;
  } else if (Definition == G4AntiNeutron::Definition()) {
    return MCSource::c_AntiNeutron;

  } else if (Definition == G4MuonPlus::Definition()) {
    return MCSource::c_MuonPlus;
  } else if (Definition == G4MuonMinus::Definition()) {
    return MCSource::c_MuonMinus;
  } else if (Definition == G4TauPlus::Definition()) {
    return MCSource::c_TauonPlus;
  } else if (Definition == G4TauMinus::Definition()) {
    return MCSource::c_TauonMinus;

  } else if (Definition == G4NeutrinoE::Definition()) {
    return MCSource::c_ElectronNeutrino;
  } else if (Definition == G4AntiNeutrinoE::Definition()) {
    return MCSource::c_AntiElectronNeutrino;
  } else if (Definition == G4NeutrinoMu::Definition()) {
    return MCSource::c_MuonNeutrino;
  } else if (Definition == G4AntiNeutrinoMu::Definition()) {
    return MCSource::c_AntiMuonNeutrino;
  } else if (Definition == G4NeutrinoTau::Definition()) {
    return MCSource::c_TauonNeutrino;
  } else if (Definition == G4AntiNeutrinoTau::Definition()) {
    return MCSource::c_AntiTauonNeutrino;

  } else if (Definition == G4Deuteron::Definition()) {
    return MCSource::c_Deuteron;
  } else if (Definition == G4Triton::Definition()) {
    return MCSource::c_Triton;
  } else if (Definition == G4He3::Definition()) {
    return MCSource::c_He3;
  } else if (Definition == G4Alpha::Definition()) {
    return MCSource::c_Alpha;

  } else if (Definition == G4GenericIon::Definition()) {
    return MCSource::c_GenericIon;

  } else if (Definition->GetParticleType() == "nucleus") { // has to come after all other nuclei such as alpha...
    G4Ions* Nucleus = dynamic_cast<G4Ions*>(Definition); 
    return 1000*Nucleus->GetAtomicNumber() + Nucleus->GetAtomicMass(); // + int(Nucleus->GetExcitationEnergy())*100000;

  } else if (Definition == G4PionPlus::Definition()) {
    return MCSource::c_PiPlus;
  } else if (Definition == G4PionMinus::Definition()) {
    return MCSource::c_PiMinus;
  } else if (Definition == G4PionZero::Definition()) {
    return MCSource::c_PiZero;
  } else if (Definition == G4Eta::Definition()) {
    return MCSource::c_Eta;
  } else if (Definition == G4EtaPrime::Definition()) {
    return MCSource::c_EtaPrime;

  } else if (Definition == G4KaonPlus::Definition()) {
    return MCSource::c_KaonPlus;
  } else if (Definition == G4KaonZero::Definition()) {
    return MCSource::c_KaonZero;
  } else if (Definition == G4AntiKaonZero::Definition()) {
    return MCSource::c_AntiKaonZero;
  } else if (Definition == G4KaonZeroShort::Definition()) {
    return MCSource::c_KaonZeroS;
  } else if (Definition == G4KaonZeroLong::Definition()) {
    return MCSource::c_KaonZeroL;
  } else if (Definition == G4KaonMinus::Definition()) {
    return MCSource::c_KaonMinus;

  } else if (Definition == G4Lambda::Definition()) {
    return MCSource::c_Lambda;
  } else if (Definition == G4AntiLambda::Definition()) {
    return MCSource::c_AntiLambda;

  } else if (Definition == G4SigmaPlus::Definition()) {
    return MCSource::c_SigmaPlus;
  } else if (Definition == G4AntiSigmaPlus::Definition()) {
    return MCSource::c_AntiSigmaPlus;
  } else if (Definition == G4SigmaZero::Definition()) {
    return MCSource::c_SigmaZero;
  } else if (Definition == G4AntiSigmaZero::Definition()) {
    return MCSource::c_AntiSigmaZero;
  } else if (Definition == G4SigmaMinus::Definition()) {
    return MCSource::c_SigmaMinus;
  } else if (Definition == G4AntiSigmaMinus::Definition()) {
    return MCSource::c_AntiSigmaMinus;

  } else if (Definition == G4XiZero::Definition()) {
    return MCSource::c_XiZero;
  } else if (Definition == G4AntiXiZero::Definition()) {
    return MCSource::c_AntiXiZero;
  } else if (Definition == G4XiMinus::Definition()) {
    return MCSource::c_XiMinus;
  } else if (Definition == G4AntiXiMinus::Definition()) {
    return MCSource::c_AntiXiMinus;

  } else if (Definition == G4OmegaMinus::Definition()) {
    return MCSource::c_OmegaMinus;
  } else if (Definition == G4AntiOmegaMinus::Definition()) {
    return MCSource::c_AntiOmegaMinus;

  } else if (Definition->GetParticleName() == "rho+") {
    return MCSource::c_RhoPlus;
  } else if (Definition->GetParticleName() == "rho-") {
    return MCSource::c_RhoMinus;
  } else if (Definition->GetParticleName() == "rho0") {
    return MCSource::c_RhoZero;

  } else if (Definition->GetParticleName().compare(0, 5, "delta") == 0) {
    G4String Name = Definition->GetParticleName();
    string Last = Name.substr(Name.size()-1, 1);
    if (Last == "-") {
      return MCSource::c_DeltaMinus;
    } else if (Last == "0") {
      return MCSource::c_DeltaZero;
    } else if (Last == "+") {
      string OneBeforeLast = Name.substr(Name.size()-2, 1);
      if (OneBeforeLast == "+") {
        return MCSource::c_DeltaPlusPlus;
      } else {
        return MCSource::c_DeltaPlus;
      }
    } else {
      merr<<"Unknown delta particle: "<<Name<<show; 
      return 0;
    }

  } else if (Definition->GetParticleName().compare(0, 2, "N(") == 0) {
    G4String Name = Definition->GetParticleName();
    string Last = Name.substr(Name.size()-1, 1);
    if (Last == "-") {
      return MCSource::c_AntiProton;
    } else if (Last == "0") {
      return MCSource::c_Neutron;
    } else if (Last == "+") {
      return MCSource::c_Proton;
    } else {
      merr<<"Unknown nucleon: "<<Name<<show; 
      return 0;
    }

  } else if (Definition->GetParticleName().compare(0, 5, "omega") == 0) {
    return MCSource::c_Omega;

  } else {
    merr<<"Unknown particle name: "<<Definition->GetParticleName()<<show; 
    return 0;
  }
}


/******************************************************************************
 * Returns the Geant4 particle definition from a MEGAlib particle ID
 */
G4ParticleDefinition* MCSteppingAction::GetParticleDefinition(int ID, double Excitation)
{
  G4ParticleDefinition* ParticleDefinition = nullptr;
  
  // Check for ions first
  if (ID > 1000) {
    
    G4IonTable* Table = G4IonTable::GetIonTable();
    int AtomicNumber = int(ID/1000);
    int AtomicMass = ID - int(ID/1000)*1000;
    
    
    if(Table->GetLifeTime(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::no_Float)!=-1001){ 
    ParticleDefinition = Table->GetIon(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::no_Float);}
    
    else if( Table->GetLifeTime(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_X)!=-1001)
    { ParticleDefinition = Table->GetIon(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_X); }
    
    else if( Table->GetLifeTime(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_Y)!=-1001)
    { ParticleDefinition = Table->GetIon(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_Y); }
    
    else if( Table->GetLifeTime(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_Z)!=-1001)
    { ParticleDefinition = Table->GetIon(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_Z); }
    
    else if( Table->GetLifeTime(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_U)!=-1001)
    { ParticleDefinition = Table->GetIon(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_U); }
    
    else if( Table->GetLifeTime(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_V)!=-1001)
    { ParticleDefinition = Table->GetIon(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_V); }
    
    else if( Table->GetLifeTime(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_W)!=-1001)
    { ParticleDefinition = Table->GetIon(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_W); }
    
    else if( Table->GetLifeTime(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_R)!=-1001)
    { ParticleDefinition = Table->GetIon(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_R); }
    
    else if( Table->GetLifeTime(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_S)!=-1001)
    { ParticleDefinition = Table->GetIon(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_S); }
    
    else if( Table->GetLifeTime(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_T)!=-1001)
    { ParticleDefinition = Table->GetIon(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_T); }
    
    else if( Table->GetLifeTime(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_T)!=-1001)
    { ParticleDefinition = Table->GetIon(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_T); }
    
    else if( Table->GetLifeTime(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_A)!=-1001)
    { ParticleDefinition = Table->GetIon(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_A); }
    
    else if( Table->GetLifeTime(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_B)!=-1001)
    { ParticleDefinition = Table->GetIon(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_B); }
    
    else if( Table->GetLifeTime(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_C)!=-1001)
    { ParticleDefinition = Table->GetIon(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_C); }
    
    else if( Table->GetLifeTime(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_D)!=-1001)
    { ParticleDefinition = Table->GetIon(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_D); }
    
    else if( Table->GetLifeTime(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_E)!=-1001)
    { ParticleDefinition = Table->GetIon(AtomicNumber, AtomicMass, Excitation,G4Ions::G4FloatLevelBase::plus_E); }
    
    
    else {
      merr<<"Particle type not yet implemented!"<<endl;
      ParticleDefinition = nullptr;
    } 
  } else if (ID == MCSource::c_Gamma) {
    ParticleDefinition = G4Gamma::Gamma();
  } else if (ID == MCSource::c_Positron) {
    ParticleDefinition = G4Positron::Positron();
  } else if (ID == MCSource::c_Electron) {
    ParticleDefinition = G4Electron::Electron();
  } else if (ID == MCSource::c_Proton) {
    ParticleDefinition = G4Proton::Proton();
  } else if (ID == MCSource::c_AntiProton) {
    ParticleDefinition = G4AntiProton::AntiProton();
  } else if (ID == MCSource::c_Neutron) {
    ParticleDefinition = G4Neutron::Neutron();
  } else if (ID == MCSource::c_AntiNeutron) {
    ParticleDefinition = G4AntiNeutron::AntiNeutron();
  } else if (ID == MCSource::c_MuonPlus) {
    ParticleDefinition = G4MuonPlus::MuonPlus();
  } else if (ID == MCSource::c_MuonMinus) {
    ParticleDefinition = G4MuonMinus::MuonMinus();
  } else if (ID == MCSource::c_TauonPlus) {
    ParticleDefinition = G4TauPlus::TauPlus();
  } else if (ID == MCSource::c_TauonMinus) {
    ParticleDefinition = G4TauMinus::TauMinus();
  } else if (ID == MCSource::c_ElectronNeutrino) {
    ParticleDefinition = G4NeutrinoE::NeutrinoE();
  } else if (ID == MCSource::c_AntiElectronNeutrino) {
    ParticleDefinition = G4AntiNeutrinoE::AntiNeutrinoE();
  } else if (ID == MCSource::c_MuonNeutrino) {
    ParticleDefinition = G4NeutrinoMu::NeutrinoMu();
  } else if (ID == MCSource::c_AntiMuonNeutrino) {
    ParticleDefinition = G4AntiNeutrinoMu::AntiNeutrinoMu();
  } else if (ID == MCSource::c_TauonNeutrino) {
    ParticleDefinition = G4NeutrinoTau::NeutrinoTau();
  } else if (ID == MCSource::c_AntiTauonNeutrino) {
    ParticleDefinition = G4AntiNeutrinoTau::AntiNeutrinoTau();
  } else if (ID == MCSource::c_Deuteron) {
    ParticleDefinition = G4Deuteron::Deuteron();
  } else if (ID == MCSource::c_Triton) {
    ParticleDefinition = G4Triton::Triton();
  } else if (ID == MCSource::c_He3) {
    ParticleDefinition = G4He3::He3();
  } else if (ID == MCSource::c_Alpha) {
    ParticleDefinition = G4Alpha::Alpha();
  } else if (ID == MCSource::c_GenericIon) {
    ParticleDefinition = G4GenericIon::GenericIon();
  } else if (ID == MCSource::c_PiPlus) {
    ParticleDefinition = G4PionPlus::PionPlus();
  } else if (ID == MCSource::c_PiZero) {
    ParticleDefinition = G4PionZero::PionZero();
  } else if (ID == MCSource::c_PiMinus) {
    ParticleDefinition = G4PionMinus::PionMinus();
  } else if (ID == MCSource::c_Eta) {
    ParticleDefinition = G4Eta::Eta();
  } else if (ID == MCSource::c_EtaPrime) {
    ParticleDefinition = G4EtaPrime::EtaPrime();
  } else if (ID == MCSource::c_KaonPlus) {
    ParticleDefinition = G4KaonPlus::KaonPlus();
  } else if (ID == MCSource::c_KaonZero) {
    ParticleDefinition = G4KaonZero::KaonZero();
  } else if (ID == MCSource::c_AntiKaonZero) {
    ParticleDefinition = G4AntiKaonZero::AntiKaonZero();
  } else if (ID == MCSource::c_KaonZeroS) {
    ParticleDefinition = G4KaonZeroShort::KaonZeroShort();
  } else if (ID == MCSource::c_KaonZeroL) {
    ParticleDefinition = G4KaonZeroLong::KaonZeroLong();
  } else if (ID == MCSource::c_KaonMinus) {
    ParticleDefinition = G4KaonMinus::KaonMinus();
  } else if (ID == MCSource::c_Lambda) {
    ParticleDefinition = G4Lambda::Lambda();
  } else if (ID == MCSource::c_AntiLambda) {
    ParticleDefinition = G4AntiLambda::AntiLambda();
  } else if (ID == MCSource::c_SigmaPlus) {
    ParticleDefinition = G4SigmaPlus::SigmaPlus();
  } else if (ID == MCSource::c_AntiSigmaPlus) {
    ParticleDefinition = G4AntiSigmaPlus::AntiSigmaPlus();
  } else if (ID == MCSource::c_SigmaZero) {
    ParticleDefinition = G4SigmaZero::SigmaZero();
  } else if (ID == MCSource::c_AntiSigmaZero) {
    ParticleDefinition = G4AntiSigmaZero::AntiSigmaZero();
  } else if (ID == MCSource::c_SigmaMinus) {
    ParticleDefinition = G4SigmaMinus::SigmaMinus();
  } else if (ID == MCSource::c_AntiSigmaMinus) {
    ParticleDefinition = G4AntiSigmaMinus::AntiSigmaMinus();
  } else if (ID == MCSource::c_XiZero) {
    ParticleDefinition = G4XiZero::XiZero();
  } else if (ID == MCSource::c_AntiXiZero) {
    ParticleDefinition = G4AntiXiZero::AntiXiZero();
  } else if (ID == MCSource::c_XiMinus) {
    ParticleDefinition = G4XiMinus::XiMinus();
  } else if (ID == MCSource::c_AntiXiMinus) {  
    ParticleDefinition = G4AntiXiMinus::AntiXiMinus();
  } else if (ID == MCSource::c_OmegaMinus) {
    ParticleDefinition = G4OmegaMinus::OmegaMinus();
  } else if (ID == MCSource::c_AntiOmegaMinus) {
    ParticleDefinition = G4AntiOmegaMinus::AntiOmegaMinus();
  } else {   
    ParticleDefinition = nullptr;
    merr<<"Particle type not yet implemented: "<<ID<<endl;
  }

  return ParticleDefinition;
}


/******************************************************************************
 * Return the Id of the detector, we are currently in
 */
int MCSteppingAction::GetDetectorId(const G4StepPoint* StepPoint)
{
  if (StepPoint->GetPhysicalVolume() != 0) {
    G4VSensitiveDetector* SensitiveDetector = 
      StepPoint->GetPhysicalVolume()->GetLogicalVolume()->GetSensitiveDetector();
    if (SensitiveDetector != 0) {
      return ((MCSD*) SensitiveDetector)->GetType();
    }
  } 

  return MCSD::c_Unknown;
}


/*
 * MCSteppingAction.cc: the end...
 ******************************************************************************/

