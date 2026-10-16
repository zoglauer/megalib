/*
 * MBinnerBayesianBlocks.cxx
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


// Include the header:
#include "MBinnerBayesianBlocks.h"

// Standard libs:
#include <algorithm>

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MBinnerBayesianBlocks)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MBinnerBayesianBlocks::MBinnerBayesianBlocks() : m_MinimumBinWidth(0.000001), m_MinimumCountsPerBin(0), m_Prior(4), m_UseBinning(true)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MBinnerBayesianBlocks::~MBinnerBayesianBlocks()
{
}


////////////////////////////////////////////////////////////////////////////////


void Print(vector<double>& Array) {
  for (unsigned int i = 0; i < Array.size(); ++i) {
    cout<<Array[i]<<" ";
  }
  cout<<endl;
}


////////////////////////////////////////////////////////////////////////////////


void Print(vector<int>& Array) {
  for (unsigned int i = 0; i < Array.size(); ++i) {
    cout<<Array[i]<<" ";
  }
  cout<<endl;
}


////////////////////////////////////////////////////////////////////////////////


//! The actual histogramming process - default just makes one bin
void MBinnerBayesianBlocks::Histogram()
{
  if (m_IsModified == false) return;
  
  m_BinEdges.clear();
  m_BinnedData.clear();
  
  if (m_Values.empty() == true) {
    // Nothing to do: One empty bin
    m_BinEdges.push_back(m_Minimum);
    m_BinEdges.push_back(m_Maximum);
    m_BinnedData.resize(m_BinEdges.size() - 1, 0);
    m_IsModified = false;
    return;
  }
  
  // Step 1: Sort the Array increasing --- only needed when the edges are set between the values,
  // otherwise we only need the minimum and the maximum
  double Front = m_Values.front().m_AxisValue;
  double Back = m_Values.front().m_AxisValue;
  if (m_UseBinning == false) {
    m_Values.sort(SortBinnedData);
    Front = m_Values.front().m_AxisValue;
    Back = m_Values.back().m_AxisValue;
  } else {
    for (list<MBinnedData>::const_iterator I = m_Values.begin(); I != m_Values.end(); ++I) {
      const double V = (*I).m_AxisValue;
      if (V < Front) Front = V;
      if (V > Back) Back = V;
    }
  }
  
  double Min = m_Minimum;
  double Max = m_Maximum;
  if (m_Adapt == true) {
    if (Front < Back) {
      if (Front > Min && Front < Max) Min = Front;
      if (Back < Max && Back > Min) Max = Back;
    }
  }
    
  unsigned int Size = 0;

  // Step 2: Create cell edges
  vector<double> Edges;
  Edges.push_back(Min);
  if (m_UseBinning == true) {
    while (Edges.back() < Max) {
      Edges.push_back(Edges.back() + m_MinimumBinWidth);
    }
    if (Edges.back() < Max) Edges.push_back(Max);
    Size = Edges.size() - 1;
  } else {
    Size = m_Values.size();
    MBinnedData Last = m_Values.front();
    for (list<MBinnedData>::iterator I = ++(m_Values.begin()); I != m_Values.end(); ++I) {
      Edges.push_back(0.5*(Last.m_AxisValue + (*I).m_AxisValue));
      Last = (*I);
    }
    Edges.push_back(Max);
  }
  //cout<<"Edges:"<<endl;
  //Print(Edges);

  // Step 3: Create Block length:
  vector<float> BlockLength;
  for (unsigned int i = 0; i < Edges.size(); ++i) {
    BlockLength.push_back(Edges.back() - Edges[i]);
  }
  //cout<<"Block length:"<<endl;
  //Print(BlockLength);
  
  // Step 4: Prepare for iterations
  // Find the bin of each value directly instead of looping over all edges: Edges[e] <= Value < Edges[e+1]
  vector<double> CountsPerBinD(Size, 0.0); 
  {
    const double FirstEdge = Edges.front();
    const double LastEdge = Edges.back();
    const size_t NEdges = Edges.size();
    for (list<MBinnedData>::const_iterator I = m_Values.begin(); I != m_Values.end(); ++I) {
      const double Value = (*I).m_AxisValue;
      if (Value < FirstEdge || Value >= LastEdge) continue;
      size_t e = 0;
      if (m_UseBinning == true) {
        // Edges are (almost) equidistant: Guess, then correct
        double Guess = (Value - FirstEdge) / m_MinimumBinWidth;
        e = (Guess > 0) ? (size_t) Guess : 0;
        if (e > NEdges - 2) e = NEdges - 2;
        while (e > 0 && Edges[e] > Value) --e;
        while (e + 2 < NEdges && Edges[e+1] <= Value) ++e;
      } else {
        e = (size_t) (std::upper_bound(Edges.begin(), Edges.end(), Value) - Edges.begin()) - 1;
      }
      CountsPerBinD[e] += (*I).m_DataValue;
    }
  }
  vector<float> CountsPerBin(Size, 0);
  for (unsigned int e = 0; e < Size; ++e) CountsPerBin[e] = (float) CountsPerBinD[e];
  
  //cout<<"Counts:"<<endl;
  //Print(CountsPerBin);
  
  vector<float> Best(Size, 0.0);
  vector<unsigned int> Last(Size, 0);

  // Step 5: Iterate
  // Same arithmetic as before, but without allocating new arrays for every block, and with a table for the logarithms of the counts 
  // (the count of a block is a whole number as long as the data values are whole numbers)
  float TotalCounts = 0;
  for (unsigned int i = 0; i < Size; ++i) TotalCounts += CountsPerBin[i];
  vector<float> LogTable;
  if (TotalCounts >= 0 && TotalCounts < 4000000.0f) {
    LogTable.resize((size_t) TotalCounts + 2);
    for (size_t i = 0; i < LogTable.size(); ++i) LogTable[i] = log((float) i);
  }
  
  // With fixed-width fine bins the width of a block is a multiple of the bin width: Use a table when the float values are identical
  const float BinWidthFloat = (float) m_MinimumBinWidth;
  vector<float> LogWidthTable;
  if (m_UseBinning == true) {
    LogWidthTable.resize(Size + 2);
    for (size_t k = 0; k < LogWidthTable.size(); ++k) LogWidthTable[k] = log((float) k * BinWidthFloat);
  }
  
  // Fast path: Possible when all counts are whole numbers (so that the counts of a block are differences of prefix sums)
  // and the edges are exactly equidistant (so that the widths are exact multiples of the bin width):
  // All arithmetic is exactly the same as in the generic path below
  bool FastPath = (m_UseBinning == true && LogTable.empty() == false);
  if (FastPath == true) {
    if (BinWidthFloat != floor(BinWidthFloat) || (Size + 2) * (double) BinWidthFloat >= 16777216.0) FastPath = false;
  }
  if (FastPath == true) {
    for (unsigned int i = 0; i < Size && FastPath == true; ++i) {
      if (CountsPerBin[i] < 0 || CountsPerBin[i] != floor(CountsPerBin[i])) FastPath = false;
    }
  }
  if (FastPath == true) {
    for (unsigned int k = 0; k <= Size && FastPath == true; ++k) {
      if (BlockLength[k] != (float) (Size - k) * BinWidthFloat) FastPath = false;
    }
  }
  
  if (FastPath == true) {
    vector<int> Prefix(Size + 1, 0);
    for (unsigned int i = 0; i < Size; ++i) Prefix[i+1] = Prefix[i] + (int) CountsPerBin[i];
    
    for (unsigned int s = 0; s < Size; ++s) {
      const int EndSum = Prefix[s+1];
      unsigned int Maximum = 0;
      float MaximumValue = 0;
      for (unsigned int i = 0; i <= s; ++i) {
        const int Counts = EndSum - Prefix[i];
        const float BlockCounts = (float) Counts;
        float Fit = BlockCounts * (LogTable[Counts] - LogWidthTable[s + 1 - i]);
        Fit -= m_Prior;
        if (i > 0) Fit += Best[i-1];
        if (i == 0) {
          MaximumValue = Fit;
        } else if (Fit > MaximumValue) {
          Maximum = i;
          MaximumValue = Fit;
        }
      }
      Last[s] = Maximum;
      Best[s] = MaximumValue;
    }
  }
  
  vector<float> Fits(Size);
  for (unsigned int s = 0; (FastPath == false) && (s < Size); ++s) {
    //cout<<s<<" / "<<Size<<endl;
    
    // Go through all possible blocks ending at s
    const float EndLength = BlockLength[s+1];
    int LastCounts = 0;
    for (unsigned int i = s; i <= s; --i) { // careful: i is unsigned
      float BlockCounts = LastCounts + CountsPerBin[i];
      LastCounts = BlockCounts;
      
      const float Width = BlockLength[i] - EndLength;
      float LogCounts;
      const int IntCounts = (int) BlockCounts;
      if (IntCounts >= 0 && (size_t) IntCounts < LogTable.size() && (float) IntCounts == BlockCounts) {
        LogCounts = LogTable[IntCounts];
      } else {
        LogCounts = log(BlockCounts);
      }
      float LogWidth;
      const unsigned int NBins = s + 1 - i; 
      if (NBins < LogWidthTable.size() && Width == (float) NBins * BinWidthFloat) {
        LogWidth = LogWidthTable[NBins];
      } else {
        LogWidth = log(Width);
      }
      float Fit = BlockCounts * (LogCounts - LogWidth);
      Fit -= m_Prior;
      Fits[i] = Fit;
    }
    
    for (unsigned int i = 1; i <= s; ++i) {
      Fits[i] += Best[i-1];
    }
    
    unsigned int Maximum = 0;
    for (unsigned int i = 0; i <= s; ++i) {
      if (Fits[i] > Fits[Maximum]) Maximum = i;
    }
    
    Last[s] = Maximum;
    Best[s] = Fits[Maximum];
  }
  
  // Scargle's implementation breaks when Size == 1
  // Step 6: Find the change points:
  vector<unsigned int> ChangePoints(Size, 0);
  unsigned int ChangePointsIndex = Size;
  unsigned int CurrentIndex = Size;
  
  while (true) {
    if (ChangePointsIndex == 0) {
      cout<<"Error: Something went wrong with the change points during Baysian Block binning... We had to stop before fully done."<<endl;
      break;
    }
    ChangePointsIndex -= 1;
    ChangePoints[ChangePointsIndex] = CurrentIndex;
    if (CurrentIndex == 0) {
      break;
    }
    CurrentIndex = Last[CurrentIndex - 1];
  }
  

  /*
  // Step 6: Find the change points:
  vector<unsigned int> ChangePoints(Size, 0);
  unsigned int ChangePointsIndex = 0;
  unsigned int CurrentIndex = Size;
  
  while (true) {
    if (ChangePointsIndex == 0) {
      cout<<"Error: Something went wrong with the change points during Baysian Block binning... We had to stop before fully done."<<endl;
      break;
    }
    ChangePointsIndex -= 1;
    ChangePoints[ChangePointsIndex] = CurrentIndex;
    if (CurrentIndex == 0) {
      break;
    }
    CurrentIndex = Last[CurrentIndex - 1];
  */
  
  //cout<<"All Change points: "<<endl;
  //Print(ChangePoints);
  //cout<<"Minimum index: "<<ChangePointsIndex<<endl;
  
  for (unsigned int i = ChangePointsIndex; i < Size; ++i) {
    m_BinEdges.push_back(Edges[ChangePoints[i]]);
  }  
  
  // Step 7: Do some sanity checks:
  if (m_UseBinning == false) {
    // Reject bins which are smaller than X
    if (m_BinEdges.size() > 2) {
      for (unsigned int i = 1; i < m_BinEdges.size(); ++i) {
        //cout<<m_BinEdges[i-1]<<".."<<m_BinEdges[i]<<endl;
        if (m_BinEdges[i] - m_BinEdges[i-1] < m_MinimumBinWidth) {
          if (i == 1) {
            m_BinEdges.erase(m_BinEdges.begin()+i);
            i--;
          } else if (i == m_BinEdges.size() - 1) {
            m_BinEdges.erase(m_BinEdges.end()-2);
            break;
          } else {
            m_BinEdges[i-1] = 0.5*(m_BinEdges[i-1] + m_BinEdges[i]);
            cout<<"New: "<<m_BinEdges[i-1]<<endl;
            m_BinEdges.erase(m_BinEdges.begin()+i);
            i--;
          }
        }
      }
      //for (unsigned int i = 1; i < m_BinEdges.size(); ++i) {
      //  cout<<m_BinEdges[i]<<endl;
      //}
    }
  }
    
  // Step 8: Finally fill the data array 
  // The bin is given by the first edge which is larger than the value
  m_BinnedData.resize(m_BinEdges.size()+1, 0);
  for (list<MBinnedData>::const_iterator I = m_Values.begin(); I != m_Values.end(); ++I) {
    const size_t e = (size_t) (std::upper_bound(m_BinEdges.begin(), m_BinEdges.end(), (*I).m_AxisValue) - m_BinEdges.begin());
    if (e > 0 && e < m_BinEdges.size()) {
      m_BinnedData[e-1] += (*I).m_DataValue;
    }
  }    
  
  // Step 9: Reject bins with less than X elements
  if (m_MinimumCountsPerBin > 0) {
    for (unsigned int e = 0; e < m_BinEdges.size()-1; ++e) {
      //cout<<"Content: "<<m_BinnedData[e]<<" going from "<<m_BinEdges[e]<<" - "<<m_BinEdges[e+1]<<endl;
      if (m_BinnedData[e] < m_MinimumCountsPerBin && e < m_BinEdges.size() - 1) {
        //cout<<"Erasing..."<<endl;
        // Move higher content down and erase bins
        m_BinnedData[e] += m_BinnedData[e+1];
        m_BinnedData.erase(m_BinnedData.begin()+e+1);
        m_BinEdges.erase(m_BinEdges.begin()+e+1);
        e--;
      }
    }
  }
  
  
  m_IsModified = false;
}


// MBinnerBayesianBlocks.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
