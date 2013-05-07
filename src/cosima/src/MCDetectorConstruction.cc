/*
 * MCDetectorConstruction.cxx
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


// Standard:
#include <sstream>
using namespace std;

// Cosima:
#include "MCDetectorConstruction.hh"
#include "MC2DStripSD.hh"
#include "MCScintillatorSD.hh"
#include "MCCalorBarSD.hh"
#include "MCDriftChamberSD.hh"
#include "MCAngerCameraSD.hh"
#include "MCVoxel3DSD.hh"
#include "MCRunManager.hh"
#include "MCSource.hh"
#include "MCRun.hh"
#include "MCEventAction.hh"
#include "MCRegion.hh"
#include "MCGeometryConverter.hh"

// Geant4:
#include "globals.hh"
#include "G4Element.hh"
#include "G4Material.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Trd.hh"
#include "G4Trap.hh"
#include "G4Cons.hh"
#include "G4Polycone.hh"
#include "G4Polyhedra.hh"
#include "G4Sphere.hh"
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4PVPlacement.hh"
#include "G4SolidStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SDManager.hh"
#include "G4UserLimits.hh"
#include "G4Region.hh"
#include "G4VisAttributes.hh"
#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4NistManager.hh"

// MEGAlib:
#include "MAssert.h"
#include "MStreams.h"
#include "MDVolume.h"
#include "MDShapeBRIK.h"
#include "MDShapeTUBS.h"
#include "MDShapeSPHE.h"
#include "MDShapeTRD1.h"
#include "MDShapeTRAP.h"
#include "MDShapeCONE.h"
#include "MDShapeCONS.h"
#include "MDShapePGON.h"
#include "MDShapePCON.h"
#include "MDMaterial.h"
#include "MDMaterialComponent.h"
#include "MDDetector.h"
#include "MDStrip2D.h"
#include "MDCalorimeter.h"
#include "MDACS.h"
#include "MDDriftChamber.h"
#include "MDVoxel3D.h"
#include "MVector.h"

// Root:
#include "TRotMatrix.h"
#include "TMatrix.h" 
#include "TMath.h" 
#include "TColor.h"


/******************************************************************************
 * Default constructor
 */
MCDetectorConstruction::MCDetectorConstruction(MCParameterFile& RunParameters) 
  : m_RunParameters(RunParameters), m_WorldVolume(0)
{
  // Intentionally left blank
}


/******************************************************************************
 * Default destructor
 */
MCDetectorConstruction::~MCDetectorConstruction()
{
  // Intentionally left blank

  delete m_Geometry;
}


/******************************************************************************
 * Default Geant4 Construct Method, returns the world volume 
 */
G4VPhysicalVolume* MCDetectorConstruction::Construct()
{
  if (m_WorldVolume == 0) {
    mout<<"Geometry is not correctly initialized"
        <<"this will end in a fatal system crash..."<<endl;
  }

  return m_WorldVolume;
}


/******************************************************************************
 * Should be directly called after constructor:
 * Initializes the geometry
 */
bool MCDetectorConstruction::Initialize()
{
  // Cleanup old geometry

  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::GetInstance()->Clean();
  G4LogicalVolumeStore::GetInstance()->Clean();
  G4SolidStore::GetInstance()->Clean();

  m_Geometry = new MDGeometryQuest(); 

  // Read and initilize the geometry from MEGAlib
  if (m_Geometry->ScanSetupFile(m_RunParameters.GetGeometryFileName().c_str(), false, false, false) 
      == false) {
    return false;
  }


  // Initialize start area for all sources, but only if it has not been set (parameter file preceeds geometry file):
  vector<MCRun>& Runs = MCRunManager::GetMCRunManager()->GetRuns();
  for (unsigned int r = 0; r < Runs.size(); ++r) {
    vector<MCSource*>& Sources = Runs[r].GetSourceList();
    for (unsigned s = 0; s < Sources.size(); ++s) {
      if (Sources[s]->GetStartAreaType() == MCSource::c_StartAreaUnknown) {
        Sources[s]->SetStartAreaType(MCSource::c_StartAreaSphere);
        if (Sources[s]->SetStartAreaParameters(m_Geometry->GetStartSpherePosition()[0]*cm,
                                               m_Geometry->GetStartSpherePosition()[1]*cm,
                                               m_Geometry->GetStartSpherePosition()[2]*cm,
                                               0.0,
                                               0.0,
                                               0.0,
                                               m_Geometry->GetStartSphereRadius()*cm) == false) {
          return false;
        }
      }
    }
  }

  if (ConstructMaterials() == false) return false;
  if (ConstructVolumes() == false) return false;

  // Recursively position all volumes
  PositionVolumes(m_Geometry->GetWorldVolume());
  if (m_WorldVolume == 0) return false;

  if (ConstructDetectors() == false) return false;

  if (ConstructRegions() == false) return false;

  // Modify the trigger unit:
  // In order to be able to modify energies, thresholds etc. we only pre-trigger here
  if (m_Geometry->GetTriggerUnit() == 0) {
    mout<<"Error: Geometry has no trigger unit. Please define a trigger criteria in the geometry file."<<endl;
    return false;
  }
  // That means:
  // (a) No vetoes
  m_Geometry->GetTriggerUnit()->IgnoreVetoes(true);
  // (b) Thresholds are at zero
  m_Geometry->GetTriggerUnit()->IgnoreThresholds(true);
  
  //G4cout << *(G4Material::GetMaterialTable()) << G4endl;

  return true;
}


/******************************************************************************
 * Create a rotation matrix 
 */
G4RotationMatrix* MCDetectorConstruction::CreateRotation(MDVolume* Volume)
{
  MVector xvcolumn, yvcolumn, zvcolumn;

  xvcolumn = MVector(1.,0.,0.);
  yvcolumn = MVector(0.,1.,0.);
  zvcolumn = MVector(0.,0.,1.);

  TMatrixD RotMatrix = Volume->GetRotationMatrix();

  xvcolumn = RotMatrix*xvcolumn;
  yvcolumn = RotMatrix*yvcolumn;
  zvcolumn = RotMatrix*zvcolumn;   

  G4ThreeVector xColumn(xvcolumn[0], xvcolumn[1], xvcolumn[2]);
  G4ThreeVector yColumn(yvcolumn[0], yvcolumn[1], yvcolumn[2]);
  G4ThreeVector zColumn(zvcolumn[0], zvcolumn[1], zvcolumn[2]);

  return new G4RotationMatrix(xColumn, yColumn, zColumn);
}


/******************************************************************************
 * Create detectors
 */
bool MCDetectorConstruction::ConstructDetectors()
{
  // Define detectors and sensitive material stuff...
  G4SDManager* SDManager = G4SDManager::GetSDMpointer();
  MCEventAction* EventAction = 
    (MCEventAction *) (G4EventManager::GetEventManager()->GetUserEventAction());

  int Type;
  MString Name;
  G4ThreeVector Vector;

  MDDetector* Detector = 0;
  MCSD* SD = 0;
  for (unsigned int d = 0; d < m_Geometry->GetNDetectors(); ++d) {

    Type = m_Geometry->GetDetectorAt(d)->GetDetectorType();
    Name = m_Geometry->GetDetectorAt(d)->GetName() + "SD";

    Detector = m_Geometry->GetDetectorAt(d);

    if (Type == MDDetector::c_Strip2D || Type == MDDetector::c_Strip3D) {
      MDStrip2D* Strip = dynamic_cast<MDStrip2D*>(Detector);
      
      MC2DStripSD* TwoDStripSD = new MC2DStripSD(Name.Data());
      SD = TwoDStripSD;
      
      if (Type == MDDetector::c_Strip3D) {
        TwoDStripSD->SetIs3D(true);
      } else {
        TwoDStripSD->SetIs3D(false);
      }

      // Add the dimensions:
      TwoDStripSD->SetSize(0.5*Strip->GetWidthX()*cm, 
                           0.5*Strip->GetWidthY()*cm);
      TwoDStripSD->SetOffset(Strip->GetOffsetX()*cm, 
                             Strip->GetOffsetY()*cm);
      TwoDStripSD->SetPitch(Strip->GetPitchX()*cm, 
                            Strip->GetPitchY()*cm);
      TwoDStripSD->SetNStrips(Strip->GetNStripsX(), 
                              Strip->GetNStripsY());
      TwoDStripSD->SetUniqueGuardringPosition(G4ThreeVector(Strip->GetUniqueGuardringPosition().X()*cm,
                                                            Strip->GetUniqueGuardringPosition().Y()*cm,
                                                            Strip->GetUniqueGuardringPosition().Z()*cm));

      SDManager->AddNewDetector(TwoDStripSD);
      mdebug<<"Adding Strip detector for "<<Name<<endl;
      
      // Get the name of the sensitive volume and set its SD:
      for (unsigned int s = 0; 
           s < Detector->GetNSensitiveVolumes(); ++s) {
        MString SenName = 
          Detector->GetSensitiveVolume(s)->GetName() + 
          "Log";

        G4LogicalVolumeStore* SS = G4LogicalVolumeStore::GetInstance();
        for (unsigned int s = 0; s < SS->size(); ++s) {
          if (SenName == SS->at(s)->GetName().c_str()) {
            SS->at(s)->SetSensitiveDetector(TwoDStripSD);
          }
        }
      }
    } 

    else if (Type == MDDetector::c_Voxel3D) {
      MDVoxel3D* Voxler = dynamic_cast<MDVoxel3D*>(Detector);
      
      MCVoxel3DSD* Voxel3DSD = new MCVoxel3DSD(Name.Data());
      SD = Voxel3DSD;

      // Add the dimensions:
      Voxel3DSD->SetSize(0.5*Voxler->GetWidthX()*cm, 
                         0.5*Voxler->GetWidthY()*cm,
                         0.5*Voxler->GetWidthZ()*cm);
      Voxel3DSD->SetOffset(Voxler->GetOffsetX()*cm, 
                           Voxler->GetOffsetY()*cm,
                           Voxler->GetOffsetZ()*cm);
      Voxel3DSD->SetVoxelSize(Voxler->GetVoxelSizeX()*cm, 
                              Voxler->GetVoxelSizeY()*cm,
                              Voxler->GetVoxelSizeZ()*cm);
      Voxel3DSD->SetNVoxels(Voxler->GetNVoxelsX(), 
                            Voxler->GetNVoxelsY(),
                            Voxler->GetNVoxelsZ());
      Voxel3DSD->SetUniqueGuardringPosition(G4ThreeVector(Voxler->GetUniqueGuardringPosition().X()*cm,
                                                          Voxler->GetUniqueGuardringPosition().Y()*cm,
                                                          Voxler->GetUniqueGuardringPosition().Z()*cm));

      SDManager->AddNewDetector(Voxel3DSD);
      mdebug<<"Adding Voxler detector for "<<Name<<endl;
      
      // Get the name of the sensitive volume and set its SD:
      for (unsigned int s = 0; 
           s < Detector->GetNSensitiveVolumes(); ++s) {
        MString SenName = 
          Detector->GetSensitiveVolume(s)->GetName() + 
          "Log";

        G4LogicalVolumeStore* SS = G4LogicalVolumeStore::GetInstance();
        for (unsigned int s = 0; s < SS->size(); ++s) {
          if (SenName == SS->at(s)->GetName().c_str()) {
            SS->at(s)->SetSensitiveDetector(Voxel3DSD);
          }
        }
      }
    } 

    else if (Type == MDDetector::c_DriftChamber) {
      MDDriftChamber* Chamber = dynamic_cast<MDDriftChamber*>(Detector);
      
      MCDriftChamberSD* ChamberSD = new MCDriftChamberSD(Name.Data());
      SD = ChamberSD;

      // Add the dimensions:
      ChamberSD->SetSize(0.5*Chamber->GetWidthX()*cm, 
                         0.5*Chamber->GetWidthY()*cm);
      ChamberSD->SetOffset(Chamber->GetOffsetX()*cm, 
                           Chamber->GetOffsetY()*cm);
      ChamberSD->SetPitch(Chamber->GetPitchX()*cm, 
                          Chamber->GetPitchY()*cm);
      ChamberSD->SetNStrips(Chamber->GetNStripsX(), 
                            Chamber->GetNStripsY());

      ChamberSD->SetLightSpeed(Chamber->GetLightSpeed()*cm/s);
      ChamberSD->SetLightDetectorPosition(Chamber->GetLightDetectorPosition());
      ChamberSD->SetDriftConstant(Chamber->GetDriftConstant());
      ChamberSD->SetEnergyPerElectron(Chamber->GetEnergyPerElectron()*keV);


      SDManager->AddNewDetector(ChamberSD);
      mdebug<<"Adding drift chamber detector for "<<Name<<endl;
      
      // Get the name of the sensitive volume and set its SD:
      for (unsigned int s = 0; 
           s < Detector->GetNSensitiveVolumes(); ++s) {
        MString SenName = 
          Detector->GetSensitiveVolume(s)->GetName() + 
          "Log";

        G4LogicalVolumeStore* SS = G4LogicalVolumeStore::GetInstance();
        for (unsigned int s = 0; s < SS->size(); ++s) {
          if (SenName == SS->at(s)->GetName().c_str()) {
            SS->at(s)->SetSensitiveDetector(ChamberSD);
          }
        }
      }
    } 

    else if (Type == MDDetector::c_Calorimeter) {
      MCCalorBarSD* CalorimeterSD = new MCCalorBarSD(Name.Data());
      SD = CalorimeterSD;
      SDManager->AddNewDetector(CalorimeterSD);
      mdebug<<"Adding calorimeter for "<<Name<<endl;

      // Check if we have a 3D calorimeter:
      MDCalorimeter* Calorimeter = dynamic_cast<MDCalorimeter*>(Detector);
      if (Calorimeter->HasDepthResolution() == true) {
        CalorimeterSD->SetIs3D(true);
      }
    
      // Get the name of the sensitive volume and set its SD:
      for (unsigned int s = 0; 
           s < Detector->GetNSensitiveVolumes(); ++s) {
        MString SenName = 
          Detector->GetSensitiveVolume(s)->GetName() 
          + "Log";

        G4LogicalVolumeStore* SS = G4LogicalVolumeStore::GetInstance();
        for (unsigned s = 0; s < SS->size(); ++s) {
          if (SenName == SS->at(s)->GetName().c_str()) {
            SS->at(s)->SetSensitiveDetector(CalorimeterSD);
          }
        }
      }
    } 

    else if (Type == MDDetector::c_ACS) {
      MCScintillatorSD* S = new MCScintillatorSD(Name.Data());
      SD = S;
      SDManager->AddNewDetector(S);
      mout<<"Adding szintillator for "<<Name<<endl;
     
      S->SetCommonVolumeName(Detector->GetCommonVolume()->GetName().Data());

      MVector UniquePositionInCommon(0.0, 0.0, 0.0);

      // Get the name of the sensitive volumes and set its SD:
      for (unsigned int s = 0; 
           s < Detector->GetNSensitiveVolumes(); ++s) {
        MString SenName = Detector->GetSensitiveVolume(s)->GetName() + "Log";
        MVector Pos = Detector->GetSensitiveVolume(s)->GetShape()->GetUniquePosition();
        // The position of the hits is the unique position in the first volume
        if (s == 0) {
          S->SetUniquePosition(SenName.Data(), G4ThreeVector(Pos[0]*cm, Pos[1]*cm, Pos[2]*cm));
          // Get the position in the common volume:
          MDVolume* V = Detector->GetSensitiveVolume(s);
          while (V != 0 && V != Detector->GetCommonVolume()) {
            UniquePositionInCommon = V->GetPositionInMotherVolume(UniquePositionInCommon);
            V = V->GetMother();
          }
        } else {
          // Build a volume tree:
          vector<MDVolume* > Vs;
          Vs.push_back(Detector->GetSensitiveVolume(s));
          while (Vs.back() != 0 && Vs.back() != Detector->GetCommonVolume()) {
            Vs.push_back(Vs.back()->GetMother());
          }
          // Rotate and translate the unique position in common volume into this sensitive volume:
          MVector NewPos = UniquePositionInCommon;
          for (int i = int(Vs.size()) - 2; i >= 0; --i) {
            NewPos -= Vs[i]->GetPosition();           // translate 
            if (Vs[i]->IsRotated() == true) {
              NewPos = Vs[i]->GetRotationMatrix() * NewPos;   // rotate
            }
          }
          S->SetUniquePosition(SenName.Data(), G4ThreeVector(NewPos[0]*cm, NewPos[1]*cm, NewPos[2]*cm));
        }
        G4LogicalVolumeStore* SS = G4LogicalVolumeStore::GetInstance();
        for (unsigned s = 0; s < SS->size(); ++s) {
          if (SenName == SS->at(s)->GetName().c_str()) {
            cout<<"SenName: "<<SenName<<endl;
            SS->at(s)->SetSensitiveDetector(S);
          }
        }
      }
    }

    else if (Type == MDDetector::c_AngerCamera) {
      MCAngerCameraSD* S = new MCAngerCameraSD(Name.Data());
      SD = S;
      SDManager->AddNewDetector(S);
      mout<<"Adding Anger camera for "<<Name<<endl;
     
      // Get the name of the sensitive volumes and set its SD:
      // Get the name of the sensitive volume and set its SD:
      for (unsigned int s = 0; 
           s < Detector->GetNSensitiveVolumes(); ++s) {
        MString SenName = 
          Detector->GetSensitiveVolume(s)->GetName() 
          + "Log";

        G4LogicalVolumeStore* SS = G4LogicalVolumeStore::GetInstance();
        for (unsigned s = 0; s < SS->size(); ++s) {
          if (SenName == SS->at(s)->GetName().c_str()) {
            SS->at(s)->SetSensitiveDetector(S);
          }
        }
      }

    } else {
      merr<<"DetectorType not yet implemented ("
          <<Detector->GetName()<<")!"<<endl;
      return false;
    }

    EventAction->SetCollectionName(Name.Data(), Type);

    // Add general information about the detector structure:
    Name = Detector->GetDetectorVolume()->GetName() + "Log";
    SD->SetDetectorVolumeName(Name.Data()); 
    Vector = G4ThreeVector(2*Detector->GetStructuralSize().X()*cm,
                           2*Detector->GetStructuralSize().Y()*cm,
                           2*Detector->GetStructuralSize().Z()*cm);
    SD->SetDetectorStructuralSize(Vector);
    Vector = G4ThreeVector(Detector->GetStructuralPitch().X()*cm,
                           Detector->GetStructuralPitch().Y()*cm,
                           Detector->GetStructuralPitch().Z()*cm);
    SD->SetDetectorStructuralPitch(Vector);
    Vector = G4ThreeVector(Detector->GetStructuralOffset().X()*cm,
                           Detector->GetStructuralOffset().Y()*cm,
                           Detector->GetStructuralOffset().Z()*cm);
    SD->SetDetectorStructuralOffset(Vector);

    MDShapeBRIK* Brik = (MDShapeBRIK*) (Detector->GetDetectorVolume()->GetShape());
    Vector = G4ThreeVector(Brik->GetSizeX()*cm,
                           Brik->GetSizeY()*cm,
                           Brik->GetSizeZ()*cm);
    SD->SetDetectorStructuralDimension(Vector);
    SD->SetDiscretizeHits(m_RunParameters.DiscretizeHits());

    if (Detector->GetEnergyLossType() == MDDetector::c_EnergyLossTypeMap) {
      SD->SetEnergyLossMap(Detector->GetEnergyLossMap());
    }

    SD->SetHasTimeResolution(Detector->HasTimeResolution());
  }

  return true;
}


/******************************************************************************
 * Create volumes
 */
bool MCDetectorConstruction::ConstructVolumes()
{
  MString Type; 
  MString VolumeName;
  MString MaterialName;
  MString LogName;
  G4VSolid* Solid = 0;

  for (unsigned int v = 0; v < m_Geometry->GetNVolumes(); ++v) {

    if (m_Geometry->GetVolumeAt(v)->GetCloneTemplate() == 0) {
      
      Type = m_Geometry->GetVolumeAt(v)->GetShape()->GetType();
      VolumeName = m_Geometry->GetVolumeAt(v)->GetName();
      MaterialName = m_Geometry->GetVolumeAt(v)->GetMaterial()->GetName();
      LogName = VolumeName + "Log";

      if (Type == "BRIK") {
        MDShapeBRIK* BRIK = 
          (MDShapeBRIK*) m_Geometry->GetVolumeAt(v)->GetShape();
        Solid = 
          new G4Box((VolumeName + "BOX").Data(), 
                    BRIK->GetSizeX()*cm, 
                    BRIK->GetSizeY()*cm, 
                    BRIK->GetSizeZ()*cm);
      } else if (Type == "TUBS") {
        MDShapeTUBS* TUBS = 
          (MDShapeTUBS*) m_Geometry->GetVolumeAt(v)->GetShape();
        Solid = 
          new G4Tubs((VolumeName + "TUBS").Data(), 
                     TUBS->GetRmin()*cm, 
                     TUBS->GetRmax()*cm, 
                     TUBS->GetHeight()*cm,
                     TUBS->GetPhi1()*deg,
                     (TUBS->GetPhi2()-TUBS->GetPhi1())*deg);
      } else if (Type == "SPHE") {
        MDShapeSPHE* SPHE = 
          (MDShapeSPHE*) m_Geometry->GetVolumeAt(v)->GetShape();
        Solid = 
          new G4Sphere((VolumeName + "SPHE").Data(), 
                       SPHE->GetRmin()*cm, 
                       SPHE->GetRmax()*cm, 
                       SPHE->GetPhimin()*deg,
                       (SPHE->GetPhimax()-SPHE->GetPhimin())*deg,
                       SPHE->GetThetamin()*deg,
                       (SPHE->GetThetamax()-SPHE->GetThetamin())*deg);
      } else if (Type == "TRD1") {
        MDShapeTRD1* TRD1 = 
          (MDShapeTRD1*) m_Geometry->GetVolumeAt(v)->GetShape();
        Solid = 
          new G4Trd((VolumeName + "TRD1").Data(), 
                    TRD1->GetDx1()*cm, 
                    TRD1->GetDx2()*cm, 
                    TRD1->GetY()*cm,
                    TRD1->GetY()*cm,
                    TRD1->GetZ()*cm);
      } else if (Type == "CONE") {
        MDShapeCONE* CONE = 
          (MDShapeCONE*) m_Geometry->GetVolumeAt(v)->GetShape();
        Solid = 
          new G4Cons((VolumeName + "CONE").Data(), 
                     CONE->GetRminBottom()*cm, 
                     CONE->GetRmaxBottom()*cm, 
                     CONE->GetRminTop()*cm,
                     CONE->GetRmaxTop()*cm, 
                     CONE->GetHalfHeight()*cm, 
                     0.0*deg, 360.0*deg);
      } else if (Type == "CONS") {
        MDShapeCONS* CONS = 
          (MDShapeCONS*) m_Geometry->GetVolumeAt(v)->GetShape();
        Solid = 
          new G4Cons((VolumeName + "CONS").Data(), 
                     CONS->GetRminBottom()*cm, 
                     CONS->GetRmaxBottom()*cm, 
                     CONS->GetRminTop()*cm,
                     CONS->GetRmaxTop()*cm, 
                     CONS->GetHalfHeight()*cm, 
                     CONS->GetPhiMin()*deg, CONS->GetPhiMax()*deg);
      } else if (Type == "PCON") {
        MDShapePCON* PCON = 
          (MDShapePCON*) m_Geometry->GetVolumeAt(v)->GetShape();
        double* z = new double[PCON->GetNSections()];
        double* rmin = new double[PCON->GetNSections()];
        double* rmax = new double[PCON->GetNSections()];
        for (unsigned int i = 0; i < PCON->GetNSections(); ++i) {
          z[i] = PCON->GetZ(i)*cm;
          rmin[i] = PCON->GetRmin(i)*cm;
          rmax[i] = PCON->GetRmax(i)*cm;
        }
        Solid = 
          new G4Polycone((VolumeName + "PCON").Data(), 
                         PCON->GetPhi()*deg, 
                         PCON->GetDPhi()*deg, 
                         PCON->GetNSections(),
                         z,
                         rmin,
                         rmax);
      } else if (Type == "PGON") {
        MDShapePGON* PGON =
          (MDShapePGON*) m_Geometry->GetVolumeAt(v)->GetShape();
        double* z = new double[PGON->GetNSections()];
        double* rmin = new double[PGON->GetNSections()];
        double* rmax = new double[PGON->GetNSections()];
        for (unsigned int i = 0; i < PGON->GetNSections(); ++i) {
          z[i] = PGON->GetZ(i)*cm;
          rmin[i] = PGON->GetRmin(i)*cm;
          rmax[i] = PGON->GetRmax(i)*cm;
        }
        Solid =
          new G4Polyhedra((VolumeName + "PGON").Data(),
              PGON->GetPhi()*deg,
              PGON->GetDPhi()*deg,
              PGON->GetNSides(),
              PGON->GetNSections(),
              z,
              rmin,
              rmax);
      } else if (Type == "TRAP") {
        MDShapeTRAP* TRAP = 
          (MDShapeTRAP*) m_Geometry->GetVolumeAt(v)->GetShape();
        // For Geant4 no value is allowed to be zero:
        double dz = TRAP->GetDz()*cm;
        if (dz <= 0) dz = 1.0E-10;  
        double theta = TRAP->GetTheta()*deg;
        double phi = TRAP->GetPhi()*deg;
        double h1 = TRAP->GetH1()*cm;
        double bl1 = TRAP->GetBl1()*cm;
        double tl1 = TRAP->GetTl1()*cm;
        double a1 = TRAP->GetAlpha1()*deg;
        double h2 = TRAP->GetH2()*cm;
        double bl2 = TRAP->GetBl2()*cm;
        double tl2 = TRAP->GetTl2()*cm;
        double a2 = TRAP->GetAlpha2()*deg;
        // Geant4 does not accept zeros...
        // So we need some kind of logic, that if we add very small values,
        // that everything still is planar...
        // This does not take into account each possible  case...

        int NZeros = 0;
        if (h1 <= 0) NZeros++;  
        if (bl1 <= 0) NZeros++;  
        if (tl1 <= 0) NZeros++;  
        if (h2 <= 0) NZeros++;  
        if (bl2 <= 0) NZeros++;  
        if (tl2 <= 0) NZeros++;  
        
        if (NZeros != 0) {
          mout<<endl;
          mout<<"SEVERE WARNING!"<<endl;
          mout<<"... for Shape TRAP:"<<endl;
          mout<<"One of the parameters is zero & Geant4 does not allow this!"<<endl;
          mout<<"Trying to estimate other parameters:"<<endl;
          mout<<"Start: "<<endl;
          mout<<"h1: "<<h1<<"  bl1: "<<bl1<<"  tl1: "<<tl1<<endl;
          mout<<"h2: "<<h2<<"  bl2: "<<bl2<<"  tl2: "<<tl2<<endl;

          const double Exp = 1E-10;
          if (NZeros == 1) {
            if (h1 <= 0) h1 = Exp*cm;  
            if (bl1 <= 0) bl1 = Exp*cm;  
            if (tl1 <= 0) tl1 = Exp*cm;  
            if (h2 <= 0) h2 = Exp*cm;  
            if (bl2 <= 0) bl2 = Exp*cm;  
            if (tl2 <= 0) tl2 = Exp*cm;  
          } else {
            double Ratio = 0.0;
            if (h1 != 0 && h2 != 0) Ratio = h1/h2;
            else if (tl1 != 0 && tl2 != 0) Ratio = tl1/tl2;
            else if (bl1 != 0 && bl2 != 0) Ratio = tl1/tl2;
            
            if (Ratio == 0.0) {
              if (h1 != 0) Ratio = 1/Exp;
              else Ratio = Exp;
            }

            if (h1 != 0 && h2 == 0) h2=h1/Ratio;
            if (h1 == 0 && h2 != 0) h1=h2*Ratio;
            
            if (tl1 == 0 && tl2 == 0) {
              if (h1 > h2) tl1 = Exp*cm;
              else tl2 = Exp*cm;
            }
            if (tl1 != 0 && tl2 == 0) tl2=tl1/Ratio;
            if (tl1 == 0 && tl2 != 0) tl1=tl2*Ratio;
            
            if (bl1 == 0 && bl2 == 0) {
              if (h1 > h2) bl1 = Exp*cm;
              else bl2 = Exp*cm;
            }
            if (bl1 != 0 && bl2 == 0) bl2=bl1/Ratio;
            if (bl1 == 0 && bl2 != 0) bl1=bl2*Ratio;
          }
          
          // If still something is missing
          if (h1 <= 0) h1 = 1.0E-6*cm;  
          if (bl1 <= 0) bl1 = 1.0E-6*cm;  
          if (tl1 <= 0) tl1 = 1.0E-6*cm;  
          if (h2 <= 0) h2 = 1.0E-6*cm;  
          if (bl2 <= 0) bl2 = 1.0E-6*cm;  
          if (tl2 <= 0) tl2 = 1.0E-6*cm;  
         
          mout<<"Final:"<<endl;
          mout<<"h1: "<<h1<<"  bl1: "<<bl1<<"  tl1: "<<tl1<<endl;
          mout<<"h2: "<<h2<<"  bl2: "<<bl2<<"  tl2: "<<tl2<<endl;
          mout<<"This estimation is a very risky thing..."<<endl;
          mout<<"Better correct your geometry by yourself!!!!"<<endl;
          mout<<"END SEVERE WARNING!"<<endl;
          mout<<endl;
        } 

        Solid = 
          new G4Trap((VolumeName + "TRAP").Data(), 
                     dz, 
                     theta, 
                     phi,
                     h1,
                     bl1,
                     tl1,
                     a1,
                     h2,
                     bl2,
                     tl2,
                     a2);
      } else {
        merr<<"Unknown volume type: "<<Type<<endl;
        return false;
      }
      G4LogicalVolume* LV = 
        new G4LogicalVolume(Solid, 
                            G4Material::GetMaterial(MaterialName.Data()), 
                            LogName.Data(), 0, 0, 0);
      G4VisAttributes* Vis = new G4VisAttributes();
      if (m_Geometry->GetVolumeAt(v)->GetVisibility() == 0) {
        Vis->SetVisibility(false);
      } else {
         Vis->SetVisibility(true);       
      }
      // Convert ROOT color to Geant4 color
      TColor* C = gROOT->GetColor(m_Geometry->GetVolumeAt(v)->GetColor());
      if (C != 0) {
        Vis->SetColor(G4Color(C->GetRed(), C->GetGreen(), C->GetBlue(), C->GetAlpha()));
      } else {
        merr<<"ROOT colors not initialized..."<<endl;
      }
      LV->SetVisAttributes(Vis);
      mdebug<<"Generating Log: "<<LogName.Data()<<" mat="
            <<MaterialName.Data()<<endl;
    } // no clone 
  }

  return true;
}


/******************************************************************************
 * Create materials
 */
bool MCDetectorConstruction::ConstructMaterials()
{
  mdebug<<"Defining materials..."<<endl;

  if (m_Geometry->IsScanned() == false) return false;
 
  G4NistElementBuilder ElementBuilder(0);
  
  for (unsigned int m = 0; m < m_Geometry->GetNMaterials(); ++m) {
    G4Material* Material = 
      new G4Material(m_Geometry->GetMaterialAt(m)->GetName().Data(), 
                     m_Geometry->GetMaterialAt(m)->GetDensity()*g/cm3,
                     m_Geometry->GetMaterialAt(m)->GetNComponents());
    
    
    for (unsigned int c = 0; 
         c < m_Geometry->GetMaterialAt(m)->GetNComponents(); ++c) {
      MDMaterialComponent* Component = 
        m_Geometry->GetMaterialAt(m)->GetComponentAt(c);

      ostringstream LongName;
      LongName<<"LongName_"<<m_Geometry->GetMaterialAt(m)->GetName().Data()
              <<"_El"<<c+1<<endl;

      ostringstream ShortName;
      ShortName<<m_Geometry->GetMaterialAt(m)->GetName().Data()
               <<"_El"<<c+1<<endl;

      double A = Component->GetA();
      double Z = Component->GetZ();

      if (A < 1 && Z < 1) {
        mout<<m_Geometry->GetMaterialAt(m)->GetName().Data()<<": Probably found Geant3 vaccum: upgrading to Geant4 vacuum"<<endl; 
        A = 1;
        Z = 1;
      }

      G4Element* Element = 0;
      if (Component->HasNaturalIsotopeComposition() == true) {
        Element = ElementBuilder.FindOrBuildElement(Z, true);
      } else {
        Element =  new G4Element(LongName.str(), ShortName.str(), Z, A*g/mole);
      }
      
      if (Element == 0) {
        merr<<"Couldn't find all elements of: "<<m_Geometry->GetMaterialAt(m)->GetName()<<" Missing: Z="<<Z<<endl;
        return false;
      }
      
      if (Component->GetType() == MDMaterialComponent::c_ByAtoms) {
        Material->AddElement(Element, TMath::Nint(Component->GetWeight()));
      } else {
        Material->AddElement(Element, double(Component->GetWeight()));
      }
    }
    mdebug<<Material<<endl;
  }

  return true;
}


/******************************************************************************
 * Create regions
 */
bool MCDetectorConstruction::ConstructRegions()
{
  mdebug<<"Defining regions..."<<endl;

  if (m_Geometry->IsScanned() == false) return false;
 
  const vector<MCRegion>& Regions = m_RunParameters.GetRegionList();

  G4LogicalVolumeStore* LVS = G4LogicalVolumeStore::GetInstance();
  
  // Loop over all regions from the input file and define them 
  for (unsigned int r = 0; r < Regions.size(); ++r) {
    // Check if volume exists in the geomega volume tree
    if (m_Geometry->GetWorldVolume()->ContainsVolume(Regions[r].GetVolumeName(), true) == true) {
      MString Name = Regions[r].GetVolumeName() + "Log";
      bool Found = false;
      // Find the volume in the logical volume store
      for (unsigned s = 0; s < LVS->size(); ++s) {
        if (Name == LVS->at(s)->GetName().c_str()) {
          // Define region. The cuts are set in the physics list
          G4Region* Region = new G4Region(Regions[r].GetName().Data());
          Region->AddRootLogicalVolume(LVS->at(s));
          Found = true;
          break;
        }
      }
      if (Found == false) {
        mout<<"Volume "<<Regions[r].GetVolumeName()<<" defining region ";
        mout<<Regions[r].GetName()<<" is not found in the LogicalVolumeStore."<<endl;
        mout<<"Available volumes are: ";
        for (unsigned s = 0; s < LVS->size(); ++s) mout<<LVS->at(s)->GetName().c_str()<<" ";
        mout<<endl;
        return false;
      }
    } else {
      mout<<"Volume "<<Regions[r].GetVolumeName()<<" defining region ";
      mout<<Regions[r].GetName()<<" is not part of the world volume."<<endl;
      mout<<"Make sure that the spelling is correct and that the volume is not a virtual volume."<<endl;
      mout<<"If the geometry contains virtual volumes, the names of the volume might have changed."<<endl;
      mout<<"In this case use geomega to determine its new name."<<endl;
      return false;
    }
  }

  return true;
}


/******************************************************************************
 * Recursively position all volumes - in the it smother volumes
 */
bool MCDetectorConstruction::PositionVolumes(MDVolume* Volume)
{
  // This is far too complex to REALLY understand without investing time ...
  // But since it works: "Don't f&*k with it!" 

  MDVolume* VC;
  MString Name, MotherName, CopyName;

  if ((VC = Volume->GetCloneTemplate()) != 0) {
      
    // If the volume is a copy, then reposition the CopyOf volume:
    
    Name = Volume->GetName();
    CopyName = Volume->GetCloneTemplate()->GetName() + "Log";
    MotherName = Volume->GetMother()->GetName() + "Log";
 

    G4LogicalVolume* ThisLog = 0;
    G4LogicalVolume* MotherLog = 0;

    G4LogicalVolumeStore* SS = G4LogicalVolumeStore::GetInstance();
    for (unsigned s = 0; s < SS->size(); ++s) {
      if (ThisLog == 0 && CopyName == SS->at(s)->GetName().c_str()) {
        ThisLog = SS->at(s);
      }
      if (MotherLog == 0 && MotherName == SS->at(s)->GetName().c_str()) {
        MotherLog = SS->at(s);
      }
      if (ThisLog != 0 && MotherLog != 0) break;
    }

    if (ThisLog == 0) {
      mout<<"Unable to find copy-log: "<<CopyName<<endl;
      return false;
    }

    if (MotherLog == 0) {
      mout<<"Unable to find mother-log: "<<MotherName<<endl;
      return false;
    }
    G4PVPlacement* P = 
      new G4PVPlacement(CreateRotation(Volume), 
                        G4ThreeVector(Volume->GetPosition().X()*cm, 
                                      Volume->GetPosition().Y()*cm, 
                                      Volume->GetPosition().Z()*cm), 
                        ThisLog, 
                        Volume->GetName().Data(), 
                        MotherLog, 
                        false, 0);
    if (m_RunParameters.PerformOverlapCheck() == true) {
      P->CheckOverlaps(m_RunParameters.ResolutionOverlapCheck(), m_RunParameters.ToleranceOverlapCheck(), false);
    }

    mdebug<<"Positioning clone: "<<CopyName<<" in "<<MotherName
          <<" at ("<<Volume->GetPosition().X()<<","
          <<Volume->GetPosition().Y()<<","<<Volume->GetPosition().Z()
          <<") with Name "<<Volume->GetName().Data()<<endl;
    
    if (Volume->GetCloneTemplate()->AreCloneTemplateDaughtersWritten() == 
        kFALSE) {
      for (unsigned int i = 0; i < Volume->GetNDaughters(); i++) {
        if (PositionVolumes(Volume->GetDaughterAt(i)) == false) return false;
      }
      Volume->GetCloneTemplate()->SetCloneTemplateDaughtersWritten(kTRUE);
    }
  } else {
    // If it is no copy then position it *only*
    // when it has a mother or is the root volume
    if (Volume->GetMother() !=  0 || Volume->IsWorldVolume() == true) {
 			
      Name = Volume->GetName() + "Log";
      if (Volume->IsWorldVolume() == false) {
        MotherName = Volume->GetMother()->GetName() + "Log";
			}

      G4LogicalVolume* ThisLog = 0;
      G4LogicalVolume* MotherLog = 0;

      G4LogicalVolumeStore* SS = G4LogicalVolumeStore::GetInstance();
      for (unsigned s = 0; s < SS->size(); ++s) {
        if (ThisLog == 0 && Name == SS->at(s)->GetName().c_str()) {
          ThisLog = SS->at(s);
        }
        if (MotherLog == 0 && MotherName == SS->at(s)->GetName().c_str()) {
          MotherLog = SS->at(s);
        }
        if (ThisLog != 0 && MotherLog != 0) break;
      }

      if (ThisLog == 0) {
        merr<<"Unable to find this-log: "<<Name<<endl;
        return false;
      }
      
      if (MotherLog == 0 && Volume->IsWorldVolume() == false) {
        merr<<"Unable to find mother-log: "<<MotherName<<endl;
        return false;
      }


      if (Volume->IsWorldVolume() == true) {
        m_WorldVolume = 
          new G4PVPlacement(0, 
                            G4ThreeVector(Volume->GetPosition().X()*cm, 
                                          Volume->GetPosition().Y()*cm, 
                                          Volume->GetPosition().Z()*cm), 
                            Volume->GetName().Data(), ThisLog, 0, false, 0);
        if (m_RunParameters.PerformOverlapCheck() == true) {
          m_WorldVolume->CheckOverlaps(m_RunParameters.ResolutionOverlapCheck(), m_RunParameters.ToleranceOverlapCheck(), false);
        }
      } else {
        G4PVPlacement* P = 
          new G4PVPlacement(CreateRotation(Volume),
                            G4ThreeVector(Volume->GetPosition().X()*cm, 
                                          Volume->GetPosition().Y()*cm, 
                                          Volume->GetPosition().Z()*cm), 
                            ThisLog, 
                            Volume->GetName().Data(), 
                            MotherLog, 
                            false, 0);
        if (m_RunParameters.PerformOverlapCheck() == true) {
          P->CheckOverlaps(m_RunParameters.ResolutionOverlapCheck(), m_RunParameters.ToleranceOverlapCheck(), false);
        }
      }

      mdebug<<"Positioning original: "<<Name<<" in "<<MotherName
            <<" at ("<<Volume->GetPosition().X()<<","<<Volume->GetPosition().Y()
            <<","<<Volume->GetPosition().Z()<<") with Name "
            <<Volume->GetName().Data()<<endl;

    } else {
      //cout<<"   is ignored..."<<endl;
    }

    // Do the same for all daughters:
    for (unsigned int i = 0; i < Volume->GetNDaughters(); i++) {
     if (PositionVolumes(Volume->GetDaughterAt(i)) == false) return false;
    }
  }
	
  return true;
}


/******************************************************************************
 * Return a random position in the given volume
 */
G4ThreeVector MCDetectorConstruction::GetRandomPosition(MString VolumeName)
{
  MVector V = m_Geometry->GetRandomPositionInVolume(VolumeName);

  return G4ThreeVector(V[0]*cm, V[1]*cm, V[2]*cm);
}


/******************************************************************************
 * Return true if the volume is valid volume in the geometry
 */
bool MCDetectorConstruction::IsValidVolume(MString VolumeName)
{
  // No clue why I need a cast here....
  if (dynamic_cast<MDGeometry*>(m_Geometry)->GetVolume(VolumeName) == 0) {
    return false;
  }

  return true;
}


/******************************************************************************
 * Return the (Geomega) hash of the given material 
 */
unsigned long MCDetectorConstruction::GetMaterialHash(const G4Material* Material)
{
  // No clue why I need a cast here....
  MDMaterial* M = dynamic_cast<MDGeometry*>(m_Geometry)->GetMaterial(Material->GetName());
  if (M == 0) {
    merr<<"Material "<<Material->GetName()<<" not found in Geomega geometry!"<<show;
    return 0;
  }

  return M->GetHash();
}


/*
 * MCDetectorConstruction.cc: the end...
 ******************************************************************************/
