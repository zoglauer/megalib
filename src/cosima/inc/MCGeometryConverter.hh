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

// MEGAlib:
#include "MString.h"

// Standard lib:
#include <map>
#include <vector>
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
  bool Convert(MString OutputFileName = "G4ToMEGAlib.geo.setup"); 

  // protected methods:
protected:
  /// Create an unique material name:
  MString CreateUniqueName(G4Material* M);
  /// Create an unique logiocal volume name:
  MString CreateUniqueName(G4LogicalVolume* L);
  /// Create an unique physical volume name:
  MString CreateUniqueName(G4VPhysicalVolume* P);

  /// Convert the name to MEGAlib conventions
  MString MakeValidName(MString Name);

  /// Convert very small numberes to zero
  double Zeroing(double Number);

  // protected members:
protected:


  // private members:
private:
  /// Map material pointers to unique names
  map<G4Material*, MString> m_UniqueMaterialNames;
  /// Map logical volume pointers to unique names
  map<G4LogicalVolume*, MString> m_UniqueLogicalVolumeNames;
  /// Map physicalvolume pointers to unique names
  map<G4VPhysicalVolume*, MString> m_UniquePhysicalVolumeNames;
};

#endif


/*
 * MCGeometryConverter.hh: the end...
 ******************************************************************************/
