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
  
  //! Set the range
  void SetRange(double Minimum, double Maximum) { m_RangeMinimum = Minimum; m_RangeMaximum = Maximum; }
  
  //! Perform the calibration
  virtual bool Calibrate() = 0;
  
  // protected methods:
 protected:
  //! Create a histogram from the read-out data group using a fixed number of counts per bin
  TH1D* CreateHistogramCountsBinned(MReadOutDataGroup& R, double Min, double Max, int MinCounts, double MinBinWidth = 1);
  
  // private methods:
 private:



  // protected members:
 protected:
  //! True when we are in diagnostics mode
  bool m_DiagnosticsMode;

  //! Set the minimum range value
  double m_RangeMinimum;
  //! The maximum range value
  double m_RangeMaximum;
  
  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MCalibrate, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
