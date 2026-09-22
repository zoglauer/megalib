/*
 * MCalibrateEnergyAssignEnergies.h
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


#ifndef __MCalibrateEnergyAssignEnergies__
#define __MCalibrateEnergyAssignEnergies__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MCalibrateEnergy.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The two modes for the energy assignment:
enum class MCalibrateEnergyAssignEnergyModes { e_LinearZeroCrossing, e_Linear };

//! A class to calibrate the lines in a spectrum
class MCalibrateEnergyAssignEnergies : public MCalibrateEnergy
{
  // public interface:
 public:
  //! Default constructor
  MCalibrateEnergyAssignEnergies();
  //! Default destuctor 
  virtual ~MCalibrateEnergyAssignEnergies();
  
  //! Set the mode 
  void SetMode(MCalibrateEnergyAssignEnergyModes Mode) { m_Mode = Mode; }
  
  //! Perform the calibration
  virtual bool Calibrate();
  
  // protected methods:
 protected:
  //! Perform the actual calibration assuming the calibration is approximate linear with zero crossing 
  virtual bool CalibrateLinearZeroCrossing();
  //! Perform the actual calibration assuming the calibration is approximate linear 
  virtual bool CalibrateLinear();
  
  // private methods:
 private:

  // protected members:
 protected:

  // private members:
 private:
   //! The mode
   MCalibrateEnergyAssignEnergyModes m_Mode;

   
#ifdef ___CLING___
 public:
  ClassDef(MCalibrateEnergyAssignEnergies, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
