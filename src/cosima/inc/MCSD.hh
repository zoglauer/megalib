/*
 * MCSD.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Base class for virtual detectors
 *
 */

#ifndef ___MCSD___
#define ___MCSD___

// Geant4:
#include "G4VSensitiveDetector.hh"

// MEGAlib:
#include "MFunction3D.h"


/******************************************************************************/

class MCSD : public G4VSensitiveDetector
{
  // public interface:
public:
  /// Default constructor
  MCSD(G4String Name);
  /// Default destructor
  virtual ~MCSD();

  /// Generate and store the hits 
  /// This function is called at the end of the UserSteppingAction to finalize 
  /// the ProcessHits-call
  virtual G4bool PostProcessHits(const G4Step*) = 0;

  /// Return the type of the detector
  int GetType();

  /// Id of an unknown detector type
  static const int c_Unknown;
  /// Id of a strip detector
  static const int c_2DStrip;
  /// Id of a colorimeter
  static const int c_Calorimeter;
  /// Id of a strip detector
  static const int c_3DStrip;
  /// Id of a scintillator
  static const int c_Scintillator;
  /// Id of a drift chamber
  static const int c_DriftChamber;
  /// Id of a voxel detector
  static const int c_Voxel3D;
  /// Id of an Anger camera
  static const int c_AngerCamera;

  /// Name of the detector volume (the layer, e.g. bachus)
  void SetDetectorVolumeName(G4String DetectorVolumeName) 
  { m_DetectorVolumeName = DetectorVolumeName; }
  /// Size of the sensitive volume in the detector
  void SetDetectorStructuralSize(G4ThreeVector StructuralSize)
  { m_StructuralSize = StructuralSize; }
  /// Pitch between sensitive volumes in the detector
  void SetDetectorStructuralPitch(G4ThreeVector StructuralPitch)
  { m_StructuralPitch = StructuralPitch; }
  /// Offset from detector edge to the first sensitive volume
  void SetDetectorStructuralOffset(G4ThreeVector StructuralOffset)
  { m_StructuralOffset = StructuralOffset; }
  /// Dimensions of the detector
  void SetDetectorStructuralDimension(G4ThreeVector StructuralDimension)
  { m_StructuralDimension = StructuralDimension; }

  /// Set a function describing the energy (e.g. charge/light) loss as a function of position
  void SetEnergyLossMap(const MFunction3D& Function) { m_UseEnergyLoss = true; m_EnergyLoss = Function; }

  /// Discretize the hits or store them all
  void SetDiscretizeHits(bool DiscretizeHits)
  { m_DiscretizeHits = DiscretizeHits; }

  /// Set if the detector has time resolution for individual hit timing
  void SetHasTimeResolution(bool Flag) { m_HasTimeResolution = Flag; }

  /// Set that the detector is never triggering (always vetoing)
  void SetIsNeverTriggering(bool Flag) { m_IsNeverTriggering = Flag; }
  
  // protected methods:
protected:
  

  // protected members:
protected:
  /// Type of this sensitive detector
  int m_Type;

  /// Name of the detector containing the sensitive volume
  G4String m_DetectorVolumeName;
  /// Dimension of the sensitive volume
  G4ThreeVector m_StructuralSize;
  /// Pitch between sensitive volumes
  G4ThreeVector m_StructuralPitch;
  /// Offset from edge of detector to first sensitive volume
  G4ThreeVector m_StructuralOffset;
  /// Dimensions of the detector
  G4ThreeVector m_StructuralDimension;
  /// True, if the hits should be discretized 
  bool m_DiscretizeHits;
  
  /// Function describing an energy loss distribution
  MFunction3D m_EnergyLoss;
  /// Flag indicating if the energy loss map is used
  bool m_UseEnergyLoss;

  /// True if the detector has time resolution
  bool m_HasTimeResolution;

  /// True if the detector is nevere ever triggering just vetoing
  bool m_IsNeverTriggering;
  
  /// Double representing an accuracy epsilon used to determine is some hits
  /// are on the boundary:
  double m_Epsilon;


  // private members:
private:

};

#endif


/*
 * MCSD.hh: the end...
 ******************************************************************************/
