/*
 * MResponseMatrixON.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MResponseMatrixON
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseMatrixON.h"

// Standard libs:
#include <iostream>
#include <iterator>
#include <functional>
#include <algorithm>
#include <limits>
#include <numeric>
#include <thread>
//#include <execution>
using namespace std;

// ROOT libs:
#include <TCanvas.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>

// MEGAlib libs:
#include "MAssert.h"
#include "MExceptions.h"
#include "MStreams.h"
#include "MFileResponse.h"
#include "MTimer.h"
#include "MResponseMatrixAxisSpheric.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MResponseMatrixON)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixON::MResponseMatrixON(bool IsSparse) : MResponseMatrix(), m_NumberOfBins(0), m_NumberOfAxes(0), m_IsSparse(IsSparse)
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixON::MResponseMatrixON(const MString& Name, bool IsSparse) : MResponseMatrix(Name), m_NumberOfBins(0), m_NumberOfAxes(0), m_IsSparse(IsSparse)
{
  // extended constructor
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixON::MResponseMatrixON(const MResponseMatrixON& M)
{
  // copy constructor
  
  m_NumberOfBins = M.m_NumberOfBins;
  m_Axes = M.m_Axes;
  m_NumberOfAxes = M.m_NumberOfAxes;
  m_IsSparse = M.m_IsSparse;
  m_Values = M.m_Values;
  m_ValuesSparse = M.m_ValuesSparse;
  m_BinsSparse = M.m_BinsSparse;
  m_ThreadRunning = M.m_ThreadRunning;
  // That's the culprit preventing a default copy constructor - do not copy: m_ThreadMutex = M.m_ThreadMutex;
  m_ThreadLines = M.m_ThreadLines;
  m_ThreadGoodData = M.m_ThreadGoodData;
  m_ThreadBins = M.m_ThreadBins;
  m_ThreadValues = M.m_ThreadValues;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixON::~MResponseMatrixON()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixON::Clear()
{
  // Clear all data
  
  //! The axes
  for (auto A: m_Axes) {
    delete A;
  }
  m_Axes.clear();
  m_NumberOfAxes = 0;
  
  m_NumberOfBins = 0;
  
  m_Values.clear();

  m_ValuesSparse.clear();
  m_BinsSparse.clear();
  
  MResponseMatrix::Clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Switch to sparse mode 
void MResponseMatrixON::SwitchToSparse()
{
  if (m_IsSparse == true) return;
  
  m_ValuesSparse.clear();
  m_BinsSparse.clear();
  
  for (unsigned long i = 0; i < m_NumberOfBins; ++i) {
    if (m_Values[i] != 0.0) {
      m_ValuesSparse.push_back(m_Values[i]);
      m_BinsSparse.push_back(i);
    }
  }
  m_IsSparse = true;
  
  m_Values.clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Switch to non-sparse mode
void MResponseMatrixON::SwitchToNonSparse()
{
  if (m_IsSparse == false) return;
  
  m_Values.clear();
  m_Values.resize(m_NumberOfBins, 0);
  
  for (unsigned long i = 0; i < m_BinsSparse.size(); ++i) {
    m_Values[m_BinsSparse[i]] = m_ValuesSparse[i];
  }
  m_IsSparse = false;
  
  m_ValuesSparse.clear();
  m_BinsSparse.clear();
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixON::AddAxis(const MResponseMatrixAxis& Axis)
{
  // Set the axis

  m_Axes.push_back(Axis.Clone());
  m_NumberOfAxes++;
  
  m_Order += Axis.GetDimension();
  
  m_NumberOfBins = CalculateNumberOfBins();
  
  if (m_IsSparse == false) {
    m_Values.resize(m_NumberOfBins, 0);
  }

  ostringstream out;
  m_Axes.back()->Write(out);
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixON::AddAxisLinear(const MString& Name, unsigned long NBins, double Min, double Max, double UnderFlowMin, double OverFlowMax)
{
  // Set the axis

  MResponseMatrixAxis* Axis = new MResponseMatrixAxis(Name);
  Axis->SetLinear(NBins, Min, Max, UnderFlowMin, OverFlowMax);

  m_Axes.push_back(Axis);
  m_NumberOfAxes++;
  
  m_Order += 1;
  
  m_NumberOfBins = CalculateNumberOfBins();
  
  if (m_IsSparse == false) {
    m_Values.resize(m_NumberOfBins, 0);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixON::AddAxisLogarithmic(const MString& Name, unsigned long NBins, double Min, double Max, double UnderFlowMin, double OverFlowMax)
{
  // Set the axis

  MResponseMatrixAxis* Axis = new MResponseMatrixAxis(Name);
  Axis->SetLogarithmic(NBins, Min, Max, UnderFlowMin, OverFlowMax);

  m_Axes.push_back(Axis);
  m_NumberOfAxes++;
  
  m_Order += 1;
  
  m_NumberOfBins = CalculateNumberOfBins();
  
  if (m_IsSparse == false) {
    m_Values.resize(m_NumberOfBins, 0);
  }
}


////////////////////////////////////////////////////////////////////////////////


vector<MString> MResponseMatrixON::GetAxisNames(unsigned int AxisIndex) const
{
  // Return the name of the axis

  if (AxisIndex >= m_NumberOfAxes) {
    throw MExceptionIndexOutOfBounds(0, m_NumberOfAxes, AxisIndex);
  }

  return m_Axes[AxisIndex]->GetNames();
}


////////////////////////////////////////////////////////////////////////////////


//! Return a const reference to the given axis
//! Throws exception MExceptionIndexOutOfBounds
const MResponseMatrixAxis& MResponseMatrixON::GetAxis(unsigned int AxisIndex) const
{
  if (AxisIndex >= m_NumberOfAxes) {
    throw MExceptionIndexOutOfBounds(0, m_NumberOfAxes, AxisIndex);
  }

  return *m_Axes[AxisIndex];
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrixON::operator==(const MResponseMatrixON& R)
{
  // Two matrixes are identical if they have the same axis:

  if (m_NumberOfAxes != R.m_NumberOfAxes) {
    return false;
  }

  for (unsigned int a = 0; a < m_NumberOfAxes; ++a) {
    if (*m_Axes[a] != *R.m_Axes[a]) {
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixON& MResponseMatrixON::operator=(const MResponseMatrixON& M)
{
  // Assignment operator
  
  if (this != &M) { // no self-assignments
    m_NumberOfBins = M.m_NumberOfBins;
    m_Axes = M.m_Axes;
    m_NumberOfAxes = M.m_NumberOfAxes;
    m_IsSparse = M.m_IsSparse;
    m_Values = M.m_Values;
    m_ValuesSparse = M.m_ValuesSparse;
    m_BinsSparse = M.m_BinsSparse;
    m_ThreadRunning = M.m_ThreadRunning;
    // That's the culprit preventing a default copy constructor - do not copy: m_ThreadMutex = M.m_ThreadMutex;
    m_ThreadLines = M.m_ThreadLines;
    m_ThreadGoodData = M.m_ThreadGoodData;
    m_ThreadBins = M.m_ThreadBins;
    m_ThreadValues = M.m_ThreadValues;
  }
  
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixON& MResponseMatrixON::operator+=(const MResponseMatrixON& R)
{
  // Append a matrix to this one

  if (*this == R) {
    if (m_IsSparse == false) {
      if (R.m_IsSparse == true) {
        for (unsigned long i = 0; i < R.m_BinsSparse.size(); ++i) {
          m_Values[R.m_BinsSparse[i]] += R.m_ValuesSparse[i];
        }
      } else {
        for (unsigned long i = 0; i < m_NumberOfBins; ++i) {
          m_Values[i] += R.m_Values[i]; 
        }
      }
    } else {
      if (R.m_IsSparse == true) {
        for (unsigned long i = 0; i < R.m_BinsSparse.size(); ++i) {
          Add(R.m_BinsSparse[i], R.m_ValuesSparse[i]);
        }
      } else {
        for (unsigned long i = 0; i < R.m_NumberOfBins; ++i) {
          if (R.m_Values[i] != 0) {
            Add(i, R.m_Values[i]);
          }
        }        
      }
    }
  } else {
    throw MExceptionObjectsNotIdentical("Response Matrix A", "Response matrix B");
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixON& MResponseMatrixON::operator-=(const MResponseMatrixON& R)
{
  // Subtract a matrix from this one

  if (*this == R) {
    if (m_IsSparse == false) {
      if (R.m_IsSparse == true) {
        for (unsigned long i = 0; i < R.m_BinsSparse.size(); ++i) {
          m_Values[R.m_BinsSparse[i]] -= R.m_ValuesSparse[i];
        }
      } else {
        for (unsigned long i = 0; i < m_NumberOfBins; ++i) {
          m_Values[i] -= R.m_Values[i]; 
        }
      }
    } else {
      if (R.m_IsSparse == true) {
        for (unsigned long i = 0; i < R.m_BinsSparse.size(); ++i) {
          Add(R.m_BinsSparse[i], -R.m_ValuesSparse[i]);
        }
      } else {
        for (unsigned long i = 0; i < R.m_NumberOfBins; ++i) {
          if (R.m_Values[i] != 0) {
            Add(i, -R.m_Values[i]);
          }
        }        
      }
    }
  } else {
    throw MExceptionObjectsNotIdentical("Response Matrix A", "Response matrix B");
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixON& MResponseMatrixON::operator/=(const MResponseMatrixON& R)
{
  // Append a matrix to this one

  if (*this == R) {
    if (m_IsSparse == false) {
      for (unsigned long i = 0; i < m_NumberOfBins; ++i) {
        float RValue = R.Get(i); // R maybe sparse or not...
        if (RValue != 0) {
          m_Values[i] /= RValue;
        } else {
          m_Values[i] = 0;
        }
      }
    } else {
      // We just need to loop over the non-zeroes here
      for (unsigned long i = 0; i < m_BinsSparse.size(); ++i) {
        float RValue = R.Get(m_BinsSparse[i]); // R maybe sparse or not...
        if (RValue != 0) {
          m_ValuesSparse[i] /= RValue;
        } else {
          // NaN, but we will set it to zero
          m_ValuesSparse[i] = 0;
        } 
      }
    }
  } else {
    throw MExceptionObjectsNotIdentical("Response Matrix A", "Response matrix B");
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixON& MResponseMatrixON::operator+=(const float& Value)
{
  // Append this value to the matrix
  
  if (isfinite(Value) == false) {
    throw MExceptionNumberNotFinite();
  }
  
  // We will be non-sparse now:
  if (m_IsSparse == true) {
    SwitchToNonSparse(); 
  }
  
  for (unsigned long i = 0; i < m_NumberOfBins; ++i) {
    m_Values[i] += Value;
  }
  
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixON& MResponseMatrixON::operator-=(const float& Value)
{
  // Subtract this value from the matrix
  
  if (isfinite(Value) == false) {
    throw MExceptionNumberNotFinite();
  }
  
  // we will be non-sparse now:
  if (m_IsSparse == true) {
    SwitchToNonSparse(); 
  }
  
  for (unsigned long i = 0; i < m_NumberOfBins; ++i) {
    m_Values[i] -= Value;
  }
  
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixON& MResponseMatrixON::operator*=(const float& Value)
{
  // Multiply a matrix by this value
  
  if (isfinite(Value) == false) {
    throw MExceptionNumberNotFinite();
  }

  if (m_IsSparse == false) {
    for (unsigned long i = 0; i < m_NumberOfBins; ++i) {
      m_Values[i] *= Value;
    }
  } else {
    for (unsigned long i = 0; i < m_BinsSparse.size(); ++i) {
      m_ValuesSparse[i] *= Value;
    }
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixON& MResponseMatrixON::operator/=(const float& Value)
{
  // Divide a matrix by this value
  
  if (Value == 0) {
    throw MExceptionDivisionByZero();
  }
  
  if (isfinite(Value) == false) {
    throw MExceptionNumberNotFinite();
  }
  
  if (m_IsSparse == false) {
    for (unsigned long i = 0; i < m_NumberOfBins; ++i) {
      m_Values[i] /= Value;
    }
  } else {
    for (unsigned long i = 0; i < m_BinsSparse.size(); ++i) {
      m_ValuesSparse[i] /= Value;
    }
  }
  
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixON MResponseMatrixON::Collapse(vector<bool> Collapse)
{ 
  //! Collapse some of the axes, true mean collapse, false measn keep

  if (Collapse.size() != m_NumberOfAxes) {
    throw MExceptionArbitrary("The size of the input vector is not the same as the number of matrix axes!");
  }

  // Create the new matrix:
  MResponseMatrixON New(m_Name);
  for (unsigned int a = 0; a < Collapse.size(); ++a) {
    if (Collapse[a] == false) {
      New.AddAxis(GetAxis(a));
    }
  }
  
  // Loop over all bins
  vector<unsigned long> NewBin(New.m_NumberOfAxes);
  if (m_IsSparse == true) {
    for (unsigned long b = 0; b < m_BinsSparse.size(); ++b) {
	    vector<unsigned long> OldBin = FindBins(m_BinsSparse[b]);
      unsigned int nb = 0;
      //NewBin.clear();
      for (unsigned long ob = 0; ob < m_NumberOfAxes; ob ++) {
        if (Collapse[ob] == false) {
          NewBin[nb++] = OldBin[ob];
        }
      }
	    New.Add(NewBin, m_ValuesSparse[b]);
	  }
  } else {
    for (unsigned long b = 0; b < m_NumberOfBins; ++b) {
	    vector<unsigned long> OldBin = FindBins(b);
      unsigned int nb = 0;
      //NewBin.clear();
      for (unsigned long ob = 0; ob < m_NumberOfAxes; ob ++) {
        if (Collapse[ob] == false) {
          NewBin[nb++] = OldBin[ob];
        }
      }
	    New.Add(NewBin, m_Values[b]);
	  }
  }
  
  return New;
}


////////////////////////////////////////////////////////////////////////////////


unsigned long MResponseMatrixON::FindBin(vector<unsigned long> X) const
{
  // Find the bin of m_Values corresponding to the axis bins X
  // Logic: a1 + S1*a2 + S1*S2*a3 + S1*S2*S3*a4 + ....

  if (X.size() != m_NumberOfAxes) {
    throw MExceptionTestFailed("The axes sizes are not identical", X.size(), "!=", m_NumberOfAxes);
  }

  double Multiplier = 0;
  unsigned long Bin = 0;
  for (unsigned int a = 0; a < m_NumberOfAxes; ++a) {
    if (X[a] >= m_Axes[a]->GetNumberOfBins()) {
      throw MExceptionIndexOutOfBounds(0, m_Axes[a]->GetNumberOfBins(), X[a]);
    }

    if (a == 0) {
      Multiplier = 1;
    } else {
      Multiplier *= m_Axes[a-1]->GetNumberOfBins();
    }
    Bin += Multiplier*X[a];
  }

  if (Bin >= m_NumberOfBins) {
    throw MExceptionIndexOutOfBounds(0, m_NumberOfBins, Bin);
  }

  return Bin;
}


////////////////////////////////////////////////////////////////////////////////


vector<unsigned long> MResponseMatrixON::FindBins(unsigned long Bin) const
{
  // Find the axes bins corresponding to the internal value bin Bin

  vector<unsigned long> Bins(m_NumberOfAxes);

  for (unsigned int a = 0; a < m_NumberOfAxes; ++a) {
    Bins[a] = Bin % m_Axes[a]->GetNumberOfBins();
    Bin = (Bin - Bins[a]) / m_Axes[a]->GetNumberOfBins();
  }

  return Bins;
}


////////////////////////////////////////////////////////////////////////////////


//! Find the axes bins corresponding to the sparse Bin
vector<unsigned long> MResponseMatrixON::FindBinsSparse(unsigned long SparseBin) const
{
  if (SparseBin < m_BinsSparse.size()) {
    return FindBins(m_BinsSparse[SparseBin]);
  } else {
    throw MExceptionIndexOutOfBounds(0, m_BinsSparse.size(), SparseBin);
    return vector<unsigned long>();
  }
}


////////////////////////////////////////////////////////////////////////////////


vector<unsigned long> MResponseMatrixON::FindBins(vector<double> X) const
{
  // Find the bin of m_Values corresponding to the axis values X

  unsigned long Xbin = 0;
  vector<unsigned long> Bins;

  if (X.size() != m_Order) {
    throw MExceptionTestFailed("The axes sizes are not identical", X.size(), "!=", m_Order);
    return Bins;
  }

  //cout<<"Find Bin: Input data: "; for (auto b: X) cout<<b<<" "; cout<<endl;

  for (unsigned int a = 0; a < m_NumberOfAxes; ++a) {
    unsigned int Dimension = m_Axes[a]->GetDimension();
    if (Dimension == 1) {
      if (m_Axes[a]->InRange(X[Xbin]) == true) {
        Bins.push_back(m_Axes[a]->GetAxisBin(X[Xbin]));
      } else {
        cout<<m_Name<<": Axis: "<<m_Axes[a]->GetNames()[0]<<" (xbin="<<Xbin<<"/"<<X.size()<<")"<<endl;
        throw MExceptionValueOutOfBounds(X[Xbin]);
      }
    } else if (Dimension == 2) {
      if (m_Axes[a]->InRange(X[Xbin], X[Xbin+1]) == true) {
        Bins.push_back(m_Axes[a]->GetAxisBin(X[Xbin], X[Xbin+1]));
      } else {
        cout<<X[Xbin]<<":"<<X[Xbin+1]<<endl;
        throw MExceptionValueOutOfBounds();
      }
    } else {
      throw MExceptionNeverReachThatLineOfCode("Dimension of the axis not handled!");
      return Bins;
    }
    Xbin += Dimension;
  }

  return Bins;
}


////////////////////////////////////////////////////////////////////////////////


unsigned long MResponseMatrixON::FindBin(vector<double> X) const
{
  // Find the bin of m_Values corresponding to the axis values X

  return FindBin(FindBins(X));
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrixON::InRange(vector<double> X) const
{
  // Check if the values are inside the range of all axes

  if (X.size() != m_Order) {
    throw MExceptionTestFailed("The matrix dimension (input vs. internal) are not identical", X.size(), "!=", m_Order);
    return false;
  }

  //cout<<"Inrange: Input data: "; for (auto b: X) cout<<b<<" "; cout<<endl;

  unsigned long Xbin = 0;
  for (unsigned int a = 0; a < m_NumberOfAxes; ++a) {
    unsigned int Dimension = m_Axes[a]->GetDimension();
    if (Dimension == 1) {
      if (m_Axes[a]->InRange(X[Xbin]) == false) {
        cout<<"Response matrix "<<m_Name<<": Not in range: "<<m_Axes[a]->GetNames()[0]<<": "<<X[Xbin]<<endl;
        return false;
      }
    } else if (Dimension == 2) {
      if (m_Axes[a]->InRange(X[Xbin], X[Xbin+1]) == false) {
        cout<<"Response matrix "<<m_Name<<": Not in range: "<<m_Axes[a]->GetNames()[0]<<": "<<X[Xbin]<<endl;
        return false;
      }
    } else {
      throw MExceptionNeverReachThatLineOfCode("Dimension of the axis not handled!");
      return false;
    }
    Xbin += Dimension;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrixON::InRange(vector<unsigned long> Bins) const
{
  // Check if the values are inside the range of all axes

  if (Bins.size() != m_NumberOfAxes) {
    throw MExceptionTestFailed("The axes sizes (input vs. internal) are not identical", Bins.size(), "!=", m_NumberOfAxes);
    return false;
  }

  for (unsigned int a = 0; a < m_NumberOfAxes; ++a) {
    if (Bins[a] >= m_Axes[a]->GetNumberOfBins()) {
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixON::Set(vector<unsigned long> X, float Value)
{
  // Set the content of the bin
  
  if (isfinite(Value) == false) {
    throw MExceptionNumberNotFinite();
  }
  
  if (InRange(X) == false) {
    return;
  }

  unsigned long Bin = FindBin(X);
  
  Set(Bin, Value);
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixON::Set(vector<double> X, float Value)
{
  // Set a value
  
  if (isfinite(Value) == false) {
    throw MExceptionNumberNotFinite();
  }
  
  if (InRange(X) == false) {
    return;
  }

  unsigned long Bin = FindBin(X);
  
  Set(Bin, Value);
}


////////////////////////////////////////////////////////////////////////////////


//! Set the content of a specific bin -- directly without error checks
//! Logic: a1 + S1*a2 + S1*S2*a3 + S1*S2*S3*a4 + ....  
void MResponseMatrixON::Set(unsigned long Bin, float Value) 
{ 
  if (m_IsSparse == false) {
    m_Values[Bin] = Value;
  } else {
    // Find the position in the sparse array which is greater or equal to Bin
    auto IterBins = lower_bound(m_BinsSparse.begin(), m_BinsSparse.end(), Bin);
    // Find the same position in the values vector
    auto IterValues = m_ValuesSparse.begin();
    advance(IterValues, distance(m_BinsSparse.begin(), IterBins));
    
    // If we have already an entry add, otherwise insert another entry sorted.
    if (IterBins != m_BinsSparse.cend() && *IterBins == Bin) {
      (*IterValues) = Value;
    } else {
      if (Value != 0) { // no need to insert 0's
        m_BinsSparse.insert(IterBins, Bin);
        m_ValuesSparse.insert(IterValues, Value);
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixON::Add(vector<unsigned long> X, float Value)
{
  // Add a value to the bin closest to x, y
  
  if (isfinite(Value) == false) {
    throw MExceptionNumberNotFinite();
  }
  
  if (InRange(X) == false) {
    return;
  }
  
  unsigned long Bin = FindBin(X);

  Add(Bin, Value);
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixON::Add(vector<double> X, float Value)
{
  // Add a value to the bin closest to x, y
  
  if (isfinite(Value) == false) {
    throw MExceptionNumberNotFinite();
  }
  
  if (InRange(X) == false) {
    return;
  }
  
  unsigned long Bin = FindBin(X);

  Add(Bin, Value);
}


////////////////////////////////////////////////////////////////////////////////


//! Add to the content of a specific bin -- directly without error checks
//! Logic: a1 + S1*a2 + S1*S2*a3 + S1*S2*S3*a4 + ....  
void MResponseMatrixON::Add(unsigned long Bin, float Value) 
{ 
  if (m_IsSparse == false) {
    m_Values[Bin] += Value;
  } else {
    // Find the position in the sparse array which is greater or equal to Bin
    auto IterBins = lower_bound(m_BinsSparse.begin(), m_BinsSparse.end(), Bin);
    // Find the same position in the values vector
    auto IterValues = m_ValuesSparse.begin();
    advance(IterValues, distance(m_BinsSparse.begin(), IterBins));
    
    // If we have already an entry add, otherwise insert another entry sorted.
    if (IterBins != m_BinsSparse.cend() && *IterBins == Bin) {
      (*IterValues) += Value;
    } else {
      m_BinsSparse.insert(IterBins, Bin);
      m_ValuesSparse.insert(IterValues, Value);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////


//! Add a collection of bin values
void MResponseMatrixON::Add(vector<unsigned long> Bins, vector<float> Values)
{
  if (Bins.size() != Values.size()) {
    throw MExceptionTestFailed("Vector size are are not identical", Bins.size(), "!=", Values.size());
    return;
  }
  
  for (unsigned int e = 0; e < Bins.size(); ++e) {
    Add(Bins[e], Values[e]);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixON::Multiply(vector<unsigned long> X, float Value)
{
  // Multiply a value to the bin closest to x, y
  
  if (isfinite(Value) == false) {
    throw MExceptionNumberNotFinite();
  }
  
  if (InRange(X) == false) {
    return;
  }
  
  unsigned long Bin = FindBin(X);

  Multiply(Bin, Value);
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixON::Multiply(vector<double> X, float Value)
{
  // Multiply a value to the bin closest to x, y
  
  if (isfinite(Value) == false) {
    throw MExceptionNumberNotFinite();
  }
  
  if (InRange(X) == false) {
    return;
  }
  
  unsigned long Bin = FindBin(X);

  Multiply(Bin, Value);
}


////////////////////////////////////////////////////////////////////////////////


//! Multiply with the content of a specific bin -- directly without error checks
//! Logic: a1 + S1*a2 + S1*S2*a3 + S1*S2*S3*a4 + ....  
void MResponseMatrixON::Multiply(unsigned long Bin, float Value) 
{ 
  if (m_IsSparse == false) {
    m_Values[Bin] *= Value;
  } else {
    // Find the position in the sparse array which is greater or equal to Bin
    auto IterBins = lower_bound(m_BinsSparse.begin(), m_BinsSparse.end(), Bin);
    // Find the same position in the values vector
    auto IterValues = m_ValuesSparse.begin();
    advance(IterValues, distance(m_BinsSparse.begin(), IterBins));
    
    // If we have already an entry add, otherwise insert another entry sorted.
    if (IterBins != m_BinsSparse.cend() && *IterBins == Bin) {
      (*IterValues) += Value;
    } else {
      m_BinsSparse.insert(IterBins, Bin);
      m_ValuesSparse.insert(IterValues, 0);
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Set the content of a sparse bin
void MResponseMatrixON::SetSparse(unsigned long SparseBin, float Value)
{
  if (m_IsSparse == true && SparseBin < m_BinsSparse.size()) {
    m_ValuesSparse[SparseBin] = Value;
  } else {
    throw MExceptionIndexOutOfBounds(0, m_BinsSparse.size(), SparseBin);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Add to the content of a sparse bin
void MResponseMatrixON::AddSparse(unsigned long SparseBin, float Value)
{
  if (m_IsSparse == true && SparseBin < m_BinsSparse.size()) {
    m_ValuesSparse[SparseBin] += Value;
  } else {
    throw MExceptionIndexOutOfBounds(0, m_BinsSparse.size(), SparseBin);
  }
}


////////////////////////////////////////////////////////////////////////////////


unsigned long MResponseMatrixON::CalculateNumberOfBins() const
{
  // Return the number of bins

  unsigned long Bins = 0;
  for (unsigned int a = 0; a < m_NumberOfAxes; ++a) {
    if (Bins == 0) {
      Bins = m_Axes[a]->GetNumberOfBins();
    } else {
      Bins *= m_Axes[a]->GetNumberOfBins();
    }
  }

  return Bins;
}


////////////////////////////////////////////////////////////////////////////////


//! Get the content of a specific bin -- directly without error checks
//! Logic: a1 + S1*a2 + S1*S2*a3 + S1*S2*S3*a4 + ....  
float MResponseMatrixON::Get(unsigned long Bin) const
{
  if (m_IsSparse == false) {
    return m_Values[Bin];
  } else {
    // Find the position in the sparse array which is greater or equal to Bin
    auto IterBins = lower_bound(m_BinsSparse.cbegin(), m_BinsSparse.cend(), Bin);
    
    // If we have already an entry add, otherwise insert another entry sorted.
    if (IterBins != m_BinsSparse.cend() && *IterBins == Bin) {
      // Find the same position in the values vector
      auto IterValues = m_ValuesSparse.cbegin();
      advance(IterValues, distance(m_BinsSparse.cbegin(), IterBins));

      return (*IterValues);
    } else {
      return 0;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixON::Get(vector<unsigned long> X) const
{
  // Return the content of bin x, y
  
  return Get(FindBin(X));
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixON::GetInterpolated(vector<double> X, bool DoExtrapolate) const
{
  // Return the array-data according to value x1

  if (InRange(X) == false) {
    cout<<"Out of range: ";
    for (auto x: X) cout<<x<<" ";
    cout<<endl;
    return 0;
  }

  mimp<<"Interpolation not implemented!"<<endl;

  return Get(FindBin(X));

  // We have to distinguish two different cases:
  // (1) the values are at the center of the bin (e.g. used by response files)
  // (2) the values are the edge (axis point) (e.g. used by MEGAlib absorption files)

  // We can assume that the bin size is larger equal 2

  // The linear interpolation goes as follows:
  // y = m*x + t
  // m = (y2-y1)/(x2-x1)
  // t = y1 - m*x1
  // y = y1 + (x-x1)*m


  /*
  if (m_ValuesCentered == true) {
    if (m_AxisO1.size() == 2) {
      return m_Values.front();
    } else {
      // Get Position (lower bound)
      long Position = FindBinCentered(m_AxisO1, x1);

      // Take care of boundaries:
      if (Position < 0) {
        if (DoExtrapolate == true) {
          Position = 0; // extrapolate below lower edge
        } else {
          return m_Values.front();
        }
      } else if (Position >= long(m_NumberOfBins-1)) {
        if (DoExtrapolate == true) {
          Position = long(m_NumberOfBins-2); // extrapolate above higher edge
          // limits of highest CENTERED bin are m_NumberOfBins-2 and m_NumberOfBins-1 !!
        } else {
          return m_Values.back();
        }
      }

      // Interpolate:
      return m_Values[Position] + (x1 - GetAxisBinCenter(Position))/
          (GetAxisBinCenter(Position+1) - GetAxisBinCenter(Position))*
          (m_Values[Position+1] - m_Values[Position]);
    }
  } else {
    // Get Position (lower bound)
    long Position = FindBin(m_AxisO1, x1);

    // Take care of boundaries:
    if (Position < 0) {
      if (DoExtrapolate == true) {
        Position = 0; // extrapolate below lower edge
      } else {
        return m_Values.front();
      }
    } else if (Position >= long(m_AxisO1.size()-1)) {
      if (DoExtrapolate == true) {
        Position = long(m_AxisO1.size()-2); // extrapolate above higher edge
        // limits of highest bin are m_AxisO2.size()-2 and  m_AxisO2.size()-1 !!
      } else {
        return m_Values.back();
      }
    }

    // Interpolate:
    return m_Values[Position] + (x1 - m_AxisO1[Position])/
      (m_AxisO1[Position+1] - m_AxisO1[Position])*
      (m_Values[Position+1] - m_Values[Position]);
  }
  */
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixON::Get(vector<double> X) const
{
  // Return the array-data according to value x

  if (InRange(X) == false) {
    cout<<"Out of range: ";
    for (auto x: X) cout<<x<<" ";
    cout<<endl;
    return 0;
  }

  return Get(FindBin(X));
}

////////////////////////////////////////////////////////////////////////////////


//! Add to the content of a sparse bin
float MResponseMatrixON::GetSparse(unsigned long SparseBin) const
{
  if (m_IsSparse == true && SparseBin < m_BinsSparse.size()) {
    return m_ValuesSparse[SparseBin];
  }
  
  throw MExceptionIndexOutOfBounds(0, m_BinsSparse.size(), SparseBin);
  
  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixON::GetArea(vector<double> X) const
{
  // Return the area of the bin corresponding to X

  if (InRange(X) == false) {
    cout<<"Out of range: ";
    for (auto x: X) cout<<x<<" ";
    cout<<endl;
    return 0.0;
  }

  vector<unsigned long> Bins = FindBins(X);

  double Area = 1;
  for (unsigned long b = 0; b < Bins.size(); ++b) {
    Area *= m_Axes[b]->GetArea(Bins[b]);
  }

  return Area;
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixON::GetMaximum() const
{
  // Return the maximum

  float Max = -numeric_limits<float>::max();
  
  if (m_IsSparse == false) {
    for (unsigned long i = 0; i < m_NumberOfBins; ++i) {
      if (m_Values[i] > Max) {
        Max = m_Values[i];
      }
    }
  } else {
    Max = 0;
    for (unsigned long i = 0; i < m_ValuesSparse.size(); ++i) {
      if (m_ValuesSparse[i] > Max) {
        Max = m_ValuesSparse[i];
      }
    }    
  }

  return Max;
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixON::GetMinimum() const
{
  // Return the minimum

  float Min = numeric_limits<float>::max();
  
  if (m_IsSparse == false) {
    for (unsigned long i = 0; i < m_NumberOfBins; ++i) {
      if (m_Values[i] < Min) {
        Min = m_Values[i];
      }
    }
  } else {
    Min = 0;
    for (unsigned long i = 0; i < m_ValuesSparse.size(); ++i) {
      if (m_ValuesSparse[i] < Min) {
        Min = m_ValuesSparse[i];
      }
    } 
  }

  return Min;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMatrixON::GetSum() const
{
  // Return the sum of all bins:

  double Sum = 0;
  
  if (m_IsSparse == false) {
    for (unsigned long i = 0; i < m_NumberOfBins; ++i) {
      Sum += m_Values[i];
    }
  } else {
    for (unsigned long i = 0; i < m_ValuesSparse.size(); ++i) {
      Sum += m_ValuesSparse[i];
    } 
  }

  return Sum;
}

////////////////////////////////////////////////////////////////////////////////


//! Sort the sparse matrix
void MResponseMatrixON::SortSparse()
{
  // Create a sort permutation:
  vector<unsigned long> Permutation(m_BinsSparse.size());
  iota(Permutation.begin(), Permutation.end(), 0);
  sort(Permutation.begin(), Permutation.end(), [&](unsigned long i, unsigned long j) { return m_BinsSparse[i] < m_BinsSparse[j]; } );
  
  // Apply the permutation
  vector<unsigned long> SortedBins(m_BinsSparse.size());
  transform(Permutation.begin(), Permutation.end(), SortedBins.begin(), [&](unsigned long i) { return m_BinsSparse[i]; });
  m_BinsSparse = SortedBins;
  
  vector<float> SortedValues(m_ValuesSparse.size());
  transform(Permutation.begin(), Permutation.end(), SortedValues.begin(), [&](unsigned long i) { return m_ValuesSparse[i]; });
  m_ValuesSparse = SortedValues;
}


////////////////////////////////////////////////////////////////////////////////


//! Given an order, return the axis it belongs to
MResponseMatrixAxis* MResponseMatrixON::GetAxisByOrder(unsigned int Order)
{
  if (Order < 1 || Order > m_Order) {
    throw MExceptionValueOutOfBounds(1, m_Order, Order);
  }

  unsigned int CountedOrder = 0;
  for (unsigned int a = 0; a < m_NumberOfAxes; ++a) {
    CountedOrder += m_Axes[a]->GetDimension();
    if (CountedOrder >= Order) {
      return m_Axes[a];
    }
  }

  throw MExceptionNeverReachThatLineOfCode();

  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrixON::ReadSpecific(MFileResponse& Parser,
                                     const MString& Type,
                                     const int Version)
{
  // Read the data from file directly into this matrix

  bool Ok = true;

  MTimer Timer;


  MTokenizer T;
  vector<MTokenizer> xAxis;
  MString x1Name;
  while (Parser.TokenizeLine(T, true) == true) {
    if (T.GetNTokens() < 2) continue;
    if (T.GetTokenAt(0) == "AN") {
      vector<MString> AxisName = T.GetTokenAtAsStringVector(1, true);
      //for (unsigned int i = 0; i < AxisName.size(); ++i) {
      //  cout<<"N: "<<AxisName[i]<<endl;
      //}
      // Sub parse until we have found axis type
      while (Parser.TokenizeLine(T, true) == true) {
        if (T.GetNTokens() < 2) continue;
        if (T.GetTokenAt(0) == "AT") {
          MString Type = T.GetTokenAfterAsString(1);
          if (Type == "1D BinEdges") {
            if (AxisName.size() != 1) {
              mout<<"MResponseMatrixON: Did not find a single axis name for the response matrix axis!"<<endl;
              return false;
            }
            MResponseMatrixAxis* A = new MResponseMatrixAxis(AxisName[0]);
            // Sub parse until we found the axis data
            while (Parser.TokenizeLine(T, true) == true) {
              if (T.GetNTokens() < 2) continue;
              if (T.GetTokenAt(0) == "AD") {
                A->SetBinEdges(T.GetTokenAtAsDoubleVector(1));
                m_Axes.push_back(A);
                break;
              }
            }
          } else if (Type == "2D FISBEL") {
            if (AxisName.size() != 2) {
              mout<<"MResponseMatrixON: Did not find two axis names for the response matrix axis!"<<endl;
              return false;
            }
            MResponseMatrixAxisSpheric* A = new MResponseMatrixAxisSpheric(AxisName[0], AxisName[1]);
            // Sub parse until we found the axis data
            while (Parser.TokenizeLine(T, true) == true) {
              if (T.GetNTokens() < 2) continue;
              if (T.GetTokenAt(0) == "AD") {
                if (T.GetNTokens() == 2) {
                  A->SetFISBEL(T.GetTokenAtAsUnsignedInt(1));
                } else if (T.GetNTokens() == 3) {
                  A->SetFISBEL(T.GetTokenAtAsUnsignedInt(1), T.GetTokenAtAsDouble(2));
                } else {
                  mout<<"MResponseMatrixON: The FISBEL AD axis key word needs 1 (only the bins) or 2 (bins & longitude shift) arguments!"<<endl;
                  return false;
                }
                m_Axes.push_back(A);
                break;
              }
            }
          }
        }
        break;
      }
    } else if (T.GetTokenAt(0) == "Type") {
    
      m_NumberOfAxes = m_Axes.size();
      m_NumberOfBins = CalculateNumberOfBins();
      
      if (Type == "ResponseMatrixONStream") {
        while (Parser.TokenizeLine(T, true) == true) {
          if (T.GetNTokens() < 2) continue;

          if (T.GetTokenAt(0) == "StartStream") {
            // Clean up and switch to non sparse
            m_IsSparse = false;
            m_Values.clear();
            m_ValuesSparse.clear();
            m_BinsSparse.clear();
            
            unsigned long StreamSize = T.GetTokenAtAsLong(1);

            if (GetNBins() != StreamSize) {
              mout<<"MResponseMatrixON: The number of bins ("<<GetNBins()<<") and the stream size ("<<StreamSize<<") are not in sync!"<<endl;
              Ok = false;
              break;
            }
            m_Values.resize(StreamSize, 0);

            m_Order = 0;
            for (unsigned int a = 0; a < m_Axes.size(); ++a) {
              m_Order += m_Axes[a]->GetDimension();
            }


            bool StreamOk = true;
            float Data;
            unsigned long x, x_max = StreamSize;
            for (x = 0; x < x_max; ++x) {
              if (StreamSize == 0) {
                StreamOk = false;
                break;
              }
              if (Parser.GetFloat(Data) == true) {
                m_Values[x] = Data;
                StreamSize--;
              } else {
                StreamOk = false;
                break;
              }
            }

            if (StreamOk == false) {
              mout<<"MResponseMatrixON: Stream was not ok!"<<endl;
              Ok = false;
              break;
            }
          }
        } // secondary tokenizer loop
      } else if (Type == "ResponseMatrixON" || Type == "ResponseMatrixONSparse") {
        
        // Clean up and switch to sparse
        m_IsSparse = true;
        m_Values.clear();
        m_ValuesSparse.clear();
        m_BinsSparse.clear();
        
        // Keep this for debugging parallel mode
        bool Parallel = false;
        
        if (Parallel == false) {
          while (Parser.TokenizeLine(T, true) == true) {
            if (T.GetNTokens() != m_Axes.size() + 2) continue;
            if (T.GetTokenAt(0) == "RD") {
              vector<unsigned long> Bins(m_Axes.size(), 0);
              for (unsigned int b = 1; b < m_Axes.size() + 1; ++b) {
                Bins[b-1] = T.GetTokenAtAsUnsignedInt(b);
              }
              // For debugging the fully mode:
              // Set(Bins, T.GetTokenAtAsFloat(m_Axes.size() + 1));
              // Comment rest out if commenting the above in
              
              if (InRange(Bins) == false) {
                continue;
              }
              
              m_BinsSparse.push_back(FindBin(Bins));
              m_ValuesSparse.push_back(T.GetTokenAtAsFloat(m_Axes.size() + 1));
            }
          }
        } else {
          // Read all lines
          m_ThreadLines.clear();
          MString Line;
          while (Parser.ReadLine(Line) == true) {
            m_ThreadLines.push_back(Line);
          }
          
          // Create temporary data storage
          m_ThreadGoodData.clear();
          m_ThreadGoodData.resize(m_ThreadLines.size(), false);
          m_ThreadBins.clear();
          m_ThreadBins.resize(m_ThreadLines.size(), 0);
          m_ThreadValues.clear();
          m_ThreadValues.resize(m_ThreadLines.size(), 0);
          
          
          // Multi-threaded part:
          unsigned int NThreads = thread::hardware_concurrency();
          unsigned int NUsedThreads = 1;
          
          unsigned int Split = m_ThreadLines.size() / NThreads;

          if (Split == 0) {
            NUsedThreads = 1;
            Split = m_ThreadLines.size();
          } else {
            NUsedThreads = NThreads;
          }
          
          vector<pair<unsigned int, unsigned int>> LineApportionment;
          
          for (unsigned int i = 0; i < NUsedThreads; ++i) {
            unsigned int Start = 0;
            if (LineApportionment.size() != 0) {
              Start = LineApportionment.back().second + 1;
            }
            unsigned int Stop = Start + Split - 1;
            if (i == NUsedThreads - 1) {
              Stop = m_ThreadLines.size() - 1;
            }
            LineApportionment.push_back(pair<unsigned int, unsigned int>(Start, Stop));
          }
                  
          
          vector<thread> Threads(NUsedThreads);
          m_ThreadRunning.resize(NUsedThreads, true);
          for (unsigned int t = 0; t < NUsedThreads; ++t) {
            m_ThreadRunning[t] = true;
            Threads[t] = thread(&MResponseMatrixON::SparseReadThreadEntry, this, t, LineApportionment[t].first, LineApportionment[t].second);
          }
          while (true) {
            bool Finished = true;
            for (unsigned int t = 0; t < NUsedThreads; ++t) {
              if (m_ThreadRunning[t] == true) {
                Finished = false;
                break;
              }
            }
            if (Finished == false) {
              this_thread::sleep_for(chrono::milliseconds(1));
            } else {
              for (unsigned int t = 0; t < NUsedThreads; ++t) {
                Threads[t].join();
              }
              break;
            }
          }
          // Now merge the data - if the file has been created with this class it is guaranteed to be in the correct sequence.
          m_BinsSparse.reserve(m_ThreadBins.size());
          m_ValuesSparse.reserve(m_ThreadValues.size());
          for (unsigned int i = 0; i < m_ThreadGoodData.size(); ++i) {
            if (m_ThreadGoodData[i] == true) {
              m_BinsSparse.push_back(m_ThreadBins[i]);
              m_ValuesSparse.push_back(m_ThreadValues[i]);
            }
          }
        } // Multi-treaded read
      } // Stream or no stream
    } // Axis/Type loop
  } // main loop

  m_Order = 0;
  for (unsigned int a = 0; a < m_Axes.size(); ++a) {
    m_Order += m_Axes[a]->GetDimension();
  }

  cout<<"Time spent reading response: "<<Timer.GetElapsed()<<" seconds"<<endl;

  // Debugging: Save it
  //MTimer WriteTimer;
  //Write("DeleteMe.rsp.gz");
  //cout<<"Time to write: "<<WriteTimer.GetElapsed()<<" seconds"<<endl;
  
  return Ok;
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixON::SparseReadThreadEntry(unsigned int ThreadID, unsigned int Start, unsigned int Stop)
{
  for (unsigned int i = Start; i <= Stop; ++i) {
    MTokenizer T;
    T.AnalyzeFast(m_ThreadLines[i]);

    if (T.GetNTokens() != m_Axes.size() + 2) continue;
    if (T.GetTokenAt(0) == "RD") {
      vector<unsigned long> Bins(m_Axes.size(), 0);
      for (unsigned int b = 1; b < m_Axes.size() + 1; ++b) {
        Bins[b-1] = T.GetTokenAtAsUnsignedInt(b);
      }

      if (InRange(Bins) == false) {
        continue;
      }

      m_ThreadGoodData[i] = true;
      m_ThreadBins[i] = FindBin(Bins);
      m_ThreadValues[i] = T.GetTokenAtAsFloat(m_Axes.size() + 1);
    }
  }   

  m_ThreadMutex.lock();
  m_ThreadRunning[ThreadID] = false;
  m_ThreadMutex.unlock();
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrixON::Write(MString FileName, bool Stream)
{
  // Write the content to file


  MFileResponse File;
  File.SetCompressionLevel(5); // 5: best performance to compression ratio for sparse matrices
  if (File.Open(FileName, MFile::c_Write) == false) return false;

  MTimer Timer;
  mdebug<<"Started writting file \""<<FileName<<"\" ... This way take a while ..."<<endl;

  ostringstream s;
  s<<"# Response Matrix "<<m_Order<<endl;
  s<<"Version 1"<<endl;
  s<<endl;
  s<<"# Name"<<endl;
  s<<"NM "<<m_Name<<endl;
  s<<endl;
  s<<"# The order of the matrix"<<endl;
  s<<"OD "<<m_Order<<endl;
  s<<endl;
  s<<"# The number of simulated events"<<endl;
  s<<"TS "<<m_NumberOfSimulatedEvents<<endl;
  s<<endl;
  s<<"# The far-field start area (if zero a non-far-field simulation, or non-spherical start area was used)"<<endl;
  s<<"SA "<<m_FarFieldStartArea<<endl;
  s<<endl;
  s<<"# Are the values centered?"<<endl;
  s<<"CE false"<<endl;
  s<<endl;
  s<<"# Is the matrix sparse?"<<endl;
  s<<"SP "<<(m_IsSparse == true ? "true" : "false")<<endl;
  s<<endl;
  File.Write(s);

  s<<endl;
  for (unsigned int a = 0; a < m_Axes.size(); ++a) {
    m_Axes[a]->Write(s);
    File.Write(s);
  }
  s<<endl;
  File.Write(s);

  // Determine the sparcity:
  /*
  unsigned long Empty = 0;
  for (unsigned long i = 0; i < m_NumberOfBins; ++i) {
    if (m_Values[i] == 0) {
      Empty += 1;
    }
  }
  if (double(Empty)/GetNBins() > 0.9) {
    Stream = false;
  }
  */

  if (m_IsSparse == false) {
    s<<"Type ResponseMatrixONStream"<<endl;
    s<<endl;

    // Write content stream
    s<<"StartStream "<<m_NumberOfBins<<endl;
    File.Write(s);
    for (unsigned long i = 0; i < m_NumberOfBins; ++i) {
      if (m_Values[i] == 0) {
        File.Write("0 "); // Little speed up (x8 for sparse matrices)
      } else {
        File.Write(m_Values[i]);
      }
    }
    s<<endl;
    s<<"StopStream"<<endl;
    File.Write(s);
  } else {
    s<<"Type ResponseMatrixONSparse"<<endl;
    s<<endl;
    
    bool IsParallel = false;

    if (IsParallel == false) {
      for (unsigned long i = 0; i < m_BinsSparse.size(); ++i) {
        vector<unsigned long> Bins = FindBins(m_BinsSparse[i]);
        s<<"RD ";
        for (unsigned long b = 0; b < Bins.size(); ++b) {
          s<<Bins[b]<<" ";
        }
        s<<m_ValuesSparse[i]<<endl;;
        File.Write(s);
      }
    } else {
      /*
      unsigned int ParallelIndices = 1000;
      vector<MString> Strings(ParallelIndices);

      std::vector<int> Indices(ParallelIndices);
      std::generate(std::execution::par, Indices.begin(), Indices.end(), [n = 0] () mutable { return n++; });

      unsigned long Index = 0;
      do {
        if (Index + ParallelIndices >= m_BinsSparse.size()) {
          Indices.resize(m_BinsSparse.size() - Index);
          Strings.resize(m_BinsSparse.size() - Index);
        }
        std::for_each(std::execution::par, Indices.begin(), Indices.end(), [&](int i) {
          vector<unsigned long> Bins = FindBins(m_BinsSparse[i+Index]);
          ostringstream s;
          s<<"RD ";
          for (unsigned long b = 0; b < Bins.size(); ++b) {
            s<<Bins[b]<<" ";
          }
          s<<m_ValuesSparse[i+Index]<<endl;
          Strings[i+Index] = s.str();
        });
        
        for (auto S: Strings) {
          File.Write(S);
        }
      }
      */
    } // is parallel
  } // is parse

  File.Close();

  mout<<"File \""<<FileName<<"\" with "<<m_NumberOfBins<<" entries written in "<<Timer.ElapsedTime()<<" sec"<<endl;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixON::ShowSlice(vector<float> AxisValues, bool Normalize, MString Title)
{
  // Create a ROOT histogram:

  //gROOT->SetBatch(true);
  
  if (AxisValues.size() < m_Order) {
    merr<<"MResponseMatrixON::ShowSlice: You have to give as many axis values as there are axes!"<<endl;
    return;
  }

  bool ContainsShowX = false;
  for (unsigned long v = 0; v < AxisValues.size(); ++v) {
    if (AxisValues[v] == MResponseMatrix::c_ShowX) {
      ContainsShowX = true;
      break;
    }
  }
  bool ContainsShowY = false;
  for (unsigned long v = 0; v < AxisValues.size(); ++v) {
    if (AxisValues[v] == MResponseMatrix::c_ShowY) {
      ContainsShowY = true;
      break;
    }
  }
  bool ContainsShowZ = false;
  for (unsigned long v = 0; v < AxisValues.size(); ++v) {
    if (AxisValues[v] == MResponseMatrix::c_ShowZ) {
      ContainsShowZ = true;
      break;
    }
  }
  if (ContainsShowX == false && (ContainsShowY == true || ContainsShowZ == true)) {
    merr<<"MResponseMatrixON::ShowSlice: Cannot show y or z axis if x axis is not shown."<<endl;
    return;
  }
  if (ContainsShowY == false && ContainsShowZ == true) {
    merr<<"MResponseMatrixON::ShowSlice: Cannot show z axis if y axis is not shown."<<endl;
    return;
  }

  vector<float> ShowMatrix;
  if (ContainsShowX == true) ShowMatrix.push_back(MResponseMatrix::c_ShowX);
  if (ContainsShowY == true) ShowMatrix.push_back(MResponseMatrix::c_ShowY);
  if (ContainsShowZ == true) ShowMatrix.push_back(MResponseMatrix::c_ShowZ);


  vector<unsigned int> Order;
  vector<bool> found(m_Order, false);

  for (unsigned int s = 0; s < ShowMatrix.size(); ++s) {
    for (unsigned int a = 0; a < AxisValues.size(); ++a) {
      if (AxisValues[a] == ShowMatrix[s]) {
        Order.push_back(a+1); //
        found[a] = true;
      }
    }
  }
  //for (unsigned int i = 0; i < Order.size(); ++i) {
  //  cout<<"Order "<<i<<": "<<Order[i]<<endl;
  //}

  unsigned int NAxes = Order.size();

  for (unsigned int a = 0; a < AxisValues.size(); ++a) {
    if (found[a] == false) {
      Order.push_back(a+1);
      //values.push_back(m_Axes[a]->GetAxisBin(AxisValues[a]));
    }
  }

  /*
  mout<<"Mapping: "<<endl;
  for (unsigned int i = 0; i < Order.size(); ++i) {
    mout<<i<<": a="<<Order[i]<<" - m="<<values[i]<<endl;
  }
  */

  vector<MResponseMatrixAxis*> DrawnAxes;
  for (unsigned int a = 0; a < NAxes; ++a) {
    DrawnAxes.push_back(GetAxisByOrder(Order[a]));
    a += DrawnAxes.back()->GetDimension() - 1; // Jump if the axis has multiple dimensions
  }

  vector<vector<double>> BinEdges;
  for (unsigned int a = 0; a < DrawnAxes.size(); ++a) {
    vector<vector<double>> ProvidedBinEdges = DrawnAxes[a]->GetDrawingAxisBinEdges();
    for (unsigned int v = 0; v < ProvidedBinEdges.size(); ++v) {
      BinEdges.push_back(ProvidedBinEdges[v]);
    }
  }

  vector<MString> AxisNames;
  for (unsigned int a = 0; a < DrawnAxes.size(); ++a) {
    vector<MString> ProvidedAxisNames = DrawnAxes[a]->GetNames();
    for (unsigned int v = 0; v < ProvidedAxisNames.size(); ++v) {
      AxisNames.push_back(ProvidedAxisNames[v]);
    }
  }


  if (GetNBins() > 0) {


    double Value = 0.0;
    double Minimum = numeric_limits<double>::max();
    vector<double> Values = vector<double>(AxisValues.begin(), AxisValues.end());
    //cout<<"Order: "<<Values.size()<<endl;
    //for (unsigned int i = 0; i < Values.size(); ++i) {
    //  cout<<"Value "<<i<<": "<<Values[i]<<endl;
    //}

    if (Title == "") Title = m_Name;

    if (NAxes == 1) {

      TH1D* Hist = new TH1D("", Title, BinEdges[0].size() - 1, &BinEdges[0][0]);
      Hist->SetStats(true);
      Hist->SetContour(50);
      Hist->SetXTitle(AxisNames[0]);

      for (int bx = 1; bx <= Hist->GetNbinsX(); ++bx) {
        Values[Order[0]-1] = Hist->GetBinCenter(bx);
        Value = Get(Values);
        if (Value == 0) continue;
        if (Normalize == true) {
          Value /= GetArea(Values);
        }
        if (Value < Minimum) Minimum = Value;
        if (isfinite(Value) == true) {
          Hist->SetBinContent(bx, Value);
        } else {
          merr<<"Bin "<<bx<<" is not a finite number!"<<endl; 
        }
      }

      TCanvas* Canvas = new TCanvas();
      Canvas->cd();
      if (GetAxisByOrder(Order[0])->IsLogarithmic() == true) {
        Canvas->SetLogx();
      }
      Hist->SetMinimum(Minimum);
      Hist->Draw();
      Canvas->Update();

    } else if (NAxes == 2) {

      TH2D* Hist = new TH2D("", Title, BinEdges[0].size() - 1, &BinEdges[0][0], BinEdges[1].size() - 1, &BinEdges[1][0]);
      Hist->SetStats(true);
      Hist->SetContour(50);
      Hist->SetXTitle(AxisNames[0]);
      Hist->SetYTitle(AxisNames[1]);

      //double Norm = 1;
      for (int bx = 1; bx <= Hist->GetNbinsX(); ++bx) {
        for (int by = 1; by <= Hist->GetNbinsY(); ++by) {
          Values[Order[0]-1] = Hist->GetXaxis()->GetBinCenter(bx);
          Values[Order[1]-1] = Hist->GetYaxis()->GetBinCenter(by);
          //cout<<"Values: "; for (unsigned int i = 0; i < Values.size(); ++i) cout<<Values[i]<<" "; cout<<" ---> "<<Get(Values)<<" "<<GetArea(Values)<<endl;
          Value = Get(Values);
          if (Value == 0) continue;
          if (Normalize == true) {
            Value /= GetArea(Values);
          }
          if (Value < Minimum) Minimum = Value;
          if (isfinite(Value) == true) {
            Hist->SetBinContent(bx, by, Value);
          } else {
            merr<<"Bin "<<bx<<", "<<by<<" is not a finite number!"<<endl; 
          }
        }
      }

      TCanvas* Canvas = new TCanvas();
      Canvas->cd();
      if (GetAxisByOrder(Order[0])->IsLogarithmic() == true) {
        Canvas->SetLogx();
      }
      if (GetAxisByOrder(Order[1])->IsLogarithmic() == true) {
        Canvas->SetLogy();
      }
      Hist->SetMinimum(Minimum);
      Hist->Draw("colz");
      Canvas->Update();
      
      //Canvas->SaveAs("Test.C");
      
    } else if (NAxes == 3) {

      TH3D* Hist = new TH3D("", Title, BinEdges[0].size() - 1, &BinEdges[0][0], BinEdges[1].size() - 1, &BinEdges[1][0], BinEdges[2].size() - 1, &BinEdges[2][0]);
      Hist->SetStats(true);
      Hist->SetContour(50);
      Hist->SetXTitle(AxisNames[0]);
      Hist->SetYTitle(AxisNames[1]);
      Hist->SetYTitle(AxisNames[2]);

      //double Norm = 1;
      for (int bx = 1; bx <= Hist->GetNbinsX(); ++bx) {
        for (int by = 1; by <= Hist->GetNbinsY(); ++by) {
          for (int bz = 1; bz <= Hist->GetNbinsZ(); ++bz) {
            Values[Order[0]-1] = Hist->GetXaxis()->GetBinCenter(bx);
            Values[Order[1]-1] = Hist->GetYaxis()->GetBinCenter(by);
            Values[Order[2]-1] = Hist->GetZaxis()->GetBinCenter(bz);
            Value = Get(Values);
            if (Value == 0) continue;
            if (Normalize == true) {
              Value /= GetArea(Values);
            }
            if (Value < Minimum) Minimum = Value;
            if (isfinite(Value) == true) {
              Hist->SetBinContent(bx, by, bz, Value);
            } else {
              merr<<"Bin "<<bx<<", "<<by<<", "<<bz<<" is not a finite number!"<<endl; 
            }
          }
        }
      }

      TCanvas* Canvas = new TCanvas();
      Canvas->cd();
      if (GetAxisByOrder(Order[0])->IsLogarithmic() == true) {
        Canvas->SetLogx();
      }
      if (GetAxisByOrder(Order[1])->IsLogarithmic() == true) {
        Canvas->SetLogy();
      }
      if (GetAxisByOrder(Order[2])->IsLogarithmic() == true) {
        Canvas->SetLogz();
      }
      Hist->SetMinimum(Minimum);
      Hist->Draw();
      Canvas->Update();

    } else {
      merr<<"Wrong number of axis: "<<NAxes<<endl;
    }

  } else {
    mout<<"Empty response matrix of order 7"<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixON::Smooth(unsigned int Times)
{
  // We access the TH1 routines here...

  /*
  if (Times > 0 && GetNBins() > 0) {
    double* Values = new double[GetAxisBins()];
    for (unsigned int i = 0; i < GetAxisBins(); ++i) {
      Values[i] = m_Values[i];
    }

    TH1::SmoothArray(GetAxisBins(), Values, Times);

    for (unsigned int i = 0; i < GetAxisBins(); ++i) {
      SetBinContent(i, Values[i]);
    }

    delete [] Values;
  }
  */
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, const MResponseMatrixON& R)
{
  /*
  os<<"Response Matrix of order 1 with "<<R.m_AxisO1.size()-1<<" entries:"<<endl;
  for (unsigned int i = 0; i < R.m_AxisO1.size()-1; ++i) {
    os<<R.m_AxisO1[i]<<": "<<R.m_Values[i]<<endl;
  }
  */
  return os;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a string with statistics numbers
MString MResponseMatrixON::GetStatistics() const
{
  // Do the calculation here instead of calling the member functions to speed things up:
  unsigned long NBins = GetNBins();
  unsigned long NumberOfNonZeroBins = 0;
  double Sum = 0;
  float Min = +numeric_limits<float>::max();
  float Max = -numeric_limits<float>::max();
  
  if (m_IsSparse == false) {
    for (unsigned long i = 0; i < m_NumberOfBins; ++i) {
      Sum += m_Values[i];
      if (m_Values[i] > Max) {
        Max = m_Values[i];
      }
      if (m_Values[i] < Min) {
        Min = m_Values[i];
      }
      if (m_Values[i] != 0) {
        ++NumberOfNonZeroBins;
      }
    }
  } else {
    for (unsigned long i = 0; i < m_ValuesSparse.size(); ++i) {
      Sum += m_ValuesSparse[i];
      if (m_ValuesSparse[i] > Max) {
        Max = m_ValuesSparse[i];
      }
      if (m_ValuesSparse[i] < Min) {
        Min = m_ValuesSparse[i];
      }
      if (m_ValuesSparse[i] != 0) {
        ++NumberOfNonZeroBins;
      }
    }
  }
  
  ostringstream out;

  out<<endl;
  out<<"Statistics for response matrix \""<<m_Name<<"\":"<<endl;
  out<<endl;
  out<<"Number of axes:           "<<m_Axes.size()<<endl;
  out<<"Number of dimensions:     "<<m_Order<<endl;
  out<<"Number of bins:           "<<NBins<<endl;
  out<<"Number of non-zero bins:  "<<NumberOfNonZeroBins<<endl;
  out<<"Sparseness:               "<<100.0 - (100.0 * NumberOfNonZeroBins) / NBins<<" %"<<endl;
  out<<"Maximum:                  "<<Max<<endl;
  out<<"Minimum:                  "<<Min<<endl;
  out<<"Sum:                      "<<Sum<<endl;
  out<<"Average value:            "<<Sum/NBins<<endl;
  out<<endl;

  out<<"Axes:"<<endl;
  for (unsigned int a = 0; a < m_Axes.size(); ++a) {
    out<<"  x"<<a<<":  ";
    for (unsigned int i = 0; i < m_Axes[a]->GetNames().size(); ++i) {
      out<<m_Axes[a]->GetNames()[i]<<" (from "<<m_Axes[a]->GetMinima()[i]<<" to "<<m_Axes[a]->GetMaxima()[i]<<" with "<<m_Axes[a]->GetNumberOfBins()<<" bins)";
      if (m_Axes[a]->GetNames().size() > 1 && i < m_Axes[a]->GetNames().size() -1) out<<"  +  ";
    }
    out<<endl;
  }

  return out.str();
}



// MResponseMatrixON.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
