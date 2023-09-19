/*
 * MFileResponse.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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


class MFileResponse : public MParser
{
  // public interface:
 public:
  explicit MFileResponse();
  virtual ~MFileResponse();

  MResponseMatrix* Read(MString FileName, bool MultiThreaded = false);

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
  MString GetBeamType() const { return m_BeamType; }
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
  //! The beam type
  MString m_BeamType;
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
