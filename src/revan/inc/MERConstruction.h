/*
 * MERConstruction.h
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


#ifndef __MERConstruction__
#define __MERConstruction__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MRawEventIncarnations.h"
#include "MGeometryRevan.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Base class for all event reconstrcution tasjs
class MERConstruction
{
  // public interface:
 public:
  //! The default constructor
  MERConstruction();
  //! The deafault destructor
  virtual ~MERConstruction();
  
  //! Set the geometry
  virtual void SetGeometry(MGeometryRevan* Geometry) { m_Geometry = Geometry; }
  
  //! Analyze a list of event incarnations
  virtual bool Analyze(MRawEventIncarnations* List);

  //! Perform per-analysis, e.g. load all files, etc.
  virtual bool PreAnalysis() { return true; }
  //! Perform post-analysis, e.g create analysis summaries, etc.
  virtual bool PostAnalysis() { return true; }

  //! Dump what we have done into a string
  virtual MString ToString(bool CoreOnly = false) const = 0;

  // protected methods:
 protected:

  //! Modification routine at the beginning of the analysis to add/modify the events in the list before the analysis
  virtual void ModifyEventList();

  // private methods:
 private:



  // protected members:
 protected:
  //! The global event (incarnation) list
  MRawEventIncarnations* m_List;

  //! The geometry - used by most of the algorithms
  MGeometryRevan* m_Geometry;

  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MERConstruction, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
