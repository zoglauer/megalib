/*
 * MCalibrationSpectrum.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MCalibrationSpectrum__
#define __MCalibrationSpectrum__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MCalibration.h"
#include "MCalibrationSpectralPoint.h"
#include "MCalibrationModel.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A calibrated spectrum with one or more calibration points
//! It is intended to be generated for one read-out element, and the points are organized
//! as points per the read-out-data groups (= spectra for one calibration file)
//! Thus you have to iterate over the read-out data groups and then the points
class MCalibrationSpectrum : public MCalibration 
{
  // public interface:
 public:
  //! Default constructor
  MCalibrationSpectrum();
  //! Default destuctor 
  virtual ~MCalibrationSpectrum();

  //! Clear the content
  void Clear();

  //! Clone this calibration - the returned calibration must be deleted!
  virtual MCalibrationSpectrum* Clone() const;
  
  //! Set the number of read-out data groups covered by this spectrum
  void SetNumberOfReadOutDataGroups(unsigned int N) { m_SpectralPoints.resize(N); }
  //! Get the number of read-out data groups
  unsigned int GetNumberOfReadOutDataGroups() const { return m_SpectralPoints.size(); }
  
  //! Add a spectral point to the read-out data group
  void AddSpectralPoint(unsigned int ROG, const MCalibrationSpectralPoint& Point); 
  //! Get the number of spectral points in the roud-out data group
  unsigned int GetNumberOfSpectralPoints(unsigned int ROG) const;
  //! Get a spectral point 
  //! If it or the read-out data group does not exist return an MExceptionIndexOutOfBounds exception
  MCalibrationSpectralPoint& GetSpectralPoint(unsigned int ROG, unsigned int Point);
  
  //! Remove a point - the check if they are indentical is soly based on the peak position.
  void RemoveAllBadSpectralPoints();
  
  //! Get a list of all unique spectral points in this spectrum
  vector<MCalibrationSpectralPoint> GetUniquePoints();
  
  //! Set the calibration model
  void SetEnergyModel(MCalibrationModel& EnergyModel);
  //! Get the calibration model, if it doesn't exist throw MExceptionObjectDoesNotExist
  MCalibrationModel& GetEnergyModel();
  //! Return true if we have a model
  bool HasEnergyModel() const { if (m_EnergyModel != nullptr) return true; else return false; }
  //! Remove the model
  void RemoveEnergyModel();
  
  //! Set the line width calibration model
  void SetFWHMModel(MCalibrationModel& FWHMModel);
  //! Get the line width calibration model, if it doesn't exist throw MExceptionObjectDoesNotExist
  MCalibrationModel& GetFWHMModel();
  //! Return true if we have a line width model
  bool HasFWHMModel() const { if (m_FWHMModel != nullptr) return true; else return false; }
  //! Remove the line width model
  void RemoveFWHMModel();
  
  //! Return the data as parsable string - this function is just here to satisfy the compiler...
  virtual MString ToParsableString(bool WithDescriptor = false) { return ToParsableString("pak", WithDescriptor); }
  //! Return the data as parsable string
  virtual MString ToParsableString(const MString& Mode, bool WithDescriptor = false);


  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The spectral points
  vector<vector<MCalibrationSpectralPoint> > m_SpectralPoints;
  //! Their calibration model
  MCalibrationModel* m_EnergyModel;
  //! Their line width calibration model
  MCalibrationModel* m_FWHMModel;

  
  
#ifdef ___CLING___
 public:
  ClassDef(MCalibrationSpectrum, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
