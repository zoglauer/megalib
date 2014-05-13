/*
 * MCCalorBarSD.cxx
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
#include "MCCalorBarSD.hh"
#include "MCCalorBarHit.hh"
#include "MCDetectorConstruction.hh"
#include "MCTrackInformation.hh"

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


/******************************************************************************/



/******************************************************************************
 * Set name, type and collection 
 */
MCCalorBarSD::MCCalorBarSD(G4String Name) : 
  MCSD(Name), m_HitCollection(0), m_Is3D(false)
{
  m_Type = c_Calorimeter;
  collectionName.insert(Name);
}


/******************************************************************************
 * Default destructor: Empty
 */
MCCalorBarSD::~MCCalorBarSD()
{
  // Intentionally left blank 
}


/******************************************************************************
 * Create hit collection if not yet done
 */
void MCCalorBarSD::Initialize(G4HCofThisEvent* HCE)
{
  if (HCE == 0) {
    massert("HCE is zero!");
  }

  m_HitCollection = 
    new MCCalorBarHitsCollection(SensitiveDetectorName, collectionName[0]);
}


/******************************************************************************
 * Process the hit: Don't do anything
 * This quirk is necessay since in the Geant4 SteppingManager the hits are 
 * processed before the UserSteppingAction is called, and for storing the 
 * correct IA information, we must first update the track information
 */
G4bool MCCalorBarSD::ProcessHits(G4Step*, G4TouchableHistory*)
{ 
  // We are not doing anything!!!

  return true;
}


/******************************************************************************
 * Process the hit: Center positions, generate a hit and 
 * add it to the hit collection
 */
G4bool MCCalorBarSD::PostProcessHits(const G4Step* Step)
{
  G4double Energy = Step->GetTotalEnergyDeposit();
  if (Energy == 0.0) return false;

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
  }

  // Apply any energy/charge/light loss
  if (m_UseEnergyLoss == true) {
    Energy *= m_EnergyLoss.Evaluate(Position.getX()/cm, Position.getY()/cm, Position.getZ()/cm);
  }
  if (Energy <= 0.0) {
    mout<<"MCCalorBarSD: Info: Energy loss map resulted in zero or negative energy: "<<m_EnergyLoss.Evaluate(Position.getX()/cm, Position.getY()/cm, Position.getZ()/cm)<<":"<<Position<<endl;    
    return false;
  }

  // Get the bars:
  int xBar = -1;
  int yBar = -1;

  xBar = 
    (G4int) ((m_StructuralDimension[0] - 
              m_StructuralOffset[0] +
              DetectorPosition[0])/
             (m_StructuralSize[0]+
              m_StructuralPitch[0]));

  yBar = 
    (G4int) ((m_StructuralDimension[1] - 
              m_StructuralOffset[1] + 
              DetectorPosition[1])/
             (m_StructuralSize[1]+
              m_StructuralPitch[1]));


  // Now center the positions:
  if (m_DiscretizeHits == true) {
    Position.setX(0.0);
    Position.setY(0.0);
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
  MCCalorBarHit* H = new MCCalorBarHit();
  H->SetEnergy(Energy);
  H->SetXBar(xBar);
  H->SetYBar(yBar);
  H->SetDetectorName(DetectorName);
  H->AddOrigin(((MCTrackInformation*) 
                  (Step->GetTrack()->GetUserInformation()))->GetId());
  for (int v =  Hist->GetHistoryDepth()-1; v >= 0; v--) {
    H->AddVolumeHistory(Hist->GetVolume(v)->GetName());
  }  
  H->SetPosition(Position);
  if (m_HasTimeResolution == true) {
    H->SetTime(Step->GetTrack()->GetGlobalTime());
  }

  // Create ADC for single and double-sided calorimeter:
  if (m_Is3D == false) {
    H->SetADCCounts(Energy/keV);
  } else {
    double Eta = (m_StructuralDimension[2] - m_StructuralOffset[2] + DetectorPosition[2])/(2*m_StructuralSize[2]);
    if (Eta > 1.0 + m_Epsilon || Eta < 0.0 - m_Epsilon) {
      merr<<"Eta out of range [0..1]: "<<Eta<<endl;
      merr<<"Pos: "<<Position/cm<<" DetPos: "<<DetectorPosition/cm<<endl;
    }
    if (Eta > 1.0) Eta = 1.0;
    if (Eta < 0.0) Eta = 0.0;
    double ADCTop = 0.5*(1 + Eta)*Energy/keV;
    double ADCBottom = Energy/keV - ADCTop;
    H->SetADCCounts(ADCTop, ADCBottom);
  }

  // Check if there is already a hit in the strips and this layer:
  bool Added = false;
  if (m_DiscretizeHits == true) {
    for (G4int h = 0; h < MCCalorBarSD::m_HitCollection->entries(); h++) {
      if (*(*MCCalorBarSD::m_HitCollection)[h] == *H) {
        *(*MCCalorBarSD::m_HitCollection)[h] += *H;
        Added = true;
        delete H;
        H = 0;
        break;
      }
    }
  }

  // Otherwise add as a new hit:
  if (H != 0) {
    MCCalorBarSD::m_HitCollection->insert(H);
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
void MCCalorBarSD::EndOfEvent(G4HCofThisEvent* HCE)
{
  if (MCCalorBarSD::m_HitCollection != 0) {
    int HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    HCE->AddHitsCollection(HCID, m_HitCollection);

    MCCalorBarSD::m_HitCollection = 0;
    // The collection is automatically deleted by G4HCofThisEvent
  }
}


/*
 * MCCalorBarSD.cc: the end...
 ******************************************************************************/










