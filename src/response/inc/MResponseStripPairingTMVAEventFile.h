/*
 * MResponseStripPairingTMVAEventFile.h
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


#ifndef __MResponseStripPairingTMVAEventFile__
#define __MResponseStripPairingTMVAEventFile__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include "TTree.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MERStripPairingDataSet.h"
#include "MRESE.h"
#include "MRERawEvent.h"
#include "MRETrack.h"
#include "MResponseMultipleComptonEventFile.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseStripPairingTMVAEventFile : public MResponseMultipleComptonEventFile
{
  // public interface:
 public:
  //! Default constructor
  MResponseStripPairingTMVAEventFile();
  //! Default destructor
  virtual ~MResponseStripPairingTMVAEventFile();
  
  //! Return a brief description of this response class
  static MString Description();
  //! Return information on the parsable options for this response class
  static MString Options();
  //! Parse the options
  virtual bool ParseOptions(const MString& Options);
  
  //! Initialize the response matrices and their generation
  virtual bool Initialize();
  
  //! Analyze th events (all if in file mode, one if in event-by-event mode)
  virtual bool Analyze();
  
  //! Finalize the response generation (i.e. save the data a final time )
  virtual bool Finalize();
  
  // protected methods:
 protected:
   
  //! Save the response matrices
  virtual bool Save();

  //! Store one element
  bool Store(unsigned int SequenceLength, bool IsGood);
  
  //! Shuffle the strips
  void ShuffleStrips(vector<unsigned int>& StripIDs, vector<double>& Energies);
  

  // private methods:
 private:



  // protected members:
 protected:
  //! Use path to first IA
  bool m_UsePathToFirstIA;
   
  // The data sets, one per number of x and y hit strips
  vector<vector<MERStripPairingDataSet*>> m_DataSets;
  
  //! The data storage for the events, one per number of x and y hit strips
  vector<vector<TTree*>> m_Trees;
   
  //! The maximum allowed number of x-hits
  unsigned int m_MaxNHitsX;
  //! The maximum allowed number of y-hits
  unsigned int m_MaxNHitsY;
  
  
  
#ifdef ___CLING___
 public:
  ClassDef(MResponseStripPairingTMVAEventFile, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
