/*
 * MFileReadOuts.h
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


#ifndef __MFileReadOuts__
#define __MFileReadOuts__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFile.h"
#include "MFileEvents.h"
#include "MTime.h"
#include "MReadOut.h"
#include "MReadOutData.h"
#include "MReadOutElement.h"
#include "MReadOutFileFormat.h"
#include "MReadOutSequence.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

//! Read the measured data from a file
class MFileReadOuts : public MFileEvents
{
  // public interface:
 public:
  //! Default constructor reading uncalibrated detector data
  MFileReadOuts();
  //! Default destructor
  virtual ~MFileReadOuts();

  //! Open the read-out file
  virtual bool Open(MString FileName, unsigned int Way = MFile::c_Read);

  //! Return the next event
  //! If SelectedDetectorID is non-negative then restrict yourself to SelectedDetectorID
  //! SelectedDetectorSide:
  //!   < 0: all
  //!     0: negative side
  //!     1: positive side
  //!   >=2: all
  bool ReadNext(MReadOutSequence& Sequence, int SelectedDetectorID = -1, int SelectedDetectorSide = -1);

  //! Return the read-out units this file declares
  const MReadOutFileFormat& GetReadOutFileFormat() const { return m_ReadOutFileFormat; }

  
  // protected methods:
 protected:
  //! Parse the special information at the end of file
  virtual bool ParseFooter(const MString& Line);

  // private methods:
 private:
  //! Parse a UF line containing the read-out unit data and add it to the prototypes
  //! Return false on error
  bool AddReadOutUnitPrototype(const MString& Line);
  //! Build the read-out data described by a UF read-out data format such as "adc" or "adcwithtiming".
  //! Returns nullptr on error
  MReadOutData* CreateReadOutData(const MString& Format);


  // protected members:
 protected:


  // private members:
 private:
  //! The number of events in the file
  unsigned int m_NEventsInFile;
  //! The number of good events in file
  unsigned int m_NGoodEventsInFile;
  //! Clock time belonging to the start of the observation time
  unsigned long m_StartClock; 
  //! Clock time belonging to the end of the observation time
  unsigned long m_EndClock;
  //! True if the end clock tag has been read
  bool m_HasEndClock;

  //! The read-out file format including the units definitions
  MReadOutFileFormat m_ReadOutFileFormat;
  //! The prototype read-out units at the same index as in m_ReadOutFileFormat
  vector<MReadOut> m_ReadOutPrototypes;
  
  

#ifdef ___CLING___
 public:
  ClassDef(MFileReadOuts, 0) // The file reader for read outs
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
