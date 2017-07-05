/*
 * MDCalorimeter.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDCalorimeter__
#define __MDCalorimeter__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MDDetector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDCalorimeter : public MDDetector
{
  // public interface:
 public:
  MDCalorimeter(MString String = "");
  MDCalorimeter(const MDCalorimeter& C);
  virtual ~MDCalorimeter();

  virtual MDDetector* Clone();
  //! Copy data to named detectors
  virtual bool CopyDataToNamedDetectors();

  virtual void Noise(MVector& Pos, double& Energy, double& Time, MDVolume* Volume) const;
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

  virtual bool SetDepthResolutionAt(const double Energy, const double Resolution, const double Sigma);
  virtual bool HasDepthResolution() const { if (m_DepthResolutionType != c_DepthResolutionTypeNone && m_DepthResolutionType != c_DepthResolutionTypeUnknown) return true; else return false; } 

  virtual bool AreNear(const MVector& Pos1, const MVector& dPos1, 
                       const MVector& Pos2, const MVector& dPos2, 
                       const double Sigma, const int Level) const;

  virtual MString GetGeomega() const;
  virtual MString ToString() const;

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


  // private members:
 private:
  int m_DepthResolutionType;
  MFunction m_DepthResolution; 
  MFunction m_DepthResolutionSigma; 


#ifdef ___CINT___
 public:
  ClassDef(MDCalorimeter, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
