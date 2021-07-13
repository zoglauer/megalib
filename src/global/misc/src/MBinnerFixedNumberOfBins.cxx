/*
 * MBinnerFixedNumberOfBins.cxx
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
#include "MBinnerFixedNumberOfBins.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MBinnerFixedNumberOfBins)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MBinnerFixedNumberOfBins::MBinnerFixedNumberOfBins() : MBinner(), m_NumberOfBins(1), m_AlignBins(false)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MBinnerFixedNumberOfBins::~MBinnerFixedNumberOfBins()
{
}


////////////////////////////////////////////////////////////////////////////////


//! The actual histogramming process - default just makes one bin
void MBinnerFixedNumberOfBins::Histogram()
{
  if (m_IsModified == false) return;
  
  m_BinEdges.clear();
  m_BinnedData.clear();
  
  double NumberOfBins = m_NumberOfBins;
  
  double Min = m_Minimum;
  double Max = m_Maximum;
  if (m_Adapt == true) {
    m_Values.sort(SortBinnedData);
    double Front = m_Values.front().m_AxisValue;
    double Back = m_Values.back().m_AxisValue;
    if (Front < Back) {
      if (Front > Min && Front < Max) Min = Front;
      if (Back < Max && Back > Min) Max = Back;
    }
  }

  if (m_AlignBins == true) {
    Min = int(Min);
    Max = int(Max) + 1;
  }
  
  double BinSize = (Max - Min)/NumberOfBins;
  if (m_AlignBins == true) {
    BinSize = int(BinSize);
    if (BinSize == 0) BinSize = 1;
    NumberOfBins = (int) ((Max - Min)/BinSize);
  }
  //cout<<"Number of bins: "<<NumberOfBins<<endl;
  
  for (unsigned int i = 0; i < NumberOfBins; ++i) {
    m_BinEdges.push_back(Min + i*BinSize);
  }
  m_BinEdges.push_back(Max);
  
  m_BinnedData.resize(m_BinEdges.size() - 1);
  
  for (list<MBinnedData>::iterator I = m_Values.begin(); I != m_Values.end(); ++I) {
    for (unsigned int e = 0; e < m_BinEdges.size(); ++e) {
      if (m_BinEdges[e] > (*I).m_AxisValue) {
        if (e > 0) {
          m_BinnedData[e-1] += (*I).m_DataValue;
        }
        break;
      }
    }
  }
  
  m_IsModified = false;
}


// MBinnerFixedNumberOfBins.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
