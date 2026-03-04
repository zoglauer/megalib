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
#include "MSettingsResolutions.h"
#include "MPointSource.h"
#include "MEventSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSettingsMimrec : public MSettings, public MSettingsEventSelections, public MSettingsImaging, public MSettingsResolutions, public MSettingsSpectralOptions
{
  // public Session:
 public:
  MSettingsMimrec(bool AutoLoad = true);
  virtual ~MSettingsMimrec();

  virtual bool SetCurrentFileName(MString File) { m_BackprojectionModified = true; m_EventSelectionModified = true; m_LikelihoodModified = true; return MSettingsBasicFiles::SetCurrentFileName(File); }

  //! Two sub-classes have coordiante systems - make sure to call the right one here:
  virtual MCoordinateSystem GetCoordinateSystem() const { return MSettingsImaging::GetCoordinateSystem(); }
  //! Set the coorfdianet system in both sub classes
  virtual void SetCoordinateSystem(MCoordinateSystem CoordinateSystem) { MSettingsImaging::SetCoordinateSystem(CoordinateSystem); MSettingsResolutions::SetCoordinateSystem(CoordinateSystem);  }

  // Polarization
  MString GetPolarizationBackgroundFileName() const { return m_PolarizationBackgroundFileName; }
  void SetPolarizationBackgroundFileName(MString PolarizationBackgroundFileName) { m_PolarizationBackgroundFileName = PolarizationBackgroundFileName; m_LikelihoodModified = true; }

  double GetPolarizationArmCut() const { return m_PolarizationArmCut; }
  void SetPolarizationArmCut(double PolarizationArmCut) { m_PolarizationArmCut = PolarizationArmCut; }

  // Bin sizes for certain histograms:
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
  // Bin sizes for certain histograms:
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
