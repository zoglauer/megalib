/*
 * MCalibrateEnergyDetermineModel.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MCalibrateEnergyDetermineModel__
#define __MCalibrateEnergyDetermineModel__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MCalibrateEnergy.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A class to calibrate the lines in a spectrum
class MCalibrateEnergyDetermineModel : public MCalibrateEnergy
{
  // public interface:
 public:
  //! Default constructor
  MCalibrateEnergyDetermineModel();
  //! Default destuctor 
  virtual ~MCalibrateEnergyDetermineModel();

  //! Set the calibration model determination method 
  void SetCalibrationModelDeterminationMethod(unsigned int Method) { m_CalibrationModelDeterminationMethod = Method; }
  //! Set the fitting model options for the calibration model determination method
  void SetCalibrationModelDeterminationMethodFittingEnergyOptions(unsigned int Model) { 
    m_CalibrationModelDeterminationMethodFittingEnergyModel = Model; }
    //! Set the fitting model options for the calibration model determination method
    void SetCalibrationModelDeterminationMethodFittingFWHMOptions(unsigned int Model) { 
      m_CalibrationModelDeterminationMethodFittingFWHMModel = Model; }
      
  //! ID for a step-wise interpolated calibration model
  static const unsigned int c_CalibrationModelStepWise = 0;
  //! ID for a fitted interpolated calibration model
  static const unsigned int c_CalibrationModelFit = 1;
  //! ID for a fitted interpolated calibration model chosen as the best fit from a selection
  static const unsigned int c_CalibrationModelBestFit = 2;
  
  //! Perform the calibration
  virtual bool Calibrate();
  
  
  // protected methods:
 protected:
   //! Perform the energy calibration
   virtual bool CalibrateEnergyModel();
   
   //! Perform the line width calibration
   virtual bool CalibrateFWHMModel();
   
  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The calibration model determination method
  unsigned int m_CalibrationModelDeterminationMethod;
  //! Fitting model of the calibration model determination method
  unsigned int m_CalibrationModelDeterminationMethodFittingEnergyModel;
  //! Fitting model of the calibration model determination method
  unsigned int m_CalibrationModelDeterminationMethodFittingFWHMModel;
  
  
#ifdef ___CLING___
 public:
  ClassDef(MCalibrateEnergyDetermineModel, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
