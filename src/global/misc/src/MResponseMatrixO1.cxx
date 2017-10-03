/*
 * MResponseMatrixO1.cxx
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
// MResponseMatrixO1
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseMatrixO1.h"

// Standard libs:
#include <iostream>
#include <iterator>
#include <functional>
#include <algorithm>
#include <limits>
using namespace std;

// ROOT libs:
#include <TCanvas.h>
#include <TH1D.h>

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MFileResponse.h"
#include "MTimer.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseMatrixO1)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO1::MResponseMatrixO1() : MResponseMatrixOx()
{
  // default constructor

  Init();
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO1::MResponseMatrixO1(vector<float> xAxis) : MResponseMatrixOx()
{
  // extended constructor

  Init();
  SetAxis(xAxis);
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO1::MResponseMatrixO1(MString Name, vector<float> xAxis) 
  : MResponseMatrixOx(Name)
{
  // extended constructor

  Init();
  SetAxis(xAxis);
}

////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO1::~MResponseMatrixO1()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO1::SetAxis(vector<float> x1Axis)
{
  // Set the axis

  if (x1Axis.size() == 0) {
    merr<<"Size of axis 1 is zero!"<<endl;
    massert(false);
    return;    
  }

  if (IsIncreasing(x1Axis) == false) {
    merr<<"Axes 1 is not in increasing order!"<<endl;
    massert(false);
    return;
  }

  if ((x1Axis.size()-1) > c_SizeLimit) {
    merr<<"The total number of bins is larger than the limit of "
        <<c_SizeLimit<<endl;
    massert(false);
    return;
  }

  m_AxisO1 = x1Axis;
  m_Values.resize(m_AxisO1.size()-1);
}



////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO1::SetAxisNames(MString x1Name)
{
  // Set the names of the axes:

  m_NameAxisO1 = x1Name;
}


////////////////////////////////////////////////////////////////////////////////


MString MResponseMatrixO1::GetAxisName(unsigned int order) const
{
  // Return the name of the axis

  massert(order == 1);

  return m_NameAxisO1;
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO1::Init()
{
  // Initializations common to all constructors

  m_Order = 1;
  m_AxisO1.clear();
  m_Values.clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrixO1::operator==(const MResponseMatrixO1& R)
{
  // Two matrixes are identical if they have the same axis:

  return (m_AxisO1 == R.m_AxisO1);
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO1& MResponseMatrixO1::operator+=(const MResponseMatrixO1& R)
{
  // Append a matrix to this one

  if (*this == R) {
    for (unsigned int i = 0; i < m_Values.size(); ++i) {
      m_Values[i] += R.m_Values[i];
    }
  } else {
    merr<<"Matrices have different axes!"<<endl;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO1& MResponseMatrixO1::operator-=(const MResponseMatrixO1& R)
{
  // Append a matrix to this one

  if (*this == R) {
    for (unsigned int i = 0; i < m_Values.size(); ++i) {
      m_Values[i] -= R.m_Values[i];
    }
  } else {
    merr<<"Matrices have different axes!"<<endl;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO1& MResponseMatrixO1::operator/=(const MResponseMatrixO1& R)
{
  // Append a matrix to this one

  if (*this == R) {
    for (unsigned int i = 0; i < m_Values.size(); ++i) {
      if (R.m_Values[i] != 0) {
        m_Values[i] /= R.m_Values[i];
      } else {
        m_Values[i] = 0;
      }
    }
  } else {
    merr<<"Matrices have different axes!"<<endl;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO1& MResponseMatrixO1::operator+=(const float& Value)
{
  // Append a matrix to this one

  for (unsigned int i = 0; i < m_Values.size(); ++i) {
    m_Values[i] += Value;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO1& MResponseMatrixO1::operator*=(const float& Value)
{
  // Append a matrix to this one

  for (unsigned int i = 0; i < m_Values.size(); ++i) {
    m_Values[i] *= Value;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO1::SetBinContent(unsigned int x, float Value)
{
  // Set the content of the bin

  massert(x < m_AxisO1.size()-1);
  
  m_Values[x] = Value;
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO1::Set(float x, float Value)
{
  // Set a value

  vector<float>::iterator Iter;
  Iter = find(m_AxisO1.begin(), m_AxisO1.end(), x);
  
  if (Iter != m_AxisO1.end()) {
    (*Iter) = Value;
  } else {
    Iter = find_if(m_AxisO1.begin(), m_AxisO1.end(), greater_than<float>(x));
    if (Iter != m_AxisO1.end()) {
      m_AxisO1.insert(Iter, x);
      m_Values.insert(m_Values.begin()+(Iter-m_AxisO1.begin()), Value);
    } else {
      m_AxisO1.push_back(x);
      m_Values.push_back(Value);
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO1::Add(float x, float Value)
{
  // Add a value to the bin closest to x, y

  // Get Position:
  int Position = -1; 
  for (unsigned int i = 0; i < m_AxisO1.size(); ++i) {
    if (m_AxisO1[i] > x) {
      break;
    } 
    Position = (int) i;
  }
  
  if (Position <= -1) {
    mdebug<<"Underflow in "<<m_Name<<": ("<<x<<") = "<<Value<<endl;
    return;
  }
  if (Position >= (int) m_AxisO1.size()-1) {
    mdebug<<"Overflow in "<<m_Name<<": ("<<x<<") = "<<Value<<endl;
    return;
  }

  m_Values[Position] += Value;
}


////////////////////////////////////////////////////////////////////////////////


unsigned long MResponseMatrixO1::GetNBins() const
{
  // Return the number of bins

  return m_AxisO1.size()-1;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MResponseMatrixO1::GetAxisBins(unsigned int order) const
{
  // Return the number of bins for the main axis O2

  massert(order == 1);

  return m_AxisO1.size()-1;
}


////////////////////////////////////////////////////////////////////////////////


vector<float> MResponseMatrixO1::GetAxis(unsigned int order) const
{
  // Return the axis content at bin b

  massert(order == 1);

  return m_AxisO1;
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO1::GetAxisContent(unsigned int b, unsigned int order) const
{
  // Return the axis content at bin b

  massert(order == 1);
  massert(b < m_AxisO1.size()); // !

  return m_AxisO1[b];
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO1::GetAxisBinCenter(unsigned int b, unsigned int order) const
{
  // Return the center of axis bin b

  massert(order == 1);
  massert(b < m_AxisO1.size()-1);

  return 0.5*(m_AxisO1[b]+m_AxisO1[b+1]);
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MResponseMatrixO1::GetAxisBin(float x, unsigned int order) const
{
  // Return the center of axis bin b

  massert(order == 1);

  // Get Position:
  unsigned int Position = c_Outside; 
  for (unsigned int i = 0; i < m_AxisO1.size(); ++i) {
    if (m_AxisO1[i] > x) {
      break;
    } 
    Position = i;
  }

  if (Position >= m_AxisO1.size()-1) {
    Position = c_Outside;
  }

  return Position;
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO1::GetAxisMinimum(unsigned int order) const
{
   // Return the lower edge of the lowest bin of the axis 

  massert(order == 1);
  massert(m_AxisO1.size() > 0);
  
  return m_AxisO1[0];
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO1::GetAxisMaximum(unsigned int order) const
{
   // Return the upper edge of the highest bin of the axis 

  massert(order == 1);
  massert(m_AxisO1.size() > 0);
  
  return m_AxisO1[m_AxisO1.size()-1];
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO1::GetAxisLowEdge(unsigned int b, unsigned int order) const
{
   // Return the lower edge of bin b 

  massert(order == 1);
  massert(b < m_AxisO1.size()-1);

  return m_AxisO1[b];
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO1::GetAxisHighEdge(unsigned int b, unsigned int order) const
{
   // Return the high edge of bin b 

  massert(order == 1);
  massert(b < m_AxisO1.size()-1);

  return m_AxisO1[b+1];
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO1::GetBinContent(unsigned int x) const
{
  // Return the content of bin x, y

  massert(GetNBins() > 0);
  massert(x < m_AxisO1.size()-1);

  return m_Values[x];
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO1::GetBinArea(unsigned int x1) const
{
  // Return the area of this bin

  massert(GetNBins() > 0);
  massert(x1 < m_AxisO1.size()-1);

  return m_AxisO1[x1+1]-m_AxisO1[x1];
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO1::GetInterpolated(float x1, bool DoExtrapolate) const
{
  // Return the array-data according to value x1

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
    if (m_AxisO1.size() == 2) {
      return m_Values.front();
    } else {
      // Get Position (lower bound)
      int Position = FindBinCentered(m_AxisO1, x1);

      // Take care of boundaries:
      if (Position < 0) {
        if (DoExtrapolate == true) {
          Position = 0; // extrapolate below lower edge
        } else {
          return m_Values.front();
        }
      } else if (Position >= int(m_Values.size()-1)) {
        if (DoExtrapolate == true) {
          Position = int(m_Values.size()-2); // extrapolate above higher edge
          // limits of highest CENTERED bin are m_Values.size()-2 and m_Values.size()-1 !!
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
    int Position = FindBin(m_AxisO1, x1);

    // Take care of boundaries:
    if (Position < 0) {
      if (DoExtrapolate == true) {
        Position = 0; // extrapolate below lower edge
      } else {
        return m_Values.front();
      }
    } else if (Position >= int(m_AxisO1.size()-1)) {
      if (DoExtrapolate == true) {
        Position = int(m_AxisO1.size()-2); // extrapolate above higher edge
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
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO1::Get(float x1) const
{
  // Return the array-data according to value x 

  if (m_AxisO1.size() == 0) {
    return 0;
  } 

  int Position = FindBin(m_AxisO1, x1);

  if (Position < 0) {
    Position = 0;
  } else if (Position >= int(GetAxisBins())) {
    Position = GetAxisBins()-1;
  }

  return m_Values[Position];
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO1::GetMaximum() const
{
  // Return the maximum

  float Max = -numeric_limits<float>::max();
  for (unsigned int i = 0; i < m_Values.size(); ++i) {
    if (m_Values[i] > Max) {
      Max = m_Values[i];
    }
  }  

  return Max;
}


////////////////////////////////////////////////////////////////////////////////


float MResponseMatrixO1::GetMinimum() const
{
  // Return the minimum

  float Min = numeric_limits<float>::max();
  for (unsigned int i = 0; i < m_Values.size(); ++i) {
    if (m_Values[i] < Min) {
      Min = m_Values[i];
    }
  }  

  return Min;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMatrixO1::GetSum() const
{
  // Return the sum of all bins:

  double Sum = 0;
  for (unsigned int i = 0; i < m_Values.size(); ++i) {
    Sum += m_Values[i];
  }  

  return Sum;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixO1 MResponseMatrixO1::GetSumMatrixO1(unsigned int a1) const
{
  // Return the sum of the matrix as function of dimension order

  massert(AreIncreasing(1, a1));

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrixO1::ReadSpecific(MFileResponse& Parser, 
                                     const MString& Type, 
                                     const int Version)
{
  // Read the data from file directly into this matrix

  bool Ok = true;
  MTokenizer T;

  if (Type == "ResponseMatrixO1") {
    // mimp<<"Backward compatibility in response matrix O1: NO stream means values are NOT centered in file "<<Parser.GetFileName()<<show;
    // m_ValuesCentered = false;
    m_AxisO1.clear();
    m_Values.clear();
    while (Parser.TokenizeLine(T, true) == true) {
      if (T.GetNTokens() == 0) continue;
      if (T.GetTokenAt(0) == "R1") {
        if (T.GetNTokens() == 3) {
          Set(T.GetTokenAtAsFloat(1), T.GetTokenAtAsFloat(2));
        } else {
          mout<<"MResponseMatrixO2: Wrong number of arguments for token R1!"<<endl;
          Ok = false;
          break;
        }
      }
    }
  } else if (Type == "ResponseMatrixO1Stream") {
    vector<float> xAxis;
    MString x1Name;
    while (Parser.TokenizeLine(T, true) == true) {
     if (T.GetNTokens() < 2) continue;
      if (T.GetTokenAt(0) == "A1") {
        xAxis = T.GetTokenAtAsFloatVector(1);
      } else if (T.GetTokenAt(0) == "N1") {
        x1Name = T.GetTokenAfterAsString(1);
      } else if (T.GetTokenAt(0) == "StartStream") {
        unsigned int StreamSize = T.GetTokenAtAsInt(1);

        if ((xAxis.size()-1) != StreamSize) {
          mout<<"MResponseMatrixO1: Axis sizes and stream size are not in sync!"<<endl;
          Ok = false;
          break;
        }
        SetAxis(xAxis);
        SetAxisNames(x1Name);

        bool StreamOk = true;
        float Data;
        unsigned int x, x_max = GetAxisBins(1); 
        for (x = 0; x < x_max; ++x) {
          if (StreamSize == 0) {
            StreamOk = false;
            break;
          }
          if (Parser.GetFloat(Data) == true) {
            SetBinContent(x, Data);
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
  } else {
    mout<<"MResponseMatrixO1: Wrong file type: "<<Type<<endl;
    Ok = false;    
  }

  return Ok;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrixO1::Write(MString FileName, bool Stream)
{
  // Write the content to file

  massert(GetNBins() > 0);

  MFileResponse File;
  if (File.Open(FileName, MFile::c_Write) == false) return false;

  MTimer Timer;
  mdebug<<"Started writting file \""<<FileName<<"\" ... This way take a while ..."<<endl;

  ostringstream s;
  WriteHeader(s);
  File.Write(s);
  
  if (Stream == false) {
    s<<"Type ResponseMatrixO1"<<endl;
    for (unsigned int i = 0; i < m_AxisO1.size()-1; ++i) {  // ? für *.rsp?
      s<<"R1 "<<m_AxisO1[i]<<" "<<m_Values[i]<<endl;
      File.Write(s);
    }
  } else {
    s<<"Type ResponseMatrixO1Stream"<<endl;
    // Write x-axis
    s<<"A1 ";
    File.Write(s);
    for (unsigned int i = 0; i < m_AxisO1.size(); ++i) {
      File.Write(m_AxisO1[i]);
    }
    s<<endl;
    s<<"N1 "<<GetAxisName(1)<<endl;
    File.Write(s);

    // Write content stream
    s<<"StartStream "<<m_Values.size()<<endl;
    File.Write(s);
    for (unsigned int i = 0; i < m_Values.size(); ++i) {
      File.Write(m_Values[i]);
    }
    s<<endl;
    s<<"StopStream"<<endl;
    File.Write(s);
  }
 
  
  mdebug<<"File \""<<FileName<<"\" with "<<m_AxisO1.size()-1
        <<" entries written in "<<Timer.ElapsedTime()<<" sec"<<endl;
  File.Close();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


TH1* MResponseMatrixO1::GetHistogram(bool Normalize)
{
  // Create a 1D ROOT histogram:
  
  TH1* Histogram = nullptr;
  
  if (GetNBins() > 0) {
  
    TH1D* Hist = 0;
    float* Bins = new float[GetAxisBins(1)+1];
    for (unsigned int i = 0; i <= GetAxisBins(1); ++i) {
      Bins[i] = GetAxisContent(i);
    }
    Hist = new TH1D("", m_Name + "_RM1", GetAxisBins(1), Bins);
    Hist->SetStats(true);
    Hist->SetContour(50);
    Hist->SetXTitle(GetAxisName(1));
    delete [] Bins;
    
    mout<<"Response matrix of order 1 with "<<GetNBins()<<" entries:"<<endl;
    double Norm = 1;
    for (unsigned int i1 = 0; i1 < GetAxisBins(1); ++i1) {
      if (Normalize == true) {
        Norm = GetAxisContent(i1+1, 1)-GetAxisContent(i1, 1);
        if (Norm != 0) {
          Norm = 1.0/Norm;
        } else {
          Norm = 1.0;
        }
      }
      Hist->SetBinContent(i1+1, GetBinContent(i1)*Norm);
    }
    
    Histogram = Hist;

  } else {
    mout<<"Empty response matrix of order 1"<<endl;
  }
  
  return Histogram;
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO1::Show(bool Normalize)
{
  // Create a 1D ROOT histogram:
  
  TH1D* Hist = dynamic_cast<TH1D*>(GetHistogram(Normalize));
  if (Hist == nullptr) {
    mout<<"Unable to generate histogram"<<endl;
    return;
  }
    
  TCanvas* Canvas = new TCanvas();
  Canvas->SetTitle(m_Name);
  Canvas->cd();
  Hist->Draw();
  Canvas->Update();
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrixO1::Smooth(unsigned int Times)
{
  // We access the TH1 routines here...

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
}


////////////////////////////////////////////////////////////////////////////////


TGraph* MResponseMatrixO1::GenerateGraph()
{
  float* Bins = new float[m_AxisO1.size()];
  float* Values = new float[m_AxisO1.size()];
  for (unsigned int i = 0; i < m_AxisO1.size(); ++i) {
    Bins[i] = m_AxisO1[i];
    Values[i] = m_Values[i];
  }
  TGraph* DisplayGraph = new TGraph(m_AxisO1.size(), Bins, Values);
  delete[] Bins;
  delete[] Values;
  return DisplayGraph;
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, const MResponseMatrixO1& R)
{
  os<<"Response Matrix of order 1 with "<<R.m_AxisO1.size()-1<<" entries:"<<endl;
  for (unsigned int i = 0; i < R.m_AxisO1.size()-1; ++i) {
    os<<R.m_AxisO1[i]<<": "<<R.m_Values[i]<<endl;
  }
  return os;
}



// MResponseMatrixO1.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
