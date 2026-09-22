/*
 * MGTI.h
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


#ifndef __MGTI__
#define __MGTI__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;
// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Representation of good and bad time intervals
class MGTI
{
  // public interface:
 public:
  //! Default constructor
  MGTI();
  //! Default destuctor 
  virtual ~MGTI();

  //! Reset
  //! Create an open GTI
  void Reset(bool AllOpen = false);

  //! Add this GTI file's intervals
  void Add(const MGTI& GTI);
  
  //! Check if the time is withing a good interval 
  //! In case of failure, we create a single time interval from 0 to 2,000,000,000
  bool IsGood(const MTime& Time) const;

  //! Load the good time interval data
  bool Load(const MString& FileName);
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Start of a good time interval
  vector<MTime> m_GoodStart;
  //! End of a good time interval
  vector<MTime> m_GoodStop;

  //! Start of a bad time interval
  vector<MTime> m_BadStart;
  //! Start of a bad time interval
  vector<MTime> m_BadStop;


#ifdef ___CLING___
 public:
  ClassDef(MGTI, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
