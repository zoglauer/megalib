/*
 * MMelinator.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MMelinator__
#define __MMelinator__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:
#include "TF1.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TThread.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOutStore.h"
#include "MCalibrationStore.h"
#include "MCalibrationSpectralPoint.h"
#include "MCalibrationModel.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The MEGAlib line calibrator - the actual thing (i.e. library class not GUI)
//! This class acts as the facade (design pattern) to the underlying fretalon base library
class MMelinator
{
  // public interface:
 public:
  //! Default constructor for MEGAlib's line calibrator library
  MMelinator();
  //! Default destructor
  ~MMelinator();

  //! Clear all data
  void Clear();

  //! Set the ID of the selected detector (use all detectors when negative)
  void SetSelectedDetectorID(int SelectedDetector) { m_SelectedDetectorID = SelectedDetector; }
  
  //! Load the calibration data containing the given isotopes and eventually merge files with identical data groups
  //! Return false if an error occurred
  //! This function performs parallel loading of all given files
  bool Load(const vector<MString>& FileName, const vector<vector<MIsotope> >& Isotopes, const vector<unsigned int>& GroupIDs);
  
  //! Set the basic properties of the histogram
  void SetHistogramProperties(double Min, double Max, unsigned int HistogramBinningMode, double HistogramBinningModeValue) { 
    if (m_HistogramMin != Min) { m_HistogramMin = Min; m_HistogramChanged = true; }
    if (m_HistogramMax != Max) { m_HistogramMax = Max; m_HistogramChanged = true; }
    if (m_HistogramBinningMode != HistogramBinningMode) { m_HistogramBinningMode = HistogramBinningMode; m_HistogramChanged = true; }
    if (m_HistogramBinningModeValue != HistogramBinningModeValue) { m_HistogramBinningModeValue = HistogramBinningModeValue; m_HistogramChanged = true; }
  }
  
  //! Set the peak parametrization method
  void SetPeakParametrizationMethod(unsigned int Method) { m_PeakParametrizationMethod = Method; }
  
  //! Set the peak parametrization options for the fitting method
  void SetPeakParametrizationMethodFittedPeakOptions(unsigned int BackgroundModel, 
                                                     unsigned int EnergyLossModel, 
                                                     unsigned int PeakShapeModel) { 
    m_PeakParametrizationMethodFittedPeakBackgroundModel = BackgroundModel; 
    m_PeakParametrizationMethodFittedPeakEnergyLossModel = EnergyLossModel; 
    m_PeakParametrizationMethodFittedPeakPeakShapeModel = PeakShapeModel; }
  
  //! Set the calibration model determination method (number identical to what is defined in MCalibrateLines.h
  void SetCalibrationModelDeterminationMethod(unsigned int Method) { m_CalibrationModelDeterminationMethod = Method; }
  //! Set the fitting model options for the calibration model determination method
  void SetCalibrationModelDeterminationMethodFittingOptions(unsigned int Model) { 
    m_CalibrationModelDeterminationMethodFittingModel = Model; }

  
  //! Get the number of collections in the store
  unsigned int GetNumberOfCollections() const { return m_Store.GetNumberOfReadOutCollections(); }
  
  //! Get the number of groups in the store
  unsigned int GetNumberOfGroups() const { return m_Store.GetNumberOfReadOutDataGroups(); }
  
  //! Return true if we have calibration model
  bool HasCalibrationModel(unsigned int Collection);
  //! Get the calibration model of the spectra
  MCalibrationModel& GetCalibrationModel(unsigned int Collection);
  //! Get the number of calibration point in the spectra
  unsigned int GetNumberOfCalibrationSpectralPoints(unsigned int Collection);
  //! Return the given spectral point
  //! If it doesn't exist, the exception MExceptionIndexOutOfBounds is thrown
  MCalibrationSpectralPoint& GetCalibrationSpectralPoint(unsigned int Collection, unsigned int Line);  
  
  //! Draw the spectra into the Canvas for the given Collection
  void DrawSpectrum(TCanvas& Canvas, unsigned int Collection, unsigned int Line);
  
  //! Draw the line fit into the Canvas for the given Collection
  void DrawLineFit(TCanvas& Canvas, unsigned int Collection, unsigned int Line, 
                   unsigned int HistogramBinningMode, double HistogramBinningModeValue);
  
  //! Draw the calibration into the Canvas for the given Collection
  void DrawCalibration(TCanvas& Canvas, unsigned int Collection);

  
  //! Return a non-const reference to a collection in the store
  //! The collection must exist otherwise the exception MExceptionIndexOutOfBounds is thrown
  MReadOutCollection& GetCollection(unsigned int Collection) { return m_Store.GetReadOutCollection(Collection); }
  
  //! Get the histogram for the given collection
  //! The collection and group must exist otherwise the exception MExceptionIndexOutOfBounds is thrown
  TH1D& GetHistogram(unsigned int Collection, unsigned int Group);
 
  //! Perform the calibration of all collections
  bool Calibrate(bool ShowDiagnostics = false);
 
  //! Perform the calibration of the given collection
  bool Calibrate(unsigned int Collection, bool ShowDiagnostics = false);

  //! Given an existing calibration for the collection, do a reassignment of energies and determination of the model
  bool ReCalibrateModel(unsigned int Collection);

  //! Return the calibration quality
  double GetCalibrationQuality(unsigned int Collection);
  
  //! Remove all collection which don't have positive data entries
  //void RemoveCollectionsWithNoPositiveEntries() { m_Store.RemoveCollectionsWithNoPositiveEntries(); }
 
  //! Save the calibration in e-cal format
  bool Save(MString FileName);
  
  //! Dump a string
  virtual MString ToString() const;  
  
  //! ID for binning the histograms with a fixed number of bins
  static const unsigned int c_HistogramBinningModeFixedNumberOfBins = 0;
  //! ID for binning the histograms with a fixed number of counts per bin
  static const unsigned int c_HistogramBinningModeFixedCountsPerBin = 1;
  //! ID for binning the histograms using Bayesian blocks
  static const unsigned int c_HistogramBinningModeBayesianBlocks = 2;

  //! This function is executed by the parallel calibration threads
  bool CalibrateParallel(unsigned int ThreadID);
  //! This function is executed by the parallel loading threads
  bool LoadParallel(unsigned int ThreadID);
  
  
  // protected methods:
 protected:
  //! Create the histogram for the given read-out data group
  TH1D* CreateSpectrum(const MString& Title, MReadOutDataGroup& G, double Min, double Max, unsigned int HistogramBinningMode, double HistogramBinningModeValue);

  
  
  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The storage for all read events
  MReadOutStore m_Store;
  //! The storage for all calibrations
  MCalibrationStore m_CalibrationStore;
  
  //! The selected detector ID (negative means all detectors)
  int m_SelectedDetectorID;
  
  //! The group IDs
  vector<unsigned int> m_GroupIDs;
  //! The calibration file names
  vector<MString> m_CalibrationFileNames;
  //! The loading progress of the given file:
  vector<double> m_CalibrationFileLoadingProgress;
  //! The isotopes per input file
  vector<vector<MIsotope> > m_Isotopes;
  
  //! The total number of lines to consider --- only for the total memory calculation
  unsigned int m_NLinesToConsider;
  
  //! The minimum range of the histogram
  double m_HistogramMin;
  //! The maximum range of the histogram
  double m_HistogramMax;
  //! The binning mode: fixed number of bins, fixed cts per bin, Bayesian block
  unsigned int m_HistogramBinningMode;
  //! Depending on the binning mode, either bins, cts/bin, or prior
  double m_HistogramBinningModeValue;
  //! True if the histogram settings have changed since the last draw
  bool m_HistogramChanged;
  //! The last drawn collection
  unsigned int m_HistogramCollection;
  //! The current histograms
  vector<TH1D*> m_Histograms;
 
  //! The peak parametrization method in during calibration of the lines
  unsigned int m_PeakParametrizationMethod;
  
  //! The background model for peak fitting
  unsigned int m_PeakParametrizationMethodFittedPeakBackgroundModel; 
  //! The energy loss model for peak fitting
  unsigned int m_PeakParametrizationMethodFittedPeakEnergyLossModel; 
  //! The peak shape model for peak fitting
  unsigned int m_PeakParametrizationMethodFittedPeakPeakShapeModel;
 
  //! The calibration model determination method
  unsigned int m_CalibrationModelDeterminationMethod;
  //! Fitting model of the calibration model determination method
  unsigned int m_CalibrationModelDeterminationMethodFittingModel;
  
  
  //! Number of threads
  unsigned int m_NThreads;
  //! Storing the threads:
  vector<TThread*> m_Threads;
  //! Storing a flag that the thread is running
  vector<bool> m_ThreadIsInitialized;
  //! Storing a flag that the thread should terminate itself
  vector<bool> m_ThreadShouldTerminate;
  //! Storing a flag that the thread is finished
  vector<bool> m_ThreadIsFinished;
  //! ID of the next item to be processed
  unsigned int m_ThreadNextItem;

  
#ifdef ___CINT___
 public:
  ClassDef(MMelinator, 0) // no description
#endif

};


////////////////////////////////////////////////////////////////////////////////


//! This function encapsulates the information transfered into the thread
class MMelinatorThreadCaller
{
 public:
  //! Standard constructor
  MMelinatorThreadCaller(MMelinator* M, unsigned int ThreadID) {
    m_Melinator = M;
    m_ThreadID = ThreadID;
  }

  //! Return the calling class
  MMelinator* GetThreadCaller() { return m_Melinator; }
  //! Return the thread ID
  unsigned int GetThreadID() { return m_ThreadID; }

 private:
  //! Store the calling class for retrieval
  MMelinator* m_Melinator;
  //! ID of the worker thread
  unsigned int m_ThreadID;
};


////////////////////////////////////////////////////////////////////////////////


//! Thread entry point for the parallel calibration
void MMelinatorCallParallelCalibrationThread(void* address);

//! Thread entry point for the parallel calibration
void MMelinatorCallParallelLoadingThread(void* address);

//! Streamify the read-out data
std::ostream& operator<<(std::ostream& os, const MMelinator& R);

#endif


////////////////////////////////////////////////////////////////////////////////
