/*
 * MDShapePCON.h
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


#ifndef __MDShapePCON__
#define __MDShapePCON__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MVector.h"
#include "MDShape.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing a polycone shape
class MDShapePCON : public MDShape
{
  // public interface:
 public:
  //! Standard constructor
  MDShapePCON(const MString& Name);
  //! Default destructor
  virtual ~MDShapePCON();

  //! Set the all parameters of the shape
  bool Set(double Phi, double DPhi, unsigned int NSection);
  //! Add a section
  bool AddSection(unsigned int Section, double z, double Rmin, double Rmax);

  //! Validate the data and create the shape 
  bool Validate();  
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  MVector GetSize();

  MString ToString();
  MString GetGeomega() const;

  double GetPhi();
  double GetDPhi();
  unsigned int GetNSections();
  double GetZ(unsigned int Section);
  double GetRmin(unsigned int Section);
  double GetRmax(unsigned int Section);

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
  double m_Phi;
  double m_DPhi;
  unsigned int m_NSections;
  vector<double> m_Z;
  vector<double> m_Rmin;
  vector<double> m_Rmax;

#ifdef ___CLING___
 public:
  ClassDef(MDShapePCON, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
