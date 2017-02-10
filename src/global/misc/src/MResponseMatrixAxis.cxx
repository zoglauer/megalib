/*
 * MResponseMatrixAxis.cxx
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
#include "MResponseMatrixAxis.h"

// Standard libs:
#include <algorithm>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MExceptions.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseMatrixAxis)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MResponseMatrixAxis::MResponseMatrixAxis(const MString& Name) : m_Dimension(1), m_IsLogarithmic(false)
{
  m_Names.push_back(Name);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MResponseMatrixAxis::~MResponseMatrixAxis()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this axis
MResponseMatrixAxis* MResponseMatrixAxis::Clone() const
{
  MResponseMatrixAxis* Axis = new MResponseMatrixAxis(m_Names[0]); // Takes care of names and dimension
  Axis->m_BinEdges = m_BinEdges;
  Axis->m_IsLogarithmic = m_IsLogarithmic;
  
  return Axis;
}


////////////////////////////////////////////////////////////////////////////////


//! Equality operator
bool MResponseMatrixAxis::operator==(const MResponseMatrixAxis& Axis)
{
  if (m_Dimension != Axis.m_Dimension) {
    return false;
  }
    
  if (m_BinEdges != Axis.m_BinEdges) {
    return false;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////



//! Set the axis in linear mode
void MResponseMatrixAxis::SetLinear(unsigned int NBins, double Min, double Max, double UnderFlowMin, double OverFlowMax)
{
  if (Min >= Max) {
    throw MExceptionArbitrary("Minimum is larger or equal Maximum"); 
  }
  if (UnderFlowMin != g_DoubleNotDefined && UnderFlowMin >= Min) {
    throw MExceptionArbitrary("The underflow must be smaller than the minimum"); 
  }
  if (OverFlowMax != g_DoubleNotDefined && OverFlowMax <= Max) {
    throw MExceptionArbitrary("The underflow must be smaller than the minimum"); 
  }
  
  m_BinEdges.clear();
  
  if (UnderFlowMin != g_DoubleNotDefined) {
    m_BinEdges.push_back(UnderFlowMin);
  }
  
  double Dist = (Max-Min)/(NBins);
  
  for (unsigned int i = 0; i < NBins+1; ++i) {
    m_BinEdges.push_back(Min+i*Dist);
  }
  
  if (OverFlowMax != g_DoubleNotDefined) {
    m_BinEdges.push_back(OverFlowMax);
  }
  
  /*
  if (Inverted == true) {
    vector<double> Temp = m_BinEdges;
    for (unsigned int i = 1; i < Temp.size()-1; ++i) {
      m_BinEdges[i] = m_BinEdges[i-1] + (Temp[Temp.size()-i]-Temp[Temp.size()-i-1]);
    }
  }
  
  for (unsigned int i = 0; i < m_BinEdges.size(); ++i) {
    m_BinEdges[i] += Offset;
  }
  */
}


////////////////////////////////////////////////////////////////////////////////


//! Set the axis in logarithmic mode
void MResponseMatrixAxis::SetLogarithmic(unsigned int NBins, double Min, double Max, double UnderFlowMin, double OverFlowMax)
{
  if (Min >= Max) {
    throw MExceptionArbitrary("Minimum is larger or equal Maximum"); 
  }
  if (UnderFlowMin != g_DoubleNotDefined && UnderFlowMin >= Min) {
    throw MExceptionArbitrary("The underflow must be smaller than the minimum"); 
  }
  if (OverFlowMax != g_DoubleNotDefined && OverFlowMax <= Max) {
    throw MExceptionArbitrary("The underflow must be smaller than the minimum"); 
  }
  
  m_BinEdges.clear();
  
  if (UnderFlowMin != g_DoubleNotDefined) {
    m_BinEdges.push_back(UnderFlowMin);
  }
  
  Min = log(Min);
  Max = log(Max);
  double Dist = (Max-Min)/(NBins);
  
  for (unsigned int i = 0; i < NBins+1; ++i) {
    m_BinEdges.push_back(exp(Min+i*Dist));
  }
  
  if (OverFlowMax != g_DoubleNotDefined) {
    m_BinEdges.push_back(OverFlowMax);
  }

  m_IsLogarithmic = true;
  
  /*
  if (Inverted == true) {
    vector<double> Temp = Axis;
    for (unsigned int i = 1; i < Temp.size()-1; ++i) {
      Axis[i] = Axis[i-1] + (Temp[Temp.size()-i]-Temp[Temp.size()-i-1]);
    }
  }
  
  for (unsigned int i = 0; i < Axis.size(); ++i) {
    Axis[i] += Offset;
  }
  */
}


////////////////////////////////////////////////////////////////////////////////


//! Return the axis bin given a axis value
unsigned int MResponseMatrixAxis::GetAxisBin(double X1, double X2) const
{
  if (m_BinEdges.size() < 2) {
    throw MExceptionArbitrary("Not enough bin edges (< 2) to determine bins");
  }
  
  if (InRange(X1, X2) == false) {
    throw MExceptionValueOutOfBounds(m_BinEdges.front(), m_BinEdges.back(), X1);
  }
  
  // C++ version
  auto UpperBound = upper_bound(m_BinEdges.begin(), m_BinEdges.end(), X1);
  unsigned int Bin = UpperBound - m_BinEdges.begin() - 1;

  if (Bin >= m_BinEdges.size() - 1) {
    throw MExceptionIndexOutOfBounds(0, m_BinEdges.size() - 1, Bin);
  }
  
  return Bin;
}


////////////////////////////////////////////////////////////////////////////////


//! Test if the x-value is within the range of the axis
bool MResponseMatrixAxis::InRange(double X1, double X2) const
{
  if (m_BinEdges.size() < 2) {
    return false;
  }
  
  if (X1 < m_BinEdges.front() || X1 > m_BinEdges.back()) {
    return false;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the axis bins for drawing --- those might be narrower than the real bins
vector<vector<double>> MResponseMatrixAxis::GetDrawingAxisBinEdges() const
{
  vector<vector<double>> AxisBinEdges;
  
  AxisBinEdges.push_back(m_BinEdges);
  
  return AxisBinEdges;
}


////////////////////////////////////////////////////////////////////////////////


//! Write the content to a stream
void MResponseMatrixAxis::Write(ostringstream& out)
{
  out<<"# Axis name"<<endl;
  out<<"AN \""<<m_Names[0]<<"\""<<endl;
  out<<"# Axis type"<<endl;
  out<<"AT 1D BinEdges"<<endl;
  out<<"# Axis data"<<endl;
  out<<"AD ";
  for (unsigned int b = 0; b < m_BinEdges.size(); ++b) {
    out<<m_BinEdges[b]<<" ";
  }
  out<<endl;
}


// MResponseMatrixAxis.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
