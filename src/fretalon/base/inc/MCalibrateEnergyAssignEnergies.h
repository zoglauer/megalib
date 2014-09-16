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


//! A class to calibrate the lines in a spectrum
class MCalibrateEnergyAssignEnergies : public MCalibrateEnergy
{
  // public interface:
 public:
  //! Default constructor
  MCalibrateEnergyAssignEnergies();
  //! Default destuctor 
  virtual ~MCalibrateEnergyAssignEnergies();
  
  //! Perform the calibration
  virtual bool Calibrate();
  
  // protected methods:
 protected:
  
  // private methods:
 private:

  // protected members:
 protected:

  // private members:
 private:

#ifdef ___CINT___
 public:
  ClassDef(MCalibrateEnergyAssignEnergies, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
