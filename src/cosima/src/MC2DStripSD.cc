/*
 * MC2DStripSD.cxx
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
#include "MC2DStripSD.hh"
#include "MC2DStripHit.hh"
#include "MCDetectorConstruction.hh"
#include "MCTrackInformation.hh"

// MEGAlib:
#include "MAssert.h"
#include "MStreams.h"

// Geant4:
#include "G4VPhysicalVolume.hh"
#include "G4Step.hh"
#include "G4VTouchable.hh"
#include "G4TouchableHistory.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "G4ThreeVector.hh"


/******************************************************************************
 * Default constructor
 */
MC2DStripSD::MC2DStripSD(G4String Name) : MCSD(Name), m_HitCollection(0)
{
  m_Type = c_2DStrip;
  collectionName.insert(Name);
}


/******************************************************************************
 * Default destructor
 */
MC2DStripSD::~MC2DStripSD()
{
  // Intentionally left blank
}


/******************************************************************************
 * Basically only initialize the hit collection
 */
void MC2DStripSD::Initialize(G4HCofThisEvent* HCE)
{
  if (HCE == 0) {
    massert("HCE is zero!");
  }

  // Where is this thing deleted??? 
  m_HitCollection = 
    new MC2DStripHitsCollection(SensitiveDetectorName, collectionName[0]);
}


/******************************************************************************
 * Set whether this is a 3D detector or not
 */
void MC2DStripSD::SetIs3D(bool Is3D) 
{ 
  m_Is3D = Is3D; 
  if (m_Is3D == true) {
    m_Type = c_3DStrip;
  } else {
    m_Type = c_2DStrip;
  }
}


/******************************************************************************
 * Process the hit: Don't do anything
 * This quirk is necessay since in the Geant4 SteppingManager the hits are 
 * processed before the UserSteppingAction is called, and for storing the 
 * correct IA information, we must first update the track information
 */
G4bool MC2DStripSD::ProcessHits(G4Step*, G4TouchableHistory*)
{ 
  // We are not doing anything!!!

  return true;
}


/******************************************************************************
 * Process the hit: Center, check if the hit did not happenend in the 
 * guard ring, generate a hit and add it to the hit collection
 */
G4bool MC2DStripSD::PostProcessHits(const G4Step* Step)
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

  // Apply any energy/charge/light loss
  if (m_UseEnergyLoss == true) {
    Energy *= m_EnergyLoss.Evaluate(Position.getX()/cm, Position.getY()/cm, Position.getZ()/cm);
  }
  
  if (Energy <= 0.0) {
    mout<<"MC2DStripSD: Info: Energy loss map resulted in zero or negative energy: "<<m_EnergyLoss.Evaluate(Position.getX()/cm, Position.getY()/cm, Position.getZ()/cm)<<":"<<Position<<endl;    
    return false;
  }

  // Now digitize the position - calculate strips and center in z:
  
  // First take care of IAs on the boundary
  if (Position.getX() >= m_XSize && Position.getX() <= m_XSize + m_Epsilon) {
    Position.setX(m_XSize - m_Epsilon);
  } else if (Position.getX() <= -m_XSize && Position.getX() >= -m_XSize - m_Epsilon) {
    Position.setX(-m_XSize + m_Epsilon);
  }
  if (Position.getY() >= m_YSize && Position.getY() <= m_YSize + m_Epsilon) {
    Position.setY(m_YSize - m_Epsilon);
  } else if (Position.getY() <= -m_YSize && Position.getY() >= -m_YSize - m_Epsilon) {
    Position.setY(-m_YSize + m_Epsilon);
  }

  // Ignore the hit if it is out side the sensitive part.
  // and take care of guard rings:
  bool IsGuardringHit = false;
  if (fabs(Position.getX()) >= m_XSize - m_XOffset || 
      fabs(Position.getY()) >= m_YSize - m_YOffset) {
    if (fabs(Position.getX()) >= m_XSize ||
        fabs(Position.getY()) >= m_YSize) {
      merr<<"MC2DStripSD: Outside of sensitive material: ("
          <<Position[0]<<", "<<Position[1]<<", "<<Position[2]<<")"<<endl;
      return false;
    } else {
      IsGuardringHit = true;
    }
  }
  
  // Now calculate the strips:
  
  // First in the sensitive volume:
  G4int xStrip = -1;
  G4int yStrip = -1;

  G4double dStrip = 0;
  G4double dStripTolerance = 0.0001;
  if (IsGuardringHit == false) {
    if (m_XNStrips == 1 || m_XPitch == 0) {
      xStrip = 0;
    } else {
      dStrip = ((Position.getX() + m_XSize - m_XOffset)/m_XPitch);
      xStrip = (G4int) dStrip;
      if (xStrip == -1 && dStrip > -dStripTolerance) xStrip = 0;
      if (xStrip == m_XNStrips && dStrip < m_XNStrips + dStripTolerance) xStrip = m_XNStrips - 1;
    }
    
    if (m_YNStrips == 1 || m_YPitch == 0) {  
      yStrip = 0;
    } else {
      dStrip = ((Position.getY() + m_YSize - m_YOffset)/m_YPitch);
      yStrip = (G4int) dStrip;
      if (yStrip == -1 && dStrip > -dStripTolerance) yStrip = 0;
      if (yStrip == m_YNStrips && dStrip < m_YNStrips + dStripTolerance) yStrip = m_YNStrips - 1;
    }

    // Check if we have a reasonable strip:
    if (xStrip < 0 || xStrip >= m_XNStrips) {
      merr<<"Invalid x-strip number: "<<xStrip<<" [0-"<<m_XNStrips-1<<"]"<<endl
          <<"   Position in detector was ("<<Position[0]/cm<<", "
          <<Position[1]/cm<<", "<<Position[2]/cm<<") cm "<<endl;
    }
    if (yStrip < 0 || yStrip >= m_YNStrips) {
      merr<<"Invalid y-strip number: "<<yStrip<<" [0-"<<m_YNStrips-1<<"]"<<endl
          <<"   Position in detector was ("<<Position[0]/cm <<", "
          <<Position[1]/cm<<", "<<Position[2]/cm<<") cm "<<endl;
    }

    // Now center the positions:
    if (m_DiscretizeHits == true) {
      Position.setX(-m_XSize + m_XOffset + (xStrip + 0.5)*m_XPitch);
      Position.setY(-m_YSize + m_YOffset + (yStrip + 0.5)*m_YPitch);
      if (m_Is3D == false) {
        Position.setZ(0.0);
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

  } else {
    // Position is a unique position on the guardring
    Position = m_UniqueGuardringPosition;
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

  // Create a hit:
  MC2DStripHit* Hit = new MC2DStripHit(m_Is3D);
  Hit->SetEnergy(Energy);
  Hit->SetXStrip(xStrip);
  Hit->SetYStrip(yStrip);
  Hit->SetIsGuardringHit(IsGuardringHit);
  Hit->SetADCCounts(Energy/keV);
  Hit->SetPosition(Position);
  Hit->SetDetectorName(DetectorName);
  for (int v =  Hist->GetHistoryDepth()-1; v >= 0; v--) {
    Hit->AddVolumeHistory(Hist->GetVolume(v)->GetName());
  }  
  Hit->AddOrigin(((MCTrackInformation*) 
                  (Step->GetTrack()->GetUserInformation()))->GetId());
  if (m_HasTimeResolution == true) {
    Hit->SetTime(Step->GetTrack()->GetGlobalTime());
  }

  // Check if there is already a hit in the strips of this layer:
  bool Added = false;
  if (m_DiscretizeHits == true) {
    for (int h = 0; h < m_HitCollection->entries(); h++) {
      if (*(*m_HitCollection)[h] == *Hit) {
        *(*m_HitCollection)[h] += *Hit;
        Added = true;
        delete Hit;
        Hit = 0;
        break;
      }
    }
  }

  // Otherwise add the hit:
  if (Hit != 0) {
    m_HitCollection->insert(Hit);
    Added = true;
  }  

  // This info makes sure the energy deposit is not added to the passive matrerial list
  if (Added == true) {
    MCTrackInformation* TrackInfo = 
      (MCTrackInformation*) (Step->GetTrack()->GetUserInformation());
    TrackInfo->SetDigitized(true);
  }

  return true;
}


/******************************************************************************
 * Standard housekeeping of event action for geant
 */
void MC2DStripSD::EndOfEvent(G4HCofThisEvent* HCE)
{
  if (MC2DStripSD::m_HitCollection != 0) {
    int HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    HCE->AddHitsCollection(HCID, m_HitCollection);

    MC2DStripSD::m_HitCollection = 0;
    // The collection is automatically deleted by G4HCofThisEvent
  }
}


/*
 * MC2DStripSD.cc: the end...
 ******************************************************************************/










