/*
 * MBinnerFixedNumberOfBins.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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
  unsigned int m_NumberOfBins;
   

#ifdef ___CINT___
 public:
  ClassDef(MBinnerFixedNumberOfBins, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
