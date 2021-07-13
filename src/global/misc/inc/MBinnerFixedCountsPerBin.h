/*
 * MBinnerFixedCountsPerBin.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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
