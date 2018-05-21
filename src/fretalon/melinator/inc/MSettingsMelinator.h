/*
 * MSettingsMelinator.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSettingsMelinator__
#define __MSettingsMelinator__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSettings.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The settings of the melinator GUI
class MSettingsMelinator : public MSettings
{
  // public Session:
 public:
  //! Standard constructor
  MSettingsMelinator(bool AutoLoad = true);
  //! Default destructor
  virtual ~MSettingsMelinator();

  
  //! Set all calibration files, group IDs, and Isotopes at once
  void SetAllCalibrationFiles(const vector<MString>& Files, const vector<unsigned int> GroupIDs, const vector<vector<MString>>& Isotopes);
  
  //! Get the calibration files
  vector<MString> GetCalibrationFiles() const { return m_CalibrationFiles; }
  //! Get the calibration group IDs
  vector<unsigned int> GetCalibrationGroupIDs() const { return m_CalibrationGroupIDs; }
  //! Get the calibration isotope names
  vector<vector<MString>> GetCalibrationIsotopes() const { return m_CalibrationIsotopes; }
    
    
  //! The minimum range of the histogram
  double GetHistogramMin() const { return m_HistogramMin; }
  //! Set the histogram minimum range
  void SetHistogramMin(double HistogramMin) { m_HistogramMin = HistogramMin; }
  
  //! The maximum range of the histogram
  double GetHistogramMax() const { return m_HistogramMax; }
  //! Set the histogram maximum range
  void SetHistogramMax(double HistogramMax) { m_HistogramMax = HistogramMax; }
  
  //! Get the binning mode: 0: fixed number of bins, 1: fixed cts per bin, 2: Bayesian block
  unsigned int GetHistogramBinningMode() const { return m_HistogramBinningMode; }
  //! Set the binning mode: 0: fixed number of bins, 1: fixed cts per bin, 2: Bayesian block
  void SetHistogramBinningMode(unsigned int HistogramBinningMode) { m_HistogramBinningMode = HistogramBinningMode; }
  
  //! Get the binning mode value: Depending on the binning mode, either bins, cts/bin, or prior
  double GetHistogramBinningModeValue() const { return m_HistogramBinningModeValue; }
  //! Set the binning mode value: Depending on the binning mode, either bins, cts/bin, or prior
  void SetHistogramBinningModeValue(double HistogramBinningModeValue) { m_HistogramBinningModeValue = HistogramBinningModeValue; }

  //! Get a flag indicating the x-axis is displayed logarithmic
  bool GetHistogramLogX() const { return m_HistogramLogX; }
  //! Set a flag indicating the x-axis is displayed logarithmic
  void SetHistogramLogX(bool HistogramLogX) { m_HistogramLogX = HistogramLogX; }

  //! Get a flag indicating the y-axis is displayed logarithmic
  bool GetHistogramLogY() const { return m_HistogramLogY; }
  //! Set a flag indicating the y-axis is displayed logarithmic
  void SetHistogramLogY(bool HistogramLogY) { m_HistogramLogY = HistogramLogY; }
  
  
  //! Get the peak binning mode: 0: fixed number of bins, 1: fixed cts per bin, 2: Bayesian block
  unsigned int GetPeakHistogramBinningMode() const { return m_PeakHistogramBinningMode; }
  //! Set the peak binning mode: 0: fixed number of bins, 1: fixed cts per bin, 2: Bayesian block
  void SetPeakHistogramBinningMode(unsigned int PeakHistogramBinningMode) { m_PeakHistogramBinningMode = PeakHistogramBinningMode; }
  
  //! Get the peak binning mode value: Depending on the binning mode, either bins, cts/bin, or prior
  double GetPeakHistogramBinningModeValue() const { return m_PeakHistogramBinningModeValue; }
  //! Set the peak binning mode value: Depending on the binning mode, either bins, cts/bin, or prior
  void SetPeakHistogramBinningModeValue(double PeakHistogramBinningModeValue) { m_PeakHistogramBinningModeValue = PeakHistogramBinningModeValue; }

  
  //! Get the histogram parametrization method (number identical to what is defined in MMelinator.h)
  unsigned int GetPeakParametrizationMethod() const { return m_PeakParametrizationMethod; }
  //! Set the histogram parametrization method (number identical to what is defined in MMelinator.h)
  void SetPeakParametrizationMethod(unsigned int PeakParametrizationMethod) { m_PeakParametrizationMethod = PeakParametrizationMethod; }

  //! Get the background model for fitting (number identical to what is defined in MCalibrateLines.h)
  unsigned int GetPeakParametrizationMethodFittingBackgroundModel() const { return m_PeakParametrizationMethodFittingBackgroundModel; } 
  //! Set the background model for fitting (number identical to what is defined in MCalibrateLines.h)
  void SetPeakParametrizationMethodFittingBackgroundModel(unsigned int Model) { m_PeakParametrizationMethodFittingBackgroundModel = Model; } 
  //! Get the energy-loss model for fitting (number identical to what is defined in MCalibrateLines.h)
  unsigned int GetPeakParametrizationMethodFittingEnergyLossModel() const { return m_PeakParametrizationMethodFittingEnergyLossModel; } 
  //! Set the energy-loss model for fitting (number identical to what is defined in MCalibrateLines.h)
  void SetPeakParametrizationMethodFittingEnergyLossModel(unsigned int Model) { m_PeakParametrizationMethodFittingEnergyLossModel = Model; } 
  //! Get the peak-shape model for fitting (number identical to what is defined in MCalibrateLines.h)
  unsigned int GetPeakParametrizationMethodFittingPeakShapeModel() const { return m_PeakParametrizationMethodFittingPeakShapeModel; } 
  //! Set the peak-shape model for fitting (number identical to what is defined in MCalibrateLines.h)
  void SetPeakParametrizationMethodFittingPeakShapeModel(unsigned int Model) { m_PeakParametrizationMethodFittingPeakShapeModel = Model; } 

  
  //! Get a flag indicating if we should assume the calibration model will cross close to 0
  bool GetCalibrationModelZeroCrossing() const { return m_CalibrationModelZeroCrossing; }
  //! Set a flag indicating if we should assume the calibration model will cross close to 0
  void SetCalibrationModelZeroCrossing(bool CalibrationModelZeroCrossing) { m_CalibrationModelZeroCrossing = CalibrationModelZeroCrossing; }
  //! Get the calibration model determination method (number identical to what is defined in MCalibrateLines.h
  unsigned int GetCalibrationModelDeterminationMethod() const { return m_CalibrationModelDeterminationMethod; }
  //! Set the calibration model determination method (number identical to what is defined in MCalibrateLines.h
  void SetCalibrationModelDeterminationMethod(unsigned int CalibrationModelDeterminationMethod) { m_CalibrationModelDeterminationMethod = CalibrationModelDeterminationMethod; }
  //! Get the calibration model for fitting (number identical to what is defined in MCalibratenModel.h)
  unsigned int GetCalibrationModelDeterminationMethodFittingModel() const { return m_CalibrationModelDeterminationMethodFittingModel; } 
  //! Set the calibration model for fitting (number identical to what is defined in MCalibratenModel.h)
  void SetCalibrationModelDeterminationMethodFittingModel(unsigned int Model) { m_CalibrationModelDeterminationMethodFittingModel = Model; } 
  
  
  //! Set the save-as file name
  void SetSaveAsFileName(const MString& SaveAsFileName) { m_SaveAsFileName = SaveAsFileName; }
  //! Get the save-as file name
  MString GetSaveAsFileName() const { return m_SaveAsFileName; }

  //! Set the single detector to use (negative means use all)
  void SetSelectedDetectorID(int ID) { m_SelectedDetectorID = ID; }
  //! Get the single detector to use (negative means use all)
  int GetSelectedDetectorID() const { return m_SelectedDetectorID; }
  
  
// protected members:
 protected:
  //! Read all data from an XML tree
  virtual bool ReadXml(MXmlNode* Node);
  //! Write all data to an XML tree
  virtual bool WriteXml(MXmlNode* Node);

  // private members:
 private:
  //! A list of all calibration files
  vector<MString> m_CalibrationFiles;
  //! A list of all calibration groups
  vector<unsigned int> m_CalibrationGroupIDs;
  //! A list of all isotopes
  vector<vector<MString>> m_CalibrationIsotopes;
  
  //! The minimum range of the histogram
  double m_HistogramMin;
  //! The maximum range of the histogram
  double m_HistogramMax;
  //! The binning mode: fixed number of bins, fixed cts per bin, Bayesian block
  unsigned int m_HistogramBinningMode;
  //! Depending on the binning mode, either bins, cts/bin, or prior
  double m_HistogramBinningModeValue;
  //! Flag indicating that the x-axis is displayed logarithmic
  bool m_HistogramLogX;
  //! Flag indicating that the y-axis is displayed logarithmic
  bool m_HistogramLogY;
  
  //! The binning mode for the peak histogram: fixed number of bins, fixed cts per bin, Bayesian block
  unsigned int m_PeakHistogramBinningMode;
  //! Depending on the binning mode, either bins, cts/bin, or prior
  double m_PeakHistogramBinningModeValue;
  
  //! The peak parametrization mode: Bayesian blocks, smoothing, fitting
  unsigned int m_PeakParametrizationMethod;
  //! The background model for peak fitting
  unsigned int m_PeakParametrizationMethodFittingBackgroundModel;
  //! The energy-loss model for peak fitting
  unsigned int m_PeakParametrizationMethodFittingEnergyLossModel;
  //! The peak-shape model for peak fitting
  unsigned int m_PeakParametrizationMethodFittingPeakShapeModel;
  
  //! Flag indicating if we should assume the calibration model will cross close to 0
  bool m_CalibrationModelZeroCrossing;
  //! The calibration model determination method
  unsigned int m_CalibrationModelDeterminationMethod;
  //! Fitting model of the calibration model determination method
  unsigned int m_CalibrationModelDeterminationMethodFittingModel;
  
  //! Set the save as file
  MString m_SaveAsFileName;
  
  //! Set the single detector to use (negative means use all)
  int m_SelectedDetectorID;
  
  
#ifdef ___CLING___
 public:
  ClassDef(MSettingsMelinator, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
