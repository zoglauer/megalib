/*
 * MSettingsMString.h
 *
 * Copyright (C) by Michelle Galloway & Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSettingsSpectralyzer__
#define __MSettingsSpectralyzer__


////////////////////////////////////////////////////////////////////////////////


// Root libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSettings.h"
#include "MSettingsSpectralOptions.h"


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSettingsSpectralyzer : public MSettings, public MSettingsSpectralOptions
{
  // public Session:
 public:
  MSettingsSpectralyzer();
  virtual ~MSettingsSpectralyzer();


  // protected members:
 protected:
  //! Read all data from an XML tree
  virtual bool ReadXml(MXmlNode* Node);
  //! Write all data to an XML tree
  virtual bool WriteXml(MXmlNode* Node);


  // private members:
 private:

    //  int m_SpectralHistBinsSpectralyzer;
  //int m_SpectralSignaltoNoiseRatio;
  //int m_SpectralPoissonLimit;
//  double m_SpectralDeconvolutionLimit;
  MString m_SpectralIsotopeFileName;
  //double m_SpectralEnergyRange;

 
  
#ifdef ___CINT___
 public:
  ClassDef(MSettingsSpectralyzer, 0) // no description
#endif

};


#endif


////////////////////////////////////////////////////////////////////////////////
