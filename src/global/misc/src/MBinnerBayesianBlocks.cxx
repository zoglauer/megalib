/*
 * MBinnerBayesianBlocks.cxx
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
#include "MBinnerBayesianBlocks.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
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
  cout<<"Edges:"<<endl;
  Print(Edges);

  // Step 3: Create Block length:
  vector<float> BlockLength;
  for (unsigned int i = 0; i < Edges.size(); ++i) {
    BlockLength.push_back(Edges.back() - Edges[i]);
  }
  cout<<"Block length:"<<endl;
  Print(BlockLength);
  
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
  cout<<"Counts:"<<endl;
  Print(CountsPerBin);
  
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
      float Fit = BlockCounts[i] * (log(BlockCounts[i]) - log(Width[i]));
      Fit -= m_Prior;
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
  
  // Step 6: Find the change points:
  vector<unsigned int> ChangePoints(Size, 0);
  unsigned int ChangePointsIndex = Size;
  unsigned int CurrentIndex = Size;
  
  while (true) {
    ChangePointsIndex -= 1;
    ChangePoints[ChangePointsIndex] = CurrentIndex;
    if (CurrentIndex == 0) {
      break;
    }
    CurrentIndex = Last[CurrentIndex - 1];
  }
  cout<<"All Change points: "<<endl;
  Print(ChangePoints);
  cout<<"Minimum index: "<<ChangePointsIndex<<endl;
  
  for (unsigned int i = ChangePointsIndex; i < Size; ++i) {
    m_BinEdges.push_back(Edges[ChangePoints[i]]);
  }  

  cout<<"Bin edges before sanity checks: "<<endl;
  Print(m_BinEdges);
  
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

  cout<<"Bin edges after 1. sanity checks: "<<endl;
  Print(m_BinEdges);
  
  // Step 8: Finally fill the data array
  /* Old
  m_BinnedData.resize(m_BinEdges.size()+1, 0);
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
  */
  m_BinnedData.resize(m_BinEdges.size()-1, 0);
  for (list<MBinnedData>::iterator I = m_Values.begin(); I != m_Values.end(); ++I) {
    vector<double>::iterator I2 = lower_bound(m_BinEdges.begin(), m_BinEdges.end(), (*I).m_AxisValue);
    if (I2 != m_BinEdges.begin() && I2 != m_BinEdges.end()) {
      m_BinnedData[I2 - m_BinEdges.begin()] += (*I).m_DataValue;
    }
  }

  cout<<"Bin edges ..."<<endl; 
  Print(m_BinEdges);
  cout<<"... and content"<<endl;
  Print(m_BinnedData);
  
  // Step 9: Reject bins with less than X elements
  if (m_MinimumCountsPerBin > 0) {
    for (unsigned int e = 0; e < m_BinEdges.size()-1; ++e) {
      cout<<"Content: "<<m_BinnedData[e]<<" going from "<<m_BinEdges[e]<<" - "<<m_BinEdges[e+1]<<endl;
      if (m_BinnedData[e] < m_MinimumCountsPerBin && e < m_BinEdges.size() - 1) {
        cout<<"Erasing..."<<endl;
        // Move higher content down and erase bins
        m_BinnedData[e] += m_BinnedData[e+1];
        m_BinnedData.erase(m_BinnedData.begin()+e+1);
        m_BinEdges.erase(m_BinEdges.begin()+e+1);
        e--;
      }
    }
  }
  
  cout<<"Bin edges after 3. sanity checks: "<<endl;
  Print(m_BinEdges);
  
  m_IsModified = false;
}


// MBinnerBayesianBlocks.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
