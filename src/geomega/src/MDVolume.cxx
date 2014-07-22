/*
 * MDVolume.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MDVolume
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDVolume.h"

// Standard libs:
#include <iostream>
#include <sstream>
#include <float.h>
#include <limits>
#include <algorithm>
using namespace std;

// ROOT libs:
#include "TRotation.h"
#include "TGeoMatrix.h"
#include "TGeoMedium.h"
#include "TGeoVolume.h"
#include "TGeoShape.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MDDetector.h"
#include "MStreams.h"
#include "MDVolumeSequence.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDVolume)
#endif


////////////////////////////////////////////////////////////////////////////////


int MDVolume::m_IDCounter = 1; 
int MDVolume::m_RotIDCounter = 1;
int MDVolume::m_WrittenRotID = 1;


////////////////////////////////////////////////////////////////////////////////


MDVolume::MDVolume(MString Name, MString ShortName)
{
  // default constructor

  // ID of this volume:
  if (m_IDCounter == g_IntNotDefined) {
    m_IDCounter = 0;
  } else {
    m_ID = m_IDCounter++;
  }

  //cout<<"Creating volume "<<Name<<" with ID"<<m_ID<<endl; 

  m_RotID = 0;
  m_SensID = 0;

  m_Name = Name;
  m_ShortName = ShortName;



  m_Mother = 0;
  m_Material = 0;
  m_Shape = 0;
  m_Position = g_VectorNotDefined;
  
  m_Rotation = 0;
  m_RotMatrix.ResizeTo(3,3);
  m_RotMatrix(0,0) = 1;
  m_RotMatrix(1,1) = 1;
  m_RotMatrix(2,2) = 1;
  m_InvertedRotMatrix.ResizeTo(3,3);
  m_InvertedRotMatrix(0,0) = 1;
  m_InvertedRotMatrix(1,1) = 1;
  m_InvertedRotMatrix(2,2) = 1;
  m_Theta1 = 90.0;
  m_Phi1 = 0.0;
  m_Theta2 = 90.0;
  m_Phi2 = 90.0;
  m_Theta3 = 0.0;
  m_Phi3 = 0.0;
  m_IsRotated = false;
  
  m_IsSensitive = false;
  m_IsVirtual = false;
  m_IsMany = false;
  m_DoAbsorptions = true;

  m_Color = g_IntNotDefined;
  m_LineWidth = g_IntNotDefined;
  m_LineStyle = g_IntNotDefined;
  m_Visibility = g_IntNotDefined;

  m_Detector = 0;
  m_IsDetectorVolume = false; 
  m_DetectorVolume = 0;

  m_CloneTemplate = 0;
  m_CloneTemplateVolumeWritten = false;
  m_CloneTemplateDaughtersWritten = false;
  m_CloneTemplateId = 0;

  m_WorldVolume = false;

  m_Tolerance = 0;
}


////////////////////////////////////////////////////////////////////////////////


MDVolume::~MDVolume()
{
  // We only delete a few things here, most is done in MDGeometry:

  // Destructor is missing....
}


////////////////////////////////////////////////////////////////////////////////


MString MDVolume::GetShortName()
{
  // Return the type of this volume

  return m_ShortName;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetShortName(MString ShortName)
{
  // Return the type of this volume

  m_ShortName = ShortName;
}


////////////////////////////////////////////////////////////////////////////////


MDMaterial* MDVolume::GetMaterial()
{
  // Return a pointer to the material of this volume or zero if there is none

  return m_Material;
}


////////////////////////////////////////////////////////////////////////////////


MDShape* MDVolume::GetShape()
{
  // Return a pointer to the shape (BRIK, SPHE) of the material or 
  // zero if there is none

  return m_Shape;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDVolume::GetPosition()
{
  // Return the position of the material in its mothers coordinate system

  return m_Position;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::IsRotated() const
{
  // Return true if the rotation matrix is not the identity

  return m_IsRotated;
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDVolume::GetMother()
{
  // Return the mother volume or zero if there is none

  return m_Mother;
}


////////////////////////////////////////////////////////////////////////////////


int MDVolume::GetVisibility()
{
  // Return the visibility of this shape for the ROOT-view:
  // 0: the volume is not displayed
  // 1: the volume is displayed

  return m_Visibility;
}


////////////////////////////////////////////////////////////////////////////////


int MDVolume::GetColor()
{
  // Return the color of this volume (ROOT-color-code)

  return m_Color;
}


////////////////////////////////////////////////////////////////////////////////


int MDVolume::GetLineWidth()
{
  // Return the line width of this volume 

  return m_LineWidth;
}


////////////////////////////////////////////////////////////////////////////////


int MDVolume::GetLineStyle()
{
  // Return the line style of this volume

  return m_LineStyle;
}


////////////////////////////////////////////////////////////////////////////////


int MDVolume::GetID()
{
  // Return the ID of this volume

  return m_ID;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDVolume::GetNDaughters()
{
  // Return the number of daughter volumes placed in this volume

  return m_Daughters.size();
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDVolume::GetDaughterAt(unsigned int i)
{
  // Return the daughter volume of this shape at position i

  if (i < GetNDaughters()) {
    return m_Daughters[i];
  } else {
    Fatal("MDVolume* MDVolume::GetDaughterAt(int i)",
          "Index (%d) out of bounds (%d, %d)", i, 0, GetNDaughters()-1);
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDVolume::GetSize()
{
  // Return the size of this shape, i.e. an outer border (== box)

  return m_Shape->GetSize();
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDVolume::GetCloneTemplate()
{
  // If this volume is a copy of another volume return a pointer to this volume 
  // otherwise return 0

  return m_CloneTemplate;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::IsClone() const
{
  // Return true if this volume is a clone of another volume

  if (m_CloneTemplate == 0) return false;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


int MDVolume::GetRotationID() const
{
  // Return the ID of the rortation-matrix

  return m_RotID;
}


////////////////////////////////////////////////////////////////////////////////


TMatrixD MDVolume::GetRotationMatrix() const
{
  // Return the rotation as matrix

  return m_RotMatrix;
}


////////////////////////////////////////////////////////////////////////////////


TMatrixD MDVolume::GetInvRotationMatrix() const
{
  // Return the rotation as the inverse matrix

  return m_InvertedRotMatrix;
}


////////////////////////////////////////////////////////////////////////////////


int MDVolume::GetSensitiveVolumeID()
{
  // If this volume is the sensitive volume of a detector return its
  // sensitive volume ID

  return m_SensID;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::IsCloneTemplateVolumeWritten()
{
  // Retrurn true if the clone template has already been written

  return m_CloneTemplateVolumeWritten;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetCloneTemplateVolumeWritten(bool Written)
{
  // Set whether the clone template has already been written to the file 

  m_CloneTemplateVolumeWritten = Written;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::AreCloneTemplateDaughtersWritten()
{
  // Retrurn true if the clone template has already been written

  return m_CloneTemplateDaughtersWritten;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetCloneTemplateDaughtersWritten(bool Written)
{
  // Set whether the clone template has already been written to the file 

  m_CloneTemplateDaughtersWritten = Written;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::IsSensitive()
{
  // Return true if this volume is sensitive, i.e. when this volume is the 
  // sensitive volume of a detector

  return m_IsSensitive;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetVirtual(bool Virtual)
{
  // Degrades this volume to a virtual volume. Virtual volumes do not appear in
  // the final geometry. They do not have any volume and therefore are allowed
  // to intersect. All volumes contained in this volume are not allowed to 
  // intersect with any other volume in the geometry.

  //cout<<"Setting virtual for "<<m_Name<<" to "<<(int) Virtual<<endl;
  //if (m_Name.CompareTo("Klotz") == 0) Fatal("", "");
  m_IsVirtual = Virtual;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::IsVirtual()
{
  // Return true if this is a virtual volume

  return m_IsVirtual;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetMany(bool Many)
{
  // Set's the many flag.
  // In Geant3 this means that the volume is allowed to intersect other volumes
  // Intersections are not allowed in any Geomega-highlevel function and not
  // in Geant4!!!!

  m_IsMany = Many;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::IsMany()
{
  // Return true if the many flag is raised!

  return m_IsMany;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetAbsorptions(bool DoAbsorption)
{
  // True if for this volume absorptions shoould be calculated:

  m_DoAbsorptions = DoAbsorption;
}

////////////////////////////////////////////////////////////////////////////////


bool MDVolume::DoAbsorptions()
{
  // True if for this volume absorptions shoould be calculated:

  return m_DoAbsorptions;
}


////////////////////////////////////////////////////////////////////////////////


MDDetector* MDVolume::GetDetector()
{
  // Return the detector this volume belongs to or zero 

  return m_Detector;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetDetector(MDDetector* Detector)
{
  // Set the detector, this volume belongs to

  //cout<<m_Name<<" with detector: "<<(Detector != 0 ? Detector->GetName() : "none")<<endl;

  m_Detector = Detector;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetIsDetectorVolume(MDDetector* Detector)
{
  // Set this flag, if this volume represents a detector

  m_IsDetectorVolume = true;

  // cout<<"I ("<<m_Name<<") am a detector volume"<<endl;

  // Set this volume as detector volume of this and of all (with exceptions) daughter volumes
  SetDetectorVolume(this, Detector);

  // Do the same for all clones:
  for (unsigned int d = 0; d < GetNClones(); d++) {
    GetCloneAt(d)->SetIsDetectorVolume(Detector);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetDetectorVolume(MDVolume* Volume, MDDetector* Detector)
{
  // If this volume represents a detector or is part a a detector,
  // "Volume" is the volume of the detector 
  
  // cout<<"Setting det.vol. "<<Volume->GetName()<<" for "<<GetName()<<" with detector: "<<Detector->GetName()<<endl;
  
  m_DetectorVolume = Volume;
  m_Detector = Detector;

  // Why are we setting this for all daughter volumes???
  for (unsigned int d = 0; d < GetNDaughters(); ++d) {
    // Only set the detector, if the daughter already has none!
    // Otherwise the search might get wrong if there are nested detector types
    if (GetDaughterAt(d)->GetDetector() == 0) {
      GetDaughterAt(d)->SetDetectorVolume(Volume, Detector);
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDVolume::GetDetectorVolume()
{
  // Return the detector volume this volume belongs to, or zero otherwise

  return m_DetectorVolume;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::IsDetectorVolume()
{
  // If the volume represents a dector return true

  mdep<<"Don't use this function --- doesn't seem to work --- delete later"<<endl;

  return m_IsDetectorVolume;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::ContainsDetectorVolume()
{
  // Check if either this volume or one of its daughters is a detector volume

  if (m_Detector != 0) {
    return true;
  }

  for (unsigned int d = 0; d < GetNDaughters(); ++d) {
    if (GetDaughterAt(d)->ContainsDetectorVolume() == true) {
      return true;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetSensitiveVolumeID(int ID)
{
  // Give this volume a sensitive volume ID

  m_SensID = ID;

  SetIsSensitive();

  unsigned int c;
  for (c = 0; c < GetNClones(); c++) {
    GetCloneAt(c)->SetIsSensitive();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetIsSensitive()
{
  // Sets this volumes sensitivity flag to true

  m_IsSensitive = true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::IsWorldVolume()
{
  // Return truie if this is the root/master/mother etc volume

  return m_WorldVolume;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetMaterial(MDMaterial *Material)
{
  // Set the material of this volume

  m_Material = Material;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetShape(MDShape* Shape)
{
  // Set the shape of this volume

  m_Shape = Shape;

  m_Shape->SetColor(m_Color);
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetPosition(MVector Position)
{
  // Set the position of this volume

  m_Position = Position;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetRotation(double theta1, double phi1, 
                           double theta2, double phi2, 
                           double theta3, double phi3)
{
  // Set Geant3 type rotations
  // 
  // This function only converts the rotation matrix in another format and then
  // calls another SetRotation() function which handles the rotation IDs

  MVector xcolumn, ycolumn, zcolumn;
  xcolumn.SetMagThetaPhi(1.0, theta1*c_Rad, phi1*c_Rad);
  ycolumn.SetMagThetaPhi(1.0, theta2*c_Rad, phi2*c_Rad);
  zcolumn.SetMagThetaPhi(1.0, theta3*c_Rad, phi3*c_Rad);


  // Set the columns of the rotation matrix: correct!
  TMatrixD Matrix(3, 3);
  Matrix(0, 0) = xcolumn[0];
  Matrix(1, 0) = ycolumn[0];
  Matrix(2, 0) = zcolumn[0];

  Matrix(0, 1) = xcolumn[1];
  Matrix(1, 1) = ycolumn[1];
  Matrix(2, 1) = zcolumn[1];

  Matrix(0, 2) = xcolumn[2];
  Matrix(1, 2) = ycolumn[2];
  Matrix(2, 2) = zcolumn[2];


  // We have to do some sanity checks, if the axis are orthogonal:
  bool Error = false;
  double tol = 1E-6;
  double u1u2 = xcolumn.Dot(ycolumn);
  double f12  = fabs(u1u2);
  if ( f12 > tol) {
    mout<<"   *** Error *** in volume "<<m_Name<<endl;
    mout<<"The columns X and Y supplied for rotation are not close to orthogonal: "
        <<"cos = "<<f12<<endl;
    Error = true;
  }
  double u1u3 = xcolumn.Dot(zcolumn);
  double f13  = fabs(u1u3);
  if ( f13 > tol) {
    mout<<"   *** Error *** in volume "<<m_Name<<endl;
    mout<<"The columns X and Z supplied for rotation are not close to orthogonal: "
        <<"cos = "<<f12<<endl;
    Error = true;
  }
  double u2u3 = ycolumn.Dot(zcolumn);
  double f23  = fabs(u2u3);
  if ( f23 > tol) {
    mout<<"   *** Error *** in volume "<<m_Name<<endl;
    mout<<"The columns Y and Z supplied for rotation are not close to orthogonal: "
        <<"cos = "<<f23<<endl;
    Error = true;
  }

  if ( (f12 <= f13) && (f12 <= f23) ) {
    if (xcolumn.Cross((ycolumn - u1u2*xcolumn).Unit()).Dot(zcolumn) < 0) {
      mout<<"   *** Error *** in volume "<<m_Name<<endl;
      mout<<"The columns X Y and Z are closer to a reflection than a rotation: "
          <<"x res = "<<xcolumn.Cross((ycolumn - u1u2*xcolumn).Unit()).Dot(zcolumn)<<endl;
      Error = true;
    }
  } else if ( f13 <= f23 ) {
    if (zcolumn.Cross((xcolumn - u1u3*zcolumn).Unit()).Dot(ycolumn) < 0) {
      mout<<"   *** Error *** in volume "<<m_Name<<endl;
      mout<<"The columns X Y and Z are closer to a reflection than a rotation: "
          <<"z res = "<<zcolumn.Cross((xcolumn - u1u3*zcolumn).Unit()).Dot(ycolumn)<<endl;
      Error = true;
    }
  } else {
    if (ycolumn.Cross((zcolumn - u2u3*ycolumn).Unit()).Dot(xcolumn) < 0) {
      mout<<"   *** Error *** in volume "<<m_Name<<endl;
      mout<<"The columns X Y and Z are closer to a reflection than a rotation: "
          <<"y res = "<<ycolumn.Cross((zcolumn - u2u3*ycolumn).Unit()).Dot(xcolumn)<<endl;
      Error = true;
    }
  }

  if (Error == false) {
    SetRotation(Matrix);
  } else {
    mout<<"Ignoring the rotation"<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetRotation(double x, double y, double z)
{
  // Set the rotation of this volume
  // x,y,z are the counterclockwise rotations around x, y and z in c_Deg
  // 
  // Then call the standard SetRotation() which handles all IDs

  double sinx, siny, sinz, cosx, cosy, cosz;
  sinx = sin(x*c_Rad);
  siny = sin(y*c_Rad);
  sinz = sin(z*c_Rad);
  cosx = cos(x*c_Rad);
  cosy = cos(y*c_Rad);
  cosz = cos(z*c_Rad);

  MVector xcolumn, ycolumn, zcolumn;
  xcolumn = MVector(1.,0.,0.);
  ycolumn = MVector(0.,1.,0.);
  zcolumn = MVector(0.,0.,1.);
  
  TMatrixD Matrix(3,3);
  Matrix(0,0) =   cosz*cosy;
  Matrix(1,0) = - sinz*cosx + cosz*siny*sinx;
  Matrix(2,0) =   sinz*sinx + cosz*siny*cosx;
 
  Matrix(0,1) =   sinz*cosy;
  Matrix(1,1) =   cosz*cosx + sinz*siny*sinx;
  Matrix(2,1) = - cosz*sinx + sinz*siny*cosx;

  Matrix(0,2) = - siny;
  Matrix(1,2) =   cosy*sinx;
  Matrix(2,2) =   cosy*cosx;

  SetRotation(Matrix);
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetRotation(TMatrixD Rotation)
{
  // Set the rotation of this volume and handle all IDs

  MVector xcolumn, ycolumn, zcolumn;

  xcolumn = MVector(1.,0.,0.);
  ycolumn = MVector(0.,1.,0.);
  zcolumn = MVector(0.,0.,1.);
  
  m_InvertedRotMatrix.ResizeTo(3,3);
  m_RotMatrix.ResizeTo(3,3);

  // I never figuered out, why we have to invert it here...
  m_RotMatrix = Rotation.Invert();
  
  xcolumn = m_RotMatrix*xcolumn;
  ycolumn = m_RotMatrix*ycolumn;
  zcolumn = m_RotMatrix*zcolumn; 

  m_Theta1 = xcolumn.Theta()*c_Deg;
  m_Phi1 = xcolumn.Phi()*c_Deg;
  
  m_Theta2 = ycolumn.Theta()*c_Deg;
  m_Phi2 = ycolumn.Phi()*c_Deg;
  
  m_Theta3 = zcolumn.Theta()*c_Deg;
  m_Phi3 = zcolumn.Phi()*c_Deg;
  
  // Make in any case a new rotation ID, otherwise gmega is wrong for virtual volumes
  m_RotID = m_RotIDCounter++;
  m_IsRotated = true;

  // Now invert back...
  m_InvertedRotMatrix = m_RotMatrix;
  m_RotMatrix.Invert();
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetRotation(TMatrixD RotationMatrix, int RotID)
{
  // Set the rotation of this volume as a rotation matrix

  m_RotMatrix = RotationMatrix;
  m_InvertedRotMatrix = RotationMatrix;
  m_InvertedRotMatrix.Invert();
  m_RotID = RotID;
  
  // Calculate rotation for Geant3
  MVector xcolumn, ycolumn, zcolumn;

  xcolumn = MVector(1.,0.,0.);
  ycolumn = MVector(0.,1.,0.);
  zcolumn = MVector(0.,0.,1.);

  TMatrixD RM = RotationMatrix;
  RM.Invert();

  xcolumn = RM*xcolumn;
  ycolumn = RM*ycolumn;
  zcolumn = RM*zcolumn; 
  
  m_Theta1 = xcolumn.Theta()*c_Deg;
  m_Phi1 = xcolumn.Phi()*c_Deg;
  
  m_Theta2 = ycolumn.Theta()*c_Deg;
  m_Phi2 = ycolumn.Phi()*c_Deg;
  
  m_Theta3 = zcolumn.Theta()*c_Deg;
  m_Phi3 = zcolumn.Phi()*c_Deg;

  m_IsRotated = true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::SetMother(MDVolume* Mother)
{
  // Set the mother volume:
  // This function breaks all iterators on m_Daughters

  // First undo all previous SetMothers:
  if (m_Mother != 0) {
    m_Mother->RemoveDaughter(this);
  }

  if (Mother != 0 && Mother->HasMother(this) == true) {
    mout<<"   ***  Error  ***  in volume "<<m_Name<<endl;
    mout<<"The desired mother volume ("<<Mother->GetName()<<") already has this volume ("<<m_Name<<") has mother!"<<endl;    
    return false;
  }
  m_Mother = Mother;
  if (m_Mother != 0) {
    m_Mother->AddDaughter(this);
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::HasMother(MDVolume* Mother)
{
  //! Check if any of the mothers is "Mother"
  
  if (m_Mother == 0) return false;
  if (m_Mother == Mother) return true;
  return m_Mother->HasMother(Mother);
}
  

////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetVisibility(int Visibility)
{
  // 0: if this volume is not seen in the Root view
  // 1: seen if ShowVolumes not false
  // 2: always seen
  // 3: always seen and also daughters

  m_Visibility = Visibility;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetColor(int Color)
{
  // Set the color of this volume

  m_Color = Color;

  if (m_Shape != 0) {
    m_Shape->SetColor(Color);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetLineWidth(int LineWidth)
{
  // Set the color of this volume

  m_LineWidth = LineWidth;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetLineStyle(int LineStyle)
{
  // Set the color of this volume

  m_LineStyle = LineStyle;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetWorldVolume()
{
  // Makes this volume the world volume

  m_WorldVolume = true;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::AddDaughter(MDVolume* Daughter)
{
  // Add a daughter volume, i.e. a volume placed within this volume

  massert(this != 0);

  // If daughter (pointer) already exists -- do not add it
  if (find(m_Daughters.begin(), m_Daughters.end(), Daughter) == m_Daughters.end()) {
    m_Daughters.push_back(Daughter);
    // Under some special circumstances (forgot why) we have to add the detector to the daughter if the daughter has not yet a detector
    if (m_DetectorVolume != 0 && Daughter->GetDetector() == 0) {
      Daughter->SetDetectorVolume(m_DetectorVolume, m_Detector);
    }
  }
}

 
////////////////////////////////////////////////////////////////////////////////


void MDVolume::RemoveAllDaughters()
{
  // Remove a all daughters

  massert(this != 0);

  m_Daughters.clear();
}

 
////////////////////////////////////////////////////////////////////////////////


MDVolume* MDVolume::RemoveDaughter(MDVolume* Daughter)
{
  // Remove a daughter AND remove the daughters mother volume

  massert(this != 0);

  vector<MDVolume*>::iterator Iter = find(m_Daughters.begin(), m_Daughters.end(), Daughter);
  if (Iter != m_Daughters.end()) {
    m_Daughters.erase(find(m_Daughters.begin(), m_Daughters.end(), Daughter));
  }

  return Daughter;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::AddClone(MDVolume* Clone)
{
  // Add the pointer to a volume which is a clone of this volume 

  massert(this != 0);

  m_Clones.push_back(Clone);
  Clone->SetCloneTemplate(this);
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDVolume::GetNClones() const
{
  // return the number clones of this volume

  massert(this != 0);

  return m_Clones.size();
}

////////////////////////////////////////////////////////////////////////////////


MDVolume* MDVolume::GetCloneAt(unsigned int i)
{
  // Return the clone at position i

  massert(this != 0);

  if (i < GetNClones()) {
    return m_Clones[i];
  } else {
    Fatal("MDVolume* MDVolume::GetCloneAt(int i)",
          "Index (%d) out of bounds (%d, %d)", i, 0, GetNClones()-1);
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDVolume::GetCloneId(MDVolume* Clone)
{
  // Get the ID of the clone

  massert(this != 0);

  for (unsigned int i = 0; i < m_Clones.size(); ++i) {
    if (Clone == m_Clones[i]) {
      return ++m_CloneTemplateId;
    }
  }
  
  return numeric_limits<unsigned int>::max();
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::SetCloneTemplate(MDVolume* CloneTemplate)
{
  // This volume is a clone of volume CloneTemplate

  massert(this != 0);

  m_CloneTemplate = CloneTemplate;
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDVolume::Clone(MString Name)
{
  // Create a new volume which is a clone of this volume - including all daughters 
  // This function should only be used in combination with "RemoveVirtual Volume"!

  massert(this != 0);

  MDVolume* V;

  if (GetCloneTemplate() == 0) {

    V = new MDVolume(Name+"_"+m_Name, m_ShortName);
    V->SetMaterial(m_Material);
    V->SetShape(m_Shape);

    V->SetVirtual(m_IsVirtual);

    V->SetMany(m_IsMany);
    V->SetMother(m_Mother);

    if (IsDetectorVolume() == true) {
      V->SetIsDetectorVolume(m_Detector);
    }
    if (IsSensitive() == true) {
      V->SetIsSensitive();
      V->SetIsDetectorVolume(m_Detector);
    }

    for (unsigned int j = 0; j < GetNDaughters(); j++) {
      V->AddDaughter(GetDaughterAt(j));
    }

    AddClone(V);

  } else {
    V = new MDVolume(Name+"_"+GetCloneTemplate()->GetName()+"["+m_Name+"]", GetCloneTemplate()->GetShortName());

    V->SetMaterial(GetCloneTemplate()->GetMaterial());
    V->SetShape(GetCloneTemplate()->GetShape());

    V->SetVirtual(GetCloneTemplate()->IsVirtual());
    V->SetMany(GetCloneTemplate()->IsMany());

    V->SetMother(GetCloneTemplate()->GetMother());
    V->SetDetector(GetCloneTemplate()->GetDetector());
//     if (m_Detector != 0) {
//       MDDetector* DClone = m_Detector->Clone();
//       DClone->SetDetectorVolume(V);
//       V->SetDetector(DClone);
//     } else {
//       V->SetDetector(0);
//     }

    if (GetCloneTemplate()->IsDetectorVolume() == true) {
      V->SetIsDetectorVolume(m_Detector);
    }
    if (GetCloneTemplate()->IsSensitive() == true) {
      V->SetIsSensitive();
      V->SetIsDetectorVolume(m_Detector);
    }
    for (unsigned int j = 0; j < GetCloneTemplate()->GetNDaughters(); j++) {
      V->AddDaughter(GetCloneTemplate()->GetDaughterAt(j));
    }
    GetCloneTemplate()->AddClone(V);
  }

  // Modifiable stuff:
  V->SetColor(m_Color);
  V->SetLineWidth(m_LineWidth);
  V->SetLineStyle(m_LineStyle);
  V->SetVisibility(m_Visibility);


  V->SetPosition(m_Position);
  if (m_IsRotated == true) {
    V->SetRotation(m_RotMatrix, m_RotID);
  }

  return V;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::CopyDataToClones()
{
  // Copy all data to clone volumes, whose data is on default values

  // Something that should not be wrong at this point:
  // Do we have a shape?
  if (m_Shape == 0) {
    cerr<<" *** Error ***";
    cerr<<"The following volume has no shape: "<<m_Name<<endl;
    return false;
  }


  MDVolume* Clone = 0;

  for (unsigned i = 0; i < m_Clones.size(); i++) {
    Clone = m_Clones[i];
    
    if (Clone->GetShortName().IsEmpty()) {
      Clone->SetShortName(m_ShortName);
    }
    
    if (Clone->GetMaterial() == 0) {
      Clone->SetMaterial(m_Material);
    }
    
    if (Clone->IsVirtual() == false) {
      Clone->SetVirtual(m_IsVirtual);
    }
    
    if (Clone->IsMany() == false) {
      Clone->SetMany(m_IsMany);
    }

    if (Clone->GetShape() == 0) {
      Clone->SetShape(m_Shape);
    }

    if (Clone->GetColor() == g_IntNotDefined) {
      Clone->SetColor(m_Color);
    }

    if (Clone->GetLineStyle() == g_IntNotDefined) {
      Clone->SetLineStyle(m_LineStyle);
    }

    if (Clone->GetLineWidth() == g_IntNotDefined) {
      Clone->SetLineWidth(m_LineWidth);
    }

    if (Clone->GetVisibility() == g_IntNotDefined) {
      Clone->SetVisibility(m_Visibility);
    }

    if (Clone->IsRotated() == false && m_IsRotated == true) {
      Clone->SetRotation(m_RotMatrix, m_RotID);
    }

    if (Clone->GetMother() == 0) {
      Clone->SetMother(m_Mother);
    }

    if (Clone->GetDetector() == 0) {
      Clone->SetDetector(m_Detector);
    }

    if (Clone->GetPosition() == MVector(g_DoubleNotDefined, g_DoubleNotDefined, g_DoubleNotDefined)) {
      Clone->SetPosition(m_Position);
    }

    for (unsigned int j = 0; j < GetNDaughters(); j++) {
      Clone->AddDaughter(GetDaughterAt(j));
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////
  

bool MDVolume::Validate()
{
  // Test if everything is ok

  if (this == 0) {
    mout<<"   ***  Error  ***  in volume "<<m_Name<<endl;
    mout<<"Never mind... I do not exist..."<<endl;
    return false;
  }

  // Do we have a shape?
  if (m_Shape == 0) {
    mout<<"   ***  Error  ***  in volume "<<m_Name<<endl;
    mout<<"The volume has no shape!"<<endl;
    return false;
  }

  // Test for position
  if (m_Position == g_VectorNotDefined) {
    //cerr<<" *** Warning ***"<<endl;
    //cerr<<"The following volume has no position information: "<<m_Name<<endl;
    //cerr<<"Setting the position to (0, 0, 0)"<<endl;
    m_Position = MVector(0.0, 0.0, 0.0);
  }

  if (m_Visibility == g_IntNotDefined) {
    SetVisibility(1);
  }

  if (m_Color == g_IntNotDefined) {
    SetColor(1);
  }
  // Geant4 defines colors only for logical volumes not the positioned ones
  // Thus we have to make sure that the clone template has a color
  // Problem: If the clones have multiple colors only the last one is set to the template and Geant4 is only showing the last one
  if (GetCloneTemplate() != 0 && GetCloneTemplate()->GetColor() == g_IntNotDefined) {
    if (m_Color == g_IntNotDefined) {
      GetCloneTemplate()->SetColor(1);
    } else {
      GetCloneTemplate()->SetColor(m_Color);
    }
  }

  if (m_LineWidth == g_IntNotDefined) {
    SetLineWidth(1);
  }

  if (m_LineStyle == g_IntNotDefined) {
    SetLineStyle(1);
  }

  // The world volume is not allowed to be virtual!
  if (m_WorldVolume == true) {
    SetVirtual(false);
    SetVisibility(0);
  }

  if (m_Material == 0) {
    mout<<"   ***  Error  ***  in volume "<<m_Name<<endl;
    mout<<"The volume has no material!"<<endl;
    return false;
  }

  if (m_Mother == 0 && m_WorldVolume == false) {
    mout<<"   ***  Error  ***  in volume "<<m_Name<<endl;
    mout<<"The volume has no mother (and is not the world volume)!"<<endl;
    return false;
  }

  // Negative visibility means this and all daughters have a visibility of -1:
  if (m_Visibility == -1 || m_Visibility == 3) {
    for (unsigned int i = 0; i < GetNDaughters(); i++) {
      GetDaughterAt(i)->SetVisibility(m_Visibility);
    }
  }

  // Give a warning if a clone template is positioned
  if (IsClone() == false && GetNClones() > 0 && m_Mother != 0) {
    mout<<"   ***  Soft warning  ***  in volume "<<m_Name<<endl;
    mout<<"You have a clone template (volume to which you apply the .Copy operator)"<<endl;
    mout<<"Which is positioned in another volume: "<<m_Mother->GetName()<<endl;
    mout<<"This is fine, but depreciated... because error prone!"<<endl;
  }

  // If we are sensitive, then we need a detector volume:
  if (m_IsSensitive == true && m_Detector == 0) {
    mout<<"   ***  Error  ***  in volume "<<m_Name<<endl;
    mout<<"We have a sensitive volume without a detector!"<<endl;
    return false;
  }
  
  // Test daughters
  for (unsigned int i = 0; i < GetNDaughters(); i++) {
    if (GetDaughterAt(i)->Validate() == false) return false;
  }

  // Now test if this volume has to be drawn (i.e. a node has to be created):
  // If this volume is visible, it has to be drawn

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::ValidateClonesHaveSameMotherVolume()
{
  // All copies must be in mothers of the some volume type
  
  MString MothersCloneTemplate = g_StringNotDefined;

  // If this is a clone template:
  if (GetCloneTemplate() != 0) {
    for (unsigned int c = 0; c < GetCloneTemplate()->GetNClones(); ++c) {
      if (GetCloneTemplate()->GetCloneAt(c)->GetMother() != 0) {
        // Ignore virtual volumes:
        if (GetCloneTemplate()->GetCloneAt(c)->GetMother()->IsVirtual() == false) {
          if (GetCloneTemplate()->GetCloneAt(c)->GetMother()->GetCloneTemplate() != 0) {
            if (MothersCloneTemplate == g_StringNotDefined) {
              MothersCloneTemplate = GetCloneTemplate()->GetCloneAt(c)->GetMother()->GetCloneTemplate()->GetName();
            } else {
              if (MothersCloneTemplate != GetCloneTemplate()->GetCloneAt(c)->GetMother()->GetCloneTemplate()->GetName()) {
                mout<<"   ***  SEVERE WARNING  ***  in volume "<<m_Name<<endl;
                mout<<"Some of the clones are residing in different mother volumes(e.g. "<<MothersCloneTemplate
                    <<" and "<<GetCloneTemplate()->GetCloneAt(c)->GetMother()->GetCloneTemplate()->GetName()<<")!"<<endl;
                mout<<"Activation simulations will fail!"<<endl;
                break;
              }
            }
          } else {
            if (MothersCloneTemplate == g_StringNotDefined) {
              MothersCloneTemplate = GetCloneTemplate()->GetCloneAt(c)->GetMother()->GetName();
            } else {
              if (MothersCloneTemplate != GetCloneTemplate()->GetCloneAt(c)->GetMother()->GetName()) {
                mout<<"   ***  SEVERE WARNING  ***  in volume "<<m_Name<<endl;
                mout<<"Some of the clones are residing in different mother volumes (e.g. "<<MothersCloneTemplate
                    <<" and "<<GetCloneTemplate()->GetCloneAt(c)->GetMother()->GetName()<<")!"<<endl;
                mout<<"Activation simulations will fail!"<<endl;
                break;
              }
            }
          }
        }
      }
    }
  }

    // Test daughters
  for (unsigned int i = 0; i < GetNDaughters(); i++) {
    if (GetDaughterAt(i)->ValidateClonesHaveSameMotherVolume() == false) return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::ValidateIntersections()
{
  // Test if
  // (1) All daughters are contained in this volume
  // (2) The daughters do not intersect

  const double Tolerance = 1E-6;

  vector<MVector> Surface;
  MVector Point;

  // (1) All daughters are contained in this volume
  for (unsigned int d = 0; d < GetNDaughters(); d++) {
    Surface = GetDaughterAt(d)->GetShape()->CreateSurfacePattern(5);
    if (Surface.size() == 0) {
      mdebug<<"Can't check intersections (1) for: "<<GetDaughterAt(d)->GetName()<<" - no surface points"<<endl;
      continue;
    }

    for (unsigned int p = 0; p < Surface.size(); ++p) {
      Point = Surface[p];

      // Modify position:
      if (GetDaughterAt(d)->IsRotated() == true) {
        Point = GetDaughterAt(d)->GetInvRotationMatrix() * Point;    // rotate
      }
      Point += GetDaughterAt(d)->GetPosition();           // translate 

      if (m_Shape->IsInside(Point) == false &&
          m_Shape->DistanceOutsideIn(Point, -Point) > Tolerance) {
        mout<<" *** Error *** "<<endl;
        mout<<" Volume "<<GetDaughterAt(d)->GetName()<<" is not completely contained in "<<m_Name<<"!"<<endl;
        mout<<" Point of "<<GetDaughterAt(d)->GetName()<<": "<<Point[0]<<"!"<<Point[1]<<"!"<<Point[2]<<endl;
        mout<<" Distance outside in (in direction of origin): "<<m_Shape->DistanceOutsideIn(Point, -Point)<<endl;
      } 
    }
  }
    
  // (2) The daughters do not intersect
  for (unsigned int d = 0; d < GetNDaughters(); d++) {
    for (unsigned int e = 0; e < GetNDaughters(); e++) {
      if (d == e) continue;
      
      Surface = GetDaughterAt(e)->GetShape()->CreateSurfacePattern(5);
      if (Surface.size() == 0) {
        mdebug<<"Can't check intersections (2) for: "<<GetDaughterAt(e)->GetName()<<" - no surface points"<<endl;
        continue;
      }

      
      //      cout<<"Shape of "<<GetDaughterAt(e)->GetName()<<endl;
      //      for (int p = 0; p < Surface->GetLast()+1; p++) {
      //        Point = *((MVector *) (Surface->At(p)));
      //      }

      for (unsigned int p = 0; p < Surface.size(); ++p) {
        Point = Surface[p];

        //        if (GetName().CompareTo("Part_A_5") == 0) 
        //          cout<<"Point of "<<GetDaughterAt(e)->GetName()<<"(before): "<<Point[0]<<"!"<<Point[1]<<"!"<<Point[2]<<endl;

        if (GetDaughterAt(e)->IsRotated() == true) {
          Point = GetDaughterAt(e)->GetInvRotationMatrix() * Point;    // rotate
        }
        Point += GetDaughterAt(e)->GetPosition();           // translate 

        //        if (GetDaughterAt(e)->GetName().CompareTo("Part_A_5") == 0) 
        //          cout<<"Point of "<<GetDaughterAt(e)->GetName()<<"(after): "<<Point[0]<<"!"<<Point[1]<<"!"<<Point[2]<<endl;

        if (GetDaughterAt(d)->IsInside(Point) == true && 
            GetDaughterAt(d)->DistanceInsideOut(Point) > Tolerance) {
          mout<<" *** Error *** in Volume "<<m_Name<<endl;
          mout<<" Daughter "<<GetDaughterAt(e)->GetName()<<" (Point:"<<
            GetDaughterAt(e)->GetPosition()[0]<<", "<<
            GetDaughterAt(e)->GetPosition()[1]<<", "<<
            GetDaughterAt(e)->GetPosition()[2]<<"  Size:"<<
            GetDaughterAt(e)->GetSize()[0]<<", "<<
            GetDaughterAt(e)->GetSize()[1]<<", "<<
            GetDaughterAt(e)->GetSize()[2]<<
            ") intersects with daughter "<<GetDaughterAt(d)->GetName()<<" (Point:"<<
            GetDaughterAt(d)->GetPosition()[0]<<", "<<
            GetDaughterAt(d)->GetPosition()[1]<<", "<<
            GetDaughterAt(d)->GetPosition()[2]<<"  Size:"<<
            GetDaughterAt(d)->GetSize()[0]<<", "<<
            GetDaughterAt(d)->GetSize()[1]<<", "<<
            GetDaughterAt(d)->GetSize()[2]<<")!"<<endl;
          mout<<" Point of "<<GetDaughterAt(e)->GetName()<<": "<<Point[0]<<"!"<<Point[1]<<"!"<<Point[2]<<endl;
          mout<<" Distance inside out from origin: "<<GetDaughterAt(d)->DistanceInsideOut(Point)<<endl;
          //return false;
        } 
      }     
    }
  }


  for (unsigned int d = 0; d < GetNDaughters(); d++) {
    if (GetDaughterAt(d)->ValidateIntersections() == false) {
      //return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::RemoveVirtualVolumes()
{
  // Recursively remove virtual volumes 
  // (1) Links daughters to mother
  // (2) Set new position & rotation 
  // (3) Jump into all daughters

  massert(this != 0);

  //cout<<"RVV: Looking at: "<<m_Name<<endl;

  bool Virtual = false;

  if (IsVirtual() == true) {
    //cout<<"Removing virtual volume: "<<m_Name<<" ND: "<<GetNDaughters()<<endl;
    Virtual = true;

    if (m_Mother == 0) {
      mout<<"   ***  Fatal error  ***  in volume "<<m_Name<<endl;
      mout<<"Virtual volume without mother!"<<endl;
      return false;
    }

    MDVolume* Volume = 0;
    MVector DaughterPosition;
    TMatrixD DaughterRotation(3,3);

    // (a) Add daughters to mother and set new positions and rotations
    vector<MDVolume*>::iterator Daughters;
    for (Daughters = m_Daughters.begin(); Daughters != m_Daughters.end(); ++Daughters) {

      // If this virtual volume is a clone, then we also have to clone its daughters
      if (IsClone() == true) {
        Volume = (*Daughters)->Clone(m_Name);
      } else {
        Volume = (*Daughters); //->Clone(m_Name);
      }

      // Add daughter to Mama and all her Clones...
      //cout<<"Make "<<Volume->GetName()<<" daughter of "<<m_Mother->GetName()<<endl;
      m_Mother->AddDaughter(Volume);
      for (unsigned int c = 0; c < m_Mother->GetNClones(); ++c) {
        //cout<<"Adding "<<Volume->GetName()<<" to "<<m_Mother->GetCloneAt(c)->GetName()<<endl; 
        m_Mother->GetCloneAt(c)->AddDaughter(Volume);
      }
      // We cannot use SetMother(), because it has some additional unwanted features:
      Volume->m_Mother = m_Mother;

      DaughterPosition = Volume->GetPosition();
      DaughterRotation = Volume->GetRotationMatrix();

      // Modify position:
      DaughterPosition = m_InvertedRotMatrix * DaughterPosition;    // rotate
      DaughterPosition += m_Position;           // translate 
      Volume->SetPosition(DaughterPosition);

      // Modify rotation:
      DaughterRotation = DaughterRotation*m_RotMatrix;
      Volume->SetRotation(DaughterRotation);
    }

    // (b) Remove this volume from its mother
    //cout<<"Removing "<<m_Name<<" from "<<m_Mother->GetName()<<endl;
    m_Mother->RemoveDaughter(this);
    // And remove it from Mother's Clones
    for (unsigned int c = 0; c < m_Mother->GetNClones(); c++) {
      //cout<<"Removing "<<m_Name<<" from "<<m_Mother->GetCloneAt(c)->GetName()<<endl;
      m_Mother->GetCloneAt(c)->RemoveDaughter(this);
    }

    // (c) This volume has no longer a mother ;-( 
    m_Mother = 0;

  } else {
    // Remove virtual volumes in the daughters --- only if this volume is not virtual!
    vector<MDVolume*>::iterator Daughters;
    for (Daughters = m_Daughters.begin(); Daughters != m_Daughters.end(); ++Daughters) {
      //cout<<int(*Daughters)<<":"<<GetNDaughters()<<":"<<m_Name<<endl;
      if ((*Daughters)->RemoveVirtualVolumes() == true) {
        // We have an erase --- so we really unfortunately have to start from the beginning
        // because we cannot catch where the erase happened...
        //cout<<"Restart:"<<m_Daughters.size()<<endl;
        Daughters = m_Daughters.begin()-1;
      }
    }
  }

  return Virtual;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::CreateRootGeometry(TGeoManager* Manager, TGeoVolume* Mother)
{
  // Create a node for this volume and its daughters
  // A node is a graphics representation of this volume

  // Do not draw it, if it has no mother volume ...
  if (m_Mother == 0 && m_WorldVolume == false) return;

  TGeoTranslation T(m_Position[0], m_Position[1], m_Position[2]);
  TGeoRotation R(m_Name, m_Theta1, m_Phi1, m_Theta2, m_Phi2, m_Theta3, m_Phi3);
  TGeoCombiTrans* C = new TGeoCombiTrans(T, R);
  
  TGeoMedium* Medium = m_Material->GetRootMedium();
  TGeoShape* Shape = m_Shape->GetRootShape();
  TGeoVolume* Volume = new TGeoVolume(m_Name, Shape, Medium);
  //Volume->SetVisibility(true);
  Volume->SetVisContainers();
  
  if (m_Color == g_IntNotDefined || m_Color <= 0) m_Color = 1;
  Volume->SetLineColor(m_Color);
  Volume->SetFillColor(m_Color);
  Volume->SetLineStyle(m_LineStyle);
  Volume->SetLineWidth(m_LineWidth);

  if (m_WorldVolume == true) {
    Manager->SetTopVolume(Volume);
    //Manager->SetTopVisible(true);
  } else {
    Mother->AddNode(Volume, 1, C); 
  }
  
  if (m_WorldVolume == true) {
    Volume->SetVisibility(false);
  } else {
    if (m_Visibility >= 1) {
      Volume->SetVisibility(true);
    } else {
      Volume->SetVisibility(false);
    }
  }

  // Do the same for all daughters
  for (unsigned int i = 0; i < m_Daughters.size(); ++i) {
    m_Daughters[i]->CreateRootGeometry(Manager, Volume);
  }
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDVolume::GetVolume(MVector Pos, bool IsRelative)
{
  // Return the lowermost volume in the hierarchy Pos is inside, i.e.
  // when Pos is inside this volume and in one of the daughters return the 
  // daughter (or the daughters daughter volume Pos is inside)
  // if it's only in thei svolume return this volume,
  // if it's not in this volume return zero

  //if (m_Mother != 0) 
  //cout<<"Checking: "<<m_Name<<" of "<<m_Mother->GetName()<<endl;

  // Pos is in the mothers coordinate system.
  // So translate and rotate the position into this volumes coordinate system
  
  if (IsRelative == false) {
    Pos -= m_Position;           // translate 
  
    if (m_IsRotated == true) {
      Pos = m_RotMatrix * Pos;    // rotate
    } 
  }

  // Now check if it is inside:
  // cout<<"Inside? "<<m_Name<<endl;
  if (m_Shape->IsInside(Pos, m_Tolerance) == false) {
    return 0;
  }

  // cout<<"Inside: "<<m_Name<<endl;
  // Now it's inside and we can check the daughters:
  unsigned int i;
  MDVolume *V = 0;
  for (i = 0; i < GetNDaughters(); i++) {
    if ((V = GetDaughterAt(i)->GetVolume(Pos, false)) != 0) {
      return V; // Return the daughter or their daugter volume or 
                // the daughter volume of the daughters daughter or ...
    }
  }

  // Ok its only inside this volume:
  return this;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::ContainsVolume(const MString& Name, bool IncludeTemplates)
{
  // Return true if the volume is part of the volume tree
  
  if (m_Name == Name) return true;
  if (IncludeTemplates == true) {
    MString NameThis;
    if (IsClone() == true) {
      NameThis = GetCloneTemplate()->GetName();
    } else {
      NameThis = GetName();
    }

    if (NameThis == Name) {
      return true;
    }
  }

  // Perhaps it's one of the daughter volumes:
  unsigned int NDaughters = GetNDaughters();
  for (unsigned int i = 0; i < NDaughters; ++i) {
    if (GetDaughterAt(i)->ContainsVolume(Name, IncludeTemplates) == true) {
      return true;
    }
  }

  // We didn't find the volume 
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::IsInside(MVector Pos, double Tolerance)
{
  Pos -= m_Position;           // translate 
  
  if (m_IsRotated == true) {
    Pos = m_RotMatrix * Pos;    // rotate
  }

  // Now check if it is inside:
  if (m_Shape->IsInside(Pos, Tolerance) == false) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MDVolume::DistanceInsideOut(MVector Pos)
{
  Pos -= m_Position;           // translate 
  
  if (m_IsRotated == true) {
    Pos = m_RotMatrix * Pos;    // rotate
  }

  return m_Shape->DistanceInsideOut(Pos, Pos);
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::Noise(MVector& Pos, double& Energy, double& Time)
{
  // Pos is in the mothers coordinate system.
  // So translate and rotate the position into this volumes coordinate system
  
  //cout<<"Position Original: "<<m_Name<<": "<<Pos.X()<<"!"<<Pos.Y()<<"!"<<Pos.Z()<<endl;
  Pos -= m_Position;           // translate 
  
  if (m_IsRotated == true) {
    Pos = m_RotMatrix * Pos;    // rotate
  }

  //cout<<"Rot Position: "<<m_Name<<": "<<Pos.X()<<"!"<<Pos.Y()<<"!"<<Pos.Z()<<endl;

  // Now check if it is inside:
  if (m_Shape->IsInside(Pos, m_Tolerance) == false) {
    //cout<<"Not inside shape!"<<endl;
    return false;
  }
  //cout<<"Position \"in\": "<<m_Name<<": "<<Pos.X()<<"!"<<Pos.Y()<<"!"<<Pos.Z()<<endl;
  //cout<<"Inside "<<m_Name<<" daughters: "<<GetNDaughters()<<endl;

  // Now it's inside and we can check the daughters:
  unsigned int i;
  MVector OldPos = Pos;
  unsigned int i_max = GetNDaughters();
  for (i = 0; i < i_max; i++) {
    //cout<<"Checking daughters... of "<<m_Name<<endl;
    Pos = OldPos;
    if (GetDaughterAt(i)->Noise(Pos, Energy, Time) == true) {
      //cout<<"Noised!!! Pos in "<<m_Name<<": "<<Pos.X()<<"!"<<Pos.Y()<<"!"<<Pos.Z()<<endl;
      // OK it had been noised
      // So rotate/translate back and return:
      if (m_IsRotated == true) {
        Pos = m_InvertedRotMatrix * Pos;    // rotate
      }
      Pos += m_Position;
      return true;
    } else {
      //cout<<"Not inside: "<<GetDaughterAt(i)->GetName()<<endl;
    }
  }

  // Ok its only inside this volume - Let's noise
  if (m_Detector != 0) {
    //cout<<"Noise in detector: "<<m_Detector->GetName()<<endl;
    
    // If we have named detectors we have to find the names detector which belongs to this volume
    if (m_Detector->GetNNamedDetectors() > 0) {
      //cout<<"Noise: we have named detectors"<<endl;
      MDVolumeSequence* VS = new MDVolumeSequence();
      GetVolumeSequenceInverse(Pos, VS);
      MDDetector* D = m_Detector->FindNamedDetector(*VS);
      if (D != 0) {
        //cout<<"Named detector found"<<endl;
        D->Noise(Pos, Energy, Time, this);
      } else {
        //cout<<"No named detector found"<<endl;
        m_Detector->Noise(Pos, Energy, Time, this);
      } 
      delete VS;
    } else {
      //cout<<"Noise: we have NO named detectors"<<endl;
      m_Detector->Noise(Pos, Energy, Time, this);
    }
    // Rotate back:
    if (m_IsRotated == true) {
      Pos = m_InvertedRotMatrix * Pos;    // rotate
    }
    Pos += m_Position;

  } else {
    
    // Rotate back:
    if (m_IsRotated == true) {
      Pos = m_InvertedRotMatrix * Pos;    // rotate
    }
    Pos += m_Position;

//     Error("MDVolume::Noise",
//           "\nTrying to noise a hit in a not sensitive volume or detector is not initialized...: (%f,%f,%f) %s\n"
//           "Did you select the correct geometry??",
//           Pos.X(), Pos.Y(), Pos.Z(), (char *) m_Name.Data());

    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::ApplyPulseShape(double Time, MVector& Pos, double& Energy)
{
  // Pos is in the mothers coordinate system.
  // So translate and rotate the position into this volumes coordinate system

  //cout<<"Position Original: "<<m_Name<<": "<<Pos.X()<<"!"<<Pos.Y()<<"!"<<Pos.Z()<<endl;
  Pos -= m_Position;           // translate 
  
  if (m_IsRotated == true) {
    Pos = m_RotMatrix * Pos;    // rotate
  }

  // Now check if it is inside:
  if (m_Shape->IsInside(Pos, m_Tolerance) == false) {
    return false;
  }
  //cout<<"Position \"in\": "<<m_Name<<": "<<Pos.X()<<"!"<<Pos.Y()<<"!"<<Pos.Z()<<endl;
  //cout<<"Inside "<<m_Name<<" daughters: "<<GetNDaughters()<<endl;

  // Now it's inside and we can check the daughters:
  unsigned int i;
  MVector OldPos = Pos;
  for (i = 0; i < GetNDaughters(); i++) {
    //cout<<"Checking daughters... of "<<m_Name<<endl;
    Pos = OldPos;
    if (GetDaughterAt(i)->ApplyPulseShape(Time, Pos, Energy) == true) {
      //cout<<"Noised!!! Pos in "<<m_Name<<": "<<Pos.X()<<"!"<<Pos.Y()<<"!"<<Pos.Z()<<endl;
      // OK it had been noised
      // So rotate/translate back and return:
      if (m_IsRotated == true) {
        Pos = m_InvertedRotMatrix * Pos;    // rotate
      }
      Pos += m_Position;
      return true;
    }
  }

  // Ok its only inside this volume - Let's noise
  if (m_Detector != 0) {
    //cout<<"Noise in detector: "<<m_Detector->GetName()<<endl;
    m_Detector->ApplyPulseShape(Time, Energy);
    // Rotate back:
    if (m_IsRotated == true) {
      Pos = m_InvertedRotMatrix * Pos;    // rotate
    }
    Pos += m_Position;

  } else {
    
    // Rotate back:
    if (m_IsRotated == true) {
      Pos = m_InvertedRotMatrix * Pos;    // rotate
    }
    Pos += m_Position;

    Error("MDVolume::ApplyPulseShape",
          "\nTrying to noise a hit in a not sensitive volume or detector is not initialized...: (%f,%f,%f) %s\n"
          "Did you select the correct geometry??",
          Pos.X(), Pos.Y(), Pos.Z(), (char *) m_Name.Data());

    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::Scale(const double Factor)
{
  // Scale this volume and all daughter volumes correctly

  if (m_CloneTemplate != 0 && 
      m_CloneTemplate->IsCloneTemplateVolumeWritten() == false) {
    m_Shape->Scale(Factor);
    m_CloneTemplate->SetCloneTemplateVolumeWritten(true);
  } else if (IsClone() == false) {
    m_Shape->Scale(Factor);
  }
  // The position of this volume is unchanged!

  // check the daughters:
  vector<MDVolume*>::iterator Daughters;
  for (Daughters = m_Daughters.begin(); Daughters != m_Daughters.end(); ++Daughters) {
    (*Daughters)->SetPosition((*Daughters)->GetPosition()*Factor);
    if ((*Daughters)->Scale(Factor) == false) {
      return false; 
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////

MVector MDVolume::GetPositionInMotherVolume(MVector Pos)
{
  // So rotate/translate back and return:
  if (m_IsRotated == true) {
    Pos = m_InvertedRotMatrix * Pos;    // rotate
  }
  Pos += m_Position;

  return Pos;
}

 
////////////////////////////////////////////////////////////////////////////////


MVector MDVolume::GetPositionInWorldVolume(MVector Pos)
{
  // Get a position in the world volume:
  
  MDVolume* Mother = this;
  while (Mother->GetMother() != 0) {
    Pos = Mother->GetPositionInMotherVolume(Pos);
    Mother = Mother->GetMother();
  }
  
  return Pos;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::GetVolumeSequence(MVector Pos, MDVolumeSequence* Sequence)
{
  // Fills the volume sequence pointer 
  // Return true if we are inside this volume
  // Behaviour is not defined if we are inside "MANY" volumes --- but which are anyway no longer supported...

  // Pos is in the mothers coordinate system.
  // So translate and rotate the position into this volumes coordinate system

  //mout<<"GetVSequence: Position Original: "<<m_Name<<endl;

  Pos -= m_Position;           // translate 
  
  if (m_IsRotated == true) {
    Pos = m_RotMatrix * Pos;   // rotate
  }

  // Now check, if it is inside:
  if (m_Shape->IsInside(Pos, m_Tolerance) == false) {
    return false;
  }

  //mout<<"GetVSequence: IsInside: "<<m_Name<<endl;

  // check the daughters:
  bool InsideDaughter = false;
  vector<MDVolume*>::iterator Daughters;
  for (Daughters = m_Daughters.begin(); Daughters != m_Daughters.end(); ++Daughters) {
    if ((*Daughters)->GetVolumeSequence(Pos, Sequence) == true) {
      InsideDaughter = true;
      // If this volume can only be in one other volume, we can stop if we have found the sensitive volume 
      if ((*Daughters)->IsMany() == false) break;
      // Otherwise, we can stop only if we have found the sensitive volume 
      if ((*Daughters)->IsMany() == true && Sequence->GetSensitiveVolume() != 0) break; 
    }
  }
  
  // Only if this hit is not within a daughter we can set this detector as the sensitive detector
  // Otherwise it could be in a non-sensitive or also sensitive volume within this detector
  if (InsideDaughter == false) {
    //mout<<"GetVSequence: Not inside any of the daughters"<<endl;
    if (m_IsSensitive == true && Sequence->GetSensitiveVolume() == 0) {
      //mout<<"GetVSequence: Adding sensitive volume: "<<m_Name<<endl;
      Sequence->Reset(); // <-----
      Sequence->SetSensitiveVolume(this);
      Sequence->SetPositionInSensitiveVolume(Pos);
      Sequence->SetDetector(GetDetector());
      if (GetDetector()->GetDetectorVolume() == this) { // Otherwise, we set it later on our way back up...
        Sequence->SetDetectorVolume(this);
        Sequence->SetPositionInDetector(Pos);
      }
      //cout<<m_Name<<": Setting detector: "<<GetDetector()->GetName()<<endl;
    }
  }

  // Only set the detector if this volume is one and the sequence has none!
  // This ensures if we have detectors in detectors that we stay at the deepest detector
  //cout<<m_Name<<" Is detector? "<<((m_IsDetectorVolume == true) ? "true" : "false")<<", "<<((Sequence->GetDetectorVolume() != 0) ? Sequence->GetDetectorVolume()->GetName() : "-")<<endl;
  if (m_IsDetectorVolume == true && Sequence->GetDetectorVolume() == 0) {
    //mout<<"GetVSequence: Adding sensitive detector: "<<m_Name<<":"<<GetDetector()->GetName()<<":"<<Pos<<endl;
    Sequence->SetPositionInDetector(Pos);
    Sequence->SetDetectorVolume(this);
    //cout<<m_Name<<": Setting detector: "<<GetDetector()->GetName()<<endl;
    Sequence->SetDetector(GetDetector()); // new Sep 22 2010? Clusters in calormeter didn't work. Not sure if this breaks anything...
  }  

  // We are inside this volume, so store the information: 
  Sequence->AddVolumeFront(this);
  Sequence->AddPositionFront(Pos);
  
  // If this is the world volume, we set the detector again, to get the named detectors right
  //cout<<"Check resetting of detector: "<<endl;
  if (Sequence->GetDetector() != 0 && Sequence->GetDetector()->HasNamedDetectors() == true) {
    MDDetector* D = Sequence->GetDetector()->FindNamedDetector(*Sequence);
    if (D != 0) {
      //cout<<"New named detector: "<<D->GetName()<<endl;
      Sequence->SetDetector(D);
    } else {
      //cout<<"No named detector found!"<<endl;
    }
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::GetVolumeSequenceInverse(MVector Pos, MDVolumeSequence* Sequence)
{
  // Fills the volume sequence pointer - inversely

  // Pos is inside this volume
  // So translate and rotate the position into the world coordinate system

  if (m_Shape->IsInside(Pos, m_Tolerance) == false) {
    merr<<"Position is outside this volume!"<<show;
    return false;
  }
  
  // Get a position in the world volume:
  MDVolume* Mother = this;
  while (Mother->GetMother() != 0) {
    Pos = Mother->GetPositionInMotherVolume(Pos);
    Mother = Mother->GetMother();
  }

  Sequence->Reset();
  Mother->GetVolumeSequence(Pos, Sequence);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::FindOverlaps(MVector Pos, vector<MDVolume*>& OverlappingVolumes)
{
  // Store all deepest volumes Pos is inside in OverlappingVolumes
  // If there is only one volume in OverlappingVolumes, than of course there 
  // are no overlaps...

  // Pos is in the mothers coordinate system.
  // So translate and rotate the position into this volumes coordinate system
  
  //cout<<"Checking "<<m_Name<<endl;

  double Tolerance = 0.000001;

  Pos -= m_Position;           // translate   
  if (m_IsRotated == true) {
    Pos = m_RotMatrix * Pos;   // rotate
  }

  // Now check, if it is inside:
  if (m_Shape->IsInside(Pos, Tolerance, true) == false) {
    //cout<<"Not inside "<<m_Name<<endl;
    return false;
  }

  // Now it's inside, if it is the deepest volume, then store the volume name
  // otherwise check all daughters:
  if (m_Daughters.size() == 0) {
    //cout<<"Inside "<<m_Name<<endl;
    OverlappingVolumes.push_back(this);
  } else {   
    // check the daughters:
    bool InsideDaughter = false;
    vector<MDVolume*>::iterator Daughters;
    for (Daughters = m_Daughters.begin(); Daughters != m_Daughters.end(); ++Daughters) {
      if ((*Daughters)->FindOverlaps(Pos, OverlappingVolumes) == true) {
        InsideDaughter = true;
      }
    }
    if (InsideDaughter == false) {
      //cout<<"Inside "<<m_Name<<endl;
      OverlappingVolumes.push_back(this);
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MDVolume::GetMasses(map<MDMaterial*, double>& Masses)
{
  // Calculate the mass of this volume and return the current volume:

  double Volume = m_Shape->GetVolume();

  // Calculate the mass of all daughters
  unsigned int i_max = GetNDaughters();
  for (unsigned int i = 0; i < i_max; i++) {
    Volume -= GetDaughterAt(i)->GetMasses(Masses);
  }
  
  Masses[GetMaterial()] += m_Material->GetDensity()*Volume;

  return m_Shape->GetVolume();
}


////////////////////////////////////////////////////////////////////////////////


bool MDVolume::GetNPlacements(MDVolume* Volume, vector<int>& Placements, int& TreeDepth)
{
  // Find the number of placements of a volume
  
  // Due to the limitation, each Copy is allowed to be positions in the some
  // mother than all other copies, copies are not allowed to have different
  // daughters, its enough to find one placement, to get the complete list

  MString NameThis;
  MString NameSearched;

  if (IsClone() == true) {
    NameThis = GetCloneTemplate()->GetName();
  } else {
    NameThis = GetName();
  }

  if (Volume->IsClone() == true) {
    NameSearched = Volume->GetCloneTemplate()->GetName();
  } else {
    NameSearched = Volume->GetName();
  }

  TreeDepth++;
  while (int(Placements.size()) < TreeDepth+1) {
    Placements.push_back(0);
  }

  bool Found = false;
  int NPlacements = 0;
  if (NameSearched == NameThis) {
    NPlacements = 1;
    Placements[TreeDepth] = 1;
    Found = true;
  } else {
    unsigned int i_max = GetNDaughters();
    for (unsigned int i = 0; i < i_max; ++i) {
      if (GetDaughterAt(i)->GetNPlacements(Volume, Placements, TreeDepth) == true) {
        if (GetDaughterAt(i)->IsClone()) {
          MDVolume* CloneTemplate = GetDaughterAt(i)->GetCloneTemplate();
          for (unsigned int c = 0; c < CloneTemplate->GetNClones(); ++c) {
            if (CloneTemplate->GetCloneAt(c)->GetMother()->IsVirtual() == false) {
              NPlacements++;
            }
          }
        } else {
          NPlacements = 1;
        }
        Placements[TreeDepth] += NPlacements;
        Found = true;
        break;
      }
    }
  }

  TreeDepth--;

  return Found;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDVolume::GetRandomPositionInVolume(MDVolume* Volume, vector<int>& Placements, int& TreeDepth)
{
  //! Returns a random position in any of the volumes defined by Volume

  //mout<<m_Name<<endl;

  MVector Position = g_VectorNotDefined;

  TreeDepth++;
  
  //cout<<m_Name<<": TreeDepth :"<<TreeDepth<<":"<<int(Placements.size())<<endl;
  if (TreeDepth == int(Placements.size()) - 1) {
    Position = m_Shape->GetRandomPositionInside();
  } else {
    unsigned int i_max = GetNDaughters();
    int Counter = -1;
    for (unsigned int i = 0; i < i_max; ++i) {
      if (GetDaughterAt(i)->ContainsVolume(Volume->GetName(), true) == true) {
        Counter++;
        //cout<<Counter<<":"<<Placements[TreeDepth]<<endl;
        if (Counter == Placements[TreeDepth]) {
          Position = GetDaughterAt(i)->GetRandomPositionInVolume(Volume, Placements, TreeDepth);          
          break;
        }
      }
    }
  }

  if (Position == g_VectorNotDefined) {
    mout<<"   *** Error *** in volume "<<m_Name<<endl;
    mout<<"No random position found for volume "<<Volume->GetName()<<endl;
  } else {
    if (m_IsRotated == true) {
      Position = m_InvertedRotMatrix * Position;
    }
    Position += m_Position;
  }

  TreeDepth--;

  //mout<<m_Name<<" Pos: "<<Position<<endl;

  return Position;
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::VirtualizeNonDetectorVolumes()
{
  // Check the list of daughters, those not containing sensitive detectors 
  // are virtualized

  if (ContainsDetectorVolume() == false) {
    SetVirtual(true);
    for (unsigned int c = 0; c < GetNClones(); ++c) {
      GetCloneAt(c)->SetVirtual(true);
    }
  }

  // Do the same in the daughters:
  unsigned int d_max = GetNDaughters();
  for (unsigned int d = 0; d < d_max; ++d) {
    GetDaughterAt(d)->VirtualizeNonDetectorVolumes();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::OptimizeVolumeTree()
{
  // Check the list of daughters, and place those to the top of the list
  // which contain sensitive volumes

  // Determine the daughters with detector volumes
  unsigned int d_max = GetNDaughters();
  vector<bool> Indices(d_max, false);
  for (unsigned int d = 0; d < d_max; ++d) {
    Indices[d] = GetDaughterAt(d)->ContainsDetectorVolume();
  }

  // Now rearrange them:
  for (unsigned int d = 0; d < d_max; ++d) {
    if (Indices[d] == false) {
      // Find the next detector volume
      for (unsigned int e = d+1; e < d_max; ++e) {
        if (Indices[e] == true) {
          MDVolume* V = m_Daughters[d];
          m_Daughters[d] = m_Daughters[e];
          m_Daughters[e] = V;
          
          Indices[e] = false;
          Indices[d] = true;

          break;
        }
      }
    }
    // if we are still false, we are done...
    if (Indices[d] == false) {
      break;
    }
  }

  // Do the same in the daughters:
  for (unsigned int d = 0; d < d_max; ++d) {
    GetDaughterAt(d)->OptimizeVolumeTree();
  }
}


////////////////////////////////////////////////////////////////////////////////


double MDVolume::GetAbsorptionLengths(map<MDMaterial*, double>& Lengths, 
                                      MVector Start, MVector Stop)
{
  // Start and stop are in the mothers coordinate system.
  // So translate and rotate the position into this volumes coordinate system
  
  if (m_DoAbsorptions == false) {
    return 0;
  }

  //mout<<"Testing absorption length in volume "<<m_Name<<endl;

  Start -= m_Position;           // translate 
  Stop -= m_Position;           // translate 
  if (m_IsRotated == true) {
    Start = m_RotMatrix * Start;    // rotate
    Stop = m_RotMatrix * Stop;    // rotate
  }

  double Length = 0;
  double LengthInDaughters = 0;
  const double Tolerance = 0.0000001;

  // Now check if it is inside:
  if (m_Shape->IsInside(Start, m_Tolerance) == true) {
    Length = m_Shape->DistanceInsideOut(Start, (Stop-Start).Unitize());
    //mout<<"Inside of shape: "<<m_Name<<" with length in shape: "<<Length<<endl;
    // Case A: The real distance (Stop-Start).Mag() is to short to leave it again:
    if (Length > (Stop-Start).Mag()) { 
      Length = (Stop-Start).Mag();
    } else if (Length > 0) {
      // Check if we can re-enter the volume --- necessary for tubes, etc.
      MVector OutsideStart = Start + (Length+Tolerance)*(Stop-Start).Unitize();
      double LengthOutsideIn = m_Shape->DistanceOutsideIn(OutsideStart, (Stop-OutsideStart).Unitize());
      //mout<<"Mystart: "<<MyStart[0]<<"!"<<MyStart[1]<<"!"<<MyStart[2]<<" Length: "<<MyLength<<endl;
      if (LengthOutsideIn > 0 && LengthOutsideIn < (Stop-OutsideStart).Mag()) {
        MVector SecondInsideStart = OutsideStart + (LengthOutsideIn+Tolerance)*(Stop-OutsideStart).Unitize();
        double LengthSecondInsideOut = m_Shape->DistanceInsideOut(SecondInsideStart, (Stop-SecondInsideStart).Unitize());
        //mout<<"Mystart (2): "<<MyStart[0]<<"!"<<MyStart[1]<<"!"<<MyStart[2]<<" Length: "<<MyLength<<endl;
        if (LengthSecondInsideOut > (Stop-SecondInsideStart).Mag()) { 
          Length += (Stop-SecondInsideStart).Mag();
        } else {
          Length += LengthSecondInsideOut;          
        }
      }
      //mout<<"Redetermined length in shape taking into account: re-entrable volumes: "<<Length<<endl;
    }
    //mout<<"Total length in shape: "<<Length<<endl;

  } else {
    // We are outside the volume, determine if we can reach it:
    double LengthOutsideIn = m_Shape->DistanceOutsideIn(Start, (Stop-Start).Unitize());

    if (LengthOutsideIn > 0 && LengthOutsideIn < (Stop-Start).Mag()) { // we have a path to the volume and we can reach it
      //mout<<"Distance to shape "<<m_Name<<": "<<Length<<endl;

      // Find a first position inside and then calculate the length inside:
      MVector InsideStart = Start + (LengthOutsideIn+Tolerance)*(Stop-Start).Unitize();
      double InsideLength = m_Shape->DistanceInsideOut(InsideStart, (Stop-InsideStart).Unitize());

      if (InsideLength > (Stop-InsideStart).Mag()) {
        Length = (Stop-InsideStart).Mag();
        //mout<<"Length in Shape with stop: "<<Length<<endl;
      } else {
        Length = InsideLength;
        //mout<<"Length in Shape (passing through): "<<Length<<endl;

        // Check if we can re-enter the volume --- necessary for tubes, etc.
        MVector OutsideStart = InsideStart + (InsideLength+Tolerance)*(Stop-InsideStart).Unitize();
        double LengthSecondOutsideIn = m_Shape->DistanceOutsideIn(OutsideStart, (Stop-OutsideStart).Unitize());
        //mout<<"I OutsideStart: "<<OutsideStart[0]<<"!"<<OutsideStart[1]<<"!"<<OutsideStart[2]<<"!"<<InsideLength<<endl;
        
        if (LengthSecondOutsideIn > 0 && LengthSecondOutsideIn < (Stop-OutsideStart).Mag()) { // we have a path to the volume and we can reach it
          //cout<<"We can re-enter! Second length: "<<LengthSecondOutsideIn<<endl;
        
          // Find a second position inside and then calculate the second length inside:
          MVector SecondInsideStart = OutsideStart + (LengthSecondOutsideIn+Tolerance)*(Stop-OutsideStart).Unitize();
          double SecondInsideLength = m_Shape->DistanceInsideOut(SecondInsideStart, (Stop-SecondInsideStart).Unitize());

          if (SecondInsideLength > (Stop-SecondInsideStart).Mag()) {
            Length += (Stop-SecondInsideStart).Mag();
          } else {
            Length += SecondInsideLength;
          }
        }
        //mout<<"Total length in shape: "<<Length<<endl;
      }
    } else {
      // mout<<"Outside of shape "<<m_Name<<" and no possibility to reach it!"<<endl;
      Length = 0;
    }
  }

  if (Length > 0) {
    unsigned int i_max = m_Daughters.size();
    for (unsigned int i = 0; i < i_max; i++) {
      LengthInDaughters += m_Daughters[i]->GetAbsorptionLengths(Lengths, Start, Stop);
    }
    
    if (Length - LengthInDaughters > 0) {
      Lengths[GetMaterial()] += (Length - LengthInDaughters);
    } else if (Length - LengthInDaughters < -Tolerance) {
      // Use cout
      cout<<"Warning: Negative length in volume: "<<m_Name<<": "<<Length - LengthInDaughters<<endl;
      cout<<"         Start: "<<Start<<"   Stop: "<<Stop<<endl;
      cout<<"         Total length in volume: "<<Length<<endl;
      cout<<"         Total length in daughters: "<<LengthInDaughters<<endl;
    }
  }

  return Length;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDVolume::GetNVisibleVolumes()
{
  // Return the number of visible volumes:

  unsigned int N = 0;
  if (m_Visibility > 0) N++;
  for (unsigned int i = 0; i < GetNDaughters(); i++) {
    N +=  GetDaughterAt(i)->GetNVisibleVolumes();
  }

  return N;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDVolume::GetNSensitiveVolumes()
{
  // Return the number of visible volumes:

  unsigned int N = 0;
  if (IsSensitive() == true) ++N;
  for (unsigned int d = 0; d < GetNDaughters(); ++d) {
    N += GetDaughterAt(d)->GetNSensitiveVolumes();
  }

  return N;
}


////////////////////////////////////////////////////////////////////////////////


MString MDVolume::GetGeant3DIM()
{
  // Write the Geant3 dimension information, e.g.
  // REAL VPCB2VOL
  // DIMENSION VPCB2VOL(3)

  if (m_CloneTemplate != 0) return MString("");

  ostringstream out;
  out<<"***** Volume: "<<m_Name<<endl;
  out<<m_Shape->GetGeant3DIM(m_ShortName)<<endl;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDVolume::GetGeant3DATA()
{
  // Write the Geant3 data information, e.g.
  // DATA VPCB3VOL/5.2500,6.3000,0.1000/ 

  if (m_CloneTemplate != 0) return MString("");

  ostringstream out;
  out<<m_Shape->GetGeant3DATA(m_ShortName);

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDVolume::GetGeant3()
{
  // Write the volume and rotation information, e.g.
  // CALL GSVOLU('PCB2','BOX ',6,VPCB2VOL,3,IVOL(38))
  // CALL GSROTM(8,90.0,-180.0,90.0,-90.0,0.0,0.0)

  ostringstream out;
  out.setf(ios::fixed, ios::floatfield);
  //out.precision(1);

  // Do not write the rotation information if the clone template has already been 
  // written and this rotations ID is identical with the master id:
  if (m_CloneTemplate != 0 && 
      m_CloneTemplate->IsCloneTemplateVolumeWritten() == true && 
      m_RotID == m_CloneTemplate->GetRotationID()) {
    // nothing
  } else {
    if (m_IsRotated == true && m_RotID > 0) {
      out<<"***** Name: "<<m_Name<<endl;
      out<<"      CALL GSROTM("<<m_RotID<<","<<m_Theta1<<","<<m_Phi1<<","
         <<m_Theta2<<","<<m_Phi2<<","<<m_Theta3<<","<<m_Phi3<<")"<<endl;
    }
  }

  // Write the volume information when
  // + it is no clone  OR
  // + it is a clone but the template has not yet been written
  if (m_CloneTemplate == 0 ||
      (m_CloneTemplate != 0 && m_CloneTemplate->IsCloneTemplateVolumeWritten() == false)) {
    // Write volume information:
    out<<"      CALL GSVOLU('"<<m_ShortName<<"','"<<m_Shape->GetGeant3ShapeName()<<"',"<<
      m_Material->GetID()<<",V"<<m_ShortName<<"VOL,"<<
      m_Shape->GetGeant3NumberOfParameters()<<",IVOL("<<m_ID<<"))"<<endl;

    // Write possible divisions:
    if (m_Detector != 0) {
      out<<m_Detector->GetGeant3Divisions();
    }

    // Advance into the volume tree:
    for (unsigned int i = 0; i < GetNDaughters(); i++) {
      out<<GetDaughterAt(i)->GetGeant3();
    }

    if (m_CloneTemplate != 0 && m_CloneTemplate->IsCloneTemplateVolumeWritten() == false) {
      m_CloneTemplate->SetCloneTemplateVolumeWritten(true);
    }
  }

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDVolume::GetMGeant()
{
  // Write the volume information in MGEANT/mggpod format.

  ostringstream out;
  out.setf(ios::fixed, ios::floatfield);
  //out.precision(1);

  // Do not write the rotation information if the clone template has already been 
  // written and this rotations ID is identical with the master id:
  if (m_CloneTemplate != 0 && 
      m_CloneTemplate->IsCloneTemplateVolumeWritten() == true && 
      m_RotID == m_CloneTemplate->GetRotationID()) {
    // nothing
  } else {
    if (m_IsRotated == true && m_RotID > 0) {
      out << endl;
      if (m_CloneTemplate == 0) {
        out << "! Rotation belongs to : " << m_Name << endl;
      } else {
        out << "! Rotation belongs to : " << m_CloneTemplate->GetName() << endl;
      }
      out << "rotm " << m_RotID << " " << m_Theta1 << " " << m_Phi1 << " "
          << m_Theta2 << " " << m_Phi2 << " " << m_Theta3 << " " << m_Phi3 << endl;
    }
  }

  // Write the volume information when
  // + it is no clone  OR
  // + it is a clone but the template has not yet been written
  out.precision(3);
  if (m_CloneTemplate == 0 ||
      (m_CloneTemplate != 0 && m_CloneTemplate->IsCloneTemplateVolumeWritten() == false)) {
    // Write volume information:
    out << endl;
    if (m_CloneTemplate == 0) {
      out << "! Geomega volume name: " << m_Name << endl;
    } else {
      out << "! Geomega volume name: " << m_CloneTemplate->GetName() << endl;
    }

    MString VolumeName = m_ShortName;
    VolumeName.ToUpper();
    MString MaterialName = m_Material->GetMGeantShortName();
    MaterialName.ToUpper();

    out << "volu " << VolumeName << " " << m_Shape->GetGeant3ShapeName() << " "
        << MaterialName << " " << m_Shape->GetGeant3NumberOfParameters() << endl;
    out << m_Shape->GetMGeantDATA(m_ShortName);
    out << "satt " << VolumeName << " SEEN " << m_Visibility << endl;
    
    // Write possible divisions:
    if (m_Detector != 0 && m_IsSensitive == true) {
      out<<m_Detector->GetMGeantDivisions();
    }

    // Advance into the volume tree:
    for (unsigned int i = 0; i < GetNDaughters(); i++) {
      out << GetDaughterAt(i)->GetMGeant();
    }

    if (m_CloneTemplate != 0 && m_CloneTemplate->IsCloneTemplateVolumeWritten() == false) {
      m_CloneTemplate->SetCloneTemplateVolumeWritten(true);
    }
  }

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::ResetCloneTemplateFlags()
{
  // Reset all clone template written falgs:
  // To be calles before any Geant3/MGeant files are written

  m_CloneTemplateVolumeWritten = false;
  m_CloneTemplateDaughtersWritten = false;
  m_CloneTemplateId = 0;
  m_WrittenRotID = 1;

  if (m_CloneTemplate != 0) {
    m_CloneTemplate->SetCloneTemplateVolumeWritten(false);
    m_CloneTemplate->SetCloneTemplateDaughtersWritten(false);
    m_CloneTemplate->m_CloneTemplateId = 0;
  }

  // Do this for all clones - shouldn't be necessary 
  for (unsigned int i = 0; i < GetNClones(); ++i) {
    GetCloneAt(i)->SetCloneTemplateVolumeWritten(false);      
    GetCloneAt(i)->SetCloneTemplateDaughtersWritten(false);
  }

  // ... and all daughters 
  for (unsigned int i = 0; i < GetNDaughters(); i++) {
    GetDaughterAt(i)->ResetCloneTemplateFlags();
  }
}


////////////////////////////////////////////////////////////////////////////////


MString MDVolume::GetMGeantPosition(int& IDCounter)
{
  // Write the volume position information in MGEANT/mggpod format.

  ostringstream out;
  out.setf(ios::fixed, ios::floatfield);
  //out.precision(3);

  MDVolume* VC;
  int ID = 0;
  MString Name, MotherName, CopyName;

  if ((VC = GetCloneTemplate()) != 0) {
      
    ID = GetCloneTemplate()->GetCloneId(this);
    
    Name = GetShortName();
    Name.ToUpper();
    CopyName = GetShortName();
    CopyName.ToUpper();
    MotherName = GetMother()->GetShortName();
    MotherName.ToUpper();

    out<<endl;
    out<<"! Positioning "<<GetCloneTemplate()->GetName()<<" in "<<GetMother()->GetName()<<":"<<endl;
    out<<"posi "<<CopyName<<" "<<ID<<" "<< MotherName<<" "
       <<GetPosition().X()<<" "<<GetPosition().Y()<<" "<< GetPosition().Z()<<" " 
       <<GetRotationID();
    if (m_IsMany == false) {
      out<<" ONLY"<<endl;
    } else {
      out<<" MANY"<<endl;
    }
    
    if (GetCloneTemplate()->AreCloneTemplateDaughtersWritten() == false) {
      for (unsigned int i = 0; i < GetNDaughters(); i++) {
        out<<GetDaughterAt(i)->GetMGeantPosition(IDCounter);
      }
      GetCloneTemplate()->SetCloneTemplateDaughtersWritten(true);
    }
  } else {
    // If it is no copy then position it only when it has a mother or is the root volume
    if (GetMother() !=  0) {
      
      //ID = IDCounter++;
      ID = 1;

      Name = GetShortName();
      Name.ToUpper();
      MotherName = GetMother()->GetShortName();
      MotherName.ToUpper();
      
      out<<endl;
      out<<"! Positioning "<<m_Name<<" in "<<GetMother()->GetName()<< ":"<<endl;
      out<<"posi "<<Name<<" "<<ID<<" "<<MotherName<<" " 
         <<GetPosition().X()<<" "<<GetPosition().Y()<<" "<<GetPosition().Z()<<" "
         <<GetRotationID();
      if (m_IsMany == false) {
        out<<" ONLY"<<endl;
      } else {
        out<<" MANY"<<endl;
      }
    }

    // Do the same for all daughters:
    for (unsigned int i = 0; i < GetNDaughters(); i++) {
      out<<GetDaughterAt(i)->GetMGeantPosition(IDCounter);
    }
  }
         
  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDVolume::GetGeant3Position(int& IDCounter)
{
  ostringstream out;
  out.setf(ios::fixed, ios::floatfield);
  //out.precision(1);

  //cout<<"Looking at volume: "<<m_Name<<endl;
  MDVolume *VC;
  int ID;
  MString Name, MotherName, CopyName;

  if ((VC = GetCloneTemplate()) != 0) {
      
    //cout<<VC->GetShortName()<<"="<<VC->GetName()<<"!"<<GetMother()->GetShortName()<<"="<<GetMother()->GetName()<<endl;
    // If the volume is a copy, then reposition the CopyOf volume:
    ID = IDCounter++;
    //ID = GetID();
    
    Name = GetShortName();
    CopyName = GetShortName();
    MotherName = GetMother()->GetShortName();
    
    out<<"      CALL GSPOS('"<<CopyName<<"',"<<ID<<",'"<<MotherName
       <<"',"<<GetPosition().X()<<","<<GetPosition().Y()<<","
       <<GetPosition().Z()<<","<<GetRotationID();
    if (m_IsMany == false) {
      out<<",'ONLY')"<<endl;
    } else {
      out<<",'MANY')"<<endl;
    }
    
    if (GetCloneTemplate()->AreCloneTemplateDaughtersWritten() == false) {
      for (unsigned int i = 0; i < GetNDaughters(); i++) {
        out<<GetDaughterAt(i)->GetGeant3Position(IDCounter);
      }
      GetCloneTemplate()->SetCloneTemplateDaughtersWritten(true);
    }
  } else {
    // If it is no copy then position it only when it has a mother or is the root volume
    if (GetMother() !=  0) {
      //cout<<"   is root"<<endl;
      
      ID = IDCounter++;

      Name = GetShortName();
      MotherName = GetMother()->GetShortName();
      
      out<<"      CALL GSPOS('"<<Name<<"',"<<ID<<",'"<<MotherName
         <<"',"<<GetPosition().X()<<","<<GetPosition().Y()<<","
         <<GetPosition().Z()<<","<<GetRotationID();
      if (m_IsMany == false) {
        out<<",'ONLY')"<<endl<<endl;
      } else {
        out<<",'MANY')"<<endl<<endl;
      }
    } else {
      //cout<<"   is ignored..."<<endl;
    }

    // Do the same for all daughters:
    for (unsigned int i = 0; i < GetNDaughters(); i++) {
      out<<GetDaughterAt(i)->GetGeant3Position(IDCounter);
    }
  }
  
  

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDVolume::GetGeomega()
{
  // Return the volume as Geomega type volume tree

  mimp<<"This function does not contain the full possible functionality!"<<show;

  ostringstream out;

  bool WriteDaughters = false;
  MDVolume* Template = 0;
  // If multiple clones of this volume exist...
  if ((Template = GetCloneTemplate()) != 0) {

    // If the clones template has not yet been written
    if (Template->IsCloneTemplateVolumeWritten() == false) {
      // Write first the template, later *once* its daughters...
      out<<"Volume "<<Template->GetName()<<endl;
      if (Template->GetShape() != 0) {
        out<<Template->GetName()<<".Shape "<<Template->GetShape()->GetGeomega()<<endl;
      }
      if (Template->GetMaterial() != 0) {
        out<<Template->GetName()<<".Material "<<Template->GetMaterial()->GetName()<<endl;
      }
      m_CloneTemplate->SetCloneTemplateVolumeWritten(true);
      WriteDaughters = true;
    }

    out<<Template->GetName()<<".Copy "<<GetName()<<endl;
  } else {
    WriteDaughters = true;
    out<<"Volume "<<GetName()<<endl;
    if (GetShape() != 0) {
      out<<GetName()<<".Shape "<<GetShape()->GetGeomega()<<endl;
    }
    if (GetMaterial() != 0) {
      out<<GetName()<<".Material "<<GetMaterial()->GetName()<<endl;
    }
  }
  if (IsVirtual() == true) {
    out<<GetName()<<".Virtual true"<<endl;
  } else if (IsMany() == true) {
    out<<GetName()<<".Many true"<<endl;
  }
  out<<GetName()<<".Visibility "<<GetVisibility()<<endl;
  out<<GetName()<<".Color "<<GetColor()<<endl;
  out<<GetName()<<".Position "<<GetPosition().X()<<" "<<GetPosition().Y()<<" "<<GetPosition().Z()<<endl;
  out<<GetName()<<".Rotation "<<m_Theta1<<" "<<m_Phi1<<" "<<m_Theta2<<" "<<m_Phi2<<" "<<m_Theta3<<" "<<m_Phi3<<" "<<endl;
  if (GetMother() != 0) {
    out<<GetName()<<".Mother "<<GetMother()->GetName()<<endl;
  } else {
    out<<GetName()<<".Mother 0"<<endl;
  }
  out<<endl;

  if (WriteDaughters == true) {
    for (unsigned int i = 0; i < GetNDaughters(); ++i) {
      out<<GetDaughterAt(i)->GetGeomega()<<endl;
    }
  }

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDVolume::ToStringVolumeTree(int Level)
{
  MString Text;
  for (int i = 0; i < Level; ++i) {
    Text += "  ";
  }
  Text += m_Name;
  if (m_Detector != 0) {
    Text += " (";
    Text += m_Detector->GetName();
    Text += ")";    
  }
  Text += " - Vis: ";
  Text += m_Visibility;
  Text += "\n";

  for (unsigned int i = 0; i < GetNDaughters(); i++) {
    Text += GetDaughterAt(i)->ToStringVolumeTree(Level+1);
  }

  return Text;
}


////////////////////////////////////////////////////////////////////////////////


MString MDVolume::ToString(bool Recursive)
{
  // Return the essential data of this class

  ostringstream out;

  out<<"Volume "<<m_Name<<endl; //" ("<<m_ShortName<<")"<<endl;
  if (m_Shape != 0) { 
    out<<"   Shape: "<<m_Shape->ToString();
  } else {
    out<<"   Shape not defined!!"<<endl;
  }
  if (m_Material != 0) { 
    out<<"   Material: "<<m_Material->GetName()<<endl;
  } else {
    out<<"   Material not defined!!"<<endl;
  }
  out<<"   Position: "<<m_Position.X()<<", "<<m_Position.Y()<<", "<<m_Position.Z()<<endl;
  if (m_WorldVolume == false) {
    if (m_Mother == 0) {
      out<<"   No mother volume defined!"<<endl;
    } else {
      out<<"   Mother volume: "<<m_Mother->GetName()<<endl;
    }
  } else {
    out<<"   World volume!"<<endl;
  }
  if (m_IsVirtual == true) {
    out<<"   This is a virtual volume!"<<endl;
  }
  out<<"   Visibility: "<<m_Visibility<<endl;

  if (Recursive == true) {
    for (unsigned int i = 0; i < GetNDaughters(); i++) {
      out<<GetDaughterAt(i)->ToString(true)<<endl;
    }
  }

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


void MDVolume::ResetIDs()
{
  //

  MDVolume::m_IDCounter = 1; 
  MDVolume::m_RotIDCounter = 1;
}


// MDVolume.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
