/*
 * MResponseImagingEfficiency.h
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


#ifndef __MResponseImagingEfficiency__
#define __MResponseImagingEfficiency__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBuilder.h"
#include "MResponseMatrixO2.h"
#include "MResponseMatrixO4.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseImagingEfficiency : public MResponseBuilder
{
  // public interface:
 public:
  MResponseImagingEfficiency();
  virtual ~MResponseImagingEfficiency();

  //! Initialize the response matrices and their generation
  virtual bool Initialize();

  //! Analyze th events (all if in file mode, one if in event-by-event mode)
  virtual bool Analyze();
    
  //! Finalize the response generation (i.e. save the data a final time )
  virtual bool Finalize();

  
  // protected methods:
 protected:

  //! Save the response matrices
  virtual bool Save();

  // private methods:
 private:



  // protected members:
 protected:
  //! The basic efficiency matrix
  MResponseMatrixO2 m_Efficiency1;
  //! The rotation associated with the basic efficiency matrix
  MRotation m_Rotation1;
  
  //! The 90 degree rotated efficiency matrix
  MResponseMatrixO2 m_Efficiency2;
  //! The rotation associated with the rotated efficiency matrix
  MRotation m_Rotation2;
  
  //! The detection efficiency
  MResponseMatrixO4 m_DetectionEfficiency;

  
  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MResponseImagingEfficiency, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
