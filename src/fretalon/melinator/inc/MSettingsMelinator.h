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

  
  //! Set the first calibration file
  void SetCalibrationFile1(const MString& File) { m_CalibrationFile1 = File; }
  //! Get the first calibration file
  MString GetCalibrationFile1() const { return m_CalibrationFile1; }
  
  //! Set the isotope 1 of calibration file 1
  void SetCalibrationFile1Isotope1(const MString& Isotope) { m_CalibrationFile1Isotope1 = Isotope; }
  //! Set the isotope 1 of calibration file 1
  MString GetCalibrationFile1Isotope1() const { return m_CalibrationFile1Isotope1; }
  //! Set the isotope 2 of calibration file 1
  void SetCalibrationFile1Isotope2(const MString& Isotope) { m_CalibrationFile1Isotope2 = Isotope; }
  //! Set the isotope 2 of calibration file 1
  MString GetCalibrationFile1Isotope2() const { return m_CalibrationFile1Isotope2; }
  //! Set the isotope 3 of calibration file 1
  void SetCalibrationFile1Isotope3(const MString& Isotope) { m_CalibrationFile1Isotope3 = Isotope; }
  //! Set the isotope 3 of calibration file 1
  MString GetCalibrationFile1Isotope3() const { return m_CalibrationFile1Isotope3; }

  
  //! Set the second calibration file
  void SetCalibrationFile2(const MString& File) { m_CalibrationFile2 = File; }
  //! Get the second calibration file
  MString GetCalibrationFile2() const { return m_CalibrationFile2; }
  
  //! Set the isotope 1 of calibration file 1
  void SetCalibrationFile2Isotope1(const MString& Isotope) { m_CalibrationFile2Isotope1 = Isotope; }
  //! Set the isotope 1 of calibration file 1
  MString GetCalibrationFile2Isotope1() const { return m_CalibrationFile2Isotope1; }
  //! Set the isotope 2 of calibration file 1
  void SetCalibrationFile2Isotope2(const MString& Isotope) { m_CalibrationFile2Isotope2 = Isotope; }
  //! Set the isotope 2 of calibration file 1
  MString GetCalibrationFile2Isotope2() const { return m_CalibrationFile2Isotope2; }
  //! Set the isotope 3 of calibration file 1
  void SetCalibrationFile2Isotope3(const MString& Isotope) { m_CalibrationFile2Isotope3 = Isotope; }
  //! Set the isotope 3 of calibration file 1
  MString GetCalibrationFile2Isotope3() const { return m_CalibrationFile2Isotope3; }

  
  //! Set the third calibration file
  void SetCalibrationFile3(const MString& File) { m_CalibrationFile3 = File; }
  //! Get the third calibration file
  MString GetCalibrationFile3() const { return m_CalibrationFile3; }
  
  //! Set the isotope 1 of calibration file 1
  void SetCalibrationFile3Isotope1(const MString& Isotope) { m_CalibrationFile3Isotope1 = Isotope; }
  //! Set the isotope 1 of calibration file 1
  MString GetCalibrationFile3Isotope1() const { return m_CalibrationFile3Isotope1; }
  //! Set the isotope 2 of calibration file 1
  void SetCalibrationFile3Isotope2(const MString& Isotope) { m_CalibrationFile3Isotope2 = Isotope; }
  //! Set the isotope 2 of calibration file 1
  MString GetCalibrationFile3Isotope2() const { return m_CalibrationFile3Isotope2; }
  //! Set the isotope 3 of calibration file 1
  void SetCalibrationFile3Isotope3(const MString& Isotope) { m_CalibrationFile3Isotope3 = Isotope; }
  //! Set the isotope 3 of calibration file 1
  MString GetCalibrationFile3Isotope3() const { return m_CalibrationFile3Isotope3; }

  
  //! Set the fourth calibration file
  void SetCalibrationFile4(const MString& File) { m_CalibrationFile4 = File; }
  //! Get the fourth calibration file
  MString GetCalibrationFile4() const { return m_CalibrationFile4; }
  
  //! Set the isotope 1 of calibration file 1
  void SetCalibrationFile4Isotope1(const MString& Isotope) { m_CalibrationFile4Isotope1 = Isotope; }
  //! Set the isotope 1 of calibration file 1
  MString GetCalibrationFile4Isotope1() const { return m_CalibrationFile4Isotope1; }
  //! Set the isotope 2 of calibration file 1
  void SetCalibrationFile4Isotope2(const MString& Isotope) { m_CalibrationFile4Isotope2 = Isotope; }
  //! Set the isotope 2 of calibration file 1
  MString GetCalibrationFile4Isotope2() const { return m_CalibrationFile4Isotope2; }
  //! Set the isotope 3 of calibration file 1
  void SetCalibrationFile4Isotope3(const MString& Isotope) { m_CalibrationFile4Isotope3 = Isotope; }
  //! Set the isotope 3 of calibration file 1
  MString GetCalibrationFile4Isotope3() const { return m_CalibrationFile4Isotope3; }

  
  //! Set the fifth calibration file
  void SetCalibrationFile5(const MString& File) { m_CalibrationFile5 = File; }
  //! Get the fifth calibration file
  MString GetCalibrationFile5() const { return m_CalibrationFile5; }
  
  //! Set the isotope 1 of calibration file 1
  void SetCalibrationFile5Isotope1(const MString& Isotope) { m_CalibrationFile5Isotope1 = Isotope; }
  //! Set the isotope 1 of calibration file 1
  MString GetCalibrationFile5Isotope1() const { return m_CalibrationFile5Isotope1; }
  //! Set the isotope 2 of calibration file 1
  void SetCalibrationFile5Isotope2(const MString& Isotope) { m_CalibrationFile5Isotope2 = Isotope; }
  //! Set the isotope 2 of calibration file 1
  MString GetCalibrationFile5Isotope2() const { return m_CalibrationFile5Isotope2; }
  //! Set the isotope 3 of calibration file 1
  void SetCalibrationFile5Isotope3(const MString& Isotope) { m_CalibrationFile5Isotope3 = Isotope; }
  //! Set the isotope 3 of calibration file 1
  MString GetCalibrationFile5Isotope3() const { return m_CalibrationFile5Isotope3; }

  
  //! Set the fifth calibration file
  void SetCalibrationFile6(const MString& File) { m_CalibrationFile6 = File; }
  //! Get the fifth calibration file
  MString GetCalibrationFile6() const { return m_CalibrationFile6; }
  
  //! Set the isotope 1 of calibration file 1
  void SetCalibrationFile6Isotope1(const MString& Isotope) { m_CalibrationFile6Isotope1 = Isotope; }
  //! Set the isotope 1 of calibration file 1
  MString GetCalibrationFile6Isotope1() const { return m_CalibrationFile6Isotope1; }
  //! Set the isotope 2 of calibration file 1
  void SetCalibrationFile6Isotope2(const MString& Isotope) { m_CalibrationFile6Isotope2 = Isotope; }
  //! Set the isotope 2 of calibration file 1
  MString GetCalibrationFile6Isotope2() const { return m_CalibrationFile6Isotope2; }
  //! Set the isotope 3 of calibration file 1
  void SetCalibrationFile6Isotope3(const MString& Isotope) { m_CalibrationFile6Isotope3 = Isotope; }
  //! Set the isotope 3 of calibration file 1
  MString GetCalibrationFile6Isotope3() const { return m_CalibrationFile6Isotope3; }

  
  //! Set the fifth calibration file
  void SetCalibrationFile7(const MString& File) { m_CalibrationFile7 = File; }
  //! Get the fifth calibration file
  MString GetCalibrationFile7() const { return m_CalibrationFile7; }
  
  //! Set the isotope 1 of calibration file 1
  void SetCalibrationFile7Isotope1(const MString& Isotope) { m_CalibrationFile7Isotope1 = Isotope; }
  //! Set the isotope 1 of calibration file 1
  MString GetCalibrationFile7Isotope1() const { return m_CalibrationFile7Isotope1; }
  //! Set the isotope 2 of calibration file 1
  void SetCalibrationFile7Isotope2(const MString& Isotope) { m_CalibrationFile7Isotope2 = Isotope; }
  //! Set the isotope 2 of calibration file 1
  MString GetCalibrationFile7Isotope2() const { return m_CalibrationFile7Isotope2; }
  //! Set the isotope 3 of calibration file 1
  void SetCalibrationFile7Isotope3(const MString& Isotope) { m_CalibrationFile7Isotope3 = Isotope; }
  //! Set the isotope 3 of calibration file 1
  MString GetCalibrationFile7Isotope3() const { return m_CalibrationFile7Isotope3; }
  
    
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

  //! Get a flag indictaing the x-axis is displayed logarithmic
  bool GetHistogramLogX() const { return m_HistogramLogX; }
  //! Set a flag indictaing the x-axis is displayed logarithmic
  void SetHistogramLogX(bool HistogramLogX) { m_HistogramLogX = HistogramLogX; }

  //! Get a flag indictaing the y-axis is displayed logarithmic
  bool GetHistogramLogY() const { return m_HistogramLogY; }
  //! Set a flag indictaing the y-axis is displayed logarithmic
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
  //! The first calibration file
  MString m_CalibrationFile1;
  //! The first isotope of the first calibration file
  MString m_CalibrationFile1Isotope1;
  //! The second isotope of the first calibration file
  MString m_CalibrationFile1Isotope2;
  //! The third isotope of the first calibration file
  MString m_CalibrationFile1Isotope3;
  
  //! The first calibration file
  MString m_CalibrationFile2;
  //! The first isotope of the first calibration file
  MString m_CalibrationFile2Isotope1;
  //! The second isotope of the first calibration file
  MString m_CalibrationFile2Isotope2;
  //! The third isotope of the first calibration file
  MString m_CalibrationFile2Isotope3;
  
  //! The first calibration file
  MString m_CalibrationFile3;
  //! The first isotope of the first calibration file
  MString m_CalibrationFile3Isotope1;
  //! The second isotope of the first calibration file
  MString m_CalibrationFile3Isotope2;
  //! The third isotope of the first calibration file
  MString m_CalibrationFile3Isotope3;
  
  //! The first calibration file
  MString m_CalibrationFile4;
  //! The first isotope of the first calibration file
  MString m_CalibrationFile4Isotope1;
  //! The second isotope of the first calibration file
  MString m_CalibrationFile4Isotope2;
  //! The third isotope of the first calibration file
  MString m_CalibrationFile4Isotope3;
  
  //! The first calibration file
  MString m_CalibrationFile5;
  //! The first isotope of the first calibration file
  MString m_CalibrationFile5Isotope1;
  //! The second isotope of the first calibration file
  MString m_CalibrationFile5Isotope2;
  //! The third isotope of the first calibration file
  MString m_CalibrationFile5Isotope3;
  
  //! The first calibration file
  MString m_CalibrationFile6;
  //! The first isotope of the first calibration file
  MString m_CalibrationFile6Isotope1;
  //! The second isotope of the first calibration file
  MString m_CalibrationFile6Isotope2;
  //! The third isotope of the first calibration file
  MString m_CalibrationFile6Isotope3;
  
  //! The first calibration file
  MString m_CalibrationFile7;
  //! The first isotope of the first calibration file
  MString m_CalibrationFile7Isotope1;
  //! The second isotope of the first calibration file
  MString m_CalibrationFile7Isotope2;
  //! The third isotope of the first calibration file
  MString m_CalibrationFile7Isotope3;
  
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
  
  //! The calibration model determination method
  unsigned int m_CalibrationModelDeterminationMethod;
  //! Fitting model of the calibration model determination method
  unsigned int m_CalibrationModelDeterminationMethodFittingModel;
  
  //! Set the save as file
  MString m_SaveAsFileName;
  
  //! Set the single detector to use (negative means use all)
  int m_SelectedDetectorID;
  
  
#ifdef ___CINT___
 public:
  ClassDef(MSettingsMelinator, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
