/*
 * MDGeometry
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDGeometry__
#define __MDGeometry__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TCanvas.h>
#include <TGeometry.h>
#include <TGeoManager.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MDMaterial.h"
#include "MDVolume.h"
#include "MDVolumeSequence.h"
#include "MDDetector.h"
#include "MDTrigger.h"
#include "MDVector.h"
#include "MDDebugInfo.h"
#include "MDTriggerUnit.h"
#include "MDSystem.h"
#include "MDOrientation.h"

// Standard libs:
#include <vector>
#include <list>
#include <map>
using std::vector;


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDGeometry
{
  // public interface:
 public:
  MDGeometry();
  virtual ~MDGeometry();

  //! Scan the setup file
  //! If create nodes is false, no nodes for drawing a created
  //! If VirtualizeNonDetectorVolumes is true, all non-detector volumes are eliminated from the volume tree
  bool ScanSetupFile(MString FileName = "", bool CreateNodes = true, bool VirtualizeNonDetectorVolumes = false, bool AllowCrossSectionCreation = true);
  
  //! Draws the geometry
  //! WARNING: This is NOT reentrant, you cannot draw two different geometries!
  virtual bool DrawGeometry(TCanvas *Canvas = 0, MString Mode = "ogle");
  
  bool WriteGeant3Files();
  bool WriteMGeantFiles(MString FilePrefix = "", bool StoreIAs = false, bool StoreVetoes = true);
  bool TestIntersections();
  void DumpInformation();
  void CalculateMasses();

  //! Set whether to ignore short names (short names are required for Geant3/MGGPOD)
  void IgnoreShortNames(bool Ignore) { m_IgnoreShortNames = Ignore; }

  //! This flag tells us we are within geomega, this enables a few options, which are not
  //! available in the normal library mode, e.g. view the surrounding sphere
  void LaunchedByGeomega() { m_LaunchedByGeomega = true; } 
  
  //! Check for overlaps using the ROOT overlap checker
  bool CheckOverlaps();
  //! Add a preferred visible volume --- if any is given, only those will be shown
  void AddPreferredVisibleVolume(const MString& Name) { m_PreferredVisibleVolumeNames.push_back(Name); }
  
  MString GetFileName();
  MString GetName();

  bool IsScanned();
  bool AreCrossSectionsPresent();
  //! Create the x-section files if cosima is present
  bool CreateCrossSectionFiles();

  void AddVolume(MDVolume* Volume);
  MDVolume* GetVolumeAt(const unsigned int i) const;
  MDVolume* GetVolume(const MString& Name);
  unsigned int GetVolumeIndex(const MString& Name);
  unsigned int GetNVolumes() const;
  MDVolume* GetWorldVolume();

  void SetStartVolumeName(MString StartVolume) { m_StartVolume = StartVolume; }
  MString GetStartVolumeName() { return m_StartVolume; }

  void AddMaterial(MDMaterial* Material);
  MDMaterial* GetMaterialAt(unsigned int i);
  MDMaterial* GetMaterial(const MString& Name);
  unsigned int GetMaterialIndex(const MString& Name);
  unsigned int GetNMaterials();

  void AddDetector(MDDetector* Detector);
  MDDetector* GetDetectorAt(unsigned int i);
  MDDetector* GetDetector(const MString& Name);
  unsigned int GetDetectorIndex(const MString& Name);
  unsigned int GetNDetectors();

  bool AddShape(const MString& Type, const MString& Name);
  void AddShape(MDShape* Shape);
  MDShape* GetShapeAt(unsigned int i);
  MDShape* GetShape(const MString& Name);
  unsigned int GetShapeIndex(const MString& Name);
  unsigned int GetNShapes();

  void AddOrientation(MDOrientation* Orientation);
  MDOrientation* GetOrientationAt(unsigned int i);
  MDOrientation* GetOrientation(const MString& Name);
  unsigned int GetOrientationIndex(const MString& Name);
  unsigned int GetNOrientations();

  MDTriggerUnit* GetTriggerUnit() { return m_TriggerUnit; }

  void AddTrigger(MDTrigger* Trigger);
  MDTrigger* GetTriggerAt(unsigned int i);
  MDTrigger* GetTrigger(const MString& Name);
  unsigned int GetTriggerIndex(const MString& Name);
  unsigned int GetNTriggers();

  MDSystem* GetSystem(const MString& Name);
  MDSystem* GetSystem() { return m_System; }
  
  // Interface to all stored vectors
  
  void AddVector(MDVector* Vector);
  MDVector* GetVectorAt(unsigned int i);
  MDVector* GetVector(const MString& Name);
  unsigned int GetVectorIndex(const MString& Name);
  unsigned int GetNVectors();


  // Interface to all included files:

  void AddInclude(MString FileName);
	bool IsIncluded(MString FileName);
  int GetNIncludes();

  void CreateNode(MDVolume *Volume);

  MString CreateShortName(MString Name, unsigned int Length = 4, bool Fill = false, bool KeepKeywords = false);
  bool ShortNameExists(MString Name);
  bool ValidName(MString Name);
  static MString MakeValidName(MString Name);

  void Typo(MString Typo);
  bool NameExists(MString);

  MString ToString();

  double GetStartSphereRadius() const;
  double GetStartSphereDistance() const;
  MVector GetStartSpherePosition() const;

  bool HasComplexER();
  void CloseGeoView3D();

  MDVolumeSequence GetVolumeSequence(MVector Pos, bool ForceDetector = false, bool ForceSensitiveVolume = false);
  MDVolumeSequence* GetVolumeSequencePointer(MVector Pos, bool ForceDetector = false, bool ForceSensitiveVolume = false);

  //! Return a list of unused materials
  vector<MDMaterial*> GetListOfUnusedMaterials();

  //! Use this function to convert a position within a NAMED detector (i.e. uniquely identifyable) into a position in the global coordinate system
  MVector GetGlobalPosition(const MVector& PositionInDetector, const MString& NamedDetector);

  //! Return a random position in the given volume --- excluding daughter volumes!
  MVector GetRandomPositionInVolume(const MString& Name);

  void Reset();

  // protected methods:
 protected:
  bool AddFile(MString Filename, vector<MDDebugInfo>& DebugInfos);

  MString WFS(MString Text);
  void ReplaceWholeWords(MString& Text, const MString& OldWord, const MString& NewWord);
  bool ContainsReplacableConstant(const MString& Text, const MString& Constant);
  
  // private methods:
 private:


  // protected members:
 protected:
  //! name of the geometry file
  MString m_FileName;

  //! Current debugging information
  MDDebugInfo m_DebugInfo;      
  //! Number of the line which is currently analyzed/debugged
  int m_NTextLine;             

  //! The world volume the geometry lays inside
  MDVolume* m_WorldVolume;
  //! A temporary world volume
  MString m_StartVolume;
  //! A list of preferred volumes - only those will be shown
  vector<MString> m_PreferredVisibleVolumeNames;

  //! Unit taking care of all triggering aspects
  MDTriggerUnit* m_TriggerUnit;  
  
  //! A list of all volumes - the structure is kept within the volumes, which are containers
  vector<MDVolume*> m_VolumeList;
  //! A list of all materials
  vector<MDMaterial*> m_MaterialList;
  //! A list of all detectors
  vector<MDDetector*> m_DetectorList;
  //! A list of all triggers  
  vector<MDTrigger*> m_TriggerList;
  //! A list of all defined shapes
  vector<MDShape*> m_ShapeList;
  //! A list of all defined orientations
  vector<MDOrientation*> m_OrientationList;

  //! The main system
  MDSystem* m_System;

  //! A list of all vectors
  vector<MDVector*> m_VectorList;

  //! A list of all constants
  vector<MString> m_ConstantList;
  //! A map of all constants and their companions
  map<MString, MString> m_ConstantMap;

  //! The different detector types
  vector<int> m_NDetectorTypes; 

  //! A list of all already included files
  vector<MString> m_IncludeList;     

  //! Name of the geometry
  MString m_Name;
  //! Version of the geometry
  MString m_Version;

  //! Center of the surrounding sphere of the geometry
  MVector m_SurroundingSpherePosition;
  //! Radius of the surrounding sphere of the geometry
  double m_SurroundingSphereRadius;
  //! Distance of the tangential plane to the center of the sphere
  double m_SurroundingSphereDistance;
  //! True if the surrounding sphere should be shown
  bool m_SurroundingSphereShow;
  
  bool m_GeometryScanned;     // true if the geometry has been scanned successfully 
  bool m_ShowVolumes; // if false, no volumes are shown except those with a visibility higher than 1
  int m_DefaultColor; // if positive, this color is applied to all volumes

  //! Ignore short names (which are only needed for Geant3/MGGPOD conversion)
  bool m_IgnoreShortNames;
  bool m_DoSanityChecks; //
  bool m_ComplexER; //
  
  //! If true, all volumes which are not detector volumes are virtualized
  bool m_VirtualizeNonDetectorVolumes;

  TCanvas* m_GeoView;
  TGeoManager* m_Geometry;

  //! Director where all cross sections are stored
  MString m_CrossSectionFileDirectory;

  //! When a hit is outside an active detector (rounding errors etc), use this tolerance to look for
  //! the detector:
  double m_DetectorSearchTolerance;

  //! A flag indicating the geometry has been launched by geomega, this enables a few additional options
  //! which should never, ever be called in library mode, e.g. adding the surrounding sphere
  bool m_LaunchedByGeomega;
  
  // private members:
 private:
  list<MDVolume*> m_LastVolumes;
  unsigned int m_LastVolumePosition;

  //! The random number generator for GetRandomPositionInVolume
  TRandom3 m_RandomPositionInVolumeRNG;
  
  //! The last found volume in GetRandomPositionInVolume
  MDVolume* m_LastFoundVolume_GetRandomPositionInVolume;
  //! The last found placements in GetRandomPositionInVolume
  vector<int> m_LastFoundPlacements_GetRandomPositionInVolume;
  
  
#ifdef ___CINT___
 public:
  ClassDef(MDGeometry, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
