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
#include <iomanip>
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


MFunction::MFunction() 
  : m_InterpolationType(c_InterpolationLinLin)
{
  // Construct an instance of MFunction

  m_Spline3 = 0;
  m_Spline5 = 0;

  m_YNonNegative = true;
}


////////////////////////////////////////////////////////////////////////////////


MFunction::MFunction(const MFunction& F)
{
  // Copy-construct an instance of MFunction

  m_InterpolationType = F.m_InterpolationType;

  m_X = F.m_X;
  m_XZero = F.m_XZero;
  m_Y = F.m_Y;
  m_Cumulative = F.m_Cumulative;
  m_YNonNegative = F.m_YNonNegative;

  m_Spline3 = 0;
  m_Spline5 = 0;
  if (m_InterpolationType == c_InterpolationSpline3 || m_InterpolationType == c_InterpolationSpline5) { 
    CreateSplines();
  }
}


////////////////////////////////////////////////////////////////////////////////


MFunction::~MFunction()
{
  // Delete this instance of MFunction

  delete m_Spline3;
  delete m_Spline5;
}


////////////////////////////////////////////////////////////////////////////////


const MFunction& MFunction::operator=(const MFunction& F)
{
  // Copy-construct an instance of MFunction

  m_InterpolationType = F.m_InterpolationType;

  m_X = F.m_X;
  m_XZero = F.m_XZero;
  m_Y = F.m_Y;
  m_Cumulative = F.m_Cumulative;
  m_YNonNegative = F.m_YNonNegative;

  m_Spline3 = 0;
  m_Spline5 = 0;
  if (m_InterpolationType == c_InterpolationSpline3 || m_InterpolationType == c_InterpolationSpline5) { 
    CreateSplines();
  }

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
      } else if (IP == "linlog") {
        m_InterpolationType = c_InterpolationLinLog;
      } else if (IP == "loglin") {
        m_InterpolationType = c_InterpolationLogLin;
      } else if (IP == "log" || IP == "loglog") {
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

  // Determine interapolation type:
  if (m_InterpolationType == c_InterpolationSpline5 || m_InterpolationType == c_InterpolationSpline3) {
    mimp<<"Currently spline is no longer supported, thus switch back to linear interpolation"<<endl;
    m_InterpolationType = c_InterpolationLinLin;
  }

  if (m_X.size() < 5 && m_InterpolationType == c_InterpolationSpline5) {
    m_InterpolationType = c_InterpolationSpline3;
  }

  if (m_X.size() < 3 && m_InterpolationType == c_InterpolationSpline3) {
    m_InterpolationType = c_InterpolationLinLin;
  }

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

  if (m_InterpolationType == c_InterpolationSpline3 || m_InterpolationType == c_InterpolationSpline5) { 
    CreateSplines();
  }

  for (auto x: m_X) {
    m_XZero.push_back(x - m_X[0]);
  }

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

  for (auto x: m_X) {
    m_XZero.push_back(x - m_X[0]);
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFunction::Set(const vector<double>& X, const vector<double>& Y, unsigned int InterpolationType)
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

  for (auto x: m_X) {
    m_XZero.push_back(x - m_X[0]);
  }

  return true;
}



////////////////////////////////////////////////////////////////////////////////


bool MFunction::Add(const double x, const double y)
{
  //! Add a data point

  if (m_X.size() == 0) {
    m_X.push_back(x);
    m_Y.push_back(y);
    m_XZero.push_back(x - m_X[0]);
  } else {
    if (x < m_X[0]) {
      m_X.insert(m_X.begin(), x);
      m_Y.insert(m_Y.begin(), y);
      m_XZero.insert(m_X.begin(), x - m_X[0]);
    } else if (x > m_X.back()) {
      m_X.push_back(x);
      m_Y.push_back(y);
      m_XZero.push_back(x - m_X[0]);
    } else {
      for (unsigned int i = 0; i < m_X.size(); ++i) {
        if (x == m_X[i]) {
          mout<<"   ***  Warning  ***  "<<endl;
          mout<<"X-value (x="<<x<<", y="<<y<<") defined twice! Replacing the original value!"<<endl;
          m_Y[i] = y;
        } else if (x < m_X[i]) {
          m_X.insert(m_X.begin()+i, x);
          m_Y.insert(m_Y.begin()+i, y);
          m_XZero.insert(m_XZero.begin()+i, x - m_X[0]);
          break;
        }
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MFunction::ScaleY(double Scaler)
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

  if (m_InterpolationType == c_InterpolationSpline3 || m_InterpolationType == c_InterpolationSpline5) { 
    CreateSplines();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MFunction::ScaleX(double Scaler)
{
  // Multiple the x-axis by some value

  for (unsigned int i = 0; i < m_X.size(); ++i) {
    m_X[i] *= Scaler;
    m_XZero[i] = m_X[i] - m_X[0];
  }

  // We clear the cumulative function:
  m_Cumulative.clear();

  if (m_InterpolationType == c_InterpolationSpline3 || m_InterpolationType == c_InterpolationSpline5) { 
    CreateSplines();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MFunction::CreateSplines()
{
  // Create the splines for interpolation

    // Generate spline fit:
  if (m_InterpolationType == c_InterpolationSpline3 || m_InterpolationType == c_InterpolationSpline5) {
    double* x = new double[m_X.size()];
    double* y = new double[m_Y.size()];
    
    for (unsigned int i = 0; i < m_X.size(); ++i) {
      x[i] = m_X[i];
      y[i] = m_Y[i];
    } 

    if (m_InterpolationType == c_InterpolationSpline3) {
      delete m_Spline3;
      m_Spline3 = new TSpline3("", x, y, m_X.size());  
    } else {
      delete m_Spline5;
      m_Spline5 = new TSpline5("", x, y, m_X.size());
      //m_Spline5->Draw("LP");
    }

    delete [] x;
    delete [] y;
  }
}


////////////////////////////////////////////////////////////////////////////////


double MFunction::Eval(double x) const 
{ 
  mdep<<"MFunction::Eval is deprecated, replace with: MFunction::Evaluate"<<show;
  return Evaluate(x); 
}


////////////////////////////////////////////////////////////////////////////////


double MFunction::Evaluate(double x) const
{
  // Evalute the function considering the different interpolation types

  auto InterpolationType = m_InterpolationType;

  if (InterpolationType == c_InterpolationUnknown) {
    merr<<"Unknown interpolation type: "<<c_InterpolationUnknown<<show;
    return 0;
  }

  if (m_X.size() == 0) {
    merr<<"This function contains no data points for evaluation!"<<show;
    massert(false);
    return 0;
  }

  if (InterpolationType == c_InterpolationLinLog) {
    if (m_X.back() > 1E+09) {
      merr<<"Info: Large x-axis values make LinLog interpolation fail. Switching to LinLin!"<<endl;
      InterpolationType = c_InterpolationLinLin;
    }
  }

  if (InterpolationType == c_InterpolationConstant || m_X.size() == 1) {
    return m_Y[0];
  } else if (InterpolationType == c_InterpolationSpline3) {
    return m_Spline3->Eval(x);
  } else if (InterpolationType == c_InterpolationSpline5) {
    return m_Spline5->Eval(x);
  } else if (InterpolationType == c_InterpolationNone) {

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

  } else if (InterpolationType == c_InterpolationLinLin ||
             InterpolationType == c_InterpolationLinLog ||
             InterpolationType == c_InterpolationLogLin ||
             InterpolationType == c_InterpolationLogLog) {

    double y = 0.0;

    int Position = -1; 
    for (unsigned int i = 0; i < m_X.size(); ++i) {
      if (m_X[i] > x) {
        break;
      } 
      Position = (int) i;
    }
    
    double x1, x2, y1, y2;
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

    // Attention for log interpolation make sure all values are positive!
    if (InterpolationType == c_InterpolationLinLog || InterpolationType == c_InterpolationLogLog) {
      y1 = log(y1);
      y2 = log(y2);
    }
    if (InterpolationType == c_InterpolationLogLin || InterpolationType == c_InterpolationLogLog) {
      x = log(x);
      x1 = log(x1);
      x2 = log(x2);
    }

    double m = (y2-y1)/(x2-x1);
    double t = y2 - m*x2;

    if (InterpolationType == c_InterpolationLinLog || InterpolationType == c_InterpolationLogLog) {
      y = exp(m*x+t);
    } else {
      y = m*x+t;
    }
    
    if (std::isnan(y)) { // std:: is required here due to multiple definitions
      merr<<"Interpolation error for interpolation type "<<InterpolationType<<": y is NaN!"<<endl;;
      merr<<"   m="<<m<<"  t="<<t<<"  x1="<<x1<<"  y1="<<y1<<"  x2="<<x2<<"  y2="<<y2<<show;
    }

    return y;
  } 

  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


double MFunction::Integrate() const
{
  // Integrate all the data from min to max

  return Integrate(m_X.front(), m_X.back());
}

////////////////////////////////////////////////////////////////////////////////


double MFunction::Integrate(double XMin, double XMax) const
{
  // Integrate the data from min to max
  
  auto InterpolationType = m_InterpolationType;

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

  // Switch to XZero to handle very large numbers
  XMin -= m_X.front();
  XMax -= m_X.front();

  // ... use m_XZero from now on for x-axis

  // More sanity checks
  if (InterpolationType == c_InterpolationLinLog) {
    if (m_X.back() > 1E+09) {
      merr<<"Info: Large x-axis values make LinLog interpolation fail. Switching to LinLin!"<<endl;
      InterpolationType = c_InterpolationLinLin;
    }
  }

  int BinMin = 0;
  if (XMin > m_XZero.front()) {
    // BinMin = find_if(m_XZero.begin(), m_XZero.end(), bind2nd(greater<double>(), XMin)) - m_XZero.begin() - 1;
    BinMin = find_if(m_XZero.begin(), m_XZero.end(), bind(greater<double>(), placeholders::_1, XMin)) - m_XZero.begin() - 1;
//     unsigned int upper = m_Cumulative.size();
//     unsigned int center = 1;
//     unsigned int lower = 0;
    
//     while (upper-lower > 1) {
//       center = (upper+lower) >> 1;
//       if (XMin == m_XZero[center]) {
//         BinMin = int(center)+1;
//       }
//       if (XMin < m_XZero[center]) {
//         upper = center;
//       } else {
//         lower = center;
//       }
//     }
//     BinMin = int(lower)+1;
  }
  int BinMax = m_XZero.size()-1;
  if (XMax < m_XZero.back()) {
    //BinMax = find_if(m_XZero.begin(), m_XZero.end(), bind2nd(greater_equal<double>(), XMax)) - m_XZero.begin();
    BinMax = find_if(m_XZero.begin(), m_XZero.end(), bind(greater_equal<double>(), placeholders::_1, XMax)) - m_XZero.begin();
//     unsigned int upper = m_Cumulative.size();
//     unsigned int center = 1;
//     unsigned int lower = 0;
    
//     while (upper-lower > 1) {
//       center = (upper+lower) >> 1;
//       if (XMax == m_XZero[center]) {
//         BinMax = int(center)+1;
//       }
//       if (XMax < m_XZero[center]) {
//         upper = center;
//       } else {
//         lower = center;
//       }
//     }
//     BinMax = int(lower)+1;
  }
  
  double Integral = 0.0;
  if (InterpolationType == c_InterpolationConstant) {
    Integral = (XMax - XMin) * m_Y[0];
  } else if (InterpolationType == c_InterpolationNone) {
    // Just sum rectangular bins:

    double BinCenterMin = m_XZero[BinMin-1] + 0.5*(m_XZero[BinMin] - m_XZero[BinMin-1]);
    double BinCenterMax = m_XZero[BinMax-1] + 0.5*(m_XZero[BinMax] - m_XZero[BinMax-1]);

    if (BinMin != BinMax) {
      if (XMin < BinCenterMin) {
        Integral += (BinCenterMin - XMin)*m_Y[BinMin-1];
        Integral += (m_XZero[BinMin] - BinCenterMin)*m_Y[BinMin];
      } else {
        Integral += (m_XZero[BinMin] - XMin)*m_Y[BinMin];
      }
      for (int b = BinMin; b < BinMax; ++b) {
        Integral += 0.5*(m_XZero[BinMin+1] - m_XZero[BinMin])*m_Y[BinMin];
        Integral += 0.5*(m_XZero[BinMin+1] - m_XZero[BinMin])*m_Y[BinMin+1];
      }
      if (XMax > BinCenterMax) {
        Integral += (BinCenterMax - m_XZero[BinMax-1])*m_Y[BinMax-1];
        Integral += (XMax - BinCenterMax)*m_Y[BinMax];
      } else {
        Integral += (XMax - m_XZero[BinMax-1])*m_Y[BinMax-1];
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
  } else if (InterpolationType == c_InterpolationLinLin ||
             InterpolationType == c_InterpolationLinLog ||
             InterpolationType == c_InterpolationLogLin ||
             InterpolationType == c_InterpolationLogLog) {
    
    double x1, x2, y1, y2;
    for (int i = BinMin; i < BinMax; ++i) {

      if (i == BinMin) {
        x1 = XMin;
        y1 = Evaluate(x1);
      } else {
        x1 = m_XZero[i];
        y1 = m_Y[i];
      }
      if (i == BinMax) {
        x2 = XMax;
        y2 = Evaluate(x2);
      } else {
        x2 = m_XZero[i+1];
        y2 = m_Y[i+1];
      }
      
      // Attention for log interpolation make sure all values are positive!
      if (InterpolationType == c_InterpolationLinLog ||
          InterpolationType == c_InterpolationLogLog) {
        y1 = log(y1);
        y2 = log(y2);
      }
      if (InterpolationType == c_InterpolationLogLin ||
          InterpolationType == c_InterpolationLogLog) {
        x1 = log(x1);
        x2 = log(x2);
      }
      
      //cout<<x1<<":"<<x2<<endl;
    
      // Calculate m, t of the interpolation "line" (it's always line in the respective mode) 
      double m = (y2-y1)/(x2-x1);
      double t = y2 - m*x2;

      // Switch back
      if (InterpolationType == c_InterpolationLinLog ||
          InterpolationType == c_InterpolationLogLog) {
        y1 = exp(y1);
        y2 = exp(y2);
      }
      if (InterpolationType == c_InterpolationLogLin ||
          InterpolationType == c_InterpolationLogLog) {
        x1 = exp(x1);
        x2 = exp(x2);
      }
      
      // Do the integration using the correct formula for the various modi:
      if (InterpolationType == c_InterpolationLinLin) {
        // y=m*x+t --> int(m*x+t)
        Integral += 0.5*m*(x2*x2-x1*x1) + t*(x2-x1);      
      } else if (InterpolationType == c_InterpolationLinLog) {
        // ln(y) = m*x+t --> int(exp(m*x+t))
        if (fabs(m) < 10E-8) {
          Integral += exp(t)*(x2-x1); 
          cout<<"x1:"<<x1<<" x2:"<<x2<<" y1:"<<y1<<" y2:"<<y2<<" m:"<<m<<" t:"<<t<<":"<<exp(t)<<":"<<x2-x1<<endl;
        } else {
          Integral += 1/m * (exp(m*x2+t) - exp(m*x1+t));
          cout<<1/m<<":"<<exp(m*x2+t)<<":"<<exp(m*x1+t)<<endl;
        }
      } else if (InterpolationType == c_InterpolationLogLin) {
        // y = m*ln(x) + t --> int(m*ln(x) + t)
        Integral += x2*(m*(log(x2) - 1) + t) - x1*(m*(log(x1) - 1) + t);
      } else if (InterpolationType == c_InterpolationLogLog) {
        // ln(y) = m*ln(x) + t --> y = exp(t)*x^m --> int(exp(t)*x^m)
        if (fabs(m+1) < 10E-8) {
          Integral += exp(t)*(log(x2)-log(x1)); 
        } else {
          Integral += exp(t)/(m+1) * (pow(x2, m+1) - pow(x1, m+1));
        }
      }
      if (std::isfinite(Integral) == false) {
        merr<<"Error: Integral is no longer finite at bin: "<<i<<endl;
      }
    }
  } else {
    merr<<"This interpolation type is no longer supported..."<<endl;
  }

  return Integral;
}


////////////////////////////////////////////////////////////////////////////////


double MFunction::GetRandom()
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


double MFunction::GetRandomTimesX()
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
  return GetRandomInterpolate(sqrt(gRandom->Rndm())*m_Cumulative.back());
}

////////////////////////////////////////////////////////////////////////////////


double MFunction::GetRandomInterpolate(double Itot)
{
  // Second stage of the GetRandom function
  // A random number on the total intensity scale, Itot, is already found
  // Now find the correct x-value via interpolation

  // Find the correct bin in m_Cumulative
  //int Bin = find_if(m_Cumulative.begin(), m_Cumulative.end(), bind2nd(greater_equal<double>(), Itot)) - m_Cumulative.begin();
  int Bin = find_if(m_Cumulative.begin(), m_Cumulative.end(), bind(greater_equal<double>(), placeholders::_1, Itot)) - m_Cumulative.begin();

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
    double I = Itot - m_Cumulative[Bin-1];
    // Absolute intensity in this bin
    double A = m_Cumulative[Bin] - m_Cumulative[Bin-1];
    
    return m_X[Bin-1] + (I/A)*(m_X[Bin] - m_X[Bin-1]);

  } else if (m_InterpolationType == c_InterpolationLinLin ||
             m_InterpolationType == c_InterpolationLinLog ||
             m_InterpolationType == c_InterpolationLogLin ||
             m_InterpolationType == c_InterpolationLogLog) {

    double x1, x2, y1, y2;
    x1 = m_X[Bin-1];
    y1 = m_Y[Bin-1];
    x2 = m_X[Bin];
    y2 = m_Y[Bin];
    
    // Attention for log interpolation make sure all values are positive!
    if (m_InterpolationType == c_InterpolationLinLog || m_InterpolationType == c_InterpolationLogLog) {
      y1 = log(y1);
      y2 = log(y2);
    }
    if (m_InterpolationType == c_InterpolationLogLin || m_InterpolationType == c_InterpolationLogLog) {
      x1 = log(x1);
      x2 = log(x2);
    }
    
    // Calculate m, t of the interpolation "line" (it's always a line in the respective mode) 
    double m = (y2-y1)/(x2-x1);
    double t = y2 - m*x2;
    
    // Switch back
    if (m_InterpolationType == c_InterpolationLinLog || m_InterpolationType == c_InterpolationLogLog) {
      y1 = exp(y1);
      y2 = exp(y2);
    }
    if (m_InterpolationType == c_InterpolationLogLin || m_InterpolationType == c_InterpolationLogLog) {
      x1 = exp(x1);
      x2 = exp(x2);
    }

    // Relative intensity in this bin:
    double I  = Itot - m_Cumulative[Bin-1];

    if (m_InterpolationType == c_InterpolationLinLin) {
      // We know m, t (from y=m*x+t), as well as I = Itot - I[Bin-1], i.e. the covered area in the given bin
      // What we want is x:
      // I = int(x'[Bin-1]->x) m*x'+t dx'
      // x = (-t+-sqrt(t*t+m*m*x0*x0+2*m*t*x0+2*m*I))/m
      
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
      double Value = (t+m*x1)*(t+m*x1) + 2*m*I;
  
      // the possible solutions
      double xs1 = (-t-sqrt(Value))/m;
      double xs2 = (-t+sqrt(Value))/m;
      
      double x = 0.0;
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
      // Here we look for the solution if I = int[x'[Bin-1]->x] exp(m*x'+t) dx'
      // Fortunately that's easier, since we have only one valid solution
      
      // Special case:
      if (fabs(m) < 10E-8) { // m == 0
        return x1 + I*exp(-t);
      }

      return (log(exp(x1*m+t) + I*m) - t)/m;

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
      
      double LW = 0.0;
      // That's just a guess --- not sure is it is not a function of (m*x1*log(x1)-m*x1+t*x1+I)/m*exp(-(m-t)/m
      if (m < 0) {
        LW = LambertW((m*x1*log(x1)-m*x1+t*x1+I)/m*exp(-(m-t)/m), -1);
      } else {
        LW = LambertW((m*x1*log(x1)-m*x1+t*x1+I)/m*exp(-(m-t)/m), 0);
      }
      
      return exp((LW*m+m-t)/m);
      
    } else if (m_InterpolationType == c_InterpolationLogLog) {
      // Here we look for the solution if I = int[x'[Bin-1]->x] exp(t)*x'^m dx'
      // Fortunately that's easier, since we have only one valid solution
      
      // Special case:
      if (fabs(m+1) < 10E-8) { // m == -1
        return (x1*exp(t) + I) * exp(-t);
      }
      
      return exp(-(t-log(pow(x1, m+1)*exp(t)+I*m+I))/(m+1));
    }
    
  } else {
    merr<<"This interpolation type is no longer supported..."<<endl;
    return 0.0;
  }

  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


double MFunction::GetXMin() const
{
  //! Get the minimum x-value

  return m_X.front();
}


////////////////////////////////////////////////////////////////////////////////


double MFunction::GetXMax() const
{
  //! Get the maximum x-value

  return m_X.back();
}


////////////////////////////////////////////////////////////////////////////////


double MFunction::GetYMin() const
{
  //! Get the minimum y-value

  double Min = numeric_limits<double>::max();
  for (unsigned int i = 0; i < m_Y.size(); ++i) {
    if (m_Y[i] < Min) Min = m_Y[i];
  }

  return Min;
}


////////////////////////////////////////////////////////////////////////////////


double MFunction::GetYMax() const
{
  //! Get the maximum y-value

  double Max = -numeric_limits<double>::max();
  for (unsigned int i = 0; i < m_Y.size(); ++i) {
    if (m_Y[i] > Max) Max = m_Y[i];
  }

  return Max;
}


////////////////////////////////////////////////////////////////////////////////


double MFunction::FindX(double XStart, double Integral, bool Cyclic)
{
  //! Find the x value starting from Start which would be achieved after integrating for "Integral"
  //! If we go beyond x_max, x_max is returned if we are not cyclic, otherwise we continue at x_0

  // To reign in larger number rounding issues, use the offset x-values offset everything

  double Modulo = 0;
  if (Cyclic == true) {
    // Project XStart into the frame of this function
    double Front = m_X.front();
    double Back = m_X.back();
    
    if (XStart < Front) {
      Modulo = double(int((Front - XStart)/(Back-Front)) + 1) * (Back - Front);
    } else if (XStart > Back) {
      Modulo = -double(int((XStart - Back)/(Back-Front)) + 1) * (Back - Front);
    }
  }

  double X = XStart + Modulo - m_X[0]; // To reign in larger number rounding issues, offset everything

  // Find the bin X is in: 
  if (X < m_XZero.front()) {
    //merr<<"XStart ("<<XStart<<") smaller than minimum x-value ("<<m_X.front()<<") --- starting at minimum x-value"<<endl;
    X = m_XZero.front();
  }
  if (X > m_XZero.back()) {
    //merr<<"XStart ("<<XStart<<") larger than maximum x-value ("<<m_X.back()<<") --- starting at minimum x-value"<<endl;
    X = m_XZero.front();
  }

  // Step 1: Go from bin to bin until we find an upper limit bin, where iIntegral > I

  unsigned int BinStart = 0;
  if (X > m_XZero.front()) {
    //BinStart = find_if(m_XZero.begin(), m_XZero.end(), bind2nd(greater<double>(), X)) - m_XZero.begin() - 1;
    BinStart = find_if(m_XZero.begin(), m_XZero.end(), bind(greater<double>(), placeholders::_1, X)) - m_XZero.begin() - 1;
  }

  //cout<<"x: "<<X<<" Bin start: "<<BinStart<<endl;
  
  unsigned int NewUpperBin = BinStart;
  double tIntegral = 0.0;
  double iIntegral = 0.0;
  do {
    NewUpperBin++;
    tIntegral = Integrate(X+m_X[0], m_X[NewUpperBin]);
    //cout<<"Int from "<<X<<" to "<<m_X[NewUpperBin]<<": "<<tIntegral<<" (total: "<<iIntegral<<")"<<endl;
    if (iIntegral + tIntegral < Integral) {
      X = m_XZero[NewUpperBin];
      iIntegral += tIntegral;
    } else {
      //cout<<"Found it"<<endl;
      break;
    }
    if (X == m_XZero.back()) {
      if (Cyclic == false) {
        break;
      } else {
        X = 0;
        NewUpperBin = 0;
        Modulo -= m_XZero.back() - m_XZero.front();
      }
    }
  } while (true);
  
  // Non-cyclic exit case
  if (X == m_XZero.back() && iIntegral < Integral) return numeric_limits<double>::max();

  // Step 2: Interpolate --- only linear at the moment --- within the given bin to find the right x-value

  double m = (m_Y[NewUpperBin-1] - m_Y[NewUpperBin]) / (m_XZero[NewUpperBin-1] - m_XZero[NewUpperBin]);
  double t = m_Y[NewUpperBin] - m*m_XZero[NewUpperBin];
  
  double x1 = 0; 
  double x2 = 0;
  
  if (m != 0) {
    double a = 0.5*m;
    double b = t;
    double c = -((Integral-iIntegral) + 0.5*m*X*X + t*X);
  
    x1 = (-b-sqrt(b*b-4*a*c))/(2*a);
    x2 = (-b+sqrt(b*b-4*a*c))/(2*a);
  } else {
    x1 = X + (Integral-iIntegral)/t; // t cannot be null here other wise we would have jumped the bin...
    x2 = numeric_limits<double>::max();
  }
  //cout<<"x1: "<<x1<<" x2: "<<x2<<endl;
  
  if (x1 >= m_XZero[NewUpperBin-1] && x1 <= m_XZero[NewUpperBin] && (x2 < m_XZero[NewUpperBin-1] || x2 > m_XZero[NewUpperBin])) {
    //mout<<"x="<<x1<<endl;
    X = x1;
  } else if (x2 >= m_XZero[NewUpperBin-1] && x2 <= m_XZero[NewUpperBin] && (x1 < m_XZero[NewUpperBin-1] || x1 > m_XZero[NewUpperBin])) {
    //mout<<"x="<<x2<<endl;
    X = x2; 
  } else if ((x2 < m_XZero[NewUpperBin-1] || x2 > m_XZero[NewUpperBin]) && (x1 < m_XZero[NewUpperBin-1] || x1 > m_XZero[NewUpperBin])) {
    merr<<std::setprecision(20)<<"FindX: Both possible results are outside choosen bin ["<<m_XZero[NewUpperBin-1]<<"-"<<m_XZero[NewUpperBin]<<"] with bin ID="<<NewUpperBin-1<<" and y range ["<<m_Y[NewUpperBin-1]<<"-"<<m_Y[NewUpperBin]<<"]: x1="<<x1<<" x2="<<x2<<endl;
  } else {
    merr<<std::setprecision(20)<<"FindX: Both possible results are within choosen bin ["<<m_X[NewUpperBin-1]<<"-"<<m_X[NewUpperBin]<<"] with bin ID="<<NewUpperBin-1<<" and y range ["<<m_Y[NewUpperBin-1]<<"-"<<m_Y[NewUpperBin]<<"]: x1="<<x1<<" x2="<<x2<<endl;
  }
  
  //cout<<"XStart: "<<XStart<<" X: "<<X<<" modulo: "<<Modulo<<endl;
  
  return X - Modulo + m_X[0];
}


////////////////////////////////////////////////////////////////////////////////


void MFunction::Plot()
{
  // Plot the function in a Canvas (diagnostics only)
  
  if (m_X.size() >= 2) {
    //double Dist = m_X.back()-m_X.front();
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


double MFunction::LambertW(double x, int Branch) 
{
  // Implementation of Lambert's W function branches 0 and -1

  double LW = 0.0;

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
      double Eta = 2 + 2*TMath::E()*x;
      
      double N2 = 4.612634277343749*sqrt(sqrt(sqrt(Eta) + 1.09556884765625)); // eqn. (6) --> Eta -> sqrt(Eta)
      double N1 = (4 - 3*sqrt(2.0) + N2*(2*sqrt(2.0) - 3))/(sqrt(2.0) - 2); // eqn. above (6)
      
      double D = N1*sqrt(Eta)/(N2 + sqrt(Eta)); // eqn. below (5)
      
      LW = -1.0 + sqrt(Eta)/(1.0 + sqrt(Eta)/(3.0 + D)); // eqn. (5)
    } else {
      // The case x > 20:
      double h = exp(-1.124491989777808/(0.4225028202459761+log(x))); // eqn. (7)
      LW = log(x/log(x/pow(log(x), h))); // eqn. (8)
    }
    
  
    // Now do some iterations:
    unsigned int NIterations = 2; // if the statements in the paper are correct two passes should be enough
    for (unsigned int i = 0; i < NIterations; ++i) {
      double zn = log(x/LW) - LW;  // eqn. 13 + 2
      double en = (zn/(1+LW))*((2*(1 + LW)*(1 + LW + 2.0/3.0*zn) - zn)/(2*(1+LW)*(1 + LW + 2.0/3.0*zn) - 2*zn)); // eqn. 13 + 1
      LW = LW*(1.0 + en); // eqn. 13
    }
  } else if (Branch == -1) {

    // Approximation according to:
    // Chapeau-Blondeau & Monir, IEEE Transactions on signal processing, v. 50, #9, p. 2160, 2002

    if (x >= -1/TMath::E() && x < -0.333) {
      double p = -sqrt(2*(TMath::E()*x + 1));
      LW = -1 + p - 1.0/3.0*pow(p, 2) + 11.0/72.0*pow(p, 3) - 43.0/540.0*pow(p, 4) + 769.0/17280.0*pow(p, 5) - 221.0/8505*pow(p, 6);
    } else if (x >= -0.333 && x <= -0.033) {
      LW = (-8.0960+391.0025*x-47.4252*x*x - 4877.6330*pow(x, 3) - 5532.7760*pow(x, 4))/(1 - 82.9423*x + 433.8688*pow(x, 2) + 1515.3060*pow(x, 3));
    } else if (x >= -0.333 && x < 0) {
      double l1 = log(-x);
      double l2 = log(-log(-x));
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
