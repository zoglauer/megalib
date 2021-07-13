/*
 * MCalibrate.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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
