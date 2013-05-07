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
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MParser.h"
#include "MTokenizer.h"

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

  MResponseMatrix* Read(MString FileName);

  //! Open the file name and read the header
  virtual bool Open(MString FileName, unsigned int Way = 1);
  //! Return the name - available after a call to Open
  MString GetName() const { return m_Name; }
  //! Are the values centered
  bool AreValuesCentered() const { return m_ValuesCentered; }
  //! Are the values centered
  unsigned long GetHash() const { return m_Hash; }

  // protected methods:
 protected:

  // private methods:
 private:

  // protected members:
 protected:
  //! Name of the response matrix
  MString m_Name;
  //! Are the values stored cenetered
  bool m_ValuesCentered;
  //! The hash value
  unsigned long m_Hash;

#ifdef ___CINT___
 public:
  ClassDef(MFileResponse, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
