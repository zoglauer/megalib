/*
 * MDStrip3DDirectional.h
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


#ifndef __MDStrip3DDirectional__
#define __MDStrip3DDirectional__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MDStrip3D.h"
#include "MStreams.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDStrip3DDirectional : public MDStrip3D
{
  // public interface:
 public:
  MDStrip3DDirectional(MString Name = "");
  MDStrip3DDirectional(const MDStrip3DDirectional& S);
  virtual ~MDStrip3DDirectional();

  virtual MDDetector* Clone();

  //! Copy data to named detectors
  virtual bool CopyDataToNamedDetectors();

  virtual bool NoiseDirection(MVector& Direction, double Energy) const;
  virtual void SetDirectionalResolutionAt(const double Energy, const double Resolution, const double Sigma);

  virtual MString ToString() const;

  //! Check if all input is reasonable
  virtual bool Validate();

  virtual MString GetGeomega() const;

  static const int c_DirectionalResolutionTypeUnknown;
  static const int c_DirectionalResolutionTypeIdeal;
  static const int c_DirectionalResolutionTypeGauss;

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  int m_DirectionalResolutionType; 
  MFunction m_DirectionalResolution; 
  MFunction m_DirectionalResolutionSigma; 


  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MDStrip3DDirectional, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
