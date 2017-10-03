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

//! FISBEL: Fixed Integral ... Equi-longitude..?
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
  //! Theta and phi are in radians!
  //! TODO: Should be degree...
  unsigned int FindBin(double Theta, double Phi) const;
  
  //! Get number of bins
  unsigned int GetNBins() const { return m_NumberOfBins; }
  
  //! Get the longitude bins
  vector<unsigned int> GetLongitudeBins() const { return m_LongitudeBins; }
  
  //! Get the latitude bin edges (in degree)
  vector<double> GetLatitudeBinEdges() const { return m_LatitudeBinEdges; } 
  
  //! Get the bin center (returns: theta, phi)
  //! Can throw: MExceptionIndexOutOfBounds
  vector<double> GetBinCenters(unsigned int Bin) const;
  
  //! View the binning
  //! If the given vector has the correct dimension, it's data will be shown
  void View(vector<double> = {}) const;
  
  //! Return axis bins edges for external drawing (1st array: longitude/phi, 2nd array: latitude/theta)
  vector<vector<double>> GetDrawingAxisBinEdges() const;
  
  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  //! The number of bins
  unsigned int m_NumberOfBins;

  //! The longitude bins per latitude slice
  vector<unsigned int> m_LongitudeBins;
  //! The latitude bin edges
  vector<double> m_LatitudeBinEdges;


  // private members:
 private:
  //! Little accelerator vector to speed up the bin search:
  //! Number of bins in the longitude bins *before* this latitude bin
  //! top bin: 0 (no bins above that)
  //! top+1 bin: number of bins in top bin, etc.
  vector<double> m_NumberOfBinsBeforeLatitudeBin;

  
#ifdef ___CINT___
 public:
  ClassDef(MBinnerFISBEL, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////

