/*
 * MCCrossSections.cxx
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
#include "MCCrossSections.hh"
#include "MCRunManager.hh"
#include "MCDetectorConstruction.hh"

// MEGAlib:
#include "MGlobal.h"
#include "MStreams.h"

// Geant4:
#include "G4SystemOfUnits.hh"
#include "G4CrossSectionHandler.hh"
#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4Version.hh"

// ROOT:
#include "TSystem.h"

// Standard lib:
#include <fstream>
#include <iostream>
#include <iomanip>
using namespace std;


/******************************************************************************
 * Not yet implemented
 */
MCCrossSections::MCCrossSections()
{
  // Intentionally left blank
}


/******************************************************************************
 * Not yet implemented
 */
MCCrossSections::~MCCrossSections()
{
  // Intentionally left blank
}


/******************************************************************************
 * Not yet implemented
 */
bool MCCrossSections::CreateCrossSectionFiles(MString Path)
{
  // Intentionally left blank

  // Make sure the path exists
  gSystem->mkdir(Path, true);

  G4CrossSectionHandler* CrossSectionHandlerPhoto = new G4CrossSectionHandler;
  CrossSectionHandlerPhoto->Clear();
  CrossSectionHandlerPhoto->LoadData("phot/pe-cs-");

  G4CrossSectionHandler* CrossSectionHandlerRayleigh = new G4CrossSectionHandler;
  CrossSectionHandlerRayleigh->Clear();
  CrossSectionHandlerRayleigh->LoadData("rayl/re-cs-");

  G4CrossSectionHandler* CrossSectionHandlerCompton = new G4CrossSectionHandler;
  CrossSectionHandlerCompton->Clear();
  CrossSectionHandlerCompton->LoadData("comp/ce-cs-");

  G4CrossSectionHandler* CrossSectionHandlerPair = new G4CrossSectionHandler;
  CrossSectionHandlerPair->Clear();
  CrossSectionHandlerPair->LoadData("pair/pp-cs-");

  // Loop over all materials and write the cross-section files
  double eMin = 1*keV;
  double eMax = 100*GeV;
  double eBins = 400;
  
  eMin = log(eMin);
  eMax = log(eMax);
  double eDist = (eMax-eMin)/eBins;

  MString File;
  const G4MaterialTable* Table = G4Material::GetMaterialTable();
  //const G4ProductionCutsTable* Table = G4ProductionCutsTable::GetProductionCutsTable();

  for (unsigned int mat = 0; mat < Table->size(); ++mat) {
    //const G4Material* M = Table->GetMaterialCutsCouple(mat)->GetMaterial();
    const G4Material* M = Table->at(mat);

    unsigned long Hash = MCRunManager::GetMCRunManager()->GetDetectorConstruction()->GetMaterialHash(M);

    ofstream out;
    out.setf(ios_base::fixed, ios_base::floatfield);
    out.precision(6);

    File = Path + "/Xsection.Photo." + M->GetName().c_str() + ".rsp";
    out.open(File);
    if (out.is_open() == true) {
      out<<"# Macroscopic cross section for Photo absorption"<<endl;
      out<<"# Derived from Geant4 version "<<G4VERSION_NUMBER<<endl;
      out<<"# X: Energy in keV"<<endl;
      out<<"# Y: Macroscopic cross section in 1/cm"<<endl;
      out<<endl; 
      out<<"Type    ResponseMatrixO1"<<endl;
      out<<"Version 1"<<endl;
      out<<endl;
      out<<"NM Photo absorption cross section for "<<M->GetName()<<endl;
      out<<"CE false"<<endl;
      out<<"HA "<<Hash<<endl;
      out<<endl;
      for (int e = 0; e <= eBins; ++e) {
        out<<"R1 "
           <<fixed<<setw(16)<<exp(eMin+e*eDist)/keV<<" "
           <<scientific<<CrossSectionHandlerPhoto->ValueForMaterial(M, exp(eMin+e*eDist))/(1/cm)<<endl;
      }
      out<<endl;
    } else {
      merr<<"Unable to open file: "<<File<<endl;
    } 
    out.close();


    File = Path + "/Xsection.Rayleigh." + M->GetName().c_str() + ".rsp";
    out.open(File);
    if (out.is_open() == true) {
      out<<"# Macroscopic cross section for Rayleigh scattering"<<endl;
      out<<"# Derived from Geant4 version "<<G4VERSION_NUMBER<<endl;
      out<<"# X: Energy in keV"<<endl;
      out<<"# Y: Macroscopic cross section in 1/cm"<<endl; 
      out<<endl; 
      out<<"Type    ResponseMatrixO1"<<endl;
      out<<"Version 1"<<endl;
      out<<endl;
      out<<"NM Rayleigh scattering cross section for "<<M->GetName()<<endl;
      out<<"CE false"<<endl;
      out<<"HA "<<Hash<<endl;
      out<<endl;
      for (int e = 0; e <= eBins; ++e) {
        out<<"R1 "
           <<fixed<<setw(16)<<exp(eMin+e*eDist)/keV<<" "
           <<scientific<<CrossSectionHandlerRayleigh->ValueForMaterial(M, exp(eMin+e*eDist))/(1/cm)<<endl;
      }
      out<<endl;
    } else {
      merr<<"Unable to open file: "<<File<<endl;
    } 
    out.close();


    File = Path + "/Xsection.Compton." + M->GetName().c_str() + ".rsp";
    out.open(File);
    if (out.is_open() == true) {
      out<<"# Macroscopic cross section for Compton scattering"<<endl;
      out<<"# Derived from Geant4 version "<<G4VERSION_NUMBER<<endl;
      out<<"# X: Energy in keV"<<endl;
      out<<"# Y: Macroscopic cross section in 1/cm"<<endl;
      out<<endl; 
      out<<"Type    ResponseMatrixO1"<<endl;
      out<<"Version 1"<<endl;
      out<<endl;
      out<<"NM Compton scattering cross section for "<<M->GetName()<<endl;
      out<<"CE false"<<endl;
      out<<"HA "<<Hash<<endl;
      out<<endl;
      for (int e = 0; e <= eBins; ++e) {
        out<<"R1 "
           <<fixed<<setw(16)<<exp(eMin+e*eDist)/keV<<" "
           <<scientific<<CrossSectionHandlerCompton->ValueForMaterial(M, exp(eMin+e*eDist))/(1/cm)<<endl;
      }
      out<<endl;
    } else {
      merr<<"Unable to open file: "<<File<<endl;
    } 
    out.close();


    File = Path + "/Xsection.Pair." + M->GetName().c_str() + ".rsp";
    out.open(File);
    if (out.is_open() == true) {
      out<<"# Macroscopic cross section for pair creation"<<endl;
      out<<"# Derived from Geant4 version "<<G4VERSION_NUMBER<<endl;
      out<<"# X: Energy in keV"<<endl;
      out<<"# Y: Macroscopic cross section in 1/cm"<<endl;
      out<<endl; 
      out<<"Type    ResponseMatrixO1"<<endl;
      out<<"Version 1"<<endl;
      out<<endl;
      out<<"NM Pair creation cross section for "<<M->GetName()<<endl;
      out<<"CE false"<<endl;
      out<<"HA "<<Hash<<endl;
      out<<endl;
      for (int e = 0; e <= eBins; ++e) {
        out<<"R1 "
           <<fixed<<setw(16)<<exp(eMin+e*eDist)/keV<<" "
           <<scientific<<CrossSectionHandlerPair->ValueForMaterial(M, exp(eMin+e*eDist))/(1/cm)<<endl;
      }
      out<<endl;
    } else {
      merr<<"Unable to open file: "<<File<<endl;
    } 
    out.close();


    File = Path + "/Xsection.Total." + M->GetName().c_str() + ".rsp";
    out.open(File);
    if (out.is_open() == true) {
      out<<"# Macroscopic cross section (total)"<<endl;
      out<<"# Derived from Geant4 version "<<G4VERSION_NUMBER<<endl;
      out<<"# X: Energy in keV"<<endl;
      out<<"# Y: Macroscopic cross section in 1/cm"<<endl;
      out<<endl;
      out<<"Type    ResponseMatrixO1"<<endl;
      out<<"Version 1"<<endl;
      out<<endl;
      out<<"NM Total cross section for "<<M->GetName()<<endl;
      out<<"CE false"<<endl;
      out<<"HA "<<Hash<<endl;
      out<<endl;
      for (int e = 0; e <= eBins; ++e) {
        out<<"R1 "
           <<fixed<<setw(16)<<exp(eMin+e*eDist)/keV<<" "
           <<scientific<<(CrossSectionHandlerPhoto->ValueForMaterial(M, exp(eMin+e*eDist)) + 
                          CrossSectionHandlerRayleigh->ValueForMaterial(M, exp(eMin+e*eDist)) + 
                          CrossSectionHandlerCompton->ValueForMaterial(M, exp(eMin+e*eDist)) + 
                          CrossSectionHandlerPair->ValueForMaterial(M, exp(eMin+e*eDist)))/(1/cm)<<endl;
      }
      out<<endl;
    } else {
      merr<<"Unable to open file: "<<File<<endl;
    } 
    out.close();
  }

  return true;
}


/*
 * MCCrossSections.cc: the end...
 ******************************************************************************/
