/*
 * MERStripPairing.h
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


#ifndef __MERStripPairing__
#define __MERStripPairing__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"
#include "MERConstruction.h"
#include "MRawEventIncarnationList.h"
#include "MRERawEvent.h"
#include "MREStripHit.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MERStripPairing : public MERConstruction
{
  // public interface:
 public:
  //! Default constructor
  MERStripPairing();
  //! Default destructor
  virtual ~MERStripPairing();

  //! Perform strip pairing - we assume to have just one event in the list
  bool Analyze(MRawEventIncarnationList* List);

  //! Perform strip pairing
  bool Analyze(MRERawEvent* RE);

  //! Dump some elemenatry information about the algorithm settings
  MString ToString(bool CoreOnly) const;

  //! ID representing no coincidence search
  static const int c_None = 0;
  //! ID representing a coincidence window
  static const int c_ChiSquare = 1;

  // protected methods:
 protected:
  // Fins a new set of combinations giving the existin gone 
   vector<vector<vector<unsigned int>>> FindNewCombinations(vector<vector<vector<unsigned int>>> OldOnes, vector<MREStripHit*> StripHits);
   
   
  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The used algorithm
  int m_Algorithm;

  //! The coincidence window
  MTime m_Window;

  //! The number of found coincidences
  int m_NFoundStripPairings;
  

#ifdef ___CLING___
 public:
  ClassDef(MERStripPairing, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
