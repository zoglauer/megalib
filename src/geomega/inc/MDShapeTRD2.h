/*
 * MDShapeTRD2.h
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


#ifndef __MDShapeTRD2__
#define __MDShapeTRD2__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MVector.h"
#include "MDShape.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing a trapezoid shape
class MDShapeTRD2 : public MDShape
{
  // public interface:
 public:
  //! Standard constructor
  MDShapeTRD2(const MString& Name);
  //! Default destructor
  virtual ~MDShapeTRD2();

  //! Set the all parameters of the shape
  bool Set(double dx1, double dx2, double dy1, double dy2, double z);

  //! Validate the data and create the shape 
  bool Validate();  
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  MVector GetSize();

  double GetDx1() const;
  double GetDx2() const;
  double GetDy1() const;
  double GetDy2() const;
  double GetZ() const;

  MString ToString();
  MString GetGeomega() const;

  double GetVolume();

  //! Scale the axes given in Axes by a factor Scaler
  virtual bool Scale(const double Scaler, const MString Axes = "XYZ");

  //! Return a unique position within the volume of the detector (center if possible)
  virtual MVector GetUniquePosition() const;

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  double m_Dx1;
  double m_Dx2;
  double m_Dy1;
  double m_Dy2;
  double m_Z;


#ifdef ___CLING___
 public:
  ClassDef(MDShapeTRD2, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
