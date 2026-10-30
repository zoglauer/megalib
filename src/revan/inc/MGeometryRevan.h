/*
 * MGeometryRevan.h
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


#ifndef __MGeometryRevan__
#define __MGeometryRevan__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MDGeometryQuest.h"
#include "MRESE.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Enhanced geometry with revan specific additions
class MGeometryRevan : public MDGeometryQuest
{
  // public interface:
 public:
  //! Standard constructor
  MGeometryRevan();
  //! Default destructor
  virtual ~MGeometryRevan();

  //! Noise a hit consi
  virtual bool NoiseHit(MVector& Pos, double& E);
  //! Return true if the two RESEs are in the same detector and the same layer
  virtual bool AreInSameLayer(MRESE* Orig, MRESE* Test);
  //! Return true if the two RESEs are in the same detector / tracker
  virtual bool AreInSameDetectorVolume(MRESE* RESE1, MRESE* RESE2);
  //! Check if the Test hit is NBelow layers below the Reference hit
  virtual bool IsBelow(MRESE* Reference, MRESE* Test, int NBelow = 1);
  //! Check if the Test hit is NBelow layers below the Reference hit
  virtual bool IsAbove(MRESE* Reference, MRESE* Test, int NAbove = 1);
  //! Return the distance in layers between the reference and test hit
  int GetLayerDistance(MRESE* Reference, MRESE* Test);
  //! Return the distance in cm between the refe
  

  static const int c_DifferentTracker;

  // protected methods:
 protected:
  //MGeometryRevan() {};
  //MGeometryRevan(const MGeometryRevan& GeometryRevan) {};

  virtual bool AreInSameTracker(MRESE *Orig, MRESE *Test);

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  double m_LayerDistance;

#ifdef ___CLING___
 public:
  ClassDef(MGeometryRevan, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
