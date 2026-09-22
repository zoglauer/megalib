/*
 * MDACS.h
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


#ifndef __MDACS__
#define __MDACS__


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


class MDACS : public MDDetector
{
  // public interface:
 public:
  MDACS(MString String = "");
  MDACS(const MDACS& A);
  virtual ~MDACS();

  virtual MDDetector* Clone();
  //! Copy data to named detectors
  virtual bool CopyDataToNamedDetectors();

  virtual bool Validate();

  virtual void SetDetectorVolume(MDVolume* Volume);

  virtual void Noise(MVector& Pos, double& Energy, double& Time, MString& Flags, MDVolume* Volume) const;
  
  //! Grid a hit -- in this case just return the single grid point of the hit
  virtual vector<MDGridPoint> Grid(const MVector& Pos, 
                                   const double& Energy,
                                   const double& Time,
                                   const MDVolume* Volume) const;
                                         
  //! Return the Grid point of this position
  virtual MDGridPoint GetGridPoint(const MVector& Pos) const;
  
  //! Return a position in detector volume coordinates
  virtual MVector GetPositionInDetectorVolume(const unsigned int xGrid, 
                                              const unsigned int yGrid,
                                              const unsigned int zGrid,
                                              const MVector PositionInGrid,
                                              const unsigned int Type,
                                              const MDVolume* Volume) const;
  virtual MVector GetPositionResolution(const MVector& Pos, const double Energy) const;

  bool IsVeto(const MVector& Pos, const double Energy) const;

  virtual MString GetGeomega() const;
  virtual MString ToString() const;


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


#ifdef ___CLING___
 public:
  ClassDef(MDACS, 0) // a basic ACS detector
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
