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
#include "MStreams.h"

// Standard libs:

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
    mout<<Array[i]<<" ";
  }
  mout<<endl;
}


////////////////////////////////////////////////////////////////////////////////


void Print(vector<int>& Array) {
  for (unsigned int i = 0; i < Array.size(); ++i) {
    mout<<Array[i]<<" ";
  }
  mout<<endl;
}


////////////////////////////////////////////////////////////////////////////////


//! The actual histogramming process - default just makes one bin
void MBinnerBayesianBlocks::Histogram()
{
  if (m_IsModified == false) return;
  
  m_BinEdges.clear();
  m_BinnedData.clear();
  
  // Step 1: Sort the Array increasing
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
  vector<float> CountsPerBin(Size, 0);
  for (list<MBinnedData>::iterator I = m_Values.begin(); I != m_Values.end(); ++I) {
    double Value = (*I).m_AxisValue;
    for (unsigned int e = 0; e < Edges.size() - 1; ++e) { // Speed improvement possible
      if (Edges[e] <= Value && Edges[e+1] > Value) {
        CountsPerBin[e] += (*I).m_DataValue;
        break;     
      }
    }
  }
  //cout<<"Counts:"<<endl;
  //Print(CountsPerBin);
  
  vector<float> Best(Size, 0.0);
  vector<unsigned int> Last(Size, 0);

  // Step 5: Iterate
  for (unsigned int s = 0; s < Size; ++s) {
    //cout<<s<<" / "<<Size<<endl;
  
    // Calculate the width of the blocks
    vector<float> Width; // log(float) is the fastest of the log calculations
    for (unsigned int i = 0; i <= s; ++i) {
      Width.push_back(BlockLength[i] - BlockLength[s+1]);
    }
    //cout<<"Width: "<<endl;
    //Print(Width);
    
    // Calculate the block count
    vector<float> BlockCounts(s+1, 0); // log(float) is the fastest of the log calculations
    int LastCounts = 0;
    for (unsigned int i = s; i <= s; --i) {
      BlockCounts[i] = LastCounts + CountsPerBin[i];
      LastCounts = BlockCounts[i];
    }
    //cout<<"BlockCounts: "<<endl;
    //Print(BlockCounts);

    //
    vector<float> Fits;
    for (unsigned int i = 0; i <= s; ++i) {
      // An empty block has the limit x*log(x) = 0, log(0) would make the fitness NaN
      float Fit = -m_Prior;
      if (BlockCounts[i] > 0) {
        Fit += BlockCounts[i] * (log(BlockCounts[i]) - log(Width[i]));
      }
      Fits.push_back(Fit);
    }
    //cout<<"Fits (2): "<<endl;
    //Print(Fits);
    for (unsigned int i = 1; i <= s; ++i) {
      Fits[i] += Best[i-1];
    }
    //cout<<"Fits (3): "<<endl;
    //Print(Fits);
  
    unsigned int Maximum = 0;
    for (unsigned int i = 0; i < Fits.size(); ++i) {
      if (Fits[i] > Fits[Maximum]) Maximum = i;
    }
    Last[s] = Maximum;
    Best[s] = Fits[Maximum];
  }
  
  // Scargle's implementation breaks when Size == 1
  // Step 6: Find the change points:
  vector<unsigned int> ChangePoints(Size + 1, 0);  // Size cells have up to Size + 1 edges
  unsigned int ChangePointsIndex = Size + 1;
  unsigned int CurrentIndex = Size;
  
  while (true) {
    if (ChangePointsIndex == 0) {
      mout<<"Error: Something went wrong with the change points during Baysian Block binning... We had to stop before fully done."<<endl;
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
      mout<<"Error: Something went wrong with the change points during Baysian Block binning... We had to stop before fully done."<<endl;
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
  
  for (unsigned int i = ChangePointsIndex; i < Size + 1; ++i) {
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
            mout<<"New: "<<m_BinEdges[i-1]<<endl;
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
  m_BinnedData.resize(m_BinEdges.size()-1, 0);
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

  // Step 9: Reject bins with less than X elements
  if (m_MinimumCountsPerBin > 0) {
    unsigned int e = 0;
    while (e < m_BinnedData.size() && m_BinnedData.size() > 1) {
      if (m_BinnedData[e] >= m_MinimumCountsPerBin) {
        ++e;
      } else if (e + 1 < m_BinnedData.size()) {
        // Move the content of the next bin down and erase it, then check this bin again
        m_BinnedData[e] += m_BinnedData[e+1];
        m_BinnedData.erase(m_BinnedData.begin()+e+1);
        m_BinEdges.erase(m_BinEdges.begin()+e+1);
      } else {
        // The last bin has no next bin: add it to the one before
        m_BinnedData[e-1] += m_BinnedData[e];
        m_BinnedData.pop_back();
        m_BinEdges.erase(m_BinEdges.end()-2);
        break;
      }
    }
  }
  
  
  m_IsModified = false;
}


// MBinnerBayesianBlocks.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
