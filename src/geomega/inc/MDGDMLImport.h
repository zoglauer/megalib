/*
 * MDGDMLImport.h
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


#ifndef __MDGDMLImport__
#define __MDGDMLImport__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:
#include "TGeoManager.h"

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A dummy definition of a class
class MDGDMLImport
{
  // public interface:
 public:
  //! Default constructor
  MDGDMLImport();
  //! Default destuctor 
  virtual ~MDGDMLImport();

  //! Parse the GDMl file
  bool Parse(MString FileName);
  
  //! Return the data as Geomega file
  MString GetAsGeomega();
  
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
   TGeoManager* m_GeoManager;


#ifdef ___CLING___
 public:
  ClassDef(MDGDMLImport, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
