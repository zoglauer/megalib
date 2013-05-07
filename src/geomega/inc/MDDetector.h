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
#include "MSpline.h"
#include "MDVolumeSequence.h"
#include "MFunction.h"
#include "MFunction3D.h"

// Standard libs:
#include <vector>
using namespace std;

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDDetector
{
  // public interface:
 public:
  MDDetector(MString Name = "");
  MDDetector(const MDDetector& D);
  virtual ~MDDetector();

  virtual MDDetector* Clone() = 0;

  virtual MString GetName() const;

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
  //! Very stupid... ??
  virtual void SetType(const MString& Type);

  //! Return true if the detector type ID exists
  static bool IsValidDetectorType(int ID);
  //! Return true if the detector type name exists
  static bool IsValidDetectorType(const MString& Name);
  //! Do a conversion from int ID to string ID
  static MString GetDetectorTypeName(const int Type);
  //! Do a conversion from string ID to int ID
  static int GetDetectorType(const MString& Type);

  //! Remove this later...
  virtual int GetDetectorType() const { return m_Type; }

  //! If set to true, then the noise threshold equals the trigger threshold (i.e. basically only the latter exists) and avoids
  //! that randomization is done twice!
  virtual void SetNoiseThresholdEqualsTriggerThreshold(bool Flag = true) { m_NoiseThresholdEqualsTriggerThreshold = Flag; }

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
  virtual bool HasGuardring() const { return m_HasGuardring; }
  //! Return true, if the detector has a time resolution
  virtual bool HasTimeResolution() const;

  virtual void AddSensitiveVolume(MDVolume* Volume);
  virtual MDVolume* GetSensitiveVolume(const unsigned int i);
  virtual unsigned int GetNSensitiveVolumes() const;
  virtual int GetGlobalNSensitiveVolumes() const;

  //! Discretize the position to a voxel of the volume, which needs to be a clone template!
  virtual vector<MDGridPoint> Discretize(const MVector& Pos, 
                                         const double& Energy, 
                                         const double& Time, 
                                         MDVolume* Volume) const = 0;
  //! Return the Grid point of this position - only the grid part is filled!
  virtual MDGridPoint GetGridPoint(const MVector& Pos) const = 0;

  //! Return a position in detector volume coordinates
  virtual MVector GetPositionInDetectorVolume(const unsigned int xGrid, 
                                              const unsigned int yGrid,
                                              const unsigned int zGrid,
                                              const MVector PositionInGrid,
                                              const unsigned int Type,
                                              MDVolume* Volume) = 0;

  virtual bool IsAboveTriggerThreshold(const double& Energy, const MDGridPoint& Point) const;
  virtual void Noise(MVector& Pos, double& Energy, double& Time, MDVolume* Volume) const = 0;
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

  virtual MString GetGeomega() const = 0;
  virtual MString GetGeant3() const = 0;
  virtual MString GetMGeant() const = 0;
  virtual MString GetGeant3Divisions() const = 0;
  virtual MString GetMGeantDivisions() const = 0;
  virtual MString ToString() const = 0;

  // The named detector interface -- this is usually only used in connection of calibrating real data, NOT for simulated data

  //! Add a named detector
  void AddNamedDetector(const MString& Name, const MDVolumeSequence& VS);
  //! Return true if this detector contains the given named detector
  bool HasNamedDetector(const MString& Name) const;
  //! Use this function to convert a position within a NAMED detector (i.e. uniquely identifyable) into a position in the global coordinate system
  MVector GetGlobalPosition(const MVector& PositionInDetector, const MString& NamedDetector);
  //! Return the number of named detectors
  unsigned int GetNNamedDetectors() const { return m_NamedDetectorNames.size(); }
  //! Return the name of the "named detector"
  MString GetNamedDetectorName(unsigned int i) const;
  //! Return the volume sequence of the "named detector"
  MDVolumeSequence GetNamedDetectorVolumeSequence(unsigned int i);


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

  static const int c_EnergyResolutionTypeUnknown;
  static const int c_EnergyResolutionTypeIdeal;
  static const int c_EnergyResolutionTypeGauss;
  static const int c_EnergyResolutionTypeLorentz;
  static const int c_EnergyResolutionTypeGaussLandau;

  static const int c_EnergyLossTypeNone;
  static const int c_EnergyLossTypeMap;


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
  //! The detctor volume
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

  MSpline* m_TimeResolution;

  //! Flag indicating that we perform an energy calibration
  bool m_UseEnergyCalibration;
  //! Function containing a 1D energy calibration
  MFunction m_EnergyCalibration;


	double m_FailureRate;

  //! If this flag is set than noise equals trigger threshold, i.e. there is only the trigger threshold
  bool m_NoiseThresholdEqualsTriggerThreshold;
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

  //! True if this detector type has a guard ring:
  bool m_HasGuardring;

  //! Flag indicating if the grid of trigger blocked channels is used 
  bool m_AreBlockedTriggerChannelsUsed;
  //! The grid of trigger blocked channels
  MDGrid m_BlockedTriggerChannels;

  //! A detector class might have several individual positioned detectors
  //! This variable stores names of all of them
  vector<MString> m_NamedDetectorNames;
  //! This variable stores VolumeSequences to them
  vector<MDVolumeSequence> m_NamedDetectorVolumeSequences;


  TF1* m_PulseShape;
  double m_PulseShapeMin;
  double m_PulseShapeMax;

  bool m_NoiseActive;

  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MDDetector, 0) // basic detector
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
