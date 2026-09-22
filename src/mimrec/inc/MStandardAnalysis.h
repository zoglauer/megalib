/*
 * MStandardAnalysis.h
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


#ifndef __MStandardAnalysis__
#define __MStandardAnalysis__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"
#include "MEventSelector.h"
#include "MDGeometryQuest.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MStandardAnalysis
{
  // public interface:
 public:
  //! Default constructor
  MStandardAnalysis();
  //! Default destructor
  virtual ~MStandardAnalysis();

  //! Set the event selector
  void SetEventSelector(MEventSelector Selector) { m_Selector = Selector; }
  
  //! Set the geometry
  void SetGeometry(MDGeometryQuest* Geometry) { m_Geometry = Geometry; }
  
  //! Set the tra file name
  void SetFileName(MString FileName) { m_FileName = FileName; }

  //! Set the position (location of point source)
  void SetPosition(MVector Position) { m_Position = Position; }
  //! Set the energy (ideal energy of mono energetic beam)
  void SetEnergy(double Energy) { m_Energy = Energy; }


  //! Do all the analysis
  bool Analyze();

  // protected methods:
 protected:
  //MStandardAnalysis() {};
  //MStandardAnalysis(const MStandardAnalysis& StandardAnalysis) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The event selector
  MEventSelector m_Selector;

  //! The geometry 
  MDGeometryQuest* m_Geometry;

  //! Set the tra file name
  MString m_FileName;
  

  //! The position
  MVector m_Position;
  //! The energy
  double m_Energy;
  

#ifdef ___CLING___
 public:
  ClassDef(MStandardAnalysis, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
