/*
 * MResponseMatrixO2.cxx
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
// MResponseMatrixO2
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseMatrixO2.h"

// Standard libs:
#include <iostream>
#include <functional>
#include <algorithm>
#include <limits>
using namespace std;

// ROOT libs:
#include "TH2.h"
#include "TCanvas.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MFileResponse.h"
#include "MTimer.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseMatrixO2)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO2::MResponseMatrixO2() :
  MResponseMatrix()
{
  // default constructor

  Init();
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO2::MResponseMatrixO2(vector<float> xAxis, vector<float> yAxis) :
  MResponseMatrix()
{
  // constructor

  Init();
  SetAxis(xAxis, yAxis);
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO2::MResponseMatrixO2(MString Name, vector<float> xAxis, 
                                     vector<float> yAxis) 
  : MResponseMatrix(Name)
{
  // constructor

  Init();
  SetAxis(xAxis, yAxis);
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO2::~MResponseMatrixO2()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO2::Init()
{
  // Initializations common to all constructors

  m_Order = 2;
  m_AxisO2.clear();
  m_AxesO1.clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrixO2::operator==(const MResponseMatrixO2& R)
{
  // Two matrixes are identical if they have the same axis:

  if (m_AxisO2 != R.m_AxisO2) {
    return false;
  }

  for (unsigned int i = 0; i < m_AxesO1.size(); ++i) {
    if (!(m_AxesO1[i] == R.m_AxesO1[i])) {
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO2& MResponseMatrixO2::operator+=(const MResponseMatrixO2& R)
{
  // Append a matrix to this one

  if (*this == R) {
    for (unsigned int i = 0; i < m_AxesO1.size(); ++i) {
      m_AxesO1[i] += R.m_AxesO1[i];
    }
  } else {
    merr<<"Matrices have different axes!"<<endl;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO2& MResponseMatrixO2::operator-=(const MResponseMatrixO2& R)
{
  // Append a matrix to this one

  if (*this == R) {
    for (unsigned int i = 0; i < m_AxesO1.size(); ++i) {
      m_AxesO1[i] -= R.m_AxesO1[i];
    }
  } else {
    merr<<"Matrices have different axes!"<<endl;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO2& MResponseMatrixO2::operator/=(const MResponseMatrixO2& R)
{
  // Append a matrix to this one

  if (*this == R) {
    for (unsigned int i = 0; i < m_AxesO1.size(); ++i) {
      m_AxesO1[i] /= R.m_AxesO1[i];
    }
  } else {
    merr<<"Matrices have different axes!"<<endl;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO2& MResponseMatrixO2::operator+=(const float& Value)
{
  // Append a matrix to this one

  for (unsigned int i = 0; i < m_AxesO1.size(); ++i) {
    m_AxesO1[i] += Value;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO2& MResponseMatrixO2::operator*=(const float& Value)
{
  // Append a matrix to this one

  for (unsigned int i = 0; i < m_AxesO1.size(); ++i) {
    m_AxesO1[i] *= Value;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO2::SetAxis(vector<float> x1Axis, vector<float> x2Axis)
{
  // Set the axes

  if (x2Axis.size() == 0) {
    merr<<"Size of axis 2 is zero!"<<endl;
    massert(false);
    return;    
  }

  if (IsIncreasing(x2Axis) == false) {
    merr<<"Axes 2 is not in increasing order!"<<endl;
    massert(false);
    return;
  }

  if ((x1Axis.size()-1)*(x2Axis.size()-1) > c_SizeLimit) {
    merr<<"The total number of bins is larger than the limit of "
        <<c_SizeLimit<<endl;
    massert(false);
    return;
  }

  m_AxisO2 = x2Axis;

  m_AxesO1.resize(m_AxisO2.size()-1);
  for (unsigned int b = 0; b < m_AxisO2.size()-1; ++b) {
    m_AxesO1[b].SetAxis(x1Axis);
    m_AxesO1[b].SetName(m_Name);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO2::SetAxisNames(MString x1Name, MString x2Name)
{
  // Set the names of the axes:

  m_NameAxisO2 = x2Name;
  for (unsigned int i = 0; i < m_AxesO1.size(); ++i) {
    m_AxesO1[i].SetAxisNames(x1Name);
  }
}


////////////////////////////////////////////////////////////////////////////////


MString MResponseMatrixO2::GetAxisName(unsigned int order) const
{
  // Return the name of the axis

  massert(order >= 1 && order <= 2);

  if (order == 2) {
    return m_NameAxisO2;
  } else {
    massert(m_AxisO2.size() > 0);
    return m_AxesO1[0].GetAxisName();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO2::SetBinContent(unsigned int x, unsigned int y, float Value)
{
  // Set the content of the bin

  massert(y < m_AxisO2.size()-1);
  
  m_AxesO1[y].SetBinContent(x, Value);
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO2::Add(float x, float y, float Value)
{
  /// Add a value to the bin closest to x, y

  // Get Position:
  int Position = -1; 
  for (unsigned int i = 0; i < m_AxisO2.size(); ++i) {
    if (m_AxisO2[i] > y) {
      break;
    } 
    Position = (int) i;
  }
  
  if (Position <= -1) {
    mdebug<<"Underflow in "<<m_Name<<": ("<<x<<", "<<y<<") = "<<Value<<endl;
    return;
  }
  if (Position >= (int) m_AxisO2.size()-1) {
    mdebug<<"Overflow in "<<m_Name<<": ("<<x<<", "<<y<<") = "<<Value<<endl;
    return;
  }

  m_AxesO1[Position].Add(x, Value);
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO2::SetMatrix(unsigned int b, MResponseMatrixO1 R)
{
  // Set a whole sub matrix

  massert(b < m_AxisO2.size());
  m_AxesO1[b] = R;
}


////////////////////////////////////////////////////////////////////////////////


unsigned long MResponseMatrixO2::GetNBins() const
{
  // Return the number of bins

  if (m_AxisO2.size() < 2) return 0;
  unsigned long Bins = m_AxisO2.size()-1;
  return Bins*m_AxesO1[0].GetNBins();
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MResponseMatrixO2::GetAxisBins(unsigned int order) const
{
  // Return the number of bins for the main axis O2

  massert(order >= 1 && order <= 2);

  if (order == 2) {
    return m_AxisO2.size()-1;
  } else {
    massert(m_AxisO2.size() > 0);
    return m_AxesO1[0].GetAxisBins(order);
  }
}


////////////////////////////////////////////////////////////////////////////////


vector<float> MResponseMatrixO2::GetAxis(unsigned int order) const
{
  // Return the axis content at bin b

  massert(order >= 1 && order <= 2);

  if (order == 2) {
    return m_AxisO2;
  } else {
    massert(GetNBins() > 0);
    return m_AxesO1[0].GetAxis(order);
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO2::GetAxisContent(unsigned int b, unsigned int order) const
{
  // Return the axis content at bin b

  massert(order >= 1 && order <= 2);
  
  if (order == 2) {
    massert(b < m_AxisO2.size());
    return m_AxisO2[b];
  } else {
    massert(GetNBins() > 0);
    return m_AxesO1[0].GetAxisContent(b);
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO2::GetAxisBinCenter(unsigned int b, unsigned int order) const
{
  // Return the center of axis bin b

  massert(order >= 1 && order <= 2);

  if (order == 2) {
    massert(b < m_AxisO2.size()-1);
    return 0.5*(m_AxisO2[b]+m_AxisO2[b+1]);
  } else {
    massert(GetNBins() > 0);
    return m_AxesO1[0].GetAxisBinCenter(b);
  }
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MResponseMatrixO2::GetAxisBin(float x, unsigned int order) const
{
  // Return the center of axis bin b

  massert(order >= 1 && order <= 2);

  // Get Position:
  if (order == 2) {
    unsigned int Position = c_Outside; 
    for (unsigned int i = 0; i < m_AxisO2.size(); ++i) {
      if (m_AxisO2[i] > x) {
        break;
      } 
      Position = i;
    }
    
    if (Position >= m_AxisO2.size()-1) {
      Position = c_Outside;
    }
    return Position;
  } else {
    massert(GetNBins() > 0);
    return m_AxesO1[0].GetAxisBin(x);
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO2::GetAxisMinimum(unsigned int order) const
{
   // Return the lower edge of the lowest bin of the axis 

  massert(order >= 1 && order <= 2);

  if (order == 2) {
    massert(m_AxisO2.size() > 0);
    return m_AxisO2[0];
  } else {
    massert(GetNBins() > 0);
    return m_AxesO1[0].GetAxisMinimum(order);
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO2::GetAxisMaximum(unsigned int order) const
{
   // Return the upper edge of the highest bin of the axis 

  massert(order >= 1 && order <= 2);

  if (order == 2) {
    massert(m_AxisO2.size() > 0);
    return m_AxisO2[m_AxisO2.size()-1];
  } else {
    massert(GetNBins() > 0);
    return m_AxesO1[0].GetAxisMaximum(order);
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO2::GetAxisLowEdge(unsigned int b, unsigned int order) const
{
   // Return the lower edge of bin b 

  massert(order >= 1 && order <= 2);

  if (order == 2) {
    massert(b < m_AxisO2.size()-1);
    return m_AxisO2[b];
  } else {
    massert(GetNBins() > 0);
    return m_AxesO1[0].GetAxisLowEdge(b, order);
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO2::GetAxisHighEdge(unsigned int b, unsigned int order) const
{
   // Return the high edge of bin b 

  massert(order >= 1 && order <= 2);

  if (order == 2) {
    massert(b < m_AxisO2.size()-1);
    return m_AxisO2[b+1];
  } else {
    massert(GetNBins() > 0);
    return m_AxesO1[0].GetAxisHighEdge(b, order);
  } 
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO2::GetBinContent(unsigned int x, unsigned int y) const
{
  // Return the content of bin x, y

  massert(GetNBins() > 0);
  massert(x < GetAxisBins(1));
  massert(y < GetAxisBins(2));

  return m_AxesO1[y].GetBinContent(x);
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO2::GetBinArea(unsigned int x1, unsigned int x2) const
{
  // Return the content of bin x, y

  massert(x2 < m_AxisO2.size()-1);

  return (m_AxisO2[x2+1]-m_AxisO2[x2])*m_AxesO1[x2].GetBinArea(x1);
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO2::GetBinContent(unsigned int x1, unsigned int x1axis, 
                                       unsigned int x2, unsigned int x2axis) const
{
  // 

  massert(GetNBins() > 0);
  massert(x1 < GetAxisBins(x1axis));
  massert(x2 < GetAxisBins(x2axis));
  
  if (x1axis == 1 && x2axis == 2) {
    return m_AxesO1[x2].GetBinContent(x1);
  } else if (x1axis == 2 && x2axis == 1) {
    return m_AxesO1[x1].GetBinContent(x2);
  } else {
    merr<<"Axes are not correctly defined: x1-axis="
        <<x1axis<<" x2-axis="<<x2axis<<endl;
    return 0;
  }
}



////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO2::GetInterpolated(float x1, float x2, bool DoExtrapolate) const
{
  // Return the array-data according to value x1, x2

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
    if (m_AxisO2.size() == 2) {
      return m_AxesO1.front().GetInterpolated(x1, DoExtrapolate);
    } else {
      // Get Position:
      int Position = FindBinCentered(m_AxisO2, x2);

      // Take care of boundaries:
      if (Position < 0) {
        if (DoExtrapolate == true) {
          Position = 0; // extrapolate below lower edge
        } else {
          return m_AxesO1.front().GetInterpolated(x1, DoExtrapolate);
        }
      } else if (Position >= int(m_AxesO1.size()-1)) {
        if (DoExtrapolate == true) {
          Position = int(m_AxesO1.size()-2); // extrapolate above higher edge
          // limits of highest CENTERED bin are m_AxesO1.size()-2 and m_AxesO1.size()-1 !!
        } else {
          return m_AxesO1.back().GetInterpolated(x1, DoExtrapolate);
        }
      }

      // Interpolate:
      return m_AxesO1[Position].GetInterpolated(x1, DoExtrapolate) + (x2 - GetAxisBinCenter(Position))/
          (GetAxisBinCenter(Position+1) - GetAxisBinCenter(Position))*
          (m_AxesO1[Position+1].GetInterpolated(x1, DoExtrapolate) - m_AxesO1[Position].GetInterpolated(x1, DoExtrapolate));
    }
  } else {
    // Get Position:
 		int Position = FindBin(m_AxisO2, x2);

    // Take care of boundaries:
		if (Position < 0) {
			if (DoExtrapolate == true) {
				Position = 0; // extrapolate below lower edge
			} else {
				return m_AxesO1.front().GetInterpolated(x1, DoExtrapolate);
			}
		} else if (Position >= int(m_AxisO2.size()-1)) {
			if (DoExtrapolate == true) {
				Position = int(m_AxisO2.size()-2); // extrapolate above higher edge
				// limits of highest bin are m_AxisO2.size()-2 and  m_AxisO2.size()-1 !!
			} else {
				return m_AxesO1.back().GetInterpolated(x1, DoExtrapolate);
			}
		}
    
    // Interpolate:
    return m_AxesO1[Position].GetInterpolated(x1, DoExtrapolate) + (x2 - m_AxisO2[Position])/
      (m_AxisO2[Position+1] - m_AxisO2[Position])*
      (m_AxesO1[Position+1].GetInterpolated(x1, DoExtrapolate) - m_AxesO1[Position].GetInterpolated(x1, DoExtrapolate));
  }
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO2::Get(float x1, float x2) const
{
  // Return the array-data according to value x 

  if (m_AxisO2.size() == 0) {
    return 0;
  } 

  int Position = FindBin(m_AxisO2, x2); 

  if (Position < 0) {
    Position = 0;
  } else if (Position >= int(GetAxisBins())) {
    Position = GetAxisBins()-1;
  }

  return m_AxesO1[Position].Get(x1);
}

////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO2::GetMaximum() const
{
  // Return the maximum

  float LocMax = -numeric_limits<float>::max();
  float Max = -numeric_limits<float>::max();
  for (unsigned int i = 0; i < m_AxesO1.size(); ++i) {
    LocMax = m_AxesO1[i].GetMaximum();
    if (LocMax > Max) {
      Max = LocMax;
    }
  }  

  return Max;
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO2::GetMinimum() const
{
  // Return the minimum

  float LocMin = numeric_limits<float>::max();
  float Min = numeric_limits<float>::max();
  for (unsigned int i = 0; i < m_AxesO1.size(); ++i) {
    LocMin = m_AxesO1[i].GetMinimum();
    if (LocMin < Min) {
      Min = LocMin;
    }
  }  

  return Min;
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO2::GetSum() const
{
  // Return the sum of all bins:

  float Sum = 0;
  for (unsigned int i = 0; i < m_AxesO1.size(); ++i) {
    Sum += m_AxesO1[i].GetSum();
  }  

  return Sum;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO1 MResponseMatrixO2::GetSumMatrixO1(unsigned int a1) const
{
  // Return the sum of the matrix as function of dimension order
  // Attention: Very inefficient algorithm!

  massert(AreIncreasing(2, a1));

  MResponseMatrixO1 R;
  R.SetAxis(GetAxis(a1));

  if (a1 == 2) {
    for (unsigned int i = 0; i < m_AxesO1.size(); ++i) {
      R.SetBinContent(i, m_AxesO1[i].GetSum());
    }
  } else {
    for (unsigned int i = 0; i < m_AxesO1.size(); ++i) {
      R += m_AxesO1[i].GetSumMatrixO1(a1);
    }
  }
  
  return R;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO2 MResponseMatrixO2::GetSumMatrixO2(unsigned int a1, 
                                                    unsigned int a2) const
{
  // Return the sum of the matrix as function of dimension order
  // Attention: Simple, but very inefficient algorithm!

  massert(AreIncreasing(2, a1, a2));

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO1 MResponseMatrixO2::GetSliceInterpolated(float x1, unsigned int order1)
{
  // Return a slice of order 1 in axis a1 of this matrix

  massert(order1 >= 1 && order1 <= 2);
  //massert(x1 >= GetAxisContent(0, order1) && x1 <= GetAxisContent(GetAxisBins(order1)-1, order1));

  unsigned int newaxis = 0;
  vector<float> values;
  for (unsigned int a = 1; a <= m_Order; ++a) {
    if (a == order1) {
      values.push_back(x1);
    } else {
      values.push_back(0);
      newaxis = a;
    }
  }

  MResponseMatrixO1 M(GetAxis(newaxis));
  
  for (unsigned int i1 = 0; i1 < GetAxisBins(newaxis); ++i1) {
    values[newaxis-1] = GetAxisBinCenter(i1, newaxis);
    M.SetBinContent(i1, GetInterpolated(values[0], values[1]));
  }

  return M;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrixO2::ReadSpecific(MFileResponse& Parser, 
                                     const MString& Type, 
                                     const int Version)
{
  // Read the data from file directly into this matrix

  bool Ok = true;
  MTokenizer T;

  if (Type == "ResponseMatrixO2") {
//     while (Parser.TokenizeLine(T) == true) {
//       if (T.GetNTokens() == 0) continue;
//       if (T.GetTokenAt(0) == "R2") {
//         if (T.GetNTokens() == 4) {
//           Set(T.GetTokenAtAsFloat(1), T.GetTokenAtAsFloat(2), T.GetTokenAtAsFloat(3));
//         } else {
//           mout<<"MResponseMatrixO2: Wrong number of arguments for token R2!"<<endl;
//           Ok = false;
//           break;
//         }
//       }
//     }
  } else if (Type == "ResponseMatrixO2Stream") {
    vector<float> xAxis;
    vector<float> yAxis;
    MString x1Name;
    MString x2Name;
    while (Parser.TokenizeLine(T) == true) {
      if (T.GetNTokens() < 1) continue;
      if (T.GetTokenAt(0) == "A1") {
        xAxis = T.GetTokenAtAsFloatVector(1);
      } else if (T.GetTokenAt(0) == "A2") {
        yAxis = T.GetTokenAtAsFloatVector(1);
      } else if (T.GetTokenAt(0) == "N1") {
        x1Name = T.GetTokenAfterAsString(1);
      } else if (T.GetTokenAt(0) == "N2") {
        x2Name = T.GetTokenAfterAsString(1);
      } else if (T.GetTokenAt(0) == "StartStream") {
        unsigned int StreamSize = T.GetTokenAtAsInt(1);

        if ((xAxis.size()-1)*(yAxis.size()-1) != StreamSize) {
          mout<<"MResponseMatrixO2: Axis sizes and stream size are not in sync!"<<endl;
          Ok = false;
          break;
        }
        SetAxis(xAxis, yAxis);
        SetAxisNames(x1Name, x2Name);

        bool StreamOk = true;
        float Data;
        unsigned int x, x_max = GetAxisBins(1); 
        unsigned int y, y_max = GetAxisBins(2); 
        for (y = 0; y < y_max; ++y) {
          for (x = 0; x < x_max; ++x) {
            if (StreamSize == 0) {
              StreamOk = false;
              break;
            }
            if (Parser.GetFloat(Data) == true) {
              SetBinContent(x, y, Data);
              StreamSize--;
            } else {
              StreamOk = false;
              break;
            }
          }
          
          if (StreamOk == false) {
            mout<<"MResponseMatrixO2: Stream was not ok!"<<endl;
            Ok = false;
            break;
          }
        }
      }
    }
  } else {
    mout<<"MResponseMatrixO2: Wrong file type: "<<Type<<endl;
    Ok = false;    
  }

  return Ok;
}




////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrixO2::Write(MString FileName, bool Stream)
{
  // Write the content to file

  massert(GetNBins() > 0);

  MFileResponse File;
  if (File.Open(FileName, MFile::c_Write) == false) return false;

  MTimer Timer;
  mdebug<<"Started writting file \""<<FileName<<"\" ... This way take a while ..."<<endl;

  ostringstream s;
  s<<"# Response Matrix 2"<<endl;
  s<<"Version 1"<<endl;
  s<<endl;
  s<<"NM "<<m_Name<<endl;
  s<<endl;
  s<<"CE "<<((m_ValuesCentered == true) ? "true" : "false")<<endl;
  File.Write(s);
  
  unsigned int x, x_max = GetAxisBins(1); 
  unsigned int y, y_max = GetAxisBins(2); 

  if (Stream == false) {
    s<<"Type ResponseMatrixO2"<<endl;
    for (y = 0; y < y_max; ++y) {
      for (x = 0; x < x_max; ++x) {
        s<<"R2 "<<GetAxisContent(x, 1)<<" "<<GetAxisContent(y, 2)<<" "<<GetBinContent(x, y)<<endl;
        File.Write(s);
      }
    }
  } else {
    s<<"Type ResponseMatrixO2Stream"<<endl;
    // Write x-axis
    s<<"A1 ";
    File.Write(s);
    for (x = 0; x <= x_max; ++x) {
      File.Write(GetAxisContent(x, 1));
    }
    s<<endl;
    s<<"N1 "<<GetAxisName(1)<<endl;
    File.Write(s);
    // Write y-axis
    s<<"A2 ";
    File.Write(s);
    for (y = 0; y <= y_max; ++y) {
      File.Write(GetAxisContent(y, 2));
    }
    s<<endl;
    s<<"N2 "<<GetAxisName(2)<<endl;
    File.Write(s);

    // Write content stream
    s<<"StartStream "<<GetNBins()<<endl;
    File.Write(s);
    for (y = 0; y < y_max; ++y) {
      for (x = 0; x < x_max; ++x) {
        File.Write(GetBinContent(x, y));
      }
    }
    s<<endl;
    s<<"StopStream"<<endl;
    File.Write(s);
  }
  
  mdebug<<"File \""<<FileName<<"\" with "<<x_max*y_max
        <<" entries written in "<<Timer.ElapsedTime()<<" sec"<<endl;
  File.Close();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO2::Show(float x1, float x2, bool Normalize)
{
  // Create a 2d ROOT histogram:
  
  vector<unsigned int> axes;
  vector<unsigned int> values;
  vector<bool> found(2, false);

  vector<float> ShowMatrix;
  ShowMatrix.push_back(MResponseMatrix::c_ShowX);
  ShowMatrix.push_back(MResponseMatrix::c_ShowY);
  //ShowMatrix.push_back(MResponseMatrix::c_ShowZ);

  for (unsigned int s = 0; s < ShowMatrix.size(); ++s) {
    if (x1 == ShowMatrix[s]) {
      axes.push_back(1);
      values.push_back(GetAxisBin(x1, 1));
      found[0] = true;
    } else if (x2 == ShowMatrix[s]) {
      axes.push_back(2);
      values.push_back(GetAxisBin(x2, 2));
      found[1] = true;
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

  merr<<"No error checks implemented!"<<endl;
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
      Hist = new TH1D(m_Name + "_RM1", m_Name + "_RM1", GetAxisBins(axes[0]), Bins);
      Hist->SetStats(true);
      Hist->SetContour(50);
      Hist->SetXTitle(GetAxisName(axes[0]));
      delete [] Bins;
    
      mout<<"Response matrix of order 2 with "<<GetAxisBins(axes[0])<<" entries:"<<endl;
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
        Hist->SetBinContent(i1+1, GetBinContent(i1, axes[0], values[1], axes[1])*Norm);
      }
    
      TCanvas* Canvas = new TCanvas(m_Name + "_RM2C", m_Name + "_RM2C", 0, 0, 600, 600);
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
      Hist = new TH2D(m_Name+"_RM2", m_Name+"_RM2", GetAxisBins(axes[0]), xBins, GetAxisBins(axes[1]), yBins);
      Hist->SetStats(true);
      Hist->SetContour(50);
      Hist->SetXTitle(GetAxisName(axes[0]));
      Hist->SetYTitle(GetAxisName(axes[1]));
      delete [] xBins;
      delete [] yBins;
      
      mout<<"Response matrix of order 2 with "<<GetNBins()<<" entries:"<<endl;
      double Norm = 1;
      for (unsigned int i2 = 0; i2 < GetAxisBins(axes[1]); ++i2) {
        for (unsigned int i1 = 0; i1 < GetAxisBins(axes[0]); ++i1) {
          if (Normalize == true) {
            Norm = (GetAxisContent(i1+1, axes[0])-GetAxisContent(i1, axes[0]))*
              (GetAxisContent(i2+1, axes[1])-GetAxisContent(i2, axes[1]));
            if (Norm != 0) Norm = 1.0/Norm;
          }
          Hist->SetBinContent(i1+1, i2+1, GetBinContent(i1, axes[0], i2, axes[1])*Norm);
        }
      }
      
      TCanvas* Canvas = new TCanvas(m_Name+"_RM2C", m_Name+"_RM2C", 0, 0, 600, 600);
      Canvas->cd();
      Hist->Draw("colz");
      Canvas->Update();
    } else {
      merr<<"Wrong number of axis: "<<NAxes<<endl;
    }

  } else {
    mout<<"Empty response matrix of order 2"<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO2::Smooth(unsigned int Times)
{
  // We can only smooth the lowest 1D histograms...

  if (Times > 0) {
    double* Values = new double[GetAxisBins(2)];
    for (unsigned int x1 = 0; x1 < GetAxisBins(1); ++x1) {
      for (unsigned int x2 = 0; x2 < GetAxisBins(2); ++x2) {
        Values[x2] = GetBinContent(x1, x2);
      }
      
      TH1::SmoothArray(GetAxisBins(2), Values, Times);
      
      for (unsigned int x2 = 0; x2 < GetAxisBins(2); ++x2) {
        SetBinContent(x1, x2, Values[x2]);
      }
    }
    delete [] Values;

    for (unsigned int x1 = 0; x1 < m_AxesO1.size(); ++x1) {
      m_AxesO1[x1].Smooth(Times);
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, const MResponseMatrixO2& R)
{
  if (R.GetNBins() > 0) {
    os<<"Response matrix of order 2 with "<<R.GetNBins()<<" entries:"<<endl;
    for (unsigned int i2 = 0; i2 < R.GetAxisBins(2); ++i2) {
      for (unsigned int i1 = 0; i1 < R.GetAxisBins(1); ++i1) {
        os<<"x/y: "<<i1<<"/"<<i2<<"  - cc: "<<R.GetAxisBinCenter(i1, 1)<<"/"<<R.GetAxisBinCenter(i2, 2)<<"  - value : "<<R.GetBinContent(i1, i2)<<endl;
      }
    }
  } else {
    os<<"Empty response matrix of order 2"<<endl;
  }
  return os;
}



// MResponseMatrixO2.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
