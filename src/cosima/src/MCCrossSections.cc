/*
 * MCCrossSections.cc
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
#include "G4LivermorePhotoElectricModel.hh"
#include "G4LivermoreRayleighModel.hh"
#include "G4LivermoreGammaConversionModel.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4DataVector.hh"
#include "G4Element.hh"

// ROOT:
#include "TSystem.h"

// Standard lib:
#include <algorithm>
#include <cmath>
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
 * Create the five macroscopic cross-section tables for each material in Path
 */
bool MCCrossSections::CreateCrossSectionFiles(MString Path)
{
  // Write ResponseMatrixO1 tables in keV and 1/cm, with the material hash in HA
  // Use one energy grid for all five processes from 1 keV to 100 GeV
  // Bracket shell edges at relative offsets of 1e-5, including trace components
  // Refine at 31 interior samples to 0.09%, targeting 0.1% between samples
  // Check photoelectric/Compton above 1e-6 of the total, Rayleigh/pair above 1e-3, and the total always
  // Edge brackets and model-step intervals below 3e-5 relative width are exempt
  // Sampling does not prove the tolerance for arbitrary model data
  // Write energies with six decimals in keV and values with seven significant digits

  // Make sure the path exists
  gSystem->mkdir(Path, true);

  
  // Get the G4ParticleTable
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();

  // Get the G4ParticleDefinition for a photon
  G4ParticleDefinition* gamma = particleTable->FindParticle("gamma");
    
  G4CrossSectionHandler* CrossSectionHandlerCompton = new G4CrossSectionHandler;
  CrossSectionHandlerCompton->Clear();
  CrossSectionHandlerCompton->LoadData("comp/ce-cs-");

  G4LivermorePhotoElectricModel* CrossSectionHandlerPhoto = new G4LivermorePhotoElectricModel;
  G4DataVector Data;
  CrossSectionHandlerPhoto->Initialise(gamma,Data);
 
  
  //G4CrossSectionHandler* CrossSectionHandlerPhoto = new G4CrossSectionHandler;
  //CrossSectionHandlerPhoto->Clear();
  //CrossSectionHandlerPhoto->LoadData("phot/pe-cs-");

  G4LivermoreRayleighModel* CrossSectionHandlerRayleigh = new G4LivermoreRayleighModel;
  //G4CrossSectionHandler* CrossSectionHandlerRayleigh = new G4CrossSectionHandler;
  //CrossSectionHandlerRayleigh->Clear();
  //CrossSectionHandlerRayleigh->LoadData("rayl/re-cs-");

  G4LivermoreGammaConversionModel*  CrossSectionHandlerPair = new G4LivermoreGammaConversionModel;
  //G4CrossSectionHandler* CrossSectionHandlerPair = new G4CrossSectionHandler;
  //CrossSectionHandlerPair->Clear();
  //CrossSectionHandlerPair->LoadData("pair/pp-cs-");

  cout<<"Cross-section Data loaded !"<<endl;

  // Loop over all materials and write the cross-section files
  const double eMin = 1*keV;
  const double eMax = 100*GeV;
  const int eBins = 400;

  MString File;
  const G4MaterialTable* Table = G4Material::GetMaterialTable();
  //const G4ProductionCutsTable* Table = G4ProductionCutsTable::GetProductionCutsTable();

  

  for (unsigned int mat = 0; mat < Table->size(); ++mat) {
    //const G4Material* M = Table->GetMaterialCutsCouple(mat)->GetMaterial();
    const G4Material* M = Table->at(mat);

    unsigned long Hash = MCRunManager::GetMCRunManager()->GetDetectorConstruction()->GetMaterialHash(M);

    // Use the same energy grid for all processes:
    vector<double> Energies = GetEnergies(M, gamma, CrossSectionHandlerPhoto, CrossSectionHandlerRayleigh,
                                          CrossSectionHandlerCompton, CrossSectionHandlerPair, eMin, eMax, eBins);

    ofstream out;
    out.imbue(locale::classic());
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
      for (double Energy : Energies) {
        out<<"R1 "
           <<fixed<<setw(16)<<Energy/keV<<" "
           <<scientific<<CrossSectionHandlerPhoto->CrossSectionPerVolume(M, gamma,Energy)/(1/cm)<<endl;
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
      for (double Energy : Energies) {
        out<<"R1 "
           <<fixed<<setw(16)<<Energy/keV<<" "
           <<scientific<<CrossSectionHandlerRayleigh->CrossSectionPerVolume(M,gamma, Energy)/(1/cm)<<endl;
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
      for (double Energy : Energies) {
        out<<"R1 "
           <<fixed<<setw(16)<<Energy/keV<<" "
           <<scientific<<CrossSectionHandlerCompton->ValueForMaterial(M, Energy)/(1/cm)<<endl;
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
      for (double Energy : Energies) {
        out<<"R1 "
           <<fixed<<setw(16)<<Energy/keV<<" "
           <<scientific<<CrossSectionHandlerPair->CrossSectionPerVolume(M, gamma,Energy)/(1/cm)<<endl;
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
      for (double Energy : Energies) {
        out<<"R1 "
           <<fixed<<setw(16)<<Energy/keV<<" "
           <<scientific<<(CrossSectionHandlerPhoto->CrossSectionPerVolume(M, gamma,Energy) +
                          CrossSectionHandlerRayleigh->CrossSectionPerVolume(M,gamma ,Energy) +
                          CrossSectionHandlerCompton->ValueForMaterial(M, Energy) +
                          CrossSectionHandlerPair->CrossSectionPerVolume(M, gamma,Energy))/(1/cm)<<endl;
      }
      out<<endl;
    } else {
      merr<<"Unable to open file: "<<File<<endl;
    } 
    out.close();
  }

  return true;
}


/******************************************************************************
 * Return a logarithmic energy grid with shell-edge samples and adaptive refinement
 */
vector<double> MCCrossSections::GetEnergies(const G4Material* Material, const G4ParticleDefinition* Gamma,
                                            G4LivermorePhotoElectricModel* Photo, G4LivermoreRayleighModel* Rayleigh,
                                            G4CrossSectionHandler* Compton, G4LivermoreGammaConversionModel* Pair,
                                            double MinimumEnergy, double MaximumEnergy, int Bins)
{
  // (1) Keep the endpoints exact - rounding can cross an absorption edge
  vector<double> Energies;
  for (int e = 0; e < Bins; ++e) {
    Energies.push_back(MinimumEnergy*pow(MaximumEnergy/MinimumEnergy, double(e)/Bins));
  }
  Energies.push_back(MaximumEnergy);

  // (2) Bracket the shell edges from the model:
  // Keep both sides distinct after text output and float conversion
  const double Distance = 1E-5;
  const G4ElementVector* Elements = Material->GetElementVector();
  for (size_t i = 0; i < Material->GetNumberOfElements(); ++i) {
    const int Z = int((*Elements)[i]->GetZ() + 0.5);
    for (int s = 0; ; ++s) {
      const double Edge = Photo->GetBindingEnergy(Z, s);
      if (Edge < 0.0) break; // no more shells
      // Include edges at the table boundaries:
      if (Edge < MinimumEnergy || Edge > MaximumEnergy) continue;
      // Keep only the edge samples inside the table:
      if (Edge*(1.0 - Distance) > MinimumEnergy) Energies.push_back(Edge*(1.0 - Distance));
      if (Edge*(1.0 + Distance) < MaximumEnergy) Energies.push_back(Edge*(1.0 + Distance));
    }
  }

  // Sort and merge nearby energies:
  sort(Energies.begin(), Energies.end());
  vector<double> Unique;
  for (double E : Energies) {
    if (Unique.empty() == true || E/Unique.back() - 1.0 > 0.1*Distance) {
      Unique.push_back(E);
    }
  }

  // (3) Refine the intervals for linear interpolation:
  vector<double> Refined;
  for (unsigned int i = 0; i + 1 < Unique.size(); ++i) {
    Refined.push_back(Unique[i]);
    array<double, 5> LowValues = GetCrossSections(Material, Gamma, Photo, Rayleigh, Compton, Pair, Unique[i]);
    array<double, 5> HighValues = GetCrossSections(Material, Gamma, Photo, Rayleigh, Compton, Pair, Unique[i+1]);
    Refine(Material, Gamma, Photo, Rayleigh, Compton, Pair, Unique[i], Unique[i+1], LowValues, HighValues, Refined);
  }
  Refined.push_back(Unique.back());

  return Refined;
}


/******************************************************************************
 * Return photoelectric, Rayleigh, Compton, pair, and total cross sections (Geant4 units)
 */
array<double, 5> MCCrossSections::GetCrossSections(const G4Material* Material, const G4ParticleDefinition* Gamma,
                                       G4LivermorePhotoElectricModel* Photo, G4LivermoreRayleighModel* Rayleigh,
                                       G4CrossSectionHandler* Compton, G4LivermoreGammaConversionModel* Pair,
                                       double Energy)
{
  array<double, 5> Values;
  Values[0] = Photo->CrossSectionPerVolume(Material, Gamma, Energy);
  Values[1] = Rayleigh->CrossSectionPerVolume(Material, Gamma, Energy);
  Values[2] = Compton->ValueForMaterial(Material, Energy);
  Values[3] = Pair->CrossSectionPerVolume(Material, Gamma, Energy);
  Values[4] = Values[0] + Values[1] + Values[2] + Values[3];

  return Values;
}


/******************************************************************************
 * Insert ordered interior points until the sampled linear interpolation meets the tolerance
 */
void MCCrossSections::Refine(const G4Material* Material, const G4ParticleDefinition* Gamma,
                             G4LivermorePhotoElectricModel* Photo, G4LivermoreRayleighModel* Rayleigh,
                             G4CrossSectionHandler* Compton, G4LivermoreGammaConversionModel* Pair,
                             double Low, double High, const array<double, 5>& LowValues, const array<double, 5>& HighValues, vector<double>& Energies)
{
  // Target 0.1% accuracy with a 10% margin between samples
  const double Tolerance = 0.9E-3;

  // Keep edge brackets and intervals below 3e-5 relative width intact
  if (High/Low - 1.0 < 3E-5) return;

  // Sample 31 interior points to resolve model kinks
  // 31 is from experimentation together with the margin of 10% in the tolerance: with 15 points, the deviation between two
  // check points at a sharp kink of the data (CsI Rayleigh at 5.039 keV) was 18% larger than at them; with 31 all tests
  // (64 points per interval) stay within 0.1%. 2^n - 1 points make the middle a check point, which is reused for the split.
  const int NChecks = 31;
  array<array<double, 5>, NChecks> Values;
  for (int k = 0; k < NChecks; ++k) {
    Values[k] = GetCrossSections(Material, Gamma, Photo, Rayleigh, Compton, Pair, Low + (k + 1.0)/(NChecks + 1)*(High - Low));
  }

  bool Split = false;
  for (int p = 0; p < 5 && Split == false; ++p) {
    // Floors: photoelectric/Compton 1e-6, Rayleigh/pair 1e-3; always check the total
    // Apply the floor over the whole interval to catch threshold crossings
    const double Floor = (p == 0 || p == 2) ? 1E-6 : 1E-3;
    bool Checked = (p == 4);
    if (LowValues[p] >= Floor*LowValues[4] || HighValues[p] >= Floor*HighValues[4]) Checked = true;
    for (int k = 0; k < NChecks; ++k) {
      if (Values[k][p] >= Floor*Values[k][4]) Checked = true;
    }
    if (Checked == false) continue;

    for (int k = 0; k < NChecks; ++k) {
      if (Values[k][p] <= 0.0) continue;
      const double Interpolated = LowValues[p] + (k + 1.0)/(NChecks + 1)*(HighValues[p] - LowValues[p]);
      if (fabs(Interpolated - Values[k][p]) > Tolerance*Values[k][p]) {
        Split = true;
        break;
      }
    }
  }
  if (Split == false) return;

  // Split at the sampled midpoint:
  const double Middle = 0.5*(Low + High);
  const array<double, 5>& MiddleValues = Values[NChecks/2];
  Refine(Material, Gamma, Photo, Rayleigh, Compton, Pair, Low, Middle, LowValues, MiddleValues, Energies);
  Energies.push_back(Middle); // we need a new left edge (high edge is added later)
  Refine(Material, Gamma, Photo, Rayleigh, Compton, Pair, Middle, High, MiddleValues, HighValues, Energies);
}


/*
 * MCCrossSections.cc: the end...
 ******************************************************************************/
