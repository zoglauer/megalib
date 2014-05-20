/*
 * MCalibrationSpectralPoint.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MCalibrationSpectralPoint__
#define __MCalibrationSpectralPoint__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MIsotope.h"
#include "MCalibrationFit.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A class representing one calibration point in a spectral calibration
class MCalibrationSpectralPoint
{
  // public interface:
 public:
  //! Default constructor
  MCalibrationSpectralPoint();
  //! Default destuctor 
  virtual ~MCalibrationSpectralPoint();

  //! Set whether this point is good or not
  void IsGood(bool Flag) { m_IsGood = Flag; }
  //! Return whether this point is good or not
  bool IsGood() const { return m_IsGood; }
  
  //! Set the peak
  void SetPeak(double Peak) { m_Peak = Peak; }
  //! Get the peak
  double GetPeak() const { return m_Peak; }
  
  //! Set the FWHM in ADCs
  void SetFWHM(double FWHM) { m_FWHM = FWHM; }
  //! Get the FWHM in ADCs
  double GetFWHM() const { return m_FWHM; }

  //! Set the energy
  void SetEnergy(double Energy) { m_Energy = Energy; }
  //! Get the energy
  double GetEnergy() const { return m_Energy; }

  //! Get the FWHM in energy units (keV)
  double GetEnergyFWHM() const { if (m_Peak == 0) return 0.0; else return m_FWHM/m_Peak * m_Energy; }

  //! Set the number of counts
  void SetCounts(double Counts) { m_Counts = Counts; }
  //! Get the number of counts
  double GetCounts() const { return m_Counts; }

  //! Set the low edge
  void SetLowEdge(double LowEdge) { m_LowEdge = LowEdge; }
  //! Get the low edge
  double GetLowEdge() const { return m_LowEdge; }

  //! Set the high edge
  void SetHighEdge(double HighEdge) { m_HighEdge = HighEdge; }
  //! Get the high edge
  double GetHighEdge() const { return m_HighEdge; }

  //! Return true if we have a fit
  bool HasFit() const { return (m_Fit != 0) ? true : false; }
  //! Set the fit
  void SetFit(const MCalibrationFit& Fit) { delete m_Fit; m_Fit = Fit.Clone(); }
  //! Get the fit, if it doesn't exist throw MExceptionObjectDoesNotExist
  //! Bad design since a new fit can be set any time...
  const MCalibrationFit& GetFit() const;
  //! Get the fit, if it doesn't exist throw MExceptionObjectDoesNotExist
  //! Bad design since a new fit can be set any time...
  MCalibrationFit& GetFit();

  //! Set the isotope
  void SetIsotope(MIsotope Isotope) { m_Isotope = Isotope; }
  //! Get the energy
  MIsotope GetIsotope() const { return m_Isotope; }
  
  //! Dump a string
  MString ToString() const;
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The peak
  double m_Peak;
  //! The FWHM
  double m_FWHM;
  //! The counts
  double m_Counts;
  //! The low edge
  double m_LowEdge;
  //! The high edge
  double m_HighEdge;

  //! The fit
  MCalibrationFit* m_Fit;
  
  //! The isotope
  MIsotope m_Isotope;
  //! The associates energy
  double m_Energy;
  
  //! Flag indicating that this point is good
  bool m_IsGood;
  
  
#ifdef ___CINT___
 public:
  ClassDef(MCalibrationSpectralPoint, 0) // no description
#endif

};

//! Streamify the read-out element
ostream& operator<<(ostream& os, const MCalibrationSpectralPoint& R);

#endif


////////////////////////////////////////////////////////////////////////////////
