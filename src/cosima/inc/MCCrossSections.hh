/*
 * MCCrossSections.hh
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



/******************************************************************************
 *
 * Class to calculate cross sections!
 *
 */

#ifndef ___MCCrossSections___
#define ___MCCrossSections___

// Geant4:
#include "globals.hh"

// Cosima:

// MEGAlib:
#include "MString.h"

// Standard lib:
#include <array>
#include <vector>
using namespace std;

// Forward declarations:
class G4Material;
class G4ParticleDefinition;
class G4LivermorePhotoElectricModel;
class G4LivermoreRayleighModel;
class G4LivermoreGammaConversionModel;
class G4CrossSectionHandler;


/******************************************************************************/

class MCCrossSections
{
  // public interface:
public:
  /// Default constructor
  MCCrossSections();
  /// Default destructor
  virtual ~MCCrossSections();

  /// Create the five macroscopic cross-section tables for each material in Path
  bool CreateCrossSectionFiles(MString Path);

  // protected methods:
protected:
  /// Return a logarithmic energy grid with shell-edge samples and adaptive refinement
  vector<double> GetEnergies(const G4Material* Material, const G4ParticleDefinition* Gamma,
                             G4LivermorePhotoElectricModel* Photo, G4LivermoreRayleighModel* Rayleigh,
                             G4CrossSectionHandler* Compton, G4LivermoreGammaConversionModel* Pair,
                             double MinimumEnergy, double MaximumEnergy, int Bins);
  /// Return photoelectric, Rayleigh, Compton, pair, and total cross sections (Geant4 units)
  array<double, 5> GetCrossSections(const G4Material* Material, const G4ParticleDefinition* Gamma,
                        G4LivermorePhotoElectricModel* Photo, G4LivermoreRayleighModel* Rayleigh,
                        G4CrossSectionHandler* Compton, G4LivermoreGammaConversionModel* Pair,
                        double Energy);
  /// Insert ordered interior points until the sampled linear interpolation meets the tolerance
  void Refine(const G4Material* Material, const G4ParticleDefinition* Gamma,
              G4LivermorePhotoElectricModel* Photo, G4LivermoreRayleighModel* Rayleigh,
              G4CrossSectionHandler* Compton, G4LivermoreGammaConversionModel* Pair,
              double Low, double High, const array<double, 5>& LowValues, const array<double, 5>& HighValues, vector<double>& Energies);

  // protected members:
protected:


  // private members:
private:


};

#endif


/*
 * MCCrossSections.hh: the end...
 ******************************************************************************/
