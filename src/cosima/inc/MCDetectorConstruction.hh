/*
 * MCDetectorConstruction.hh
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
 * This class converts the geomega geometry into a Geant4 geometry
 *
 */


#ifndef MCDetectorConstruction_H
#define MCDetectorConstruction_H 1

// Geant4:
#include "G4VUserDetectorConstruction.hh"
#include "G4RotationMatrix.hh"
#include "G4Material.hh"

// MEGAlib:
#include "MDGeometryQuest.h"
#include "MDOrientation.h"

// Cosima:
#include "MCParameterFile.hh"

// Forward declarations:
class G4VPhysicalVolume;
class MDVolume;

/******************************************************************************/

class MCDetectorConstruction : public G4VUserDetectorConstruction
{
  // public interface:
public:
  /// Default constructor
  MCDetectorConstruction(MCParameterFile& RunParameters);
  /// Default destructor
  virtual ~MCDetectorConstruction();

  /// Do all initializations here which can go wrong
  bool Initialize();
  
  /// Construct()-method from derived class - called by Geant4
  G4VPhysicalVolume* Construct();

  /// Return true if the volume is valid volume in the geometry
  bool IsValidVolume(MString VolumeName);
  
  /// Return a random position in the given volume
  G4ThreeVector GetRandomPosition(MString VolumeName);
  
  /// Return true if the volume exists:
  bool HasVolume(const MString& VolumeName) const;
  
  /// Return the (Geomega) hash of the given material 
  unsigned long GetMaterialHash(const G4Material* Material);

  /// Return the geometry
  MDGeometryQuest* GetGeometry() { return m_Geometry; }

  /// Return true is the position is within the world volume
  bool IsInsideWorldVolume(const G4ThreeVector& Position) const;
  
  
  // protected methods:
protected:
  /// Convert from MEGAlib materials to Geant4 ones
  bool ConstructMaterials();
  /// Convert from MEGAlib volumes to Geant4 ones
  bool ConstructVolumes();
  /// Construct the regions with different range cuts:
  bool ConstructRegions();
  /// Convert from MEGAlib detectors to Geant4 ones
  bool ConstructDetectors();
  /// Recursivly position the volumes, starting with the world volume
  bool PositionVolumes(MDVolume* Volume);

  /// Create a Geant4 Rotation from the volume information
  G4RotationMatrix* CreateRotation(MDVolume *Volume);

  /// Create a Geant4 Rotation from the orientation information
  G4RotationMatrix* CreateRotation(MDOrientation* Orientation);

  // protected members:
protected:


  // private members:
private:
  /// Parameters of the *.source file
  MCParameterFile& m_RunParameters;

  /// MEGAlib geometry representation
  MDGeometryQuest* m_Geometry;

  /// The mother volume
  G4VPhysicalVolume* m_WorldVolume;
};

#endif


/*
 * MCDetectorConstruction.hh: the end...
 ******************************************************************************/
