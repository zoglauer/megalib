/*
 * MFunction3DSpherical.cxx
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
// MFunction3DSpherical
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFunction3DSpherical.h"

// Standard libs:
#include <fstream>
#include <iomanip>
using namespace std;

// ROOT libs:
#include "TH2.h"
#include "TH3.h"
#include "TRandom.h"
#include "TCanvas.h"
#include "TSystem.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MFunction3DSpherical)
#endif


////////////////////////////////////////////////////////////////////////////////


MFunction3DSpherical::MFunction3DSpherical() : MFunction3D()
{
  // Construct an instance of MFunction3DSpherical
}


////////////////////////////////////////////////////////////////////////////////


MFunction3DSpherical::MFunction3DSpherical(const MFunction3DSpherical& F)
{
  // Copy-construct an instance of MFunction3DSpherical

  m_InterpolationType = F.m_InterpolationType;

  m_X = F.m_X;
  m_Y = F.m_Y;
  m_Z = F.m_Z;
  m_V = F.m_V;
  
  m_Maximum = F.m_Maximum;
}


////////////////////////////////////////////////////////////////////////////////


MFunction3DSpherical::~MFunction3DSpherical()
{
  // Delete this instance of MFunction3DSpherical
}


////////////////////////////////////////////////////////////////////////////////


const MFunction3DSpherical& MFunction3DSpherical::operator=(const MFunction3DSpherical& F)
{
  // Copy-construct an instance of MFunction3DSpherical

  m_InterpolationType = F.m_InterpolationType;

  m_X = F.m_X;
  m_Y = F.m_Y;
  m_Z = F.m_Z;
  m_V = F.m_V;
  
  m_Maximum = F.m_Maximum;

  return *this;
}



////////////////////////////////////////////////////////////////////////////////


bool MFunction3DSpherical::Set(const MString FileName, const MString KeyWord,
                               const unsigned int InterpolationType)
{
  // Set the basic data, load the file and parse it

  m_InterpolationType = InterpolationType;

  MParser Parser;

  cout<<"Started parsing 3D spherical function file into memory... stand by... this may take some time..."<<endl;

  if (Parser.Open(FileName, MFile::c_Read) == false) {
    mout<<"Unable to open file "<<FileName<<endl;
    return false;
  }

  cout<<"Started reading 3D Spherical function (header)... this may take a long time..."<<endl;

  // Round one: Look for axis and interpolation type:
  for (unsigned int i = 0; i < Parser.GetNLines(); ++i) {
    if (Parser.GetTokenizerAt(i)->GetNTokens() == 0) continue;
    if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "PA") == true) {
      if (Parser.GetTokenizerAt(i)->GetNTokens() >= 2) {
        m_X = Parser.GetTokenizerAt(i)->GetTokenAtAsDoubleVector(1);
      } else {
        mout<<"In the function defined by: "<<FileName<<endl;
        mout<<"PA keyword (phi-axis) has not enough parameters!"<<endl;
        return false;
      }
    }
    if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "TA") == true) {
      if (Parser.GetTokenizerAt(i)->GetNTokens() >= 2) {
        m_Y = Parser.GetTokenizerAt(i)->GetTokenAtAsDoubleVector(1);
      } else {
        mout<<"In the function defined by: "<<FileName<<endl;
        mout<<"TA keyword (theta axis) has not enough parameters!"<<endl;
        return false;
      }
    }
    if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "EA") == true) {
      if (Parser.GetTokenizerAt(i)->GetNTokens() >= 2) {
        m_Z = Parser.GetTokenizerAt(i)->GetTokenAtAsDoubleVector(1);
      } else {
        mout<<"In the function defined by: "<<FileName<<endl;
        mout<<"EA keyword (energy axis) has not enough parameters!"<<endl;
        return false;
      }
    }
    if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "IP") == true) {
      if (Parser.GetTokenizerAt(i)->GetNTokens() == 2) {
        if (Parser.GetTokenizerAt(i)->GetTokenAt(1) == "LIN") {
          m_InterpolationType = c_InterpolationLinear;
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
  m_V.clear();
  m_V.resize(m_X.size()*m_Y.size()*m_Z.size());

  cout<<"Started reading 3D Spherical function (body)... this may take an even longer time..."<<endl;

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
  if (m_Z.size() < 2) {
    mout<<"In the function defined by: "<<FileName<<endl;
    mout<<"You need at least 2 z-bins!"<<endl;
    return false;
  }

  // Are m_X in increasing order?
  for (unsigned int i = 0; i < m_X.size()-1; ++i) {
    if (m_X[i] >= m_X[i+1]) {
      mout<<"In the function defined by: "<<FileName<<endl;
      mout<<"phi values are not in increasing order!"<<endl;
      return false;
    }
  }
  // Are m_Y in increasing order?
  for (unsigned int i = 0; i < m_Y.size()-1; ++i) {
    if (m_Y[i] >= m_Y[i+1]) {
      mout<<"In the function defined by: "<<FileName<<endl;
      mout<<"theta values are not in increasing order!"<<endl;
      return false;
    }
  }
  // Are m_Z in increasing order?
  for (unsigned int i = 0; i < m_Z.size()-1; ++i) {
    if (m_Z[i] >= m_Z[i+1]) {
      mout<<"In the function defined by: "<<FileName<<endl;
      mout<<"energy values are not in increasing order!"<<endl;
      return false;
    }
  }

  // Round two: Parse the actual data
  for (unsigned int i = 0; i < Parser.GetNLines(); ++i) {
    if (Parser.GetTokenizerAt(i)->GetNTokens() == 0) continue;
    if (Parser.GetTokenizerAt(i)->IsTokenAt(0, KeyWord) == true) {
      if (Parser.GetTokenizerAt(i)->GetNTokens() != 5) {
        mout<<"In the function defined by: "<<FileName<<endl;
        mout<<"Wrong number of arguments!"<<endl;
        return false;
      } else {
//         int x = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(1);
//         int y = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(2);
//         int z = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(3);
//         if (x < 0 || x > (int) m_X.size()) {
//           mout<<"In the function defined by: "<<FileName<<endl;
//           mout<<"X-axis out of bounds!"<<endl;
//           return false;
//         }
//         if (y < 0 || y > (int) m_Y.size()) {
//           mout<<"In the function defined by: "<<FileName<<endl;
//           mout<<"Y-axis out of bounds!"<<endl;
//           return false;
//         }
//         if (z < 0 || z > (int) m_Z.size()) {
//           mout<<"In the function defined by: "<<FileName<<endl;
//           mout<<"Z-axis out of bounds!"<<endl;
//           return false;
//         }

        int xPosition = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(1);
        int yPosition = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(2);
        int zPosition = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(3);

        if (xPosition < 0 || xPosition > (int) m_X.size()) {
          mout<<"In the function defined by: "<<FileName<<endl;
          mout<<"Phi-axis out of bounds: "<<xPosition<<endl;
          return false;
        }
        if (yPosition < 0 || yPosition > (int) m_Y.size()) {
          mout<<"In the function defined by: "<<FileName<<endl;
          mout<<"Theta-axis out of bounds: "<<yPosition<<endl;
          return false;
        }
        if (zPosition < 0 || zPosition > (int) m_Z.size()) {
          mout<<"In the function defined by: "<<FileName<<endl;
          mout<<"Energy-axis out of bounds: "<<zPosition<<endl;
          return false;
        }

        m_V[xPosition + m_X.size()*yPosition + m_X.size()*m_Y.size()*zPosition] = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(4);
      }
    }
  }

  // Make sure we have spherical coordinates, i.e. theta has to be within [0..180]
  if (m_Y[0] < 0 || m_Y.back() > 180) {
    mout<<"In the function defined by: "<<FileName<<endl;
    mout<<"The theta axis values are not all within [0...180]"<<endl;
    return false;    
  }
  
  cout<<"Done reading 3D Spherical function (body)!"<<endl;

  // Determine interapolation type:
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
    m_Z.push_back(0);
  }

  // Clean up:
  m_Maximum = g_DoubleNotDefined;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFunction3DSpherical::Save(const MString FileName, const MString Keyword) 
{
  // Save to a file:
  
  ofstream out;
  out.open(FileName);
  if (out.is_open() == false) {
    merr<<"Unable to open file: "<<FileName<<endl;
    return false;
  }
  
  out<<scientific;
  out<<setprecision(8);
  
  out<<"IP LIN"<<endl;

  out<<"PA ";
  for (unsigned int x = 0; x < m_X.size(); ++x) {
    out<<m_X[x]<<" ";
  }
  out<<endl;

  out<<"TA ";
  for (unsigned int y = 0; y < m_Y.size(); ++y) {
    out<<m_Y[y]<<" ";
  }
  out<<endl;

  out<<"EA ";
  for (unsigned int z = 0; z < m_Z.size(); ++z) {
    out<<m_Z[z]<<" ";
  }
  out<<endl;

  for (unsigned int x = 0; x < m_X.size(); ++x) {
    for (unsigned int y = 0; y < m_Y.size(); ++y) {
      for (unsigned int z = 0; z < m_Z.size(); ++z) {
        if (m_V[x + m_X.size()*y + m_X.size()*m_Y.size()*z] != 0) {
          out<<Keyword<<" "<<x<<" "<<y<<" "<<z<<" "<<m_V[x + m_X.size()*y + m_X.size()*m_Y.size()*z]<<endl;
        }
      }
    }
  }
  out<<"EN"<<endl;
  
  out.close();

  return true;
}
  
  
////////////////////////////////////////////////////////////////////////////////


double MFunction3DSpherical::Integrate() const
{
  //! Return the total content of the function
  //! This is a simplified integration which uses the value at the bin centers and the (3D) bin size
  
  double Integral = 0.0;

  for (unsigned int x = 0; x < m_X.size()-1; ++x) {
    double xCenter = 0.5*(m_X[x+1] + m_X[x]);
    double xSize = fabs((m_X[x+1] - m_X[x]))*c_Rad;
    for (unsigned int y = 0; y < m_Y.size()-1; ++y) {
      double yCenter = 0.5*(m_Y[y+1] + m_Y[y]);
      double ySize = fabs(cos(m_Y[y]*c_Rad) - cos(m_Y[y+1]*c_Rad));
      for (unsigned int z = 0; z < m_Z.size()-1; ++z) {
        double zCenter = 0.5*(m_Z[z+1] + m_Z[z]);
        double zSize = fabs(m_Z[z+1] - m_Z[z]);
        Integral += xSize*ySize*zSize*Evaluate(xCenter, yCenter, zCenter);
      }
    }
  }

  return Integral;
}
  

////////////////////////////////////////////////////////////////////////////////


void MFunction3DSpherical::GetRandom(double& x, double& y, double& z)
{
  // Return a random number distributed as the underlying function
  // The following is an accurate and safe version but rather slow...

  if (m_Maximum == g_DoubleNotDefined) {
    m_Maximum = GetVMax();
  }

  double x_min = GetXMin();
  double x_diff = GetXMax() - GetXMin();
  
  double y_min = cos(m_Y[0]*c_Rad);
  double y_diff = cos(m_Y[0]*c_Rad) - cos(m_Y.back()*c_Rad);
  
  double z_min = GetZMin();
  double z_diff = GetZMax() - GetZMin();
  
  double v = 0;
  do {
    x = gRandom->Rndm()*x_diff + x_min;
    y = acos(y_min - gRandom->Rndm()*y_diff)*c_Deg;
    z = gRandom->Rndm()*z_diff + z_min;
    
    v = Evaluate(x, y, z);
  } while (m_Maximum*gRandom->Rndm() > v);
}


////////////////////////////////////////////////////////////////////////////////


void MFunction3DSpherical::Plot(bool Random)
{
  // Plot the function in a Canvas (diagnostics only)
  
  if (m_X.size() >= 2 && m_Y.size() >= 2 && m_Z.size() >= 2) {

    TH3D* Hist = new TH3D("MFunction3DSpherical", "MFunction3DSpherical", m_X.size(), m_X.front(), m_X.back(), m_Y.size(), m_Y.front(), m_Y.back(), m_Z.size(), m_Z.front(), m_Z.back());
    Hist->SetXTitle("phi in degree");
    Hist->SetYTitle("theta in degree");
    Hist->SetZTitle("energy in keV");

    TH2D* Projection = new TH2D("MFunction3DSpherical-P", "MFunction3DSpherical-P", 
                          m_X.size(), m_X.front(), m_X.back(), 
                          m_Y.size(), m_Y.front(), m_Y.back());
    Projection->SetXTitle("phi in degree");
    Projection->SetYTitle("theta in degree");

    TCanvas* Canvas = new TCanvas("DiagnosticsCanvas", "DiagnosticsCanvas");
    Canvas->cd();

    if (Random == true) {
      double x, y, z;
      unsigned int i_max = 100000;
      for (unsigned int i = 0; i < 100000; ++i) {
        if (i % (i_max/1000) == 0) {
          cout<<"\rSimulation progress: "<<100.0 * i/i_max<<"     "<<flush;
          Hist->Draw("colz");
          Canvas->Update();
          gSystem->ProcessEvents();
        }
        GetRandom(x, y, z);
        Hist->Fill(x, y, z, 1);
      }
    } else {
      for (int bx = 1; bx <= Hist->GetXaxis()->GetNbins(); ++bx) {
        for (int by = 1; by <= Hist->GetYaxis()->GetNbins(); ++by) {
          for (int bz = 1; bz <= Hist->GetZaxis()->GetNbins(); ++bz) {
            Hist->SetBinContent(bx, by, bz, Evaluate(Hist->GetXaxis()->GetBinCenter(bx), Hist->GetYaxis()->GetBinCenter(by), Hist->GetZaxis()->GetBinCenter(bz)));
            Projection->SetBinContent(bx, by, Projection->GetBinContent(bx, by) + Evaluate(Hist->GetXaxis()->GetBinCenter(bx), Hist->GetYaxis()->GetBinCenter(by), Hist->GetZaxis()->GetBinCenter(bz)));
          }
        }
      }
    }


    Hist->Draw("colz");
    Canvas->Update();

    TCanvas* ProjectionCanvas = new TCanvas();
    ProjectionCanvas->cd();
    Projection->Draw("colz");
    ProjectionCanvas->Update();
    
    
    gSystem->ProcessEvents();

    for (unsigned int i = 0; i < 10; ++i) {
      gSystem->ProcessEvents();
      gSystem->Sleep(10);
    }

  } else {
    mout<<"Not enough data points for diagnostics: x="<<m_X.size()<<" y="<<m_Y.size()<<" z="<<m_Z.size()<<endl;
  }
}


// MFunction3DSpherical.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
