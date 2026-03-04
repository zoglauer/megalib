/*
 * MSettingsResolutions.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSettingsResolutions__
#define __MSettingsResolutions__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// Root libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSettings.h"
#include "MSettingsSpectralOptions.h"
#include "MSettingsEventSelections.h"
#include "MSettingsImaging.h"
#include "MPointSource.h"
#include "MEventSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The settings for the ARM and spectral resolution plots
class MSettingsResolutions : public MSettingsInterface
{
  // public:
 public:
  //! Default constructor
  MSettingsResolutions();
  //! Defayult destructor
  virtual ~MSettingsResolutions();

 
  //! Get the coordiante system
  virtual MCoordinateSystem GetCoordinateSystem() const { return m_CoordinateSystem; }
  //! Set the coordinate system
  virtual void SetCoordinateSystem(MCoordinateSystem CoordinateSystem) { m_CoordinateSystem = CoordinateSystem; }


  // Test positions where to calculate the angular resolution
  
  bool GetTPUse() const { return m_TPUse; }
  void SetTPUse(bool TPUse) { m_TPUse = TPUse; } 
  
  double GetTPTheta() const { return m_TPTheta; }
  void SetTPTheta(double TPTheta) { m_TPTheta = TPTheta; }
  double GetTPPhi() const { return m_TPPhi; }
  void SetTPPhi(double TPPhi) { m_TPPhi = TPPhi; }
  
  double GetTPGalLongitude() const { return m_TPGalLongitude; }
  void SetTPGalLongitude(double TPGalLongitude) { m_TPGalLongitude = TPGalLongitude; }
  double GetTPGalLatitude() const { return m_TPGalLatitude; }
  void SetTPGalLatitude(double TPGalLatitude) { m_TPGalLatitude = TPGalLatitude; }
  
  double GetTPX() const { return m_TPX; }
  void SetTPX(double TPX) { m_TPX = TPX; }
  double GetTPY() const { return m_TPY; }
  void SetTPY(double TPY) { m_TPY = TPY; }
  double GetTPZ() const { return m_TPZ; }
  void SetTPZ(double TPZ) { m_TPZ = TPZ; }
  
  double GetTPDistanceTrans() const { return m_TPDistanceTrans; }
  void SetTPDistanceTrans(double TPDistanceTrans) { m_TPDistanceTrans = TPDistanceTrans; }
  
  double GetTPDistanceLong() const { return m_TPDistanceLong; }
  void SetTPDistanceLong(double TPDistanceLong) { m_TPDistanceLong = TPDistanceLong; }


  // Log binning
  bool GetLogBinningSpectrum() const { return m_LogBinningSpectrum; }
  void SetLogBinningSpectrum(bool LogBinningSpectrum) { m_LogBinningSpectrum = LogBinningSpectrum; } 
  

  // Bin sizes for certain histograms:  double GetTPDistanceTrans();
  int GetHistBinsARMGamma() const { return m_HistBinsARMGamma; }
  void SetHistBinsARMGamma(int HistBinsARMGamma) { m_HistBinsARMGamma = HistBinsARMGamma; } 
  int GetHistBinsARMElectron() const { return m_HistBinsARMElectron; }
  void SetHistBinsARMElectron(int HistBinsARMElectron) { m_HistBinsARMElectron = HistBinsARMElectron; } 
  int GetHistBinsSpectrum() const { return m_HistBinsSpectrum; }
  void SetHistBinsSpetrum(int HistBinsSpectrum) { m_HistBinsSpectrum = HistBinsSpectrum; } 

  // Log binning
  bool GetOptimizeBinningARMGamma() const { return m_OptimizeBinningARMGamma; }
  void SetOptimizeBinningARMGamma(bool OptimizeBinningARMGamma) { m_OptimizeBinningARMGamma = OptimizeBinningARMGamma; }

  unsigned int GetNumberOfFitsARMGamma() const { return m_NumberOfFitsARMGamma; }
  void SetNumberOfFitsARMGamma(unsigned int NumberOfFitsARMGamma) { m_NumberOfFitsARMGamma = NumberOfFitsARMGamma; }

  unsigned int GetFitFunctionIDARMGamma() const { return m_FitFunctionIDARMGamma; }
  void SetFitFunctionIDARMGamma(unsigned int FitFunctionIDARMGamma) { m_FitFunctionIDARMGamma = FitFunctionIDARMGamma; }

  bool GetUseUnbinnedFittingARMGamma() const { return m_UseUnbinnedFittingARMGamma; }
  void SetUseUnbinnedFittingARMGamma(bool UseUnbinnedFittingARMGamma) { m_UseUnbinnedFittingARMGamma = UseUnbinnedFittingARMGamma; }


  // protected members:
 protected:
  //! Read all data from an XML tree
  virtual bool ReadXml(MXmlNode* Node);
  //! Write all data to an XML tree
  virtual bool WriteXml(MXmlNode* Node);


  // private members:
 private:
  //! The coordiante system
  MCoordinateSystem m_CoordinateSystem;

  // Test positions for ARM's etc.
  bool m_TPUse;
  double m_TPTheta;
  double m_TPPhi;
  double m_TPGalLongitude;
  double m_TPGalLatitude;
  double m_TPX;
  double m_TPY;
  double m_TPZ;
  double m_TPDistanceTrans;
  double m_TPDistanceLong;
  
  // Log binning
  bool m_LogBinningSpectrum;

  // Bin sizes for certain histograms:
  int m_HistBinsARMGamma;
  int m_HistBinsARMElectron;
  int m_HistBinsSpectrum;

  //
  bool m_OptimizeBinningARMGamma;
  unsigned int m_NumberOfFitsARMGamma;
  unsigned int m_FitFunctionIDARMGamma;
  bool m_UseUnbinnedFittingARMGamma;



#ifdef ___CLING___
 public:
  ClassDef(MSettingsResolutions, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
