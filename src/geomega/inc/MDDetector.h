/*
 * MDDetector.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDDetector__
#define __MDDetector__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TObjArray.h>
#include <MString.h>
#include <TRandom3.h>
#include <TF1.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MDVolume.h"
#include "MDGrid.h"
#include "MDGridPoint.h"
#include "MDGridPointCollection.h"
#include "MDVolumeSequence.h"
#include "MFunction.h"
#include "MFunction3D.h"

// Standard libs:
#include <vector>
using namespace std;

// Forward declarations:
class MDGuardRing;


////////////////////////////////////////////////////////////////////////////////


class MDDetector
{
  // public interface:
 public:
  MDDetector(MString Name = "");
  MDDetector(const MDDetector& D);
  virtual ~MDDetector();

  //! Clone the detector and all its properties
  virtual MDDetector* Clone() = 0;

  //! Copy data to named detectors
  virtual bool CopyDataToNamedDetectors();

  //! Return the name of the detector
  virtual MString GetName() const { return m_Name; }
  //! Set the name of the detector
  virtual void SetName(const MString& Name) { m_Name = Name; }
  
  //! Call this function if the detector should be split in voxels for the simulation;
  //! Voxels are only used for strip detectors!
  virtual void UseDivisions(const MString& ShortNameX, const MString& ShortNameY, const MString& ShortNameZ);
  //! True, if the simulation should split the volume in voxels
  virtual bool UseDivisions() const { return m_UseDivisions; }
  //! Return the short name of divions direction x
  virtual MString GetShortNameDivisionX() const { return m_ShortNameDivisionX; }
  //! Return the short name of divions direction y
  virtual MString GetShortNameDivisionY() const { return m_ShortNameDivisionY; }
  //! Return the short name of divions direction z
  virtual MString GetShortNameDivisionZ() const { return m_ShortNameDivisionZ; }

  //! Return the detector type integer-encoded
  virtual int GetType() const { return m_Type; }
  //! Return the name of the type of this detector
  virtual MString GetTypeName() const { return m_Description; }

  //! Return true if the detector type ID exists
  static bool IsValidDetectorType(int ID);
  //! Return true if the detector type name exists
  static bool IsValidDetectorType(const MString& Name);
  //! Do a conversion from int ID to string ID
  static MString GetDetectorTypeName(const int Type);
  //! Do a conversion from string ID to int ID
  static int GetDetectorType(const MString& Type);


  //! If set to true, then the noise threshold equals the trigger threshold (i.e. basically only the latter exists) and avoids
  //! that randomization is done twice!
  virtual void SetNoiseThresholdEqualsTriggerThreshold(bool Flag = true) { m_NoiseThresholdEqualsTriggerThresholdSet = true; m_NoiseThresholdEqualsTriggerThreshold = Flag; }
  virtual bool GetNoiseThresholdEqualsTriggerThreshold() const { return m_NoiseThresholdEqualsTriggerThreshold; }

  virtual void SetNoiseThreshold(const double Threshold);
  virtual double GetNoiseThreshold(const MVector& Position = c_NullVector) const;

  virtual void SetNoiseThresholdSigma(const double ThresholdSigma);
  virtual double GetNoiseThresholdSigma(const MVector& Position = c_NullVector) const;

  virtual void SetTriggerThreshold(const double Threshold);
  virtual double GetTriggerThreshold(const MVector& Position = c_NullVector) const;

  virtual void SetTriggerThresholdSigma(const double ThresholdSigma);
  virtual double GetTriggerThresholdSigma(const MVector& Position = c_NullVector) const;

  //! In ANY case the real trigger threshold is below this value...
  virtual double GetSecureUpperLimitTriggerThreshold() const;

  virtual void SetPulseShape(const double Pol0, const double Pol1, const double Pol2, 
                             const double Pol3, const double Pol4, const double Pol5, 
                             const double Pol6, const double Pol7, const double Pol8, 
                             const double Pol9, const double Min, const double Max);

  virtual void SetFailureRate(const double Threshold);
  virtual double GetFailureRate() const;

  //! Return the type of used energy loss
  virtual int GetEnergyLossType() const { return m_EnergyLossType; }
  //! Use a energy loss based on a 3D energy loss map
  virtual void SetEnergyLossMap(const MString& EnergyLossMap);
  //! Use a energy loss based on a 3D energy loss map
  virtual MFunction3D GetEnergyLossMap() const { return m_EnergyLossMap; }

  //! Set the energy resolution type, return false if you try to overwrite an existing type
  virtual bool SetEnergyResolutionType(const int EnergyResolutionType);
  //! Return the energy resolution type
  virtual double GetEnergyResolutionType() const { return m_EnergyResolutionType; }

  virtual void SetEnergyResolution(const double InputEnergy, 
                                   const double PeakEnergy1, 
                                   const double Width1, 
                                   const double PeakEnergy2 = g_DoubleNotDefined, 
                                   const double Width2 = g_DoubleNotDefined, 
                                   const double Ratio = g_DoubleNotDefined);
  virtual double GetEnergyResolutionPeak1(const double Energy, const MVector& PositionInDetector = c_NullVector) const;
  virtual double GetEnergyResolutionWidth1(const double Energy, const MVector& PositionInDetector = c_NullVector) const;
  virtual double GetEnergyResolutionPeak2(const double Energy, const MVector& PositionInDetector = c_NullVector) const;
  virtual double GetEnergyResolutionWidth2(const double Energy, const MVector& PositionInDetector = c_NullVector) const;
  virtual double GetEnergyResolutionRatio(const double Energy, const MVector& PositionInDetector = c_NullVector) const;

  //! Returns an average energy resolution width
  virtual double GetEnergyResolution(const double Energy, const MVector& PositionInDetector = c_NullVector) const;

  virtual void SetEnergyCalibration(const MFunction& EnergyCalibration);

  virtual void SetTimeResolution(const double Energy, const double Sigma);
  virtual double GetTimeResolution(double Energy) const;

  virtual void SetDetectorVolume(MDVolume* Volume);
  virtual MDVolume* GetDetectorVolume();

  //! If there are more than one sensitive volume this is their common mother volume, otherwise it is the detector volume --- set during MDGeometry::Validate();
  virtual void SetCommonVolume(MDVolume* Volume) { m_CommonVolume = Volume; }
  //! If there are more than one sensitive volume this is their common mother volume, otherwise it is the detector volume
  virtual MDVolume* GetCommonVolume() { return m_CommonVolume; };

  virtual void SetOverflow(const double Min);
  virtual double GetOverflow() const;

  virtual void SetOverflowSigma(const double Min);
  virtual double GetOverflowSigma() const;

  virtual void SetStructuralOffset(const MVector& Offset);
  virtual MVector GetStructuralOffset() const;

  virtual void SetStructuralPitch(const MVector& Pitch);
  virtual MVector GetStructuralPitch() const;

  virtual MVector GetStructuralSize() const { return m_StructuralSize; }

  //! Return true, if the detector has guard ring
  virtual bool HasGuardRing() const { return m_HasGuardRing; }
  //! Return the guard ring detector or a nullptr if we don't have any
  MDGuardRing* GetGuardRing() { return m_GuardRing; }
  //! Return the guard ring detector or a nullptr if we don't have any
  const MDGuardRing* GetGuardRing() const { return m_GuardRing; }
  
  
  //! Return true, if the detector has a time resolution
  virtual bool HasTimeResolution() const;

  virtual void AddSensitiveVolume(MDVolume* Volume);
  virtual MDVolume* GetSensitiveVolume(const unsigned int i);
  virtual unsigned int GetNSensitiveVolumes() const;
  virtual int GetGlobalNSensitiveVolumes() const;

  //! Return the Grid point of this position - only the grid part is filled!
  virtual MDGridPoint GetGridPoint(const MVector& Pos) const = 0;

  //! Return a position in detector volume coordinates
  virtual MVector GetPositionInDetectorVolume(const unsigned int xGrid, const unsigned int yGrid, const unsigned int zGrid, const MVector PositionInGrid, const unsigned int Type, const MDVolume* Volume) const = 0;
                                              
  //! Return true if the energy is above the trigger threshold
  virtual bool IsAboveTriggerThreshold(const double& Energy, const MDGridPoint& Point) const;
  
  //! Return true if the energy is above the noise threshold
  virtual bool IsAboveNoiseThreshold(const double& Energy, const MDGridPoint& Point) const;
  
  //! Return the sigmas above the noise level this energy is
  virtual double SigmasAboveNoiseLevel(const double& Energy, const MDGridPoint& Point) const;
  
  //! Grid a hit, e.g. split due to e.g. charge transport, or do nothing and just return the grid of the hit
  virtual vector<MDGridPoint> Grid(const MVector& Pos, const double& Energy, const double& Time, const MDVolume* Volume) const = 0;
    
  //! Noise a normal hit
  virtual void Noise(MVector& Pos, double& Energy, double& Time, MString& Flags, MDVolume* Volume) const = 0;
  
  //! Noise a guard ring hit - if the detector has a guard ring
  //virtual bool NoiseGuardRing(double&) const { return false; }
  
  virtual bool AreNear(const MVector& Pos1, const MVector& dPos1, 
                       const MVector& Pos2, const MVector& dPos2, 
                       const double Sigma, const int Level) const;
  virtual double ApplyPulseShape(const double Time, const double Energy) const;

  //! Block some channels from triggering
  virtual bool BlockTriggerChannel(const unsigned int xGrid, 
                                   const unsigned int yGrid);

  virtual void ActivateNoising(const bool Activate = true);

  virtual MVector GetPositionResolution(const MVector& Pos, const double Energy) const;

  virtual bool IsVeto(const MVector& Pos, const double Energy) const;

  virtual MString ToString() const;

  // The named detector interface -- this is usually only used in connection of calibrating real data, NOT for simulated data

  //! Return true if this is a named detector
  bool IsNamedDetector() const { return m_IsNamedDetector; }
  
  //! Add a named detector - returns false in case the detector cannot be added
  bool AddNamedDetector(MDDetector* Detector);
  //! Return true if this detector contains the given named detector
  bool HasNamedDetectors() const { return (m_NamedDetectors.size() > 0 ? true : false);  }
  //! Return true if this detector contains the given named detector
  bool HasNamedDetector(const MString& Name) const;
  //! Use this function to convert a position within a NAMED detector (i.e. uniquely identifyable) into a position in the global coordinate system
  MVector GetGlobalPosition(const MVector& PositionInDetector, const MString& NamedDetector);
  //! Return the number of named detectors
  unsigned int GetNNamedDetectors() const { return m_NamedDetectors.size(); }
  //! Return the name of the "named detector"
  MString GetNamedDetectorName(unsigned int i) const;
  //! Return the volume sequence of the "named detector"
  MDVolumeSequence GetNamedDetectorVolumeSequence(unsigned int i);
  
  //! Find the named detector
  MDDetector* FindNamedDetector(const MDVolumeSequence& VS);
  //! In case this is a named detector, return the pointer it was named after
  MDDetector* GetNamedAfterDetector() const { return m_NamedAfter; }

  //! Set volume sequence
  void SetVolumeSequence(const MDVolumeSequence& VS) { m_VolumeSequence = VS; }
  
  //! Validates the detector information and creates the Grid
  virtual bool Validate();

  static void ResetIDs();

  static const int c_NoDetectorType;
  static const int c_Strip2D;
  static const int c_Calorimeter;
  static const int c_Strip3D;
  static const int c_ACS;
  static const int c_Scintillator;
  static const int c_DriftChamber;
  static const int c_Strip3DDirectional;
  static const int c_AngerCamera;
  static const int c_Voxel3D;
  static const int c_GuardRing;
  
  static const int c_MinDetector;
  static const int c_MaxDetector;

  static const MString c_NoDetectorTypeName;
  static const MString c_Strip2DName;
  static const MString c_CalorimeterName;
  static const MString c_Strip3DName;
  static const MString c_ScintillatorName;
  static const MString c_ACSName;
  static const MString c_DriftChamberName;
  static const MString c_Strip3DDirectionalName;
  static const MString c_AngerCameraName;
  static const MString c_Voxel3DName;
  static const MString c_GuardRingName;
  
  static const int c_EnergyResolutionTypeUnknown;
  static const int c_EnergyResolutionTypeNone;
  static const int c_EnergyResolutionTypeIdeal;
  static const int c_EnergyResolutionTypeGauss;
  static const int c_EnergyResolutionTypeLorentz;
  static const int c_EnergyResolutionTypeGaussLandau;

  static const int c_EnergyLossTypeUnknown;
  static const int c_EnergyLossTypeNone;
  static const int c_EnergyLossTypeMap;

  static const int c_TimeResolutionTypeUnknown;
  static const int c_TimeResolutionTypeNone;
  static const int c_TimeResolutionTypeIdeal;
  static const int c_TimeResolutionTypeGauss;

  static const int c_DepthResolutionTypeUnknown;
  static const int c_DepthResolutionTypeNone;
  static const int c_DepthResolutionTypeIdeal;
  static const int c_DepthResolutionTypeGauss;
  
  
  
  // protected methods:
 protected:
  virtual bool ApplyEnergyResolution(double& Energy, const MVector& Position = c_NullVector) const;
  virtual bool ApplyTimeResolution(double& Time, const double Energy) const;
  virtual bool ApplyNoiseThreshold(double& Energy, const MVector& Position = c_NullVector) const;
  virtual bool ApplyOverflow(double& Energy) const;
  virtual bool ApplyEnergyCalibration(double& Energy) const;

  //! The grid is only created if it is actually used
  virtual void CreateBlockedTriggerChannelsGrid();

  MString GetGeomegaCommon(bool PrintVolumes = true,
                           bool PrintStructural = true,
                           bool PrintEnergyResolution = true,
                           bool PrintTimeResolution = true,
                           bool PrintTriggerThreshold = true,
                           bool PrintNoiseThreshold = true,
                           bool PrintOverflow = true,
                           bool PrintFailureRate = true,
                           bool PrintPulseShape = true) const;

  // private methods:
 private:



  // public members:
 public:

  // protected members:
 protected:
  //! The name of this detector class
  MString m_Name;
  
  //! 
  int m_Type;
  MString m_Description;

  //! A list of sensitive volumes
  vector<MDVolume*> m_SVs;
  //! The detector volume
  MDVolume* m_DetectorVolume;
  //! If there are multiple sensitive volumes, this is the common volume in which they are all included
  MDVolume* m_CommonVolume;

  int m_ID;
  static int m_IDCounter;
  int m_SensID;            
  static int m_SensIDCounter;

  //! True, if the simulation should split the simulation volume into small voxels
  bool m_UseDivisions;
  //! Short (G3) name for divisions in x direction - not supported by all derived classes
  MString m_ShortNameDivisionX;
  //! Short (G3) name for divisions in y direction - not supported by all derived classes
  MString m_ShortNameDivisionY;
  //! Short (G3) name for divisions in z direction - not supported by all derived classes
  MString m_ShortNameDivisionZ;

  int m_EnergyLossType;
  MFunction3D m_EnergyLossMap;

  int m_EnergyResolutionType;
  MFunction m_EnergyResolutionPeak1;
  MFunction m_EnergyResolutionWidth1;
  MFunction m_EnergyResolutionPeak2;
  MFunction m_EnergyResolutionWidth2;
  MFunction m_EnergyResolutionRatio;

  //! The type of time resolution we use
  int m_TimeResolutionType; 
  //! The time resolution
  MFunction m_TimeResolution;

  //! Flag indicating that the energy calibration has been set
  bool m_EnergyCalibrationSet;
  //! Flag indicating that we perform an energy calibration
  bool m_UseEnergyCalibration;
  //! Function containing a 1D energy calibration
  MFunction m_EnergyCalibration;

  //! A random failure rate for each read-out element
  double m_FailureRate;

  //! If this flag is set than noise equals trigger threshold, i.e. there is only the trigger threshold
  bool m_NoiseThresholdEqualsTriggerThreshold;
  //! Flag indicating the m_NoiseThresholdEqualsTriggerThreshold has been set
  bool m_NoiseThresholdEqualsTriggerThresholdSet;
  
  double m_NoiseThreshold;
  double m_NoiseThresholdSigma;

  double m_TriggerThreshold;
  double m_TriggerThresholdSigma;

  double m_Overflow;
  double m_OverflowSigma;


  //! Half (!) size of the box-like detector volume
  MVector m_StructuralDimension;     
  //! Half (!) size of one volume element (crystal, Si-Pixel), i.e. size of the sensitive volume!
  MVector m_StructuralSize;     
  //! Distance to the first sensitive volume element calculated from the negative axis
  MVector m_StructuralOffset;
  //! Spacing between the sensitive volume elements
  MVector m_StructuralPitch;    

  //! True if this detector has a guard ring:
  bool m_HasGuardRing;
  //! The guard ring itself - nullptr if we don't have any
  MDGuardRing* m_GuardRing; 

  //! Flag indicating if the grid of trigger blocked channels is used 
  bool m_AreBlockedTriggerChannelsUsed;
  //! The grid of trigger blocked channels
  MDGrid m_BlockedTriggerChannels;

  
  //! Return true if this is a named detector
  bool m_IsNamedDetector;
  //! If this is a named detector, the detector we where named after
  MDDetector* m_NamedAfter;
  //! The list of named detectors
  vector<MDDetector*> m_NamedDetectors;
  //! The volume sequence - filled AND ONLY VALID if this is a named detector
  // TODO: Rename to m_NamedVolumeSequence
  MDVolumeSequence m_VolumeSequence;


  bool m_PulseShapeSet;
  TF1* m_PulseShape;
  double m_PulseShapeMin;
  double m_PulseShapeMax;

  bool m_NoiseActive;

  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MDDetector, 0) // basic detector
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
