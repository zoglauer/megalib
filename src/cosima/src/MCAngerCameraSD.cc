/*
 * MCAngerCameraSD.cxx
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
#include "MCAngerCameraSD.hh"
#include "MCAngerCameraHit.hh"
#include "MCDetectorConstruction.hh"
#include "MCTrackInformation.hh"
#include "MCEventAction.hh"

// MEGAlib:
#include "MAssert.h"
#include "MStreams.h"

// Geant4:
#include "G4SystemOfUnits.hh"
#include "G4VPhysicalVolume.hh"
#include "G4Step.hh"
#include "G4VTouchable.hh"
#include "G4TouchableHistory.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "G4ThreeVector.hh"
#include "G4EventManager.hh"


/******************************************************************************/



/******************************************************************************
 * Set name, type and collection 
 */
MCAngerCameraSD::MCAngerCameraSD(G4String Name) : 
  MCSD(Name), m_HitCollection(0), m_Is3D(false)
{
  m_Type = c_AngerCamera;
  collectionName.insert(Name);
}


/******************************************************************************
 * Default destructor: Empty
 */
MCAngerCameraSD::~MCAngerCameraSD()
{
  // Intentionally left blank 
}


/******************************************************************************
 * Create hit collection if not yet done
 */
void MCAngerCameraSD::Initialize(G4HCofThisEvent* HCE)
{
  if (HCE == 0) {
    massert("HCE is zero!");
  }

  if (MCAngerCameraSD::m_HitCollection == 0) {
    MCAngerCameraSD::m_HitCollection = 
      new MCAngerCameraHitsCollection(SensitiveDetectorName, collectionName[0]);
  }
}


/******************************************************************************
 * Process the hit: Don't do anything
 * This quirk is necessay since in the Geant4 SteppingManager the hits are 
 * processed before the UserSteppingAction is called, and for storing the 
 * correct IA information, we must first update the track information
 */
G4bool MCAngerCameraSD::ProcessHits(G4Step*, G4TouchableHistory*)
{ 
  // We are not doing anything!!!

  return true;
}


/******************************************************************************
 * Process the hit: Center positions, generate a hit and 
 * add it to the hit collection
 */
G4bool MCAngerCameraSD::PostProcessHits(const G4Step* Step)
{ 
  G4double Energy = Step->GetTotalEnergyDeposit();
  if (Energy == 0.0) return false;
  
  if (m_IsNeverTriggering == true) {
    dynamic_cast<MCEventAction *>(G4EventManager::GetEventManager()->GetUserEventAction())->AddEnergyLoss(Energy);
  }
  
  G4ThreeVector Position;
  switch (Step->GetPostStepPoint()->GetStepStatus()) {
  case fPostStepDoItProc:
  case fGeomBoundary:
    Position = Step->GetPostStepPoint()->GetPosition();
    break;
  case fAlongStepDoItProc:
    Position = 0.5*(Step->GetPreStepPoint()->GetPosition() + 
                    Step->GetPostStepPoint()->GetPosition());
    break;
  default:
    Position = Step->GetPreStepPoint()->GetPosition();
    break;
  }
  
  G4String DetectorName;
  G4ThreeVector DetectorPosition;
  
  // Let's rotate into the correct position:
  G4TouchableHistory* Hist = (G4TouchableHistory*) (Step->GetPreStepPoint()->GetTouchable());
  G4VPhysicalVolume* Vol = 0;
  for (int v =  Hist->GetHistoryDepth()-1; v >= 0; v--) {
    Vol = Hist->GetVolume(v);

    // Translate and rotate the position into sensitive detector coordinates.
    Position += Vol->GetFrameTranslation();
    if (Vol->GetFrameRotation() != 0) {
      Position = (*Vol->GetFrameRotation()) * Position;
    }

    if (Vol->GetLogicalVolume()->GetName() == m_DetectorVolumeName) {
      DetectorPosition = Position;
      DetectorName = Vol->GetName();
    }
  }

  // Apply any energy/charge/light loss
  if (m_UseEnergyLoss == true) {
    Energy *= m_EnergyLoss.Evaluate(Position.getX()/cm, Position.getY()/cm, Position.getZ()/cm);
  }
  if (Energy == 0.0) {
    mout<<"MCAngerCameraSD: Info: Energy loss map resulted in zero energy: "<<m_EnergyLoss.Evaluate(Position.getX()/cm, Position.getY()/cm, Position.getZ()/cm)<<":"<<Position<<endl;    
    return false;
  }


  // Now center the positions:
  if (m_DiscretizeHits == true) {
    if (m_Is3D == false) {
      Position.setZ(0.0);
    }
  }

  // Rotate back into the original system:
  for (int v = 0; v < Hist->GetHistoryDepth(); v++) {
    Vol = Hist->GetVolume(v);

    // Translate and rotate the position into sensitive detector coordinates.
    if (Vol->GetFrameRotation() != 0) {
      Position = (*Vol->GetObjectRotation()) * Position;
    }
    Position += Vol->GetObjectTranslation();
  }

  // Now create a hit:
  MCAngerCameraHit* H = new MCAngerCameraHit();
  H->SetEnergy(Energy);
  H->SetDetectorName(DetectorName);
  H->AddOrigin(((MCTrackInformation*) (Step->GetTrack()->GetUserInformation()))->GetId());
  for (int v =  Hist->GetHistoryDepth()-1; v >= 0; v--) {
    H->AddVolumeHistory(Hist->GetVolume(v)->GetName());
  }  
  H->SetPosition(Position);
  if (m_HasTimeResolution == true) {
    H->SetTime(Step->GetTrack()->GetGlobalTime());
  }


  // Check if there is already a hit in the strips and this layer:
  bool Added = false;
  if (m_DiscretizeHits == true) {
    for (G4int h = 0; h < MCAngerCameraSD::m_HitCollection->entries(); h++) {
      if (*(*MCAngerCameraSD::m_HitCollection)[h] == *H) {
        *(*MCAngerCameraSD::m_HitCollection)[h] += *H;
        Added = true;
        delete H;
        H = 0;
        break;
      }
    }
  }

  // Otherwise add as a new hit:
  if (H != 0) {
    MCAngerCameraSD::m_HitCollection->insert(H);
    Added = true;
  }  

  // This info makes sure the energy deposit is not added to the passive matrerial list
  if (Added == true) {
    // Set info that this hit has been added/digitized
    MCTrackInformation* TrackInfo = 
      (MCTrackInformation*) (Step->GetTrack()->GetUserInformation());
    TrackInfo->SetDigitized(true);
  }

  return true;
}


/******************************************************************************
 * Transfer hits to Geant
 */
void MCAngerCameraSD::EndOfEvent(G4HCofThisEvent* HCE)
{
  if (MCAngerCameraSD::m_HitCollection != 0) {
    int HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    HCE->AddHitsCollection(HCID, m_HitCollection);

    MCAngerCameraSD::m_HitCollection = 0;
    // The collection is automatically deleted by G4HCofThisEvent
  }
}


/*
 * MCAngerCameraSD.cc: the end...
 ******************************************************************************/










