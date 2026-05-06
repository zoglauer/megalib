/*
 * MImage3D.cxx
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
// MImage3D
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MImage3D.h"

// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:
#include <TH3.h>
#include <TMath.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MImage3D)
#endif


////////////////////////////////////////////////////////////////////////////////


MImage3D::MImage3D() : MImage2D()
{
  // default constructor

  // Initialization is mostly done in the MImage constructor
  SetZAxis("z-Axis", 0, 1, 1);
  
  m_HistXY = nullptr;
  m_HistXZ = nullptr;
  m_HistYZ = nullptr;

  m_CanvasXY = nullptr;
  m_CanvasXZ = nullptr;
  m_CanvasYZ = nullptr;
}


////////////////////////////////////////////////////////////////////////////////


MImage3D::MImage3D(MString Title, double* IA, 
                   MString xTitle, double xMin, double xMax, int xNBins, 
                   MString yTitle, double yMin, double yMax, int yNBins, 
                   MString zTitle, double zMin, double zMax, int zNBins, 
                   MString vTitle, int Spectrum, int DrawOption) :
  MImage2D()
{
  // standard constructor

  SetTitle(Title);
  SetXAxis(xTitle, xMin, xMax, xNBins);
  SetYAxis(yTitle, yMin, yMax, yNBins);
  SetZAxis(zTitle, zMin, zMax, zNBins);
  SetValueAxisTitle(vTitle);
  SetSpectrum(Spectrum);
  SetDrawOption(DrawOption);
  SetImageArray(IA);

  m_HistXY = nullptr;
  m_HistXZ = nullptr;
  m_HistYZ = nullptr;

  m_CanvasXY = nullptr;
  m_CanvasXZ = nullptr;
  m_CanvasYZ = nullptr;
}


////////////////////////////////////////////////////////////////////////////////


MImage3D::~MImage3D()
{
  // default destructor

  if (m_HistXY != nullptr) {
    delete m_HistXY;
    m_HistXY = nullptr;
  }
  if (m_HistXZ != nullptr) {
    delete m_HistXZ;
    m_HistXZ = nullptr;
  }
  if (m_HistYZ != nullptr) {
    delete m_HistYZ;
    m_HistYZ = nullptr;
  }

  if (m_CanvasXY != nullptr) {
    delete m_CanvasXY;
    m_CanvasXY = nullptr;
  }
  if (m_CanvasXZ != nullptr) {
    delete m_CanvasXZ;
    m_CanvasXZ = nullptr;
  }
  if (m_CanvasYZ != nullptr) {
    delete m_CanvasYZ;
    m_CanvasYZ = nullptr;
  }
}


////////////////////////////////////////////////////////////////////////////////


MImage* MImage3D::Clone()
{
  //! Clone this image

  MImage* I = 
    new MImage3D(m_Title, m_IA, 
                 m_xTitle, m_xMin, m_xMax, m_xNBins, 
                 m_yTitle, m_yMin, m_yMax, m_yNBins, 
                 m_zTitle, m_zMin, m_zMax, m_zNBins, 
                 m_vTitle, m_Spectrum, m_DrawOption);

  I->Normalize(m_Normalize);
    
  return I;
}

////////////////////////////////////////////////////////////////////////////////


void MImage3D::SetImageArray(double* IA)
{
  // Fill the image array of this class
  // The assumed fill-mode for the array is: Array[x + y*xNBins+ z*xNBins*yNBins]

  int NEntries = GetNEntries();

  if (m_IA != 0) delete [] m_IA;
  m_IA = new double[NEntries];

  for (int x = 0; x < NEntries; x++) {
    if (IA != 0) {
      m_IA[x] = IA[x];
    } else {
      m_IA[x] = 0.0;
    } 
  }

  TH3D* Histogram = dynamic_cast<TH3D*>(m_Histogram);
  if (Histogram != nullptr && m_Canvas != nullptr) {
    m_Canvas->cd();

    double Content = 0.0;
    for (int x = 1; x <= m_xNBins; ++x) {
      for (int y = 1; y <= m_yNBins; ++y) {
        for (int z = 1; z <= m_zNBins; ++z) {
          // Clear the bin first so skipped non-finite values do not leave stale content behind.
          Histogram->SetBinContent(x, y, z, 0.0);
          Content = m_IA[(x-1) + (y-1)*m_xNBins + (z-1)*m_xNBins*m_yNBins];
          if (!TMath::IsNaN(Content)) {
            if (TMath::Finite(Content)) {
              Histogram->SetBinContent(x, y, z, Content);
            } else {
              merr<<"Image contains Inf ("<<x<<", "<<y<<", "<<z<<")"<<show;
            }
          } else {
              merr<<"Image contains NaN ("<<x<<", "<<y<<", "<<z<<")"<<show;
          }
        }
      }
    }
    m_Histogram->Draw();
    m_Canvas->Update();
    
    if (m_CanvasXY != nullptr && m_HistXY != nullptr) {
      m_CanvasXY->cd();
      delete m_HistXY;
      m_HistXY = Histogram->Project3D("xy");
      m_HistXY->SetDirectory(0);
      m_HistXY->SetTitle(MString(m_Histogram->GetTitle()) + " - xy-projection");
      m_HistXY->Draw(m_DrawOptionString);
      m_CanvasXY->Update();
    }

    if (m_CanvasXZ != nullptr && m_HistXZ != nullptr) {
      m_CanvasXZ->cd();
      delete m_HistXZ;
      m_HistXZ = Histogram->Project3D("xz");
      m_HistXZ->SetDirectory(0);
      m_HistXZ->Draw(m_DrawOptionString);
      m_HistXZ->SetTitle(MString(m_Histogram->GetTitle()) + " - xz-projection");
      m_CanvasXZ->Update();
    }

    if (m_CanvasYZ != nullptr && m_HistYZ != nullptr) {
      m_CanvasYZ->cd();
      delete m_HistYZ;
      m_HistYZ = Histogram->Project3D("yz");
      m_HistYZ->SetDirectory(0);
      m_HistYZ->Draw(m_DrawOptionString);
      m_HistYZ->SetTitle(MString(m_Histogram->GetTitle()) + " - yz-projection");
      m_CanvasYZ->Update();
    } 
  }
}


////////////////////////////////////////////////////////////////////////////////


void MImage3D::SetZAxis(MString zTitle, double zMin, double zMax, int zNBins)
{
  // Set z-Axis attributes

  m_zTitle = zTitle;
  m_zMin = zMin;
  m_zMax = zMax;
  m_zNBins = zNBins;
}


////////////////////////////////////////////////////////////////////////////////


void MImage3D::Display(TCanvas* Canvas)
{
  // Display the image in a canvas for the first time

  if (m_xNBins <= 0 || m_yNBins <= 0 || m_zNBins <= 0 || GetNEntries() <= 0 || m_IA == nullptr) {
    merr<<"Unable to display image: invalid 3D image geometry or missing image data"<<endl;
    return;
  }

  if (Canvas == 0) {
    m_CanvasTitle = MakeCanvasTitle();
    Canvas = new TCanvas(m_CanvasTitle, m_Title, 40, 40, 600, 600);
  } else {
    if (MString(Canvas->GetTitle()).IsEmpty() == false) {
      m_CanvasTitle = Canvas->GetTitle();
    } else {
      m_CanvasTitle = MakeCanvasTitle();
      Canvas->SetTitle(m_CanvasTitle);
    }
  }
  m_Canvas = Canvas;

  TH3D* Hist = 0;
  if (m_Histogram == 0) {
    Hist = new TH3D(m_Title + m_ID, m_Title, m_xNBins, m_xMin, m_xMax, m_yNBins, m_yMin, m_yMax, m_zNBins, m_zMin, m_zMax);
    m_Histogram = dynamic_cast<TH1*>(Hist);

    Hist->SetDirectory(0);
    Hist->SetXTitle(m_xTitle);
    Hist->SetYTitle(m_yTitle);
    Hist->SetZTitle(m_zTitle);
    Hist->SetFillColor(0);
    Hist->SetLabelOffset(0.003f, "X");
    Hist->SetLabelOffset(0.003f, "Y");
    Hist->SetLabelOffset(0.003f, "Z");
    Hist->SetTitleOffset(1.2f, "X");
    Hist->SetTitleOffset(1.2f, "Y");
    Hist->SetTitleOffset(1.2f, "Z");
    Hist->SetLabelSize(0.03f, "X");
    Hist->SetLabelSize(0.03f, "Y");
    Hist->SetLabelSize(0.03f, "Z");
    Hist->SetStats(false);
  } else {
    Hist = dynamic_cast<TH3D*>(m_Histogram);
    Hist->SetTitle(m_Title);
  }

  double Content = 0.0;
  for (int x = 1; x <= m_xNBins; ++x) {
    for (int y = 1; y <= m_yNBins; ++y) {
      for (int z = 1; z <= m_zNBins; ++z) {
        // Clear the bin first so skipped non-finite values do not leave stale content behind.
        Hist->SetBinContent(x, y, z, 0.0);
        Content = m_IA[(x-1) + (y-1)*m_xNBins + (z-1)*m_xNBins*m_yNBins];
        if (!TMath::IsNaN(Content)) {
          if (TMath::Finite(Content)) {
            Hist->SetBinContent(x, y, z, Content);
          } else {
            merr<<"Image contains Inf at ("<<x<<", "<<y<<", "<<z<<")"<<endl;
          }
        } else {
          merr<<"Image contains NaN at ("<<x<<", "<<y<<", "<<z<<")"<<endl;
        }
      }
    }
  }
  
  // Rescale to 1:
  if (m_Normalize == true && Hist->GetMaximum() > 0) {
    Hist->Scale(1.0/Hist->GetMaximum());
  }

  m_Canvas->cd();
  Hist->Draw();
  m_Canvas->Update();


  // Create projections, and draw them too:
  
  if (m_HistXY == 0) {
    m_HistXY = Hist->Project3D("xy");
    m_HistXY->SetDirectory(0);
  } else {
    delete m_HistXY;
    m_HistXY = Hist->Project3D("xy");
    m_HistXY->SetDirectory(0);
  }
  m_HistXY->SetTitle(MString(Hist->GetTitle()) + " - xy-projection");
  
  if (m_HistXZ == 0) {
    m_HistXZ = Hist->Project3D("xz");
    m_HistXZ->SetDirectory(0);
  } else {
    delete m_HistXZ;
    m_HistXZ = Hist->Project3D("xz");
    m_HistXZ->SetDirectory(0);
  }
  m_HistXZ->SetTitle(MString(Hist->GetTitle()) + " - xz-projection");
  
  if (m_HistYZ == 0) {
    m_HistYZ = Hist->Project3D("yz");
    m_HistYZ->SetDirectory(0);
  } else {
    delete m_HistYZ;
    m_HistYZ = Hist->Project3D("yz");
    m_HistYZ->SetDirectory(0);
  }
  m_HistYZ->SetTitle(MString(Hist->GetTitle()) + " - yz-projection");

  if (m_CanvasXY == nullptr) {
    m_CanvasXY = new TCanvas(m_CanvasTitle + "_XY", m_Title + "_XY", 40, 40, 600, 600);
  }
  if (m_CanvasXY != nullptr && m_HistXY != nullptr) {
    m_CanvasXY->cd();
    m_HistXY->Draw(m_DrawOptionString);
    m_CanvasXY->Update();
  }

  if (m_CanvasXZ == nullptr) {
    m_CanvasXZ = new TCanvas(m_CanvasTitle + "_XZ", m_Title + "_XZ", 40, 40, 600, 600);
  }
  if (m_CanvasXZ != nullptr && m_HistXZ != nullptr) {
    m_CanvasXZ->cd();
    m_HistXZ->Draw(m_DrawOptionString);
    m_CanvasXZ->Update();
  }

  if (m_CanvasYZ == nullptr) {
    m_CanvasYZ = new TCanvas(m_CanvasTitle + "_YZ", m_Title + "_YZ", 40, 40, 600, 600);
  }
  if (m_CanvasYZ != nullptr && m_HistYZ != nullptr) {
    m_CanvasYZ->cd();
    m_HistYZ->Draw(m_DrawOptionString);
    m_CanvasYZ->Update();
  } 

  gSystem->ProcessEvents();

  SetCreated();

  return;
}


////////////////////////////////////////////////////////////////////////////////


//! Determine the maximum and its coordiantes, the vector is filled up to the number of dimensions the histogram has
void MImage3D::DetermineMaximum(double& MaxValue, vector<double>& Coordinate)
{
  Coordinate.clear();

  if (GetNEntries() <= 0 || m_IA == nullptr) {
    MaxValue = 0;
    return;
  }

  MaxValue = m_IA[0];
  int xMaxIndex = 0;
  int yMaxIndex = 0;
  int zMaxIndex = 0;
  
  for (int x = 0; x < m_xNBins; ++x) {
    for (int y = 0; y < m_yNBins; ++y) {
      for (int z = 0; z < m_zNBins; ++z) {
        if (x == 0 && y == 0 && z == 0) continue;
        if (m_IA[x + y*m_xNBins + z*m_xNBins*m_yNBins] > MaxValue) {
          MaxValue = m_IA[x + y*m_xNBins + z*m_xNBins*m_yNBins];
          xMaxIndex = x;
          yMaxIndex = y;
          zMaxIndex = z;
        }
      }
    }
  }
  
  
  Coordinate.push_back((xMaxIndex + 0.5) * (m_xMax-m_xMin)/m_xNBins + m_xMin);
  Coordinate.push_back((yMaxIndex + 0.5) * (m_yMax-m_yMin)/m_yNBins + m_yMin);
  Coordinate.push_back((zMaxIndex + 0.5) * (m_zMax-m_zMin)/m_zNBins + m_zMin);
}


// MImage3D.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
