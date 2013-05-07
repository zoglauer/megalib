/*
 * MResponseMatrixO5.cxx
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
// MResponseMatrixO5
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseMatrixO5.h"

// Standard libs:
#include <iostream>
#include <functional>
#include <algorithm>
#include <limits>
using namespace std;

// ROOT libs:
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TCanvas.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MFileResponse.h"
#include "MTimer.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseMatrixO5)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO5::MResponseMatrixO5() :
  MResponseMatrix()
{
  // default constructor

  Init();
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO5::MResponseMatrixO5(vector<float> x1Axis, vector<float> x2Axis, 
                                     vector<float> x3Axis, vector<float> x4Axis, 
                                     vector<float> x5Axis) :
  MResponseMatrix()
{
  // constructor

  Init();
  SetAxis(x1Axis, x2Axis, x3Axis, x4Axis, x5Axis);
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO5::MResponseMatrixO5(MString Name, vector<float> x1Axis, 
                                     vector<float> x2Axis, vector<float> x3Axis, 
                                     vector<float> x4Axis, vector<float> x5Axis) :
  MResponseMatrix(Name)
{
  // constructor

  Init();
  SetAxis(x1Axis, x2Axis, x3Axis, x4Axis, x5Axis);
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO5::~MResponseMatrixO5()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO5::Init()
{
  // Initializations common to all constructors

  m_Order = 5;
  m_AxisO5.clear();
  m_AxesO4.clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrixO5::operator==(const MResponseMatrixO5& R)
{
  // Two matrixes are identical if they have the same axis:

  if (m_AxisO5!= R.m_AxisO5) {
    return false;
  }

  for (unsigned int i = 0; i < m_AxesO4.size(); ++i) {
    if (!(m_AxesO4[i] == R.m_AxesO4[i])) {
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO5& MResponseMatrixO5::operator+=(const MResponseMatrixO5& R)
{
  // Append a matrix to this one

  if (*this == R) {
    for (unsigned int i = 0; i < m_AxesO4.size(); ++i) {
      m_AxesO4[i] += R.m_AxesO4[i];
    }
  } else {
    merr<<"Matrices have different axes!"<<endl;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO5& MResponseMatrixO5::operator-=(const MResponseMatrixO5& R)
{
  // Append a matrix to this one

  if (*this == R) {
    for (unsigned int i = 0; i < m_AxesO4.size(); ++i) {
      m_AxesO4[i] -= R.m_AxesO4[i];
    }
  } else {
    merr<<"Matrices have different axes!"<<endl;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO5& MResponseMatrixO5::operator/=(const MResponseMatrixO5& R)
{
  // Append a matrix to this one

  if (*this == R) {
    for (unsigned int i = 0; i < m_AxesO4.size(); ++i) {
      m_AxesO4[i] /= R.m_AxesO4[i];
    }
  } else {
    merr<<"Matrices have different axes!"<<endl;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO5& MResponseMatrixO5::operator+=(const float& Value)
{
  // Append a matrix to this one

  for (unsigned int i = 0; i < m_AxesO4.size(); ++i) {
    m_AxesO4[i] += Value;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO5& MResponseMatrixO5::operator*=(const float& Value)
{
  // Append a matrix to this one

  for (unsigned int i = 0; i < m_AxesO4.size(); ++i) {
    m_AxesO4[i] *= Value;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO5::SetAxis(vector<float> x1Axis, vector<float> x2Axis, 
                                vector<float> x3Axis, vector<float> x4Axis, 
                                vector<float> x5Axis)
{
  // Set the axes

  if (x5Axis.size() == 0) {
    merr<<"Size of axis 5 is zero!"<<endl;
    massert(false);
    return;    
  }

  if (IsIncreasing(x5Axis) == false) {
    merr<<"Axes 5 is not in increasing order!"<<endl;
    massert(false);
    return;
  }

  if ((x1Axis.size()-1)*(x2Axis.size()-1)*(x3Axis.size()-1)*
      (x4Axis.size()-1)*(x5Axis.size()-1) > c_SizeLimit) {
    merr<<"The total number of bins is larger than the limit of "
        <<c_SizeLimit<<endl;
    massert(false);
    return;
  }

  m_AxisO5 = x5Axis;

  m_AxesO4.resize(m_AxisO5.size()-1);
  for (unsigned int b = 0; b < m_AxisO5.size()-1; ++b) {
    m_AxesO4[b].SetAxis(x1Axis, x2Axis, x3Axis, x4Axis);
    m_AxesO4[b].SetName(m_Name);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO5::SetAxisNames(MString x1Name, MString x2Name, 
                                     MString x3Name, MString x4Name, 
                                     MString x5Name)
{
  // Set the names of the axes:

  m_NameAxisO5 = x5Name;
  for (unsigned int i = 0; i < m_AxesO4.size(); ++i) {
    m_AxesO4[i].SetAxisNames(x1Name, x2Name, x3Name, x4Name);
  }
}


////////////////////////////////////////////////////////////////////////////////


MString MResponseMatrixO5::GetAxisName(unsigned int order) const
{
  // Return the name of the axis

  massert(order >= 1 && order <= 5);

  if (order == 5) {
    return m_NameAxisO5;
  } else {
    massert(m_AxisO5.size() > 0);
    return m_AxesO4[0].GetAxisName(order);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO5::SetBinContent(unsigned int x1, unsigned int x2, 
                                      unsigned int x3, unsigned int x4, 
                                      unsigned int x5, float Value)
{
  // Set the content of the bin

  massert(x5 < m_AxisO5.size()-1);
  
  m_AxesO4[x5].SetBinContent(x1, x2, x3, x4, Value);
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO5::Add(float x1, float x2, float x3, float x4, 
                            float x5, float Value)
{
  /// Add a value to the bin closest to x, y, z

  // Get Position:
  int Position = -1; 
  for (unsigned int i = 0; i < m_AxisO5.size(); ++i) {
    if (m_AxisO5[i] > x5) {
      break;
    } 
    Position = (int) i;
  }
  
  if (Position <= -1) {
    mdebug<<"Underflow in "<<m_Name<<": ("<<x1<<", "<<x2<<", "<<x3<<", "<<x4<<", "<<x5<<") = "<<Value<<endl;
    return;
  }
  if (Position >= (int) m_AxisO5.size()-1) {
    mdebug<<"Overflow in "<<m_Name<<": ("<<x1<<", "<<x2<<", "<<x3<<", "<<x4<<", "<<x5<<") = "<<Value<<endl;
    return;
  }

  m_AxesO4[Position].Add(x1, x2, x3, x4, Value);
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO5::SetMatrix(unsigned int b, MResponseMatrixO4 R)
{
  // Set a whole sub matrix

  massert(b < m_AxisO5.size());
  m_AxesO4[b] = R;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MResponseMatrixO5::GetNBins() const
{
  // Return the number of bins

  if (m_AxisO5.size() < 2) return 0;
  unsigned int Bins = m_AxisO5.size()-1;
  return Bins*m_AxesO4[0].GetNBins();
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MResponseMatrixO5::GetAxisBins(unsigned int order) const
{
  // Return the number of bins for the main axis O5

  massert(order >= 1 && order <= 5);

  if (order == 5) {
    return m_AxisO5.size()-1;
  } else {
    massert(m_AxisO5.size() > 0);
    return m_AxesO4[0].GetAxisBins(order);
  }
}


////////////////////////////////////////////////////////////////////////////////


vector<float> MResponseMatrixO5::GetAxis(unsigned int order) const
{
  // Return the axis content at bin b

  massert(order >= 1 && order <= 5);

  if (order == 5) {
    return m_AxisO5;
  } else {
    massert(GetNBins() > 0);
    return m_AxesO4[0].GetAxis(order);
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO5::GetAxisContent(unsigned int b, unsigned int order) const
{
  // Return the axis content at bin b

  massert(order >= 1 && order <= 5);

  if (order == 5) {
    massert(b < m_AxisO5.size());
    return m_AxisO5[b];
  } else {
    massert(GetNBins() > 0);
    return m_AxesO4[0].GetAxisContent(b, order);
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO5::GetBinArea(unsigned int x1, unsigned int x2, 
                                    unsigned int x3, unsigned int x4, 
                                    unsigned int x5) const
{
  // Return the area of this bin

  massert(x5 < m_AxisO5.size()-1);

  return (m_AxisO5[x5+1]-m_AxisO5[x5])*m_AxesO4[x5].GetBinArea(x1, x2, x3, x4);
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO5::GetAxisBinCenter(unsigned int b, unsigned int order) const
{
  // Return the center of axis bin b

  massert(order >= 1 && order <= 5);

  if (order == 5) {
    massert(b < m_AxisO5.size()-1);
    return 0.5*(m_AxisO5[b]+m_AxisO5[b+1]);
  } else {
    massert(GetNBins() > 0);
    return m_AxesO4[0].GetAxisBinCenter(b, order);
  }
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MResponseMatrixO5::GetAxisBin(float x, unsigned int order) const
{
  // Return the center of axis bin b

  massert(order >= 1 && order <= 5);

  // Get Position:
  if (order == 5) {
    unsigned int Position = c_Outside; 
    for (unsigned int i = 0; i < m_AxisO5.size(); ++i) {
      if (m_AxisO5[i] > x) {
        break;
      } 
      Position = i;
    }
    
    if (Position >= m_AxisO5.size()-1) {
      Position = c_Outside;
    }
    return Position;
  } else {
    massert(GetNBins() > 0);
    return m_AxesO4[0].GetAxisBin(x, order);
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO5::GetAxisMinimum(unsigned int order) const
{
   // Return the lower edge of the lowest bin of the axis 

  massert(order >= 1 && order <= 5);

  if (order == 5) {
    massert(m_AxisO5.size() > 0);
    return m_AxisO5[0];
  } else {
    massert(GetNBins() > 0);
    return m_AxesO4[0].GetAxisMinimum(order);
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO5::GetAxisMaximum(unsigned int order) const
{
   // Return the upper edge of the highest bin of the axis 

  massert(order >= 1 && order <= 5);

  if (order == 5) {
    massert(m_AxisO5.size() > 0);
    return m_AxisO5[m_AxisO5.size()-1];
  } else {
    massert(GetNBins() > 0);
    return m_AxesO4[0].GetAxisMaximum(order);
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO5::GetAxisLowEdge(unsigned int b, unsigned int order) const
{
   // Return the lower edge of bin b 

  massert(order >= 1 && order <= 5);

  if (order == 5) {
    massert(b < m_AxisO5.size()-1);
    return m_AxisO5[b];
  } else {
    massert(GetNBins() > 0);
    return m_AxesO4[0].GetAxisLowEdge(b, order);
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO5::GetAxisHighEdge(unsigned int b, unsigned int order) const
{
   // Return the high edge of bin b 

  massert(order >= 1 && order <= 5);

  if (order == 5) {
    massert(b < m_AxisO5.size()-1);
    return m_AxisO5[b+1];
  } else {
    massert(GetNBins() > 0);
    return m_AxesO4[0].GetAxisHighEdge(b, order);
  } 
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO5::GetBinContent(unsigned int x1, unsigned int x2, 
                                       unsigned int x3, unsigned int x4, 
                                       unsigned int x5) const
{
  // Return the content of bin x, y

  massert(GetNBins() > 0);
  massert(x1 < GetAxisBins(1));
  massert(x2 < GetAxisBins(2));
  massert(x3 < GetAxisBins(3));
  massert(x4 < GetAxisBins(4));
  massert(x5 < GetAxisBins(5));

  return m_AxesO4[x5].GetBinContent(x1, x2, x3, x4);
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO5::GetBinContent(unsigned int x1, unsigned int x1axis, 
                                       unsigned int x2, unsigned int x2axis,
                                       unsigned int x3, unsigned int x3axis,
                                       unsigned int x4, unsigned int x4axis,
                                       unsigned int x5, unsigned int x5axis) const
{
  // 

  massert(GetNBins() > 0);
  massert(x1 < GetAxisBins(x1axis));
  massert(x2 < GetAxisBins(x2axis));
  massert(x3 < GetAxisBins(x3axis));
  massert(x4 < GetAxisBins(x4axis));
  massert(x5 < GetAxisBins(x5axis));
  
  if (x1axis == 5) {
    return m_AxesO4[x1].GetBinContent(x2, x2axis, x3, x3axis, x4, x4axis, x5, x5axis);
  } else if (x2axis == 5) {
    return m_AxesO4[x2].GetBinContent(x1, x1axis, x3, x3axis, x4, x4axis, x5, x5axis);
  } else if (x3axis == 5) {
    return m_AxesO4[x3].GetBinContent(x1, x1axis, x2, x2axis, x4, x4axis, x5, x5axis);
  } else if (x4axis == 5) {
    return m_AxesO4[x4].GetBinContent(x1, x1axis, x2, x2axis, x3, x3axis, x5, x5axis);
  } else if (x5axis == 5) {
    return m_AxesO4[x5].GetBinContent(x1, x1axis, x2, x2axis, x3, x3axis, x4, x4axis);
  } else {
    merr<<"Axes are not correctly defined: x1-axis="
        <<x1axis<<" x2-axis="<<x2axis<<" x3-axis="<<x3axis
        <<" x4-axis="<<x4axis<<" x5-axis="<<x5axis<<endl;
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO5::GetInterpolated(float x1, float x2, float x3, float x4, 
                                          float x5, bool DoExtrapolate) const
{
  // Return the array-data according to value x1, x2, x3, x4, x5

  // We have to distinguish to different cases:
  // (1) the values are at the center of the bin (e.g. used by response files)
  // (2) the values are the edge (axis point) (e.g. used by MEGAlib absorption files) 

  // We can assume that the bin size is larger equal 2

  // The linear interpolation goes as follows:
  // y = m*x + t
  // m = (y2-y1)/(x2-x1)
  // t = y1 - m*x1
  // y = y1 + (x-x1)*m

  if (m_ValuesCentered == true) { 
    if (m_AxisO5.size() == 2) {
      return m_AxesO4.front().GetInterpolated(x1, x2, x3, x4, DoExtrapolate);
    } else {
      // Get Position:
      int Position = FindBinCentered(m_AxisO5, x5);

      // Take care of boundaries:
      if (Position < 0) {
        if (DoExtrapolate == true) {
          Position = 0; // extrapolate below lower edge
        } else {
          return m_AxesO4.front().GetInterpolated(x1, x2, x3, x4, DoExtrapolate);
        }
      } else if (Position >= int(m_AxesO4.size()-1)) {
        if (DoExtrapolate == true) {
          Position = int(m_AxesO4.size()-2); // extrapolate above higher edge
          // limits of highest CENTERED bin are m_AxesO4.size()-2 and m_AxesO4.size()-1 !!
        } else {
          return m_AxesO4.back().GetInterpolated(x1, x2, x3, x4, DoExtrapolate);
        }
      }

      // Interpolate:
      return m_AxesO4[Position].GetInterpolated(x1, x2, x3, x4, DoExtrapolate) + (x5 - GetAxisBinCenter(Position))/
          (GetAxisBinCenter(Position+1) - GetAxisBinCenter(Position))*
          (m_AxesO4[Position+1].GetInterpolated(x1, x2, x3, x4, DoExtrapolate) - m_AxesO4[Position].GetInterpolated(x1, x2, x3, x4, DoExtrapolate));
    }
  } else {
    // Get Position:
 		int Position = FindBin(m_AxisO5, x5);

    // Take care of boundaries:
		if (Position < 0) {
			if (DoExtrapolate == true) {
				Position = 0; // extrapolate below lower edge
			} else {
				return m_AxesO4.front().GetInterpolated(x1, x2, x3, x4, DoExtrapolate);
			}
		} else if (Position >= int(m_AxisO5.size()-1)) {
			if (DoExtrapolate == true) {
				Position = int(m_AxisO5.size()-2); // extrapolate above higher edge
				// limits of highest bin are m_AxisO5.size()-2 and  m_AxisO5.size()-1 !!
			} else {
				return m_AxesO4.back().GetInterpolated(x1, x2, x3, x4, DoExtrapolate);
			}
		}
    
    // Interpolate:
    return m_AxesO4[Position].GetInterpolated(x1, x2, x3, x4, DoExtrapolate) + (x5 - m_AxisO5[Position])/
      (m_AxisO5[Position+1] - m_AxisO5[Position])*
      (m_AxesO4[Position+1].GetInterpolated(x1, x2, x3, x4, DoExtrapolate) - m_AxesO4[Position].GetInterpolated(x1, x2, x3, x4, DoExtrapolate));
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO5::Get(float x1, float x2, float x3, float x4, float x5) const
{
  // Return the array-data according to value x 

  if (m_AxisO5.size() == 0) {
    return 0;
  } 

  int Position = FindBin(m_AxisO5, x5);

  if (Position < 0) {
    Position = 0;
  } else if (Position >= int(GetAxisBins())) {
    Position = GetAxisBins()-1;
  }

  return m_AxesO4[Position].Get(x1, x2, x3, x4);
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO5::GetMaximum() const
{
  // Return the maximum

  float LocMax = -numeric_limits<float>::max();
  float Max = -numeric_limits<float>::max();
  for (unsigned int i = 0; i < m_AxesO4.size(); ++i) {
    LocMax = m_AxesO4[i].GetMaximum();
    if (LocMax > Max) {
      Max = LocMax;
    }
  }  

  return Max;
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO5::GetMinimum() const
{
  // Return the minimum

  float LocMin = numeric_limits<float>::max();
  float Min = numeric_limits<float>::max();
  for (unsigned int i = 0; i < m_AxesO4.size(); ++i) {
    LocMin = m_AxesO4[i].GetMinimum();
    if (LocMin < Min) {
      Min = LocMin;
    }
  }  

  return Min;
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO5::GetSum() const
{
  // Return the sum of all bins:

  float Sum = 0;
  for (unsigned int i = 0; i < m_AxesO4.size(); ++i) {
    Sum += m_AxesO4[i].GetSum();
  }  

  return Sum;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO1 MResponseMatrixO5::GetSumMatrixO1(unsigned int a1) const
{
  // Return the sum of the matrix as function of dimension order
  // Attention: Very inefficient algorithm!

  massert(AreIncreasing(5, a1));

  MResponseMatrixO1 R;
  R.SetAxis(GetAxis(a1));

  if (a1 == 5) {
    for (unsigned int i = 0; i < m_AxesO4.size(); ++i) {
      R.SetBinContent(i, m_AxesO4[i].GetSum());
    }
  } else {
    for (unsigned int i = 0; i < m_AxesO4.size(); ++i) {
      R += m_AxesO4[i].GetSumMatrixO1(a1);
    }
  }
  
  return R;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO2 MResponseMatrixO5::GetSumMatrixO2(unsigned int a1, 
                                                    unsigned int a2) const
{
  // Return the sum of the matrix as function of dimension order
  // Attention: Simple, but very inefficient algorithm!

  massert(AreIncreasing(5, a1, a2));

  MResponseMatrixO2 R;
  R.SetAxis(GetAxis(a1), GetAxis(a2));
  
  if (a2 == 5) {
    for (unsigned int i = 0; i < m_AxesO4.size(); ++i) {
      R.SetMatrix(i, m_AxesO4[i].GetSumMatrixO1(a1));
    }
  } else {
    for (unsigned int i = 0; i < m_AxesO4.size(); ++i) {
      R += m_AxesO4[i].GetSumMatrixO2(a1, a2);
    }
  }

  return R;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO3 MResponseMatrixO5::GetSumMatrixO3(unsigned int a1, 
                                                    unsigned int a2,
                                                    unsigned int a3) const
{
  // Return the sum of the matrix as function of dimension order
  // Attention: Simple, but very inefficient algorithm!

  massert(AreIncreasing(5, a1, a2, a3));

  MResponseMatrixO3 R;
  R.SetAxis(GetAxis(a1), GetAxis(a2), GetAxis(a3));
  
  if (a3 == 5) {
    for (unsigned int i = 0; i < m_AxesO4.size(); ++i) {
      R.SetMatrix(i, m_AxesO4[i].GetSumMatrixO2(a1, a2));
    }
  } else {
    for (unsigned int i = 0; i < m_AxesO4.size(); ++i) {
      R += m_AxesO4[i].GetSumMatrixO3(a1, a2, a3);
    }
  }

  return R;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO4 MResponseMatrixO5::GetSumMatrixO4(unsigned int a1, 
                                                    unsigned int a2,
                                                    unsigned int a3,
                                                    unsigned int a4) const
{
  // Return the sum of the matrix as function of dimension order
  // Attention: Simple, but very inefficient algorithm!

  massert(AreIncreasing(5, a1, a2, a3, a4));

  MResponseMatrixO4 R;
  R.SetAxis(GetAxis(a1), GetAxis(a2), GetAxis(a3), GetAxis(a4));
  
  if (a4 == 5) {
    for (unsigned int i = 0; i < m_AxesO4.size(); ++i) {
      R.SetMatrix(i, m_AxesO4[i].GetSumMatrixO3(a1, a2, a3));
    }
  } else {
    for (unsigned int i = 0; i < m_AxesO4.size(); ++i) {
      R += m_AxesO4[i].GetSumMatrixO4(a1, a2, a3, a4);
    }
  }

  return R;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO5 MResponseMatrixO5::GetSumMatrixO5(unsigned int a1, 
                                                    unsigned int a2, 
                                                    unsigned int a3, 
                                                    unsigned int a4, 
                                                    unsigned int a5) const
{
  // Return the sum of the matrix as function of dimension order
  // Attention: Simple, but very inefficient algorithm!

  massert(AreIncreasing(5, a1, a2, a3, a4, a5));

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrixO5::ReadSpecific(MFileResponse& Parser, 
                                     const MString& Type, 
                                     const int Version)
{
  // Read the data from file directly into this matrix

  bool Ok = true;
  MTokenizer T;

  if (Type == "ResponseMatrixO5") {
//     while (Parser.TokenizeLine(T) == true) {
//       if (T.GetNTokens() == 0) continue;
//       if (T.GetTokenAt(0) == "R2") {
//         if (T.GetNTokens() == 4) {
//           Set(T.GetTokenAtAsFloat(1), T.GetTokenAtAsFloat(2), T.GetTokenAtAsFloat(3));
//         } else {
//           mout<<"MResponseMatrixO5: Wrong number of arguments for token R2!"<<endl;
//           Ok = false;
//           break;
//         }
//       }
//     }
  } else if (Type == "ResponseMatrixO5Stream") {
    vector<float> x1Axis;
    vector<float> x2Axis;
    vector<float> x3Axis;
    vector<float> x4Axis;
    vector<float> x5Axis;
    MString x1Name;
    MString x2Name;
    MString x3Name;
    MString x4Name;
    MString x5Name;
    while (Parser.TokenizeLine(T) == true) {
      if (T.GetNTokens() == 0) continue;
      if (T.GetTokenAt(0) == "A1") {
        x1Axis = T.GetTokenAtAsFloatVector(1);
      } else if (T.GetTokenAt(0) == "A2") {
        x2Axis = T.GetTokenAtAsFloatVector(1);
      } else if (T.GetTokenAt(0) == "A3") {
        x3Axis = T.GetTokenAtAsFloatVector(1);
      } else if (T.GetTokenAt(0) == "A4") {
        x4Axis = T.GetTokenAtAsFloatVector(1);
      } else if (T.GetTokenAt(0) == "A5") {
        x5Axis = T.GetTokenAtAsFloatVector(1);
      } else if (T.GetTokenAt(0) == "N1") {
        x1Name = T.GetTokenAfterAsString(1);
      } else if (T.GetTokenAt(0) == "N2") {
        x2Name = T.GetTokenAfterAsString(1);
      } else if (T.GetTokenAt(0) == "N3") {
        x3Name = T.GetTokenAfterAsString(1);
      } else if (T.GetTokenAt(0) == "N4") {
        x4Name = T.GetTokenAfterAsString(1);
      } else if (T.GetTokenAt(0) == "N5") {
        x5Name = T.GetTokenAfterAsString(1);
      } else if (T.GetTokenAt(0) == "StartStream") {
        unsigned int StreamSize = T.GetTokenAtAsInt(1);

        if ((x1Axis.size()-1)*(x2Axis.size()-1)*(x3Axis.size()-1)*
            (x4Axis.size()-1)*(x5Axis.size()-1) != StreamSize) {
          mout<<"MResponseMatrixO5: Axis sizes and stream size are not in sync!"<<endl;
          Ok = false;
          break;
        }
        SetAxis(x1Axis, x2Axis, x3Axis, x4Axis, x5Axis);
        SetAxisNames(x1Name, x2Name, x3Name, x4Name, x5Name);

        bool StreamOk = true;
        float Data;
        unsigned int x1, x1_max = GetAxisBins(1); 
        unsigned int x2, x2_max = GetAxisBins(2); 
        unsigned int x3, x3_max = GetAxisBins(3); 
        unsigned int x4, x4_max = GetAxisBins(4); 
        unsigned int x5, x5_max = GetAxisBins(5); 
        for (x5 = 0; x5 < x5_max; ++x5) {
          for (x4 = 0; x4 < x4_max; ++x4) {
            for (x3 = 0; x3 < x3_max; ++x3) {
              for (x2 = 0; x2 < x2_max; ++x2) {
                for (x1 = 0; x1 < x1_max; ++x1) {
                  if (StreamSize == 0) {
                    StreamOk = false;
                    break;
                  }
                  if (Parser.GetFloat(Data) == true) {
                    SetBinContent(x1, x2, x3, x4, x5, Data);
                    StreamSize--;
                  } else {
                    StreamOk = false;
                    break;
                  }
                }
                
                if (StreamOk == false) {
                  mout<<"MResponseMatrixO5: Stream was not ok!"<<endl;
                  Ok = false;
                  break;
                }
              }
            }
          }
        }
      }
    }
  }  else {
    mout<<"MResponseMatrixO5: Wrong file type: "<<Type<<endl;
    Ok = false;    
  }

  return Ok;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrixO5::Write(MString FileName, bool Stream)
{
  // Write the content to file

  massert(GetNBins() > 0);

  MFileResponse File;
  if (File.Open(FileName, MFile::c_Write) == false) return false;

  MTimer Timer;
  mdebug<<"Started writting file \""<<FileName<<"\" ... This way take a while ..."<<endl;

  ostringstream s;
  s<<"# Response Matrix 5"<<endl;
  s<<"Version 1"<<endl;
  s<<endl;
  s<<"NM "<<m_Name<<endl;
  s<<endl;
  s<<"CE "<<((m_ValuesCentered == true) ? "true" : "false")<<endl;
  File.Write(s);
  
  unsigned int x1, x1_max = GetAxisBins(1); 
  unsigned int x2, x2_max = GetAxisBins(2); 
  unsigned int x3, x3_max = GetAxisBins(3); 
  unsigned int x4, x4_max = GetAxisBins(4); 
  unsigned int x5, x5_max = GetAxisBins(5); 

  if (Stream == false) {
    s<<"Type ResponseMatrixO5"<<endl;
    for (x5 = 0; x5 < x5_max; ++x5) {
      for (x4 = 0; x4 < x4_max; ++x4) {
        for (x3 = 0; x3 < x3_max; ++x3) {
          for (x2 = 0; x2 < x2_max; ++x2) {
            for (x1 = 0; x1 < x1_max; ++x1) {
              s<<"R5 "<<GetAxisContent(x1, 1)<<" "<<GetAxisContent(x2, 2)<<" "
               <<GetAxisContent(x3, 3)<<" "<<GetAxisContent(x4, 4)<<" "<<GetAxisContent(x5, 5)
               <<" "<<GetBinContent(x1, x2, x3, x4, x5)<<endl;
              File.Write(s);
            }
          }
        }
      }
    }
  } else {
    s<<"Type ResponseMatrixO5Stream"<<endl;
    // Write x1-axis
    s<<"A1 ";
    File.Write(s);
    for (x1 = 0; x1 <= x1_max; ++x1) {
      File.Write(GetAxisContent(x1, 1));
    }
    s<<endl;
    s<<"N1 "<<GetAxisName(1)<<endl;
    File.Write(s);
    // Write x2-axis
    s<<"A2 ";
    File.Write(s);
    for (x2 = 0; x2 <= x2_max; ++x2) {
      File.Write(GetAxisContent(x2, 2));
    }
    s<<endl;
    s<<"N2 "<<GetAxisName(2)<<endl;
    File.Write(s);
    // Write x3-axis
    s<<"A3 ";
    File.Write(s);
    for (x3 = 0; x3 <= x3_max; ++x3) {
      File.Write(GetAxisContent(x3, 3));
    }
    s<<endl;
    s<<"N3 "<<GetAxisName(3)<<endl;
    File.Write(s);
    // Write x4-axis
    s<<"A4 ";
    File.Write(s);
    for (x4 = 0; x4 <= x4_max; ++x4) {
      File.Write(GetAxisContent(x4, 4));
    }
    s<<endl;
    s<<"N4 "<<GetAxisName(4)<<endl;
    File.Write(s);
    // Write x5-axis
    s<<"A5 ";
    File.Write(s);
    for (x5 = 0; x5 <= x5_max; ++x5) {
      File.Write(GetAxisContent(x5, 5));
    }
    s<<endl;
    s<<"N5 "<<GetAxisName(5)<<endl;
    File.Write(s);

    // Write content stream
    s<<"StartStream "<<GetNBins()<<endl;
    File.Write(s);
    for (x5 = 0; x5 < x5_max; ++x5) {
      for (x4 = 0; x4 < x4_max; ++x4) {
        for (x3 = 0; x3 < x3_max; ++x3) {
          for (x2 = 0; x2 < x2_max; ++x2) {
            for (x1 = 0; x1 < x1_max; ++x1) {
              File.Write(GetBinContent(x1, x2, x3, x4, x5));
            }
          }
        }
      }
    }
    s<<endl;
    s<<"StopStream"<<endl;
    File.Write(s);
  }
  
  mdebug<<"File \""<<FileName<<"\" with "<<x1_max*x2_max*x3_max*x4_max*x5_max
        <<" entries written in "<<Timer.ElapsedTime()<<" sec"<<endl;
  File.Close();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO5::Show(float x1, float x2, float x3, float x4, 
                             float x5, bool Normalize)
{
  // Create a 3d ROOT histogram:
  
  vector<unsigned int> axes;
  vector<unsigned int> values;
  vector<bool> found(5, false);

  vector<float> ShowMatrix;
  ShowMatrix.push_back(MResponseMatrix::c_ShowX);
  ShowMatrix.push_back(MResponseMatrix::c_ShowY);
  ShowMatrix.push_back(MResponseMatrix::c_ShowZ);


  for (unsigned int s = 0; s < ShowMatrix.size(); ++s) {
    if (x1 == ShowMatrix[s]) {
      axes.push_back(1);
      values.push_back(GetAxisBin(x1, 1));
      found[0] = true;
    } else if (x2 == ShowMatrix[s]) {
      axes.push_back(2);
      values.push_back(GetAxisBin(x2, 2));
      found[1] = true;
    } else if (x3 == ShowMatrix[s]) {
      axes.push_back(3);
      values.push_back(GetAxisBin(x3, 3));
      found[2] = true;
    } else if (x4 == ShowMatrix[s]) {
      axes.push_back(4);
      values.push_back(GetAxisBin(x4, 4));
      found[3] = true;
    } else if (x5 == ShowMatrix[s]) {
      axes.push_back(5);
      values.push_back(GetAxisBin(x5, 5));
      found[4] = true;
    }
    if (axes.size() == s) break;
  }

  unsigned int NAxes = axes.size();

  if (found[0] == false) {
    axes.push_back(1);
    values.push_back(GetAxisBin(x1, 1));    
  }
  if (found[1] == false) {
    axes.push_back(2);
    values.push_back(GetAxisBin(x2, 2));    
  }
  if (found[2] == false) {
    axes.push_back(3);
    values.push_back(GetAxisBin(x3, 3));    
  }
  if (found[3] == false) {
    axes.push_back(4);
    values.push_back(GetAxisBin(x4, 4));    
  }
  if (found[4] == false) {
    axes.push_back(5);
    values.push_back(GetAxisBin(x5, 5));    
  }

  merr<<"No error checks implemented!"<<endl;
  // e.g. overflow for values...

  mout<<"Mapping: "<<endl;
  for (unsigned int i = 0; i < axes.size(); ++i) {
    mout<<i<<": a="<<axes[i]<<" - m="<<values[i]<<endl;
  }




  if (GetNBins() > 0) {

    if (NAxes == 1) {

      TH1D* Hist = 0;
      float* Bins = new float[GetAxisBins(axes[0])+1];
      for (unsigned int i = 0; i <= GetAxisBins(axes[0]); ++i) {
        Bins[i] = GetAxisContent(i, axes[0]);
      }
      Hist = new TH1D(m_Name + "_RM5", m_Name + "_RM5", GetAxisBins(axes[0]), Bins);
      Hist->SetStats(true);
      Hist->SetContour(50);
      Hist->SetXTitle(GetAxisName(axes[0]));
      delete [] Bins;
    
      mout<<"Response matrix of order 5 with "<<GetNBins()<<" entries:"<<endl;
      double Norm = 1;
      for (unsigned int i1 = 0; i1 < GetAxisBins(axes[0]); ++i1) {
        if (Normalize == true) {
          Norm = GetAxisContent(i1+1, axes[0])-GetAxisContent(i1, axes[0]);
          if (Norm != 0) {
            Norm = 1.0/Norm;
          } else {
            Norm = 1.0;
          }
        }
        Hist->SetBinContent(i1+1, GetBinContent(i1, axes[0], 
                                                values[1], axes[1], 
                                                values[2], axes[2], 
                                                values[3], axes[3], 
                                                values[4], axes[4])*Norm);
      }
    
      TCanvas* Canvas = new TCanvas(m_Name + "_RM5C", m_Name + "_RM5C", 0, 0, 600, 600);
      Canvas->cd();
      Hist->Draw();
      Canvas->Update();

    } else if (NAxes == 2) {
      TH2D* Hist = 0;
      float* xBins = new float[GetAxisBins(axes[0])+1];
      float* yBins = new float[GetAxisBins(axes[1])+1];
      for (unsigned int i = 0; i <= GetAxisBins(axes[0]); ++i) {
        xBins[i] = GetAxisContent(i, axes[0]);
      }
      for (unsigned int i = 0; i <= GetAxisBins(axes[1]); ++i) {
        yBins[i] = GetAxisContent(i, axes[1]);
      }
      Hist = new TH2D(m_Name+"_RM5", m_Name+"_RM5", GetAxisBins(axes[0]), xBins, GetAxisBins(axes[1]), yBins);
      Hist->SetStats(true);
      Hist->SetContour(50);
      Hist->SetXTitle(GetAxisName(axes[0]));
      Hist->SetYTitle(GetAxisName(axes[1]));
      delete [] xBins;
      delete [] yBins;
      
      mout<<"Response matrix of order 5 with "<<GetNBins()<<" entries:"<<endl;
      double Norm = 1;
      for (unsigned int i2 = 0; i2 < GetAxisBins(axes[1]); ++i2) {
        for (unsigned int i1 = 0; i1 < GetAxisBins(axes[0]); ++i1) {
          if (Normalize == true) {
            Norm = (GetAxisContent(i1+1, axes[0])-GetAxisContent(i1, axes[0]))*
              (GetAxisContent(i2+1, axes[1])-GetAxisContent(i2, axes[1]));
            if (Norm != 0) {
              Norm = 1.0/Norm;
            } else {
              Norm = 1.0;
            }
          }
          Hist->SetBinContent(i1+1, i2+1, GetBinContent(i1, axes[0], 
                                                        i2, axes[1], 
                                                        values[2], axes[2], 
                                                        values[3], axes[3], 
                                                        values[4], axes[4])*Norm);
        }
      }
      
      TCanvas* Canvas = new TCanvas(m_Name+"_RM5C", m_Name+"_RM5C", 0, 0, 600, 600);
      Canvas->cd();
      Hist->Draw("colz");
      Canvas->Update();
    } else if (NAxes == 3) {
      TH3D* Hist = 0;
      float* x1Bins = new float[GetAxisBins(axes[0])+1];
      float* x2Bins = new float[GetAxisBins(axes[1])+1];
      float* x3Bins = new float[GetAxisBins(axes[2])+1];
      for (unsigned int i = 0; i <= GetAxisBins(axes[0]); ++i) {
        x1Bins[i] = GetAxisContent(i, axes[0]);
      }
      for (unsigned int i = 0; i <= GetAxisBins(axes[1]); ++i) {
        x2Bins[i] = GetAxisContent(i, axes[1]);
      }
      for (unsigned int i = 0; i <= GetAxisBins(axes[2]); ++i) {
        x3Bins[i] = GetAxisContent(i, axes[2]);
      }
      Hist = new TH3D(m_Name+"_RM5", m_Name+"_RM5", 
                      GetAxisBins(axes[0]), x1Bins, 
                      GetAxisBins(axes[1]), x2Bins, 
                      GetAxisBins(axes[2]), x3Bins);
      Hist->SetStats(true);
      Hist->SetContour(50);
      Hist->SetXTitle(GetAxisName(axes[0]));
      Hist->SetYTitle(GetAxisName(axes[1]));
      Hist->SetZTitle(GetAxisName(axes[2]));
      delete [] x1Bins;
      delete [] x2Bins;
      delete [] x3Bins;
      
      mout<<"Response matrix of order 5 with "<<GetNBins()<<" entries:"<<endl;
      double Norm = 1;
      for (unsigned int i3 = 0; i3 < GetAxisBins(axes[2]); ++i3) {
        for (unsigned int i2 = 0; i2 < GetAxisBins(axes[1]); ++i2) {
          for (unsigned int i1 = 0; i1 < GetAxisBins(axes[0]); ++i1) {
            if (Normalize == true) {
              Norm = (GetAxisContent(i1+1, axes[0])-GetAxisContent(i1, axes[0]))*
                (GetAxisContent(i2+1, axes[1])-GetAxisContent(i2, axes[1]))*
                (GetAxisContent(i3+1, axes[2])-GetAxisContent(i3, axes[2]));
              if (Norm != 0) {
                Norm = 1.0/Norm;
              } else {
                Norm = 1.0;
              }
            }            
            Hist->SetBinContent(i1+1, i2+1, i3+1, GetBinContent(i1, axes[0], 
                                                                i2, axes[1], 
                                                                i3, axes[2], 
                                                                values[3], axes[3], 
                                                                values[4], axes[4])*Norm);
          }
        }
      }

      TCanvas* Canvas = new TCanvas(m_Name + "_RM5C", m_Name + "_RM5C", 0, 0, 600, 600);
      Canvas->cd();
      Hist->Draw("box");
      Canvas->Update();

    } else {
      merr<<"Wrong number of axis: "<<NAxes<<endl;
    }

  } else {
    mout<<"Empty response matrix of order 5"<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, const MResponseMatrixO5& R)
{
  if (R.GetNBins() > 0) {
    os<<"Response matrix of order 5 with "<<R.GetNBins()<<" entries:"<<endl;
    for (unsigned int i5 = 0; i5 < R.GetAxisBins(5); ++i5) {
      for (unsigned int i4 = 0; i4 < R.GetAxisBins(4); ++i4) {
        for (unsigned int i3 = 0; i3 < R.GetAxisBins(3); ++i3) {
          for (unsigned int i2 = 0; i2 < R.GetAxisBins(2); ++i2) {
            for (unsigned int i1 = 0; i1 < R.GetAxisBins(1); ++i1) {
              os<<"x1/x2/x3/x4/x5: "<<i1<<"/"<<i2<<"/"<<i3<<"/"<<i4<<"/"<<i5
                <<"  - cc: "<<R.GetAxisBinCenter(i1, 1)
                <<"/"<<R.GetAxisBinCenter(i2, 2)
                <<"/"<<R.GetAxisBinCenter(i3, 3)
                <<"/"<<R.GetAxisBinCenter(i4, 4)
                <<"/"<<R.GetAxisBinCenter(i5, 5)
                <<"  - value : "<<R.GetBinContent(i1, i2, i3, i4, i5)<<endl;
            }
          }
        }
      }
    }
  } else {
    os<<"Empty response matrix of order 5"<<endl;
  }
  return os;
}



// MResponseMatrixO5.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
