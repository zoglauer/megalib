/*
 * MBinnerFixedCountsPerBin.h
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


#ifndef __MBinnerFixedCountsPerBin__
#define __MBinnerFixedCountsPerBin__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MBinner.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A base class for all binners
class MBinnerFixedCountsPerBin : public MBinner
{
  // public interface:
 public:
  //! Default constructor
  MBinnerFixedCountsPerBin();
  //! Default destuctor 
  virtual ~MBinnerFixedCountsPerBin();

  //! Set the counts per bin - last bin might have up to double the counts
  void SetCountsPerBin(double CountsPerBin) { m_CountsPerBin = CountsPerBin; }
  
  // protected methods:
 protected:
  //! The actual histogramming process
  virtual void Histogram(); 

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The counts per bin
  double m_CountsPerBin;


#ifdef ___CLING___
 public:
  ClassDef(MBinnerFixedCountsPerBin, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
