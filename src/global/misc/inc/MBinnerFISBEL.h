/*
 * MBinnerFISBEL.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBinnerFISBEL__
#define __MBinnerFISBEL__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! This is just the binner, it does not store any data itself
class MBinnerFISBEL
{
  // public interface:
 public:
  //! Default constructor - if NBins > 0 also creates the bins
  MBinnerFISBEL(unsigned int NBins = 0);
  //! Default destuctor 
  virtual ~MBinnerFISBEL();
  
  //! Create the binning
  void Create(unsigned int NBins);
  
  //! Set the binning
  void Set(vector<unsigned int>& LongitudeBins, vector<double>& LatitudeBinEdges, unsigned int NumberOfBins);
  
  //! Check if we have equal bins
  bool operator ==(const MBinnerFISBEL& Binner) const;
  //! ... or not
  bool operator !=(const MBinnerFISBEL& Binner) const { return !operator==(Binner); }
  
  //! Find a bin
  //! Theta (= latitude) and phi (= longitude) are in (mathematical) spherical coordinates
  //! The bins are arranged along the iso-latitude lines starting at the north pole (theta = latitude = 0)
  //! Since this is MEGAlib, theta and phi are in degrees!
  unsigned int FindBin(double Theta, double Phi) const;
  
  //! Get number of bins
  unsigned int GetNBins() const { return m_NumberOfBins; }
  
  //! Get the longitude bins
  vector<unsigned int> GetLongitudeBins() const { return m_LongitudeBins; }
  
  //! Get the latitude bin edges (in degree)
  vector<double> GetLatitudeBinEdges() const { return m_LatitudeBinEdges; } 
  
  //! View the binning
  //! If the given vector has the correct dimension, it's data will be shown
  void View(vector<double> = {}) const;
  
  
  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  //! The number of bins
  unsigned int m_NumberOfBins;

  //! The longitude bins
  vector<unsigned int> m_LongitudeBins;
  //! The latitude bin edges
  vector<double> m_LatitudeBinEdges;


  // private members:
 private:
  //! Little accelerator vector to speed up the bin search:
  //! Number of bins in the longitudebins *before* this latitude bin
  vector<double> m_NumberOfBinsBeforeLatitudeBin;

  
#ifdef ___CINT___
 public:
  ClassDef(MBinnerFISBEL, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////

