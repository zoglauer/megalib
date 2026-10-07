/*
 * MDGuardRing.h
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


#ifndef __MDGuardRing__
#define __MDGuardRing__


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


// A guard ring of a strip, pixel, etc. detector
class MDGuardRing : public MDDetector
{
  // public interface:
 public:
  //! Default constructor with an optional name
  MDGuardRing(MString String = "");
  //! Copy constructor
  //! The mother detector is set to zero, since not two detectors can have the same guard ring
  MDGuardRing(const MDGuardRing& A);
  //! Default destructor
  virtual ~MDGuardRing();

  //! Set the mother detector, i.e. the strip or voxel detector the guard ring belongs too
  void SetMotherDetector(MDDetector* MotherDetector) { m_MotherDetector = MotherDetector; }
  
  //! Determine if the guard ring is active
  void SetActive(bool IsActive) { m_IsActive = IsActive; }
  //! Determine if the guard ring is active
  bool IsActive() const { return m_IsActive; }
  
  //! Clone the detector
  virtual MDDetector* Clone();
  //! Copy data to named detectors
  virtual bool CopyDataToNamedDetectors();

  //! Validate all user set entries
  virtual bool Validate();

  //! Set the unique position of this detector
  void SetUniquePosition(MVector UniquePosition) { m_UniquePosition = UniquePosition; }
  //! Get the unique position of this detector
  MVector GetUniquePosition() const { return m_UniquePosition; }
  
  //! Noise a hit
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

  virtual MString GetGeomega() const;
  virtual MString ToString() const;


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The mother detector, i.e. the strip or voxel detector this detector belongs too 
  MDDetector* m_MotherDetector;
  //! Is the guard ring active, i.e. instrumented and doing something
  bool m_IsActive;
  //! The unique position in this guard ring
  MVector m_UniquePosition;
   

#ifdef ___CLING___
 public:
  ClassDef(MDGuardRing, 0) // a basic GuardRing detector
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
