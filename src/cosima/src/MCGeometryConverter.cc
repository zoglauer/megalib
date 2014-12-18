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
// --- Make sure you use only Geant4 and C++ classes

// MEGAlib:
// --- Make sure you use only Geant4 and C++ classes

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
#include "G4Cons.hh"
#include "G4Trd.hh"
#include "G4Trap.hh"
#include "G4Polyhedra.hh"
#include "G4Polycone.hh"
#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4Element.hh"
#include "G4DisplacedSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4IntersectionSolid.hh"

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
bool MCGeometryConverter::Convert(G4String OutputfileName) 
{
  ofstream fout;
  fout.open(OutputfileName);
  fout.precision(9);
  
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
  
  cout<<"Converting "<<T->size()<<" materials"<<endl;
  
  for (unsigned int i = 0; i < T->size(); ++i) {
    G4Material* M = T->at(i);
    G4String Name = CreateUniqueName(M);
    fout<<"Material "<<Name<<endl;
    fout<<Name<<".Density "<<M->GetDensity()*cm3/g<<endl;
    for (unsigned int e = 0; e < M->GetNumberOfElements(); ++e) {
      const G4Element* E = M->GetElement(e);
      fout<<Name<<".ComponentByMass "<<E->GetN()<<" "<<E->GetZ()<<" "<<M->GetFractionVector()[e]<<endl;
    }
    fout<<endl;
  }
  
  // Start with the volumes:
  fout<<endl;
  fout<<"# Logical volume descriptions: "<<endl;
  
  G4LogicalVolumeStore* L = G4LogicalVolumeStore::GetInstance();
  
  for (unsigned int i = 0; i < L->size(); ++i) {
    G4LogicalVolume* V = L->at(i);
    G4String Name = CreateUniqueName(V);
    fout<<"Volume "<<Name<<endl;
    fout<<Name<<".Material "<<m_UniqueMaterialNames[V->GetMaterial()]<<endl;
    
    ConvertShape(fout, Name, V->GetSolid());
    fout<<endl;
  }
  
  // Start with the volumes:
  fout<<endl;
  fout<<"# Physical volume descriptions: "<<endl;
  
  G4PhysicalVolumeStore* S = G4PhysicalVolumeStore::GetInstance();
  
  for (unsigned int i = 0; i < S->size(); ++i) {
    G4VPhysicalVolume* V = S->at(i);
    G4String Name = m_UniqueLogicalVolumeNames[V->GetLogicalVolume()];
    if (V->GetMotherLogical() != 0) {
      Name = CreateUniqueName(V);
      fout<<m_UniqueLogicalVolumeNames[V->GetLogicalVolume()]<<".Copy "<<Name<<endl;
    }
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
 * Convert the shape
 */
bool MCGeometryConverter::ConvertShape(ofstream& fout, G4String& Name, G4VSolid* S, bool PartOfVolume)
{
  G4String Type;
  if (S->GetEntityType() == "G4Box") {
    Type = "BOX";
  } else if (S->GetEntityType() == "G4Sphere") {
    Type = "SPHE";
  } else if (S->GetEntityType() == "G4Tubs") {
    Type = "TUBS";
  } else if (S->GetEntityType() == "G4Cons") {
    Type = "CONS";
  } else if (S->GetEntityType() == "G4Trd") {
    Type = "TRD2";
  } else if (S->GetEntityType() == "G4Trap") {
    Type = "TRAP";
  } else if (S->GetEntityType() == "G4Polyhedra") {
    Type = "PGON";
  } else if (S->GetEntityType() == "G4Polycone") {
    Type = "PCON";
  } else if (S->GetEntityType() == "G4SubtractionSolid") {
    Type = "Subtraction";
  } else if (S->GetEntityType() == "G4UnionSolid") {
    Type = "Union";
  } else if (S->GetEntityType() == "G4IntersectionSolid") {
    Type = "Intersection";
  } else {
    Type = "ERROR";
  }
  
  
  G4String Init;
  if (PartOfVolume == true) {
    Init = Name;
    Init += ".Shape ";
    Init += Type;
    Init += " ";
  } else {
    fout<<"Shape "<<Type<<" "<<Name<<endl; 
    Init = Name;
    Init += ".Parameters ";
  }
  
  if (S->GetEntityType() == "G4Box") {
    G4Box* Box = (G4Box*) S;
    fout<<Init<<Box->GetXHalfLength()/cm<<" "<<Box->GetYHalfLength()/cm<<" "<<Box->GetZHalfLength()/cm<<endl;
  } else if (S->GetEntityType() == "G4Sphere") {
    G4Sphere* Sphere = (G4Sphere*) S;
    fout<<Init
    <<Sphere->GetInsideRadius()/cm<<" "
    <<Sphere->GetOuterRadius()/cm<<" "
    <<Sphere->GetStartThetaAngle()/deg<<" "
    <<Sphere->GetStartThetaAngle()/deg + Sphere->GetDeltaThetaAngle()/deg<<" "
    <<Sphere->GetStartPhiAngle()/deg<<" "
    <<Sphere->GetStartPhiAngle()/deg + Sphere->GetDeltaPhiAngle()/deg<<endl;
  } else if (S->GetEntityType() == "G4Tubs") {
    G4Tubs* Tubs = (G4Tubs*) S;
    fout<<Init
    <<Tubs->GetInnerRadius()/cm<<" "
    <<Tubs->GetOuterRadius()/cm<<" "
    <<Tubs->GetZHalfLength()/cm<<" "
    <<Tubs->GetStartPhiAngle()/deg<<" "
    <<Tubs->GetStartPhiAngle()/deg + Tubs->GetDeltaPhiAngle()/deg<<endl;
  } else if (S->GetEntityType() == "G4Cons") {
    G4Cons* Cons = (G4Cons*) S;
    fout<<Init
    <<Cons->GetZHalfLength()/cm<<" "
    <<Cons->GetInnerRadiusMinusZ()/cm<<" "
    <<Cons->GetOuterRadiusMinusZ()/cm<<" "
    <<Cons->GetInnerRadiusPlusZ()/cm<<" "
    <<Cons->GetOuterRadiusPlusZ()/cm<<" "
    <<Cons->GetStartPhiAngle()/deg<<" "
    <<Cons->GetStartPhiAngle()/deg + Cons->GetDeltaPhiAngle()/deg<<endl;
  } else if (S->GetEntityType() == "G4Trd") {
    G4Trd* Trd = (G4Trd*) S;
    fout<<Init
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
    fout<<Init
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
    fout<<Init
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
    fout<<Init
    <<Hist->Start_angle/deg<<" "
    <<Hist->Opening_angle/deg<<" "
    <<Hist->Num_z_planes<<" ";
    for (int j = 0; j < Hist->Num_z_planes; ++j) {
      fout<<Hist->Z_values[j]/cm<<" "
      <<Hist->Rmin[j]/cm<<" "
      <<Hist->Rmax[j]/cm<<" ";
    }
    fout<<endl;
  } else if (S->GetEntityType() == "G4SubtractionSolid") {
    G4SubtractionSolid* Subtraction = (G4SubtractionSolid*) S;
    G4VSolid* A = Subtraction->GetConstituentSolid(0);
    G4String NameA = Name + "_SubtractionVolumeA";
    G4VSolid* B = Subtraction->GetConstituentSolid(1);
    G4String NameB = Name + "_SubtractionVolumeB";
    
    if (B->GetEntityType() == "G4DisplacedSolid") {
      G4DisplacedSolid* Displaced = (G4DisplacedSolid*) B;
      G4String OrientationNameB = NameB + "_Orientation";
      
      fout<<Init<<NameA<<" "<<NameB<<" "<<OrientationNameB<<endl;
      
      fout<<endl;
      ConvertOrientation(fout, OrientationNameB, Displaced->GetObjectTranslation(), Displaced->GetObjectRotation());
      fout<<endl;
      
      B = Displaced->GetConstituentMovedSolid();  
    } else {
      fout<<Init<<NameA<<" "<<NameB<<endl;
    }
    
    fout<<endl;
    ConvertShape(fout, NameA, A, false);
    fout<<endl;
    ConvertShape(fout, NameB, B, false);
    fout<<endl;
    
  } else if (S->GetEntityType() == "G4UnionSolid") {
    G4UnionSolid* Union = (G4UnionSolid*) S;
    G4VSolid* A = Union->GetConstituentSolid(0);
    G4String NameA = Name + "_UnionVolumeA";
    G4VSolid* B = Union->GetConstituentSolid(1);
    G4String NameB = Name + "_UnoinVolumeB";
    
    if (B->GetEntityType() == "G4DisplacedSolid") {
      G4DisplacedSolid* Displaced = (G4DisplacedSolid*) B;
      G4String OrientationNameB = NameB + "_Orientation";
      
      fout<<Init<<NameA<<" "<<NameB<<" "<<OrientationNameB<<endl;
      
      fout<<endl;
      ConvertOrientation(fout, OrientationNameB, Displaced->GetObjectTranslation(), Displaced->GetObjectRotation());
      fout<<endl;
      
      B = Displaced->GetConstituentMovedSolid();  
    } else {
      fout<<Init<<NameA<<" "<<NameB<<endl;
    }
    
    fout<<endl;
    ConvertShape(fout, NameA, A, false);
    fout<<endl;
    ConvertShape(fout, NameB, B, false);
    fout<<endl;
    
  } else if (S->GetEntityType() == "G4IntersectionSolid") {
    G4IntersectionSolid* Intersection = (G4IntersectionSolid*) S;
    G4VSolid* A = Intersection->GetConstituentSolid(0);
    G4String NameA = Name + "_IntersectionVolumeA";
    G4VSolid* B = Intersection->GetConstituentSolid(1);
    G4String NameB = Name + "_IntersectionVolumeB";
    
    if (B->GetEntityType() == "G4DisplacedSolid") {
      G4DisplacedSolid* Displaced = (G4DisplacedSolid*) B;
      G4String OrientationNameB = NameB + "_Orientation";
      
      fout<<Init<<NameA<<" "<<NameB<<" "<<OrientationNameB<<endl;
      
      fout<<endl;
      ConvertOrientation(fout, OrientationNameB, Displaced->GetObjectTranslation(), Displaced->GetObjectRotation());
      fout<<endl;
      
      B = Displaced->GetConstituentMovedSolid();  
    } else {
      fout<<Init<<NameA<<" "<<NameB<<endl;
    }
    
    fout<<endl;
    ConvertShape(fout, NameA, A, false);
    fout<<endl;
    ConvertShape(fout, NameB, B, false);
    fout<<endl;
    
  } else {
    fout<<"Print Error for volume "<<Name<<": Unknown shape: "<<S->GetEntityType()<<endl;
    fout<<Init<<"  BOX 100.0 100.0 100.0"<<endl;
    fout<<Name<<".Virtual true"<<endl;
  }
  
  
  return true;
}  


/******************************************************************************
 * Convert an orientation
 */
bool MCGeometryConverter::ConvertOrientation(ofstream& fout, G4String Name, G4ThreeVector T, G4RotationMatrix R)
{
  fout<<"Orientation "<<Name<<endl;
  fout<<Name<<".Position "<<T.x()/cm<<" "<<T.y()/cm<<" "<<T.z()/cm<<endl;
  fout<<Name<<".Rotation "<<R.thetaX()/deg<<" "<<R.phiX()/deg<<" "<<R.thetaY()/deg<<" "<<R.phiY()/deg<<" "<<R.thetaZ()/deg<<" "<<R.phiZ()/deg<<endl;
  
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
G4String MCGeometryConverter::CreateUniqueName(G4Material* M)
{
  int Index = 0;
  G4String BaseName = MakeValidName(M->GetName());
  
  ostringstream outt;
  outt<<"m"<<BaseName<<"_"<<Index;
  G4String UniqueName = outt.str();
  
  bool Found;
  do {
    Found = false;
    for (map<G4Material*, G4String>::iterator Iter = m_UniqueMaterialNames.begin();
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
G4String MCGeometryConverter::CreateUniqueName(G4LogicalVolume* M)
{
  int Index = 0;
  G4String BaseName = MakeValidName(M->GetName());
  
  ostringstream outt;
  outt<<"l"<<BaseName<<"_"<<Index;
  G4String UniqueName = outt.str();
  
  bool Found;
  do {
    Found = false;
    for (map<G4LogicalVolume*, G4String>::iterator Iter = m_UniqueLogicalVolumeNames.begin();
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
G4String MCGeometryConverter::CreateUniqueName(G4VPhysicalVolume* M)
{
  int Index = 0;
  G4String BaseName = MakeValidName(M->GetName());
  
  ostringstream outt;
  outt<<"p"<<BaseName<<"_"<<Index;
  G4String UniqueName = outt.str();
  
  bool Found;
  
  do {
    Found = false;
    for (map<G4VPhysicalVolume*, G4String>::iterator Iter = m_UniquePhysicalVolumeNames.begin();
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
G4String MCGeometryConverter::MakeValidName(G4String Name)
{
  // Makes a valid name out of "Name"
  // If the returned string is empty this task was impossible
  
  G4String ValidName;
  
  for (unsigned int i = 0; i < Name.size(); ++i) {
    if (std::isalnum(Name[i]) != 0 || 
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
