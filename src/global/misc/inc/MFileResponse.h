/*
 * MFileResponse.h
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


#ifndef __MFileResponse__
#define __MFileResponse__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MParser.h"
#include "MTokenizer.h"
#include "MString.h"

// Standard libs:
#include <vector>
#include <sstream>
using namespace std;

// Forward declarations:
class MResponseMatrix;

////////////////////////////////////////////////////////////////////////////////


//! Class to read response files
class MFileResponse : public MParser
{
  // public interface:
 public:
  //! Default constructor
  explicit MFileResponse();
  //! Default destructor
  virtual ~MFileResponse();

  //! Read a response matrix
  MResponseMatrix* Read(MString FileName);
  //! We cannot use the base class version here
  bool Read(MBinaryStore& Store, unsigned int CharactersToRead) { return false; }

  //! Open the file name and read the header
  virtual bool Open(MString FileName, unsigned int Way = 1);
  //! Return the name - available after a call to Open
  MString GetName() const { return m_Name; }
  //! Return the number of simulated events
  long GetSimulatedEvents() const { return m_NumberOfSimulatedEvents; }
  //! Return the far field start area
  double GetFarFieldStartArea() const { return m_FarFieldStartArea; }
  //! Return the spectral type
  MString GetSpectralType() const { return m_SpectralType; }
  //! Return the spectral parameters
  vector<double> GetSpectralParameters() const { return m_SpectralParameters; }
  //! Are the values centered
  bool AreValuesCentered() const { return m_ValuesCentered; }
  //! Are the values centered
  unsigned long GetHash() const { return m_Hash; }

  // we want also the Write's from MFile
  using MFile::Write;
  //! Write some text (and clear the stream)
  virtual void Write(ostringstream& S);
  //! Write some text plus space afterwards
  virtual void Write(const double d);

  // protected methods:
 protected:

  // private methods:
 private:

  // protected members:
 protected:
  //! Name of the response matrix
  MString m_Name;
  //! The number of simulated events
  long m_NumberOfSimulatedEvents;
  //! The area from which the photons are started in far field
  double m_FarFieldStartArea;
  //! The spectral type (Linear, Mono, Powerlaw)
  MString m_SpectralType;
  //! The spectral parameters (depend on type)
  vector<double> m_SpectralParameters;
  //! Are the values stored centered
  bool m_ValuesCentered;
  //! The hash value
  unsigned long m_Hash;

#ifdef ___CLING___
 public:
  ClassDef(MFileResponse, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
