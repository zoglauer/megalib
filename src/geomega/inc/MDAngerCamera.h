/*
 * MDAngerCamera.h
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */


#ifndef __MDAngerCamera__
#define __MDAngerCamera__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <map>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MDDetector.h"
#include "MDVolume.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDAngerCamera : public MDDetector
{
  // public interface:
 public:
  MDAngerCamera(MString String = "");
  MDAngerCamera(const MDAngerCamera& A);
  virtual ~MDAngerCamera();

  virtual MDDetector* Clone();
  //! Copy data to named detectors
  virtual bool CopyDataToNamedDetectors();

  virtual bool Validate();

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
                                              const unsigned int ZGrid,
                                              const MVector PositionInGrid,
                                              const unsigned int Type,
                                              MDVolume* Volume);
  virtual MVector GetPositionResolution(const MVector& Pos, const double Energy) const;

  virtual MString GetGeomega() const;
  virtual MString ToString() const;

  //! Set the type XY-only or XYZ-only
  void SetPositioning(int Positioning) { m_PositionResolutionType = Positioning; }
  //! Get the positioning
  int GetPositioning() const { return m_PositionResolutionType; }
  
  
  void SetPositionResolution(const double Energy, const double Resolution);
  void SetPositionResolutionXYZ(const double Energy, const double ResolutionX, const double ResolutionY, const double ResolutionZ);
  
  static const int c_PositionResolutionUnknown;
  static const int c_PositionResolutionXY;
  static const int c_PositionResolutionXYZ;
  static const int c_PositionResolutionXYZIndependent;
  

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Type of the positioning (XY only or XYZ)
  int m_PositionResolutionType;
  //! Position solution
  MFunction m_PositionResolution; 
  
  //! Position solution
  MFunction m_PositionResolutionX; 
  //! Position solution
  MFunction m_PositionResolutionY; 
  //! Position solution
  MFunction m_PositionResolutionZ; 
  
  
#ifdef ___CLING___
 public:
  ClassDef(MDAngerCamera, 0) // a basic AngerCamera detector
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
