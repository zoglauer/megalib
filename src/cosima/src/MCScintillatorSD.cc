/*
 * MCScintillatorSD.cxx
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
#include "MCScintillatorSD.hh"
#include "MCScintillatorHit.hh"
#include "MCDetectorConstruction.hh"
#include "MCTrackInformation.hh"
#include "MCEventAction.hh"

// MEGAlib:
#include "MAssert.h"

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


/******************************************************************************
 * Set name, type and collection 
 */
MCScintillatorSD::MCScintillatorSD(G4String Name) : 
  MCSD(Name), m_HitCollection(0)
{
  m_Type = c_Scintillator;
  m_HitCollection = 0;
  collectionName.insert(Name);
}


/******************************************************************************
 * Default destructor: Empty
 */
MCScintillatorSD::~MCScintillatorSD()
{
  // Intentionally left blank 
}


/******************************************************************************
 * Create hit collection if not yet done
 */
void MCScintillatorSD::Initialize(G4HCofThisEvent* HCE)
{
  if (HCE == 0) {
    massert("HCE is zero!");
  }

  m_HitCollection = 
    new MCScintillatorHitsCollection(SensitiveDetectorName, collectionName[0]);
}


/******************************************************************************
 * For hit centering: add the unique position to which it will be centered
 */  
void MCScintillatorSD::SetUniquePosition(string Name, G4ThreeVector Position)
{
  m_UniquePositions[Name] = Position;
}


/******************************************************************************
 * Process the hit: Don't do anything
 * This quirk is necessay since in the Geant4 SteppingManager the hits are 
 * processed before the UserSteppingAction is called, and for storing the 
 * correct IA information, we must first update the track information
 */
G4bool MCScintillatorSD::ProcessHits(G4Step*, G4TouchableHistory*)
{ 
  // We are not doing anything!!!

  return true;
}


/******************************************************************************
 * Process the hit: Center positions, generate a hit and 
 * add it to the hit collection
 */
G4bool MCScintillatorSD::PostProcessHits(const G4Step* Step)
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

  MString LogVolume;
  MString HitVolume;
  G4ThreeVector HitPosition;
  
  // Let's rotate into the correct position:
  G4TouchableHistory* Hist = 
    (G4TouchableHistory*) (Step->GetPreStepPoint()->GetTouchable());

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

    HitPosition = m_UniquePositions[Vol->GetLogicalVolume()->GetName()];
    LogVolume = Vol->GetLogicalVolume()->GetName().c_str();
  }

  // Apply any energy/charge/light loss
  if (m_UseEnergyLoss == true) {
    Energy *= m_EnergyLoss.Evaluate(Position.getX()/cm, Position.getY()/cm, Position.getZ()/cm);
  }
  if (Energy <= 0.0) {
    mout<<"MCScintillatorSD: Info: Energy loss map resulted in zero or negative energy: "<<m_EnergyLoss.Evaluate(Position.getX()/cm, Position.getY()/cm, Position.getZ()/cm)<<":"<<Position<<endl;    
    return false;
  }

  // Now digitize the position - center in the volume and add the unique position (which should be relative to the center)
  // Even if the hits are now in different volumes, additonal hits will always be added to the first one,
  // assuring that not multiple hits are generated 
  if (m_DiscretizeHits == true) {
    Position.setX(0.0);
    Position.setY(0.0);
    Position.setZ(0.0);
    Position += HitPosition;
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
  MCScintillatorHit* H = new MCScintillatorHit();
  H->SetEnergy(Energy);
  H->SetADCCounts(Energy/keV);
  H->SetPosition(Position);
  H->SetDetectorName(DetectorName);
  H->AddOrigin(((MCTrackInformation*) (Step->GetTrack()->GetUserInformation()))->GetId());
  if (m_HasTimeResolution == true) {
    H->SetTime(Step->GetTrack()->GetGlobalTime());
  }


  // We store the history starting with the common volume!
  for (int v = Hist->GetHistoryDepth()-1; v >= 0; v--) {
    H->AddVolumeHistory(Hist->GetVolume(v)->GetName());
    if (Hist->GetVolume(v)->GetLogicalVolume()->GetName() == m_CommonVolumeName) break;
  }  


  // Check if there is already a hit in this scintillator:
  bool Added = false;
  if (m_DiscretizeHits == true) {
    for (size_t h = 0; h < m_HitCollection->entries(); h++) {
      if (*(*m_HitCollection)[h] == *H) {
        *(*m_HitCollection)[h] += *H;
        Added = true;
        delete H;
        H = 0;
        break;
      }
    }
  }

  // Otherwise add the hit:
  if (H != 0) {
    m_HitCollection->insert(H);
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
void MCScintillatorSD::EndOfEvent(G4HCofThisEvent* HCE)
{
  if  (MCScintillatorSD::m_HitCollection != 0) {
    int HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    HCE->AddHitsCollection(HCID, m_HitCollection);

    MCScintillatorSD::m_HitCollection = 0;
    // The collection is automatically deleted by G4HCofThisEvent
  }
}


/*
 * MCScintillatorSD.cc: the end...
 ******************************************************************************/










