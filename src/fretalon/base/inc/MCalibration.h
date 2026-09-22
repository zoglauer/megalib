/*
 * MCalibration.h
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


#ifndef __MCalibration__
#define __MCalibration__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A general calibration result
class MCalibration
{
  // public interface:
 public:
  //! Default constructor
  MCalibration();
  //! Default destuctor 
  virtual ~MCalibration();

  //! Clone this calibration - the returned calibration must be deleted!
  virtual MCalibration* Clone() const;

  //! Return true if the calibration is completd
  bool IsCalibrated() const { return m_IsCalibrated; }
  //! Set the calibration status
  void IsCalibrated(bool Flag) { m_IsCalibrated = Flag; }
  
  //! Return the data as parsable string
  virtual MString ToParsableString(bool WithDescriptor = false) { return ""; } 
  
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:
  //! Flag indicating that the calibration is completed
  bool m_IsCalibrated;


  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MCalibration, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
