/*
 * MSettingsSpectralyzer.h
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

 
  
#ifdef ___CLING___
 public:
  ClassDef(MSettingsSpectralyzer, 0) // no description
#endif

};


#endif


////////////////////////////////////////////////////////////////////////////////
