/*
 * MCalibrateEnergyAssignEnergies.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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

   
#ifdef ___CINT___
 public:
  ClassDef(MCalibrateEnergyAssignEnergies, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
