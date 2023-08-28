/*
 * MFunctionTime.cxx
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
// MFunctionTime
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFunctionTime.h"

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
ClassImp(MFunctionTime)
#endif


////////////////////////////////////////////////////////////////////////////////


const unsigned int MFunctionTime::c_InterpolationUnknown  = 0;
const unsigned int MFunctionTime::c_InterpolationLinLin   = 1;


////////////////////////////////////////////////////////////////////////////////


MFunctionTime::MFunctionTime() : m_InterpolationType(c_InterpolationLinLin)
{
  // Construct an instance of MFunctionTime

  m_YNonNegative = true;
}


////////////////////////////////////////////////////////////////////////////////


MFunctionTime::MFunctionTime(const MFunctionTime& F)
{
  // Copy-construct an instance of MFunctionTime

  m_InterpolationType = F.m_InterpolationType;

  m_X = F.m_X;
  m_Y = F.m_Y;
  m_Cumulative = F.m_Cumulative;
  m_YNonNegative = F.m_YNonNegative;
}


////////////////////////////////////////////////////////////////////////////////


MFunctionTime::~MFunctionTime()
{
  // Delete this instance of MFunctionTime
}


////////////////////////////////////////////////////////////////////////////////


const MFunctionTime& MFunctionTime::operator=(const MFunctionTime& F)
{
  // Copy-construct an instance of MFunctionTime

  m_InterpolationType = F.m_InterpolationType;

  m_X = F.m_X;
  m_Y = F.m_Y;
  m_Cumulative = F.m_Cumulative;
  m_YNonNegative = F.m_YNonNegative;

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


bool MFunctionTime::Set(const MString FileName, const MString KeyWord)
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

      m_X.push_back(Parser.GetTokenizerAt(i)->GetTokenAtAsTime(1));
      m_Y.push_back(Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(2));

    } else if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "IP") == true) {
      if (Parser.GetTokenizerAt(i)->GetNTokens() != 2) {
        mout<<"In the function defined by: "<<FileName<<endl;
        mout<<"Wrong number of arguments for IP keyword!"<<endl;
        return false;
      } 

      MString IP = Parser.GetTokenizerAt(i)->GetTokenAt(1);
      IP.ToLower();
      if (IP == "lin" || IP == "linlin") {
        m_InterpolationType = c_InterpolationLinLin;
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
  
  if (GetYMin() < 0) {
    m_YNonNegative = false;
  } else {
    m_YNonNegative = true;
  }

  // Clean up:
  m_Cumulative.clear();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFunctionTime::Save(const MString FileName, const MString Keyword)
{
  // Save the data:

  ofstream out;
  out.open(FileName);
  if (out.is_open() == false) {
    mout<<"Unable to open file \""<<FileName<<"\" for writting."<<endl;
    return false;
  }
  
  if (m_InterpolationType == c_InterpolationLinLin) {
    out<<"IP LinLin"<<endl;
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


bool MFunctionTime::Set(const vector<MTime>& X, const vector<double>& Y, unsigned int InterpolationType)
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

  return true;
}



////////////////////////////////////////////////////////////////////////////////


bool MFunctionTime::Add(const MTime& x, const double y)
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


void MFunctionTime::ScaleY(double Scaler)
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


void MFunctionTime::ScaleX(double Scaler)
{
  // Multiple the x-axis by some value

   for (unsigned int i = 0; i < m_X.size(); ++i) {
    m_X[i] *= Scaler;
  }  

  // We clear the cumulative function:
  m_Cumulative.clear();
}


////////////////////////////////////////////////////////////////////////////////


double MFunctionTime::Evaluate(const MTime& x) const
{
  // Evalute the function considering the different interpolation types

  if (m_X.size() < 2) {
    merr<<"This function contains not enough data points for evaluation!"<<show;
    massert(false);
    return 0;
  }

  if (m_InterpolationType == c_InterpolationLinLin) {

    double y = 0.0;

    int Position = -1; 
    for (unsigned int i = 0; i < m_X.size(); ++i) {
      if (m_X[i] > x) {
        break;
      } 
      Position = (int) i;
    }
    
    MTime x1, x2;
    double y1, y2;
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

    double m = (y2-y1)/(x2-x1).GetAsDouble();
    double t = y2 - m*x2.GetAsDouble();

    y = m*x.GetAsDouble()+t;
    
    if (std::isnan(y)) { // std:: is required here due to multiple definitions
      merr<<"Interpolation error for interpolation type "<<m_InterpolationType<<": y is NaN!"<<endl;;
      merr<<"   m="<<m<<"  t="<<t<<"  x1="<<x1<<"  y1="<<y1<<"  x2="<<x2<<"  y2="<<y2<<show;
    }

    return y;
  } 

  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


double MFunctionTime::Integrate() const
{
  // Integrate all the data from min to max

  return Integrate(m_X.front(), m_X.back());
}

////////////////////////////////////////////////////////////////////////////////


double MFunctionTime::Integrate(MTime XMin, MTime XMax) const
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
    //BinMin = find_if(m_X.begin(), m_X.end(), bind(greater<double>(), placeholders::_1, XMin)) - m_X.begin() - 1;
    auto I = std::lower_bound(m_X.begin(), m_X.end(), XMin);
    if (I == m_X.begin()) {
      BinMin = 0;
    } else if (I == m_X.end()) {
      BinMin = m_X.size();
    } else {
      BinMin = distance(m_X.begin(), I);
    }
  }
  int BinMax = m_X.size()-1;
  if (XMax < m_X.back()) {
    //BinMax = find_if(m_X.begin(), m_X.end(), bind(greater_equal<double>(), placeholders::_1, XMax)) - m_X.begin();
    auto I = lower_bound(m_X.begin(), m_X.end(), XMax);
    if (I == m_X.begin()) {
      BinMax = 0;
    } else if (I == m_X.end()) {
      BinMax = m_X.size();
    } else {
      BinMax = distance(m_X.begin(), I);
    }
  }
  
  double Integral = 0.0;
  if (m_InterpolationType == c_InterpolationLinLin) {
    
    MTime x1, x2;
    double y1, y2;
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
    
      // Calculate m, t of the interpolation "line" (it's always line in the respective mode) 
      double m = (y2-y1)/(x2-x1).GetAsDouble();
      double t = y2 - m*x2.GetAsDouble();
      
      // Do the integration using the correct formula for the various modi:
      if (m_InterpolationType == c_InterpolationLinLin) {
        // y=m*x+t --> int(m*x+t)
        Integral += 0.5*m*(x2*x2-x1*x1).GetAsDouble() + t*(x2-x1).GetAsDouble();
      }
    }

  } else {
    merr<<"This interpolation type is no longer supported..."<<endl;
  }

  return Integral;
}


////////////////////////////////////////////////////////////////////////////////

/*
double MFunctionTime::GetRandom()
{
  // Return a random number distributed as the underlying function

  if (m_YNonNegative == false) {
    merr<<"GetRandom only works if all y-values are positive ot zero."<<show;      
    return 0;
  }

  // Check if we have to determine the cumulative function:
  if (m_Cumulative.size() == 0) {
    if (m_Y.size() > 5000) {
      mout<<"MFunctionTime: Determining the cumulative function --- this can take a very long time, especially when MEGAlib was compiled in debug mode..."<<endl;
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
*/

////////////////////////////////////////////////////////////////////////////////

/*
double MFunctionTime::GetRandomTimesX()
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
*/

////////////////////////////////////////////////////////////////////////////////

/*
double MFunctionTime::GetRandomInterpolate(double Itot)
{
  // Second stage of the GetRandom function
  // A random number on the total intensity scale, Itot, is already found
  // Now find the correct x-value via interpolation

  // Find the correct bin in m_Cumulative
  //int Bin = find_if(m_Cumulative.begin(), m_Cumulative.end(), bind2nd(greater_equal<double>(), Itot)) - m_Cumulative.begin();
  int Bin = find_if(m_Cumulative.begin(), m_Cumulative.end(), bind(greater_equal<double>(), placeholders::_1, Itot)) - m_Cumulative.begin();


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
*/


////////////////////////////////////////////////////////////////////////////////


MTime MFunctionTime::GetXMin() const
{
  //! Get the minimum x-value

  return m_X.front();
}


////////////////////////////////////////////////////////////////////////////////


MTime MFunctionTime::GetXMax() const
{
  //! Get the maximum x-value

  return m_X.back();
}


////////////////////////////////////////////////////////////////////////////////


double MFunctionTime::GetYMin() const
{
  //! Get the minimum y-value

  double Min = numeric_limits<double>::max();
  for (unsigned int i = 0; i < m_Y.size(); ++i) {
    if (m_Y[i] < Min) Min = m_Y[i];
  }

  return Min;
}


////////////////////////////////////////////////////////////////////////////////


double MFunctionTime::GetYMax() const
{
  //! Get the maximum y-value

  double Max = -numeric_limits<double>::max();
  for (unsigned int i = 0; i < m_Y.size(); ++i) {
    if (m_Y[i] > Max) Max = m_Y[i];
  }

  return Max;
}


////////////////////////////////////////////////////////////////////////////////


MTime MFunctionTime::FindX(const MTime& XStart, double Integral, bool Cyclic)
{
  //! Find the x value starting from Start which would be achieved after integrating for "Integral"
  //! If we go beyond x_max, x_max is returned if we are not cyclic, otherwise we continue at x_0

  //cout<<"XStart: "<<XStart<<"  Integral: "<<Integral<<endl;
  
  MTime Modulo = 0;
  if (Cyclic == true) {
    // Project XStart into the frame of this function
    MTime Front = m_X.front();
    MTime Back = m_X.back();
    
    if (XStart < Front) {
      Modulo = double(((Front - XStart)/(Back-Front)).GetAsLong() + 1) * (Back - Front);
    } else if (XStart > Back) {
      Modulo = -double(((XStart - Back)/(Back-Front)).GetAsLong() + 1) * (Back - Front);
    }
  }

  MTime X = XStart + Modulo;

  cout<<"Moduloed time: "<<X<<" ( real time: "<<XStart<<" lower bin: "<<m_X.front()<<", upper value: "<<m_X.back()<<" )"<<endl;

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
    //BinStart = find_if(m_X.begin(), m_X.end(), bind2nd(greater<double>(), X)) - m_X.begin() - 1;
    //BinStart = find_if(m_X.begin(), m_X.end(), bind(greater<double>(), placeholders::_1, X)) - m_X.begin() - 1;
    auto I = std::lower_bound(m_X.begin(), m_X.end(), X);
    if (I == m_X.begin()) {
      BinStart = 0;
    } else if (I == m_X.end()) {
      BinStart = m_X.size();
    } else {
      BinStart = distance(m_X.begin(), I);
    }
  }

  cout<<"x: "<<X<<" Bin start: "<<BinStart<<" of "<<m_X.size()<<" bins"<<endl;
  
  unsigned int NewUpperBin = BinStart;
  double tIntegral = 0.0;
  double iIntegral = 0.0;
  do {
    NewUpperBin++;
    tIntegral = Integrate(X, m_X[NewUpperBin]);
    cout<<"Int from "<<X<<" to "<<m_X[NewUpperBin]<<": "<<tIntegral<<" (total: "<<iIntegral<<")"<<endl;
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
  if (X == m_X.back() && iIntegral < Integral) return numeric_limits<double>::max();
  
  cout<<"UpperBin: "<<NewUpperBin<<" (Intgeral:"<<Integral<<")"<<endl;
  
  // Step 2: Interpolate --- only linear at the moment --- within the given bin to find the right x-value
  
  MTime m = (m_Y[NewUpperBin-1] - m_Y[NewUpperBin]) / (m_X[NewUpperBin-1] - m_X[NewUpperBin]);
  MTime t = m_Y[NewUpperBin] - m*m_X[NewUpperBin];
  
  //cout<<"m: "<<m<<" t: "<<t<<endl;
  
  MTime x1 = 0;
  MTime x2 = 0;
  
  if (m != 0) {
    MTime a = 0.5*m;
    MTime b = t;
    MTime c = -((Integral-iIntegral) + 0.5*m*X*X + t*X);
  
    x1 = (-b-sqrt(b*b-4*a*c))/(2*a);
    x2 = (-b+sqrt(b*b-4*a*c))/(2*a);
  } else {
    x1 = X + (Integral-iIntegral)/t; // t cannot be null here other wise we would have jumped the bin...
    x2 = MTime::Max();
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
  
  cout<<"XStart: "<<XStart<<" X: "<<X<<" modulo: "<<Modulo<<endl;
  
  return X - Modulo;
}


////////////////////////////////////////////////////////////////////////////////


void MFunctionTime::Plot()
{
  // Plot the function in a Canvas (diagnostics only)
  
  if (m_X.size() >= 2) {
    //double Dist = m_X.back()-m_X.front();
    //TH1D* Hist = new TH1D("Diagnostics", "Diagnostics", 10000, m_X.front()-0.1*Dist, m_X.back()+0.1*Dist);
    
    TH1D* Hist = new TH1D("Diagnostics", "Diagnostics", 10000, m_X.front().GetAsDouble(), m_X.back().GetAsDouble());
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


// MFunctionTime.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
