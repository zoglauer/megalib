/*
 * MCVoxel3DSD.cxx
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
#include "MCVoxel3DSD.hh"
#include "MCVoxel3DHit.hh"
#include "MCDetectorConstruction.hh"
#include "MCTrackInformation.hh"

// MEGAlib:
#include "MAssert.h"
#include "MStreams.h"
#include "MDDetector.h"

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
MCVoxel3DSD::MCVoxel3DSD(G4String Name) : 
  MCSD(Name), m_HitCollection(0)
{
  m_Type = MDDetector::c_Voxel3D;
  collectionName.insert(Name);
}


/******************************************************************************
 * Default destructor
 */
MCVoxel3DSD::~MCVoxel3DSD()
{
  // Intentionally left blank
}


/******************************************************************************
 * Basically only initialize the hit collection
 */
void MCVoxel3DSD::Initialize(G4HCofThisEvent* HCE)
{
  if (HCE == 0) {
    massert("HCE is zero!");
  }

  // Where is this thing deleted??? 
  m_HitCollection = 
    new MCVoxel3DHitsCollection(SensitiveDetectorName, collectionName[0]);
}


/******************************************************************************
 * Process the hit: Don't do anything
 * This quirk is necessay since in the Geant4 SteppingManager the hits are 
 * processed before the UserSteppingAction is called, and for storing the 
 * correct IA information, we must first update the track information
 */
G4bool MCVoxel3DSD::ProcessHits(G4Step*, G4TouchableHistory*)
{ 
  // We are not doing anything!!!

  return true;
}


/******************************************************************************
 * Process the hit: Center, check if the hit did not happenend in the 
 * guard ring, generate a hit and add it to the hit collection
 */
G4bool MCVoxel3DSD::PostProcessHits(const G4Step* Step)
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

  if (Position[0] != Position[0]) { // We cannot use isnan() here for windows compatibility
    mout<<"MCVoxel3DSD: Info: Geant4 propagated NaN as position: "<<Position<<endl;
    mout<<"Skipping this hit..."<<endl;
    return false;
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
    mout<<"MCVoxel3DSD: Info: Energy loss map resulted in zero or negative energy: "<<m_EnergyLoss.Evaluate(Position.getX()/cm, Position.getY()/cm, Position.getZ()/cm)<<":"<<Position<<endl;    
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
  if (Position.getZ() >= m_ZSize && Position.getZ() <= m_ZSize + m_Epsilon) {
    Position.setZ(m_ZSize - m_Epsilon);
  } else if (Position.getZ() <= -m_ZSize && Position.getZ() >= -m_ZSize - m_Epsilon) {
    Position.setZ(-m_ZSize + m_Epsilon);
  }

  
  // Ignore the hit if it is out side the sensitive part.
  // and take care of guard rings:
  bool IsGuardringHit = false;
  if (fabs(Position.getX()) >= m_XSize - m_XOffset || 
      fabs(Position.getY()) >= m_YSize - m_YOffset ||
      fabs(Position.getZ()) >= m_ZSize - m_ZOffset) {
    if (fabs(Position.getX()) >= m_XSize ||
        fabs(Position.getY()) >= m_YSize ||
        fabs(Position.getZ()) >= m_ZSize) {
      merr<<"MCVoxel3DSD: Outside of sensitive material: ("
            <<Position[0]<<", "<<Position[1]<<", "<<Position[2]<<")"<<endl;
      return false;
    } else {
      IsGuardringHit = true;
    }
  }
  
  // Now calculate the strips:
  
  // First in the sensitive volume:
  G4int xVoxel = -1;
  G4int yVoxel = -1;
  G4int zVoxel = -1;


  if (IsGuardringHit == false) {
    if (m_XNVoxels == 1) {
      xVoxel = 0;
    } else {
      xVoxel = (G4int) ((Position.getX() + m_XSize - m_XOffset)/m_XVoxelSize);
    }
    
    if (m_YNVoxels == 1) {  
      yVoxel = 0;
    } else {
      yVoxel = (G4int) ((Position.getY() + m_YSize - m_YOffset)/m_YVoxelSize);
    }
    
    if (m_ZNVoxels == 1) {  
      zVoxel = 0;
    } else {
      zVoxel = (G4int) ((Position.getZ() + m_ZSize - m_ZOffset)/m_ZVoxelSize);
    }

    
    // Check if we have a reasonable strip:
    if (xVoxel < 0 || xVoxel > m_XNVoxels-1) {
      merr<<"Invalid x-voxel number: "<<xVoxel<<endl
          <<"   Position was "<<Position[0]<<", "
          <<Position[1]<<", "<<Position[2]<<endl;
    }
    if (yVoxel < 0 || yVoxel > m_YNVoxels-1) {
      merr<<"Invalid y-voxel number: "<<yVoxel<<endl
          <<"   Position was "<<Position[0]<<", "
          <<Position[1]<<", "<<Position[2]<<endl;
    }
    if (zVoxel < 0 || zVoxel > m_ZNVoxels-1) {
      merr<<"Invalid z-voxel number: "<<zVoxel<<endl
          <<"   Position was "<<Position[0]<<", "
          <<Position[1]<<", "<<Position[2]<<endl;
    }

    // Now center the positions:
    if (m_DiscretizeHits == true) {
      Position.setX(-m_XSize + m_XOffset + (xVoxel + 0.5)*m_XVoxelSize);
      Position.setY(-m_YSize + m_YOffset + (yVoxel + 0.5)*m_YVoxelSize);
      Position.setZ(-m_ZSize + m_ZOffset + (zVoxel + 0.5)*m_ZVoxelSize);
    }
    
    // Determine the above position in the detector volume:
    DetectorPosition = Position;
    for (int v = 0; v < Hist->GetHistoryDepth(); v++) {
      Vol = Hist->GetVolume(v);
      if (Vol->GetLogicalVolume()->GetName() == m_DetectorVolumeName) {
        break;
      }

      // Translate and rotate the position into sensitive detector coordinates.
      if (Vol->GetFrameRotation() != 0) {
        DetectorPosition = (*Vol->GetObjectRotation()) * DetectorPosition;
      }
      DetectorPosition += Vol->GetObjectTranslation();
    }

    // Then in the detector volume:
    xVoxel += 
      (G4int) ((m_StructuralDimension[0] - 
                m_StructuralOffset[0] + 
                DetectorPosition[0])/
               (m_StructuralSize[0]+
                m_StructuralPitch[0]))*m_XNVoxels;

    yVoxel += 
      (G4int) ((m_StructuralDimension[1] - 
                m_StructuralOffset[1] + 
                DetectorPosition[1])/
               (m_StructuralSize[1]+
                m_StructuralPitch[1]))*m_YNVoxels;
    
    zVoxel += 
      (G4int) ((m_StructuralDimension[2] - 
                m_StructuralOffset[2] + 
                DetectorPosition[2])/
               (m_StructuralSize[2]+
                m_StructuralPitch[2]))*m_ZNVoxels;
  } else {
    // Position is a unique position on the guard ring
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
  MCVoxel3DHit* Hit = new MCVoxel3DHit();
  Hit->SetEnergy(Energy);
  Hit->SetXVoxel(xVoxel);
  Hit->SetYVoxel(yVoxel);
  Hit->SetZVoxel(zVoxel);
  Hit->SetIsGuardringHit(IsGuardringHit);
  Hit->SetADCCounts(Energy/keV);
  Hit->SetPosition(Position);
  Hit->SetDetectorName(DetectorName);
  for (int v =  Hist->GetHistoryDepth()-1; v >= 0; v--) {
    Hit->AddVolumeHistory(Hist->GetVolume(v)->GetName());
  }  
  Hit->AddOrigin(((MCTrackInformation*) 
                  (Step->GetTrack()->GetUserInformation()))->GetId());
  if (m_HasTimeResolution) {
    Hit->SetTime(Step->GetTrack()->GetGlobalTime());
  }

  // Check if there is already a hit in the voxel of this layer:
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
void MCVoxel3DSD::EndOfEvent(G4HCofThisEvent* HCE)
{
  if (MCVoxel3DSD::m_HitCollection != 0) {
    int HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    HCE->AddHitsCollection(HCID, m_HitCollection);

    MCVoxel3DSD::m_HitCollection = 0;
    // The collection is automatically deleted by G4HCofThisEvent
  }
}


/*
 * MCVoxel3DSD.cc: the end...
 ******************************************************************************/










