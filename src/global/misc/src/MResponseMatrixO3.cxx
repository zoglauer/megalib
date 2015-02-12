/*
 * MResponseMatrixO3.cxx
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
// MResponseMatrixO3
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseMatrixO3.h"

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
ClassImp(MResponseMatrixO3)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO3::MResponseMatrixO3() :
  MResponseMatrix()
{
  // default constructor

  Init();
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO3::MResponseMatrixO3(vector<float> x1Axis, vector<float> x2Axis, 
                                     vector<float> x3Axis) :
  MResponseMatrix()
{
  // constructor

  Init();
  SetAxis(x1Axis, x2Axis, x3Axis);
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO3::MResponseMatrixO3(MString Name, vector<float> x1Axis, 
                                     vector<float> x2Axis, vector<float> x3Axis) :
  MResponseMatrix(Name)
{
  // constructor

  Init();
  SetAxis(x1Axis, x2Axis, x3Axis);
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO3::~MResponseMatrixO3()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO3::Init()
{
  // Initializations common to all constructors

  m_Order = 3;
  m_AxisO3.clear();
  m_AxesO2.clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrixO3::operator==(const MResponseMatrixO3& R)
{
  // Two matrixes are identical if they have the same axis:

  if (m_AxisO3 != R.m_AxisO3) {
    return false;
  }

  for (unsigned int i = 0; i < m_AxesO2.size(); ++i) {
    if (!(m_AxesO2[i] == R.m_AxesO2[i])) {
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO3& MResponseMatrixO3::operator+=(const MResponseMatrixO3& R)
{
  // Append a matrix to this one

  if (*this == R) {
    for (unsigned int i = 0; i < m_AxesO2.size(); ++i) {
      m_AxesO2[i] += R.m_AxesO2[i];
    }
  } else {
    merr<<"Matrices have different axes!"<<endl;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO3& MResponseMatrixO3::operator-=(const MResponseMatrixO3& R)
{
  // Append a matrix to this one

  if (*this == R) {
    for (unsigned int i = 0; i < m_AxesO2.size(); ++i) {
      m_AxesO2[i] -= R.m_AxesO2[i];
    }
  } else {
    merr<<"Matrices have different axes!"<<endl;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO3& MResponseMatrixO3::operator/=(const MResponseMatrixO3& R)
{
  // Append a matrix to this one

  if (*this == R) {
    for (unsigned int i = 0; i < m_AxesO2.size(); ++i) {
      m_AxesO2[i] /= R.m_AxesO2[i];
    }
  } else {
    merr<<"Matrices have different axes!"<<endl;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO3& MResponseMatrixO3::operator+=(const float& Value)
{
  // Append a matrix to this one

  for (unsigned int i = 0; i < m_AxesO2.size(); ++i) {
    m_AxesO2[i] += Value;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO3& MResponseMatrixO3::operator*=(const float& Value)
{
  // Append a matrix to this one

  for (unsigned int i = 0; i < m_AxesO2.size(); ++i) {
    m_AxesO2[i] *= Value;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO3::SetAxis(vector<float> x1Axis, vector<float> x2Axis, 
                                vector<float> x3Axis)
{
  // Set the axes

  if (x3Axis.size() == 0) {
    merr<<"Size of axis 3 is zero!"<<endl;
    massert(false);
    return;    
  }

  if (IsIncreasing(x3Axis) == false) {
    merr<<"Axes 3 is not in increasing order!"<<endl;
    massert(false);
    return;
  }

  if ((x1Axis.size()-1)*(x2Axis.size()-1)*(x3Axis.size()-1) > c_SizeLimit) {
    merr<<"The total number of bins is larger than the limit of "
        <<c_SizeLimit<<endl;
    massert(false);
    return;
  }

  m_AxisO3 = x3Axis;

  m_AxesO2.resize(m_AxisO3.size()-1);
  for (unsigned int b = 0; b < m_AxisO3.size()-1; ++b) {
    m_AxesO2[b].SetAxis(x1Axis, x2Axis);
    m_AxesO2[b].SetName(m_Name);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO3::SetAxisNames(MString x1Name, MString x2Name, 
                                     MString x3Name)
{
  // Set the names of the axes:

  m_NameAxisO3 = x3Name;
  for (unsigned int i = 0; i < m_AxesO2.size(); ++i) {
    m_AxesO2[i].SetAxisNames(x1Name, x2Name);
  }
}


////////////////////////////////////////////////////////////////////////////////


MString MResponseMatrixO3::GetAxisName(unsigned int order) const
{
  // Return the name of the axis

  massert(order >= 1 && order <= 3);

  if (order == 3) {
    return m_NameAxisO3;
  } else {
    massert(m_AxisO3.size() > 0);
    return m_AxesO2[0].GetAxisName(order);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO3::SetBinContent(unsigned int x1, unsigned int x2, 
                                      unsigned int x3, float Value)
{
  // Set the content of the bin

  massert(x3 < m_AxisO3.size()-1);
  
  m_AxesO2[x3].SetBinContent(x1, x2, Value);
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO3::Add(float x1, float x2, float x3, float Value)
{
  /// Add a value to the bin closest to x, y, z

  // Get Position:
  int Position = -1; 
  for (unsigned int i = 0; i < m_AxisO3.size(); ++i) {
    if (m_AxisO3[i] > x3) {
      break;
    } 
    Position = (int) i;
  }
  
  if (Position <= -1) {
    mdebug<<"Underflow in "<<m_Name<<": ("<<x1<<", "<<x2<<", "<<x3<<") = "<<Value<<endl;
    return;
  }
  if (Position >= (int) m_AxisO3.size()-1) {
    mdebug<<"Overflow in "<<m_Name<<": ("<<x1<<", "<<x2<<", "<<x3<<") = "<<Value<<endl;
    return;
  }

  m_AxesO2[Position].Add(x1, x2, Value);
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO3::SetMatrix(unsigned int b, MResponseMatrixO2 R)
{
  // Set a whole sub matrix

  massert(b < m_AxisO3.size());
  m_AxesO2[b] = R;
}


////////////////////////////////////////////////////////////////////////////////


unsigned long MResponseMatrixO3::GetNBins() const
{
  // Return the number of bins

  if (m_AxisO3.size() < 2) return 0;
  unsigned long Bins = m_AxisO3.size()-1;
  return Bins*m_AxesO2[0].GetNBins();
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MResponseMatrixO3::GetAxisBins(unsigned int order) const
{
  // Return the number of bins for the main axis O3

  massert(order >= 1 && order <= 3);

  if (order == 3) {
    return m_AxisO3.size()-1;
  } else {
    massert(m_AxisO3.size() > 0);
    return m_AxesO2[0].GetAxisBins(order);
  }
}


////////////////////////////////////////////////////////////////////////////////


vector<float> MResponseMatrixO3::GetAxis(unsigned int order) const
{
  // Return the axis content at bin b

  massert(order >= 1 && order <= 3);

  if (order == 3) {
    return m_AxisO3;
  } else {
    massert(GetNBins() > 0);
    return m_AxesO2[0].GetAxis(order);
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO3::GetAxisContent(unsigned int b, unsigned int order) const
{
  // Return the axis content at bin b

  massert(order >= 1 && order <= 3);

  if (order == 3) {
    massert(b < m_AxisO3.size());
    return m_AxisO3[b];
  } else {
    massert(GetNBins() > 0);
    return m_AxesO2[0].GetAxisContent(b, order);
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO3::GetAxisBinCenter(unsigned int b, unsigned int order) const
{
  // Return the center of axis bin b

  massert(order >= 1 && order <= 3);

  if (order == 3) {
    massert(b < m_AxisO3.size()-1);
    return 0.5*(m_AxisO3[b]+m_AxisO3[b+1]);
  } else {
    massert(GetNBins() > 0);
    return m_AxesO2[0].GetAxisBinCenter(b, order);
  }
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MResponseMatrixO3::GetAxisBin(float x, unsigned int order) const
{
  // Return the center of axis bin b

  massert(order >= 1 && order <= 3);

  // Get Position:
  if (order == 3) {
    unsigned int Position = c_Outside; 
    for (unsigned int i = 0; i < m_AxisO3.size(); ++i) {
      if (m_AxisO3[i] > x) {
        break;
      } 
      Position = i;
    }
    
    if (Position >= m_AxisO3.size()-1) {
      Position = c_Outside;
    }
    return Position;
  } else {
    massert(GetNBins() > 0);
    return m_AxesO2[0].GetAxisBin(x, order);
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO3::GetAxisMinimum(unsigned int order) const
{
   // Return the lower edge of the lowest bin of the axis 

  massert(order >= 1 && order <= 3);

  if (order == 3) {
    massert(m_AxisO3.size() > 0);
    return m_AxisO3[0];
  } else {
    massert(GetNBins() > 0);
    return m_AxesO2[0].GetAxisMinimum(order);
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO3::GetAxisMaximum(unsigned int order) const
{
   // Return the upper edge of the highest bin of the axis 

  massert(order >= 1 && order <= 3);

  if (order == 3) {
    massert(m_AxisO3.size() > 0);
    return m_AxisO3[m_AxisO3.size()-1];
  } else {
    massert(GetNBins() > 0);
    return m_AxesO2[0].GetAxisMaximum(order);
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO3::GetAxisLowEdge(unsigned int b, unsigned int order) const
{
   // Return the lower edge of bin b 

  massert(order >= 1 && order <= 3);

  if (order == 3) {
    massert(b < m_AxisO3.size()-1);
    return m_AxisO3[b];
  } else {
    massert(GetNBins() > 0);
    return m_AxesO2[0].GetAxisLowEdge(b, order);
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO3::GetAxisHighEdge(unsigned int b, unsigned int order) const
{
   // Return the high edge of bin b 

  massert(order >= 1 && order <= 3);

  if (order == 3) {
    massert(b < m_AxisO3.size()-1);
    return m_AxisO3[b+1];
  } else {
    massert(GetNBins() > 0);
    return m_AxesO2[0].GetAxisHighEdge(b, order);
  } 
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO3::GetBinContent(unsigned int x1, unsigned int x2, unsigned int x3) const
{
  // Return the content of bin x, y

  massert(GetNBins() > 0);
  massert(x1 < GetAxisBins(1));
  massert(x2 < GetAxisBins(2));
  massert(x3 < GetAxisBins(3));

  return m_AxesO2[x3].GetBinContent(x1, x2);
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO3::GetBinContent(unsigned int x1, unsigned int x1axis, 
                                       unsigned int x2, unsigned int x2axis,
                                       unsigned int x3, unsigned int x3axis) const
{
  // 

  massert(GetNBins() > 0);
  massert(x1 < GetAxisBins(x1axis));
  massert(x2 < GetAxisBins(x2axis));
  massert(x3 < GetAxisBins(x3axis));
  
  if (x1axis == 3) {
    return m_AxesO2[x1].GetBinContent(x2, x2axis, x3, x3axis);
  } else if (x2axis == 3) {
    return m_AxesO2[x2].GetBinContent(x1, x1axis, x3, x3axis);
  } else if (x3axis == 3) {
    return m_AxesO2[x3].GetBinContent(x2, x2axis, x1, x1axis);
  } else {
    merr<<"Axes are not correctly defined: x1-axis="
        <<x1axis<<" x2-axis="<<x2axis<<" x3-axis="<<x3axis<<endl;
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO3::GetBinArea(unsigned int x1, unsigned int x2, 
                                    unsigned int x3) const
{
  // Return the area of this bin

  massert(x3 < m_AxisO3.size()-1);

  return (m_AxisO3[x3+1]-m_AxisO3[x3])*m_AxesO2[x3].GetBinArea(x1, x2);
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO3::GetInterpolated(float x1, float x2, float x3, bool DoExtrapolate) const
{
  // Return the array-data according to value x1, x2, x3

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
    if (m_AxisO3.size() == 2) {
      return m_AxesO2.front().GetInterpolated(x1, x2, DoExtrapolate);
    } else {
      // Get Position:
      int Position = FindBinCentered(m_AxisO3, x3);

      // Take care of boundaries:
      if (Position < 0) {
        if (DoExtrapolate == true) {
          Position = 0; // extrapolate below lower edge
        } else {
          return m_AxesO2.front().GetInterpolated(x1, x2, DoExtrapolate);
        }
      } else if (Position >= int(m_AxesO2.size()-1)) {
        if (DoExtrapolate == true) {
          Position = int(m_AxesO2.size()-2); // extrapolate above higher edge
          // limits of highest CENTERED bin are m_AxesO2.size()-2 and m_AxesO2.size()-1 !!
        } else {
          return m_AxesO2.back().GetInterpolated(x1, x2, DoExtrapolate);
        }
      }

      // Interpolate:
      return m_AxesO2[Position].GetInterpolated(x1, x2, DoExtrapolate) + (x3 - GetAxisBinCenter(Position))/
          (GetAxisBinCenter(Position+1) - GetAxisBinCenter(Position))*
          (m_AxesO2[Position+1].GetInterpolated(x1, x2, DoExtrapolate) - m_AxesO2[Position].GetInterpolated(x1, x2, DoExtrapolate));
    }
  } else {
    // Get Position:
 		int Position = FindBin(m_AxisO3, x3);

    // Take care of boundaries:
		if (Position < 0) {
			if (DoExtrapolate == true) {
				Position = 0; // extrapolate below lower edge
			} else {
				return m_AxesO2.front().GetInterpolated(x1, x2, DoExtrapolate);
			}
		} else if (Position >= int(m_AxisO3.size()-1)) {
			if (DoExtrapolate == true) {
				Position = int(m_AxisO3.size()-2); // extrapolate above higher edge
				// limits of highest bin are m_AxisO3.size()-2 and  m_AxisO3.size()-1 !!
			} else {
				return m_AxesO2.back().GetInterpolated(x1, x2, DoExtrapolate);
			}
		}
    
    // Interpolate:
    return m_AxesO2[Position].GetInterpolated(x1, x2, DoExtrapolate) + (x3 - m_AxisO3[Position])/
      (m_AxisO3[Position+1] - m_AxisO3[Position])*
      (m_AxesO2[Position+1].GetInterpolated(x1, x2, DoExtrapolate) - m_AxesO2[Position].GetInterpolated(x1, x2, DoExtrapolate));
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO3::Get(float x1, float x2, float x3) const
{
  // Return the array-data according to value x 

  if (m_AxisO3.size() == 0) {
    return 0;
  } 

  int Position = FindBin(m_AxisO3, x3); 

  if (Position < 0) {
    Position = 0;
  } else if (Position >= int(GetAxisBins())) {
    Position = GetAxisBins()-1;
  }

  return m_AxesO2[Position].Get(x1, x2);
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO3::GetMaximum() const
{
  // Return the maximum

  float LocMax = -numeric_limits<float>::max();
  float Max = -numeric_limits<float>::max();
  for (unsigned int i = 0; i < m_AxesO2.size(); ++i) {
    LocMax = m_AxesO2[i].GetMaximum();
    if (LocMax > Max) {
      Max = LocMax;
    }
  }  

  return Max;
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO3::GetMinimum() const
{
  // Return the minimum

  float LocMin = numeric_limits<float>::max();
  float Min = numeric_limits<float>::max();
  for (unsigned int i = 0; i < m_AxesO2.size(); ++i) {
    LocMin = m_AxesO2[i].GetMinimum();
    if (LocMin < Min) {
      Min = LocMin;
    }
  }  

  return Min;
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO3::GetSum() const
{
  // Return the sum of all bins:

  float Sum = 0;
  for (unsigned int i = 0; i < m_AxesO2.size(); ++i) {
    Sum += m_AxesO2[i].GetSum();
  }  

  return Sum;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO1 MResponseMatrixO3::GetSumMatrixO1(unsigned int a1) const
{
  // Return the sum of the matrix as function of dimension order
  // Attention: Very inefficient algorithm!

  massert(AreIncreasing(3, a1));

  MResponseMatrixO1 R;
  R.SetAxis(GetAxis(a1));

  if (a1 == 3) {
    for (unsigned int i = 0; i < m_AxesO2.size(); ++i) {
      R.SetBinContent(i, m_AxesO2[i].GetSum());
    }
  } else {
    for (unsigned int i = 0; i < m_AxesO2.size(); ++i) {
      R += m_AxesO2[i].GetSumMatrixO1(a1);
    }
  }
  
  return R;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO2 MResponseMatrixO3::GetSumMatrixO2(unsigned int a1, 
                                                    unsigned int a2) const
{
  // Return the sum of the matrix as function of dimension order
  // Attention: Simple, but very inefficient algorithm!

  massert(AreIncreasing(3, a1, a2));

  MResponseMatrixO2 R;
  R.SetAxis(GetAxis(a1), GetAxis(a2));
  
  if (a2 == 3) {
    for (unsigned int i = 0; i < m_AxesO2.size(); ++i) {
      R.SetMatrix(i, m_AxesO2[i].GetSumMatrixO1(a1));
    }
  } else {
    for (unsigned int i = 0; i < m_AxesO2.size(); ++i) {
      R += m_AxesO2[i].GetSumMatrixO2(a1, a2);
    }
  }

  return R;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO3 MResponseMatrixO3::GetSumMatrixO3(unsigned int a1, 
                                                    unsigned int a2, 
                                                    unsigned int a3) const
{
  // Return the sum of the matrix as function of dimension order
  // Attention: Simple, but very inefficient algorithm!

  massert(AreIncreasing(3, a1, a2, a3));

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO1 MResponseMatrixO3::GetSliceInterpolated(float x1, unsigned int order1,
                                                          float x2, unsigned int order2)
{
  // Return a slice of order 1 in axis a1 of this matrix

  massert(order1 >= 1 && order1 <= 3);
  massert(order2 >= 1 && order2 <= 3);
  //massert(x1 >= GetAxisContent(0, order1) && x1 <= GetAxisContent(GetAxisBins(order1)-1, order1));
  //massert(x2 >= GetAxisContent(0, order2) && x2 <= GetAxisContent(GetAxisBins(order2)-1, order2));

  unsigned int newaxis = 0;
  vector<float> values;
  for (unsigned int a = 1; a <= m_Order; ++a) {
    if (a == order1) {
      values.push_back(x1);
    } else if (a == order2) {
      values.push_back(x2);
    } else {
      values.push_back(0);
      newaxis = a;
    }
  }

  MResponseMatrixO1 M(GetAxis(newaxis));
  
  for (unsigned int i1 = 0; i1 < GetAxisBins(newaxis); ++i1) {
    values[newaxis-1] = GetAxisBinCenter(i1, newaxis);
    M.SetBinContent(i1, GetInterpolated(values[0], values[1], values[2]));
  }

  return M;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrixO3::ReadSpecific(MFileResponse& Parser, 
                                     const MString& Type, 
                                     const int Version)
{
  // Read the data from file directly into this matrix

  bool Ok = true;
  MTokenizer T;

  if (Type == "ResponseMatrixO3") {
//     while (Parser.TokenizeLine(T) == true) {
//       if (T.GetNTokens() == 0) continue;
//       if (T.GetTokenAt(0) == "R2") {
//         if (T.GetNTokens() == 4) {
//           Set(T.GetTokenAtAsFloat(1), T.GetTokenAtAsFloat(2), T.GetTokenAtAsFloat(3));
//         } else {
//           mout<<"MResponseMatrixO3: Wrong number of arguments for token R2!"<<endl;
//           Ok = false;
//           break;
//         }
//       }
//     }
  } else if (Type == "ResponseMatrixO3Stream") {
    vector<float> x1Axis;
    vector<float> x2Axis;
    vector<float> x3Axis;
    MString x1Name;
    MString x2Name;
    MString x3Name;
    while (Parser.TokenizeLine(T) == true) {
      if (T.GetNTokens() == 0) continue;
      if (T.GetTokenAt(0) == "A1") {
        x1Axis = T.GetTokenAtAsFloatVector(1);
      } else if (T.GetTokenAt(0) == "A2") {
        x2Axis = T.GetTokenAtAsFloatVector(1);
      } else if (T.GetTokenAt(0) == "A3") {
        x3Axis = T.GetTokenAtAsFloatVector(1);
      } else if (T.GetTokenAt(0) == "N1") {
        x1Name = T.GetTokenAfterAsString(1);
      } else if (T.GetTokenAt(0) == "N2") {
        x2Name = T.GetTokenAfterAsString(1);
      } else if (T.GetTokenAt(0) == "N3") {
        x3Name = T.GetTokenAfterAsString(1);
      } else if (T.GetTokenAt(0) == "StartStream") {
        unsigned int StreamSize = T.GetTokenAtAsInt(1);

        if ((x1Axis.size()-1)*(x2Axis.size()-1)*(x3Axis.size()-1) != StreamSize) {
          mout<<"MResponseMatrixO3: Axis sizes and stream size are not in sync!"<<endl;
          Ok = false;
          break;
        }
        SetAxis(x1Axis, x2Axis, x3Axis);
        SetAxisNames(x1Name, x2Name, x3Name);

        bool StreamOk = true;
        float Data;
        unsigned int x1, x1_max = GetAxisBins(1); 
        unsigned int x2, x2_max = GetAxisBins(2); 
        unsigned int x3, x3_max = GetAxisBins(3); 
        for (x3 = 0; x3 < x3_max; ++x3) {
          for (x2 = 0; x2 < x2_max; ++x2) {
            for (x1 = 0; x1 < x1_max; ++x1) {
              if (StreamSize == 0) {
                StreamOk = false;
                break;
              }
              if (Parser.GetFloat(Data) == true) {
                SetBinContent(x1, x2, x3, Data);
                StreamSize--;
              } else {
                StreamOk = false;
                break;
              }
            }
          
            if (StreamOk == false) {
              mout<<"MResponseMatrixO3: Stream was not ok!"<<endl;
              Ok = false;
              break;
            }
          }
        }
      }
    }
  } else {
    mout<<"MResponseMatrixO3: Wrong file type: "<<Type<<endl;
    Ok = false;    
  }

  return Ok;
}




////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrixO3::Write(MString FileName, bool Stream)
{
  // Write the content to file

  massert(GetNBins() > 0);

  MFileResponse File;
  if (File.Open(FileName, MFile::c_Write) == false) return false;

  MTimer Timer;
  mdebug<<"Started writting file \""<<FileName<<"\" ... This way take a while ..."<<endl;

  ostringstream s;
  s<<"# Response Matrix 3"<<endl;
  s<<"Version 1"<<endl;
  s<<endl;
  s<<"NM "<<m_Name<<endl;
  s<<endl;
  s<<"CE "<<((m_ValuesCentered == true) ? "true" : "false")<<endl;
  File.Write(s);
  
  unsigned int x1, x1_max = GetAxisBins(1); 
  unsigned int x2, x2_max = GetAxisBins(2); 
  unsigned int x3, x3_max = GetAxisBins(3); 

  if (Stream == false) {
    s<<"Type ResponseMatrixO3"<<endl;
    for (x3 = 0; x3 < x3_max; ++x3) {
      for (x2 = 0; x2 < x2_max; ++x2) {
        for (x1 = 0; x1 < x1_max; ++x1) {
          s<<"R3 "<<GetAxisContent(x1, 1)<<" "<<GetAxisContent(x2, 2)<<" "<<
            GetAxisContent(x3, 3)<<" "<<GetBinContent(x1, x2, x3)<<endl;
          File.Write(s);
        }
      }
    }
  } else {
    s<<"Type ResponseMatrixO3Stream"<<endl;
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

    // Write content stream
    s<<"StartStream "<<GetNBins()<<endl;
    File.Write(s);
    for (x3 = 0; x3 < x3_max; ++x3) {
      for (x2 = 0; x2 < x2_max; ++x2) {
        for (x1 = 0; x1 < x1_max; ++x1) {
          File.Write(GetBinContent(x1, x2, x3));
        }
      }
    }
    s<<endl;
    s<<"StopStream"<<endl;
    File.Write(s);
  }
  
  mdebug<<"File \""<<FileName<<"\" with "<<x1_max*x2_max*x3_max
        <<" entries written in "<<Timer.ElapsedTime()<<" sec"<<endl;
  File.Close();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO3::Show(float x1, float x2, float x3, bool Normalize)
{
  // Create a ROOT histogram:
  
  vector<unsigned int> axes;
  vector<unsigned int> values;
  vector<bool> found(3, false);

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
      Hist = new TH1D(m_Name + "_RM3D1", m_Name + "_RM3D1", GetAxisBins(axes[0]), Bins);
      Hist->SetStats(true);
      Hist->SetContour(50);
      Hist->SetXTitle(GetAxisName(axes[0]));
      delete [] Bins;
    
      mout<<"Response matrix of order 3 with "<<GetNBins()<<" entries:"<<endl;
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
                                                values[2], axes[2])*Norm);
      }
    
      TCanvas* Canvas = new TCanvas(m_Name + "_RM3D1C", m_Name + "_RM3D1C", 0, 0, 600, 600);
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
      Hist = new TH2D(m_Name+"_RM3D2", m_Name+"_RM3D2", GetAxisBins(axes[0]), xBins, GetAxisBins(axes[1]), yBins);
      Hist->SetStats(true);
      Hist->SetContour(50);
      Hist->SetXTitle(GetAxisName(axes[0]));
      Hist->SetYTitle(GetAxisName(axes[1]));
      delete [] xBins;
      delete [] yBins;
      
      mout<<"Response matrix of order 3 with "<<GetNBins()<<" entries:"<<endl;
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
                                                        values[2], axes[2])*Norm);
        }
      }
      
      TCanvas* Canvas = new TCanvas(m_Name+"_RM3D2C", m_Name+"_RM3D2C", 0, 0, 600, 600);
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
      Hist = new TH3D(m_Name+"_RM3D3", m_Name+"_RM3D3", 
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
      
      mout<<"Response matrix of order 3 with "<<GetNBins()<<" entries:"<<endl;
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
            Hist->SetBinContent(i1+1, i2+1, i3+1, GetBinContent(i1, i2, i3)*Norm);
          }
        }
      }

      TCanvas* Canvas = new TCanvas(m_Name + "_RM3D3C", m_Name + "_RM3D3C", 0, 0, 600, 600);
      Canvas->cd();
      Hist->Draw("box");
      Canvas->Update();

    } else {
      merr<<"Wrong number of axis: "<<NAxes<<endl;
    }

  } else {
    mout<<"Empty response matrix of order 3"<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO3::Smooth(unsigned int Times)
{
  // We can only smooth the lowest 1D histograms...

  if (Times > 0) {
    double* Values = new double[GetAxisBins(2)];
    for (unsigned int x1 = 0; x1 < GetAxisBins(1); ++x1) {
      for (unsigned int x2 = 0; x2 < GetAxisBins(2); ++x2) {
        for (unsigned int x3 = 0; x3 < GetAxisBins(3); ++x3) {
          Values[x3] = GetBinContent(x1, x2, x3);
        }
        
        TH1::SmoothArray(GetAxisBins(3), Values, Times);
        
        for (unsigned int x3 = 0; x3 < GetAxisBins(3); ++x3) {
          SetBinContent(x1, x2, x3, Values[x3]);
        }
      }
    }
    delete [] Values;

    for (unsigned int x1 = 0; x1 < m_AxesO2.size(); ++x1) {
      m_AxesO2[x1].Smooth(Times);
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, const MResponseMatrixO3& R)
{
  if (R.GetNBins() > 0) {
    os<<"Response matrix of order 3 with "<<R.GetNBins()<<" entries:"<<endl;
    for (unsigned int i3 = 0; i3 < R.GetAxisBins(3); ++i3) {
      for (unsigned int i2 = 0; i2 < R.GetAxisBins(2); ++i2) {
        for (unsigned int i1 = 0; i1 < R.GetAxisBins(1); ++i1) {
          os<<"x1/x2/x3: "<<i1<<"/"<<i2<<"/"<<i3
            <<"  - cc: "<<R.GetAxisBinCenter(i1, 1)
            <<"/"<<R.GetAxisBinCenter(i2, 2)
            <<"/"<<R.GetAxisBinCenter(i3, 3)
            <<"  - value : "<<R.GetBinContent(i1, i2, i3)<<endl;
        }
      }
    }
  } else {
    os<<"Empty response matrix of order 3"<<endl;
  }
  return os;
}



// MResponseMatrixO3.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
