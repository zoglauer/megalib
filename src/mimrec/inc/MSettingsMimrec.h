/*
 * MSettingsMimrec.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSettingsMimrec__
#define __MSettingsMimrec__


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


class MSettingsMimrec : public MSettings, public MSettingsEventSelections, public MSettingsImaging, public MSettingsSpectralOptions
{
  // public Session:
 public:
  MSettingsMimrec(bool AutoLoad = true);
  virtual ~MSettingsMimrec();

  virtual bool SetCurrentFileName(MString File) { m_BackprojectionModified = true; m_EventSelectionModified = true; m_LikelihoodModified = true; return MSettingsBasicFiles::SetCurrentFileName(File); }

 
  // Menu Response
  //////////////////

  // Polarization
  MString GetPolarizationBackgroundFileName() const { return m_PolarizationBackgroundFileName; }
  void SetPolarizationBackgroundFileName(MString PolarizationBackgroundFileName) { m_PolarizationBackgroundFileName = PolarizationBackgroundFileName; m_LikelihoodModified = true; }

  double GetPolarizationArmCut() const { return m_PolarizationArmCut; }
  void SetPolarizationArmCut(double PolarizationArmCut) { m_PolarizationArmCut = PolarizationArmCut; }

  // Test positions for ARM's etc.
  
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
  int GetHistBinsPolarization() const { return m_HistBinsPolarization; }
  void SetHistBinsPolarization(int HistBinsPolarization) { m_HistBinsPolarization = HistBinsPolarization; } 



  // protected members:
 protected:
  //! Read all data from an XML tree
  virtual bool ReadXml(MXmlNode* Node);
  //! Write all data to an XML tree
  virtual bool WriteXml(MXmlNode* Node);


  // private members:
 private:


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
  int m_HistBinsPolarization;

  // Polarization:
  MString m_PolarizationBackgroundFileName;
  double m_PolarizationArmCut;


#ifdef ___CLING___
 public:
  ClassDef(MSettingsMimrec, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
