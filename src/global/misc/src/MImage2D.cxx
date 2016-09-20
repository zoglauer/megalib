/*
 * MImage2D.cxx
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
// MImage2D
//
// Base class for all other image classes:
//
// Its designed for 2D-Images
//
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MImage2D.h"

// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:
#include <TMath.h>

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MImage2D)
#endif


////////////////////////////////////////////////////////////////////////////////


MImage2D::MImage2D() : MImage()
{
  // Constructor initialising an empty image

  // Initialization is mostly done in the MImage constructor
  SetYAxis("y-Axis", 0, 1, 1);
}


////////////////////////////////////////////////////////////////////////////////


MImage2D::MImage2D(MString Title, double* IA, 
                   MString xTitle, double xMin, double xMax, int xNBins, 
                   MString yTitle, double yMin, double yMax, int yNBins, 
                   MString vTitle, int Spectrum, int DrawOption) :
  MImage(Title, IA, xTitle, xMin, xMax, xNBins, vTitle, Spectrum, DrawOption)
{
  // Construct an image but do not display it, i.e. save only the data
  // 
  // Title:      Title of the image, 0 means no title
  // IA:         data-array
  // NEntries:   length of the data array
  // xTitle:     title of the x axis
  // xMin:       minimum x-value
  // xMax:       maximum x-value
  // xNBins:     number of bins in x
  // yTitle:     title of the y-axis
  // yMin:       minimum y-value
  // yMax:       maximum y-value
  // yNBins:     number of bins in y
  // Spectrum:   spectrum
  // DrawOption: ROOT draw option

  m_NEntries = xNBins*yNBins;

  SetYAxis(yTitle, yMin, yMax, yNBins);
  SetImageArray(IA);
}


////////////////////////////////////////////////////////////////////////////////


MImage2D::~MImage2D()
{
  // standard destructor
}


////////////////////////////////////////////////////////////////////////////////


MImage* MImage2D::Clone()
{
  //! Clone this image

  MImage2D* I = 
    new MImage2D(m_Title, m_IA, 
                 m_xTitle, m_xMin, m_xMax, m_xNBins, 
                 m_yTitle, m_yMin, m_yMax, m_yNBins, 
                 m_vTitle, m_Spectrum, m_DrawOption);

  I->Normalize(m_Normalize);

  return I;
}


////////////////////////////////////////////////////////////////////////////////


void MImage2D::SetImageArray(double* IA)
{
  // Copy the data array

  if (m_IA != 0) delete [] m_IA;
  m_IA = new double[m_NEntries];

  for (int x = 0; x < m_NEntries; x++) {
    if (IA != 0) {
      m_IA[x] = IA[x];
    } else {
      m_IA[x] = 0.0;
    }
  }

  if (dynamic_cast<TH2*>(m_Histogram) != 0) {
    m_Canvas->cd();

    double Content = 0;
    for (int x = 1; x <= m_xNBins; ++x) {
      for (int y = 1; y <= m_yNBins; ++y) {
        Content = m_IA[(x-1) + (y-1) * m_xNBins];
        if (!TMath::IsNaN(Content)) {  
          if (TMath::Finite(Content)) {
            dynamic_cast<TH2*>(m_Histogram)->SetBinContent(x, y, Content);
          } else {
            merr<<"Image contains Inf ("<<x<<", "<<y<<")"<<show;
          }
        } else {
          merr<<"Image contains NaN ("<<x<<", "<<y<<")"<<show;
        }
      }
    }
    m_Histogram->Scale(1.0/m_Histogram->GetMaximum());
    m_Histogram->Draw(m_DrawOptionString);
    m_Canvas->Update();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MImage2D::SetYAxis(MString yTitle, double yMin, double yMax, int yNBins)
{
  // Set the data of the y-axis

  m_yTitle = yTitle;
  m_yMin = yMin;
  m_yMax = yMax;
  m_yNBins = yNBins;
}


////////////////////////////////////////////////////////////////////////////////


void MImage2D::Display(TCanvas* Canvas)
{
  // Display the image in a canvas

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

  if (m_xNBins > m_yNBins) {
    m_Canvas->SetWindowSize(900, int(900.0/m_xNBins*m_yNBins));
  } else {
    m_Canvas->SetWindowSize(int(700.0/m_yNBins*m_xNBins), 700);
  }


  TH2D* Hist = 0;
  if (m_Histogram == 0) {  
    Hist = new TH2D(m_Title, m_Title, m_xNBins, m_xMin, m_xMax, m_yNBins, m_yMin, m_yMax);
    m_Histogram = dynamic_cast<TH1*>(Hist);

    //Hist->SetDirectory(0);
    Hist->SetXTitle(m_xTitle);
    Hist->SetYTitle(m_yTitle);
    Hist->SetZTitle(m_vTitle);
    Hist->SetFillColor(0);

    Hist->SetTitleOffset(1.2f, "X");
    Hist->SetTitleOffset(1.2f, "Y");

    Hist->SetLabelSize(0.03f, "X");
    Hist->SetLabelSize(0.03f, "Y");
    Hist->SetLabelSize(0.03f, "Z");

    Hist->SetTickLength(-0.02f, "X");
    Hist->SetTickLength(-0.02f, "Y");
    
    Hist->SetLabelOffset(0.025f, "X");
    Hist->SetLabelOffset(0.020f, "Y");

    Hist->SetStats(false);
    Hist->SetContour(50);
  } else {
    Hist = dynamic_cast<TH2D*>(m_Histogram);
    Hist->SetTitle(m_Title);
  }

  double Content = 0.0;
  for (int x = 1; x <= m_xNBins; ++x) {
    for (int y = 1; y <= m_yNBins; ++y) {
      Content = m_IA[(x-1) + (y-1) * m_xNBins];
      if (!TMath::IsNaN(Content)) {
        if (TMath::Finite(Content)) {
          Hist->SetBinContent(x, y, Content);
        } else {
          merr<<"Image contains Inf ("<<x<<", "<<y<<")"<<endl;
        }
      } else {
        merr<<"Image contains NaN ("<<x<<", "<<y<<")"<<endl;
      }
    }
  }
  
  // Rescale to 1:
  if (m_Normalize == true && Hist->GetMaximum() > 0) {
    Hist->Scale(1.0/Hist->GetMaximum());
  }
  
  Hist->Draw(m_DrawOptionString);

  return;
}


////////////////////////////////////////////////////////////////////////////////


//! Determine the maximum and its coordiantes, the vector is filled up to the number of dimensions the histogram has
void MImage2D::DetermineMaximum(double& MaxValue, vector<double>& Coordinate)
{
  MaxValue = 0;
  double xMaxIndex = 0;
  double yMaxIndex = 0;
  
  for (int x = 0; x < m_xNBins; ++x) {
    for (int y = 0; y < m_yNBins; ++y) {
      if (m_IA[x + y * m_xNBins] > MaxValue) {
        MaxValue = m_IA[x + y * m_xNBins];
        xMaxIndex = x;
        yMaxIndex = y;
      }
    }
  }
  
  
  Coordinate.clear();
  Coordinate.push_back((xMaxIndex + 0.5) * (m_xMax-m_xMin)/m_xNBins + m_xMin);
  Coordinate.push_back((yMaxIndex + 0.5) * (m_yMax-m_yMin)/m_yNBins + m_yMin);
}


////////////////////////////////////////////////////////////////////////////////

/*
void MImage2D::ExportFits()
{
  fitsfile *fptr;
  
  int status;
  long exposure;

  long naxis = 2;
  long naxes[2];
  naxes[0] = m_xNBins;
  naxes[1] = m_yNBins;


  status = 0;
  fits_create_file(&fptr, "!testfile.fits", &status);
  cout<<"1. "<<endl;
  fits_report_error(stderr, status);


  fits_create_img(fptr, DOUBLE_IMG, naxis, naxes, &status);
  cout<<"2. "<<endl;
  fits_report_error(stderr, status);

  exposure = 1500;

  fits_update_key(fptr, TLONG, m_Title, &exposure, "more text", &status);
  cout<<"3. "<<endl;
  fits_report_error(stderr, status);

  double Image[m_NEntries];

  for (int i = 0; i < m_yNBins; i++)
    for (int j = 0; j < m_xNBins; j++) {
      Image[j + i*m_yNBins] = m_IA[j + (m_yNBins - i -1) * m_yNBins];
    }

  fits_write_img(fptr, TDOUBLE, 1, m_NEntries, Image, &status);
  cout<<"4. "<<endl;
  fits_report_error(stderr, status);

  fits_close_file(fptr, &status);
  cout<<"5. "<<endl;
  fits_report_error(stderr, status);
  
  fits_report_error(stderr, status);
  cout<<status<<endl;
}
*/


// MImage2D": the end...
////////////////////////////////////////////////////////////////////////////////
