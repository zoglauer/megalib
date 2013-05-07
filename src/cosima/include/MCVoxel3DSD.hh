/*
 * MCVoxel3DSD.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Class representing the sensitive detector of a strip detector
 *
 */

#ifndef ___MCVoxel3DSD___
#define ___MCVoxel3DSD___

// Geant4:
#include "G4VSensitiveDetector.hh"
#include "globals.hh"

// Cosima:
#include "MCVoxel3DHit.hh"
#include "MCSD.hh"

// Forward declarations:
class G4HCofThisEvent;
class G4Step;

/******************************************************************************/


class MCVoxel3DSD : public MCSD
{
  // public interface:
public:
  /// Default constructor: Sets the name, type and default values for geometry
  MCVoxel3DSD(G4String Name);
  /// Default destructor: empty
  ~MCVoxel3DSD();
  
  /// Initialize the hit collection
  void Initialize(G4HCofThisEvent*);
  /// Analyze and store the hits
  G4bool ProcessHits(G4Step* astep, G4TouchableHistory* ROHist);
  /// Generate and store the hits 
  G4bool PostProcessHits(const G4Step* Step);
  /// Housekeeping
  void EndOfEvent(G4HCofThisEvent*);

  /// Size of the wafer
  void SetSize(double xSize, double ySize, double zSize) 
  { m_XSize = xSize; m_YSize = ySize; m_ZSize = zSize; }
  /// Offset from the edge to the first voxel
  void SetOffset(double xOffset, double yOffset, double zOffset) 
  { m_XOffset = xOffset; m_YOffset = yOffset; m_ZOffset = zOffset; }
  /// Distance between the voxels
  void SetVoxelSize(double xVoxelSize, double yVoxelSize, double zVoxelSize) 
  { m_XVoxelSize = xVoxelSize; m_YVoxelSize = yVoxelSize; m_ZVoxelSize = zVoxelSize; }
  /// Number of voxels
  void SetNVoxels(int xNVoxels, int yNVoxels, int zNVoxels) 
  { m_XNVoxels = xNVoxels; m_YNVoxels = yNVoxels; m_ZNVoxels = zNVoxels; }

  /// Set a unique position on the guard ring 
  void SetUniqueGuardringPosition(G4ThreeVector Pos) 
  { m_UniqueGuardringPosition = Pos; }


  // protected methods:
protected:
  

  // protected members:
protected:


  // private members:
protected:
  /// Container for the hits of one event
  MCVoxel3DHitsCollection* m_HitCollection;      

  /// x-Size of the volume of this detector
  double m_XSize;
  /// x-Offset from the edge to the sensitive volume
  double m_XOffset;
  /// x-Size of one voxel
  double m_XVoxelSize;
  /// x-Number of voxels:
  int m_XNVoxels;

  /// y-Size of the volume of this detector
  double m_YSize;
  /// y-Offset from the edge to the sensitive volume
  double m_YOffset;
  /// y-Size of one voxel
  double m_YVoxelSize;
  /// y-Number of voxels:
  int m_YNVoxels;

  /// z-Size of the volume of this detector
  double m_ZSize;
  /// z-Offset from the edge to the sensitive volume
  double m_ZOffset;
  /// z-Size of one voxel
  double m_ZVoxelSize;
  /// z-Number of voxels:
  int m_ZNVoxels;

  /// A unique position in the guardring
  G4ThreeVector m_UniqueGuardringPosition;

};

#endif


/*
 * MCVoxel3DSD.hh: the end...
 ******************************************************************************/





