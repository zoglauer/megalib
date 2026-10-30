/*
 * MDShapeTUBS.h
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


#ifndef __MDShapeTUBS__
#define __MDShapeTUBS__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MVector.h"
#include "MDShape.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing a tube shape
class MDShapeTUBS : public MDShape
{
  // public interface:
 public:
  //! Standard constructor
  MDShapeTUBS(const MString& Name);
  //! Default destructor
  virtual ~MDShapeTUBS();

  //! Set the all parameters of the shape
  bool Set(double Rmin, double Rmax, double HalfHeight, double Phi1 = 0.0, double Phi2 = 360.0);

  //! Validate the data and create the shape 
  bool Validate();  
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  MVector GetSize();

  MString ToString();
  MString GetGeomega() const;
  
  double GetRmin();
  double GetRmax();
  double GetHeight();
  double GetPhi1();
  double GetPhi2();

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
  double m_Rmin;
  double m_Rmax;
  double m_HalfHeight;
  double m_Phi1;
  double m_Phi2;


#ifdef ___CLING___
 public:
  ClassDef(MDShapeTUBS, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
