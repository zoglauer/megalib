/*
 * MFunction2D.cxx
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
// MFunction2D
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFunction2D.h"

// Standard libs:
#include <algorithm>
using namespace std;

// ROOT libs:
#include "TH2.h"
#include "TRandom.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TGraph2D.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MFunction2D)
#endif


////////////////////////////////////////////////////////////////////////////////


const unsigned int MFunction2D::c_InterpolationUnknown  = 0;
const unsigned int MFunction2D::c_InterpolationConstant = 1;
const unsigned int MFunction2D::c_InterpolationNone     = 2;
const unsigned int MFunction2D::c_InterpolationLinear   = 3;


////////////////////////////////////////////////////////////////////////////////


MFunction2D::MFunction2D() 
  : m_InterpolationType(c_InterpolationUnknown)
{
  // Construct an instance of MFunction2D
}


////////////////////////////////////////////////////////////////////////////////


MFunction2D::MFunction2D(const MFunction2D& F)
{
  // Copy-construct an instance of MFunction2D

  m_InterpolationType = F.m_InterpolationType;

  m_X = F.m_X;
  m_Y = F.m_Y;
  m_Z = F.m_Z;
  m_Maximum = F.m_Maximum;
  m_Cumulative = F.m_Cumulative;
}


////////////////////////////////////////////////////////////////////////////////


MFunction2D::~MFunction2D()
{
  // Delete this instance of MFunction2D
}


////////////////////////////////////////////////////////////////////////////////


const MFunction2D& MFunction2D::operator=(const MFunction2D& F)
{
  // Copy-construct an instance of MFunction2D

  m_InterpolationType = F.m_InterpolationType;

  m_X = F.m_X;
  m_Y = F.m_Y;
  m_Z = F.m_Z;
  m_Maximum = F.m_Maximum;
  m_Cumulative = F.m_Cumulative;

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


bool MFunction2D::Set(const MString FileName, const MString KeyWord, 
                    const unsigned int InterpolationType)
{
  // Set the basic data, load the file and parse it

  m_InterpolationType = InterpolationType;
  m_X.clear();
  m_Y.clear();
  m_Z.clear();

  MParser Parser;

  if (Parser.Open(FileName, MFile::c_Read) == false) {
    mout<<"Unable to open file "<<FileName<<endl;
    return false;
  }

  // Round one: Look for axis and interpolation type:
  for (unsigned int i = 0; i < Parser.GetNLines(); ++i) {
    if (Parser.GetTokenizerAt(i)->GetNTokens() == 0) continue;
    if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "XA") == true) {
      if (Parser.GetTokenizerAt(i)->GetNTokens() >= 2) {
        m_X = Parser.GetTokenizerAt(i)->GetTokenAtAsDoubleVector(1);
      } else {
        mout<<"In the function defined by: "<<FileName<<endl;
        mout<<"XA keyword has not enough parameters!"<<endl;
        return false;
      }
    } else if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "XB") == true) {
      if (Parser.GetTokenizerAt(i)->GetNTokens() == 4) {
        if (Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(2) <= Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(1)) {
          mout<<"In the function defined by: "<<FileName<<endl;
          mout<<"XB keyword: The second parameter must be larger than the first!"<<endl;
          return false;
        }
        if (Parser.GetTokenizerAt(i)->GetTokenAtAsInt(3) <= 1) {
          mout<<"In the function defined by: "<<FileName<<endl;
          mout<<"XB keyword: You need at least two bin centers (third parameter >= 2 not "<<Parser.GetTokenizerAt(i)->GetTokenAtAsInt(3)<<")!"<<endl;
          return false;
        }
        
        m_X.clear();
        for (int x = 0; x < Parser.GetTokenizerAt(i)->GetTokenAtAsInt(3); ++x) {
          m_X.push_back(Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(1) + x*(Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(2) - Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(1))/(Parser.GetTokenizerAt(i)->GetTokenAtAsInt(3)-1));
        }
      } else {
        mout<<"In the function defined by: "<<FileName<<endl;
        mout<<"XB keyword has not the correct number of parameters: "<<Parser.GetTokenizerAt(i)->GetNTokens()<<" instead of 4"<<endl;
        return false;
      }
    }

    if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "YA") == true) {
      if (Parser.GetTokenizerAt(i)->GetNTokens() >= 2) {
        m_Y = Parser.GetTokenizerAt(i)->GetTokenAtAsDoubleVector(1);
      } else {
        mout<<"In the function defined by: "<<FileName<<endl;
        mout<<"YA keyword has not enough parameters!"<<endl;
        return false;
      }
    } else if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "YB") == true) {
      if (Parser.GetTokenizerAt(i)->GetNTokens() == 4) {
        if (Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(2) <= Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(1)) {
          mout<<"In the function defined by: "<<FileName<<endl;
          mout<<"YB keyword: The second parameter must be larger than the first!"<<endl;
          return false;
        }
        if (Parser.GetTokenizerAt(i)->GetTokenAtAsInt(3) <= 1) {
          mout<<"In the function defined by: "<<FileName<<endl;
          mout<<"YB keyword: You need at least two bin centers (third parameter >= 2 not "<<Parser.GetTokenizerAt(i)->GetTokenAtAsInt(3)<<")!"<<endl;
          return false;
        }
        
        m_Y.clear();
        for (int x = 0; x < Parser.GetTokenizerAt(i)->GetTokenAtAsInt(3); ++x) {
          m_Y.push_back(Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(1) + x*(Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(2) - Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(1))/ (Parser.GetTokenizerAt(i)->GetTokenAtAsInt(3)-1));
        }
      } else {
        mout<<"In the function defined by: "<<FileName<<endl;
        mout<<"YB keyword has not the correct number of parameters!"<<endl;
        return false;
      }
    }

    if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "IP") == true) {
      if (Parser.GetTokenizerAt(i)->GetNTokens() == 2) {
        MString InterpolationType = Parser.GetTokenizerAt(i)->GetTokenAt(1);
        InterpolationType.ToLower();
        if (InterpolationType == "lin") {
          m_InterpolationType = c_InterpolationLinear;
        } else if (InterpolationType == "none") {
          m_InterpolationType = c_InterpolationNone;
        } else {
          mout<<"In the function defined by: "<<FileName<<endl;
          mout<<"Unknown interpolation type!"<<endl;
          return false;
        }
      } else {
        mout<<"In the function defined by: "<<FileName<<endl;
        mout<<"IP keyword incorrect!"<<endl;
        return false;
      }
    }
  }
  m_Z.clear();
  m_Z.resize(m_X.size()*m_Y.size());


  // Sanity checks:

  // We need at least two bins in x and y direction:
  if (m_X.size() < 2) {
    mout<<"In the function defined by: "<<FileName<<endl;
    mout<<"You need at least 2 x-bins!"<<endl;
    return false;
  }
  if (m_Y.size() < 2) {
    mout<<"In the function defined by: "<<FileName<<endl;
    mout<<"You need at least 2 y-bins!"<<endl;
    return false;
  }

  // Are m_X in increasing order?
  for (unsigned int i = 0; i < m_X.size()-1; ++i) {
    if (m_X[i] >= m_X[i+1]) {
      mout<<"In the function defined by: "<<FileName<<endl;
      mout<<"x values are not in increasing order!"<<endl;
      return false;
    }
  }
  // Are m_Y in increasing order?
  for (unsigned int i = 0; i < m_Y.size()-1; ++i) {
    if (m_Y[i] >= m_Y[i+1]) {
      mout<<"In the function defined by: "<<FileName<<endl;
      mout<<"y values are not in increasing order!"<<endl;
      return false;
    }
  }

  // Round two: Parse the actual data
  for (unsigned int i = 0; i < Parser.GetNLines(); ++i) {
    if (Parser.GetTokenizerAt(i)->GetNTokens() == 0) continue;
    if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "AP") == true) {
      if (Parser.GetTokenizerAt(i)->GetNTokens() != 4) {
        mout<<"In the function defined by: "<<FileName<<endl;
        mout<<"Wrong number of arguments!"<<endl;
        return false;
      } else {
        if (Parser.GetTokenizerAt(i)->GetTokenAtAsInt(1) < 0 ||
            Parser.GetTokenizerAt(i)->GetTokenAtAsInt(1) > (int) m_X.size()) {
          mout<<"In the function defined by: "<<FileName<<endl;
          mout<<"X-axis out of bounds!"<<endl;
          return false;          
        }
        if (Parser.GetTokenizerAt(i)->GetTokenAtAsInt(2) < 0 ||
            Parser.GetTokenizerAt(i)->GetTokenAtAsInt(2) > (int) m_Y.size()) {
          mout<<"In the function defined by: "<<FileName<<endl;
          mout<<"Y-axis out of bounds!"<<endl;
          return false;                    
        }

        m_Z[Parser.GetTokenizerAt(i)->GetTokenAtAsInt(1) + m_X.size()*Parser.GetTokenizerAt(i)->GetTokenAtAsInt(2)] = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(3);
      }
    } else if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "GR") == true) {
      if (Parser.GetTokenizerAt(i)->GetNTokens() != 2 + m_X.size()) {
        mout<<"In the function defined by: "<<FileName<<endl;
        mout<<"Wrong number of arguments!"<<endl;
        return false;
      } else {
        if (Parser.GetTokenizerAt(i)->GetTokenAtAsInt(1) < 0 ||
            Parser.GetTokenizerAt(i)->GetTokenAtAsInt(1) > (int) m_Y.size()) {
          mout<<"In the function defined by: "<<FileName<<endl;
          mout<<"Y-axis out of bounds!"<<endl;
          return false;                    
        }

        vector<double> V = Parser.GetTokenizerAt(i)->GetTokenAtAsDoubleVector(2);

        for (unsigned int x = 0; x < m_X.size(); ++x) {
          m_Z[x + m_X.size()*Parser.GetTokenizerAt(i)->GetTokenAtAsInt(1)] = V[x];
        }
      }
    }
  }

  // Determine interpolation type:
  if (m_X.size() > 1 && m_InterpolationType == c_InterpolationConstant) {
    m_InterpolationType = c_InterpolationLinear;
  } 

  if (m_X.size() == 1) {
    m_InterpolationType = c_InterpolationConstant;
  } 
  
  if (m_X.size() == 0) {
    m_InterpolationType = c_InterpolationConstant;
    m_X.push_back(0);
    m_Y.push_back(0);
  } 

  // Clean up:
  m_Maximum = g_DoubleNotDefined;
  m_Cumulative.clear();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MFunction2D::ScaleZ(double Scaler)
{
  // Scale the z-content by some value

  for (unsigned int i = 0; i < m_Z.size(); ++i) {
    m_Z[i] *= Scaler;
  } 

  // Clean up:
  m_Maximum = g_DoubleNotDefined;
  m_Cumulative.clear();
}


////////////////////////////////////////////////////////////////////////////////


void MFunction2D::ScaleX(double Scaler)
{
  // Multiple the x-axis by some value

   for (unsigned int i = 0; i < m_X.size(); ++i) {
    m_X[i] *= Scaler;
  }  

  // We clear the cumulative function:
  m_Maximum = g_DoubleNotDefined;
  m_Cumulative.clear();
}


////////////////////////////////////////////////////////////////////////////////


void MFunction2D::ScaleY(double Scaler)
{
  // Multiple the y-axis by some value

   for (unsigned int i = 0; i < m_Y.size(); ++i) {
    m_Y[i] *= Scaler;
  }  

  // We clear the cumulative function:
  m_Maximum = g_DoubleNotDefined;
  m_Cumulative.clear();
}


////////////////////////////////////////////////////////////////////////////////


double MFunction2D::Eval(double x, double y) const
{
  mdep<<"MFunction2D::Eval is deprecated, replace with: MFunction2D::Evaluate"<<show;
  return Evaluate(x, y); 
}


////////////////////////////////////////////////////////////////////////////////


double MFunction2D::Evaluate(double x, double y) const
{
  // Evalute the function

  if (m_InterpolationType == c_InterpolationUnknown) {
    merr<<"Unknown fit type: "<<c_InterpolationUnknown<<endl;
    return 0;
  }

  if (m_InterpolationType == c_InterpolationConstant) {
    return m_Z[0];
  } else if (m_InterpolationType == c_InterpolationNone) {

    // Get Position:
    int xPosition = -1; 
    for (unsigned int i = 0; i < m_X.size(); ++i) {
      if (m_X[i] > x) {
        break;
      } 
      xPosition = (int) i;
    }

    int yPosition = -1; 
    for (unsigned int i = 0; i < m_Y.size(); ++i) {
      if (m_Y[i] > y) {
        break;
      } 
      yPosition = (int) i;
    }

    if (xPosition < 0) xPosition = 0;
    if (yPosition < 0) yPosition = 0;
    if (xPosition >= (int) m_X.size()-1) xPosition = (int) (m_X.size()-2);
    if (yPosition >= (int) m_Y.size()-1) yPosition = (int) (m_Y.size()-2);

    if (x > m_X[xPosition] + 0.5*(m_X[xPosition+1] - m_X[xPosition])) xPosition += 1;
    if (y > m_Y[yPosition] + 0.5*(m_Y[yPosition+1] - m_Y[yPosition])) yPosition += 1;

    if (xPosition > (int) m_X.size()-1) xPosition = (int) (m_X.size()-1);
    if (yPosition > (int) m_Y.size()-1) yPosition = (int) (m_Y.size()-1);

    return m_Z[xPosition + m_X.size()*yPosition];

  } else if (m_InterpolationType == c_InterpolationLinear) {

    // Get Position:
    int xPosition = -1; 
    for (unsigned int i = 0; i < m_X.size(); ++i) {
      if (m_X[i] > x) {
        break;
      } 
      xPosition = (int) i;
    }

    int yPosition = -1; 
    for (unsigned int i = 0; i < m_Y.size(); ++i) {
      if (m_Y[i] > y) {
        break;
      } 
      yPosition = (int) i;
    }

    if (xPosition < 0) xPosition = 0;
    if (yPosition < 0) yPosition = 0;
    if (xPosition >= (int) m_X.size()-1) xPosition = (int) (m_X.size()-2);
    if (yPosition >= (int) m_Y.size()-1) yPosition = (int) (m_Y.size()-2);

    // Interpolate at y min:
    double zmin = m_Z[(xPosition+1) + m_X.size()*yPosition] - (m_X[xPosition+1] - x)/
      (m_X[xPosition+1] - m_X[xPosition])*
      (m_Z[(xPosition+1) + m_X.size()*yPosition] - m_Z[xPosition + m_X.size()*yPosition]);

    // Interpolate at y max:
    double zmax = m_Z[(xPosition+1) + m_X.size()*(yPosition+1)] - (m_X[xPosition+1] - x)/
      (m_X[xPosition+1] - m_X[xPosition])*
      (m_Z[(xPosition+1) + m_X.size()*(yPosition+1)] - m_Z[xPosition + m_X.size()*(yPosition+1)]);

    // Interpolate y:
    return zmax - (m_Y[yPosition+1] - y)/
      (m_Y[yPosition+1] - m_Y[yPosition])*
      (zmax - zmin);
  } 

  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


void MFunction2D::GetRandom(double& x, double& y)
{
  // Return a random number distributed as the underlying function
  // The following is an accurate and safe version but rather slow...

  if (GetZMin() < 0) {
    merr<<"GetRandom() currently only works if all y-values are positive (or zero)."<<show;
    x = g_DoubleNotDefined;
    y = g_DoubleNotDefined;
    return;
  }

  if (m_Maximum == g_DoubleNotDefined) {
    m_Maximum = GetZMax();
  }

  // Old, simple and slooowwww:
  double z = 0;
  double xmax = GetXMax();
  double ymax = GetYMax();
  double xmin = GetXMin();
  double ymin = GetYMin();
  do {
    x = xmin + gRandom->Rndm()*(xmax - xmin);
    y = ymin + gRandom->Rndm()*(ymax - ymin);
//     x = gRandom->Rndm()*(GetXMax() - GetXMin()) + GetXMin();
//     y = gRandom->Rndm()*(GetYMax() - GetYMin()) + GetYMin();

    z = Evaluate(x, y);
  } while (m_Maximum*gRandom->Rndm() > z);



  // The following is WRONG but left in just in case I find once a good solution....
  // Key problem: 3 points define a plane not 4 ...

//   // Check if we have to (re-) determine the cumulative function:
//   if (m_Cumulative.size() == 0) {
//     m_Cumulative.push_back(0);
//     for (unsigned int x = 1; x < m_X.size(); ++x) {
//       for (unsigned int y = 1; y < m_Y.size(); ++y) {
//         // Determine volume below each box - since we only do linear interpolation just calculate the pyramid height
//         double Max = -numeric_limits<double>::max();
//         if (m_Z[x+y*m_X.size()] > Max) Max = m_Z[x+y*m_X.size()];
//         if (m_Z[(x-1)+y*m_X.size()] > Max) Max = m_Z[(x-1)+y*m_X.size()];
//         if (m_Z[x+(y-1)*m_X.size()] > Max) Max = m_Z[x+(y-1)*m_X.size()];
//         if (m_Z[(x-1)+(y-1)*m_X.size()] > Max) Max = m_Z[(x-1)+(y-1)*m_X.size()];
//         double Min = numeric_limits<double>::max();
//         if (m_Z[x+y*m_X.size()] < Min) Min = m_Z[x+y*m_X.size()];
//         if (m_Z[(x-1)+y*m_X.size()] < Min) Min = m_Z[(x-1)+y*m_X.size()];
//         if (m_Z[x+(y-1)*m_X.size()] < Min) Min = m_Z[x+(y-1)*m_X.size()];
//         if (m_Z[(x-1)+(y-1)*m_X.size()] < Min) Min = m_Z[(x-1)+(y-1)*m_X.size()];

//         double Volume = 1.0/3.0 * (m_X[x] - m_X[x-1])*(m_Y[y] - m_Y[y-1]) *  (Max-Min);  <-- WRONG!! Shape can not be approximated easily (problem is overdetermined!!!)

//         m_Cumulative.push_back(m_Cumulative.back() + Volume);
//       }
//     }
//   }

//   // Determine a random integral value in m_Cumulative
//   double Integral = gRandom->Rndm()*m_Cumulative.back();

//   // Find the correct bin in m_Cumulative
//   int Bin = find_if(m_Cumulative.begin(), m_Cumulative.end(), bind2nd(greater_equal<double>(), Integral)) - m_Cumulative.begin();

//   // Find the UPPER x and y value
//   int yBin = Bin%m_X.size();
//   int xBin = Bin - yBin; 

//   // Find the minimum and maximum in this bin (might be stored to increase speed)
//   double Max = -numeric_limits<double>::max();
//   if (m_Z[xBin+yBin*m_X.size()] > Max) Max = m_Z[xBin+yBin*m_X.size()];
//   if (m_Z[(xBin-1)+yBin*m_X.size()] > Max) Max = m_Z[(xBin-1)+yBin*m_X.size()];
//   if (m_Z[xBin+(yBin-1)*m_X.size()] > Max) Max = m_Z[xBin+(yBin-1)*m_X.size()];
//   if (m_Z[(xBin-1)+(yBin-1)*m_X.size()] > Max) Max = m_Z[(xBin-1)+(yBin-1)*m_X.size()];
//   double Min = numeric_limits<double>::max();
//   if (m_Z[xBin+yBin*m_X.size()] < Min) Min = m_Z[xBin+yBin*m_X.size()];
//   if (m_Z[(xBin-1)+yBin*m_X.size()] < Min) Min = m_Z[(xBin-1)+yBin*m_X.size()];
//   if (m_Z[xBin+(yBin-1)*m_X.size()] < Min) Min = m_Z[xBin+(yBin-1)*m_X.size()];
//   if (m_Z[(xBin-1)+(yBin-1)*m_X.size()] < Min) Min = m_Z[(xBin-1)+(yBin-1)*m_X.size()];
  
//   // Do a simple random lookup in this bin only
//   // If somebody knows a better solution let me know - the solution is not a simple integration over a plane
//   double z = 0;
//   do {
//     x = gRandom->Rndm()*(m_X[xBin] - m_X[xBin-1]) + m_X[xBin-1];
//     y = gRandom->Rndm()*(m_Y[yBin] - m_Y[yBin-1]) + m_Y[yBin-1];

//     z = Evaluate(x, y);
//   } while (Min + (Max-Min)*gRandom->Rndm() > z);

  return;
}


////////////////////////////////////////////////////////////////////////////////


double MFunction2D::GetXMin() const
{
  //! Get the minimum x-value

  return m_X.front();
}


////////////////////////////////////////////////////////////////////////////////


double MFunction2D::GetXMax() const
{
  //! Get the maximum x-value

  return m_X.back();
}


////////////////////////////////////////////////////////////////////////////////


double MFunction2D::GetYMin() const
{
  //! Get the minimum y-value

  return m_Y.front();
}


////////////////////////////////////////////////////////////////////////////////


double MFunction2D::GetYMax() const
{
  //! Get the maximum y-value

  return m_Y.back();
}


////////////////////////////////////////////////////////////////////////////////


double MFunction2D::GetZMin()
{
  //! Get the minimum z-value

  double Min = numeric_limits<double>::max();
  for (unsigned int i = 0; i < m_Z.size(); ++i) {
    if (m_Z[i] < Min) Min = m_Z[i];
  }

  return Min;
}


////////////////////////////////////////////////////////////////////////////////


double MFunction2D::GetZMax()
{
  //! Get the maximum z-value

  if (m_Maximum == g_DoubleNotDefined) {
    m_Maximum = -numeric_limits<double>::max();
    for (unsigned int i = 0; i < m_Z.size(); ++i) {
      if (m_Z[i] > m_Maximum) m_Maximum = m_Z[i];
    }
  }

  return m_Maximum;
}


////////////////////////////////////////////////////////////////////////////////


void MFunction2D::Plot()
{
  // Plot the function in a Canvas (diagnostics only)
  
  if (m_X.size() >= 2 && m_Y.size() >= 2) {

//     TGraph2D* Graph = new TGraph2D();
//     int N = 0;
//     for (unsigned int x = 0; x < m_X.size(); ++x) {
//       for (unsigned int y = 0; y < m_Y.size(); ++y) {
//         Graph->SetPoint(N++, m_X[x], m_Y[y], m_Z[x + m_X.size()*y]);
//       }
//     }


    TH2D* Hist = new TH2D("Diagnostics", "Diagnostics", 
                          m_X.size(), m_X.front() - 0.5*(m_X[1]-m_X[0]), m_X.back() + 0.5*(m_X[1]-m_X[0]), 
                          m_Y.size(), m_Y.front() - 0.5*(m_Y[1]-m_Y[0]), m_Y.back() + 0.5*(m_Y[1]-m_Y[0]));
    for (int bx = 1; bx <= Hist->GetXaxis()->GetNbins(); ++bx) {
      for (int by = 1; by <= Hist->GetYaxis()->GetNbins(); ++by) {
        Hist->SetBinContent(bx, by, Evaluate(Hist->GetXaxis()->GetBinCenter(bx), Hist->GetYaxis()->GetBinCenter(by)));
      }
    }

    TCanvas* Canvas = new TCanvas("DiagnosticsCanvas", "DiagnosticsCanvas");
    Canvas->cd();
    Hist->Draw("colz");
    //Graph->Draw("PCOL");
    Canvas->Update();

    gSystem->ProcessEvents();

    for (unsigned int i = 0; i < 10; ++i) {
      gSystem->ProcessEvents();
      gSystem->Sleep(10);
    }

  } else {
    mout<<"Not enough data points for diagnostics: x="<<m_X.size()<<" y="<<m_Y.size()<<endl;
  }
}


// MFunction2D.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
