/*
 * MImageSpheric.cxx
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
// MImageSpheric
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MImageSpheric.h"

// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:
#include <TMath.h>
#include <TText.h>
#include <TMarker.h>
#include <TGaxis.h>
#include <TAxis.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MVector.h"
#include "MPointSourceSelector.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MImageSpheric)
#endif


////////////////////////////////////////////////////////////////////////////////


MImageSpheric::MImageSpheric() : MImage2D()
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MImageSpheric::MImageSpheric(MString Title, double* IA, 
                             MString xTitle, double xMin, double xMax, int xNBins, 
                             MString yTitle, double yMin, double yMax, int yNBins, 
                             MString vTitle, int Spectrum, int DrawOption) :
  MImage2D(Title, IA, xTitle, xMin, xMax, xNBins, yTitle, yMin, 
           yMax, yNBins, vTitle, Spectrum, DrawOption), m_YAxis(0)
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
  // vTitle:     title of the value axis
  // Spectrum:   spectrum
  // DrawOption: ROOT draw option
}


////////////////////////////////////////////////////////////////////////////////


MImageSpheric::~MImageSpheric()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


MImage* MImageSpheric::Clone()
{
  //! Clone this image

  MImage* I = 
    new MImageSpheric(m_Title, m_IA, 
                      m_xTitle, m_xMin, m_xMax, m_xNBins, 
                      m_yTitle, m_yMin, m_yMax, m_yNBins, 
                      m_vTitle, m_Spectrum, m_DrawOption);
    
  I->Normalize(m_Normalize);

  return I;
}


////////////////////////////////////////////////////////////////////////////////


void MImageSpheric::SetImageArray(double* IA)
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

    double Content = 0.0;
    for (int x = 1; x <= m_xNBins; ++x) {
      for (int y = 1; y <= m_yNBins; ++y) {
        Content = m_IA[(x-1) + (y-1) * m_xNBins];
        if (!TMath::IsNaN(Content)) {
          if (TMath::Finite(Content)) {
            // Make sure to invert y-axis
            dynamic_cast<TH2*>(m_Histogram)->SetBinContent(x, m_yNBins-y+1, Content);
          } else {
            merr<<"Image contains Inf ("<<x<<", "<<y<<")"<<show;
          }
        } else {
          merr<<"Image contains NaN ("<<x<<", "<<y<<")"<<show;
        }
      }
    }
    m_Histogram->Draw(m_DrawOptionString);
    //AddNamedSources();
    m_YAxis->Draw();
    m_Canvas->Update();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MImageSpheric::Display(TCanvas* Canvas)
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
  
  // m_Canvas->cd(); // No cd in order to allow the user to use a sub-pad here

  bool IsNew = false;
  TH2D* Hist = 0;
  if (m_Histogram == 0) {
    Hist = new TH2D(m_CanvasTitle + "ImageSpheric" + m_IDCounter, m_Title, m_xNBins, m_xMin, m_xMax, m_yNBins, m_yMin, m_yMax);
    Hist->SetContour(50);
    m_Histogram = dynamic_cast<TH1*>(Hist);

    Hist->SetStats(false);
    Hist->GetXaxis()->SetTitle(m_xTitle);
    Hist->GetXaxis()->CenterTitle();

    Hist->SetZTitle(m_vTitle);
    
    IsNew = true;
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
          // Make sure to invert y-axis
          Hist->SetBinContent(x, m_yNBins-y+1, m_IA[(x-1) + (y-1)*m_xNBins]);
        } else {
          merr<<"Image contains Inf ("<<x<<", "<<y<<")"<<show;
        }
      } else {
        merr<<"Image contains NaN ("<<x<<", "<<y<<")"<<show;
      }
    }
  }
  
  // Rescale to 1:
  if (m_Normalize == true && Hist->GetMaximum() > 0) {
    Hist->Scale(1.0/Hist->GetMaximum());
  }

  if (IsNew == true) {
    Hist->Draw(m_DrawOptionString);

    // Redraw the new axis
    gPad->Update();
    if (m_DrawOption == c_COLZ || m_DrawOption == c_COL) {
      m_YAxis = new TGaxis(gPad->GetUxmin(),
                           gPad->GetUymax(),
                           gPad->GetUxmin()-0.001,
                           gPad->GetUymin(),
                           Hist->GetYaxis()->GetXmin(),
                           Hist->GetYaxis()->GetXmax(),
                           510,"+");
      m_YAxis->ImportAxisAttributes(Hist->GetYaxis());
      m_YAxis->CenterTitle(true);
      m_YAxis->SetTitle(m_yTitle);
      m_YAxis->SetLabelOffset(-0.03f);
      m_YAxis->SetTitleOffset(1.5f);
    } else {
      m_YAxis = new TGaxis(gPad->GetUxmin(),
                           gPad->GetUymax(),
                           0.9999*gPad->GetUxmin(),
                           gPad->GetUymin(),
                           Hist->GetYaxis()->GetXmin(),
                           Hist->GetYaxis()->GetXmax(),
                           510,"+");
      m_YAxis->ImportAxisAttributes(Hist->GetYaxis());
      m_YAxis->CenterTitle(true);
      m_YAxis->SetTitle(m_yTitle);
      m_YAxis->SetLabelOffset(-0.03f);
      m_YAxis->SetTitleOffset(1.5f);
      m_YAxis->SetBit(TAxis::kRotateTitle);
    }


    // Remove the current axis
    Hist->GetYaxis()->SetLabelOffset(999);
    Hist->GetYaxis()->SetTickLength(0);
  
    // Draw the new one
    m_YAxis->Draw();
  } else {
    m_YAxis->Draw();
  }

  m_Canvas->Update();

  return;
}


// MImageSpheric.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
