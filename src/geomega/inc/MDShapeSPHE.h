/*
 * MDShapeSPHE.h
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


#ifndef __MDShapeSPHE__
#define __MDShapeSPHE__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MVector.h"
#include "MDShape.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing a spherical shape
class MDShapeSPHE : public MDShape
{
  // public interface:
 public:
  //! Standard constructor
  MDShapeSPHE(const MString& Name);
  //! Default destructor
  virtual ~MDShapeSPHE();

  //! Set the all parameters of the shape
  bool Set(double Rmin, double Rmax);
  //! Set the all parameters of the shape
  bool Set(double Rmin, double Rmax, double Thetamin, double Thetamax, double Phimin, double Phimax);

  //! Validate the data and create the shape 
  bool Validate();  
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  MVector GetSize();

  double GetRmin() const;
  double GetRmax() const;
  double GetThetamin() const;
  double GetThetamax() const;
  double GetPhimin() const;
  double GetPhimax() const;

  MString ToString();
  MString GetGeomega() const;

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
  double m_Thetamin;
  double m_Thetamax;
  double m_Phimin;
  double m_Phimax;


#ifdef ___CLING___
 public:
  ClassDef(MDShapeSPHE, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
