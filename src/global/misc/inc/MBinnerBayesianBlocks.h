/*
 * MBinnerBayesianBlocks.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBinnerBayesianBlocks__
#define __MBinnerBayesianBlocks__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <cmath>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MBinner.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A binning class using Bayesian blocks for binning
//! Reference for bayesian blocks:
//! Jeffrey D. Scargle et al. "Studies in Astronomical Time Series Analysis. VI. Bayesian Block Representations", ApJ 764, 2013 
class MBinnerBayesianBlocks : public MBinner
{
  // public interface:
 public:
  //! Default constructor
  MBinnerBayesianBlocks();
  //! Default destuctor 
  virtual ~MBinnerBayesianBlocks();
  
  //! Set the minimum bin width
  void SetMinimumBinWidth(double MinimumBinWidth) { m_MinimumBinWidth = MinimumBinWidth; }
  
  //! Set the minimum counts per bin
  void SetMinimumCountsPerBin(double MinimumCountsPerBin) { m_MinimumCountsPerBin = MinimumCountsPerBin; }
  
  //! Set the minimum bin width
  void SetPrior(double Prior) { m_Prior = Prior; }
  
  // protected methods:
 protected:
  //! The actual histogramming process
  virtual void Histogram(); 

  //! Fast logarithm approximation with accuracy better than 1.5% - but not useful here...
  inline float FastLog(float a) {
    int e = 0;
    float m = frexp(a, &e);
    return 0.69314718f *  (e - 1.78070 + (2.61329 - 0.833304*m)*m);
  }

  
  // private methods:
 private:
  //! Some quick print method for a vector of floats
  void Print(vector<float>& Array) {
    for (unsigned int i = 0; i < Array.size(); ++i) {
      cout<<i<<":"<<Array[i]<<"  ";
    }
    cout<<endl;
  }
  //! Some quick print method for a vector of doubles
  void Print(vector<double>& Array) {
    for (unsigned int i = 0; i < Array.size(); ++i) {
      cout<<i<<":"<<Array[i]<<"  ";
    }
    cout<<endl;
  }

  //! Some quick print method for a vector of unsigned ints
  void Print(vector<unsigned int>& Array) {
    for (unsigned int i = 0; i < Array.size(); ++i) {
      cout<<i<<":"<<Array[i]<<"  ";
    }
    cout<<endl; 
  }

  // protected members:
 protected:


  // private members:
 private:
  //! The minimum bin width
  double m_MinimumBinWidth;
  //! The minimum counts per bin
  double m_MinimumCountsPerBin;
  //! The prior
  double m_Prior;
  //! True if we use binning
  bool m_UseBinning;

  
#ifdef ___CINT___
 public:
  ClassDef(MBinnerBayesianBlocks, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
