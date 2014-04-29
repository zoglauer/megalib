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

// Geant4:
#include "G4SteppingManager.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4TrackStatus.hh"
#include "G4VPhysicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4EventManager.hh"
#include "G4NuclearLevel.hh"
#include "G4NuclearLevelManager.hh"
#include "G4NuclearLevelStore.hh"
#include "G4Ions.hh"

// MEGAlib
#include "MStreams.h"
#include "MAssert.h"
#include "MDMaterial.h"
#include "MSimEvent.h"

// Standard lib:
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

  m_BlackAbsorbers = RunParameters.GetBlackAbsorbers();

  m_DecayMode = RunParameters.GetDecayMode();

  m_DetectorTimeConstant = RunParameters.GetDetectorTimeConstant();
  
  
  m_KnownProcess.push_back("polarLowEnCompt"); m_KnownProcessID.push_back(c_ProcessIDCompton);
  m_KnownProcess.push_back("PenCompton"); m_KnownProcessID.push_back(c_ProcessIDCompton);
  m_KnownProcess.push_back("LowEnCompton"); m_KnownProcessID.push_back(c_ProcessIDCompton);
  m_KnownProcess.push_back("compt"); m_KnownProcessID.push_back(c_ProcessIDCompton);

  m_KnownProcess.push_back("conv"); m_KnownProcessID.push_back(c_ProcessIDPair);
  m_KnownProcess.push_back("hPairProd"); m_KnownProcessID.push_back(c_ProcessIDPair);
  m_KnownProcess.push_back("PenConversion"); m_KnownProcessID.push_back(c_ProcessIDPair);
  m_KnownProcess.push_back("LowEnConversion"); m_KnownProcessID.push_back(c_ProcessIDPair);
  m_KnownProcess.push_back("LowEnPolarizConversion"); m_KnownProcessID.push_back(c_ProcessIDPair);

  m_KnownProcess.push_back("annihil"); m_KnownProcessID.push_back(c_ProcessIDAnnihilation);
  m_KnownProcess.push_back("PenAnnih"); m_KnownProcessID.push_back(c_ProcessIDAnnihilation);
  
  m_KnownProcess.push_back("eBrem"); m_KnownProcessID.push_back(c_ProcessIDBremsstrahlung);
  m_KnownProcess.push_back("hBrems"); m_KnownProcessID.push_back(c_ProcessIDBremsstrahlung);
  m_KnownProcess.push_back("PenelopeBrem"); m_KnownProcessID.push_back(c_ProcessIDBremsstrahlung);
  m_KnownProcess.push_back("LowEnBrem"); m_KnownProcessID.push_back(c_ProcessIDBremsstrahlung);
  
  m_KnownProcess.push_back("Rayl"); m_KnownProcessID.push_back(c_ProcessIDRayleigh);
  m_KnownProcess.push_back("PenRayleigh"); m_KnownProcessID.push_back(c_ProcessIDRayleigh);
  m_KnownProcess.push_back("LowEnRayleigh"); m_KnownProcessID.push_back(c_ProcessIDRayleigh);
  
  m_KnownProcess.push_back("LowEnPhotoElec"); m_KnownProcessID.push_back(c_ProcessIDPhoto);
  m_KnownProcess.push_back("PenPhotoElec"); m_KnownProcessID.push_back(c_ProcessIDPhoto);
  m_KnownProcess.push_back("phot"); m_KnownProcessID.push_back(c_ProcessIDPhoto);
  
  m_KnownProcess.push_back("hadElastic"); m_KnownProcessID.push_back(c_ProcessIDElasticScattering);
  
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
  m_KnownProcess.push_back("AntiLambdaInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("SigmaMinusInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("SigmaPlusInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("He3Inelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("alphaInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("PositronNuclear"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("ElectroNuclear"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  m_KnownProcess.push_back("ionInelastic"); m_KnownProcessID.push_back(c_ProcessIDInelasticScattering);
  
  m_KnownProcess.push_back("HadronCapture"); m_KnownProcessID.push_back(c_ProcessIDCapture);
  m_KnownProcess.push_back("nCapture"); m_KnownProcessID.push_back(c_ProcessIDCapture);
  m_KnownProcess.push_back("CHIPSNuclearCaptureAtRest"); m_KnownProcessID.push_back(c_ProcessIDCapture);
  m_KnownProcess.push_back("muMinusCaptureAtRest"); m_KnownProcessID.push_back(c_ProcessIDCapture);
  m_KnownProcess.push_back("hBertiniCaptureAtRest"); m_KnownProcessID.push_back(c_ProcessIDCapture);
  
  m_KnownProcess.push_back("Decay"); m_KnownProcessID.push_back(c_ProcessIDDecay);
  
  m_KnownProcess.push_back("RadioactiveDecay"); m_KnownProcessID.push_back(c_ProcessIDRadioactiveDecay);
  
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

  for (unsigned int i = 0; i < m_KnownProcess.size(); ++i) m_KnownProcessFrequency.push_back(0);
  m_KnownProcessCounter = 0;
  m_KnownProcessUpdateCounter = 1000;
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
  int GeneratedSecondaries = fpSteppingManager->GetSecondary()->size() - m_NSecondaries;
  m_NSecondaries = fpSteppingManager->GetSecondary()->size();

  MCEventAction* EventAction = (MCEventAction *) (G4EventManager::GetEventManager()->GetUserEventAction());
 
  double Time = Step->GetPostStepPoint()->GetGlobalTime()/second;

  // Quick sanity check to prevent some Geant4 hick-ups:
  if (isnan(Track->GetPosition().getX())) {
    merr<<"Geant4 hick-up: Detected NaN! Aborting track!"<<endl;
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
        if (GetParticleId(TrackA->GetDefinition()) > 1000) {
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

        for (int s = (int) fpSteppingManager->GetSecondary()->size() - GeneratedSecondaries; 
             s < (int) fpSteppingManager->GetSecondary()->size(); ++s) {
          
          m_InteractionId++;
          // The photon may deposit a small bit of energy locally
          // To aid the response generation, we add handle this energy as if it originated from the new electron track
          ((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);
            
          G4Track* TrackA = (*fpSteppingManager->GetSecondary())[s];
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
                             GetParticleId(TrackA->GetDefinition()),
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

      for (int s = (int) fpSteppingManager->GetSecondary()->size()-1; 
           s > (int) fpSteppingManager->GetSecondary()->size()-1 - 
             GeneratedSecondaries; --s) {

        m_InteractionId++;
        //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);
 
        G4Track* TrackA = (*fpSteppingManager->GetSecondary())[s];
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
                           GetParticleId(TrackA->GetDefinition()),
                           TrackA->GetMomentumDirection(),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           TrackA->GetKineticEnergy());
        
      }
    } else if (ProcessID == c_ProcessIDAnnihilation) {
      // Annihilation should generate at least two particles, e.g. two 511keV photons

      // Take care of secondaries below the threshold, 
      // for which no new track is generated:
      for (int s = (int) fpSteppingManager->GetSecondary()->size()-1; 
           s > (int) fpSteppingManager->GetSecondary()->size()-1 - 
             GeneratedSecondaries; --s) {

        m_InteractionId++;
        //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

        G4Track* TrackA = (*fpSteppingManager->GetSecondary())[s];
        TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));


        // Now write some data about the real interaction positions 
        // in order to compare with the measured ones:
        EventAction->AddIA("ANNI", 
                           m_InteractionId,
                           ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                           GetDetectorId(Step->GetPreStepPoint()),
                           Time,
                           TrackA->GetPosition(),
                           GetParticleId(Track->GetDefinition()),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           0.0,
                           GetParticleId(TrackA->GetDefinition()),
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
                           GetParticleId(Track->GetDefinition()),
                           Track->GetMomentumDirection(),
                           Track->GetPolarization(),
                           Track->GetKineticEnergy(),
                           0,
                           G4ThreeVector(0.0, 0.0, 0.0),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           0.0);

      } else {

        for (int s = (int) fpSteppingManager->GetSecondary()->size() - GeneratedSecondaries; 
             s < (int) fpSteppingManager->GetSecondary()->size(); ++s) {
          
          m_InteractionId++;
          //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);
            
          G4Track* TrackA = (*fpSteppingManager->GetSecondary())[s];
          TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));
          
          EventAction->AddIA("BREM", 
                             m_InteractionId,
                             ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                             GetDetectorId(Step->GetPreStepPoint()),
                             Time,
                             TrackA->GetPosition(),
                             GetParticleId(Track->GetDefinition()),
                             Track->GetMomentumDirection(),
                             Track->GetPolarization(),
                             Track->GetKineticEnergy(),
                             GetParticleId(TrackA->GetDefinition()),
                             TrackA->GetMomentumDirection(),
                             TrackA->GetPolarization(),
                             TrackA->GetKineticEnergy());
          
        }
      }

    } else if (ProcessID == c_ProcessIDRayleigh) {

      // Noo secondary should have been generated
      if (GeneratedSecondaries != 0) {
        merr<<"Rayleigh scattering generated a secondary!!!"<<show;
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
        //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

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

        for (int s = (int) fpSteppingManager->GetSecondary()->size() - GeneratedSecondaries; 
             s < (int) fpSteppingManager->GetSecondary()->size(); ++s) {
          
          m_InteractionId++;
          //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);
            
          G4Track* TrackA = (*fpSteppingManager->GetSecondary())[s];
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
                             GetParticleId(TrackA->GetDefinition()),
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
      for (int s = (int) fpSteppingManager->GetSecondary()->size()-1; 
           s > (int) fpSteppingManager->GetSecondary()->size()-1 - GeneratedSecondaries; --s) {

        m_InteractionId++;
        //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

        G4Track* TrackA = (*fpSteppingManager->GetSecondary())[s];
        TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));

        // Now write some data about the real interaction positions 
        // in order to compare with the measured ones:
        EventAction->AddIA("ELAS", 
                           m_InteractionId,
                           ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                           GetDetectorId(Step->GetPreStepPoint()),
                           Time,
                           Track->GetPosition(),
                           GetParticleId(Track->GetDefinition()),
                           Track->GetMomentumDirection(),
                           Track->GetPolarization(),
                           Track->GetKineticEnergy(),
                           //TrackA->GetDefinition()->GetExcitationEnergy(),
                           //((MString(Track->GetDefinition()->GetParticleName().c_str()).Contains("[") == true) ? dynamic_cast<G4Ions*>(TrackA->GetDefinition())->GetExcitationEnergy() : 0.0),
                           GetParticleId(TrackA->GetDefinition()),
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
                          GetParticleId(Track->GetDefinition()),
                          Track->GetMomentumDirection(),
                          Track->GetPolarization(),
                          Track->GetKineticEnergy(),
                          0,
                          G4ThreeVector(0.0, 0.0, 0.0),
                          G4ThreeVector(0.0, 0.0, 0.0),
                          0.0);
      } else {
        // Take care of all secondary particles which are generated
        for (int s = (int) fpSteppingManager->GetSecondary()->size()-1; 
           s > (int) fpSteppingManager->GetSecondary()->size()-1 - GeneratedSecondaries; --s) {

          m_InteractionId++;
          //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

          G4Track* TrackA = (*fpSteppingManager->GetSecondary())[s];
          TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));

          // Now write some data about the real interaction positions 
          // in order to compare with the measured ones:
          EventAction->AddIA("FISS", 
                            m_InteractionId,
                            ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                            GetDetectorId(Step->GetPreStepPoint()),
                            Time,
                            Track->GetPosition(),
                            GetParticleId(Track->GetDefinition()),
                            Track->GetMomentumDirection(),
                            Track->GetPolarization(),
                            Track->GetKineticEnergy(),
                            //TrackA->GetDefinition()->GetExcitationEnergy(),
                            //((MString(Track->GetDefinition()->GetParticleName().c_str()).Contains("[") == true) ? dynamic_cast<G4Ions*>(TrackA->GetDefinition())->GetExcitationEnergy() : 0.0),
                            GetParticleId(TrackA->GetDefinition()),
                            TrackA->GetMomentumDirection(),
                            TrackA->GetPolarization(),
                            TrackA->GetKineticEnergy());
      
        }
      }
    } else if (ProcessID == c_ProcessIDInelasticScattering) {

      // There always has to be a generated secondary, since we generate a new nucleus
      if (GeneratedSecondaries == 0) {
        merr<<"Inelastic scattering didn't generate secondaries! Either your thresholds are insane or a simulation error occurred!"<<endl;
        merr<<"Changed particle: "<<GetParticleId(Track->GetDefinition())<<show;
      }

      // Take care of all secondary particles which are generated
      for (int s = (int) fpSteppingManager->GetSecondary()->size()-1; 
           s > (int) fpSteppingManager->GetSecondary()->size()-1 - GeneratedSecondaries; --s) {

        m_InteractionId++;
        //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

        G4Track* TrackA = (*fpSteppingManager->GetSecondary())[s];
        TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));

        // Now write some data about the real interaction positions 
        // in order to compare with the measured ones:
        EventAction->AddIA("INEL", 
                           m_InteractionId,
                           ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                           GetDetectorId(Step->GetPreStepPoint()),
                           Time,
                           Track->GetPosition(),
                           GetParticleId(Track->GetDefinition()),
                           Track->GetMomentumDirection(),
                           Track->GetPolarization(),
                           Track->GetKineticEnergy(),
                           //TrackA->GetDefinition()->GetExcitationEnergy(),
                           //((MString(Track->GetDefinition()->GetParticleName().c_str()).Contains("[") == true) ? dynamic_cast<G4Ions*>(TrackA->GetDefinition())->GetExcitationEnergy() : 0.0),
                           GetParticleId(TrackA->GetDefinition()),
                           TrackA->GetMomentumDirection(),
                           TrackA->GetPolarization(),
                           TrackA->GetKineticEnergy());
      
      }
    } else if (ProcessID == c_ProcessIDCapture) {

      // There always has to be a generated secondary, since we generate a new nucleus
      if (GeneratedSecondaries == 0) {
        merr<<"HadronCapture/nCapture didn't generate secondaries! Either your thresholds are insane or a simulation error occurred!"<<show;
        merr<<"Changed particle: "<<GetParticleId(Track->GetDefinition())<<show;
      }

      // Take care of secondaries below the threshold, 
      // for which no new track is generated:
      for (int s = (int) fpSteppingManager->GetSecondary()->size()-1; 
           s > (int) fpSteppingManager->GetSecondary()->size()-1 - GeneratedSecondaries; --s) {

        m_InteractionId++;
        //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

        G4Track* TrackA = (*fpSteppingManager->GetSecondary())[s];
        TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));


        // Now write some data about the real interaction positions 
        // in order to compare with the measured ones:
        EventAction->AddIA("CAPT", 
                           m_InteractionId,
                           ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                           GetDetectorId(Step->GetPreStepPoint()),
                           Time,
                           TrackA->GetPosition(),
                           GetParticleId(Track->GetDefinition()),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           0.0,
                           GetParticleId(TrackA->GetDefinition()),
                           TrackA->GetMomentumDirection(),
                           TrackA->GetPolarization(),
                           TrackA->GetKineticEnergy());
        
      }
    } else if (ProcessID == c_ProcessIDDecay) {

      // There always has to be a generated secondary, since something decays into something else
      // "19" doesn't generate secondaries in Geant4... 
      if (GeneratedSecondaries == 0 && GetParticleId(Track->GetDefinition()) != 19) {
        merr<<"Decay didn't generate secondaries! Either your thresholds are insane or a simulation error occurred!"<<endl;
        merr<<"Changed particle: "<<GetParticleId(Track->GetDefinition())<<show;
      }

      // Take care of secondaries below the threshold, 
      // for which no new track is generated:
      for (int s = (int) fpSteppingManager->GetSecondary()->size()-1; 
           s > (int) fpSteppingManager->GetSecondary()->size()-1 - GeneratedSecondaries; --s) {

        m_InteractionId++;
        //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

        G4Track* TrackA = (*fpSteppingManager->GetSecondary())[s];
        TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));
      
        // Now write some data about the real interaction positions 
        // in order to compare with the measured ones:
        EventAction->AddIA("DECA", 
                           m_InteractionId,
                           ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                           GetDetectorId(Step->GetPreStepPoint()),
                           Time,
                           TrackA->GetPosition(),
                           GetParticleId(Track->GetDefinition()),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           G4ThreeVector(0.0, 0.0, 0.0),
                           0.0,
                           GetParticleId(TrackA->GetDefinition()),
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
              if (GetParticleId(Track->GetDefinition()) == m_InitialParticles[i]) {
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
            Keep = false;
            Store = false;
            FutureEvent = true;
            DoNotStart = false;
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
        
        if (Store == true) {
          //cout<<"Store"<<endl;
          // Since we store only when we do not keep, this code occurs only once per decay
 
          // Make sure the excitation energy is unique to avoid storing nucleii with similar excitation energies
          G4Ions* Nucleus = dynamic_cast<G4Ions*>(Track->GetDefinition());
          if (Nucleus->GetExcitationEnergy() > 1*keV) {
            //cout<<"Alignment > 1 keV"<<endl;
            G4NuclearLevelManager* M = G4NuclearLevelStore::GetInstance()->GetManager(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass());
            if (M->IsValid() == true) {
              const G4NuclearLevel* Level = M->NearestLevel(Nucleus->GetExcitationEnergy());
              if (Level != 0) {
                G4ParticleTable* ParticleTable = G4ParticleTable::GetParticleTable();
                Nucleus = dynamic_cast<G4Ions*>(ParticleTable->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), Level->Energy()));
              }
            }
          } else {
            //cout<<"Alignment < 1 keV"<<endl;
            G4ParticleTable* ParticleTable = G4ParticleTable::GetParticleTable();
            Nucleus = dynamic_cast<G4Ions*>(ParticleTable->GetIon(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass(), 0.0));
          }
          //cout<<"NE: "<<Nucleus->GetExcitationEnergy()/keV<<"keV"<<endl;

          G4TouchableHistory* Hist = (G4TouchableHistory*) (Step->GetPreStepPoint()->GetTouchable());
          MCRunManager::GetMCRunManager()->GetCurrentRun().AddIsotope(Nucleus, Hist);

          //mout<<"Storing isotope: "<<Nucleus->GetParticleName()<<endl;
        }
        
        if (Keep == true) {
          //cout<<"Keep"<<endl;
          // Take care of secondaries below the threshold, 
          // for which no new track is generated:
          
          //mout<<"Keeping isotope: "<<GetParticleId(Track->GetDefinition())<<endl;
          
          // There always has to be a generated secondary, since we generate a new nucleus
          //if (GeneratedSecondaries == 0) {
          //  merr<<"RadioactiveDecay didn't generate secondaries! Either your thresholds are insane or a simulation error occurred!"<<show;
          //}
          
          for (int s = (int) fpSteppingManager->GetSecondary()->size()-1; 
               s > (int) fpSteppingManager->GetSecondary()->size()-1 - 
                 GeneratedSecondaries; --s) {
            
            m_InteractionId++;
            //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);
            
            G4Track* TrackA = (*fpSteppingManager->GetSecondary())[s];
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
                               GetParticleId(Track->GetDefinition()),
                               G4ThreeVector(0.0, 0.0, 0.0),
                               G4ThreeVector(0.0, 0.0, 0.0),
                               //dynamic_cast<G4Ions*>(Track->GetDefinition())->GetExcitationEnergy(),
                               0.0,
                               GetParticleId(TrackA->GetDefinition()),
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
          Name.remove(Name.size()-3, 3);
          MCRunManager::GetMCRunManager()->GetCurrentRun().SkipOneEvent(Track->GetDefinition(), Name);
        }
        
        if (FutureEvent == true) { // && Track->GetDefinition()->GetParticleName() != "Ge73[13.3]") {
          //cout<<"Future: "<<Track->GetDefinition()->GetParticleName()<<" --> "<<Step->GetPostStepPoint()->GetGlobalTime()/second<<" sec "<<Step->GetPreStepPoint()->GetGlobalTime()/second<<" sec"<<endl;
 
          // Get the volume name, so that we know from which volume we have to skip an event later:
          G4TouchableHistory* Hist = (G4TouchableHistory*) (Step->GetPreStepPoint()->GetTouchable());
          G4LogicalVolume* V = Hist->GetVolume(0)->GetLogicalVolume();
          G4String Name = V->GetName();
          Name.remove(Name.size()-3, 3);

          double Time = Step->GetPostStepPoint()->GetGlobalTime();
//           if (Track->GetDefinition()->GetParticleName() == "Ge73[66.7]") {
//             Time = gRandom->Exp(0.499*s/log(2));
//             cout<<"New time: "<<Time<<endl;
//           }

          MCRunManager::GetMCRunManager()->GetCurrentRun().AddToBuildUpEventList(0.0,
                                                                                 Track->GetPosition(),
                                                                                 G4ThreeVector(0.0, 0.0, 0.0),
                                                                                 G4ThreeVector(0.0, 0.0, 0.0),
                                                                                 Time,
                                                                                 //step->GetPostStepPoint()->GetGlobalTime(),
                                                                                 Track->GetDefinition(),
                                                                                 Name);
          //cout<<"Global time: "<<Step->GetPostStepPoint()->GetGlobalTime()/second<<"sec"<<endl;
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
                           GetParticleId(Track->GetDefinition()),
                           Track->GetMomentumDirection(),
                           Track->GetPolarization(),
                           Track->GetKineticEnergy(),
                           0,
                           G4ThreeVector(0.0, 0.0, 0.0),
                           G4ThreeVector(0.0, 0.0, 0.0), 
                           0.0);
        
      } else {
        // Take care of secondary electrons generate by ionization
        for (int s = (int) fpSteppingManager->GetSecondary()->size() - GeneratedSecondaries; 
             s < (int) fpSteppingManager->GetSecondary()->size(); ++s) {
          
          m_InteractionId++;
          //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);

          G4Track* TrackA = (*fpSteppingManager->GetSecondary())[s];
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
                             GetParticleId(Track->GetDefinition()),
                             Track->GetMomentumDirection(),
                             Track->GetPolarization(),
                             Track->GetKineticEnergy(),
                             GetParticleId(TrackA->GetDefinition()),
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
                             GetParticleId(Track->GetDefinition()),
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
        for (int s = (int) fpSteppingManager->GetSecondary()->size() - GeneratedSecondaries; 
             s < (int) fpSteppingManager->GetSecondary()->size(); ++s) {
          
          m_InteractionId++;
          //((MCTrackInformation*) Track->GetUserInformation())->SetId(m_InteractionId);
          
          G4Track* TrackA = (*fpSteppingManager->GetSecondary())[s];
          TrackA->SetUserInformation(new MCTrackInformation(m_InteractionId, m_InteractionId));
          
          // Now write some data about the real interaction positions 
          // in order to compare with the measured ones:
          EventAction->AddIA("IONI",
                             m_InteractionId,
                             ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                             GetDetectorId(Step->GetPreStepPoint()),
                             Time,
                             TrackA->GetPosition(),
                             GetParticleId(Track->GetDefinition()),
                             Track->GetMomentumDirection(),
                             Track->GetPolarization(),
                             Track->GetKineticEnergy(),
                             GetParticleId(TrackA->GetDefinition()),
                             TrackA->GetMomentumDirection(),
                             TrackA->GetPolarization(), 
                             TrackA->GetKineticEnergy());
        }
      }
    } else {
      if (ProcessID == c_ProcessIDUncovered) {
        merr<<"Uncovered process: "<<ProcessName<<" - please inform the lead developers of this problem."<<endl
            <<"-> Your simulations are ok - you only might miss an IA line in the sim file."<<show;
      }

  
      // For speed ups:
      // We have a FIFO stack here, thus if we start from the end we can stop the search 
      // if we have found the first one with an entry
      if (fpSteppingManager->GetSecondary()->size() > 0) {
        for (unsigned int s = fpSteppingManager->GetSecondary()->size()-1; 
            s < fpSteppingManager->GetSecondary()->size(); --s) { 
          if (fpSteppingManager->GetSecondary()->at(s)->GetUserInformation() != 0) break;
 
          // Set the additional Track info about the ID's
          MCTrackInformation* Info = 
            new MCTrackInformation(((MCTrackInformation*) Track->
                                    GetUserInformation())->GetId(), 
                                  ((MCTrackInformation*) Track->
                                    GetUserInformation())->GetOriginId());
          fpSteppingManager->GetSecondary()->at(s)->SetUserInformation(Info);
        }
      }
		} // All processes
  } 

  // Check if the particle escapes the world volume:
  if (Track->GetNextVolume() == 0) {

    if (m_StoreSimulationInfo == true) {

      m_InteractionId++;

      EventAction->AddIA("ESCP",
                         m_InteractionId,
                         ((MCTrackInformation*) Track->GetUserInformation())->GetOriginId(),
                         GetDetectorId(Step->GetPreStepPoint()),
                         Time,
                         Track->GetPosition(),
                         GetParticleId(Track->GetDefinition()),
                         Track->GetMomentumDirection(),
                         Track->GetPolarization(),
                         Track->GetKineticEnergy(),
                         0,
                         G4ThreeVector(0.0, 0.0, 0.0),
                         G4ThreeVector(0.0, 0.0, 0.0), 
                         0.0);
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
        
        for (int h = 0; h < Step->GetPreStepPoint()->GetTouchable()->GetHistoryDepth(); ++h) {
          if (Step->GetPreStepPoint()->GetTouchable()->GetVolume(h)->GetName().c_str() == m_WatchedVolumes[w]) {
            FoundInPre = true;
            break;
          }
        }
        
        for (int h = 0; h < Step->GetPostStepPoint()->GetTouchable()->GetHistoryDepth(); ++h) {
          if (Step->GetPostStepPoint()->GetTouchable()->GetVolume(h)->GetName().c_str() == m_WatchedVolumes[w]) {
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
                             GetParticleId(Track->GetDefinition()),
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
                             GetParticleId(Track->GetDefinition()),
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
                             GetParticleId(Track->GetDefinition()),
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
      EventAction->AddDepositPassiveMaterial(Step->GetTotalEnergyDeposit(), 
                                             Track->GetVolume()->GetLogicalVolume()->GetMaterial()->GetName());
    }
  }

  return;
}


/******************************************************************************
 * Return the Process ID
 */
struct CompareDesc { 
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
  
  return c_ProcessIDUncovered;
}
  

/******************************************************************************
 * If during the step a Compton or pair process happend in the primary track,
 * the process information (positions, direction, energies) are transmitted 
 * to the event action class, for later file output
 */
int MCSteppingAction::GetParticleId(G4String Name)
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
    G4String::size_type loc = Name.find("[", 0);
    if (loc != G4String::npos) {
      G4String Start = Name.substr(0, loc);
      G4String Stop = Name.substr(loc+1, Name.find("]", 0)-1);
      //double Excitation = atof(Stop);
      //if (Excitation > 0) {
      //  cout<<"Excitation: "<<Excitation<<" ("<<Name<<")"<<endl;
      //}
      // Seperate text from number, Z from A
      G4String sZ = "";
      G4String sA = "";
      for (unsigned int i = 0; i < Start.size(); ++i) {
        if (isalpha(Start.c_str()[i])) {
          sZ += Start[i];
        } else {
          sA += Start[i];
        }
      }
      int Z = MDMaterial::ConvertZToNumber(sZ);
      int A = atoi(sA);
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
 * If during the step a Compton or pair process happend in the primary track,
 * the process information (positions, direction, energies) are transmitted 
 * to the event action class, for later file output
 */
int MCSteppingAction::GetParticleId(G4ParticleDefinition* Definition)
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

