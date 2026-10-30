/*
 * MBinnerFixedNumberOfBins.h
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


#ifndef __MBinnerFixedNumberOfBins__
#define __MBinnerFixedNumberOfBins__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MBinner.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A base class for all binners
class MBinnerFixedNumberOfBins : public MBinner
{
  // public interface:
 public:
  //! Default constructor
  MBinnerFixedNumberOfBins();
  //! Default destuctor 
  virtual ~MBinnerFixedNumberOfBins();

  //! Set the minimum and maximum
  void SetNumberOfBins(unsigned int NumberOfBins) { m_NumberOfBins = NumberOfBins; if (m_NumberOfBins < 1) m_NumberOfBins = 1; }
  
  //! Set if the bins should be aligned
  void AlignBins(bool AlignBins) { m_AlignBins = AlignBins; }
  
  // protected methods:
 protected:

  // private methods:
 private:
  //! The actual histogramming process
  virtual void Histogram(); 



  // protected members:
 protected:


  // private members:
 private:
  //! The number of bins
  unsigned int m_NumberOfBins;
  //! Should the bins be aligned at integer number
  bool m_AlignBins;

#ifdef ___CLING___
 public:
  ClassDef(MBinnerFixedNumberOfBins, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
