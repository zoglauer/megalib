/*
 * MDVoxel3D.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDVoxel3D__
#define __MDVoxel3D__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MDDetector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDVoxel3D : public MDDetector
{
  // public interface:
 public:
  MDVoxel3D(MString String = "");
  MDVoxel3D(const MDVoxel3D& S);
  virtual ~MDVoxel3D();

  virtual MDDetector* Clone();
  //! Copy data to named detectors
  virtual bool CopyDataToNamedDetectors();

  virtual void Noise(MVector& Pos, double& Energy, double& Time, MDVolume* Volume) const;
  virtual bool NoiseGuardring(double& Energy) const;
  virtual vector<MDGridPoint> Discretize(const MVector& Pos, 
                                         const double& Energy, 
                                         const double& Time, 
                                         MDVolume* Volume) const;
  //! Return the Grid point of this position
  virtual MDGridPoint GetGridPoint(const MVector& Pos) const;
  //! Return a position in detector volume coordinates
  virtual MVector GetPositionInDetectorVolume(const unsigned int xGrid, 
                                              const unsigned int yGrid,
                                              const unsigned int zGrid,
                                              const MVector PositionInGrid,
                                              const unsigned int Type,
                                              MDVolume* Volume);
  virtual MVector GetPositionResolution(const MVector& Pos, const double Energy) const;

  virtual MString GetGeomega() const;
  virtual MString GetGeant3() const;
  virtual MString GetMGeant() const;
  virtual MString GetGeant3Divisions() const;
  virtual MString GetMGeantDivisions() const;
  virtual MString ToString() const;

  bool SetGuardringEnergyResolution(const double Energy, const double Resolution);
  virtual double GetGuardringEnergyResolution(const double Energy) const;

  void SetGuardringTriggerThreshold(const double Threshold) { m_GuardringTriggerThreshold = Threshold; }
  double GetGuardringTriggerThreshold() const { return m_GuardringTriggerThreshold; }

  void SetGuardringTriggerThresholdSigma(const double Sigma) { m_GuardringTriggerThresholdSigma = Sigma; }
  double GetGuardringTriggerThresholdSigma() const { return m_GuardringTriggerThresholdSigma; }

  virtual bool IsAboveGuardringTriggerThreshold(const double& Energy) const;

  //! Retrieve a unique position on the guard ring 
  MVector GetUniqueGuardringPosition() const;

  void SetWidth(const double x, const double y, const double z);
  double GetWidthX() const { return m_WidthX; }
  double GetWidthY() const { return m_WidthY; }
  double GetWidthZ() const { return m_WidthZ; }

  void SetOffset(const double x, const double y, const double z);
  double GetOffsetX() const { return m_OffsetX; }
  double GetOffsetY() const { return m_OffsetY; }
  double GetOffsetZ() const { return m_OffsetY; }

  void SetNVoxels(const int x, const int y, const int z);
  int GetNVoxelsX() const { return m_NVoxelsX; }
  int GetNVoxelsY() const { return m_NVoxelsY; }
  int GetNVoxelsZ() const { return m_NVoxelsZ; }

  // VoxelSize is not set but calculated from the given numbers
  double GetVoxelSizeX() const { return m_VoxelSizeX; }
  double GetVoxelSizeY() const { return m_VoxelSizeY; }
  double GetVoxelSizeZ() const { return m_VoxelSizeZ; }

  virtual bool AreNear(const MVector& Pos1, const MVector& dPos1, 
                       const MVector& Pos2, const MVector& dPos2, 
                       const double Sigma, const int Level) const;
  //virtual bool DetermineVoxels(const MVector& Pos, int& xVoxel, int& yVoxel, int& zVoxel) const;

  //! Check if all input is reasonable
  virtual bool Validate();


  // protected methods:
 protected:

  //! The grid is only created if it is actually used
  virtual void CreateBlockedTriggerChannelsGrid();


  // private methods:
 private:



  // protected members:
 protected:
  //! x width of one wafer
  double m_WidthX;
  //! y width of one wafer
  double m_WidthY;
  //! z width of one wafer
  double m_WidthZ;

  //! x offset after which the first strip starts
  double m_OffsetX;
  //! y offset after which the first strip starts
  double m_OffsetY;
  //! z offset after which the first strip starts
  double m_OffsetZ;

  //! x number of strips
  int m_NVoxelsX;
  //! y number of strips
  int m_NVoxelsY;
  //! z number of strips
  int m_NVoxelsZ;

  //! x size of one voxel
  double m_VoxelSizeX;
  //! y size of one voxel
  double m_VoxelSizeY;
  //! z size of one voxel
  double m_VoxelSizeZ;

  //! x number of blocks (sensitive volumes) in detector
  int m_NBlocksX;
  //! y number of blocks (sensitive volumes) in detector
  int m_NBlocksY;
  //! z number of blocks (sensitive volumes) in detector
  int m_NBlocksZ;

  //! Trigger threshold of the guard ring
  double m_GuardringTriggerThreshold;
  //! Trigger threshold of the guard ring one sigma resolution
  double m_GuardringTriggerThresholdSigma;
  
  
  //! The energy resolution type of the guard ring
  int m_GuardringEnergyResolutionType;
  //! Energy resolution of the guard ring
  MFunction m_GuardringEnergyResolution; 

  //! A unique position in the guardring of one wafer relative to the wafer center!!
  MVector m_UniqueGuardringPosition;

  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MDVoxel3D, 0) // a basic SiStrip detetctor
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
