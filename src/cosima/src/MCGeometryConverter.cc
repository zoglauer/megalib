/*
 * MCGeometryConverter.cxx
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
#include "MCGeometryConverter.hh"

// MEGAlib:

// Geant4:
#include "G4SystemOfUnits.hh"
#include "globals.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4VSolid.hh"
#include "G4Box.hh"
#include "G4Sphere.hh"
#include "G4Tubs.hh"
#include "G4Trd.hh"
#include "G4Trap.hh"
#include "G4Polyhedra.hh"
#include "G4Polycone.hh"
#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4Element.hh"

// Standard lib:
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
using namespace std;


/******************************************************************************
 * Default constructor for the Converter from Geant4 to MEGAlib
 * Simply add this class to your Geant4 program
 * Then call it after the geometry is completely initialized:
 *   MCGeometryConverter* C = new MCGeometryConverter();
 *   C->Convert();
 */
MCGeometryConverter::MCGeometryConverter()
{
  // Intentionally left blank
}


/******************************************************************************
 * default destructor
 */
MCGeometryConverter::~MCGeometryConverter()
{
  // Intentionally left blank
}

/******************************************************************************
 * Perfrom the conversion from Geant4 to MEGAlib
 */
bool MCGeometryConverter::Convert(MString OutputfileName) 
{
  ofstream fout;
  fout.open(OutputfileName);

  fout<<"# This geometry has been converted from Geant4 to MEGAlib"<<endl;
  fout<<"# Please check it carefully for any errors and "<<endl;
  fout<<"# implement the detector description as necessary."<<endl;
  fout<<endl;
  fout<<endl;
  fout<<"Name NameTbd"<<endl;
  fout<<endl;
  fout<<"# Please adapt this surround sphere, so that it encloses your geometry"<<endl;
  fout<<"SurroundingSphere 10000.0 0.0 0.0 0.0 10000.0"<<endl;
  fout<<endl;

  // Start with the materials
  fout<<"# Material descriptions: "<<endl;
  
  const G4MaterialTable* T = G4Material::GetMaterialTable();
  
  for (unsigned int i = 0; i < T->size(); ++i) {
    G4Material* M = T->at(i);
    MString Name = CreateUniqueName(M);
    fout<<"Material "<<Name<<endl;
    fout<<Name<<".Density "<<M->GetDensity()*cm3/g<<endl;
    for (unsigned int e = 0; e < M->GetNumberOfElements(); ++e) {
      const G4Element* E = M->GetElement(e);
      fout<<Name<<".ComponentByAtoms "<<E->GetN()<<" "<<E->GetZ()<<" "<<E->GetA()*mole/g<<endl;
    }
    fout<<endl;
  }

  // Start with the volumes:
  fout<<endl;
  fout<<"# Logical volume descriptions: "<<endl;

  G4LogicalVolumeStore* L = G4LogicalVolumeStore::GetInstance();

  for (unsigned int i = 0; i < L->size(); ++i) {
    G4LogicalVolume* V = L->at(i);
    MString Name = CreateUniqueName(V);
    fout<<"Volume "<<Name<<endl;
    fout<<Name<<".Material "<<m_UniqueMaterialNames[V->GetMaterial()]<<endl;

    G4VSolid* S = V->GetSolid();
    if (S->GetEntityType() == "G4Box") {
      G4Box* Box = (G4Box*) S;
      fout<<Name<<".Shape BOX "<<Box->GetXHalfLength()/cm<<" "<<Box->GetYHalfLength()/cm<<" "<<Box->GetZHalfLength()/cm<<endl;
    } else if (S->GetEntityType() == "G4Sphere") {
      G4Sphere* Sphere = (G4Sphere*) S;
      fout<<Name<<".Shape SPHE "
          <<Sphere->GetInsideRadius()/cm<<" "
          <<Sphere->GetOuterRadius()/cm<<" "
          <<Sphere->GetStartThetaAngle()/deg<<" "
          <<Sphere->GetStartThetaAngle()/deg + Sphere->GetDeltaThetaAngle()/deg<<" "
          <<Sphere->GetStartPhiAngle()/deg<<" "
          <<Sphere->GetStartPhiAngle()/deg + Sphere->GetDeltaPhiAngle()/deg<<endl;
    } else if (S->GetEntityType() == "G4Tubs") {
      G4Tubs* Tubs = (G4Tubs*) S;
      fout<<Name<<".Shape TUBS "
          <<Tubs->GetInnerRadius()/cm<<" "
          <<Tubs->GetOuterRadius()/cm<<" "
          <<Tubs->GetZHalfLength()/cm<<" "
          <<Tubs->GetStartPhiAngle()/deg<<" "
          <<Tubs->GetStartPhiAngle()/deg + Tubs->GetDeltaPhiAngle()/deg<<endl;
    } else if (S->GetEntityType() == "G4Trd") {
      G4Trd* Trd = (G4Trd*) S;
      fout<<Name<<".Shape TRD2 "
          <<Trd->GetXHalfLength1()/cm<<" "
          <<Trd->GetXHalfLength2()/cm<<" "
          <<Trd->GetYHalfLength1()/cm<<" "
          <<Trd->GetYHalfLength2()/cm<<" "
          <<Trd->GetZHalfLength()/cm<<endl;
    } else if (S->GetEntityType() == "G4Trap") {
      G4Trap* Trap = (G4Trap*) S;

      // Unfortunately Geant4 does not provide theta and phi, so we have to calculate it from what it provides:
      G4ThreeVector SymAxis = Trap->GetSymAxis();
      double theta = acos(SymAxis.getZ());
      double phi1 = 0.0;
      double phi2 = 0.0;
      if (theta > 0.0 && theta < CLHEP::pi) {
        phi1 = acos(SymAxis.getX()/SymAxis.getZ()/tan(theta));
        phi2 = asin(SymAxis.getY()/SymAxis.getZ()/tan(theta));
        if (fabs(phi1-phi2) > 1e-5) {
          fout<<"Echo Error for volume "<<Name<<": G4Trap probably wrong: phi1="<<phi1<<" phi2="<<phi2<<endl;
        } 
      }
      fout<<setprecision(12);
      fout<<Name<<".Shape TRAP "
          <<Trap->GetZHalfLength()/cm<<" "
          <<theta/deg<<" "
          <<phi1/deg<<" "
          <<Trap->GetYHalfLength1()/cm<<" "
          <<Trap->GetXHalfLength1()/cm<<" "
          <<Trap->GetXHalfLength2()/cm<<" "
          <<atan(Trap->GetTanAlpha1())/deg<<" "
          <<Trap->GetYHalfLength2()/cm<<" "
          <<Trap->GetXHalfLength3()/cm<<" "
          <<Trap->GetXHalfLength4()/cm<<" "
          <<atan(Trap->GetTanAlpha2())/deg<<endl;
      fout<<setprecision(6);
    } else if (S->GetEntityType() == "G4Polyhedra") {
      G4Polyhedra* Polyhedra = (G4Polyhedra*) S;
      G4PolyhedraHistorical* Hist = Polyhedra->GetOriginalParameters();
      fout<<Name<<".Shape PGON "
          <<Hist->Start_angle/deg<<" "
          <<Hist->Opening_angle/deg<<" "
          <<Hist->numSide<<" "
          <<Hist->Num_z_planes<<" ";
      for (int j = 0; j < Hist->Num_z_planes; ++j) {
        fout<<Hist->Z_values[j]/cm<<" "
            <<Hist->Rmin[j]/cm<<" "
            <<Hist->Rmax[j]/cm<<" ";
      }
      fout<<endl;
    } else if (S->GetEntityType() == "G4Polycone") {
      G4Polycone* Polycone = (G4Polycone*) S;
      G4PolyconeHistorical* Hist = Polycone->GetOriginalParameters();
      fout<<Name<<".Shape PCON "
          <<Hist->Start_angle/deg<<" "
          <<Hist->Opening_angle/deg<<" "
          <<Hist->Num_z_planes<<" ";
      for (int j = 0; j < Hist->Num_z_planes; ++j) {
        fout<<Hist->Z_values[j]/cm<<" "
            <<Hist->Rmin[j]/cm<<" "
            <<Hist->Rmax[j]/cm<<" ";
      }
      fout<<endl;
    } else {
      fout<<"Print Error for volume "<<Name<<": Unknown shape: "<<S->GetEntityType()<<endl;
      fout<<Name<<".Shape BOX 100.0 100.0 100.0"<<endl;
      fout<<Name<<".Virtual true"<<endl;
    }
    fout<<endl;
  }

  // Start with the volumes:
  fout<<endl;
  fout<<"# Physical volume descriptions: "<<endl;

  G4PhysicalVolumeStore* S = G4PhysicalVolumeStore::GetInstance();

  for (unsigned int i = 0; i < S->size(); ++i) {
    G4VPhysicalVolume* V = S->at(i);
    MString Name = CreateUniqueName(V);
    fout<<m_UniqueLogicalVolumeNames[V->GetLogicalVolume()]<<".Copy "<<Name<<endl;
    fout<<setprecision(12);
    fout<<Name<<".Rotation "
        <<Zeroing(V->GetObjectRotationValue().thetaX()/deg)<<" "<<Zeroing(V->GetObjectRotationValue().phiX()/deg)<<" "
        <<Zeroing(V->GetObjectRotationValue().thetaY()/deg)<<" "<<Zeroing(V->GetObjectRotationValue().phiY()/deg)<<" "
        <<Zeroing(V->GetObjectRotationValue().thetaZ()/deg)<<" "<<Zeroing(V->GetObjectRotationValue().phiZ()/deg)<<endl;
    fout<<Name<<".Position "<<V->GetObjectTranslation().getX()/cm<<"  "<<V->GetObjectTranslation().getY()/cm<<"  "<<V->GetObjectTranslation().getZ()/cm<<endl;
    fout<<setprecision(6);
    if (V->GetMotherLogical() != 0) {
      fout<<Name<<".Mother "<<m_UniqueLogicalVolumeNames[V->GetMotherLogical()]<<endl;
    } else {
      fout<<Name<<".Mother 0"<<endl;
    }
    fout<<endl;
  }

  fout<<endl;
  fout<<"# Detector descriptions: "<<endl;
  fout<<"# Dummy still missing!"<<endl;

  fout.close();

  return true;
}


/******************************************************************************
 * Convert very small numberes to zero
 */
double MCGeometryConverter::Zeroing(double Number) 
{
  if (fabs(Number) < 1E-10) {
    return 0.0;
  }

  return Number;
}


/******************************************************************************
 * Create an unique material name:
 */
MString MCGeometryConverter::CreateUniqueName(G4Material* M)
{
  int Index = 0;
  MString BaseName = MakeValidName(M->GetName());

  ostringstream outt;
  outt<<"m"<<BaseName<<"_"<<Index;
  MString UniqueName = outt.str();

  bool Found;
  do {
    Found = false;
    for (map<G4Material*, MString>::iterator Iter = m_UniqueMaterialNames.begin();
	 Iter != m_UniqueMaterialNames.end();
	 ++Iter) {
      if ((*Iter).second == UniqueName) {
	Found = true;
	Index++;
	ostringstream out;
	out<<"m"<<BaseName<<"_"<<Index;
	UniqueName = out.str();
      }
    }
  } while (Found == true);

  m_UniqueMaterialNames[M] = UniqueName;

  return UniqueName;
}


/******************************************************************************
 * Create an unique logiocal volume name:
 */
MString MCGeometryConverter::CreateUniqueName(G4LogicalVolume* M)
{
  int Index = 0;
  MString BaseName = MakeValidName(M->GetName());

  ostringstream outt;
  outt<<"l"<<BaseName<<"_"<<Index;
  MString UniqueName = outt.str();

  bool Found;
  do {
    Found = false;
    for (map<G4LogicalVolume*, MString>::iterator Iter = m_UniqueLogicalVolumeNames.begin();
	 Iter != m_UniqueLogicalVolumeNames.end();
	 ++Iter) {
      if ((*Iter).second == UniqueName) {
	Found = true;
	Index++;
	ostringstream out;
	out<<"l"<<BaseName<<"_"<<Index;
	UniqueName = out.str();
      }
    }
  } while (Found == true);

  m_UniqueLogicalVolumeNames[M] = UniqueName;

  return UniqueName;

  return "";
}


/******************************************************************************
 * Create an unique physical volume name:
 */
MString MCGeometryConverter::CreateUniqueName(G4VPhysicalVolume* M)
{
  int Index = 0;
  MString BaseName = MakeValidName(M->GetName());

  ostringstream outt;
  outt<<"p"<<BaseName<<"_"<<Index;
  MString UniqueName = outt.str();

  bool Found;

  do {
    Found = false;
    for (map<G4VPhysicalVolume*, MString>::iterator Iter = m_UniquePhysicalVolumeNames.begin();
	 Iter != m_UniquePhysicalVolumeNames.end();
	 ++Iter) {
      if ((*Iter).second == UniqueName) {
	Found = true;
	Index++;
	ostringstream out;
	out<<"p"<<BaseName<<"_"<<Index;
	UniqueName = out.str();
      }
    }
  } while (Found == true);

  m_UniquePhysicalVolumeNames[M] = UniqueName;

  return UniqueName;
}


/******************************************************************************
 * Convert the name to MEGAlib conventions
 */
MString MCGeometryConverter::MakeValidName(MString Name)
{
  // Makes a valid name out of "Name"
  // If the returned string is empty this task was impossible

  MString ValidName;

  for (unsigned int i = 0; i < Name.Length(); ++i) {
    if (isalnum(Name[i]) != 0 || 
        Name[i] == '_' ||
        Name[i] == '-') {
      ValidName += Name[i];
    }
  }

  return ValidName;
}


/*
 * MCGeometryConverter.cc: the end...
 ******************************************************************************/
