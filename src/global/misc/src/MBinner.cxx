/*
 * MBinner.cxx
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
#include "MBinner.h"

// Standard libs:
#include <algorithm>
using namespace std;

// ROOT libs:
#include "TCanvas.h"

// MEGAlib libs:
#include "MGlobal.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MBinner)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Sort binned data by its axis value
bool SortBinnedData(const MBinnedData& D1, const MBinnedData& D2) 
{ 
  return D1.m_AxisValue < D2.m_AxisValue; 
}


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MBinner::MBinner() : m_IsModified(true), m_Minimum(0), m_Maximum(1000), m_Adapt(false)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MBinner::~MBinner()
{
}

////////////////////////////////////////////////////////////////////////////////


//! Clear all data sets
void MBinner::Clear()
{
  m_Values.clear();
  m_BinEdges.clear();
  m_BinnedData.clear();
  
  m_IsModified = true;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the minimum and maximum. If Adapt is true, then use for
//! min=max(m_Minimum, real minimum)
//! max=min(m_Maximum, real maximum)
//! Adapt will not take place if there is no data between minimum and maximum
//! If Maximum <= Minimum no changes will take place to the data
void MBinner::SetMinMax(double Minimum, double Maximum, bool Adapt) 
{ 
  if (m_Maximum <= m_Minimum) return;
  
  m_Minimum = Minimum;
  m_Maximum = Maximum;
  m_Adapt = Adapt;
}


////////////////////////////////////////////////////////////////////////////////


//! Return as a histogram (cts)
TH1D* MBinner::GetHistogram(const MString& Title, const MString& xTitle, const MString& yTitle)
{
  if (m_IsModified == true) Histogram();
  
  TH1D* Hist = 0;
  {
    TLockGuard G(g_Mutex);
    Hist = new TH1D("", Title, m_BinEdges.size()-1, &m_BinEdges[0]);
  }
  Hist->SetXTitle(xTitle);
  Hist->SetYTitle(yTitle);
  
  for (unsigned int i = 0; i < m_BinnedData.size(); ++i) {
    Hist->SetBinContent(i+1, m_BinnedData[i]);
  }
  
  return Hist;
}


////////////////////////////////////////////////////////////////////////////////


//! Return as a normalized histogram (e.g. cts/bin width)
TH1D* MBinner::GetNormalizedHistogram(const MString& Title, const MString& xTitle, const MString& yTitle)
{
  TH1D* Hist = GetHistogram(Title, xTitle, yTitle);
 
  for (int b = 0; b <= Hist->GetNbinsX(); ++b) {
    if (Hist->GetBinWidth(b) > 0) {
      Hist->SetBinContent(b, Hist->GetBinContent(b)/Hist->GetBinWidth(b));
    }
  }
 
  return Hist;
}


////////////////////////////////////////////////////////////////////////////////


//! Draw normalized histogram (e.g. cts/bin width)
void MBinner::DrawNormalizedHistogram(const MString& Title, const MString& xTitle, const MString& yTitle)
{
  TH1D* Hist = GetNormalizedHistogram(Title, xTitle, yTitle);

  TCanvas* C = new TCanvas();
  C->cd();
  Hist->SetBit(kCanDelete);
  Hist->Draw();
  C->Update();
}


////////////////////////////////////////////////////////////////////////////////


//! Return the binned data as a vector
vector<double> MBinner::GetBinnedData()
{ 
  if (m_IsModified == true) Histogram();
  return m_BinnedData; 
}


////////////////////////////////////////////////////////////////////////////////


//! Return the bin edges as a vector
vector<double> MBinner::GetBinEdges()
{ 
  if (m_IsModified == true) Histogram();
  return m_BinEdges; 
}


////////////////////////////////////////////////////////////////////////////////


//! The actual histogramming process - default just makes one bin
void MBinner::Histogram()
{
  if (m_IsModified == false) return;
  
  m_BinEdges.clear();
  m_BinnedData.clear();
  
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
    
  m_BinEdges.push_back(Min);
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
  

// MBinner.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
