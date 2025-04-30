/*
 * MDGDMLImport.cxx
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


// Include the header:
#include "MDGDMLImport.h"

// Standard libs:

// ROOT libs:
#include "TSystem.h"
#include "TList.h"
#include "TGeoBBox.h"
#include "TGeoNode.h"

// MEGAlib libs:
#include "MStreams.h"
#include "MDShapeBRIK.h"
#include "MDMaterial.h"
#include "MDShape.h"
#include "MDVolume.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDGDMLImport)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MDGDMLImport::MDGDMLImport() : m_GeoManager(nullptr)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MDGDMLImport::~MDGDMLImport()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the GDMl file
bool MDGDMLImport::Parse(MString FileName)
{
  if (gSystem->AccessPathName(FileName) == 1) {
    mout<<"   ***  Error  ***  "<<endl;
    mout<<"File to import \""<<FileName<<"\" does not exist."<<endl;
    return false;
  }
  
  TGeoManager* Dummy = new TGeoManager("GDMLImport", "GDMLImport");
  m_GeoManager = Dummy->Import(FileName);
  

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the data as Geomega file
MString MDGDMLImport::GetAsGeomega()
{
  if (m_GeoManager == nullptr) {
    return MString("");
  }
  
  cout<<"Converting"<<endl;
  
  
  
  // Get a list of materials:
  vector<MDMaterial*> Materials;
  cout<<"Converting "<<m_GeoManager->GetListOfMaterials()->GetEntries()<<" materials"<<endl;
  TIter NextMaterial(m_GeoManager->GetListOfMaterials());
  while (TObject* O = NextMaterial()) {
    TGeoMaterial* M = dynamic_cast<TGeoMaterial*>(O);
    if (M == nullptr) {
      mout<<"   ***  Error  ***  "<<endl;
      mout<<"Unable to cast a material."<<endl;
      return false;
    }
    cout<<M->ClassName()<<": "<<M->GetName()<<endl;
  }
  
  // Get a list of shapes:
  vector<MDShape*> Shapes;
  cout<<"Converting "<<m_GeoManager->GetListOfShapes()->GetEntries()<<" shapes"<<endl;
  TIter NextShape(m_GeoManager->GetListOfShapes());
  while (TObject* O = NextShape()) {
    MString ClassName = O->ClassName();
    if (ClassName == "TGeoBBox") {
      MDShapeBRIK* BRIK = new MDShapeBRIK(O->GetName());
      BRIK->Set(dynamic_cast<TGeoBBox*>(O));
      Shapes.push_back(BRIK);
      cout<<BRIK->GetGeomega()<<endl;
    } else {
      mout<<"   ***  Error  ***  "<<endl;
      mout<<"Unknown shape "<<ClassName<<endl;
      return false;    }
  }
  
  // Get a list of volumes - round one
  vector<MDVolume*> Volumes;
  cout<<"Converting "<<m_GeoManager->GetListOfVolumes()->GetEntries()<<" volumes"<<endl;
  TIter NextVolume(m_GeoManager->GetListOfVolumes());
  while (TObject* O = NextVolume()) {
    TGeoVolume* GeoVolume = dynamic_cast<TGeoVolume*>(O);
    if (GeoVolume == nullptr) {
      mout<<"   ***  Error  ***  "<<endl;
      mout<<"Unable to cast a volume."<<endl;
      return false;
    }
    MDVolume* Volume = new MDVolume(GeoVolume->GetName());
    Volumes.push_back(Volume);
  }
  
  TIter NextVolume2(m_GeoManager->GetListOfVolumes());
  while (TObject* O = NextVolume2()) {
    TGeoVolume* GeoVolume = dynamic_cast<TGeoVolume*>(O);
    
    auto Iter = find_if(Volumes.begin(), Volumes.end(), [GeoVolume](MDVolume* NV) { return NV->GetName() == GeoVolume->GetName(); } );
    MDVolume* Volume = (*Iter);
    cout<<"Re-found: "<<Volume->GetName()<<endl;
    
  }
  
  /*
    cout<<V->ClassName()<<": "<<V->GetName()<<endl;
    
    
    V->Dump();
    cout<<"Number of nodes: "<<V->GetNtotal()<<endl;
    
    TIter NextNode(V->GetNodes());
    while (TObject* N = NextNode()) {
      TGeoNode* Node = dynamic_cast<TGeoNode*>(N);
      TGeoVolume* Mother  = Node->GetMotherVolume();
      if (Mother != nullptr) {
        cout<<"\"mother\": "<<Mother->GetName()<<endl;
        cout<<"\"this\": "<< Node->GetVolume()->GetName()<<endl;
      }
    }
  }
  */
  
  return "";
}


// MDGDMLImport.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
