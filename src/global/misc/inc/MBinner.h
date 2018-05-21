/*
 * MBinner.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBinner__
#define __MBinner__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <list>
using namespace std;

// ROOT libs:
#include "TH1.h"

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A binned data tuple
class MBinnedData
{
  public:
  //! Default constructor
  MBinnedData() : m_AxisValue(0), m_DataValue(0) {}
  //! Standard constructor with one axis and data value
  MBinnedData(double AxisValue, double DataValue = 1) : m_AxisValue(AxisValue), m_DataValue(DataValue) {}
  
  //! operator >
  bool operator>(const MBinnedData& D) { return m_AxisValue > D.m_AxisValue; } 
  //! operator <
  bool operator<(const MBinnedData& D) { return m_AxisValue < D.m_AxisValue; } 
  
  
  //! The axis value
  double m_AxisValue;
  //! the value
  double m_DataValue;
};

//! Sort binned data by its axis value
bool SortBinnedData(const MBinnedData& D1, const MBinnedData& D2);

//! A base class for all binners
class MBinner
{
  // public interface:
 public:
  //! Default constructor
  MBinner();
  //! Default destuctor 
  virtual ~MBinner();

  //! Clear all data sets
  void Clear();
  
  //! Set the minimum and maximum. If Adapt is true, then use for
  //! min=max(m_Minimum, real minimum)
  //! max=min(m_Maximum, real maximum)
  //! Adapt will not take place if there is no data between minimum and maximum
  //! If Maximum <= Minimum no changes will take place to the data
  void SetMinMax(double Minimum, double Maximum, bool Adapt = false); 

  //! Add another element to the array
  void Add(double AxisValue, double DataValue = 1) { m_Values.push_back(MBinnedData(AxisValue, DataValue)); m_IsModified = true; }
  
  //! Return as a histogram (cts)
  TH1D* GetHistogram(const MString& Title, const MString& xTitle, const MString& yTitle);
  
  //! Return as a normalized histogram (e.g. cts/bin width)
  TH1D* GetNormalizedHistogram(const MString& Title, const MString& xTitle, const MString& yTitle);
  
  //! Draw normalized histogram (e.g. cts/bin width)
  void DrawNormalizedHistogram(const MString& Title, const MString& xTitle, const MString& yTitle);
  
  //! Return the binned data as a vector
  vector<double> GetBinnedData();
  //! Return the bin edges as a vector
  vector<double> GetBinEdges();
  
  // protected methods:
 protected:
  //! The actual histogramming process - default just makes one bin
  virtual void Histogram(); 

  // private methods:
 private:



  // protected members:
 protected:
  //! Flag indicating that the content has been modified since the data has been histogrammed last time 
  bool m_IsModified;
  
  //! The list of values
  list<MBinnedData> m_Values;
   
  //! The minimum
  double m_Minimum;
  //! The maximum
  double m_Maximum;
  //! If true adapt the minimum and maximum to the real minimum and maximum if their are within [m_Minimum, m_Maximum] 
  bool m_Adapt;

  //! The bin edges
  vector<double> m_BinEdges;
  //! The bin content
  vector<double> m_BinnedData;


  // private members:
 private:
  

#ifdef ___CLING___
 public:
  ClassDef(MBinner, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
