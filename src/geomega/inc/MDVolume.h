/*
 * MDVolume.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDVolume__
#define __MDVolume__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObjArray.h"
#include "TRotMatrix.h"
#include "TGeoManager.h"
#include "TGeoVolume.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MDShape.h"
#include "MDMaterial.h"
#include "MVector.h"
#include "MRotation.h"
#include "MString.h"

// Standard libs::
#include <vector>
#include <map>
using namespace std;

// Forward declarations:
class MDDetector;
class MDVolumeSequence;

////////////////////////////////////////////////////////////////////////////////


class MDVolume
{
  // public interface:
 public:
  MDVolume(MString Name = "");
  virtual ~MDVolume();

  inline const MString& GetName() const { return m_Name; }

  void SetWorldVolume();
  bool IsWorldVolume();

  void SetVirtual(bool Virtual);
  bool IsVirtual();

  void SetMany(bool Many);
  bool IsMany();

  void SetAbsorptions(bool DoAbsorption);
  bool DoAbsorptions();

  //! Check if any of the mothers is "Mother"
  bool HasMother(MDVolume* Mother);
  //! Set the mother, if an error occurs return false and do not set the mother
  bool SetMother(MDVolume* Mother);
  MDVolume* GetMother();

  void AddDaughter(MDVolume* Daughter);
  void RemoveAllDaughters();
  MDVolume* RemoveDaughter(MDVolume* Daughter);
  unsigned int GetNDaughters();
  MDVolume* GetDaughterAt(unsigned int i);

  void SetShape(MDShape* Shape);
  MDShape* GetShape();

  void SetMaterial(MDMaterial *Material);
  MDMaterial* GetMaterial();

  void SetPosition(MVector Pos);
  MVector GetPosition();

  void SetRotation(double x, double y, double z);
  void SetRotation(MRotation Rotation, int RotID);
  void SetRotation(MRotation Rotation);
  void SetRotation(double theta1, double phi1, 
                   double theta2, double phi2, 
                   double theta3, double phi3);
  MRotation GetRotationMatrix() const;
  MRotation GetInvRotationMatrix() const;
  MRotation GetInverseRotationMatrix() const { return GetInvRotationMatrix(); }
  bool IsRotated() const;
  int GetRotationID() const;

  void SetVisibility(int Visibility);
  int GetVisibility();

  void SetColor(int Color);
  int GetColor();

  void SetLineWidth(int LineWidth);
  int GetLineWidth();

  void SetLineStyle(int LineStyle);
  int GetLineStyle();

  void SetDetector(MDDetector* Detector);
  MDDetector* GetDetector();
  void SetDetectorVolume(MDVolume* Volume, MDDetector* Detector);
  MDVolume* GetDetectorVolume();
  void SetIsDetectorVolume(MDDetector* Detector);
  //! If the volume represents a detector return true
  bool IsDetectorVolume();
  //! Check if either this volume or one of its daughters is a detector volume
  bool ContainsDetectorVolume();
  //! Check the list of daughters, those not containing sensitive detectors are virtualized
  void VirtualizeNonDetectorVolumes();

  void AddClone(MDVolume* Clone);
  unsigned int GetNClones() const;
  MDVolume* GetCloneAt(unsigned int i);
  unsigned int GetCloneId(MDVolume* Clone);
  bool IsClone() const;
  bool IsCloneTemplate() const { return (GetNClones() > 0) ? true : false; }

  void SetCloneTemplate(MDVolume* Template);
  MDVolume* GetCloneTemplate();

  bool IsCloneTemplateVolumeWritten();
  void SetCloneTemplateVolumeWritten(bool Written);

  bool AreCloneTemplateDaughtersWritten();
  void SetCloneTemplateDaughtersWritten(bool Written);

  //! Reset all "CloneTemplate...Written" flags
  void ResetCloneTemplateFlags();

  MVector GetSize();

  int GetID();
  static void ResetIDs();

  void SetSensitiveVolumeID(int ID);
  int GetSensitiveVolumeID();
  bool IsSensitive();
  void SetIsSensitive();

  bool CopyDataToClones();
  bool Validate();
  bool ValidateIntersections();
  //! This is a special validation: All clones must have the some mother volumes for activation simulations
  //! It has been separated out, because some (non-simulation) optimizations are allowed to break this rule
  bool ValidateClonesHaveSameMotherVolume();

  //! Check if a position in this volumes mother volume is inside this volume
  bool IsInside(MVector Pos, double Tolerance = 0); // TODO: RENAME! MISLEADING --> or make at least protected
  double DistanceInsideOut(MVector Pos);
  double DistanceOutsideIn(MVector Pos);

  //! Create the Root geometry presentation of thhis volume and its daughters
  void CreateRootGeometry(TGeoManager* Manager, TGeoVolume* Mother);

  
  // Interface for RECURSIVE manipulation of the volume tree or data retrieval

  //! Return the (deepest) volume at given position - if IsRelative == true, then position is assumed to be already in this volumes coordinate system
  MDVolume* GetVolume(MVector Pos, bool IsRelative = false);
  //! Return true if the volume is part of the volume tree
  bool ContainsVolume(const MString& Name, bool IncludeTemplates=false);
  //! Find detector volume and apply (random) noise to position, energy and time
  bool Noise(MVector& Pos, double& Energy, double& Time); 
  //! Find detector volume and apply pulse-shape correction
  bool ApplyPulseShape(double Time, MVector& Pos, double& Energy);
  
  //! FILL the MDVolumeSequence object with the sequence of volumes of the deepest volume the given position is in
  //! The return value is for internal purposes only!
  bool GetVolumeSequence(MVector Pos, MDVolumeSequence* Sequence);
  //! FILL the MDVolumeSequence object with the sequence of volumes of the deepest volume the given position is in
  //! The position starts in this volume, but is translated to the world volume, and then the sequecning starts
  //! The return value is for internal purposes only!
  bool GetVolumeSequenceInverse(MVector Pos, MDVolumeSequence* Sequence);
  
  //! FILL the PATH lengths in the different materials between the positions start and stop
  //! The return value is for internal purposes only (volume (cm3) of the last volume)
  double GetAbsorptionLengths(map<MDMaterial*, double>& Lengths, MVector Start, MVector Stop);
  //! FILL the masses sorted by material
  //! The return value is for internal purposes only (volume (cm3) of the last volume)
  double GetMasses(map<MDMaterial*, double>& Materials);
  //! Return the number of visible volumes in the tree
  unsigned int GetNVisibleVolumes();
  //! Return the number of sensitive volumes in the tree
  unsigned int GetNSensitiveVolumes();
  //! Finds all DEEPEST volumes Pos is in...
  //! The return value is for internal purposes only!
  bool FindOverlaps(MVector Pos, vector<MDVolume*>& OverlappingVolumes);
  //! Scale this volume
  //! The return value is for internal purposes only!
  bool Scale(const double Scale);
  //! Correctly remove all virtual volumes from the volume tree
  //! The return value is for internal purposes only!
  //! The vector contains a list of all newly generated volumes
  bool RemoveVirtualVolumes(vector<MDVolume*>& NewVolumes);
  //! Optimized the arrangement of the volume true, for the search of hits in sensitive detectors
  void OptimizeVolumeTree();

  //! Returns the position in the mother volume - pos is in this volume - test if there is mother volume first
  MVector GetPositionInMotherVolume(MVector Pos);
  //! Returns the position in the world volume - pos is inside this volume
  MVector GetPositionInWorldVolume(MVector Pos);

  //! Returns a random position in any of the volumes defined by Volume
  MVector GetRandomPositionInVolume(MDVolume* Volume, vector<int>& Placements, int& TreeDepth);
  //! Find the number of placements of a volume
  bool GetNPlacements(MDVolume* Volume, vector<int>& Placements, int& TreeDepth);


  //! Recursively return a geomega setup file type string 
  MString GetGeomega();

  MString ToString(bool Recursive = false);
  MString ToStringVolumeTree(int Level = 0);

  // protected methods:
 protected:
  MDVolume* Clone(MString Name); // use only in remove virtual volume!!!!



  // private methods:
 private:


 public:
  static int m_IDCounter;    // Maximum distributed volume ID

  // protected members:
 protected:


  // private members:
 private:
  MString m_Name;              // Name of this volume

  bool m_WorldVolume;        // True if this is the world volume  
  bool m_IsVirtual;          // True if this is a virtual volume, i.e. it does not appear in the final geometry
  bool m_IsMany;             // True if the many flag is raised

  MDVolume* m_Mother;          // Mother volume
  vector<MDVolume*> m_Daughters;      // Number of daughter volumes

  MDMaterial *m_Material;      // Material of the volume
  MDShape* m_Shape;            // Shape of the volume
  MVector m_Position;         // Position of the volume in the mother volume

  bool m_IsRotated;          // True if this volume is rotated

  MRotation m_RotMatrix;         // Rotation as matrix for computaions
  MRotation m_InvertedRotMatrix; // Inverted rotation as matrix for computaions
  double m_Theta1, m_Phi1;   // Rotation of the new x-Axis in MCS (Mother Coo)
  double m_Theta2, m_Phi2;   // Rotation of the new y-Axis in MCS
  double m_Theta3, m_Phi3;   // Rotation of the new z-Axis in MCS


  MRotation m_WorldRotation;    // Rotation matrix, for rotation a point in world to this volume
  MVector m_WorldTranslation;  // Translation vector, for translation a point in world coordinates into this volume

  int m_Visibility;          // 2: volume is always visible, 1: volume is visible, 0: volume is not visible
  int m_Color;               // Color of the volume
  int m_LineStyle;           // Line style
  int m_LineWidth;           // Line Width

  MDDetector* m_Detector;      // If this is a sensitive volume: associated detector
  // Don't use --- something ancient!
  bool m_IsDetectorVolume;   // True, if this volume represents a detector
  MDVolume* m_DetectorVolume;  // The mother-volume of this volume, which represents the detector, this volume is inside

  vector<MDVolume*> m_Clones;         // Pointer to all clones of this volume
  MDVolume* m_CloneTemplate;   // Pointer to the clone template, 0 otherwise
  bool m_CloneTemplateVolumeWritten;   // True if the clone template has already been written to the geant file
  bool m_CloneTemplateDaughtersWritten;   // True if the clone template has already been written to the geant file
  int m_CloneTemplateId;

  bool m_IsSensitive;        // True if this volume is sensitive...
  bool m_DoAbsorptions;      // True if absorptions should be calculated for this volume

  double m_Tolerance;        // The tolerance for IsInside Calculations

  int m_ID;                  // ID of this volume
  int m_RotID;               // ID of the rotation
  static int m_RotIDCounter; // Maximum distributed rotation ID
  static int m_WrittenRotID; // Used for geant3/mmgpod writing of rotation IDs 
  int m_SensID;              // ID of sensitive volume, if it is senitive




#ifdef ___CLING___
 public:
  ClassDef(MDVolume, 0)         // a detector volume
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
