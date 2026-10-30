/*
 * MCalibrate.h
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


#ifndef __MCalibrate__
#define __MCalibrate__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:
#include "TH1.h"

// MEGAlib libs:
#include "MCalibration.h"
#include "MReadOutDataGroup.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A base class for any calibration task
class MCalibrate
{
  // public interface:
 public:
  //! Default constructor
  MCalibrate();
  //! Default destuctor 
  virtual ~MCalibrate();

  //! Set the diagnoistiocs mode
  void SetDiagnosticsMode(bool Mode = true) { m_DiagnosticsMode = Mode; }
  
  //! Perform the calibration
  virtual bool Calibrate() = 0;
  
  // protected methods:
 protected:
  
  // private methods:
 private:



  // protected members:
 protected:
  //! True when we are in diagnostics mode
  bool m_DiagnosticsMode;
  
  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MCalibrate, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
