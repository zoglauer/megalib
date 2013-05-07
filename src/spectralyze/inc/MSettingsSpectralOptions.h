/*
 * MSettingsSpectralOptions.h
 *
 * Copyright (C) by Michelle Galloway & Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSettingsSpectralOptions__
#define __MSettingsSpectralOptions__


////////////////////////////////////////////////////////////////////////////////


// Root libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSettingsInterface.h"

// Forward declarations:

////////////////////////////////////////////////////////////////////////////////


class MSettingsSpectralOptions : public MSettingsInterface
{
  // public Session:
 public:
  MSettingsSpectralOptions();
  virtual ~MSettingsSpectralOptions();

		//  void SetSpectralHistBinsSpectralyzer(int SpectralHistBinsSpectralyzer) { m_SpectralHistBinsSpectralyzer = SpectralHistBinsSpectralyzer; }
		//	int GetSpectralHistBinsSpectralyzer() const { return m_SpectralHistBinsSpectralyzer; }

  void SetSpectralSignaltoNoiseRatio(int SpectralSignaltoNoiseRatio) { m_SpectralSignaltoNoiseRatio = SpectralSignaltoNoiseRatio; }
  int GetSpectralSignaltoNoiseRatio() const { return m_SpectralSignaltoNoiseRatio; }

  void SetSpectralPoissonLimit(int SpectralPoissonLimit) { m_SpectralPoissonLimit = SpectralPoissonLimit; }
	int GetSpectralPoissonLimit() const { return m_SpectralPoissonLimit; }
	
//  void SetSpectralDeconvolutionLimit(double SpectralDeconvolutionLimit) { m_SpectralDeconvolutionLimit = SpectralDeconvolutionLimit; }
//	double GetSpectralDeconvolutionLimit() const { return m_SpectralDeconvolutionLimit; }
  
  void SetSpectralIsotopeFileName(MString SpectralIsotopeFileName) { m_SpectralIsotopeFileName = SpectralIsotopeFileName; }
	MString GetSpectralIsotopeFileName() const { return m_SpectralIsotopeFileName; }
  
  void SetSpectralEnergyRange(double SpectralEnergyRange) { m_SpectralEnergyRange = SpectralEnergyRange; }
	double GetSpectralEnergyRange() const { return m_SpectralEnergyRange; }


  // protected members:
 protected:
  //! Read all data from an XML tree
  virtual bool ReadXml(MXmlNode* Node);
  //! Write all data to an XML tree
  virtual bool WriteXml(MXmlNode* Node);


  // private members:
 private:
  
		//  int m_SpectralHistBinsSpectralyzer;
	int m_SpectralSignaltoNoiseRatio;
	int m_SpectralPoissonLimit;
//	double m_SpectralDeconvolutionLimit;
  MString m_SpectralIsotopeFileName;
  double m_SpectralEnergyRange;

 
  
#ifdef ___CINT___
 public:
  ClassDef(MSettingsSpectralyzer, 0) // no description
#endif

};


#endif


////////////////////////////////////////////////////////////////////////////////

