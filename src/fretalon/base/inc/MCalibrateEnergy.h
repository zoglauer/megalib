/*
 * MCalibrateEnergy.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MCalibrateEnergy__
#define __MCalibrateEnergy__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:
#include "TH1.h"

// MEGAlib libs:
#include "MCalibrate.h"
#include "MCalibrationSpectrum.h"
#include "MReadOutDataGroup.h"
#include "MIsotope.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A base class for the energy calibration tasks
class MCalibrateEnergy : public MCalibrate
{
  // public interface:
 public:
  //! Default constructor
  MCalibrateEnergy();
  //! Default destuctor 
  virtual ~MCalibrateEnergy();
  
  //! Set the range
  void SetRange(double Minimum, double Maximum) { m_RangeMinimum = Minimum; m_RangeMaximum = Maximum; }

  //! Add a read-out data group and the associated isotopes
  void AddReadOutDataGroup(const MReadOutDataGroup& ROG, const vector<MIsotope>& Isotopes);
  
  //! Perform the calibration
  virtual bool Calibrate() = 0;
    
  //! Get the current calibration result
  MCalibrationSpectrum GetCalibrationResult() { return m_Results; }
  //! Set the calibration result from a previous step
  void SetCalibrationResult(const MCalibrationSpectrum& Result) { m_Results = Result; } 
  
  // protected methods:
 protected:
  //! Create a histogram from the read-out data group using a fixed number of counts per bin
  TH1D* CreateHistogramCountsBinned(MReadOutDataGroup& R, double Min, double Max, int MinCounts, double MinBinWidth = 1);
  
  // private methods:
 private:



  // protected members:
 protected:
  //! Set the minimum range value
  double m_RangeMinimum;
  //! The maximum range value
  double m_RangeMaximum;
    
  //! The read-out data groups to be calibrated
  vector<MReadOutDataGroup> m_ROGs;
  //! The isotopes associated with the read-out data groups
  vector<vector<MIsotope> > m_Isotopes;

  //! Store the calibration results
  MCalibrationSpectrum m_Results;
  
  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MCalibrateEnergy, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
