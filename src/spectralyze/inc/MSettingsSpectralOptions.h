/*
 * MSettingsSpectralOptions.h
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
    //  int GetSpectralHistBinsSpectralyzer() const { return m_SpectralHistBinsSpectralyzer; }

  void SetSpectralSignaltoNoiseRatio(int SpectralSignaltoNoiseRatio) { m_SpectralSignaltoNoiseRatio = SpectralSignaltoNoiseRatio; }
  int GetSpectralSignaltoNoiseRatio() const { return m_SpectralSignaltoNoiseRatio; }

  void SetSpectralPoissonLimit(int SpectralPoissonLimit) { m_SpectralPoissonLimit = SpectralPoissonLimit; }
  int GetSpectralPoissonLimit() const { return m_SpectralPoissonLimit; }
  
//  void SetSpectralDeconvolutionLimit(double SpectralDeconvolutionLimit) { m_SpectralDeconvolutionLimit = SpectralDeconvolutionLimit; }
//  double GetSpectralDeconvolutionLimit() const { return m_SpectralDeconvolutionLimit; }
  
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
//  double m_SpectralDeconvolutionLimit;
  MString m_SpectralIsotopeFileName;
  double m_SpectralEnergyRange;

 
  
#ifdef ___CLING___
 public:
   ClassDef(MSettingsSpectralOptions, 0) // no description
#endif

};


#endif


////////////////////////////////////////////////////////////////////////////////

