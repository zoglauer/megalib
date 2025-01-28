/*
 * MFunction.cxx
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
// MFunction
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFunction.h"

// Standard libs:
#include <algorithm>
#include <cmath>
using namespace std;

// ROOT libs:
#include "TH1.h"
#include "TRandom.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TMath.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MFunction)
#endif


////////////////////////////////////////////////////////////////////////////////


const unsigned int MFunction::c_InterpolationUnknown  = 0;
const unsigned int MFunction::c_InterpolationConstant = 1;
const unsigned int MFunction::c_InterpolationNone     = 2;
const unsigned int MFunction::c_InterpolationLinLin   = 3;
const unsigned int MFunction::c_InterpolationLinLog   = 4;
const unsigned int MFunction::c_InterpolationLogLin   = 5;
const unsigned int MFunction::c_InterpolationLogLog   = 6;
const unsigned int MFunction::c_InterpolationSpline3  = 7;
const unsigned int MFunction::c_InterpolationSpline5  = 8;


////////////////////////////////////////////////////////////////////////////////


MFunction::MFunction() : m_InterpolationType(c_InterpolationLinLin)
{
  // Construct an instance of MFunction

  m_YNonNegative = true;
}


////////////////////////////////////////////////////////////////////////////////


MFunction::MFunction(const MFunction& F)
{
  // Copy-construct an instance of MFunction

  m_InterpolationType = F.m_InterpolationType;

  m_X = F.m_X;
  m_Y = F.m_Y;
  m_Cumulative = F.m_Cumulative;
  m_YNonNegative = F.m_YNonNegative;
}


////////////////////////////////////////////////////////////////////////////////


MFunction::~MFunction()
{
  // Delete this instance of MFunction
}


////////////////////////////////////////////////////////////////////////////////


const MFunction& MFunction::operator=(const MFunction& F)
{
  // Copy-construct an instance of MFunction

  m_InterpolationType = F.m_InterpolationType;

  m_X = F.m_X;
  m_Y = F.m_Y;
  m_Cumulative = F.m_Cumulative;
  m_YNonNegative = F.m_YNonNegative;

  CheckDynamicRange();

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


bool MFunction::Set(const MString FileName, const MString KeyWord)
{
  // Set the basic data, load the file and parse it

  m_InterpolationType = c_InterpolationUnknown;

  m_X.clear();
  m_Y.clear();

  MParser Parser;

  if (Parser.Open(FileName, MFile::c_Read) == false) {
    mout<<"Unable to open file "<<FileName<<endl;
    return false;
  }

  for (unsigned int i = 0; i < Parser.GetNLines(); ++i) {
      
    if (Parser.GetTokenizerAt(i)->GetNTokens() == 0) continue;
    if (Parser.GetTokenizerAt(i)->IsTokenAt(0, KeyWord) == true) {
      if (Parser.GetTokenizerAt(i)->GetNTokens() != 3) {
        mout<<"In the function defined by: "<<FileName<<endl;
        mout<<"Wrong number of arguments: "<<Parser.GetTokenizerAt(i)->GetNTokens()<<endl;
        return false;
      }

      m_X.push_back(Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(1));
      m_Y.push_back(Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(2));

    } else if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "IP") == true) {
      if (Parser.GetTokenizerAt(i)->GetNTokens() != 2) {
        mout<<"In the function defined by: "<<FileName<<endl;
        mout<<"Wrong number of arguments for IP keyword!"<<endl;
        return false;
      } 

      MString IP = Parser.GetTokenizerAt(i)->GetTokenAt(1);
      IP.ToLower();
      if (IP == "no" || IP == "none") {
        m_InterpolationType = c_InterpolationNone;
      } else if (IP == "lin" || IP == "linlin") {
        m_InterpolationType = c_InterpolationLinLin;
      } else if (IP == "linlogl") {
        m_InterpolationType = c_InterpolationLinLog;
      } else if (IP == "logllin") {
        m_InterpolationType = c_InterpolationLogLin;
      } else if (IP == "logl" || IP == "logllogl") {
        m_InterpolationType = c_InterpolationLogLog;
      } else {
        mout<<"In the function defined by: "<<FileName<<endl;
        mout<<"Unknown interpolation mode: "<<Parser.GetTokenizerAt(i)->GetTokenAt(1)<<endl;
        return false;        
      }
    }
  }

  // Sanity check:

  if (m_InterpolationType == c_InterpolationUnknown) {
    mout<<"In the function defined by: "<<FileName<<endl;
    mout<<"No interpolation type given in the file! Using linear."<<endl;
    m_InterpolationType = c_InterpolationLinLin;
  }

  // Are m_X in increasing order?
  for (unsigned int i = 1; i < m_X.size(); ++i) {
    if (m_X[i-1] >= m_X[i]) {
      mout<<"In the function defined by: "<<FileName<<endl;
      mout<<"x values are not in increasing order!"<<endl;
      return false;
    }
  }

  if (m_InterpolationType == c_InterpolationLinLog || m_InterpolationType == c_InterpolationLogLog) {
    if (GetYMin() <= 0) {
      mout<<"In the function defined by: "<<FileName<<endl;
      mout<<"Logarithmic interpolation of y requires all y values to be positive! Using linear interpolation."<<endl;      
      m_InterpolationType = c_InterpolationLinLin;
    }
  }

  if (m_InterpolationType == c_InterpolationLogLin || m_InterpolationType == c_InterpolationLogLog) {
    if (GetXMin() <= 0) {
      mout<<"In the function defined by: "<<FileName<<endl;
      mout<<"Logarithmic interpolation of x requires all x values to be positive! Using linear interpolation."<<endl;      
      m_InterpolationType = c_InterpolationLinLin;
    }
  }

  // Determine interpolation type:
  if (m_X.size() > 1 && m_InterpolationType == c_InterpolationConstant) {
    m_InterpolationType = c_InterpolationLinLin;
  } 

  if (m_X.size() == 1) {
    m_InterpolationType = c_InterpolationConstant;
  } 
  
  if (m_X.size() == 0) {
    m_InterpolationType = c_InterpolationConstant;
    m_X.push_back(0);
    m_Y.push_back(0);
  } 
  
  if (GetYMin() < 0) {
    m_YNonNegative = false;
  } else {
    m_YNonNegative = true;
  }

  // Clean up:
  m_Cumulative.clear();

  CheckDynamicRange();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFunction::Save(const MString FileName, const MString Keyword)
{
  // Save the data:

  ofstream out;
  out.open(FileName);
  if (out.is_open() == false) {
    mout<<"Unable to open file \""<<FileName<<"\" for writting."<<endl;
    return false;
  }
  
  if (m_InterpolationType == c_InterpolationNone) {
    out<<"IP None"<<endl;
  } else if (m_InterpolationType == c_InterpolationLinLin) {
    out<<"IP LinLin"<<endl;
  } else if (m_InterpolationType == c_InterpolationLinLog) {
    out<<"IP LinLog"<<endl;
  } else if (m_InterpolationType == c_InterpolationLogLin) {
    out<<"IP LogLin"<<endl;
  } else if (m_InterpolationType == c_InterpolationLogLog) {
    out<<"IP LogLog"<<endl;
  }
  out<<endl;
  for (unsigned int x = 0 ; x < m_X.size(); ++x) {
    out<<Keyword<<" "<<m_X[x]<<" "<<m_Y[x]<<endl;
  }
  out<<"EN"<<endl;
  out<<endl;
  out.close();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFunction::Set(const MResponseMatrixO1& Response)
{
  //! Set the basic data from a 1D ResponseMatrix

  m_X.clear();
  m_Y.clear();

  vector<float> Axis = Response.GetAxis();
  for (unsigned int a = 1; a < Axis.size(); ++a) {
    m_X.push_back(0.5*(Axis[a-1] + Axis[a]));
    m_Y.push_back(Response.GetBinContent(a-1));
  }

  m_InterpolationType = c_InterpolationLinLin;

  if (GetYMin() < 0) {
    m_YNonNegative = false;
  } else {
    m_YNonNegative = true;
  }

  // Clean up:
  m_Cumulative.clear();

  CheckDynamicRange();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFunction::Set(const vector<long double>& X, const vector<long double>& Y, unsigned int InterpolationType)
{
  //! Set the basic data from a 1D ResponseMatrix

  m_X = X;
  m_Y = Y;

  m_InterpolationType = InterpolationType;

  if (GetYMin() < 0) {
    m_YNonNegative = false;
  } else {
    m_YNonNegative = true;
  }

  // Clean up:
  m_Cumulative.clear();

  CheckDynamicRange();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFunction::Set(const vector<double>& X, const vector<double>& Y, unsigned int InterpolationType)
{
  //! Set the basic data from a 1D ResponseMatrix

  m_X.clear();
  for (auto& V: X) m_X.push_back(V);
  m_Y.clear();
  for (auto& V: Y) m_Y.push_back(V);

  m_InterpolationType = InterpolationType;

  if (GetYMin() < 0) {
    m_YNonNegative = false;
  } else {
    m_YNonNegative = true;
  }

  // Clean up:
  m_Cumulative.clear();

  CheckDynamicRange();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFunction::Add(const long double x, const long double y)
{
  //! Add a data point

  if (m_X.size() == 0) {
    m_X.push_back(x);
    m_Y.push_back(y);
  } else {
    if (x < m_X[0]) {
      m_X.insert(m_X.begin(), x);
      m_Y.insert(m_Y.begin(), y);
    } else if (x > m_X.back()) {
      m_X.push_back(x);
      m_Y.push_back(y);
    } else {
      for (unsigned int i = 0; i < m_X.size(); ++i) {
        if (x == m_X[i]) {
          mout<<"   ***  Warning  ***  "<<endl;
          mout<<"X-value (x="<<x<<", y="<<y<<") defined twice! Replacing the original value!"<<endl;
          m_Y[i] = y;
        } else if (x < m_X[i]) {
          m_X.insert(m_X.begin()+i, x);
          m_Y.insert(m_Y.begin()+i, y);
          break;
        }
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////


void MFunction::CheckDynamicRange()
{
  //! Check the dynamic range and give a warning message if we are too big

  static bool FirstCall = true;
  if (FirstCall == true && m_X.size() > 0) {
    if (sizeof(long double) < 16) {
      if (m_X.back() / m_X.front() >= 100000000 && m_X.back() - m_X.front() >= 100000000) { // best guess how to handle zero
        mout<<endl;
        mout<<"Attention:"<<endl;
        mout<<"The size of \"long double\" on this system is just "<<sizeof(long double)<<" bytes."<<endl;
        mout<<"This can negatively affect analyses / simulations which require an extreme dynamic range, such as"<<endl;
        mout<<"+ spectra reaching from eV to TeV,"<<endl;
        mout<<"+ time scales going from nano-seconds to years, or"<<endl;
        mout<<"+ position accuracies going from detector pixel sizes (mm) to Galactic sizes."<<endl;
        mout<<"You have a MFunction object (I don't know which one) with a dynamic range from "<<m_X.front()<<" to "<<m_X.back()<<endl;
        mout<<"Depending on what you are doing exactly, this might impact the accuracy of your results."<<endl;
        mout<<endl;
        FirstCall = false;
      }
    } else {
      FirstCall = false;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MFunction::ScaleY(long double Scaler)
{
  // Scale the y-content by some value

  for (unsigned int i = 0; i < m_Y.size(); ++i) {
    m_Y[i] *= Scaler;
  } 

  if (GetYMin() < 0) {
    m_YNonNegative = false;
  } else {
    m_YNonNegative = true;
  }

  // Clean up:
  m_Cumulative.clear();
}


////////////////////////////////////////////////////////////////////////////////


void MFunction::ScaleX(long double Scaler)
{
  // Multiple the x-axis by some value

   for (unsigned int i = 0; i < m_X.size(); ++i) {
    m_X[i] *= Scaler;
  }  

  // We clear the cumulative function:
  m_Cumulative.clear();
}


////////////////////////////////////////////////////////////////////////////////


long double MFunction::Eval(long double x) const
{ 
  mdep<<"MFunction::Eval is deprecated, replace with: MFunction::Evaluate"<<show;
  return Evaluate(x); 
}


////////////////////////////////////////////////////////////////////////////////


long double MFunction::Evaluate(long double x) const
{
  // Evalute the function considering the different interpolation types

  if (m_InterpolationType == c_InterpolationUnknown) {
    merr<<"Unknown interpolation type: "<<c_InterpolationUnknown<<show;
    return 0;
  }

  if (m_X.size() == 0) {
    merr<<"This function contains no data points for evaluation!"<<show;
    massert(false);
    return 0;
  }

  if (m_InterpolationType == c_InterpolationConstant || m_X.size() == 1) {
    return m_Y[0];
  } else if (m_InterpolationType == c_InterpolationNone) {

    // Get Position:
    int xPosition = -1; 
    for (unsigned int i = 0; i < m_X.size(); ++i) {
      if (m_X[i] > x) {
        break;
      } 
      xPosition = (int) i;
    }

    if (xPosition < 0) xPosition = 0;
    if (xPosition >= (int) m_X.size()-1) xPosition = (int) (m_X.size()-2);

    if (x > m_X[xPosition] + 0.5*(m_X[xPosition+1] - m_X[xPosition])) xPosition += 1;
    if (xPosition > (int) m_X.size()-1) xPosition = (int) (m_X.size()-1);

    return m_Y[xPosition];

  } else if (m_InterpolationType == c_InterpolationLinLin ||
             m_InterpolationType == c_InterpolationLinLog ||
             m_InterpolationType == c_InterpolationLogLin ||
             m_InterpolationType == c_InterpolationLogLog) {

    long double y = 0.0;

    int Position = -1; 
    for (unsigned int i = 0; i < m_X.size(); ++i) {
      if (m_X[i] > x) {
        break;
      } 
      Position = (int) i;
    }
    
    long double x1, x2, y1, y2;
    // Position = -1: Extrapolate to lower x
    if (Position == -1) {
      x1 = m_X[0];
      x2 = m_X[1];
      y1 = m_Y[0];
      y2 = m_Y[1];
    } 
    // Position = MAX: Extrapolate to higher x
    else if (Position >= (int) m_X.size()-1) {
      x1 = m_X[Position-1];
      x2 = m_X[Position];
      y1 = m_Y[Position-1];
      y2 = m_Y[Position];
    }
    // Position = [0..MAX-1] : interpolate
    else {
      x1 = m_X[Position];
      x2 = m_X[Position+1];
      y1 = m_Y[Position];
      y2 = m_Y[Position+1];
    }

    // Attention for logl interpolation make sure all values are positive!
    if (m_InterpolationType == c_InterpolationLinLog || m_InterpolationType == c_InterpolationLogLog) {
      y1 = logl(y1);
      y2 = logl(y2);
    }
    if (m_InterpolationType == c_InterpolationLogLin || m_InterpolationType == c_InterpolationLogLog) {
      x = logl(x);
      x1 = logl(x1);
      x2 = logl(x2);
    }

    long double m = (y2-y1)/(x2-x1);
    long double t = y2 - m*x2;

    if (m_InterpolationType == c_InterpolationLinLog || m_InterpolationType == c_InterpolationLogLog) {
      y = expl(m*x+t);
    } else {
      y = m*x+t;
    }
    
    if (std::isnan(y)) { // std:: is required here due to multiple definitions
      merr<<"Interpolation error for interpolation type "<<m_InterpolationType<<": y is NaN!"<<endl;;
      merr<<"   m="<<m<<"  t="<<t<<"  x1="<<x1<<"  y1="<<y1<<"  x2="<<x2<<"  y2="<<y2<<show;
    }

    return y;
  } 

  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


long double MFunction::Integrate() const
{
  // Integrate all the data from min to max

  return Integrate(m_X.front(), m_X.back());
}

////////////////////////////////////////////////////////////////////////////////


long double MFunction::Integrate(long double XMin, long double XMax) const
{
  // Integrate the data from min to max
  
  if (XMin < m_X.front()) {
    merr<<"XMin ("<<XMin<<") smaller than minimum x-value ("<<m_X.front()<<") --- starting at minimum x-value"<<endl;
    XMin = m_X.front();
  }
  if (XMax > m_X.back()) {
    merr<<"XMax ("<<XMax<<") larger than maximum x-value ("<<m_X.back()<<") --- ending at maximum x-value"<<endl;
    XMax = m_X.back();
  }
  if (XMin >= XMax) {
    merr<<"XMin ("<<XMin<<") not smaller than XMax ("<<XMax<<")"<<endl;
    return 0.0;    
  }

  int BinMin = 0;
  if (XMin > m_X.front()) {
    // BinMin = find_if(m_X.begin(), m_X.end(), bind2nd(greater<long double>(), XMin)) - m_X.begin() - 1;
    BinMin = find_if(m_X.begin(), m_X.end(), bind(greater<long double>(), placeholders::_1, XMin)) - m_X.begin() - 1;
//     unsigned int upper = m_Cumulative.size();
//     unsigned int center = 1;
//     unsigned int lower = 0;
    
//     while (upper-lower > 1) {
//       center = (upper+lower) >> 1;
//       if (XMin == m_X[center]) {
//         BinMin = int(center)+1;
//       }
//       if (XMin < m_X[center]) {
//         upper = center;
//       } else {
//         lower = center;
//       }
//     }
//     BinMin = int(lower)+1;
  }
  int BinMax = m_X.size()-1;
  if (XMax < m_X.back()) {
    //BinMax = find_if(m_X.begin(), m_X.end(), bind2nd(greater_equal<long double>(), XMax)) - m_X.begin();
    BinMax = find_if(m_X.begin(), m_X.end(), bind(greater_equal<long double>(), placeholders::_1, XMax)) - m_X.begin();
//     unsigned int upper = m_Cumulative.size();
//     unsigned int center = 1;
//     unsigned int lower = 0;
    
//     while (upper-lower > 1) {
//       center = (upper+lower) >> 1;
//       if (XMax == m_X[center]) {
//         BinMax = int(center)+1;
//       }
//       if (XMax < m_X[center]) {
//         upper = center;
//       } else {
//         lower = center;
//       }
//     }
//     BinMax = int(lower)+1;
  }
  
  long double Integral = 0.0;
  if (m_InterpolationType == c_InterpolationConstant) {
    Integral = (XMax - XMin) * m_Y[0];
  } else if (m_InterpolationType == c_InterpolationNone) {
    // Just sum rectangular bins:

    long double BinCenterMin = m_X[BinMin-1] + 0.5*(m_X[BinMin] - m_X[BinMin-1]);
    long double BinCenterMax = m_X[BinMax-1] + 0.5*(m_X[BinMax] - m_X[BinMax-1]);

    if (BinMin != BinMax) {
      if (XMin < BinCenterMin) {
        Integral += (BinCenterMin - XMin)*m_Y[BinMin-1];
        Integral += (m_X[BinMin] - BinCenterMin)*m_Y[BinMin];
      } else {
        Integral += (m_X[BinMin] - XMin)*m_Y[BinMin];
      }
      for (int b = BinMin; b < BinMax; ++b) {
        Integral += 0.5*(m_X[BinMin+1] - m_X[BinMin])*m_Y[BinMin];
        Integral += 0.5*(m_X[BinMin+1] - m_X[BinMin])*m_Y[BinMin+1];
      }
      if (XMax > BinCenterMax) {
        Integral += (BinCenterMax - m_X[BinMax-1])*m_Y[BinMax-1];
        Integral += (XMax - BinCenterMax)*m_Y[BinMax];
      } else {
        Integral += (XMax - m_X[BinMax-1])*m_Y[BinMax-1];
      }
    } else {
      if (XMin < BinCenterMin && XMax < BinCenterMin) {
        Integral += (XMax - XMin)*m_Y[BinMin-1];
      } else if (XMin > BinCenterMin && XMax > BinCenterMin) {
        Integral += (XMax - XMin)*m_Y[BinMin];
      } else {
        Integral += (BinCenterMin - XMin)*m_Y[BinMin-1];
        Integral += (XMax - BinCenterMax)*m_Y[BinMin];
      }
    }      
  } else if (m_InterpolationType == c_InterpolationLinLin ||
             m_InterpolationType == c_InterpolationLinLog ||
             m_InterpolationType == c_InterpolationLogLin ||
             m_InterpolationType == c_InterpolationLogLog) {
    
    long double x1, x2, y1, y2;
    for (int i = BinMin; i < BinMax; ++i) {

      if (i == BinMin) {
        x1 = XMin;
        y1 = Evaluate(x1);
      } else {
        x1 = m_X[i];
        y1 = m_Y[i];
      }
      if (i == BinMax) {
        x2 = XMax;
        y2 = Evaluate(x2);
      } else {
        x2 = m_X[i+1];
        y2 = m_Y[i+1];
      }
      
      // Attention for logl interpolation make sure all values are positive!
      if (m_InterpolationType == c_InterpolationLinLog || 
          m_InterpolationType == c_InterpolationLogLog) {
        y1 = logl(y1);
        y2 = logl(y2);
      }
      if (m_InterpolationType == c_InterpolationLogLin || 
          m_InterpolationType == c_InterpolationLogLog) {
        x1 = logl(x1);
        x2 = logl(x2);
      }
      
      //cout<<x1<<":"<<x2<<endl;
    
      // Calculate m, t of the interpolation "line" (it's always line in the respective mode) 
      long double m = (y2-y1)/(x2-x1);
      long double t = y2 - m*x2;

      // Switch back
      if (m_InterpolationType == c_InterpolationLinLog || 
          m_InterpolationType == c_InterpolationLogLog) {
        y1 = expl(y1);
        y2 = expl(y2);
      }
      if (m_InterpolationType == c_InterpolationLogLin || 
          m_InterpolationType == c_InterpolationLogLog) {
        x1 = expl(x1);
        x2 = expl(x2);
      }
      
      // Do the integration using the correct formula for the various modi:
      if (m_InterpolationType == c_InterpolationLinLin) {
        // y=m*x+t --> int(m*x+t)
        Integral += 0.5*m*(x2*x2-x1*x1) + t*(x2-x1);      
      } else if (m_InterpolationType == c_InterpolationLinLog) {
        // ln(y) = m*x+t --> int(expl(m*x+t))
        if (fabs(m) < 10E-8) {
          Integral += expl(t)*(x2-x1);
        } else {
          Integral += 1/m * (expl(m*x2+t) - expl(m*x1+t));
        }
      } else if (m_InterpolationType == c_InterpolationLogLin) {
        // y = m*ln(x) + t --> int(m*ln(x) + t)
        Integral += x2*(m*(logl(x2) - 1) + t) - x1*(m*(logl(x1) - 1) + t);
      } else if (m_InterpolationType == c_InterpolationLogLog) {
        // ln(y) = m*ln(x) + t --> y = expl(t)*x^m --> int(expl(t)*x^m)
        if (fabs(m+1) < 10E-8) {
          Integral += expl(t)*(logl(x2)-logl(x1));
        } else {
          Integral += expl(t)/(m+1) * (powl(x2, m+1) - powl(x1, m+1));
        }
      }
    }

  } else {
    merr<<"This interpolation type is no longer supported..."<<endl;
  }

  return Integral;
}


////////////////////////////////////////////////////////////////////////////////


long double MFunction::GetRandom()
{
  // Return a random number distributed as the underlying function

  if (m_YNonNegative == false) {
    merr<<"GetRandom only works if all y-values are positive ot zero."<<show;      
    return 0;
  }

  // Check if we have to determine the cumulative function:
  if (m_Cumulative.size() == 0) {
    if (m_Y.size() > 5000) {
      mout<<"MFunction: Determining the cumulative function --- this can take a very long time, especially when MEGAlib was compiled in debug mode..."<<endl;
    }
    m_Cumulative.push_back(0);
    for (unsigned int i = 1; i < m_Y.size(); ++i) {
      m_Cumulative.push_back(m_Cumulative.back() + Integrate(m_X[i-1], m_X[i]));
    }
  }

  // Find a random number on the total intensity scale and then (function call)
  // the appropriate x-value
  return GetRandomInterpolate(gRandom->Rndm()*m_Cumulative.back());
}


////////////////////////////////////////////////////////////////////////////////


long double MFunction::GetRandomTimesX()
{
  // Return a random number distributed as the underlying function times X

  if (m_YNonNegative == false) {
    merr<<"GetRandom only works if all y-values are positive ot zero."<<show;      
    return 0;
  }

  // Check if we have to determine the cumulative function:
  if (m_Cumulative.size() == 0) {
    m_Cumulative.push_back(0);
    for (unsigned int i = 1; i < m_Y.size(); ++i) {
      m_Cumulative.push_back(m_Cumulative.back() + Integrate(m_X[i-1], m_X[i]));
    }
  }

  // Find a random number on the total intensity scale and then (function call)
  // the appropriate x-value
  return GetRandomInterpolate(sqrtl(gRandom->Rndm())*m_Cumulative.back());
}

////////////////////////////////////////////////////////////////////////////////


long double MFunction::GetRandomInterpolate(long double Itot)
{
  // Second stage of the GetRandom function
  // A random number on the total intensity scale, Itot, is already found
  // Now find the correct x-value via interpolation

  // Find the correct bin in m_Cumulative
  //int Bin = find_if(m_Cumulative.begin(), m_Cumulative.end(), bind2nd(greater_equal<long double>(), Itot)) - m_Cumulative.begin();
  int Bin = find_if(m_Cumulative.begin(), m_Cumulative.end(), bind(greater_equal<long double>(), placeholders::_1, Itot)) - m_Cumulative.begin();

//   // Binary search:
//   unsigned int upper = m_Cumulative.size();
//   unsigned int center = 1;
//   unsigned int lower = 0;
  
//   while (upper-lower > 1) {
//     center = (upper+lower) >> 1;
//     if (Itot == m_Cumulative[center]) {
//       Bin = int(center)+1;
//     }
//     if (Itot < m_Cumulative[center]) {
//       upper = center;
//     } else {
//       lower = center;
//     }
//   }
//   Bin = int(lower)+1;


  // And the x-value via the given interpolation method:
  if (Bin == 0) {
    return m_X[0];
  }

  if (m_InterpolationType == c_InterpolationConstant) {
    // Constant is just a dummy if something went wrong dramatically, thus:
    // Just a random number between x_min and x_max:
    if (m_X.size() > 1) {
      return gRandom->Rndm()*(m_X.back()-m_X.front());
    } else {
      return m_X.front();
    }
  } else if (m_InterpolationType == c_InterpolationNone) {

    // Relative intensity in this bin:
    long double I = Itot - m_Cumulative[Bin-1];
    // Absolute intensity in this bin
    long double A = m_Cumulative[Bin] - m_Cumulative[Bin-1];
    
    return m_X[Bin-1] + (I/A)*(m_X[Bin] - m_X[Bin-1]);

  } else if (m_InterpolationType == c_InterpolationLinLin ||
             m_InterpolationType == c_InterpolationLinLog ||
             m_InterpolationType == c_InterpolationLogLin ||
             m_InterpolationType == c_InterpolationLogLog) {

    long double x1, x2, y1, y2;
    x1 = m_X[Bin-1];
    y1 = m_Y[Bin-1];
    x2 = m_X[Bin];
    y2 = m_Y[Bin];
    
    // Attention for logl interpolation make sure all values are positive!
    if (m_InterpolationType == c_InterpolationLinLog || m_InterpolationType == c_InterpolationLogLog) {
      y1 = logl(y1);
      y2 = logl(y2);
    }
    if (m_InterpolationType == c_InterpolationLogLin || m_InterpolationType == c_InterpolationLogLog) {
      x1 = logl(x1);
      x2 = logl(x2);
    }
    
    // Calculate m, t of the interpolation "line" (it's always a line in the respective mode) 
    long double m = (y2-y1)/(x2-x1);
    long double t = y2 - m*x2;
    
    // Switch back
    if (m_InterpolationType == c_InterpolationLinLog || m_InterpolationType == c_InterpolationLogLog) {
      y1 = expl(y1);
      y2 = expl(y2);
    }
    if (m_InterpolationType == c_InterpolationLogLin || m_InterpolationType == c_InterpolationLogLog) {
      x1 = expl(x1);
      x2 = expl(x2);
    }

    // Relative intensity in this bin:
    long double I  = Itot - m_Cumulative[Bin-1];

    if (m_InterpolationType == c_InterpolationLinLin) {
      // We know m, t (from y=m*x+t), as well as I = Itot - I[Bin-1], i.e. the covered area in the given bin
      // What we want is x:
      // I = int(x'[Bin-1]->x) m*x'+t dx'
      // x = (-t+-sqrtl(t*t+m*m*x0*x0+2*m*t*x0+2*m*I))/m
      
      // Special case:
      if (m == 0) {
        // Super-special case:
        if (t == 0) {
          return x1;
        } else {
          return I/t + x1;
        }
      }
      
      // Standard case
      long double Value = (t+m*x1)*(t+m*x1) + 2*m*I;
  
      // the possible solutions
      long double xs1 = (-t-sqrtl(Value))/m;
      long double xs2 = (-t+sqrtl(Value))/m;
      
      long double x = 0.0;
      if (m >= 0) {
        // Only one positive solution on positive branch of parabola (top is open):
        x = ((xs1 > xs2) ? xs1 : xs2);
      } else {
        // Two solutions on positive branch of parabola, the smaller one is the correct one
        if (x1 != 0) {
          x = ((xs1 > xs2) ? xs2 : xs1);
        } else {
          // Special case: if x1 = 0, then one solution is zero, use the one which is not zero
          if (xs1 == 0 && xs2 > 0) {
            x = xs2;
          } else if (xs1 > 0 && xs2 == 0 ) {
            x = xs1;
          } else {
            merr<<"This case should have never happened: "<<endl;
            merr<<"Either solution 1 or solution 2 has to be zero and the other one has to be positive!"<<endl;
            merr<<"X0: "<<x1<<" - X1: "<<xs1<<" - X2: "<<xs2<<endl;
            return 0.0;
          }
        }
      }
      
      if (m_X[Bin-1] > x || m_X[Bin] < x) {
        merr<<"This case should have never happened: "<<endl;
        merr<<"x is outside the interval boundaries: "<<endl;
        merr<<"x_min: "<<m_X[Bin-1]<<" - x: "<<x<<" ("<<xs1<<", "<<xs2<<") - x_max: "<<m_X[Bin]<<endl;
        return 0.0;
      }

      return x;

    } else if (m_InterpolationType == c_InterpolationLinLog) {
      // Here we look for the solution if I = int[x'[Bin-1]->x] expl(m*x'+t) dx'
      // Fortunately that's easier, since we have only one valid solution
      
      // Special case:
      if (fabs(m) < 10E-8) { // m == 0
        return x1 + I*expl(-t);
      }

      return (logl(expl(x1*m+t) + I*m) - t)/m;

    } else if (m_InterpolationType == c_InterpolationLogLin) {
      // Here we look for the solution if I = int[x'[Bin-1]->x] m*ln(x')+t dx'
      
      // Special case:
      if (fabs(m) < 10E-8) { // m == 0
        if (fabs(t) < 10E-8) { // t == 0
          return 0.0;
        } else {
          return (t*x1 + I)/t;
        }
      }
      
      long double LW = 0.0;
      // That's just a guess --- not sure is it is not a function of (m*x1*logl(x1)-m*x1+t*x1+I)/m*expl(-(m-t)/m
      if (m < 0) {
        LW = LambertW((m*x1*logl(x1)-m*x1+t*x1+I)/m*expl(-(m-t)/m), -1);
      } else {
        LW = LambertW((m*x1*logl(x1)-m*x1+t*x1+I)/m*expl(-(m-t)/m), 0);
      }
      
      return expl((LW*m+m-t)/m);
      
    } else if (m_InterpolationType == c_InterpolationLogLog) {
      // Here we look for the solution if I = int[x'[Bin-1]->x] expl(t)*x'^m dx'
      // Fortunately that's easier, since we have only one valid solution
      
      // Special case:
      if (fabs(m+1) < 10E-8) { // m == -1
        return (x1*expl(t) + I) * expl(-t);
      }
      
      return expl(-(t-logl(powl(x1, m+1)*expl(t)+I*m+I))/(m+1));
    }
    
  } else {
    merr<<"This interpolation type is no longer supported..."<<endl;
    return 0.0;
  }

  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


long double MFunction::GetXMin() const
{
  //! Get the minimum x-value

  return m_X.front();
}


////////////////////////////////////////////////////////////////////////////////


long double MFunction::GetXMax() const
{
  //! Get the maximum x-value

  return m_X.back();
}


////////////////////////////////////////////////////////////////////////////////


long double MFunction::GetYMin() const
{
  //! Get the minimum y-value

  long double Min = numeric_limits<long double>::max();
  for (unsigned int i = 0; i < m_Y.size(); ++i) {
    if (m_Y[i] < Min) Min = m_Y[i];
  }

  return Min;
}


////////////////////////////////////////////////////////////////////////////////


long double MFunction::GetYMax() const
{
  //! Get the maximum y-value

  long double Max = -numeric_limits<long double>::max();
  for (unsigned int i = 0; i < m_Y.size(); ++i) {
    if (m_Y[i] > Max) Max = m_Y[i];
  }

  return Max;
}


////////////////////////////////////////////////////////////////////////////////


long double MFunction::FindX(long double XStart, long double Integral, bool Cyclic)
{
  //! Find the x value starting from Start which would be achieved after integrating for "Integral"
  //! If we go beyond x_max, x_max is returned if we are not cyclic, otherwise we continue at x_0

  //cout<<"XStart: "<<XStart<<"  Integral: "<<Integral<<endl;
  
  long double Modulo = 0;
  if (Cyclic == true) {
    // Project XStart into the frame of this function
    long double Front = m_X.front();
    long double Back = m_X.back();
    
    if (XStart < Front) {
      Modulo = static_cast<long double>(static_cast<int>((Front - XStart)/(Back - Front)) + 1) * (Back - Front);
    } else if (XStart > Back) {
      Modulo = -static_cast<long double>(static_cast<int>((XStart - Back)/(Back - Front)) + 1) * (Back - Front);
    }
  }

  long double X = XStart + Modulo;

  // Find the bin X is in: 
  if (X < m_X.front()) {
    //merr<<"XStart ("<<XStart<<") smaller than minimum x-value ("<<m_X.front()<<") --- starting at minimum x-value"<<endl;
    X = m_X.front();
  }
  if (X > m_X.back()) {
    //merr<<"XStart ("<<XStart<<") larger than maximum x-value ("<<m_X.back()<<") --- starting at minimum x-value"<<endl;
    X = m_X.front();
  }

  // Step 1: Go from bin to bin until we find an upper limit bin, where iIntegral > I

  unsigned int BinStart = 0;
  if (X > m_X.front()) {
    //BinStart = find_if(m_X.begin(), m_X.end(), bind2nd(greater<long double>(), X)) - m_X.begin() - 1;
    BinStart = find_if(m_X.begin(), m_X.end(), bind(greater<long double>(), placeholders::_1, X)) - m_X.begin() - 1;
  }

  //cout<<"x: "<<X<<" Bin start: "<<BinStart<<endl;
  
  unsigned int NewUpperBin = BinStart;
  long double tIntegral = 0.0;
  long double iIntegral = 0.0;
  do {
    NewUpperBin++;
    tIntegral = Integrate(X, m_X[NewUpperBin]);
    //cout<<"Int from "<<X<<" to "<<m_X[NewUpperBin]<<": "<<tIntegral<<" (total: "<<iIntegral<<")"<<endl;
    if (iIntegral + tIntegral < Integral) {
      X = m_X[NewUpperBin];
      iIntegral += tIntegral;
    } else {
      //cout<<"Found it"<<endl;
      break;
    }
    if (X == m_X.back()) {
      if (Cyclic == false) {
        break;
      } else {
        X = 0;
        NewUpperBin = 0;
        Modulo -= m_X.back() - m_X.front();
        cout<<"New Modulo (while): "<<Modulo<<endl;
      }
    }
  } while (true);
  
  // Non-cyclic exit case
  if (X == m_X.back() && iIntegral < Integral) return numeric_limits<long double>::max();
  
  //cout<<"UpperBin: "<<NewUpperBin<<endl;
  
  // Step 2: Interpolate --- only linear at the moment --- within the given bin to find the right x-value
  
  long double m = (m_Y[NewUpperBin-1] - m_Y[NewUpperBin]) / (m_X[NewUpperBin-1] - m_X[NewUpperBin]);
  long double t = m_Y[NewUpperBin] - m*m_X[NewUpperBin];
  
  //cout<<"m: "<<m<<" t: "<<t<<endl;
  
  long double x1 = 0;
  long double x2 = 0;
  
  if (m != 0) {
    long double a = 0.5*m;
    long double b = t;
    long double c = -((Integral-iIntegral) + 0.5*m*X*X + t*X);
  
    x1 = (-b-sqrtl(b*b-4*a*c))/(2*a);
    x2 = (-b+sqrtl(b*b-4*a*c))/(2*a);
  } else {
    x1 = X + (Integral-iIntegral)/t; // t cannot be null here other wise we would have jumped the bin...
    x2 = numeric_limits<long double>::max();
  }
  //cout<<"x1: "<<x1<<" x2: "<<x2<<endl;
  
  if (x1 >= m_X[NewUpperBin-1] && x1 <= m_X[NewUpperBin] && (x2 < m_X[NewUpperBin-1] || x2 > m_X[NewUpperBin])) {
    //mout<<"x="<<x1<<endl;
    X = x1;
  } else if (x2 >= m_X[NewUpperBin-1] && x2 <= m_X[NewUpperBin] && (x1 < m_X[NewUpperBin-1] || x1 > m_X[NewUpperBin])) {
    //mout<<"x="<<x2<<endl;
    X = x2; 
  } else if ((x2 < m_X[NewUpperBin-1] || x2 > m_X[NewUpperBin]) && (x1 < m_X[NewUpperBin-1] || x1 > m_X[NewUpperBin])) {
    merr<<"FindX: Both possible results are outside choosen bin ["<<m_X[NewUpperBin-1]<<"-"<<m_X[NewUpperBin]<<"]: x1="<<x1<<" x2="<<x2<<endl;
  } else {
    merr<<"FindX: Both possible results are within choosen bin ["<<m_X[NewUpperBin-1]<<"-"<<m_X[NewUpperBin]<<"]: x1="<<x1<<" x2="<<x2<<endl;    
  }
  
  //cout<<"XStart: "<<XStart<<" X: "<<X<<" modulo: "<<Modulo<<endl;
  
  return X - Modulo;
}


////////////////////////////////////////////////////////////////////////////////


void MFunction::Plot()
{
  // Plot the function in a Canvas (diagnostics only)
  
  if (m_X.size() >= 2) {
    //long double Dist = m_X.back()-m_X.front();
    //TH1D* Hist = new TH1D("Diagnostics", "Diagnostics", 10000, m_X.front()-0.1*Dist, m_X.back()+0.1*Dist);
    
    TH1D* Hist = new TH1D("Diagnostics", "Diagnostics", 10000, m_X.front(), m_X.back());
    for (int b = 1; b <= Hist->GetXaxis()->GetNbins(); ++b) {
      Hist->SetBinContent(b, Evaluate(Hist->GetBinCenter(b)));
    }
    TCanvas* Canvas = new TCanvas("DiagnosticsCanvas", "DiagnosticsCanvas");
    Canvas->cd();
    Hist->Draw();
    Canvas->Update();

    gSystem->ProcessEvents();

    for (unsigned int i = 0; i < 10; ++i) {
      gSystem->ProcessEvents();
      gSystem->Sleep(10);
    }

  } else {
    mout<<"Not enough data points for diagnostics: "<<m_X.size()<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


long double MFunction::LambertW(long double x, int Branch)
{
  // Implementation of Lambert's W function branches 0 and -1

  long double LW = 0.0;

  if (Branch == 0) {

    if (x < -1/TMath::E()) {
      merr<<"This branch (\"0\") of the Labert W function is only defined within ["<<-1/TMath::E()<<";inf[! Input is "<<x<<". Returning zero..."<<endl;
      return 0.0;
    }
    
    if (x == -1.0/TMath::E()) {
      return -1.0;
    }
   
    // Approximation according to:
    // Barry & Culling-Hensely, ACM Transactions on Mathematical Software, 21(2), 1995

    // Calculate the primary branch:
    if (x <= 20) {
      long double Eta = 2 + 2*TMath::E()*x;
      
      long double N2 = 4.612634277343749*sqrtl(sqrtl(sqrtl(Eta) + 1.09556884765625)); // eqn. (6) --> Eta -> sqrtl(Eta)
      long double N1 = (4 - 3*sqrtl(2.0) + N2*(2*sqrtl(2.0) - 3))/(sqrtl(2.0) - 2); // eqn. above (6)
      
      long double D = N1*sqrtl(Eta)/(N2 + sqrtl(Eta)); // eqn. below (5)
      
      LW = -1.0 + sqrtl(Eta)/(1.0 + sqrtl(Eta)/(3.0 + D)); // eqn. (5)
    } else {
      // The case x > 20:
      long double h = expl(-1.124491989777808/(0.4225028202459761+logl(x))); // eqn. (7)
      LW = logl(x/logl(x/powl(logl(x), h))); // eqn. (8)
    }
    
  
    // Now do some iterations:
    unsigned int NIterations = 2; // if the statements in the paper are correct two passes should be enough
    for (unsigned int i = 0; i < NIterations; ++i) {
      long double zn = logl(x/LW) - LW;  // eqn. 13 + 2
      long double en = (zn/(1+LW))*((2*(1 + LW)*(1 + LW + 2.0/3.0*zn) - zn)/(2*(1+LW)*(1 + LW + 2.0/3.0*zn) - 2*zn)); // eqn. 13 + 1
      LW = LW*(1.0 + en); // eqn. 13
    }
  } else if (Branch == -1) {

    // Approximation according to:
    // Chapeau-Blondeau & Monir, IEEE Transactions on signal processing, v. 50, #9, p. 2160, 2002

    if (x >= -1/TMath::E() && x < -0.333) {
      long double p = -sqrtl(2*(TMath::E()*x + 1));
      LW = -1 + p - 1.0/3.0*powl(p, 2) + 11.0/72.0*powl(p, 3) - 43.0/540.0*powl(p, 4) + 769.0/17280.0*powl(p, 5) - 221.0/8505*powl(p, 6);
    } else if (x >= -0.333 && x <= -0.033) {
      LW = (-8.0960+391.0025*x-47.4252*x*x - 4877.6330*powl(x, 3) - 5532.7760*powl(x, 4))/(1 - 82.9423*x + 433.8688*powl(x, 2) + 1515.3060*powl(x, 3));
    } else if (x >= -0.333 && x < 0) {
      long double l1 = logl(-x);
      long double l2 = logl(-logl(-x));
      LW = l1 -l2 + l2/l1 + (-2 + l2)*l2/(2*l1*l1) + (6 - 9*l2 + 2*l2*l2)*l2/(6*l1*l1*l1) + (-12 + 36*l2 - 22*l2*l2 + 3*l2*l2*l2)*l2/(12*l1*l1*l1*l1) + (60 - 300*l2 + 350*l2*l2 - 125*l2*l2*l2 + 12*l2*l2*l2*l2)*l2/(60*l1*l1*l1*l1*l1);
    } else {
      cout<<"This branch (\"-1\") of the LabertW function is only defined within ["<<-1/TMath::E()<<";0[! Input is "<<x<<". Returning zero..."<<endl;
      LW = 0;
    }

  } else {
    merr<<"Only branches 0 and -1 of the Lambert W function are implemented. Returning zero..."<<endl;
  }
  
  return LW;
}


// MFunction.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
