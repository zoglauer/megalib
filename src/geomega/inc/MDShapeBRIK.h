/*
 * MDShapeBRIK.h
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


#ifndef __MDShapeBRIK__
#define __MDShapeBRIK__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MVector.h"
#include "MDShape.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing a box shape
class MDShapeBRIK : public MDShape
{
  // public interface:
 public:
  //! Default constructor
  MDShapeBRIK(const MString& Name);
  //! Default destructor
  virtual ~MDShapeBRIK();
  
  //! Set the shape data
  bool Set(double x, double y, double z);
  
  //! Set the shape data form its ROOT volume
  bool Set(TGeoBBox* V);
  
  //! Validate the data and create the shape 
  bool Validate();  
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  MVector GetSize();

  virtual bool IsInside(const MVector& Pos, const double Tolerance = 0, const bool PreferOutside = false);

  virtual double DistanceOutsideIn(const MVector& Pos, const MVector& Dir, double Tolerance = 0);
  
  
  MString ToString();
  MString GetGeomega() const;


  double GetSizeX();
  double GetSizeY();
  double GetSizeZ();

  double GetVolume();

  //! Scale the axes given in Axes by a factor Scaler
  virtual bool Scale(const double Scaler, const MString Axes = "XYZ");

  //! Return a unique position within the volume of the detector (center if possible)
  virtual MVector GetUniquePosition() const;

  //! Return a random position withn this volume
  virtual MVector GetRandomPositionInside(); 

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  double m_Dx;
  double m_Dy;
  double m_Dz;


#ifdef ___CLING___
 public:
  ClassDef(MDShapeBRIK, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
