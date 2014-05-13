/*
 * MCDriftChamberSD.cxx
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
#include "MCDriftChamberSD.hh"
#include "MCDriftChamberHit.hh"
#include "MCDetectorConstruction.hh"
#include "MCTrackInformation.hh"

// MEGAlib:
#include "MGlobal.h"
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
#include "Randomize.hh"


/******************************************************************************
 * Default constructor
 */
MCDriftChamberSD::MCDriftChamberSD(G4String Name) : 
  MC2DStripSD(Name), m_HitCollection(0)
{
  m_Type = c_DriftChamber;
  collectionName.insert(Name);
}


/******************************************************************************
 * Default destructor
 */
MCDriftChamberSD::~MCDriftChamberSD()
{
  // Intentionally left blank
}


/******************************************************************************
 * Basically only initialize the hit collection
 */
void MCDriftChamberSD::Initialize(G4HCofThisEvent* HCE)
{
  if (HCE == 0) {
    massert("HCE is zero!");
  }

  // Where is this thing deleted??? 
  m_HitCollection = 
    new MCDriftChamberHitsCollection(SensitiveDetectorName, collectionName[0]);
}


/******************************************************************************
 * Process the hit: Don't do anything
 * This quirk is necessay since in the Geant4 SteppingManager the hits are 
 * processed before the UserSteppingAction is called, and for storing the 
 * correct IA information, we must first update the track information
 */
G4bool MCDriftChamberSD::ProcessHits(G4Step*, G4TouchableHistory*)
{ 
  // We are not doing anything!!!

  return true;
}


/******************************************************************************
 * Process the hit: Center, check if the hit did not happenend in the 
 * guard ring, generate a hit and add it to the hit collection
 */
G4bool MCDriftChamberSD::PostProcessHits(const G4Step* Step)
{

  G4double Energy = Step->GetTotalEnergyDeposit();

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

  if (Energy == 0.0) return false;

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
  //cout<<"Final: "<<Position<<endl;

  // Apply any energy/charge/light loss
  if (m_UseEnergyLoss == true) {
    Energy *= m_EnergyLoss.Evaluate(Position.getX()/cm, Position.getY()/cm, Position.getZ()/cm);
  }
  if (Energy <= 0.0) {
    mout<<"MCDriftChamberSD: Info: Energy loss map resulted in zero or negative energy: "<<m_EnergyLoss.Evaluate(Position.getX()/cm, Position.getY()/cm, Position.getZ()/cm)<<":"<<Position<<endl;    
    return false;
  }

  // Now digitize the position - calculate strips and center in z:
  
  // Ignore the hit if it is outside the sensitive part (guard ring?):
  if (fabs(Position.getX()) >= m_XSize - m_XOffset || 
      fabs(Position.getY()) >= m_YSize - m_YOffset) {
    mdebug<<"MCDriftChamberSD: Ouside of sensitive material: ("
          <<Position[0]<<", "<<Position[1]<<", "<<Position[2]<<")"<<endl;
    return false;
  }
  
  // Now it's time for the drift:
  
  // Split the step into "electrons"
  int NElectrons = int(Energy/m_EnergyPerElectron);
  if (NElectrons == 0) NElectrons = 1;
  //double EnergyPerElectron = Energy/NElectrons;
  NElectrons = 1;
  
  // Calculate the drift parameters
  double DriftLength = Position.getZ() + m_StructuralSize.getZ()/2;
  massert(DriftLength >= 0);
  //double DriftRadiusSigma = m_DriftConstant * sqrt(DriftLength);

  //double DriftRadius = 0;
  //double DriftAngle = 0;
  double DriftX = 0;
  double DriftY = 0;

  for (int e = 0; e < NElectrons; ++e) {
    // Randomize x, y position:
    
//     DriftRadius = RandGauss::shoot(0, DriftRadiusSigma);
//     DriftAngle = RandFlat::shoot() * c_Pi;
//     DriftX = DriftRadius*cos(DriftAngle);
//     DriftY = DriftRadius*sin(DriftAngle);
   
    G4ThreeVector DriftPosition = Position + G4ThreeVector(DriftX, DriftY, 0);
//     if (fabs(DriftPosition.getX()) > m_XSize || 
//         fabs(DriftPosition.getY()) > m_YSize) {
//       mout<<"Energy lost at "<<DriftPosition<<": "<<m_XSize<<", "<<m_YSize<<endl;
//       continue;
//     }
    
    // Now calculate the strips:
  
    // First in the sensitive volume:
    G4int xStrip;
    if (m_XNStrips == 1 || m_XPitch == 0) {
      xStrip = 0;
    } else {
      xStrip = (G4int) ((DriftPosition.getX() + m_XSize - m_XOffset)/m_XPitch);
    }

    G4int yStrip;
    if (m_YNStrips == 1 || m_YPitch == 0) {  
      yStrip = 0;
    } else {
      yStrip = (G4int) ((DriftPosition.getY() + m_YSize - m_YOffset)/m_YPitch);
    }

    // Check if we have a reasonable strip:
    if (xStrip < 0 || xStrip >= m_XNStrips) {
      merr<<"Invalid x-strip number: "<<xStrip<<endl
          <<"   Position was "<<DriftPosition[0]<<", "
          <<DriftPosition[1]<<", "<<DriftPosition[2]<<endl;
    }
    if (yStrip < 0 || yStrip >= m_YNStrips) {
      merr<<"Invalid y-strip number: "<<yStrip<<endl
          <<"   Position was "<<DriftPosition[0]<<", "
          <<DriftPosition[1]<<", "<<DriftPosition[2]<<endl;
    }

    // Now center the positions:
    if (m_DiscretizeHits == true) {
      DriftPosition.setX(-m_XSize + m_XOffset + (xStrip + 0.5)*m_XPitch);
      DriftPosition.setY(-m_YSize + m_YOffset + (yStrip + 0.5)*m_YPitch);
      if (m_Is3D == false) {
        DriftPosition.setZ(0.0);
      }
    }

    // Then in the detector volume:
    xStrip += 
      (G4int) ((m_StructuralDimension[0] - 
                m_StructuralOffset[0] + 
                DetectorPosition[0])/
               (m_StructuralSize[0]+
                m_StructuralPitch[0]))*m_XNStrips;
    
    yStrip += 
      (G4int) ((m_StructuralDimension[1] - 
                m_StructuralOffset[1] + 
                DetectorPosition[1])/
               (m_StructuralSize[1]+
                m_StructuralPitch[1]))*m_YNStrips;
    
    // Rotate back into the original system:
    for (int v = 0; v < Hist->GetHistoryDepth(); v++) {
      Vol = Hist->GetVolume(v);

      // Translate and rotate the position into sensitive detector coordinates.
      if (Vol->GetFrameRotation() != 0) {
        DriftPosition = (*Vol->GetObjectRotation()) * DriftPosition;
      }
      DriftPosition += Vol->GetObjectTranslation();
    }

    // Create a hit:
    MCDriftChamberHit* H = new MCDriftChamberHit();
    H->SetEnergy(Energy/NElectrons);
    H->SetXStrip(xStrip);
    H->SetYStrip(yStrip);
    H->SetADCCounts(Energy/NElectrons/keV);
    H->SetPosition(DriftPosition);
    H->SetDetectorName(DetectorName);
    for (int v =  Hist->GetHistoryDepth()-1; v >= 0; v--) {
      H->AddVolumeHistory(Hist->GetVolume(v)->GetName());
    }  
    H->AddOrigin(((MCTrackInformation*) 
                    (Step->GetTrack()->GetUserInformation()))->GetId());
    if (m_HasTimeResolution == true) {
      H->SetTime(Step->GetTrack()->GetGlobalTime());
    }

    // Check if there is already a hit in the strips of this layer:
    bool Added = false;
    if (m_DiscretizeHits == true) {
      for (int h = 0; h < m_HitCollection->entries(); h++) {
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
      MCTrackInformation* TrackInfo = 
        (MCTrackInformation*) (Step->GetTrack()->GetUserInformation());
      TrackInfo->SetDigitized(true);
    }
  }

  return true;
}


/******************************************************************************
 * Standard housekeeping of event action for geant
 */
void MCDriftChamberSD::EndOfEvent(G4HCofThisEvent* HCE)
{
  if (MCDriftChamberSD::m_HitCollection != 0) {
    int HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    HCE->AddHitsCollection(HCID, m_HitCollection);

    MCDriftChamberSD::m_HitCollection = 0;
    // The collection is automatically deleted by G4HCofThisEvent
  }
}


/*
 * MCDriftChamberSD.cc: the end...
 ******************************************************************************/










