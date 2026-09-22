/*
 * MDShapeCONS.h
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


#ifndef __MDShapeCONS__
#define __MDShapeCONS__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MVector.h"
#include "MDShape.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing a cone shape
class MDShapeCONS : public MDShape
{
  // public interface:
 public:
  //! Standard constructor
  MDShapeCONS(const MString& Name);
  //! Default destructor
  virtual ~MDShapeCONS();

  //! Set the all parameters of the shape
  bool Set(double HalfHeight, double RminBottom, double RmaxBottom, double RminTop, double RmaxTop, double PhiMin, double PhiMax);

  //! Validate the data and create the shape 
  bool Validate();  
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  TShape* GetShape();
  void CreateShape();
  MVector GetSize();

  MString ToString();
  MString GetGeomega() const;

  double GetHalfHeight();
  double GetRminBottom();
  double GetRmaxBottom();
  double GetRminTop();
  double GetRmaxTop();
  double GetPhiMin();
  double GetPhiMax();

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
  double m_HalfHeight;
  double m_RminBottom;
  double m_RmaxBottom;
  double m_RminTop;
  double m_RmaxTop;
  double m_PhiMin;
  double m_PhiMax;


#ifdef ___CLING___
 public:
  ClassDef(MDShapeCONS, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
