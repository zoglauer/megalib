/*
 * MCGeometryConverter.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Converts a Geant4 geometry into a Geomega geometry
 *
 */

#ifndef ___MCGeometryConverter___
#define ___MCGeometryConverter___

// Geant4:
#include "globals.hh"
#include "G4Material.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"

// Cosima:
// --- Make sure you use only Geant4 and C++ classes

// MEGAlib:
// --- Make sure you use only Geant4 and C++ classes

// Standard lib:
#include <map>
#include <vector>
#include <fstream>
using namespace std;

// Forward declarations:


/******************************************************************************/

class MCGeometryConverter
{
  // public interface:
public:
  /// Default constructor
  MCGeometryConverter();
  /// Default destructor
  virtual ~MCGeometryConverter();

  /// Convert the geometry from G4 to MEGAlib format
  bool Convert(G4String OutputFileName = "G4ToMEGAlib.geo.setup"); 

  // protected methods:
protected:
  /// Create an unique material name:
  G4String CreateUniqueName(G4Material* M);
  /// Create an unique logiocal volume name:
  G4String CreateUniqueName(G4LogicalVolume* L);
  /// Create an unique physical volume name:
  G4String CreateUniqueName(G4VPhysicalVolume* P);

  /// Convert the name to MEGAlib conventions
  G4String MakeValidName(G4String Name);

  // Convert the shape in a logical volume
  // If volume name is given, it is part of a volume
  bool ConvertShape(ofstream& fout, G4String& Name, G4VSolid* S, bool PartOfVolume = true);
  
  // Convert an orientation
  bool ConvertOrientation(ofstream& fout, G4String Name, G4ThreeVector Translation, G4RotationMatrix Rotation);
  
  /// Convert very small numberes to zero
  double Zeroing(double Number);

  // protected members:
protected:


  // private members:
private:
  /// Map material pointers to unique names
  map<G4Material*, G4String> m_UniqueMaterialNames;
  /// Map logical volume pointers to unique names
  map<G4LogicalVolume*, G4String> m_UniqueLogicalVolumeNames;
  /// Map physicalvolume pointers to unique names
  map<G4VPhysicalVolume*, G4String> m_UniquePhysicalVolumeNames;
};

#endif


/*
 * MCGeometryConverter.hh: the end...
 ******************************************************************************/
