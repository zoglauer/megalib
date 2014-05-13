/*
 * MBinnerFixedCountsPerBin.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Include the header:
#include "MBinnerFixedCountsPerBin.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MBinnerFixedCountsPerBin)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MBinnerFixedCountsPerBin::MBinnerFixedCountsPerBin() : m_CountsPerBin(1)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MBinnerFixedCountsPerBin::~MBinnerFixedCountsPerBin()
{
}


////////////////////////////////////////////////////////////////////////////////


//! The actual histogramming process - default just makes one bin
void MBinnerFixedCountsPerBin::Histogram()
{
  if (m_IsModified == false) return;
  
  m_BinEdges.clear();
  m_BinnedData.clear();
  
  m_Values.sort(SortBinnedData);

  double Min = m_Minimum;
  double Max = m_Maximum;
  if (m_Adapt == true) {
    double Front = m_Values.front().m_AxisValue;
    double Back = m_Values.back().m_AxisValue;
    if (Front < Back) {
      if (Front > Min && Front < Max) Min = Front;
      if (Back < Max && Back > Min) Max = Back;
    }
  }

  m_BinEdges.push_back(Min);
  m_BinnedData.push_back(0);
  
  for (list<MBinnedData>::iterator I = m_Values.begin(); I != m_Values.end(); ++I) {
    if ((*I).m_AxisValue < Min || (*I).m_AxisValue > Max) continue;
    m_BinnedData.back() += (*I).m_DataValue;
    if (m_BinnedData.back() >= m_CountsPerBin) {
      list<MBinnedData>::iterator Next = I;
      Next++;
      if (Next != m_Values.end()) { // Not the last element
        m_BinEdges.push_back(0.5*((*I).m_AxisValue + (*Next).m_AxisValue));    
        m_BinnedData.push_back(0);
      } else {
        m_BinEdges.push_back((*I).m_AxisValue);
        break;
      }
    }
  }
  m_BinEdges.push_back(Max);
      
  m_IsModified = false;
}


// MBinnerFixedCountsPerBin.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
