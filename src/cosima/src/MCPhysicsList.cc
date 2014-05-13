/*
 * MCPhysicsList.cxx
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
#include "MCPhysicsList.hh"
#include "MCPhysicsListParticles.hh"

// Geant4:
#include "QGSP_BIC_HP.hh"
#include "QGSP_BERT_HP.hh"
#include "FTFP_BERT_HP.hh"

#include "G4EmExtraPhysics.hh"
#include "G4HadronElasticPhysicsHP.hh"
#include "G4StoppingPhysics.hh"
#include "G4IonPhysics.hh"
#include "G4IonBinaryCascadePhysics.hh"
#include "G4NeutronTrackingCut.hh"

#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "G4EmLivermorePhysics.hh"
#include "G4EmLivermorePolarizedPhysics.hh"
#include "G4EmPenelopePhysics.hh"
#include "G4EmStandardPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4VProcess.hh"
#include "G4VEmProcess.hh"
#include "G4VEnergyLossProcess.hh"
#include "G4ProcessVector.hh"
#include "G4ProcessManager.hh"
#include "G4LivermorePolarizedPhotoElectricModel.hh"
#include "G4LivermorePhotoElectricModel.hh"
#include "G4PenelopePhotoElectricModel.hh"
#include "G4LivermoreIonisationModel.hh"
#include "G4PenelopeIonisationModel.hh"
#include "G4PenelopeComptonModel.hh"

// MEGAlib:
#include "MStreams.h"


/******************************************************************************/

const int MCPhysicsList::c_EMNone = 0; 
const int MCPhysicsList::c_EMStandard = 1; 
const int MCPhysicsList::c_EMLivermore = 2; 
const int MCPhysicsList::c_EMLivermorePolarized = 3; 
const int MCPhysicsList::c_EMLivermoreG4LECS = 4; 
const int MCPhysicsList::c_EMPenelope = 5; 
const int MCPhysicsList::c_EMMin = 0; 
const int MCPhysicsList::c_EMMax = 5; 

const int MCPhysicsList::c_HDNone = 0; 
const int MCPhysicsList::c_HDQGSP_BIC_HP = 1; 
const int MCPhysicsList::c_HDQGSP_BERT_HP = 2; 
const int MCPhysicsList::c_HDFTFP_BERT_HP = 3; 
const int MCPhysicsList::c_HDMin = 0; 
const int MCPhysicsList::c_HDMax = 3; 


/******************************************************************************
 * Default constructor: Does nothing
 */
MCPhysicsList::MCPhysicsList(MCParameterFile& RunParameters) : G4VModularPhysicsList(),
  m_PhysicsListEM(RunParameters.GetPhysicsListEM()),
  m_PhysicsListHD(RunParameters.GetPhysicsListHD()),
  m_Regions(RunParameters.GetRegionList())
{
  // The default cut
  defaultCutValue = RunParameters.GetDefaultRangeCut();

  // Register the processes:
  Register();
}


/******************************************************************************
 * Default destructor: Does nothing
 */
MCPhysicsList::~MCPhysicsList()
{
  // Intentionally left blank
}

/******************************************************************************
 * Register the selcted processes
 */
void MCPhysicsList::Register()
{
  // Always register all particles
  RegisterPhysics(new MCPhysicsListParticles());

  // Take care of selected EM physics
  if (m_PhysicsListEM == c_EMNone) {
    // None
  } else if (m_PhysicsListEM == c_EMStandard) {
    RegisterPhysics(new G4EmStandardPhysics());
  } else if (m_PhysicsListEM == c_EMPenelope) {
    RegisterPhysics(new G4EmPenelopePhysics());
  } else if (m_PhysicsListEM == c_EMLivermore) {
    RegisterPhysics(new G4EmLivermorePhysics());
  } else if (m_PhysicsListEM == c_EMLivermorePolarized) {
    RegisterPhysics(new G4EmLivermorePolarizedPhysics());
  } else {
    mout<<"Unknown EM physics list ID: "<<m_PhysicsListEM<<endl;
    mout<<"Using Penelope"<<endl;
    RegisterPhysics(new G4EmPenelopePhysics());
  }
  
  
  // Take care of selected hadron physics
  G4VModularPhysicsList* HDPhysics = 0;
  if (m_PhysicsListHD == c_HDNone) {
    // None
  } else if (m_PhysicsListHD == c_HDQGSP_BIC_HP) {
    HDPhysics = new QGSP_BIC_HP();
  } else if (m_PhysicsListHD == c_HDQGSP_BERT_HP) {
    HDPhysics = new QGSP_BERT_HP();
  } else if (m_PhysicsListHD == c_HDFTFP_BERT_HP) {
    HDPhysics = new FTFP_BERT_HP();
  } else {
    mout<<"Unknown HD physics list ID: "<<m_PhysicsListHD<<endl;
    mout<<"Using QGSP-BIC-HP"<<endl;
    HDPhysics = new QGSP_BIC_HP();
  }
  if (HDPhysics != 0) {
    for (int i = 0; ; ++i) {
      G4VPhysicsConstructor* PC = const_cast<G4VPhysicsConstructor*>(HDPhysics->GetPhysics(i));
      if (PC == 0) break;
      
      bool Ignore = false;
      for (unsigned int p = 0; ; ++p) {
        if (GetPhysics(p) == 0) break;
        if (GetPhysics(p)->GetPhysicsType() == 0) continue;
        if (PC->GetPhysicsType() == 0) continue;
        if (GetPhysics(p)->GetPhysicsType() == PC->GetPhysicsType()) {
          cout<<"Not registering "<<PC->GetPhysicsName()<<" since we already have "<<GetPhysics(p)->GetPhysicsName()<<endl;
          Ignore = true;
          break;
        }
      }
      if (Ignore == true) continue;
      
      RegisterPhysics(PC);
    }
  }  

  // We always register all sorts of decays:
  RegisterPhysics(new G4RadioactiveDecayPhysics());
  
  // Check what we got:
  cout<<"Chosen physics: "<<endl;
  for (unsigned int p = 0; ; ++p) {
    if (GetPhysics(p) == 0) {
      break;
    }
    cout<<GetPhysics(p)->GetPhysicsName()<<"  "<<endl;
  }
  cout<<endl;
}


/******************************************************************************
 * Construct the processes - overwritten to post-modify some processes
 */
void MCPhysicsList::ConstructProcess()
{
  G4VModularPhysicsList::ConstructProcess();
  
  /* unnecessary as of Geant4 9.6
  // Do some additional modifications to the default lists:
  bool PhotoElectricAugerFound = false;
  bool ElectronIonizationAugerFound = false;
  bool ComptonAugerFound = false;
  
  theParticleIterator->reset();
  while ((*theParticleIterator)() == true) {
    G4ParticleDefinition* particle = theParticleIterator->value();
    G4ProcessManager* pmanager = particle->GetProcessManager();
    G4ProcessVector* pvector = pmanager->GetProcessList();
    //cout<<particle->GetParticleName()<<endl;
    //cout<<"Gamma processes: "<<pvector->size()<<endl;
    for (G4int p = 0; p < pvector->size(); ++p) {
      G4VProcess* P = (*pvector)[p];
      //cout<<P->GetProcessName()<<endl;
      if (dynamic_cast<G4VEmProcess*>(P) != 0) {
        G4VEmProcess* Em = dynamic_cast<G4VEmProcess*>(P);
        //cout<<"Past EM: "<<int(Em->GetModelByIndex(0))<<endl;
        int m = 0;
        G4VEmModel* Model = 0;
        while ((Model = Em->GetModelByIndex(m++)) != 0) {
          //cout<<"  "<<Model->GetName()<<endl;
        
          // Photo electric effect
          if (Model->GetName() == "LivermorePolarizedPhotoElectric") {
            //dynamic_cast<G4LivermorePolarizedPhotoElectricModel*>(Model)->ActivateAuger(true);
            cout<<"Activating Auger effect in "<<Model->GetName()<<endl;
            PhotoElectricAugerFound = true;
          } else if (Model->GetName() == "LivermorePhElectric") {
            //dynamic_cast<G4LivermorePhotoElectricModel*>(Model)->ActivateAuger(true);
            //cout<<"Activating Auger effect in "<<Model->GetName()<<endl;
            PhotoElectricAugerFound = true;
          } else if (Model->GetName() == "PenPhotoElec") {
            //dynamic_cast<G4PenelopePhotoElectricModel*>(Model)->ActivateAuger(true);
            //cout<<"Activating Auger effect in "<<Model->GetName()<<endl;
            PhotoElectricAugerFound = true;
          } else if (Model->GetName() == "phot") {
            PhotoElectricAugerFound = true; // No Auger here, so set it to true          
          }
         
          // Compton: 
          if (Model->GetName() == "PenCompton") {
            //dynamic_cast<G4PenelopeComptonModel*>(Model)->ActivateAuger(true);
            //cout<<"Activating Auger effect in "<<Model->GetName()<<endl;
            ComptonAugerFound = true;
          } else if (Model->GetName() == "LivermorePolarizedCompton" || Model->GetName() == "LivermoreCompton") {
            cout<<"Model "<<Model->GetName()<<" does not provide Auger effect --- check back in future Geant4 versions..."<<endl;
            ComptonAugerFound = true; // No Auger here, so set it to true          
         } else if (Model->GetName() == "compt") {
            ComptonAugerFound = true; // No Auger here, so set it to true          
          }
        }
      } else if (dynamic_cast<G4VEnergyLossProcess*>(P) != 0) {
        G4VEnergyLossProcess* Em = dynamic_cast<G4VEnergyLossProcess*>(P);
        int m = 0;
        G4VEmModel* Model = 0;
        while ((Model = Em->GetModelByIndex(m++)) != 0) {
          //cout<<"  "<<Model->GetName()<<endl;
         
          // Ionization:
          if (Model->GetName() == "LowEnergyIoni") {
            //dynamic_cast<G4LivermoreIonisationModel*>(Model)->ActivateAuger(true);
            //cout<<"Activating Auger effect in "<<Model->GetName()<<endl;
            ElectronIonizationAugerFound = true;
          } else if (Model->GetName() == "PenelopeIoni") {
            //dynamic_cast<G4PenelopeIonisationModel*>(Model)->ActivateAuger(true);
            cout<<"Activating Auger effect in "<<Model->GetName()<<endl;
            ElectronIonizationAugerFound = true;
          } else if (Model->GetName() == "eIoni") {
            ElectronIonizationAugerFound = true; // No Auger here, so set it to true          
          }
         
        }
      }
    }
  }
  
  if (m_PhysicsListEM != c_EMStandard) { // There are no models (because std is default) and no Auger in Standard
    if (PhotoElectricAugerFound == false) {
      merr<<"Unable to activate Auger effect for photo effect (this probably means that the processes got renamed in a Geant4 update...)"<<endl;
    } 
    if (ElectronIonizationAugerFound == false) {
      merr<<"Unable to activate Auger effect for ionization (this probably means that the processes got renamed in a Geant4 update...)"<<endl;
    }
    if (ComptonAugerFound == false) {
      merr<<"Unable to activate Auger effect for Compton scattering (this probably means that the processes got renamed in a Geant4 update...)"<<endl;
    }
  }
  */
}

  
/******************************************************************************
 * Only the default cuts are implemented right now
 */
void MCPhysicsList::SetCuts()
{
  // Start with default cuts for all particles
  SetCutsWithDefault();

  // The set the cuts for specific regions
  for (unsigned int i = 0; i < m_Regions.size(); ++i) {
    G4Region* Region = G4RegionStore::GetInstance()->GetRegion(m_Regions[i].GetName().Data());
    G4ProductionCuts* Cuts = new G4ProductionCuts;
    Cuts->SetProductionCut(m_Regions[i].GetRangeCut());
    Region->SetProductionCuts(Cuts);
  }

//  	// The default cut is 1% of the thickness of one strip
// 	G4double StandardCut = 0.005*mm;
//   SetCutValue(StandardCut, "gamma");
//   SetCutValue(StandardCut, "e+");

//   // Since we always need to generate secondaries during Compton-effect,
//   // we have to set a rather low value for electrons
//   // 0.000005*mm corresponds to a cut of ~0.15 keV in Silicon
//   // 0.00001*mm corresponds to a cut of ~0.42 keV in Silicon
//   // 0.00002*mm corresponds to a cut of ~0.8 keV in Silicon
//   SetCutValue(0.00002*mm, "e-");

  if (verboseLevel > 0) DumpCutValuesTable();
}


/*
 * MCPhysicsList.cc: the end...
 ******************************************************************************/
