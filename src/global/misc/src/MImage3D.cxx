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


#ifdef ___CINT___
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
  MImage2D(Title, IA, xTitle, xMin, xMax, xNBins,  
           yTitle, yMin, yMax, yNBins, vTitle, Spectrum, DrawOption)
{
  // standard constructor

  m_NEntries = xNBins*yNBins*zNBins;

  SetZAxis(zTitle, zMin, zMax, zNBins);
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

  if (m_IA != 0) delete [] m_IA;
  m_IA = new double[m_NEntries];

  for (int x = 0; x < m_NEntries; x++) {
    if (IA != 0) {
      m_IA[x] = IA[x];
    } else {
      m_IA[x] = 0.0;
    } 
  }

  if (dynamic_cast<TH3D*>(m_Histogram) != 0) {
    m_Canvas->cd();

    double Content = 0.0;
    for (int x = 1; x <= m_xNBins; ++x) {
      for (int y = 1; y <= m_yNBins; ++y) {
        for (int z = 1; z <= m_zNBins; ++z) {
          Content = m_IA[(x-1) + (y-1)*m_xNBins + (z-1)*m_xNBins*m_yNBins];
          if (!TMath::IsNaN(Content)) {
            if (TMath::Finite(Content)) {
              ((TH3D*) m_Histogram)->SetBinContent(x, y, z, Content);
            } else {
              merr<<"Image contains Inf ("<<x<<", "<<y<<")"<<show;
            }
          } else {
            merr<<"Image contains NaN ("<<x<<", "<<y<<")"<<show;
          }
        }
      }
    }
    m_Histogram->Draw();
    m_Canvas->Update();
    
    m_CanvasXY->cd();
    m_HistXY->Reset();
    m_HistXY->Add(dynamic_cast<TH3D*>(m_Histogram)->Project3D("xy"));    
    m_HistXY->SetTitle(MString(m_Histogram->GetTitle()) + " - xy-projection");
    m_HistXY->Draw(m_DrawOptionString);
    m_CanvasXY->Update();

    m_CanvasXZ->cd();
    m_HistXZ->Reset();
    m_HistXZ->Add(dynamic_cast<TH3*>(m_Histogram)->Project3D("xz"));    
    m_HistXZ->Draw(m_DrawOptionString);
    m_HistXZ->SetTitle(MString(m_Histogram->GetTitle()) + " - xz-projection");
    m_CanvasXZ->Update();

    m_CanvasYZ->cd();
    m_HistYZ->Reset();
    m_HistYZ->Add(dynamic_cast<TH3*>(m_Histogram)->Project3D("yz"));    
    m_HistYZ->Draw(m_DrawOptionString);
    m_HistYZ->SetTitle(MString(m_Histogram->GetTitle()) + " - yz-projection");
    m_CanvasYZ->Update(); 
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
    Hist = new TH3D(m_Title + m_IDCounter, m_Title, m_xNBins, m_xMin, m_xMax, m_yNBins, m_yMin, m_yMax, m_zNBins, m_zMin, m_zMax);
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

  Hist->Draw();


  // Create projections, and draw them too:
  
  if (m_HistXY == 0) {
    m_HistXY = Hist->Project3D("xy");
  } else {
    m_HistXY->Reset();
    m_HistXY->Add(Hist->Project3D("xy"));
  }
  m_HistXY->SetTitle(MString(Hist->GetTitle()) + " - xy-projection");
  
  if (m_HistXZ == 0) {
    m_HistXZ = Hist->Project3D("xz");
  } else {
    m_HistXZ->Reset();
    m_HistXZ->Add(Hist->Project3D("xz"));
  }
  m_HistXZ->SetTitle(MString(Hist->GetTitle()) + " - xz-projection");
  
  if (m_HistYZ == 0) {
    m_HistYZ = Hist->Project3D("yz");
  } else {
    m_HistYZ->Reset();
    m_HistYZ->Add(Hist->Project3D("yz"));
  }
  m_HistYZ->SetTitle(MString(Hist->GetTitle()) + " - yz-projection");

  if (m_CanvasXY == 0) {
    m_CanvasXY = new TCanvas(m_CanvasTitle + "_XY", m_Title + "_XY", 40, 40, 600, 600);
  }
  m_CanvasXY->cd();
  m_HistXY->Draw(m_DrawOptionString);
  m_CanvasXY->Update();

  if (m_CanvasXZ == 0) {
    m_CanvasXZ = new TCanvas(m_CanvasTitle + "_XZ", m_Title + "_XZ", 40, 40, 600, 600);
  }
  m_CanvasXZ->cd();
  m_HistXZ->Draw(m_DrawOptionString);
  m_CanvasXZ->Update();

  if (m_CanvasYZ == 0) {
    m_CanvasYZ = new TCanvas(m_CanvasTitle + "_YZ", m_Title + "_YZ", 40, 40, 600, 600);
  }
  m_CanvasYZ->cd();
  m_HistYZ->Draw(m_DrawOptionString);
  m_CanvasYZ->Update(); 

  gSystem->ProcessEvents();

  return;
}


////////////////////////////////////////////////////////////////////////////////


//! Determine the maximum and its coordiantes, the vector is filled up to the number of dimensions the histogram has
void MImage3D::DetermineMaximum(double& MaxValue, vector<double>& Coordinate)
{
  MaxValue = 0;
  double xMaxIndex = 0;
  double yMaxIndex = 0;
  double zMaxIndex = 0;
  
  for (int x = 0; x < m_xNBins; ++x) {
    for (int y = 0; y < m_yNBins; ++y) {
      for (int z = 0; z < m_zNBins; ++z) {
        if (m_IA[x + y*m_xNBins + z*m_xNBins*m_yNBins] > MaxValue) {
          MaxValue = m_IA[x + y*m_xNBins + z*m_xNBins*m_yNBins];
          xMaxIndex = x;
          yMaxIndex = y;
          zMaxIndex = z;
        }
      }
    }
  }
  
  
  Coordinate.clear();
  Coordinate.push_back((xMaxIndex + 0.5) * (m_xMax-m_xMin)/m_xNBins + m_xMin);
  Coordinate.push_back((yMaxIndex + 0.5) * (m_yMax-m_yMin)/m_yNBins + m_yMin);
  Coordinate.push_back((zMaxIndex + 0.5) * (m_zMax-m_zMin)/m_zNBins + m_zMin);
}


// MImage3D.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
